#pragma once

#include "MainWindow.h"
#include <Messenger.h>
#include <Window.h>

class BTextControl;
class BButton;
class BStringView;
class BMenuField;
class BPopUpMenu;

const uint32 kMsgSaveAPIKey = 'SvAK';

class SettingsWindow : public BWindow {
public:
	SettingsWindow(BWindow* owner);
	virtual ~SettingsWindow();

	void MessageReceived(BMessage* msg);
	void AttachedToWindow();

private:
	BTextControl* fAPIKeyInput;
	BButton* fSaveButton;
	BStringView* fInstructions;
	BMenuField* fSpreadMenuField;
	BPopUpMenu* fSpreadMenu;
	BMessenger fOwnerMessenger;
};
