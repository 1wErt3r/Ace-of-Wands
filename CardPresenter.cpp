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
	fCurrentReading(""),
	fSpread(THREE_CARD)
{
	fModel->Initialize();
	fSpread = Config::LoadSpreadFromFile();
	fView->SetSpread(fSpread);
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
CardPresenter::SetSpread(const BString& spreadName)
{
	if (spreadName == "Three Card")
		fSpread = THREE_CARD;
	else if (spreadName == "Tree of Life")
		fSpread = TREE_OF_LIFE;
	fView->SetSpread(fSpread);
}


void
CardPresenter::NewReading()
{
	fModel->ClearCurrentSpread();
	if (fSpread == THREE_CARD)
		LoadThreeCardSpread();
	else if (fSpread == TREE_OF_LIFE)
		LoadTreeOfLifeSpread();
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
	fModel->GetCardSpread(cards,
		fSpread == THREE_CARD ? Config::kThreeCardSpreadCount : Config::kTreeOfLifeSpreadCount);

	BString content = "Tarot Reading:\n\n";
	content << "Spread: " << (fSpread == THREE_CARD ? "Three Card" : "Tree of Life") << "\n\n";
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

	int32 spreadStart = content.FindFirst("Spread:");
	if (spreadStart != B_ERROR) {
		int32 spreadEnd = content.FindFirst("\n", spreadStart);
		BString spreadLine;
		content.CopyInto(spreadLine, spreadStart, spreadEnd - spreadStart);
		spreadLine.Remove(0, spreadLine.FindFirst(":") + 2);
		spreadLine.Trim();
		SetSpread(spreadLine);
	}

	int32 cardStart = content.FindFirst("Card 1:");
	if (cardStart != B_ERROR) {
		int numCards = (fSpread == THREE_CARD) ? Config::kThreeCardSpreadCount
											   : Config::kTreeOfLifeSpreadCount;
		for (int i = 0; i < numCards; ++i) {
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

	if (loadedCards.size()
		== (fSpread == THREE_CARD ? Config::kThreeCardSpreadCount
								  : Config::kTreeOfLifeSpreadCount)) {
		fModel->SetCardSpread(loadedCards);
		fView->DisplayCards(loadedCards);
		fView->DisplayReading(aiReadingText);
	} else {
		printf("Error: Could not parse cards from file.\n");
	}
}


void
CardPresenter::LoadThreeCardSpread()
{
	printf("Loading three card spread\n");
	std::vector<CardInfo> cards;
	fModel->GetCardSpread(cards, Config::kThreeCardSpreadCount);
	printf("Got %d cards from model\n", (int)cards.size());
	fView->DisplayCards(cards);

	// Show loading message while fetching AI reading
	fView->DisplayReading("Fetching reading...");

	// Launch asynchronous task to get the reading
	fReadingFuture = std::async(std::launch::async, [this, cards]() {
		// Add a small delay to simulate network request
		std::this_thread::sleep_for(std::chrono::milliseconds(Config::kAPITimeout));

		BString reading;
		if (Config::GetAPIKey().IsEmpty()) {
			std::vector<BString> cardNames;
			for (const auto& card : cards)
				cardNames.push_back(card.displayName);
			fReading = new Reading(cardNames);
			reading = fReading->GetInterpretation();
		} else {
			// Get an AI reading for the cards
			reading = AIReading::GetReading(cards, fSpread);
		}

		fCurrentReading = reading;
		printf("Reading: %s\n", reading.String());

		// Update the UI with the reading in a thread-safe manner
		fView->UpdateReading(reading);
	});
}


void
CardPresenter::LoadTreeOfLifeSpread()
{
	printf("Loading Tree of Life spread\n");
	std::vector<CardInfo> cards;
	fModel->GetCardSpread(cards, Config::kTreeOfLifeSpreadCount);
	printf("Got %d cards from model\n", (int)cards.size());
	fView->DisplayCards(cards);

	// Show loading message while fetching AI reading
	fView->DisplayReading("Fetching reading...");

	// Launch asynchronous task to get the reading
	fReadingFuture = std::async(std::launch::async, [this, cards]() {
		// Add a small delay to simulate network request
		std::this_thread::sleep_for(std::chrono::milliseconds(Config::kAPITimeout));

		BString reading;
		if (Config::GetAPIKey().IsEmpty()) {
			std::vector<BString> cardNames;
			for (const auto& card : cards)
				cardNames.push_back(card.displayName);
			fReading = new Reading(cardNames);
			reading = fReading->GetInterpretation();
		} else {
			// Get an AI reading for the cards
			reading = AIReading::GetReading(cards, fSpread);
		}

		fCurrentReading = reading;
		printf("Reading: %s\n", reading.String());

		// Update the UI with the reading in a thread-safe manner
		fView->UpdateReading(reading);
	});
}
