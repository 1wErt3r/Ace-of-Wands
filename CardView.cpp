#include "CardView.h"
#include "CardModel.h"

#include <Application.h>
#include <Bitmap.h>
#include <BitmapStream.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <Resources.h>
#include <ScrollBar.h>
#include <ScrollView.h>
#include <StringView.h>
#include <TextView.h>
#include <TranslationUtils.h>
#include <TranslatorRoster.h>
#include <stdio.h>


CardView::CardView(BRect frame)
	:
	BView(frame, "CardView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
	fCardWidth(150),
	fCardHeight(210), // More proportional to tarot card aspect ratio
	fLabelHeight(40), // Increased for better text display
	fCardAreaHeight(0),
	fReadingView(NULL),
	fScrollView(NULL)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// Set a better font for card labels
	BFont font;
	font.SetSize(18.0f); // Increased text size
	SetFont(&font);

	// Create the text view for readings
	BRect textRect(0, 0, 1, 1); // Will be resized later
	fReadingView = new BTextView(textRect, "readingView", textRect, B_FOLLOW_ALL_SIDES);
	fReadingView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fReadingView->MakeEditable(false); // Make it non-editable
	fReadingView->MakeSelectable(true); // Allow text selection
	fReadingView->SetWordWrap(true); // Enable word wrapping

	// Create scroll view for the text view
	fScrollView = new BScrollView("readingScrollView", fReadingView, B_FOLLOW_ALL_SIDES, false,
		true, B_FANCY_BORDER); // Horizontal=false, Vertical=true

	// Add the scroll view as a child
	AddChild(fScrollView);

	// Hide the scroll view until we have content
	fScrollView->Hide();
}


CardView::~CardView()
{
	ClearCards();
	// fReadingView and fScrollView will be deleted automatically as children
}


void
CardView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case 'UPDR':
		{
			BString reading;
			if (message->FindString("reading", &reading) == B_OK)
				DisplayReading(reading);
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
	LayoutReadingArea();
}


void
CardView::Draw(BRect updateRect)
{
	BView::Draw(updateRect);

	// Define area for cards (reading area is handled by the scroll view)
	BRect bounds = Bounds();
	BRect cardArea(bounds.left, bounds.top, bounds.right, bounds.top + fCardAreaHeight);

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
				destRect.OffsetTo(imageArea.left + (imageArea.Width() - scaledWidth) / 2,
					imageArea.top + (imageArea.Height() - scaledHeight) / 2);

				DrawBitmap(fCards[i].image, imageFrame, destRect);
			}
		}

		// Draw label with system default style
		font_height fh;
		GetFontHeight(&fh);
		float labelY = cardFrame.bottom - (fLabelHeight / 2) + (fh.ascent / 2) - fh.descent / 2;

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
			while (StringWidth(truncatedName.String()) > cardFrame.Width() - 20
				&& truncatedName.Length() > 3) {
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
	LayoutReadingArea();
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

		if (display.image)
			printf("Loaded image: %d\n", cards[i].resourceID);
		else
			printf("Failed to load image: %d\n", cards[i].resourceID);

		fCards.push_back(display);
	}

	LayoutCards();
	Invalidate();
}


void
CardView::DisplayReading(const BString& reading)
{
	fReading = reading;
	if (fReadingView) {
		fReadingView->SetText(reading.String());
		// Show the scroll view when we have content
		if (!reading.IsEmpty())
			fScrollView->Show();
		else
			fScrollView->Hide();
	}
	RefreshLayout();
}


void
CardView::UpdateReading(const BString& reading)
{
	// This method can be called from a background thread
	// We need to synchronize with the UI thread
	BMessage* message = new BMessage('UPDR');
	message->AddString("reading", reading);

	// Post message to main thread
	if (Looper())
		Looper()->PostMessage(message, this);
	else
		delete message;
}


void
CardView::ClearCards()
{
	for (int i = 0; i < fCards.size(); i++) {
		delete fCards[i].image;
		fCards[i].image = NULL;
	}

	fCards.clear();
	fReading = ""; // Clear the reading when cards are cleared
	RefreshLayout();
}


void
CardView::RefreshLayout()
{
	LayoutCards();
	LayoutReadingArea();
	Invalidate();
}


void
CardView::LayoutCards()
{
	// Calculate card dimensions based on view size
	BRect bounds = Bounds();
	float totalWidth = bounds.Width();

	if (fCards.size() == 0 && fReading.IsEmpty()) {
		fCardAreaHeight = bounds.Height(); // Cards take 100% height if no cards and no reading
	} else {
		fCardAreaHeight
			= bounds.Height() * 0.7; // Cards take 70% height if there's a reading or cards
	}
	float totalHeight = fCardAreaHeight;

	if (fCards.size() == 0)
		return;

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
	if (fLabelHeight < 30)
		fLabelHeight = 30; // Minimum label height
	if (fLabelHeight > 60)
		fLabelHeight = 60; // Maximum label height

	// Adjust available height after determining label height
	availableHeight -= fLabelHeight;

	// Remove the artificial maximum size limits to use more available space
	// Only keep minimum size limits to ensure cards remain visible
	if (fCardWidth < 100)
		fCardWidth = 100;
	if (fCardHeight < 140)
		fCardHeight = 140;

	// Position cards in a row (centered vertically within the top 70%)
	float totalRowWidth = (fCardWidth * cardsPerRow) + (cardSpacing * (cardsPerRow - 1));
	float rowStartX = (totalWidth - totalRowWidth) / 2;
	float contentStartY = (totalHeight - fCardHeight - fLabelHeight) / 2;

	for (int i = 0; i < fCards.size(); i++) {
		float xPosition = rowStartX + i * (fCardWidth + cardSpacing);
		float yPosition = contentStartY;

		// Set card frame
		fCards[i].frame.Set(xPosition, yPosition, xPosition + fCardWidth,
			yPosition + fCardHeight + fLabelHeight);
	}

	// Update font size based on label height
	BFont font;
	GetFont(&font);
	float fontSize = fLabelHeight * 0.5f; // Font size is 50% of label height
	if (fontSize < 12)
		fontSize = 12; // Minimum font size
	if (fontSize > 24)
		fontSize = 24; // Maximum font size
	font.SetSize(fontSize);
	SetFont(&font);
}


void
CardView::LayoutReadingArea()
{
	if (!fScrollView || !fReadingView)
		return;

	BRect bounds = Bounds();

	// Reading area takes the bottom 30% of the view
	BRect readingArea(bounds.left, bounds.top + fCardAreaHeight, bounds.right, bounds.bottom);

	// Set the scroll view to fill the reading area
	fScrollView->MoveTo(readingArea.left, readingArea.top);
	fScrollView->ResizeTo(readingArea.Width(), readingArea.Height());

	// Set the text view to be slightly smaller than the scroll view for borders
	BRect textViewRect = readingArea;
	textViewRect.OffsetTo(0, 0);
	textViewRect.InsetBy(10, 10); // Add some padding

	// Adjust textRect to account for the scrollbar
	textViewRect.right -= B_V_SCROLL_BAR_WIDTH;

	fReadingView->ResizeTo(textViewRect.Width(), textViewRect.Height());
	fReadingView->SetTextRect(textViewRect);
}
