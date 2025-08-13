#pragma once

#include <vector>
#include <thread>
#include <future>
#include "CardModel.h"

class CardView;

class CardPresenter {
public:
						CardPresenter(CardModel* model, CardView* view);
						~CardPresenter();

		void			LoadThreeCardSpread();

private:
		CardModel*		fModel;
		CardView*		fView;
		std::future<void> fReadingFuture;
};
