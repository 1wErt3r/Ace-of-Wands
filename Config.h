#pragma once

#include <String.h>

class Config {
public:
	static	BString		GetAPIKey();
	static	bool		IsAPIKeySet();
};
