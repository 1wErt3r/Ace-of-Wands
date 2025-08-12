/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef AIREADING_H
#define AIREADING_H

#include <String.h>
#include <vector>

struct CardInfo;

class AIReading {
public:
	static	BString		GetReading(const std::vector<CardInfo>& cards);
};

#endif // AIREADING_H