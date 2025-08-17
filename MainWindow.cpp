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
#include <String.h>
#include <View.h>
#include <cstdio>


MainWindow::MainWindow()
	:
	BWindow(BRect(100, 100, 900, 700), "Ace of Wands", B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE),
	fMenuBar(NULL),
	fCardPresenter(NULL),
	fOpenFilePanel(NULL),
	fSaveFilePanel(NULL)
{
	// Create menu bar
	_CreateMenuBar();

	// Create the presenter
	fCardPresenter = new CardPresenter();

	// Create file panels
	fOpenFilePanel
		= new BFilePanel(B_OPEN_PANEL, new BMessenger(this), NULL, B_FILE_NODE, false, NULL);
	fSaveFilePanel
		= new BFilePanel(B_SAVE_PANEL, new BMessenger(this), NULL, B_FILE_NODE, false, NULL);

	printf("B_SAVE_REQUESTED: %lu\n", B_SAVE_REQUESTED);

	// Use layout builder to arrange menu bar and card view
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0).Add(fMenuBar).Add(fCardPresenter->GetView());
}


MainWindow::~MainWindow()
{
	// Clean up in proper order
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
				printf("API key has been set and stored.\n");
			}
			break;
		}
		case kMsgSpreadChanged:
		{
			BString spreadName;
			if (message->FindString("spread", &spreadName) == B_OK) {
				if (fCardPresenter)
					fCardPresenter->SetSpread(spreadName);
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
			printf("Save requested to: %s\n", path.Path());
			if (fCardPresenter)
				fCardPresenter->SaveFile(path);
			break;
		}
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			message->FindRef("refs", 0, &ref);

			BPath path(&ref);
			printf("Open requested for: %s\n", path.Path());
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
