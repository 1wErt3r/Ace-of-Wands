#include "Config.h"

#include <stdlib.h> // for getenv

BString
Config::GetAPIKey()
{
	// First try to get the API key from environment variable
	const char* apiKey = getenv("OPENAI_API_KEY");
	if (apiKey != NULL) {
		return BString(apiKey);
	}
	
	// If no environment variable, return empty string
	return BString("");
}

bool
Config::IsAPIKeySet()
{
	return !GetAPIKey().IsEmpty();
}