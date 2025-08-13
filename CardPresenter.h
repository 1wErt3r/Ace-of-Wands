#pragma once

#include "CardModel.h"
#include <future>
#include <thread>
#include <vector>

class CardView;

class CardPresenter {
public:
	CardPresenter(CardModel* model, CardView* view);
	~CardPresenter();

	void LoadThreeCardSpread();

private:
	CardModel* fModel;
	CardView* fView;
	std::future<void> fReadingFuture;
};
