#pragma once

#include <support/String.h>
#include <vector>

class Reading {
public:
	Reading(const std::vector<BString>& cardNames);
	BString GetInterpretation();

private:
	std::vector<BString> fCardNames;
	BString GenerateInterpretation();
};
