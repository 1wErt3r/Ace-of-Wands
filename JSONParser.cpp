#include "JSONParser.h"
#include "Config.h"
#include <iostream>


BString
JSONParser::ParseAPIResponse(const BString& jsonResponse)
{
	try {
		json::value jsonValue = json::parse(jsonResponse.String());

		if (HasError(jsonValue))
			return ExtractErrorMessage(jsonValue);

		BString content = ExtractContent(jsonValue);
		if (content.IsEmpty())
			return BString("Error: Unexpected API response format.");

		if (IsResponseTruncated(jsonValue))
			content += " [Response truncated due to token limit]";

		return content;
	} catch (const std::exception& e) {
		BString errorMsg = "Error: Failed to parse API response: ";
		errorMsg += e.what();
		errorMsg += "\nResponse: ";
		errorMsg += jsonResponse.String();
		return errorMsg;
	}
}


BString
JSONParser::BuildPayload(const BString& prompt, int maxTokens, float temperature)
{
	json::object payload;

	payload["model"] = "deepseek-chat";

	json::array messages;
	json::object message;
	message["role"] = "user";
	message["content"] = prompt.String();
	messages.push_back(message);

	payload["messages"] = messages;
	payload["max_tokens"] = maxTokens;
	payload["temperature"] = temperature;

	return BString(json::serialize(payload).c_str());
}


bool
JSONParser::HasError(const json::value& jsonValue)
{
	if (!jsonValue.is_object())
		return false;

	const json::object& obj = jsonValue.as_object();
	return obj.contains("error");
}


BString
JSONParser::ExtractErrorMessage(const json::value& jsonValue)
{
	const json::object& obj = jsonValue.as_object();
	const json::value& error = obj.at("error");

	if (!error.is_object())
		return BString("API Error: Unknown error format");

	const json::object& errorObj = error.as_object();

	if (errorObj.contains("message") && errorObj.at("message").is_string()) {
		BString errorMsg = "API Error: ";
		errorMsg += errorObj.at("message").as_string().c_str();
		return errorMsg;
	}

	return BString("API Error: Unknown API error");
}


BString
JSONParser::ExtractContent(const json::value& jsonValue)
{
	if (!jsonValue.is_object())
		return BString();

	const json::object& obj = jsonValue.as_object();

	if (!obj.contains("choices") || !obj.at("choices").is_array())
		return BString();

	const json::array& choices = obj.at("choices").as_array();
	if (choices.empty())
		return BString();

	const json::value& firstChoice = choices[0];
	if (!firstChoice.is_object())
		return BString();

	const json::object& choiceObj = firstChoice.as_object();

	if (!choiceObj.contains("message") || !choiceObj.at("message").is_object())
		return BString();

	const json::object& messageObj = choiceObj.at("message").as_object();

	if (!messageObj.contains("content") || !messageObj.at("content").is_string())
		return BString();

	return BString(messageObj.at("content").as_string().c_str());
}


bool
JSONParser::IsResponseTruncated(const json::value& jsonValue)
{
	if (!jsonValue.is_object())
		return false;

	const json::object& obj = jsonValue.as_object();

	if (!obj.contains("choices") || !obj.at("choices").is_array())
		return false;

	const json::array& choices = obj.at("choices").as_array();
	if (choices.empty())
		return false;

	const json::value& firstChoice = choices[0];
	if (!firstChoice.is_object())
		return false;

	const json::object& choiceObj = firstChoice.as_object();

	if (!choiceObj.contains("finish_reason") || !choiceObj.at("finish_reason").is_string())
		return false;

	std::string finishReason = choiceObj.at("finish_reason").as_string().c_str();
	return finishReason == "length";
}
