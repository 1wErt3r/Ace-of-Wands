/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <String.h>

class Config {
public:
	static	BString		GetAPIKey();
	static	bool		IsAPIKeySet();
};

#endif // CONFIG_H