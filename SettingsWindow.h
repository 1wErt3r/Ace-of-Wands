#pragma once

#include "MainWindow.h"
#include <Messenger.h>
#include <Window.h>

class BTextControl;
class BButton;
class BStringView;
class BMenuField;
class BPopUpMenu;
class BCheckBox;

const uint32 kMsgSaveAPIKey = 'SvAK';
const uint32 kMsgLogReadingsChanged = 'LogR';
const uint32 kMsgFontSizeChanged = 'FnSz';

class SettingsWindow : public BWindow {
public:
	SettingsWindow(BWindow* owner);
	virtual ~SettingsWindow();

	void MessageReceived(BMessage* msg);
	void AttachedToWindow();

private:
	BTextControl* fAPIKeyInput;
	BTextControl* fFontSizeInput;
	BButton* fSaveButton;
	BStringView* fInstructions;
	BMenuField* fSpreadMenuField;
	BPopUpMenu* fSpreadMenu;
	BCheckBox* fLogReadingsCheckbox;
	BMessenger fOwnerMessenger;
};
