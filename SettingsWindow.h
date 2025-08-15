#pragma once

#include <Messenger.h>
#include <Window.h>

class BTextControl;
class BButton;
class BStringView;

const uint32 kMsgSaveAPIKey = 'SvAK';

class SettingsWindow : public BWindow {
public:
	SettingsWindow(BWindow* owner);
	virtual ~SettingsWindow();

	void MessageReceived(BMessage* msg);

private:
	BTextControl* fAPIKeyInput;
	BButton* fSaveButton;
	BStringView* fInstructions;
	BMessenger fOwnerMessenger;
};
