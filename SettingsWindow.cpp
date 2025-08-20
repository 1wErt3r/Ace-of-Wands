#include "SettingsWindow.h"
#include "Config.h"
#include "MainWindow.h"

#include <Alert.h>
#include <Button.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <GroupLayout.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <StringView.h>
#include <TextControl.h>
#include <cstdio>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SettingsWindow"


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

	// Create the log readings checkbox
	fLogReadingsCheckbox = new BCheckBox("logReadings", "Log readings to file",
		new BMessage(kMsgLogReadingsChanged));
	fLogReadingsCheckbox->SetValue(Config::GetLogReadings() ? B_CONTROL_ON : B_CONTROL_OFF);

	fFontSizeInput
		= new BTextControl("fontSizeInput", "Font Size:", "", new BMessage(kMsgFontSizeChanged));
	BString fontSize;
	fontSize << Config::GetFontSize();
	fFontSizeInput->SetText(fontSize.String());

	fSaveButton = new BButton("saveButton", "OK", new BMessage(kMsgSaveAPIKey));

	// Create form groups for better organization
	BGroupView* apiKeyGroup = new BGroupView("API Key", B_VERTICAL, 0);
	BGroupLayout* apiKeyLayout = apiKeyGroup->GroupLayout();
	apiKeyLayout->SetInsets(0, 0, 0, 0);
	apiKeyLayout->AddView(fInstructions);
	apiKeyLayout->AddView(fAPIKeyInput);

	BGroupView* spreadGroup = new BGroupView("Spread Settings", B_VERTICAL, 0);
	BGroupLayout* spreadLayout = spreadGroup->GroupLayout();
	spreadLayout->SetInsets(0, 0, 0, 0);
	spreadLayout->AddView(fSpreadMenuField);
	spreadLayout->AddView(fLogReadingsCheckbox);

	// Use a BGroupLayout for a cleaner, more professional appearance
	BGroupLayout* layout = new BGroupLayout(B_VERTICAL, B_USE_DEFAULT_SPACING);
	this->SetLayout(layout);
	layout->SetInsets(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING,
		B_USE_DEFAULT_SPACING);

	// Set the window background to the default panel color
	this->GetLayout()->View()->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// Add all controls to the layout with appropriate spacing
	layout->AddView(apiKeyGroup);
	layout->AddView(fFontSizeInput);
	layout->AddView(spreadGroup);

	// Add the save button with proper spacing
	layout->AddItem(BSpaceLayoutItem::CreateGlue());
	layout->AddView(fSaveButton);
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
			const char* text = fFontSizeInput->Text();
			if (text != NULL && *text != '\0') {
				char* end;
				long size = strtol(text, &end, 10);
				if (*end != '\0' || size <= 0) {
					BAlert* alert = new BAlert("Error",
						"Invalid font size. Please enter a positive number.", "OK");
					alert->Go();
					return;
				}
				Config::SetFontSize(size);
			}
			Config::SetAPIKey(fAPIKeyInput->Text());
			BMenuItem* item = fSpreadMenu->FindMarked();
			if (item) {
				int32 index = fSpreadMenu->IndexOf(item);
				Config::SetSpread(static_cast<SpreadType>(index));
			}
			// Save the log readings setting
			Config::SetLogReadings(fLogReadingsCheckbox->Value() == B_CONTROL_ON);

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
		case kMsgLogReadingsChanged:
		{
			// The checkbox state has changed, but we don't need to do anything here
			// since we'll save all settings when the user clicks OK
			break;
		}
		case kMsgFontSizeChanged:
		{
			// Send font size change message to main window immediately
			const char* text = fFontSizeInput->Text();
			if (text != NULL && *text != '\0') {
				char* end;
				long size = strtol(text, &end, 10);
				if (*end == '\0' && size > 0) {
					BMessage fontSizeMsg(kMsgFontSizeChanged);
					fontSizeMsg.AddString("fontSize", text);
					fOwnerMessenger.SendMessage(&fontSizeMsg);
				}
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
