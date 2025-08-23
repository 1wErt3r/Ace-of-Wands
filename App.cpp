#include "App.h"
#include "CardModel.h"
#include "CardPresenter.h"
#include "CardView.h"
#include "Config.h"
#include "MainWindow.h"
#include <Application.h>
#include <Rect.h>
#include <iostream>


App::App()
	:
	BApplication("application/x-vnd.Ace-of-Wands"),
	fWindow(NULL)
{
}


App::~App()
{
	delete fWindow;
	fWindow = NULL;
}


void
App::ReadyToRun()
{
	// Load the settings from file
	Config::LoadSettingsFromFile();

	// Load the API key from file if it exists
	BString savedApiKey = Config::LoadAPIKeyFromFile();
	std::cout << "Loaded API key: " << savedApiKey.String() << std::endl;
	if (!savedApiKey.IsEmpty())
		Config::SetAPIKey(savedApiKey);

	CardModel* model = new CardModel();
	CardView* view = new CardView(BRect(0, 0, 0, 0));

	CardPresenter* presenter = new CardPresenter(model, view);

	fWindow = new MainWindow(presenter);
	fWindow->Show();
}


int
main()
{
	App* app = new App();
	app->Run();

	return 0;
}
