#pragma once

#include <View.h>
#include <String.h>
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
	virtual			~CardView();

	virtual	void			AttachedToWindow();
	virtual	void			Draw(BRect updateRect);
	virtual	void			FrameResized(float width, float height);
	virtual	void			MessageReceived(BMessage* message);

			void			DisplayCards(const std::vector<class CardInfo>& cards);
			void			DisplayReading(const BString& reading);
			
			// Thread-safe method to update reading from background thread
			void			UpdateReading(const BString& reading);

			void			ClearCards();
			void			RefreshLayout();

private:
			void			LayoutCards();
			
			std::vector<CardDisplay> fCards;
			BString			fReading;
			float			fCardWidth;
			float			fCardHeight;
			float			fLabelHeight;
};
