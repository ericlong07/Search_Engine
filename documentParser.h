#ifndef DOCUMENTPARSER_H
#define DOCUMENTPARSER_H

#include <vector>
#include <string>
#include "indexHandler.h"

using namespace std;

class documentParser {
private:
    vector<string> documentPERSON;      // list of people to pass to indexHandler
    vector<string> documentORG;         // list of orgs
    vector<string> documentKEYWORD;     // list of words
    vector<string> stopList;            // list of stop words
    indexHandler* handler;

public:
    documentParser(indexHandler*);
    void read(const string&);
    void setIndexHandler(indexHandler*);
    bool isStopWord(const string&);

private:
    void process(const string&);
    void getTerms(const char* str);
};

#endif
