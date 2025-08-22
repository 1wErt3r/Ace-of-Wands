#include "MainWindow.h"
#include "CardPresenter.h"
#include "Config.h"
#include "SettingsWindow.h"

#include <Application.h>
#include <Button.h>
#include <FilePanel.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <ScrollView.h>
#include <String.h>
#include <View.h>
#include <iostream>


MainWindow::MainWindow(CardPresenter* presenter)
	:
	BWindow(BRect(Config::kMainWindowLeft, Config::kMainWindowTop, Config::kMainWindowRight,
				Config::kMainWindowBottom),
		"Ace of Wands", B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE),
	fMenuBar(NULL),
	fCardPresenter(presenter), // Injected presenter
	fOpenFilePanel(NULL),
	fSaveFilePanel(NULL)
{
	// Create menu bar
	_CreateMenuBar();

	// Create file panels
	fOpenFilePanel
		= new BFilePanel(B_OPEN_PANEL, new BMessenger(this), NULL, B_FILE_NODE, false, NULL);
	fSaveFilePanel
		= new BFilePanel(B_SAVE_PANEL, new BMessenger(this), NULL, B_FILE_NODE, false, NULL);

	// Create a scroll view for the card view
	BScrollView* scrollView = new BScrollView("CardScrollView", presenter->GetView(),
		B_FOLLOW_ALL_SIDES, false, true, B_FANCY_BORDER);

	// Use layout builder to arrange menu bar and scroll view
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0).Add(fMenuBar).Add(scrollView);
}


MainWindow::~MainWindow()
{
	delete fCardPresenter;
	fCardPresenter = NULL;

	delete fOpenFilePanel;
	delete fSaveFilePanel;
}


void
MainWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kMsgNewReading:
			if (fCardPresenter)
				fCardPresenter->NewReading();
			break;
		case kMsgOpen:
			if (fOpenFilePanel != NULL)
				fOpenFilePanel->Show();
			break;
		case kMsgSave:
			if (fSaveFilePanel != NULL)
				fSaveFilePanel->Show();
			break;
		case kMsgSettings:
		{
			SettingsWindow* settingsWindow = new SettingsWindow(this);
			settingsWindow->Show();
			break;
		}
		case kMsgAPIKeyReceived:
		{
			// Handle API key received from settings window
			const char* apiKey;
			if (message->FindString("apiKey", &apiKey) == B_OK) {
				if (fCardPresenter)
					fCardPresenter->SetAPIKey(BString(apiKey));
				std::cout << "API key has been set and stored." << std::endl;
			}
			break;
		}
		case kMsgSpreadChanged:
		{
			const char* spreadName;
			if (message->FindString("spread", &spreadName) == B_OK) {
				if (fCardPresenter)
					fCardPresenter->SetSpread(BString(spreadName));
			}
			break;
		}
		case kMsgFontSizeChanged:
		{
			const char* fontSizeStr;
			if (message->FindString("fontSize", &fontSizeStr) == B_OK) {
				float fontSize = atof(fontSizeStr);
				if (fCardPresenter)
					fCardPresenter->SetFontSize(fontSize);
			}
			break;
		}
		case B_SAVE_REQUESTED:
		{
			entry_ref directoryRef;
			const char* name;
			message->FindRef("directory", &directoryRef);
			message->FindString("name", &name);

			BPath path(&directoryRef);
			path.Append(name);
			std::cout << "Save requested to: " << path.Path() << std::endl;
			if (fCardPresenter)
				fCardPresenter->SaveFile(path);
			break;
		}
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			message->FindRef("refs", 0, &ref);

			BPath path(&ref);
			std::cout << "Open requested for: " << path.Path() << std::endl;
			if (fCardPresenter)
				fCardPresenter->OpenFile(path);
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

	if (fCardPresenter)
		fCardPresenter->OnFrameResized();
}


bool
MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
MainWindow::_CreateMenuBar()
{
	fMenuBar = new BMenuBar("MenuBar");

	// Create Ace of Wands menu
	BMenu* appMenu = new BMenu("Ace of Wands");
	appMenu->AddItem(new BMenuItem("New Reading", new BMessage(kMsgNewReading), 'N'));
	appMenu->AddItem(new BMenuItem("Settings...", new BMessage(kMsgSettings), 'P'));

	appMenu->AddSeparatorItem();

	appMenu->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q'));

	fMenuBar->AddItem(appMenu);

	// Create File menu
	BMenu* fileMenu = new BMenu("File");
	fileMenu->AddItem(new BMenuItem("Open...", new BMessage(kMsgOpen), 'O'));
	fileMenu->AddItem(new BMenuItem("Save...", new BMessage(kMsgSave), 'S'));

	fMenuBar->AddItem(fileMenu);
}
