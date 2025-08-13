#pragma once

#include "CardModel.h"
#include "Reading.h"
#include <future>
#include <thread>
#include <vector>

class CardView;

class CardPresenter {
public:
	CardPresenter(CardModel* model, CardView* view);
	~CardPresenter();

	void LoadThreeCardSpread();
	BString GetCurrentReading() const { return fCurrentReading; }

private:
	CardModel* fModel;
	CardView* fView;
	std::future<void> fReadingFuture;
	Reading* fReading;
	BString fCurrentReading;
};
