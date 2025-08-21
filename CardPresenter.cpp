#include "CardPresenter.h"
#include "AIReading.h"
#include "CardModel.h"
#include "CardView.h"
#include "Config.h"
#include "Reading.h"
#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Node.h>
#include <Path.h>
#include <View.h>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <thread>


CardPresenter::CardPresenter(CardModel* model, CardView* view)
	:
	fModel(model),
	fView(view),
	fReading(nullptr),
	fCurrentReading(""),
	fSpread(Config::GetSpread())
{
	if (fModel)
		fModel->Initialize();
	if (fView)
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


void
CardPresenter::SetView(CardView* view)
{
	fView = view;
	if (fView)
		fView->SetSpread(fSpread);
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
	SpreadType newSpread;
	if (spreadName == "Three Card")
		newSpread = THREE_CARD;
	else if (spreadName == "Tree of Life")
		newSpread = TREE_OF_LIFE;
	else
		return;

	fSpread = newSpread;
	Config::SetSpread(newSpread);
	fView->SetSpread(newSpread);
}


void
CardPresenter::SetFontSize(float fontSize)
{
	Config::SetFontSize(fontSize);
	fView->SetFontSize(fontSize);
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

	// Register the file with our application's MIME type
	Config::RegisterFileWithMime(path.Path(), "application/x-vnd.Ace-of-Wands");

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
	BString spreadLine; // Moved declaration here

	int32 spreadStart = content.FindFirst("Spread:");
	if (spreadStart != B_ERROR) {
		int32 spreadEnd = content.FindFirst("\n", spreadStart);
		content.CopyInto(spreadLine, spreadStart, spreadEnd - spreadStart);
		spreadLine.Remove(0, spreadLine.FindFirst(":") + 2);
		spreadLine.Trim();
		SetSpread(spreadLine);
	} else {
		// If "Spread:" is not found, assume it's a Three Card spread for backward compatibility
		spreadLine = "Three Card";
		SetSpread(spreadLine);
	}

	int32 cardStart = content.FindFirst("Card 1:");
	if (cardStart != B_ERROR) {
		int numCards = 0;
		if (spreadLine == "Three Card")
			numCards = Config::kThreeCardSpreadCount;
		else if (spreadLine == "Tree of Life")
			numCards = Config::kTreeOfLifeSpreadCount;

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

	int32 expectedCardCount = 0;
	if (spreadLine == "Three Card")
		expectedCardCount = Config::kThreeCardSpreadCount;
	else if (spreadLine == "Tree of Life")
		expectedCardCount = Config::kTreeOfLifeSpreadCount;

	if (loadedCards.size() == expectedCardCount) {
		fModel->SetCardSpread(loadedCards);
		fView->DisplayCards(loadedCards);
		fView->DisplayReading(aiReadingText);
	} else {
		printf("Error: Could not parse cards from file. Expected %d cards, found %d.\n",
			expectedCardCount, (int)loadedCards.size());
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

		// Log the reading if enabled
		if (Config::GetLogReadings())
			SaveReadingToFile(cards, reading);

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

		// Log the reading if enabled
		if (Config::GetLogReadings())
			SaveReadingToFile(cards, reading);

		// Update the UI with the reading in a thread-safe manner
		fView->UpdateReading(reading);
	});
}


void
CardPresenter::SaveReadingToFile(const std::vector<CardInfo>& cards, const BString& reading)
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return;

	path.Append("AceOfWands/readings");

	// Create the directory if it doesn't exist
	BDirectory dir;
	if (dir.CreateDirectory(path.Path(), &dir) != B_OK && dir.SetTo(path.Path()) != B_OK)
		return;

	// Generate a filename based on timestamp
	time_t now = time(NULL);
	struct tm* tm_now = localtime(&now);
	char filename[256];
	strftime(filename, sizeof(filename), "reading_%Y-%m-%d_%H-%M-%S.txt", tm_now);

	path.Append(filename);

	BFile file;
	status_t status = file.SetTo(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	if (status != B_OK) {
		printf("Error creating reading file: %s\n", strerror(status));
		return;
	}

	BString content = "Tarot Reading\n";
	content << "Date: " << ctime(&now); // ctime includes newline
	content << "Spread: " << (fSpread == THREE_CARD ? "Three Card" : "Tree of Life") << "\n\n";

	for (size_t i = 0; i < cards.size(); ++i)
		content << "Card " << (i + 1) << ": " << cards[i].displayName << "\n";

	content << "\nAI Reading:\n" << reading << "\n";

	file.Write(content.String(), content.Length());
	file.Unset();

	// Register the file with MIME type
	BNode node(path.Path());
	if (node.InitCheck() == B_OK) {
		const char* mimeType = "application/x-vnd.Ace-of-Wands";
		node.WriteAttr("BEOS:TYPE", B_STRING_TYPE, 0, mimeType, strlen(mimeType) + 1);
	}

	printf("Reading saved to: %s\n", path.Path());
}
