#include "AIReading.h"
#include "CardModel.h"
#include "Config.h"

#include <curl/curl.h>
#include <json/json.h>

#include <iostream>
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
		for (size_t i = 0; i < cards.size(); ++i)
			prompt << "\n- " << cards[i].displayName;
	} else if (spreadType == TREE_OF_LIFE) {
		prompt = "Provide a tarot card reading for the following ten cards drawn in a Tree of Life "
				 "spread: ";
		const char* positions[] = {"1. Kether (The Crown) - Highest spiritual aspirations",
			"2. Chokmah (Wisdom) - Spiritual potential realized",
			"3. Binah (Understanding) - Spiritual limitations and restrictions",
			"4. Chesed (Mercy) - Constructive influences and prosperity",
			"5. Geburah (Severity) - Destructive influences and misuse of power",
			"6. Tiphareth (Beauty) - Your true self, the heart of the matter",
			"7. Netzach (Victory) - Your emotional state, love, and passion",
			"8. Hod (Splendor) - Your intellectual state, communication, and work",
			"9. Yesod (Foundation) - Your unconscious state, intuition, and dreams",
			"10. Malkuth (Kingdom) - The final outcome and material manifestation"};
		for (size_t i = 0; i < cards.size() && i < 10; ++i)
			prompt << "\n- " << positions[i] << ": " << cards[i].displayName;
	}

	prompt
		<< ". Give a detailed, insightful reading focusing on the combined meaning of these cards. "
		   "Provide a comprehensive interpretation that explores the nuances of the cards' "
		   "interactions. "
		   "Keep the response to 5-7 sentences. Do not use markdown or any special formatting.";

	std::cout << "AI Prompt: " << prompt.String() << std::endl;

	// Initialize curl
	CURL* curl = curl_easy_init();
	if (!curl)
		return BString("Error: Could not initialize CURL for API request.");

	// Prepare the JSON payload
	Json::Value jsonPayload;
	jsonPayload["model"] = "deepseek-chat";
	jsonPayload["messages"][0]["role"] = "user";
	jsonPayload["messages"][0]["content"] = prompt.String();
	jsonPayload["max_tokens"] = Config::kAPIMaxTokens;
	jsonPayload["temperature"] = Config::kAPITemperature;

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
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, Config::kAPITimeout); // 30 second timeout

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
		// Check if the response was cut off due to max_tokens
		if (jsonResponse["choices"][0].isMember("finish_reason")
			&& jsonResponse["choices"][0]["finish_reason"].asString() == "length") {
			// Append a message indicating the response was cut off
			content += " [Response truncated due to token limit]";
		}
		return BString(content.c_str());
	}

	return BString("Error: Unexpected API response format.");
}
