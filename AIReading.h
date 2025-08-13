#pragma once

#include <String.h>
#include <vector>

struct CardInfo;

class AIReading {
public:
	static	BString		GetReading(const std::vector<CardInfo>& cards);
};
