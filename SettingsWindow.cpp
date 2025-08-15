#include "SettingsWindow.h"
#include "Config.h"
#include "MainWindow.h"

#include <Alert.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <StringView.h>
#include <TextControl.h>
#include <cstdio>


SettingsWindow::SettingsWindow(BWindow* owner)
	:
	BWindow(BRect(100, 100, 500, 300), "Settings", B_TITLED_WINDOW,
		B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS),
	fOwnerMessenger(owner)
{
	fInstructions = new BStringView("instructions", "Enter your DeepSeek API Key:");
	fAPIKeyInput = new BTextControl("apiKeyInput", "API Key:", "", NULL);
	fAPIKeyInput->TextView()->HideTyping(true); // Make it a password field

	// Populate the API key field when the window is created
	BString apiKey = Config::GetAPIKey();
	fAPIKeyInput->SetText(apiKey.String());

	fSaveButton = new BButton("saveButton", "Save", new BMessage(kMsgSaveAPIKey));

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(fInstructions)
		.Add(fAPIKeyInput)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
		.AddGlue()
		.Add(fSaveButton)
		.End()
		.Layout();
}


SettingsWindow::~SettingsWindow()
{
}


void
SettingsWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kMsgSaveAPIKey:
		{
			BMessage reply(kMsgAPIKeyReceived);
			reply.AddString("apiKey", fAPIKeyInput->Text());
			fOwnerMessenger.SendMessage(&reply);
			Quit();
			break;
		}
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}
}
