#include "CountingWord.h"
#include <string>

using namespace std;

CountingWord::~CountingWord() { } // no dynamic mem allocation
CountingWord::CountingWord(string & t_word, double t_frequency) {
    word = t_word;
    frequency = t_frequency;
}

bool CountingWord::operator==(const CountingWord &x) const {

    string rhs = x.word;
    string lhs = (*this).word;

    if (rhs.size() != lhs.size()) return false;

    for (int i = 0 ; i < (int) rhs.size() ; i++) {
        if (lhs[i] != rhs[i]) return false;
    }

    return true;
}



bool CountingWord::operator<(const CountingWord &rhs) const {
    return (*this).frequency < rhs.frequency;
}
