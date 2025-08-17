#pragma once

#include "CardPresenter.h"
#include <String.h>

class Config {
public:
	static BString GetAPIKey();
	static bool IsAPIKeySet();
	static void SetAPIKey(const BString& apiKey);
	static BString LoadAPIKeyFromFile();

	static void SetSpread(SpreadType spread);
	static SpreadType GetSpread();
	static SpreadType LoadSpreadFromFile();

private:
	static BString sAPIKey;
	static SpreadType sSpread;
	static void SaveAPIKeyToFile(const BString& apiKey);
	static void SaveSpreadToFile(SpreadType spread);
};
