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
#include <MenuBar.h>
#include <Menu.h>
#include <MenuItem.h>
#include <stdio.h>

const uint32 kMsgNewReading = 'NEW_';


MainWindow::MainWindow()
	:
	BWindow(BRect(100, 100, 900, 700), "Ace of Wands", B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE),
	fMenuBar(NULL),
	fCardModel(NULL),
	fCardView(NULL),
	fCardPresenter(NULL)
{
	// Create menu bar
	_CreateMenuBar();
	
	// Create the card model
	fCardModel = new CardModel();
	
	if (fCardModel->Initialize() != B_OK) {
		printf("Failed to initialize card model\n");
	}
	
	// Create the card view
	fCardView = new CardView(Bounds());
	
	// Set resizing mode to follow window size changes
	fCardView->SetResizingMode(B_FOLLOW_ALL_SIDES);
	
	// Create the presenter
	fCardPresenter = new CardPresenter(fCardModel, fCardView);
	
	// Use layout builder to arrange menu bar and card view
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fMenuBar)
		.Add(fCardView);
	
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
		case kMsgNewReading:
			LoadSpread();
			break;
			
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


void
MainWindow::_CreateMenuBar()
{
	fMenuBar = new BMenuBar("MenuBar");
	
	// Create File menu
	BMenu* fileMenu = new BMenu("File");
	fileMenu->AddItem(new BMenuItem("New Reading", new BMessage(kMsgNewReading), 'N'));
	fileMenu->AddSeparatorItem();
	fileMenu->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q'));
	
	fMenuBar->AddItem(fileMenu);
}


