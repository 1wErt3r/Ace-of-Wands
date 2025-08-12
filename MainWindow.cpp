/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "MainWindow.h"
#include "CardModel.h"
#include "CardView.h"
#include "CardPresenter.h"

#include <Application.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <String.h>
#include <View.h>
#include <stdio.h>


MainWindow::MainWindow()
	:
	BWindow(BRect(100, 100, 900, 700), "Tarot Card Reader", B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE),
	fCardModel(NULL),
	fCardView(NULL),
	fCardPresenter(NULL)
{
	// Create the card model
	fCardModel = new CardModel();
	
	// Set up the cards directory
	BString cardsPath = "/boot/home/Ace_of_Wands/cards";
	
	if (fCardModel->Initialize(cardsPath.String()) != B_OK) {
		// Try relative path
		cardsPath = "cards";
		if (fCardModel->Initialize(cardsPath.String()) != B_OK) {
			printf("Failed to initialize card model with path: %s\n", cardsPath.String());
		} else {
			printf("Initialized card model with relative path: %s\n", cardsPath.String());
		}
	} else {
		printf("Initialized card model with absolute path: %s\n", cardsPath.String());
	}
	
	// Create the card view
	fCardView = new CardView(Bounds());
	
	// Set resizing mode to follow window size changes
	fCardView->SetResizingMode(B_FOLLOW_ALL_SIDES);
	
	// Create the presenter
	fCardPresenter = new CardPresenter(fCardModel, fCardView);
	
	// Add the card view to the window
	AddChild(fCardView);
	
	// Load initial spread
	LoadSpread();
}


MainWindow::~MainWindow()
{
	// Clean up in proper order
	delete fCardPresenter;
	fCardPresenter = NULL;
	
	// Remove the card view from the window before deleting it
	if (fCardView) {
		fCardView->RemoveSelf();
		delete fCardView;
		fCardView = NULL;
	}
	
	delete fCardModel;
	fCardModel = NULL;
}


void
MainWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		default:
		{
			BWindow::MessageReceived(message);
			break;
		}
	}
}


void
MainWindow::FrameResized(float width, float height)
{
	BWindow::FrameResized(width, height);
	
	// Explicitly tell the card view to relayout
	if (fCardView) {
		fCardView->RefreshLayout();
	}
}


bool
MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
MainWindow::LoadSpread()
{
	if (fCardPresenter) {
		fCardPresenter->LoadThreeCardSpread();
	}
}


