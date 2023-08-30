#ifndef QUERYENGINE_H
#define QUERYENGINE_H

#include "AVLTree.h"
#include "indexHandler.h"
#include "CountingWord.h"
#include "documentParser.h"
#include <string>
#include <vector>

class queryEngine {

private:
    double numSearchTypes = 1;
    // elements of query
    vector<string> ORGs = {};             // includes all words beginning with "ORG:"
    vector<string> PERSONs = {};          // includes all words beginning with "PERSON:"
    vector<string> removedORGs = {};      // includes all words beginning with "-ORG:"
    vector<string> removedPERSONs = {};   // includes all words beginning with "-PERSON:"
    vector<string> removedKEYWORDs = {};  // includes all other words beginning with "-"
    vector<string> KEYWORDs = {};         // includes all other search words
    // searching
    indexHandler* handler;   // pointer to handler generated by document parser
    vector<CountingWord> orderedResults;
    // internal methods
    void process(string); // Processes string into vectors
    vector<string> tokenize(string); // Splits string into words
    vector<CountingWord> searchVector(string, vector<string>&); // performs search through index handler and adds documents
    void removeVector(vector<string>&, vector<string>&, vector<string>&); // removes negative results
    vector<CountingWord> saveCommonElements(vector<CountingWord>&, vector<CountingWord>&); // combines two document lists
    vector<CountingWord> saveCommonResults(vector<CountingWord>&, vector<CountingWord>&); // combines two document lists (averages relevancy score)
public:
    queryEngine(indexHandler*);
    ~queryEngine();
    vector<CountingWord> query(string);
    void setIndexHandler(indexHandler*);
    // document parser
    documentParser* parser;
};

#endif
