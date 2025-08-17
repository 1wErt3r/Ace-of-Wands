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
	SaveSpreadToFile(spread);
}


SpreadType
Config::GetSpread()
{
	return sSpread;
}


void
Config::SaveSpreadToFile(SpreadType spread)
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return;

	path.Append("AceOfWands");

	// Create the directory if it doesn't exist
	BDirectory dir;
	if (dir.CreateDirectory(path.Path(), &dir) != B_OK && dir.SetTo(path.Path()) != B_OK)
		return;

	path.Append("spread.txt");

	BFile file;
	if (file.SetTo(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE) == B_OK) {
		int32 value = static_cast<int32>(spread);
		file.Write(&value, sizeof(value));
		file.Unset();
	}
}


SpreadType
Config::LoadSpreadFromFile()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return THREE_CARD;

	path.Append("AceOfWands/spread.txt");

	BFile file;
	if (file.SetTo(path.Path(), B_READ_ONLY) != B_OK)
		return THREE_CARD;

	int32 value;
	if (file.Read(&value, sizeof(value)) == sizeof(value)) {
		file.Unset();
		return static_cast<SpreadType>(value);
	}

	file.Unset();
	return THREE_CARD;
}
