#include "SettingsWindow.h"
#include "Config.h"
#include "MainWindow.h"

#include <Alert.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <StringView.h>
#include <TextControl.h>
#include <cstdio>


SettingsWindow::SettingsWindow(BWindow* owner)
	:
	BWindow(BRect(Config::kSettingsWindowLeft, Config::kSettingsWindowTop,
				Config::kSettingsWindowRight, Config::kSettingsWindowBottom),
		"Settings", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS),
	fOwnerMessenger(owner)
{
	fInstructions = new BStringView("instructions", "Enter your DeepSeek API Key:");
	fAPIKeyInput = new BTextControl("apiKeyInput", "API Key:", "", NULL);
	fAPIKeyInput->TextView()->HideTyping(true); // Make it a password field

	// Populate the API key field when the window is created
	BString apiKey = Config::LoadAPIKeyFromFile();
	fAPIKeyInput->SetText(apiKey.String());

	fSpreadMenu = new BPopUpMenu("Spread");
	fSpreadMenu->AddItem(new BMenuItem("Three Card", new BMessage(kMsgSpreadChanged)));
	fSpreadMenu->AddItem(new BMenuItem("Tree of Life", new BMessage(kMsgSpreadChanged)));

	fSpreadMenuField = new BMenuField("spreadMenuField", "Tarot Spread:", fSpreadMenu);

	// Set the initial value of the spread menu
	SpreadType spread = Config::GetSpread(); // Use in-memory state
	BMenuItem* item = fSpreadMenu->ItemAt(static_cast<int32>(spread));
	if (item)
		item->SetMarked(true);

	fSaveButton = new BButton("saveButton", "Save", new BMessage(kMsgSaveAPIKey));

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(fInstructions)
		.Add(fAPIKeyInput)
		.Add(fSpreadMenuField)
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
			Config::SetAPIKey(fAPIKeyInput->Text());
			BMenuItem* item = fSpreadMenu->FindMarked();
			if (item) {
				int32 index = fSpreadMenu->IndexOf(item);
				Config::SetSpread(static_cast<SpreadType>(index));
			}
			BMessage reply(kMsgAPIKeyReceived);
			reply.AddString("apiKey", fAPIKeyInput->Text());
			fOwnerMessenger.SendMessage(&reply);
			Quit();
			break;
		}
		case kMsgSpreadChanged:
		{
			BMenuItem* item = fSpreadMenu->FindMarked();
			if (item) {
				BMessage spreadMsg(kMsgSpreadChanged);
				spreadMsg.AddString("spread", item->Label());
				fOwnerMessenger.SendMessage(&spreadMsg);
			}
			break;
		}
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}
}
