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
#include <TextView.h> // Include BTextView
#include <TranslationUtils.h>
#include <TranslatorRoster.h>
#include <iostream>


CardView::CardView(BRect frame)
	:
	BView(frame, "CardView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
	fReadingView(new BTextView("ReadingView")), // Initialize BTextView
	fReading(""), // Initialize fReading
	fCardWidth(Config::kInitialCardWidth),
	fCardHeight(Config::kInitialCardHeight), // More proportional to tarot card aspect ratio
	fLabelHeight(Config::kInitialLabelHeight), // Increased for better text display
	fReadingAreaWidth(0),
	fReadingAreaHeight(0),
	fPreferredSize(frame),
	fSpread(THREE_CARD)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// Set a better font for card labels using configured font size
	BFont font;
	font.SetSize(Config::GetFontSize()); // Use configured font size
	SetFont(&font);

	// Configure the BTextView for read-only display
	fReadingView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fReadingView->MakeEditable(false);
	fReadingView->MakeSelectable(true); // Allow text selection
	fReadingView->SetWordWrap(true);
	// Set a better font for the reading text using configured font size
	BFont readingFont;
	readingFont.SetSize(Config::GetFontSize()); // Use configured font size
	fReadingView->SetFontAndColor(&readingFont);
}


CardView::~CardView()
{
	ClearCards();
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
	AddChild(fReadingView);
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

		font.SetSize(Config::GetFontSize());
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

	// Get the scroll offset
	BPoint scrollOffset = LeftTop();


	BRect bounds = Bounds();
	BRect cardArea(bounds.left + fReadingAreaWidth, bounds.top, bounds.right, bounds.bottom);

	// Draw cards in the card area, accounting for scroll offset
	for (size_t i = 0; i < fCards.size(); i++) {
		// Adjust card frame to fit within card area and account for scroll offset
		BRect cardFrame = fCards[i].frame;
		cardFrame.OffsetBy(-scrollOffset.x,
			-scrollOffset.y); // Adjust for both horizontal and vertical scroll

		// Only draw cards that are within the update rect
		if (!updateRect.Intersects(cardFrame))
			continue;

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
CardView::ScrollTo(BPoint where)
{
	// Call the parent implementation
	BView::ScrollTo(where);

	// Redraw the view
	Invalidate();
}


float
CardView::CalculateTextHeightForTextView(BTextView* textView, const BString& text)
{
	if (text.IsEmpty() || textView == NULL)
		return 0;

	// Get font information from the text view
	BFont font;
	textView->GetFontAndColor(0, &font); // Get font from the first character
	font_height fh;
	font.GetHeight(&fh);
	float lineHeight = fh.ascent + fh.descent + fh.leading;

	// Get the width of the text view for word wrapping
	float width = textView->Bounds().Width();

	// If width is not set or is zero, use a default width for calculation
	if (width <= 0)
		width = 300; // Default width for calculation

	// Split the text into lines
	BString textCopy = text;
	int32 start = 0;
	int32 end = 0;
	float totalHeight = 0;

	while (start < textCopy.Length()) {
		// Find the next line break
		end = textCopy.FindFirst("\n", start);
		if (end == B_ERROR)
			end = textCopy.Length();

		// Get the line
		BString line;
		textCopy.CopyInto(line, start, end - start);

		// Handle word wrapping
		while (line.Length() > 0) {
			// Find the portion that fits
			BString displayLine = line;
			// Use the font from the text view to calculate string width
			while (font.StringWidth(displayLine.String()) > width && displayLine.Length() > 1) {
				// Try to break at a space
				int32 lastSpace = displayLine.FindLast(' ');
				if (lastSpace != B_ERROR && lastSpace > 0) {
					displayLine.Truncate(lastSpace);
				} else {
					// Just truncate
					displayLine.Truncate(displayLine.Length() - 1);
				}
			}

			// Add height for this line
			totalHeight += lineHeight;

			// Move to the next portion
			if (displayLine.Length() < line.Length()) {
				// Remove the displayed portion from line
				if (displayLine.Length() < line.Length()) {
					line.Remove(0, displayLine.Length());
					// Remove leading spaces
					while (line.Length() > 0 && line[0] == ' ')
						line.Remove(0, 1);
				} else {
					line = "";
				}
			} else {
				line = "";
			}
		}

		// Move to the next line
		start = end + 1;
		totalHeight += lineHeight; // Extra space between paragraphs
	}

	return totalHeight + 20; // Add some padding
}


void
CardView::LayoutReadingArea()
{
	BRect bounds = Bounds();
	// Calculate reading area width based on whether there is text to display
	// This ensures card layout accounts for the reading area
	if (!fReading.IsEmpty()) {
		fReadingAreaWidth
			= bounds.Width() * Config::kReadingAreaWidthRatio; // 30% for the reading area
	} else {
		fReadingAreaWidth = 0;
	}

	// Position and size the BTextView
	BRect readingViewRect(bounds.left, bounds.top, bounds.left + fReadingAreaWidth, bounds.bottom);
	readingViewRect.InsetBy(Config::kReadingAreaInset, Config::kReadingAreaInset);

	// Add additional left and top margin
	const float kLeftMargin = 10.0f;
	const float kTopMargin = 10.0f;
	readingViewRect.left += kLeftMargin;
	readingViewRect.top += kTopMargin;

	// Calculate the height needed for the text in the BTextView
	// Get the text from the BTextView itself or from fReading
	// Using fReading is more reliable as it's set by DisplayReading
	BString text = fReading;
	float textHeight = CalculateTextHeightForTextView(fReadingView, text);
	// Ensure the BTextView is at least as tall as the view bounds
	textHeight = textHeight > readingViewRect.Height() ? textHeight : readingViewRect.Height();
	readingViewRect.bottom = readingViewRect.top + textHeight;

	fReadingView->MoveTo(readingViewRect.left, readingViewRect.top);
	fReadingView->ResizeTo(readingViewRect.Width(), readingViewRect.Height());

	// Update the preferred size to accommodate the text height
	// fPreferredSize is initially set by LayoutCards.
	// We need to ensure it's large enough for the text as well.
	if (fPreferredSize.Height() < textHeight + 50)
		fPreferredSize.bottom = fPreferredSize.top + textHeight + 50; // Add some padding
	// If LayoutCards has already set a larger preferred size, we keep it.
}


BSize
CardView::MinSize()
{
	return BView::MinSize();
}


BSize
CardView::MaxSize()
{
	return BView::MaxSize();
}


BSize
CardView::PreferredSize()
{
	// Return the preferred size which accounts for all content
	// Ensure we return a valid size
	if (fPreferredSize.Width() <= 0 || fPreferredSize.Height() <= 0)
		return BView::PreferredSize();
	return BSize(fPreferredSize.Width(), fPreferredSize.Height());
}


void
CardView::DisplayCards(const std::vector<CardInfo>& cards)
{
	ClearCards();

	for (size_t i = 0; i < cards.size(); i++) {
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

		fCards.push_back(display);
	}

	LayoutCards();
	Invalidate();
}


void
CardView::DisplayReading(const BString& reading)
{
	fReading = reading;
	fReadingView->SetText(reading.String());
	RefreshLayout();
}


void
CardView::UpdateReading(const BString& reading)
{
	// This method can be called from a background thread
	// We need to synchronize with the UI thread
	// BTextView is not thread-safe, so we need to use a message
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
	for (size_t i = 0; i < fCards.size(); i++) {
		delete fCards[i].image;
		fCards[i].image = NULL;
	}

	fCards.clear();
	fReading = ""; // Clear the reading text
	fReadingView->SetText(""); // Clear the reading view when cards are cleared
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
CardView::SetFontSize(float size)
{
	// Update the font size for the CardView (card labels)
	BFont font;
	GetFont(&font);
	font.SetSize(size);
	SetFont(&font);

	// Update the font size for the reading view
	BFont readingFont;
	fReadingView->GetFont(&readingFont);
	readingFont.SetSize(size);
	fReadingView->SetFontAndColor(&readingFont);

	// Update the font size in the configuration
	Config::SetFontSize(size);

	// Refresh the layout to apply changes
	RefreshLayout();
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

	// Card area is now on the right, after the reading area
	float cardAreaWidth = totalWidth - fReadingAreaWidth;

	// Simplified responsive design - always use 3 columns for the spread
	int cardsPerRow = fCards.size(); // Keep all cards in one row for the spread
	float marginX = Config::kMarginX;

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
	// Cards are positioned after the reading area (fReadingAreaWidth)
	float totalRowWidth = (fCardWidth * cardsPerRow) + (cardSpacing * (cardsPerRow - 1));
	float rowStartX = fReadingAreaWidth + (cardAreaWidth - totalRowWidth) / 2;
	float contentStartY = (totalHeight - fCardHeight - fLabelHeight) / 2;

	for (size_t i = 0; i < fCards.size(); i++) {
		float xPosition = rowStartX + i * (fCardWidth + cardSpacing);
		float yPosition = contentStartY;

		// Set card frame
		fCards[i].frame.Set(xPosition, yPosition, xPosition + fCardWidth,
			yPosition + fCardHeight + fLabelHeight);
	}

	// Update preferred size - for three card spread, we ensure it's at least the bounds height
	fPreferredSize = bounds;
	fPreferredSize.bottom = totalHeight > bounds.Height() ? totalHeight : bounds.Height();
}


void
CardView::LayoutTreeOfLifeSpread()
{
	BRect bounds = Bounds();
	float totalWidth = bounds.Width();

	if (!fReading.IsEmpty())
		fReadingAreaWidth = totalWidth * Config::kReadingAreaWidthRatio;
	else
		fReadingAreaWidth = 0;

	// Card area is now on the right, after the reading area
	float cardAreaWidth = totalWidth - fReadingAreaWidth;

	float marginX = Config::kMarginX;
	float marginY = Config::kMarginY;

	float availableWidth = cardAreaWidth - (marginX * 2);

	fCardWidth = availableWidth / Config::kTreeOfLifeCardWidthRatio;
	fCardHeight = fCardWidth * Config::kCardAspectRatio;

	if (fCards.size() != 10)
		return;

	// Positions for the 10 cards in the Tree of Life spread
	// Cards are positioned after the reading area (fReadingAreaWidth)
	BPoint positions[10];
	positions[0] = BPoint(fReadingAreaWidth + cardAreaWidth / 2, marginY + fCardHeight / 2);
	positions[1] = BPoint(fReadingAreaWidth + cardAreaWidth / 4, marginY + fCardHeight * 1.5);
	positions[2] = BPoint(fReadingAreaWidth + cardAreaWidth * 3 / 4, marginY + fCardHeight * 1.5);
	positions[3] = BPoint(fReadingAreaWidth + cardAreaWidth / 4, marginY + fCardHeight * 2.5);
	positions[4] = BPoint(fReadingAreaWidth + cardAreaWidth * 3 / 4, marginY + fCardHeight * 2.5);
	positions[5] = BPoint(fReadingAreaWidth + cardAreaWidth / 2, marginY + fCardHeight * 2.5);
	positions[6] = BPoint(fReadingAreaWidth + cardAreaWidth / 4, marginY + fCardHeight * 3.5);
	positions[7] = BPoint(fReadingAreaWidth + cardAreaWidth * 3 / 4, marginY + fCardHeight * 3.5);
	positions[8] = BPoint(fReadingAreaWidth + cardAreaWidth / 2, marginY + fCardHeight * 3.5);
	positions[9] = BPoint(fReadingAreaWidth + cardAreaWidth / 2, marginY + fCardHeight * 4.5);

	// Calculate the preferred size to accommodate all cards
	float requiredHeight = marginY + fCardHeight * 5.5 + marginY;

	for (int i = 0; i < 10; i++) {
		float x = positions[i].x - fCardWidth / 2;
		float y = positions[i].y - fCardHeight / 2;
		fCards[i].frame.Set(x, y, x + fCardWidth, y + fCardHeight);
	}

	// Update preferred size to accommodate all cards
	fPreferredSize = bounds;
	fPreferredSize.bottom = requiredHeight > bounds.Height() ? requiredHeight : bounds.Height();
}
