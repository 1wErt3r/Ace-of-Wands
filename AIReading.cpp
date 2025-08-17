#include "AIReading.h"
#include "CardModel.h"
#include "Config.h"

#include <curl/curl.h>
#include <json/json.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Callback function to write received data into a string
static size_t
WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response)
{
	size_t realsize = size * nmemb;
	response->append((char*)contents, realsize);
	return realsize;
}


BString
AIReading::GetReading(const std::vector<CardInfo>& cards, SpreadType spreadType)
{
	// Check if API key is set
	if (!Config::IsAPIKeySet()) {
		BString message
			= "DeepSeek API key not set. Please set the DEEPSEEK_API_KEY environment variable.\n\n";
		message += "Card spread: ";

		for (size_t i = 0; i < cards.size(); i++) {
			if (i > 0)
				message += ", ";
			message += cards[i].displayName;
		}

		return message;
	}

	// Build the prompt from the card names
	BString prompt;
	if (spreadType == THREE_CARD) {
		prompt = "Provide a tarot card reading for the following three cards drawn in a simple "
				 "spread: ";
	} else {
		prompt = "Provide a tarot card reading for the following ten cards drawn in a Tree of Life "
				 "spread: ";
	}

	for (size_t i = 0; i < cards.size(); i++) {
		if (i > 0)
			prompt += ", ";
		prompt += cards[i].displayName;
	}

	prompt += ". Give a brief, insightful reading focusing on the combined meaning of these cards. "
			  "Keep the response to 3-4 sentences. Do not use markdown or any special formatting.";

	// Initialize curl
	CURL* curl = curl_easy_init();
	if (!curl)
		return BString("Error: Could not initialize CURL for API request.");

	// Prepare the JSON payload
	Json::Value jsonPayload;
	jsonPayload["model"] = "deepseek-chat";
	jsonPayload["messages"][0]["role"] = "user";
	jsonPayload["messages"][0]["content"] = prompt.String();
	jsonPayload["max_tokens"] = 150;
	jsonPayload["temperature"] = 0.7;

	Json::StreamWriterBuilder builder;
	builder["indentation"] = ""; // Compact formatting
	std::string jsonString = Json::writeString(builder, jsonPayload);

	// Set up the request
	std::string response;
	struct curl_slist* headers = NULL;

	// Add headers
	BString authHeader = "Authorization: Bearer ";
	authHeader += Config::GetAPIKey();
	headers = curl_slist_append(headers, authHeader.String());
	headers = curl_slist_append(headers, "Content-Type: application/json");

	curl_easy_setopt(curl, CURLOPT_URL, "https://api.deepseek.com/v1/chat/completions");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L); // 30 second timeout

	// Perform the request
	CURLcode res = curl_easy_perform(curl);

	// Clean up
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	if (res != CURLE_OK) {
		BString errorMsg = "Error: API request failed with error: ";
		errorMsg += curl_easy_strerror(res);
		return errorMsg;
	}

	// Parse the JSON response
	Json::CharReaderBuilder readerBuilder;
	Json::Value jsonResponse;
	std::string errs;

	std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
	bool parsingSuccessful
		= reader->parse(response.c_str(), response.c_str() + response.size(), &jsonResponse, &errs);

	if (!parsingSuccessful) {
		BString errorMsg = "Error: Failed to parse API response: ";
		errorMsg += errs.c_str();
		return errorMsg;
	}

	// Check for API errors
	if (jsonResponse.isMember("error")) {
		BString errorMsg = "API Error: ";
		if (jsonResponse["error"].isMember("message"))
			errorMsg += jsonResponse["error"]["message"].asString().c_str();
		else
			errorMsg += "Unknown API error";
		return errorMsg;
	}

	// Extract the response text
	if (jsonResponse.isMember("choices") && jsonResponse["choices"].isArray()
		&& jsonResponse["choices"].size() > 0 && jsonResponse["choices"][0].isMember("message")
		&& jsonResponse["choices"][0]["message"].isMember("content")) {
		std::string content = jsonResponse["choices"][0]["message"]["content"].asString();
		return BString(content.c_str());
	}

	return BString("Error: Unexpected API response format.");
}
