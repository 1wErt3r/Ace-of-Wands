#pragma once

#include <FilePanel.h>
#include <Window.h>

class BMenuBar;
class CardPresenter;
class SettingsWindow;

const uint32 kAppMessageBase = 'AOW_';
const uint32 kMsgNewReading = kAppMessageBase + 1;
const uint32 kMsgSave = kAppMessageBase + 2;
const uint32 kMsgOpen = kAppMessageBase + 3;
const uint32 kMsgSettings = kAppMessageBase + 4;
const uint32 kMsgAPIKeyReceived = kAppMessageBase + 5;
const uint32 kMsgSpreadChanged = kAppMessageBase + 6;

class MainWindow : public BWindow {
public:
	MainWindow();
	virtual ~MainWindow();

	void MessageReceived(BMessage* msg);
	bool QuitRequested();
	void FrameResized(float width, float height);

private:
	void _CreateMenuBar();

	BMenuBar* fMenuBar;
	BFilePanel* fOpenFilePanel;
	BFilePanel* fSaveFilePanel;

	CardPresenter* fCardPresenter;
};
