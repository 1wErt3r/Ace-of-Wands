#include "CardPresenter.h"
#include "AIReading.h"
#include "CardView.h"
#include "Config.h"
#include "Reading.h"
#include <chrono>
#include <stdio.h>
#include <thread>


CardPresenter::CardPresenter(CardModel* model, CardView* view)
	:
	fModel(model),
	fView(view),
	fReading(nullptr),
	fCurrentReading("")
{
}


CardPresenter::~CardPresenter()
{
	// Wait for any ongoing reading generation to complete
	if (fReadingFuture.valid())
		fReadingFuture.wait();
	delete fReading;
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
	fView->DisplayReading("Fetching reading...");

	// Launch asynchronous task to get the reading
	fReadingFuture = std::async(std::launch::async, [this, cards]() {
		// Add a small delay to simulate network request
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		BString reading;
		if (Config::GetAPIKey().IsEmpty()) {
			std::vector<BString> cardNames;
			for (const auto& card : cards)
				cardNames.push_back(card.displayName);
			fReading = new Reading(cardNames);
			reading = fReading->GetInterpretation();
		} else {
			// Get an AI reading for the cards
			reading = AIReading::GetReading(cards);
		}

		fCurrentReading = reading;
		printf("Reading: %s\n", reading.String());

		// Update the UI with the reading in a thread-safe manner
		fView->UpdateReading(reading);
	});
}
