#pragma once

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

	status_t Initialize();
	void GetThreeCardSpread(std::vector<CardInfo>& cards);
	void SetThreeCardSpread(const std::vector<CardInfo>& cards);
	void ClearCurrentSpread();
	BString FormatCardName(const BString& resourceName);
	int32 GetResourceID(const BString& displayName);

private:
	std::vector<CardResourceInfo> fCardResources;
	std::vector<CardInfo> fCurrentSpread;
};
