/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CARDMODEL_H
#define CARDMODEL_H

#include <Directory.h>
#include <Entry.h>
#include <Path.h>
#include <String.h>
#include <vector>

struct CardInfo {
	BString filePath;
	BString displayName;
};

class CardModel {
public:
							CardModel();
							~CardModel();

			status_t		Initialize(const char* cardsDirectory);
			void			GetThreeCardSpread(std::vector<CardInfo>& cards);
			BString			FormatCardName(const BString& fileName);

private:
			void			ScanCardsDirectory();
			
			BPath			fCardsDirectory;
			std::vector<BString> fCardFiles;
};

#endif // CARDMODEL_H