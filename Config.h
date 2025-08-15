#pragma once

#include <String.h>

class Config {
public:
	static BString GetAPIKey();
	static bool IsAPIKeySet();
	static void SetAPIKey(const BString& apiKey);
	static BString LoadAPIKeyFromFile();

private:
	static BString sAPIKey;
	static void SaveAPIKeyToFile(const BString& apiKey);
};
