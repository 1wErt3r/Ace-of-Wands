#include "CardPresenter.h"
#include "AIReading.h"
#include "CardModel.h"
#include "CardView.h"
#include "Config.h"
#include "Reading.h"
#include <File.h>
#include <View.h>
#include <chrono>
#include <cstdio>
#include <thread>


CardPresenter::CardPresenter()
	:
	fModel(new CardModel()),
	fView(new CardView(BRect(0, 0, 0, 0))),
	fReading(nullptr),
	fCurrentReading("")
{
	fModel->Initialize();
}


CardPresenter::~CardPresenter()
{
	// Wait for any ongoing reading generation to complete
	if (fReadingFuture.valid())
		fReadingFuture.wait();
	delete fReading;
	delete fModel;
}


BView*
CardPresenter::GetView()
{
	return fView;
}


BString
CardPresenter::GetAPIKey()
{
	return Config::GetAPIKey();
}


void
CardPresenter::SetAPIKey(const BString& apiKey)
{
	Config::SetAPIKey(apiKey);
}


void
CardPresenter::NewReading()
{
	fModel->ClearCurrentSpread();
	LoadThreeCardSpread();
}


void
CardPresenter::OnFrameResized()
{
	fView->RefreshLayout();
}


void
CardPresenter::SaveFile(const BPath& path)
{
	BFile file;
	status_t status = file.SetTo(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	if (status != B_OK) {
		printf("Error creating file: %s\n", strerror(status));
		return;
	}

	std::vector<CardInfo> cards;
	fModel->GetThreeCardSpread(cards);

	BString content = "Tarot Reading:\n\n";
	for (size_t i = 0; i < cards.size(); ++i)
		content << "Card " << (i + 1) << ": " << cards[i].displayName << "\n";
	content << "\nAI Reading:\n" << GetCurrentReading() << "\n";

	file.Write(content.String(), content.Length());
	file.Unset();
	printf("File saved successfully to: %s\n", path.Path());
}


void
CardPresenter::OpenFile(const BPath& path)
{
	BFile file;
	status_t status = file.SetTo(path.Path(), B_READ_ONLY);
	if (status != B_OK) {
		printf("Error opening file: %s\n", strerror(status));
		return;
	}

	off_t size;
	file.GetSize(&size);
	char* buffer = new char[size + 1];
	file.Read(buffer, size);
	buffer[size] = '\0';

	BString content(buffer);
	delete[] buffer;
	file.Unset();

	std::vector<CardInfo> loadedCards;
	BString aiReadingText;

	int32 cardStart = content.FindFirst("Card 1:");
	if (cardStart != B_ERROR) {
		for (int i = 0; i < 3; ++i) {
			BString cardLine;
			int32 lineEnd = content.FindFirst("\n", cardStart);
			if (lineEnd != B_ERROR) {
				content.CopyInto(cardLine, cardStart, lineEnd - cardStart);
				cardLine.Remove(0, cardLine.FindFirst(":") + 2);
				cardLine.Trim();

				CardInfo info;
				info.displayName = cardLine;
				info.resourceID = fModel->GetResourceID(cardLine);
				loadedCards.push_back(info);
				cardStart = lineEnd + 1;
			} else {
				break;
			}
		}
	}

	int32 aiReadingStart = content.FindFirst("AI Reading:");
	if (aiReadingStart != B_ERROR) {
		aiReadingStart = content.FindFirst("\n", aiReadingStart) + 1;
		aiReadingText = content.String() + aiReadingStart;
		aiReadingText.Trim();
	}

	if (loadedCards.size() == 3) {
		fModel->SetThreeCardSpread(loadedCards);
		fView->DisplayCards(loadedCards);
		fView->DisplayReading(aiReadingText);
	} else {
		printf("Error: Could not parse 3 cards from file.\n");
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
