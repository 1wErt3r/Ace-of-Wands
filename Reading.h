#pragma once

#include <vector>
#include <support/String.h>

class Reading {
public:
    Reading(const std::vector<BString>& cardNames);
    BString GetInterpretation();

private:
    std::vector<BString> fCardNames;
    BString GenerateInterpretation();
};
