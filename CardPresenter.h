#pragma once

#include "CardModel.h"
#include "Reading.h"
#include <Path.h>
#include <String.h>
#include <future>
#include <thread>
#include <vector>

class CardModel;
class CardView;
class BView;

enum SpreadType { THREE_CARD, TREE_OF_LIFE };

class CardPresenter {
public:
	// Constructor now takes dependencies
	CardPresenter(CardModel* model, CardView* view);
	~CardPresenter();

	void NewReading();
	void OnFrameResized();
	void SaveFile(const BPath& path);
	void OpenFile(const BPath& path);
	BString GetCurrentReading() const { return fCurrentReading; }
	BView* GetView();
	void SetView(CardView* view); // New method to set the view
	BString GetAPIKey();
	void SetAPIKey(const BString& apiKey);
	void SetSpread(const BString& spreadName);
	void SetFontSize(float fontSize);

private:
	void LoadThreeCardSpread();
	void LoadTreeOfLifeSpread();
	void SaveReadingToFile(const std::vector<CardInfo>& cards, const BString& reading);

	CardModel* fModel;
	CardView* fView;
	std::future<void> fReadingFuture;
	Reading* fReading;
	BString fCurrentReading;
	SpreadType fSpread;
};
