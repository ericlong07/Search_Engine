#ifndef COUNTINGWORD_H
#define COUNTINGWORD_H

#include <string>

// Normal string except it has a frequency value

class CountingWord
{
public:
    ~CountingWord();
    CountingWord();
    CountingWord(std::string &, double);
    bool operator==(const CountingWord &) const;
    bool operator<(const CountingWord &) const;

    std::string word;
    double frequency;
};

#endif
