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
	BView(frame, "CardView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
	fCardWidth(Config::kInitialCardWidth),
	fCardHeight(Config::kInitialCardHeight), // More proportional to tarot card aspect ratio
	fLabelHeight(Config::kInitialLabelHeight), // Increased for better text display
	fReadingAreaWidth(0),
	fReadingAreaHeight(0),
	fTextHeight(0),
	fPreferredSize(frame),
	fSpread(THREE_CARD)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// Set a better font for card labels
	BFont font;
	font.SetSize(Config::kInitialFontSize); // Increased text size
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

	// Get the scroll offset
	BPoint scrollOffset = LeftTop();

	// Define area for reading text (now on the left)
	BRect bounds = Bounds();
	BRect readingArea(bounds.left, bounds.top, bounds.left + fReadingAreaWidth, bounds.bottom);
	readingArea.OffsetBy(-scrollOffset.x, -scrollOffset.y); // Adjust for scroll offset

	// Draw reading text in the reading area if we have content
	if (!fReading.IsEmpty() && fReadingAreaWidth > 0) {
		// Only draw if the reading area intersects with the update rect
		if (updateRect.Intersects(readingArea)) {
			// Set up drawing parameters for the text
			SetHighColor(ui_color(B_CONTROL_TEXT_COLOR));
			SetLowColor(ui_color(B_CONTROL_BACKGROUND_COLOR));

			BFont font;
			GetFont(&font);

			// Create a text drawer for the reading with some margin
			BRect textRect = readingArea;
			textRect.InsetBy(Config::kReadingAreaInset, Config::kReadingAreaInset);

			// Add additional left and top margin
			const float kLeftMargin = 10.0f;
			const float kTopMargin = 10.0f;
			textRect.left += kLeftMargin;
			textRect.top += kTopMargin;

			// Draw the text
			font_height fh;
			font.GetHeight(&fh);
			float lineHeight = fh.ascent + fh.descent + fh.leading;

			// Split the text into lines
			BString text = fReading;
			int32 start = 0;
			int32 end = 0;
			float y = textRect.top + fh.ascent;
			float maxWidth = textRect.Width();

			while (start < text.Length()) {
				// Find the next line break
				end = text.FindFirst("\n", start);
				if (end == B_ERROR)
					end = text.Length();

				// Get the line
				BString line;
				text.CopyInto(line, start, end - start);

				// Handle word wrapping
				while (line.Length() > 0) {
					// Find the portion that fits
					BString displayLine = line;
					while (
						StringWidth(displayLine.String()) > maxWidth && displayLine.Length() > 1) {
						// Try to break at a space
						int32 lastSpace = displayLine.FindLast(' ');
						if (lastSpace != B_ERROR && lastSpace > 0) {
							displayLine.Truncate(lastSpace);
						} else {
							// Just truncate
							displayLine.Truncate(displayLine.Length() - 1);
						}
					}

					// Draw the line
					if (y >= textRect.top - lineHeight && y <= textRect.bottom + lineHeight)
						DrawString(displayLine.String(), BPoint(textRect.left, y));

					y += lineHeight;

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
				y += lineHeight; // Extra space between paragraphs
			}
		}
	}

	// Define area for cards (now on the right, after the reading area)
	BRect cardArea(bounds.left + fReadingAreaWidth, bounds.top, bounds.right, bounds.bottom);

	// Draw cards in the card area, accounting for scroll offset
	for (int i = 0; i < fCards.size(); i++) {
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
CardView::CalculateTextHeight(const BString& text, float width)
{
	if (text.IsEmpty() || width <= 0)
		return 0;

	// Get font information
	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);
	float lineHeight = fh.ascent + fh.descent + fh.leading;

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
			while (StringWidth(displayLine.String()) > width && displayLine.Length() > 1) {
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
	return BSize(fPreferredSize.Width(), fPreferredSize.Height());
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

	// Card area is now on the right, after the reading area
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
	// Cards are positioned after the reading area (fReadingAreaWidth)
	float totalRowWidth = (fCardWidth * cardsPerRow) + (cardSpacing * (cardsPerRow - 1));
	float rowStartX = fReadingAreaWidth + (cardAreaWidth - totalRowWidth) / 2;
	float contentStartY = (totalHeight - fCardHeight - fLabelHeight) / 2;

	for (int i = 0; i < fCards.size(); i++) {
		float xPosition = rowStartX + i * (fCardWidth + cardSpacing);
		float yPosition = contentStartY;

		// Set card frame
		fCards[i].frame.Set(xPosition, yPosition, xPosition + fCardWidth,
			yPosition + fCardHeight + fLabelHeight);
	}

	// Update preferred size - for three card spread, we ensure it's at least the bounds height
	fPreferredSize = bounds;
	fPreferredSize.bottom = totalHeight > bounds.Height() ? totalHeight : bounds.Height();

	// Update font size based on window size, up to the maximum defined in config
	BFont font;
	GetFont(&font);

	// Calculate font size based on window width - scale from min to max
	float minWindowWidth = 600.0f; // Minimum window width for min font size
	float maxWindowWidth = 1200.0f; // Maximum window width for max font size
	float windowWidth = bounds.Width();

	float fontSize;
	if (windowWidth <= minWindowWidth) {
		fontSize = Config::kMinFontSize;
	} else if (windowWidth >= maxWindowWidth) {
		fontSize = Config::kMaxFontSize;
	} else {
		// Linear interpolation between min and max font sizes
		float ratio = (windowWidth - minWindowWidth) / (maxWindowWidth - minWindowWidth);
		fontSize = Config::kMinFontSize + ratio * (Config::kMaxFontSize - Config::kMinFontSize);
	}

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

	// Card area is now on the right, after the reading area
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

	// Update font size based on window size, up to the maximum defined in config
	BFont font;
	GetFont(&font);

	// Calculate font size based on window width - scale from min to max
	float minWindowWidth = 600.0f; // Minimum window width for min font size
	float maxWindowWidth = 1200.0f; // Maximum window width for max font size
	float windowWidth = bounds.Width();

	float fontSize;
	if (windowWidth <= minWindowWidth) {
		fontSize = Config::kMinFontSize;
	} else if (windowWidth >= maxWindowWidth) {
		fontSize = Config::kMaxFontSize;
	} else {
		// Linear interpolation between min and max font sizes
		float ratio = (windowWidth - minWindowWidth) / (maxWindowWidth - minWindowWidth);
		fontSize = Config::kMinFontSize + ratio * (Config::kMaxFontSize - Config::kMinFontSize);
	}

	font.SetSize(fontSize);
	SetFont(&font);
}


void
CardView::LayoutReadingArea()
{
	BRect bounds = Bounds();
	float readingAreaWidth
		= bounds.Width() * Config::kReadingAreaWidthRatio; // 30% for the reading area

	// Calculate the height needed for the text
	float textHeight
		= CalculateTextHeight(fReading, readingAreaWidth - 2 * Config::kReadingAreaInset);

	// Store the reading area dimensions for drawing
	fReadingAreaWidth = readingAreaWidth;
	fTextHeight = textHeight;

	// Update the preferred size to accommodate the text height if it's larger than the view
	float totalHeight = bounds.Height();
	if (textHeight > totalHeight) {
		fPreferredSize = bounds;
		fPreferredSize.bottom = bounds.top + textHeight + 50; // Add some padding
	} else {
		fPreferredSize = bounds;
	}
}
