#include "App.h"
#include "Config.h"
#include "MainWindow.h"
#include <Alert.h>
#include <cstdio>


const char* kApplicationSignature = "application/x-vnd.Ace-of-Wands";


App::App()
	:
	BApplication(kApplicationSignature),
	fWindow(NULL)
{
}


App::~App()
{
}


void
App::ReadyToRun()
{
	// Load the settings from file
	Config::LoadSettingsFromFile();

	// Load the API key from file if it exists
	BString savedApiKey = Config::LoadAPIKeyFromFile();
	printf("Loaded API key: %s\n", savedApiKey.String());
	if (!savedApiKey.IsEmpty())
		Config::SetAPIKey(savedApiKey);

	fWindow = new MainWindow();
	fWindow->Show();
}


int
main()
{
	App* app = new App();
	app->Run();
	delete app;
	return 0;
}
