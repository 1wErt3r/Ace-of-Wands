/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "CardModel.h"

#include <Application.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>
#include <String.h>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <cstdio>

CardModel::CardModel()
{
	// Initialize random seed
	srand(time(NULL));
}


CardModel::~CardModel()
{
}


status_t
CardModel::Initialize(const char* cardsDirectory)
{
	fCardsDirectory.SetTo(cardsDirectory);
	ScanCardsDirectory();
	
	printf("Found %d card files in directory: %s\n", (int)fCardFiles.size(), cardsDirectory);
	
	return fCardFiles.size() > 0 ? B_OK : B_ERROR;
}


void
CardModel::ScanCardsDirectory()
{
	BDirectory dir(fCardsDirectory.Path());
	if (dir.InitCheck() != B_OK)
		return;
		
	entry_ref ref;
	while (dir.GetNextRef(&ref) == B_OK) {
		BEntry entry(&ref, true);
		if (entry.IsFile()) {
			BPath path(&entry);
			BString fileName = path.Leaf();
			
			// Check if it's a JPG file
			if (fileName.IFindLast(".jpg") != B_ERROR) {
				fCardFiles.push_back(fileName);
			}
		}
	}
}


void
CardModel::GetThreeCardSpread(std::vector<CardInfo>& cards)
{
	cards.clear();
	
	if (fCardFiles.size() < 3)
		return;
	
	// Select 3 random cards
	std::vector<int> selectedIndices;
	while (selectedIndices.size() < 3) {
		int index = rand() % fCardFiles.size();
		
		// Check if already selected
		bool alreadySelected = false;
		for (int i = 0; i < selectedIndices.size(); i++) {
			if (selectedIndices[i] == index) {
				alreadySelected = true;
				break;
			}
		}
		
		if (!alreadySelected) {
			selectedIndices.push_back(index);
		}
	}
	
	// Create card info for each selected card
	for (int i = 0; i < 3; i++) {
		CardInfo info;
		info.filePath = fCardsDirectory.Path();
		info.filePath << "/" << fCardFiles[selectedIndices[i]];
		info.displayName = FormatCardName(fCardFiles[selectedIndices[i]]);
		cards.push_back(info);
	}
}


BString
CardModel::FormatCardName(const BString& fileName)
{
	// Remove file extension
	BString name = fileName;
	int32 extIndex = name.IFindLast(".jpg");
	if (extIndex != B_ERROR) {
		name.Truncate(extIndex);
	}
	
	// Replace underscores with spaces
	name.ReplaceAll("_", " ");
	
	// Capitalize first letter of each word
	BString newName = "";
	for (int32 i = 0; i < name.Length(); i++) {
		if (i == 0 || name[i-1] == ' ') {
			newName += toupper(name[i]);
		} else {
			newName += name[i];
		}
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
		if (number == "01") numberWord = "1";
		else if (number == "02") numberWord = "2";
		else if (number == "03") numberWord = "3";
		else if (number == "04") numberWord = "4";
		else if (number == "05") numberWord = "5";
		else if (number == "06") numberWord = "6";
		else if (number == "07") numberWord = "7";
		else if (number == "08") numberWord = "8";
		else if (number == "09") numberWord = "9";
		else if (number == "10") numberWord = "10";
		else if (number == "11") numberWord = "11";
		else if (number == "12") numberWord = "12";
		else if (number == "13") numberWord = "13";
		else if (number == "14") numberWord = "14";
		else if (number == "15") numberWord = "15";
		else if (number == "16") numberWord = "16";
		else if (number == "17") numberWord = "17";
		else if (number == "18") numberWord = "18";
		else if (number == "19") numberWord = "19";
		else if (number == "20") numberWord = "20";
		else if (number == "21") numberWord = "21";
		else numberWord = number;
		
		name = numberWord;
		name << " " << rest;
	}
	
	return name;
}