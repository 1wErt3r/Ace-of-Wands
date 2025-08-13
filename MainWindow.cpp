#include "MainWindow.h"
#include "CardModel.h"
#include "CardPresenter.h"
#include "CardView.h"

#include "AIReading.h"

#include <Application.h>
#include <Button.h>
#include <FilePanel.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <String.h>
#include <View.h>
#include <stdio.h>

const uint32 kAppMessageBase = 'AOW_';
const uint32 kMsgNewReading = kAppMessageBase + 1;
const uint32 kMsgSave = kAppMessageBase + 2;
const uint32 kMsgOpen = kAppMessageBase + 3;


MainWindow::MainWindow()
	:
	BWindow(BRect(100, 100, 900, 700), "Ace of Wands", B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE),
	fMenuBar(NULL),
	fCardModel(NULL),
	fCardView(NULL),
	fCardPresenter(NULL),
	fOpenFilePanel(NULL),
	fSaveFilePanel(NULL)
{
	// Create menu bar
	_CreateMenuBar();

	// Create the card model
	fCardModel = new CardModel();

	if (fCardModel->Initialize() != B_OK)
		printf("Failed to initialize card model\n");

	// Create the card view
	fCardView = new CardView(Bounds());

	// Set resizing mode to follow window size changes
	fCardView->SetResizingMode(B_FOLLOW_ALL_SIDES);

	// Create the presenter
	fCardPresenter = new CardPresenter(fCardModel, fCardView);

	// Create file panels
	fOpenFilePanel
		= new BFilePanel(B_OPEN_PANEL, new BMessenger(this), NULL, B_FILE_NODE, false, NULL);
	fSaveFilePanel
		= new BFilePanel(B_SAVE_PANEL, new BMessenger(this), NULL, B_FILE_NODE, false, NULL);

	printf("B_SAVE_REQUESTED: %lu\n", B_SAVE_REQUESTED);

	// Use layout builder to arrange menu bar and card view
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0).Add(fMenuBar).Add(fCardView);

	// Load initial spread
	LoadSpread();
}


MainWindow::~MainWindow()
{
	// Clean up in proper order
	delete fCardPresenter;
	fCardPresenter = NULL;

	// Remove the card view from the window before deleting it
	if (fCardView) {
		fCardView->RemoveSelf();
		delete fCardView;
		fCardView = NULL;
	}

	delete fCardModel;
	fCardModel = NULL;

	delete fOpenFilePanel;
	delete fSaveFilePanel;
}


void
MainWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kMsgNewReading:
			LoadSpread();
			break;
		case kMsgOpen:
			if (fOpenFilePanel != NULL)
				fOpenFilePanel->Show();
			break;
		case kMsgSave:
			if (fSaveFilePanel != NULL)
				fSaveFilePanel->Show();
			break;
		case B_SAVE_REQUESTED:
		{
			entry_ref directoryRef;
			const char* name;
			message->FindRef("directory", &directoryRef);
			message->FindString("name", &name);

			BPath path(&directoryRef);
			path.Append(name);
			printf("Save requested to: %s\n", path.Path());
			_SaveFile(path);
			break;
		}
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			message->FindRef("refs", 0, &ref);

			BPath path(&ref);
			printf("Open requested for: %s\n", path.Path());
			_OpenFile(path);
			break;
		}


		default:
		{
			BWindow::MessageReceived(message);
			break;
		}
	}
}


void
MainWindow::FrameResized(float width, float height)
{
	BWindow::FrameResized(width, height);

	// Explicitly tell the card view to relayout
	if (fCardView)
		fCardView->RefreshLayout();
}


bool
MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
MainWindow::LoadSpread()
{
	if (fCardPresenter) {
		fCardModel->ClearCurrentSpread();
		fCardPresenter->LoadThreeCardSpread();
	}
}


void
MainWindow::_CreateMenuBar()
{
	fMenuBar = new BMenuBar("MenuBar");

	// Create File menu
	BMenu* fileMenu = new BMenu("File");
	fileMenu->AddItem(new BMenuItem("New Reading", new BMessage(kMsgNewReading), 'N'));
	fileMenu->AddSeparatorItem();
	fileMenu->AddItem(new BMenuItem("Open...", new BMessage(kMsgOpen), 'O'));
	fileMenu->AddItem(new BMenuItem("Save...", new BMessage(kMsgSave), 'S'));
	fileMenu->AddSeparatorItem();
	fileMenu->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q'));

	fMenuBar->AddItem(fileMenu);
}


void
MainWindow::_SaveFile(BPath path)
{
	BFile file;
	status_t status = file.SetTo(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	if (status != B_OK) {
		printf("Error creating file: %s\n", strerror(status));
		return;
	}

	std::vector<CardInfo> cards;
	fCardModel->GetThreeCardSpread(cards);

	BString content = "Tarot Reading:\n\n";
	for (size_t i = 0; i < cards.size(); ++i)
		content << "Card " << (i + 1) << ": " << cards[i].displayName << "\n";
	content << "\nAI Reading:\n" << AIReading::GetReading(cards) << "\n";

	file.Write(content.String(), content.Length());
	file.Unset();
	printf("File saved successfully to: %s\n", path.Path());
}


void
MainWindow::_OpenFile(BPath path)
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
				info.resourceID = fCardModel->GetResourceID(cardLine);
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
		fCardModel->SetThreeCardSpread(loadedCards);
		fCardView->DisplayCards(loadedCards);
		fCardView->DisplayReading(aiReadingText);
	} else {
		printf("Error: Could not parse 3 cards from file.\n");
	}
}
