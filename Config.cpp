#include "Config.h"

#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>
#include <stdio.h>
#include <stdlib.h> // for getenv

// Static member definition
BString Config::sAPIKey = "";
SpreadType Config::sSpread = THREE_CARD;

// UI Constants
const float Config::kInitialCardWidth = 150;
const float Config::kInitialCardHeight = 210;
const float Config::kInitialLabelHeight = 40;
const float Config::kInitialFontSize = 18.0f;
const float Config::kImageInset = 10;
const float Config::kLabelHeightMargin = 10;
const float Config::kCardWidthMargin = 10;
const float Config::kReadingAreaWidthRatio = 0.3;
const float Config::kMarginX = 20;
const float Config::kMarginY = 20;
const float Config::kCardSpacing = 20.0f;
const float Config::kCardAspectRatio = 1.4f;
const float Config::kLabelHeightRatio = 0.15f;
const float Config::kMinLabelHeight = 30;
const float Config::kMaxLabelHeight = 60;
const float Config::kMinCardWidth = 100;
const float Config::kMinCardHeight = 140;
const float Config::kFontSizeRatio = 0.5f;
const float Config::kMinFontSize = 12;
const float Config::kMaxFontSize = 24;
const float Config::kTreeOfLifeCardWidthRatio = 4.5;
const float Config::kReadingAreaInset = 10;

// Game Logic Constants
const int Config::kThreeCardSpreadCount = 3;
const int Config::kTreeOfLifeSpreadCount = 10;

// API Constants
const int Config::kAPIMaxTokens = 300; // Increased to allow for longer responses
const double Config::kAPITemperature = 0.7;
const long Config::kAPITimeout = 30L;

// Main Window Constants
const float Config::kMainWindowLeft = 100;
const float Config::kMainWindowTop = 100;
const float Config::kMainWindowRight = 800;
const float Config::kMainWindowBottom = 600;

// Settings Window Constants
const float Config::kSettingsWindowLeft = 100;
const float Config::kSettingsWindowTop = 100;
const float Config::kSettingsWindowRight = 500;
const float Config::kSettingsWindowBottom = 350;


BString
Config::GetAPIKey()
{
	// First try to get the API key from environment variable
	const char* envApiKey = getenv("DEEPSEEK_API_KEY");
	if (envApiKey != NULL)
		return BString(envApiKey);

	// If no environment variable, use the stored key
	if (!sAPIKey.IsEmpty())
		return sAPIKey;

	// If no stored key, return empty string
	return BString("");
}


bool
Config::IsAPIKeySet()
{
	// Check if we have an API key from either environment or stored value
	return !GetAPIKey().IsEmpty();
}


void
Config::SetAPIKey(const BString& apiKey)
{
	sAPIKey = apiKey;
	// Save the API key to a file
	SaveAPIKeyToFile(apiKey);
}


void
Config::SaveAPIKeyToFile(const BString& apiKey)
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return;

	path.Append("AceOfWands");

	// Create the directory if it doesn't exist
	BDirectory dir;
	if (dir.CreateDirectory(path.Path(), &dir) != B_OK && dir.SetTo(path.Path()) != B_OK)
		return;

	path.Append("api_key.txt");

	BFile file;
	if (file.SetTo(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE) == B_OK) {
		file.Write(apiKey.String(), apiKey.Length());
		file.Unset();
	}
}


BString
Config::LoadAPIKeyFromFile()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return BString();

	path.Append("AceOfWands/api_key.txt");

	BFile file;
	if (file.SetTo(path.Path(), B_READ_ONLY) != B_OK)
		return BString();

	off_t size;
	if (file.GetSize(&size) != B_OK) {
		file.Unset();
		return BString();
	}

	char* buffer = new char[size + 1];
	file.Read(buffer, size);
	buffer[size] = '\0';

	BString apiKey(buffer);
	delete[] buffer;
	file.Unset();

	return apiKey;
}


void
Config::SetSpread(SpreadType spread)
{
	sSpread = spread;
	// Removed file saving logic to use in-memory state only
}


SpreadType
Config::GetSpread()
{
	return sSpread;
}
