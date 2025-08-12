/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "App.h"
#include "MainWindow.h"

#include <Alert.h>


const char* kApplicationSignature = "application/x-vnd.MyName-MyApp";


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
	fWindow = new MainWindow();
	fWindow->Show();
}


void
App::AboutRequested()
{
	BAlert* aboutAlert = new BAlert("About Tarot Reader", 
		"Tarot Card Reader\n\n"
		"A beautiful tarot card reading application for Haiku.\n\n"
		"Version 1.0\n"
		"Copyright 2024", 
		"OK");
	aboutAlert->Go();
}



int
main()
{
	App* app = new App();
	app->Run();
	delete app;
	return 0;
}
