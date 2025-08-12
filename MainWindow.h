/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Window.h>

class BMenuBar;
class CardModel;
class CardView;
class CardPresenter;

class MainWindow : public BWindow
{
public:
					MainWindow();
	virtual			~MainWindow();

		void		MessageReceived(BMessage* msg);
		bool		QuitRequested();
		void		FrameResized(float width, float height);
		
		void		LoadSpread();

private:
		void		_CreateMenuBar();
		
		BMenuBar*		fMenuBar;
		CardModel*		fCardModel;
		CardView*		fCardView;
		CardPresenter*	fCardPresenter;
};

#endif // MAINWINDOW_H
