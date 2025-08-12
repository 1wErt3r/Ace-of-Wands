/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "CardPresenter.h"
#include "CardView.h"
#include "AIReading.h"
#include <stdio.h>

CardPresenter::CardPresenter(CardModel* model, CardView* view)
	:
	fModel(model),
	fView(view)
{
}


CardPresenter::~CardPresenter()
{
}


void
CardPresenter::LoadThreeCardSpread()
{
	printf("Loading three card spread\n");
	std::vector<CardInfo> cards;
	fModel->GetThreeCardSpread(cards);
	printf("Got %d cards from model\n", (int)cards.size());
	fView->DisplayCards(cards);
	
	// Show loading message while fetching AI reading
	fView->DisplayReading("Fetching AI reading...");
	
	// Get an AI reading for the cards
	BString reading = AIReading::GetReading(cards);
	printf("AI Reading: %s\n", reading.String());
	fView->DisplayReading(reading);
}