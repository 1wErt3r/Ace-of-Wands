#include "CardPresenter.h"
#include "CardView.h"
#include "AIReading.h"
#include <stdio.h>
#include <thread>
#include <chrono>

CardPresenter::CardPresenter(CardModel* model, CardView* view)
	:
	fModel(model),
	fView(view)
{
}


CardPresenter::~CardPresenter()
{
	// Wait for any ongoing reading generation to complete
	if (fReadingFuture.valid()) {
		fReadingFuture.wait();
	}
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
	
	// Launch asynchronous task to get the AI reading
	fReadingFuture = std::async(std::launch::async, [this, cards]() {
		// Add a small delay to simulate network request
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		
		// Get an AI reading for the cards
		BString reading = AIReading::GetReading(cards);
		printf("AI Reading: %s\n", reading.String());
		
		// Update the UI with the reading in a thread-safe manner
		fView->UpdateReading(reading);
	});
}