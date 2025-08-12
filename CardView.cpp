/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "CardView.h"

#include "CardModel.h"

#include <Bitmap.h>
#include <BitmapStream.h>
#include <TranslatorRoster.h>
#include <TranslationUtils.h>
#include <StringView.h>
#include <LayoutBuilder.h>
#include <stdio.h>

CardView::CardView(BRect frame)
	:
	BView(frame, "CardView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
	fCardWidth(150),
	fCardHeight(250),
	fLabelHeight(30)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}


CardView::~CardView()
{
	ClearCards();
}


void
CardView::AttachedToWindow()
{
	BView::AttachedToWindow();
	LayoutCards();
}


void
CardView::Draw(BRect updateRect)
{
	BView::Draw(updateRect);
	
	// Draw cards
	for (int i = 0; i < fCards.size(); i++) {
		// Draw border
		BRect cardFrame = fCards[i].frame;
		cardFrame.bottom -= fLabelHeight; // Adjust for label
		StrokeRect(cardFrame);
		
		// Draw image
		if (fCards[i].image) {
			BRect imageFrame = fCards[i].image->Bounds();
			
			// Scale image to fit card frame while maintaining aspect ratio
			float scaleX = cardFrame.Width() / imageFrame.Width();
			float scaleY = (cardFrame.Height() - 10) / imageFrame.Height(); // Leave margin
			float scale = scaleX < scaleY ? scaleX : scaleY;
			
			if (scale > 0) {
				float scaledWidth = imageFrame.Width() * scale;
				float scaledHeight = imageFrame.Height() * scale;
				
				BRect destRect(0, 0, scaledWidth, scaledHeight);
				destRect.OffsetTo(
					cardFrame.left + (cardFrame.Width() - scaledWidth) / 2,
					cardFrame.top + (cardFrame.Height() - scaledHeight) / 2
				);
				
				DrawBitmap(fCards[i].image, imageFrame, destRect);
			}
		}
		
		// Draw label background
		BRect labelFrame = fCards[i].frame;
		labelFrame.top = labelFrame.bottom - fLabelHeight;
		FillRect(labelFrame, B_SOLID_LOW);
		
		// Draw label text
		font_height fh;
		GetFontHeight(&fh);
		float labelY = labelFrame.top + (labelFrame.Height() - (fh.ascent + fh.descent)) / 2 + fh.ascent;
		
		BString displayName = fCards[i].displayName;
		float stringWidth = StringWidth(displayName.String());
		float labelX = labelFrame.left + (labelFrame.Width() - stringWidth) / 2;
		
		// Ensure text fits in label area
		if (stringWidth > labelFrame.Width() - 10) {
			// Truncate if too long
			BString truncatedName = displayName;
			while (StringWidth(truncatedName.String()) > labelFrame.Width() - 20 && truncatedName.Length() > 3) {
				truncatedName.Truncate(truncatedName.Length() - 4);
				truncatedName.Append("...");
			}
			stringWidth = StringWidth(truncatedName.String());
			DrawString(truncatedName.String(), BPoint(labelX, labelY));
		} else {
			DrawString(displayName.String(), BPoint(labelX, labelY));
		}
	}
}


void
CardView::FrameResized(float width, float height)
{
	BView::FrameResized(width, height);
	LayoutCards();
	Invalidate();
}


void
CardView::DisplayCards(const std::vector<CardInfo>& cards)
{
	printf("Displaying %d cards\n", (int)cards.size());
	ClearCards();
	
	for (int i = 0; i < cards.size(); i++) {
		CardDisplay display;
		display.displayName = cards[i].displayName;
		
		// Load image
		display.image = BTranslationUtils::GetBitmapFile(cards[i].filePath.String());
		if (display.image) {
			printf("Loaded image: %s\n", cards[i].filePath.String());
		} else {
			printf("Failed to load image: %s\n", cards[i].filePath.String());
		}
		
		fCards.push_back(display);
	}
	
	LayoutCards();
	Invalidate();
}


void
CardView::ClearCards()
{
	for (int i = 0; i < fCards.size(); i++) {
		delete fCards[i].image;
		fCards[i].image = NULL;
	}
	
	fCards.clear();
}


void
CardView::LayoutCards()
{
	if (fCards.size() == 0)
		return;
	
	// Calculate card dimensions based on view size
	BRect bounds = Bounds();
	float totalWidth = bounds.Width();
	float totalHeight = bounds.Height();
	
	// Leave margins around the cards
	float marginX = 20;
	float marginY = 20;
	
	// Calculate available space for cards
	float availableWidth = totalWidth - (marginX * 2);
	float availableHeight = totalHeight - (marginY * 2) - fLabelHeight;
	
	// Calculate card width and height (responsive)
	fCardWidth = (availableWidth - 40) / 3; // 40px total spacing between cards
	fCardHeight = availableHeight - 20; // Leave some margin
	
	// Set minimum and maximum sizes
	if (fCardWidth < 100) fCardWidth = 100;
	if (fCardWidth > 300) fCardWidth = 300;
	
	if (fCardHeight < 150) fCardHeight = 150;
	if (fCardHeight > 400) fCardHeight = 400;
	
	// Calculate positions for 3 cards
	float cardSpacing = (availableWidth - (fCardWidth * 3)) / 2;
	if (cardSpacing < 10) cardSpacing = 10; // Minimum spacing
	
	float yPosition = marginY;
	
	for (int i = 0; i < fCards.size(); i++) {
		float xPosition = marginX + i * (fCardWidth + cardSpacing);
		
		// Set card frame
		fCards[i].frame.Set(xPosition, yPosition, 
			xPosition + fCardWidth, yPosition + fCardHeight + fLabelHeight);
	}
}


