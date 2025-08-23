#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <FilePanel.h>
#include <Window.h>

class CardPresenter;
class CardView;
class BMenuBar;

enum {
	kMsgNewReading = 'newr',
	kMsgOpen = 'open',
	kMsgSave = 'save',
	kMsgSettings = 'sett',
	kMsgAPIKeyReceived = 'akrc',
	kMsgSpreadChanged = 'spch',
	kMsgFontSizeChanged = 'fsch'
};

class MainWindow : public BWindow {
public:
	// Constructor now takes the presenter
	MainWindow(CardPresenter* presenter);
	virtual ~MainWindow();

	virtual void MessageReceived(BMessage* message);
	virtual void FrameResized(float width, float height);
	virtual bool QuitRequested();

private:
	void _CreateMenuBar();

	BMenuBar* fMenuBar;
	CardPresenter* fCardPresenter;
	BFilePanel* fOpenFilePanel;
	BFilePanel* fSaveFilePanel;
};

#endif // MAIN_WINDOW_H
