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
#include <Message.h>
#include <stdio.h>
#include <Application.h>
#include <Resources.h>

CardView::CardView(BRect frame)
	: BView(frame, "CardView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
	fCardWidth(150),
	fCardHeight(210), // More proportional to tarot card aspect ratio
	fLabelHeight(40)  // Increased for better text display
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	// Set a better font for card labels
	BFont font;
	font.SetSize(18.0f); // Increased text size
	SetFont(&font);
}


CardView::~CardView()
{
	ClearCards();
}


void
CardView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case 'UPDR': {
			BString reading;
			if (message->FindString("reading", &reading) == B_OK) {
				DisplayReading(reading);
			}
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
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
	
	// Define areas for cards and reading
	BRect bounds = Bounds();
	float cardAreaHeight = bounds.Height() * 0.7; // Use 70% of height for cards
	BRect cardArea(bounds.left, bounds.top, bounds.right, bounds.top + cardAreaHeight);
	BRect readingArea(bounds.left + 20, bounds.top + cardAreaHeight + 20, bounds.right - 20, bounds.bottom - 20); // Add margins
	
	// Draw cards in the top area
	for (int i = 0; i < fCards.size(); i++) {
		// Adjust card frame to fit within card area
		BRect cardFrame = fCards[i].frame;
		cardFrame.OffsetBy(0, cardArea.top);
		
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
		
		// Draw label with system default style
		font_height fh;
		GetFontHeight(&fh);
		float labelY = cardFrame.bottom - (fLabelHeight/2) + (fh.ascent/2) - fh.descent/2;
		
		BString displayName = fCards[i].displayName;
		float stringWidth = StringWidth(displayName.String());
		float labelX = cardFrame.left + (cardFrame.Width() - stringWidth) / 2;
		
		// Use system default colors for text
		SetHighColor(ui_color(B_CONTROL_TEXT_COLOR));
		SetLowColor(ui_color(B_CONTROL_BACKGROUND_COLOR));
		
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
	
	// Draw the AI reading in the bottom area
	if (!fReading.IsEmpty()) {
		// Draw a background for the reading
		SetHighColor(240, 240, 240, 255); // Light gray background
		FillRect(readingArea);
		
		// Draw a border around the reading
		SetHighColor(150, 150, 150, 255); // Gray border
		StrokeRect(readingArea);
		
		// Set a larger font for the reading
		BFont font;
		GetFont(&font);
		font.SetSize(18.0f); // Increased font size
		SetFont(&font);
		
		// Draw the reading text with proper word wrapping
		SetHighColor(0, 0, 0, 255); // Black text
		
		// Split the reading into lines that fit within the reading area
		BString reading = fReading;
		float lineHeight = font.Size() * 1.5f;
		float y = readingArea.top + lineHeight;
		float maxWidth = readingArea.Width() - 20; // Leave some padding
		
		int32 start = 0;
		
		while (start < reading.Length() && y < readingArea.bottom - lineHeight) {
			// Find the end of the line that fits within maxWidth
			int32 end = start;
			int32 lastSpace = start;
			
			// Measure words until we exceed the width
			while (end < reading.Length()) {
				// Check for space to track word boundaries
				if (reading[end] == ' ') {
					lastSpace = end;
				}
				
				// Check for explicit line breaks
				if (reading[end] == '\n') {
					end++;
					break;
				}
				
				// Create a substring to measure
				BString substring;
				reading.CopyInto(substring, start, end - start + 1);
				
				// If this substring exceeds our max width
				if (StringWidth(substring.String()) > maxWidth) {
					// If we've found a space, break there
					if (lastSpace > start) {
						end = lastSpace;
					}
					// Otherwise, we have a long word that we need to break
					else if (end > start) {
						end--; // Step back one character
					}
					break;
				}
				
				end++;
			}
			
			// Handle case where we reached the end of the string
			if (end >= reading.Length()) {
				end = reading.Length();
			}
			
			// Extract the line
			BString line;
			reading.CopyInto(line, start, end - start);
			
			// Skip leading spaces
			int32 lineStart = 0;
			while (lineStart < line.Length() && line[lineStart] == ' ') {
				lineStart++;
			}
			
			if (lineStart < line.Length()) {
				BString trimmedLine;
				line.CopyInto(trimmedLine, lineStart, line.Length() - lineStart);
				// Draw the line
				DrawString(trimmedLine.String(), BPoint(readingArea.left + 10, y));
			}
			
			// Move to the next line
			y += lineHeight;
			
			// Move start position to the next non-space character
			start = end;
			while (start < reading.Length() && reading[start] == ' ') {
				start++;
			}
			
			// Skip the newline character if that's what we ended on
			if (start < reading.Length() && reading[start] == '\n') {
				start++;
			}
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
		
		// Load image from resources
		BResources* appResources = BApplication::AppResources();
		if (appResources) {
			size_t size;
			const void* data = appResources->LoadResource('BBMP', cards[i].resourceID, &size);
			if (data) {
				BMemoryIO stream(data, size);
				display.image = BTranslationUtils::GetBitmap(&stream);
			}
		}

		if (display.image) {
			printf("Loaded image: %d\n", cards[i].resourceID);
		} else {
			printf("Failed to load image: %d\n", cards[i].resourceID);
		}
		
		fCards.push_back(display);
	}
	
	LayoutCards();
	Invalidate();
}


void
CardView::DisplayReading(const BString& reading)
{
	fReading = reading;
	Invalidate();
}


void
CardView::UpdateReading(const BString& reading)
{
	// This method can be called from a background thread
	// We need to synchronize with the UI thread
	BMessage* message = new BMessage('UPDR');
	message->AddString("reading", reading);
	
	// Post message to main thread
	if (Looper()) {
		Looper()->PostMessage(message, this);
	} else {
		delete message;
	}
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
	float totalHeight = bounds.Height() * 0.7; // Use only 70% of height for cards
	
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
	
	// Make label height responsive to card size
	fLabelHeight = fCardHeight * 0.15f; // 15% of card height for label
	if (fLabelHeight < 30) fLabelHeight = 30; // Minimum label height
	if (fLabelHeight > 60) fLabelHeight = 60; // Maximum label height
	
	// Adjust available height after determining label height
	availableHeight -= fLabelHeight;
	
	// Remove the artificial maximum size limits to use more available space
	// Only keep minimum size limits to ensure cards remain visible
	if (fCardWidth < 100) fCardWidth = 100;
	if (fCardHeight < 140) fCardHeight = 140;
	
	// Position cards in a row (centered vertically within the top 70%)
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
	
	// Update font size based on label height
	BFont font;
	GetFont(&font);
	float fontSize = fLabelHeight * 0.5f; // Font size is 50% of label height
	if (fontSize < 12) fontSize = 12; // Minimum font size
	if (fontSize > 24) fontSize = 24; // Maximum font size
	font.SetSize(fontSize);
	SetFont(&font);
}