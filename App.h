#pragma once

#include <Application.h>

class MainWindow;

class App : public BApplication
{
public:
						App();
	virtual				~App();
	
	virtual void		ReadyToRun();

private:
	MainWindow*			fWindow;
};
