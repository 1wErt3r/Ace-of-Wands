#include <TranslationDefs.h>

#include "CardModel.h"

#include <Application.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>
#include <Resources.h>
#include <String.h>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <iostream>


CardModel::CardModel()
{
	// Initialize random seed
	srand(time(NULL));
}


CardModel::~CardModel()
{
}


status_t
CardModel::Initialize()
{
	BResources* appResources = BApplication::AppResources();
	if (appResources == NULL) {
		std::cout << "Error: appResources is NULL" << std::endl;
		return B_ERROR;
	}

	int32 count = 0;
	type_code type;
	int32 id;
	const char* name;
	size_t size;

	while (appResources->GetResourceInfo(count, &type, &id, &name, &size)) {
		if (type == 'BBMP' && BString(name).IFindLast(".webp") != B_ERROR)
			fCardResources.push_back({id, BString(name)});
		count++;
	}

	return fCardResources.size() > 0 ? B_OK : B_ERROR;
}


void
CardModel::GetCardSpread(std::vector<CardInfo>& cards, int32 numCards)
{
	if (!fCurrentSpread.empty()) {
		cards = fCurrentSpread;
		return;
	}

	cards.clear();

	if (fCardResources.size() < static_cast<size_t>(numCards))
		return;

	// Select random cards
	std::vector<int> selectedIndices;
	while (selectedIndices.size() < static_cast<size_t>(numCards)) {
		int index = rand() % fCardResources.size();

		// Check if already selected
		bool alreadySelected = false;
		for (size_t i = 0; i < selectedIndices.size(); i++) {
			if (selectedIndices[i] == index) {
				alreadySelected = true;
				break;
			}
		}

		if (!alreadySelected)
			selectedIndices.push_back(index);
	}

	// Create card info for each selected card
	for (int i = 0; i < numCards; i++) {
		CardInfo info;
		info.resourceID = fCardResources[selectedIndices[i]].id;
		info.displayName = FormatCardName(fCardResources[selectedIndices[i]].name);
		cards.push_back(info);
	}
	fCurrentSpread = cards;
}


BString
CardModel::FormatCardName(const BString& resourceName)
{
	// Remove file extension
	BString name = resourceName;
	int32 extIndex = name.IFindLast(".webp");
	if (extIndex != B_ERROR)
		name.Truncate(extIndex);

	// Replace underscores with spaces
	name.ReplaceAll("_", " ");

	// Capitalize first letter of each word
	BString newName = "";
	for (int32 i = 0; i < name.Length(); i++) {
		if (i == 0 || name[i - 1] == ' ')
			newName += toupper(name[i]);
		else
			newName += name[i];
	}
	name = newName;

	// Special handling for numbered cards
	if (isdigit(name[0]) && name.FindFirst(" ") > 0) {
		int32 spaceIndex = name.FindFirst(" ");
		BString number = name;
		number.Truncate(spaceIndex);

		BString rest = name;
		rest.Remove(0, spaceIndex + 1);

		// Convert number words
		BString numberWord;
		if (number == "01")
			numberWord = "1";
		else if (number == "02")
			numberWord = "2";
		else if (number == "03")
			numberWord = "3";
		else if (number == "04")
			numberWord = "4";
		else if (number == "05")
			numberWord = "5";
		else if (number == "06")
			numberWord = "6";
		else if (number == "07")
			numberWord = "7";
		else if (number == "08")
			numberWord = "8";
		else if (number == "09")
			numberWord = "9";
		else if (number == "10")
			numberWord = "10";
		else if (number == "11")
			numberWord = "11";
		else if (number == "12")
			numberWord = "12";
		else if (number == "13")
			numberWord = "13";
		else if (number == "14")
			numberWord = "14";
		else if (number == "15")
			numberWord = "15";
		else if (number == "16")
			numberWord = "16";
		else if (number == "17")
			numberWord = "17";
		else if (number == "18")
			numberWord = "18";
		else if (number == "19")
			numberWord = "19";
		else if (number == "20")
			numberWord = "20";
		else if (number == "21")
			numberWord = "21";
		else
			numberWord = number;

		name = numberWord;
		name << " " << rest;
	}

	return name;
}


void
CardModel::SetCardSpread(const std::vector<CardInfo>& cards)
{
	fCurrentSpread = cards;
}


void
CardModel::ClearCurrentSpread()
{
	fCurrentSpread.clear();
}


int32
CardModel::GetResourceID(const BString& displayName)
{
	for (size_t i = 0; i < fCardResources.size(); ++i) {
		BString formattedName = FormatCardName(fCardResources[i].name);
		if (formattedName == displayName)
			return fCardResources[i].id;
	}
	return -1; // Not found
}
