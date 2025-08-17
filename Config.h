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

	static void SaveSettingsToFile();
	static void LoadSettingsFromFile();

	// UI Constants
	static const float kInitialCardWidth;
	static const float kInitialCardHeight;
	static const float kInitialLabelHeight;
	static const float kInitialFontSize;
	static const float kImageInset;
	static const float kLabelHeightMargin;
	static const float kCardWidthMargin;
	static const float kReadingAreaWidthRatio;
	static const float kMarginX;
	static const float kMarginY;
	static const float kCardSpacing;
	static const float kCardAspectRatio;
	static const float kLabelHeightRatio;
	static const float kMinLabelHeight;
	static const float kMaxLabelHeight;
	static const float kMinCardWidth;
	static const float kMinCardHeight;
	static const float kFontSizeRatio;
	static const float kMinFontSize;
	static const float kMaxFontSize;
	static const float kTreeOfLifeCardWidthRatio;
	static const float kReadingAreaInset;

	// Game Logic Constants
	static const int kThreeCardSpreadCount;
	static const int kTreeOfLifeSpreadCount;

	// API Constants
	static const int kAPIMaxTokens;
	static const double kAPITemperature;
	static const long kAPITimeout;

	// Main Window Constants
	static const float kMainWindowLeft;
	static const float kMainWindowTop;
	static const float kMainWindowRight;
	static const float kMainWindowBottom;

	// Settings Window Constants
	static const float kSettingsWindowLeft;
	static const float kSettingsWindowTop;
	static const float kSettingsWindowRight;
	static const float kSettingsWindowBottom;

private:
	static BString sAPIKey;
	static SpreadType sSpread;
	static void SaveAPIKeyToFile(const BString& apiKey);
};
