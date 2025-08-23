#pragma once

#include <String.h>
#include <boost/json.hpp>
#include <string>
#include <vector>

namespace json = boost::json;

class JSONParser {
public:
	static BString ParseAPIResponse(const BString& jsonResponse);
	static BString BuildPayload(const BString& prompt, int maxTokens, float temperature);

private:
	static bool HasError(const json::value& jsonValue);
	static BString ExtractErrorMessage(const json::value& jsonValue);
	static BString ExtractContent(const json::value& jsonValue);
	static bool IsResponseTruncated(const json::value& jsonValue);
};
