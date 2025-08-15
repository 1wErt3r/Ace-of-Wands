#pragma once

#include <Entry.h>
#include <File.h>
#include <FilePanel.h>
#include <Path.h>
#include <Window.h>

class BMenuBar;
class CardModel;
class CardView;
class CardPresenter;
class SettingsWindow;

const uint32 kAppMessageBase = 'AOW_';
const uint32 kMsgNewReading = kAppMessageBase + 1;
const uint32 kMsgSave = kAppMessageBase + 2;
const uint32 kMsgOpen = kAppMessageBase + 3;
const uint32 kMsgSettings = kAppMessageBase + 4;
const uint32 kMsgAPIKeyReceived = kAppMessageBase + 5;

class MainWindow : public BWindow {
public:
	MainWindow();
	virtual ~MainWindow();

	void MessageReceived(BMessage* msg);
	bool QuitRequested();
	void FrameResized(float width, float height);

	void LoadSpread();

private:
	void _CreateMenuBar();
	void _SaveFile(BPath path);
	void _OpenFile(BPath path);

	BMenuBar* fMenuBar;
	BFilePanel* fOpenFilePanel;
	BFilePanel* fSaveFilePanel;

	CardModel* fCardModel;
	CardView* fCardView;
	CardPresenter* fCardPresenter;
};
