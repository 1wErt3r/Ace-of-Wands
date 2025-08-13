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
	CardModel* fCardModel;
	CardView* fCardView;
	CardPresenter* fCardPresenter;
	BFilePanel* fOpenFilePanel;
	BFilePanel* fSaveFilePanel;
};
