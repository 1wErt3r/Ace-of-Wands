#include "CardView.h"
#include "CardModel.h"
#include "Config.h"

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
	fCardWidth(Config::kInitialCardWidth),
	fCardHeight(Config::kInitialCardHeight), // More proportional to tarot card aspect ratio
	fLabelHeight(Config::kInitialLabelHeight), // Increased for better text display
	fReadingAreaWidth(0),
	fReadingView(NULL),
	fScrollView(NULL),
	fSpread(THREE_CARD)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// Set a better font for card labels
	BFont font;
	font.SetSize(Config::kInitialFontSize); // Increased text size
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

	if (fCards.empty()) {
		// If there are no cards, draw a welcome message
		SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
		FillRect(Bounds());

		const char* message = "Choose New Reading from the Ace of Wands menu to get started";
		BFont font;
		GetFont(&font);
		font.SetSize(Config::kInitialFontSize);
		SetFont(&font);

		font_height fh;
		font.GetHeight(&fh);
		float stringWidth = font.StringWidth(message);
		float stringHeight = fh.ascent + fh.descent;

		BRect bounds = Bounds();
		float x = bounds.left + (bounds.Width() - stringWidth) / 2;
		float y = bounds.top + (bounds.Height() - stringHeight) / 2 + fh.ascent;

		SetHighColor(ui_color(B_CONTROL_TEXT_COLOR));
		DrawString(message, BPoint(x, y));
		return;
	}

	// Define area for cards (reading area is handled by the scroll view)
	BRect bounds = Bounds();
	BRect cardArea(bounds.left, bounds.top, bounds.right - fReadingAreaWidth, bounds.bottom);

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
			imageArea.InsetBy(Config::kImageInset, Config::kImageInset);
			// Reduce inset at bottom to leave space for label
			imageArea.bottom -= fLabelHeight - Config::kLabelHeightMargin;

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
		if (stringWidth > cardFrame.Width() - Config::kCardWidthMargin) {
			// Truncate if too long
			BString truncatedName = displayName;
			while (StringWidth(truncatedName.String())
					> cardFrame.Width() - (2 * Config::kCardWidthMargin)
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
CardView::SetSpread(SpreadType spread)
{
	fSpread = spread;
}


void
CardView::LayoutCards()
{
	if (fSpread == THREE_CARD)
		LayoutThreeCardSpread();
	else if (fSpread == TREE_OF_LIFE)
		LayoutTreeOfLifeSpread();
}


void
CardView::LayoutThreeCardSpread()
{
	BRect bounds = Bounds();
	float totalWidth = bounds.Width();

	if (!fReading.IsEmpty())
		fReadingAreaWidth = totalWidth * 0.3;
	else
		fReadingAreaWidth = 0;

	float cardAreaWidth = totalWidth - fReadingAreaWidth;

	// Simplified responsive design - always use 3 columns for the spread
	int cardsPerRow = fCards.size(); // Keep all cards in one row for the spread
	float marginX = Config::kMarginX;
	float marginY = Config::kMarginY;

	// Calculate available space for cards
	float availableWidth = cardAreaWidth - (marginX * 2);
	float cardSpacing = Config::kCardSpacing;

	// Calculate card dimensions
	fCardWidth = (availableWidth - (cardSpacing * (cardsPerRow - 1))) / cardsPerRow;
	fCardHeight = fCardWidth * Config::kCardAspectRatio; // 3.5/2.5 = 1.4

	// Make label height responsive to card size
	fLabelHeight = fCardHeight * Config::kLabelHeightRatio; // 15% of card height for label
	if (fLabelHeight < Config::kMinLabelHeight)
		fLabelHeight = Config::kMinLabelHeight; // Minimum label height
	if (fLabelHeight > Config::kMaxLabelHeight)
		fLabelHeight = Config::kMaxLabelHeight; // Maximum label height

	// Only keep minimum size limits to ensure cards remain visible
	if (fCardWidth < Config::kMinCardWidth)
		fCardWidth = Config::kMinCardWidth;
	if (fCardHeight < Config::kMinCardHeight)
		fCardHeight = Config::kMinCardHeight;

	float totalHeight = bounds.Height();

	if (fCards.size() == 0)
		return;

	// Position cards in a row (centered vertically within the top area)
	float totalRowWidth = (fCardWidth * cardsPerRow) + (cardSpacing * (cardsPerRow - 1));
	float rowStartX = (cardAreaWidth - totalRowWidth) / 2;
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
	float fontSize = fLabelHeight * Config::kFontSizeRatio; // Font size is 50% of label height
	if (fontSize < Config::kMinFontSize)
		fontSize = Config::kMinFontSize; // Minimum font size
	if (fontSize > Config::kMaxFontSize)
		fontSize = Config::kMaxFontSize; // Maximum font size
	font.SetSize(fontSize);
	SetFont(&font);
}


void
CardView::LayoutTreeOfLifeSpread()
{
	BRect bounds = Bounds();
	float totalWidth = bounds.Width();
	float totalHeight = bounds.Height();

	if (!fReading.IsEmpty())
		fReadingAreaWidth = totalWidth * Config::kReadingAreaWidthRatio;
	else
		fReadingAreaWidth = 0;

	float cardAreaWidth = totalWidth - fReadingAreaWidth;

	float marginX = Config::kMarginX;
	float marginY = Config::kMarginY;

	float availableWidth = cardAreaWidth - (marginX * 2);
	float availableHeight = totalHeight - (marginY * 2);

	fCardWidth = availableWidth / Config::kTreeOfLifeCardWidthRatio;
	fCardHeight = fCardWidth * Config::kCardAspectRatio;

	if (fCards.size() != 10)
		return;

	// Positions for the 10 cards in the Tree of Life spread
	BPoint positions[10];
	positions[0] = BPoint(cardAreaWidth / 2, marginY + fCardHeight / 2);
	positions[1] = BPoint(cardAreaWidth / 4, marginY + fCardHeight * 1.5);
	positions[2] = BPoint(cardAreaWidth * 3 / 4, marginY + fCardHeight * 1.5);
	positions[3] = BPoint(cardAreaWidth / 4, marginY + fCardHeight * 2.5);
	positions[4] = BPoint(cardAreaWidth * 3 / 4, marginY + fCardHeight * 2.5);
	positions[5] = BPoint(cardAreaWidth / 2, marginY + fCardHeight * 2.5);
	positions[6] = BPoint(cardAreaWidth / 4, marginY + fCardHeight * 3.5);
	positions[7] = BPoint(cardAreaWidth * 3 / 4, marginY + fCardHeight * 3.5);
	positions[8] = BPoint(cardAreaWidth / 2, marginY + fCardHeight * 3.5);
	positions[9] = BPoint(cardAreaWidth / 2, marginY + fCardHeight * 4.5);

	for (int i = 0; i < 10; i++) {
		float x = positions[i].x - fCardWidth / 2;
		float y = positions[i].y - fCardHeight / 2;
		fCards[i].frame.Set(x, y, x + fCardWidth, y + fCardHeight);
	}
}


void
CardView::LayoutReadingArea()
{
	if (!fScrollView || !fReadingView)
		return;

	BRect bounds = Bounds();
	float readingAreaWidth
		= bounds.Width() * Config::kReadingAreaWidthRatio; // 30% for the reading area

	// Reading area takes the right 30% of the view
	BRect readingArea(bounds.right - readingAreaWidth, bounds.top, bounds.right, bounds.bottom);

	// Set the scroll view to fill the reading area
	fScrollView->MoveTo(readingArea.left, readingArea.top);
	fScrollView->ResizeTo(readingArea.Width(), readingArea.Height());

	// Set the text view to be slightly smaller than the scroll view for borders
	BRect textViewRect = readingArea;
	textViewRect.OffsetTo(0, 0);
	textViewRect.InsetBy(Config::kReadingAreaInset, Config::kReadingAreaInset); // Add some padding

	// Adjust textRect to account for the scrollbar
	textViewRect.right -= B_V_SCROLL_BAR_WIDTH;

	fReadingView->ResizeTo(textViewRect.Width(), textViewRect.Height());
	fReadingView->SetTextRect(textViewRect);
}
