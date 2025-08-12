/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CARDVIEW_H
#define CARDVIEW_H

#include <View.h>
#include <String.h>
#include <vector>

class BBitmap;
class BStringView;

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

			void			DisplayCards(const std::vector<class CardInfo>& cards);
			void			ClearCards();
			void			RefreshLayout();

private:
			void			LayoutCards();
			
			std::vector<CardDisplay> fCards;
			float			fCardWidth;
			float			fCardHeight;
			float			fLabelHeight;
};

#endif // CARDVIEW_H