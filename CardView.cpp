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
	fCardHeight(210), // More proportional to tarot card aspect ratio
	fLabelHeight(35)  // Slightly taller for better text display
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	// Set a better font for card labels
	BFont font;
	font.SetSize(14.0f); // Increased text size
	SetFont(&font);
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
		// Draw border with rounded corners for modern look
		BRect cardFrame = fCards[i].frame;
		
		// Draw card background with subtle gradient effect
		SetHighColor(250, 250, 250, 255); // Very light gray background
		FillRoundRect(cardFrame, 10, 10);
		
		// Draw a subtle shadow effect
		SetHighColor(200, 200, 200, 100); // Light gray, semi-transparent
		BRect shadowFrame = cardFrame;
		shadowFrame.OffsetBy(2, 2);
		FillRoundRect(shadowFrame, 10, 10);
		
		// Draw card border
		SetHighColor(150, 150, 150, 255); // Gray border
		StrokeRoundRect(cardFrame, 10, 10);
		
		// Draw image
		if (fCards[i].image) {
			BRect imageFrame = fCards[i].image->Bounds();
			
			// Scale image to fit card frame while maintaining aspect ratio
			// Leave a small margin around the image
			BRect imageArea = cardFrame;
			imageArea.InsetBy(10, 10);
			// Reduce inset at bottom to leave space for label
			imageArea.bottom -= fLabelHeight - 10;
			
			float scaleX = imageArea.Width() / imageFrame.Width();
			float scaleY = imageArea.Height() / imageFrame.Height();
			float scale = scaleX < scaleY ? scaleX : scaleY;
			
			if (scale > 0) {
				float scaledWidth = imageFrame.Width() * scale;
				float scaledHeight = imageFrame.Height() * scale;
				
				BRect destRect(0, 0, scaledWidth, scaledHeight);
				destRect.OffsetTo(
					imageArea.left + (imageArea.Width() - scaledWidth) / 2,
					imageArea.top + (imageArea.Height() - scaledHeight) / 2
				);
				
				DrawBitmap(fCards[i].image, imageFrame, destRect);
			}
		}
		
		// Draw label text directly on the card
		font_height fh;
		GetFontHeight(&fh);
		float labelY = cardFrame.bottom - (fLabelHeight/2) + (fh.ascent/2) - fh.descent/2;
		
		BString displayName = fCards[i].displayName;
		float stringWidth = StringWidth(displayName.String());
		float labelX = cardFrame.left + (cardFrame.Width() - stringWidth) / 2;
		
		// Semi-transparent dark background for text readability
		BRect labelBgRect(labelX - 5, cardFrame.bottom - fLabelHeight, labelX + stringWidth + 5, cardFrame.bottom);
		SetHighColor(0, 0, 0, 128); // Semi-transparent black
		FillRoundRect(labelBgRect, 5, 5);
		
		// White text for better contrast
		SetHighColor(255, 255, 255, 255); // White
		SetLowColor(0, 0, 0, 128); // Match background for text rendering
		
		// Ensure text is centered and fits in label area
		if (stringWidth > cardFrame.Width() - 10) {
			// Truncate if too long
			BString truncatedName = displayName;
			while (StringWidth(truncatedName.String()) > cardFrame.Width() - 20 && truncatedName.Length() > 3) {
				truncatedName.Truncate(truncatedName.Length() - 4);
				truncatedName.Append("...");
			}
			stringWidth = StringWidth(truncatedName.String());
			labelX = cardFrame.left + (cardFrame.Width() - stringWidth) / 2;
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
CardView::RefreshLayout()
{
	LayoutCards();
	Invalidate();
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
	
	// Simplified responsive design - always use 3 columns for the spread
	int cardsPerRow = fCards.size(); // Keep all cards in one row for the spread
	float marginX = 20;
	float marginY = 20;
	
	// Calculate available space for cards
	float availableWidth = totalWidth - (marginX * 2);
	float availableHeight = totalHeight - (marginY * 2) - fLabelHeight;
	
	// Calculate card dimensions
	float cardSpacing = 20.0f;
	fCardWidth = (availableWidth - (cardSpacing * (cardsPerRow - 1))) / cardsPerRow;
	
	// Calculate card height maintaining aspect ratio (standard tarot card ratio ~ 2.5:3.5)
	fCardHeight = fCardWidth * 1.4f; // 3.5/2.5 = 1.4
	
	// Remove the artificial maximum size limits to use more available space
	// Only keep minimum size limits to ensure cards remain visible
	if (fCardWidth < 100) fCardWidth = 100;
	if (fCardHeight < 140) fCardHeight = 140;
	
	// Position cards in a row
	float totalRowWidth = (fCardWidth * cardsPerRow) + (cardSpacing * (cardsPerRow - 1));
	float rowStartX = (totalWidth - totalRowWidth) / 2;
	float contentStartY = (totalHeight - fCardHeight - fLabelHeight) / 2;
	
	for (int i = 0; i < fCards.size(); i++) {
		float xPosition = rowStartX + i * (fCardWidth + cardSpacing);
		float yPosition = contentStartY;
		
		// Set card frame
		fCards[i].frame.Set(xPosition, yPosition, 
			xPosition + fCardWidth, yPosition + fCardHeight + fLabelHeight);
	}
}


