/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CARDPRESENTER_H
#define CARDPRESENTER_H

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

#endif // CARDPRESENTER_H