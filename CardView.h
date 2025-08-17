#pragma once

#include "CardPresenter.h"
#include <String.h>
#include <View.h>
#include <vector>

class BBitmap;

struct CardDisplay {
	BBitmap* image;
	BRect frame;
	BString displayName;
};

class CardView : public BView {
public:
	CardView(BRect frame);
	virtual ~CardView();

	virtual void AttachedToWindow();
	virtual void Draw(BRect updateRect);
	virtual void FrameResized(float width, float height);
	virtual void MessageReceived(BMessage* message);
	virtual void ScrollTo(BPoint where);

	// Override to provide the preferred size for scrolling
	virtual BSize MinSize();
	virtual BSize MaxSize();
	virtual BSize PreferredSize();

	void DisplayCards(const std::vector<class CardInfo>& cards);
	void DisplayReading(const BString& reading);

	// Thread-safe method to update reading from background thread
	void UpdateReading(const BString& reading);

	void ClearCards();
	void RefreshLayout();
	void SetSpread(SpreadType spread);

private:
	void LayoutCards();
	void LayoutReadingArea();
	void LayoutThreeCardSpread();
	void LayoutTreeOfLifeSpread();
	float CalculateTextHeight(const BString& text, float width);

	std::vector<CardDisplay> fCards;
	BString fReading;
	float fCardWidth;
	float fCardHeight;
	float fLabelHeight;
	float fReadingAreaWidth;
	float fReadingAreaHeight;
	BRect fPreferredSize;
	float fTextHeight;
	SpreadType fSpread;
};
