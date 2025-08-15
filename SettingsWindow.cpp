#include "SettingsWindow.h"
#include "Config.h"
#include "MainWindow.h"

#include <Alert.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <StringView.h>
#include <TextControl.h>


SettingsWindow::SettingsWindow(BWindow* owner)
	:
	BWindow(BRect(100, 100, 500, 300), "Settings", B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS),
	fOwnerMessenger(owner)
{
	fInstructions = new BStringView("instructions", "Enter your DeepSeek API Key:");
	fAPIKeyInput = new BTextControl("apiKeyInput", "API Key:", "", NULL);
	fAPIKeyInput->TextView()->HideTyping(true); // Make it a password field

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

	// Set the current API key if one is already set
	// This needs to be done after the layout is created
	BString currentApiKey = Config::GetAPIKey();
	if (!currentApiKey.IsEmpty())
		fAPIKeyInput->SetText(currentApiKey.String());
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
