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
	int32 resourceID;
	BString displayName;
};

struct CardResourceInfo {
	int32 id;
	BString name;
};

class CardModel {
public:
							CardModel();
							~CardModel();

			status_t		Initialize();
			void			GetThreeCardSpread(std::vector<CardInfo>& cards);
			BString			FormatCardName(const BString& resourceName);

private:
			std::vector<CardResourceInfo> fCardResources;
};

#endif // CARDMODEL_H