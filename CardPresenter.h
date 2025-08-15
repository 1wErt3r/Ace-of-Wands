#pragma once

#include "Reading.h"
#include <Path.h>
#include <String.h>
#include <future>
#include <thread>
#include <vector>

class CardModel;
class CardView;
class BView;

class CardPresenter {
public:
	CardPresenter();
	~CardPresenter();

	void NewReading();
	void OnFrameResized();
	void SaveFile(const BPath& path);
	void OpenFile(const BPath& path);
	BString GetCurrentReading() const { return fCurrentReading; }
	BView* GetView();
	BString GetAPIKey();
	void SetAPIKey(const BString& apiKey);

private:
	void LoadThreeCardSpread();

	CardModel* fModel;
	CardView* fView;
	std::future<void> fReadingFuture;
	Reading* fReading;
	BString fCurrentReading;
};
