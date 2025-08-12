/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef APP_H
#define APP_H

#include <Application.h>

class MainWindow;

class App : public BApplication
{
public:
						App();
	virtual				~App();
	
	virtual void		ReadyToRun();
	virtual void		AboutRequested();

private:
	MainWindow*			fWindow;
};

#endif // APP_H
