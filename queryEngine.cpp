#include "indexHandler.h"
#include "queryEngine.h"
#include "documentParser.h"
#include "AVLTree.h"
#include <string>
#include <vector>
//#include "porter2_stemmer.h"

using namespace std;

queryEngine::~queryEngine() {
}
queryEngine::queryEngine(indexHandler* pointer) {
    handler = pointer;
}
void queryEngine::setIndexHandler(indexHandler* pointer) {
    handler = pointer;
}

vector<string> queryEngine::tokenize(string input) {
    vector<string> words;
    while (input.length() >= 1) {
        //deals with peculiar spacing
        if (input[0] == ' ') {
            if (input.length() == 1) { break; }
            input = input.substr(1,input.length()-1);
            continue;
        }
        if (input[input.length()-1] == ' ') {
            input = input.substr(0,input.length()-1);
        }
        int delimeterPosition = input.find(' ');
        int length = input.length();
        //dealing with things starting with "ORG:, PERSON:, -ORG:, -PERSON:"
        if (input.substr(0,5) == "-ORG:" || input.substr(0,8) == "-PERSON:" || input.substr(0,7) == "PERSON:" || input.substr(0,4) == "ORG:") {
            long unsigned int removedorgfind = input.substr(2,length-2).find("-ORG:");
            long unsigned int orgfind = input.substr(2,length-2).find("ORG:");
            long unsigned int removedpersonfind = input.substr(2,length-2).find("-PERSON:");
            long unsigned int personfind = input.substr(2,length-2).find("PERSON:");

            if (removedorgfind == string::npos) { removedorgfind = length-1; }
            if (orgfind == string::npos) { orgfind = length-1;}
            if (removedpersonfind == string::npos) { removedpersonfind = length-1; }
            if (personfind == string::npos) { personfind = length-1; }

            delimeterPosition = min(min(removedorgfind+1,orgfind+1),min(removedpersonfind+1,personfind+1));
        }
        if (delimeterPosition == length || delimeterPosition == string::npos) {
            words.push_back(input);
            break;
        }

        //if there isn't any spaces left
        words.push_back(input.substr(0,delimeterPosition));
        input = input.substr(delimeterPosition+1,input.length()-delimeterPosition-1);
    }

    return words;
}

void queryEngine::process(string input) {
    vector<string> words = tokenize(input);

    for (int i = 0 ; i < (int) words.size() ; i++) {
        if (parser->isStopWord(words[i]) == true) {
            continue;
        }

        // ORGs
        if (words[i].substr(0,5) == "-ORG:") {
            removedORGs.push_back(words[i].substr(5,words[i].length() - 5));
            continue;
        }

        // removedPERSONs
        if (words[i].substr(0,8) == "-PERSON:") {
            removedPERSONs.push_back(words[i].substr(8,words[i].length() - 8));
            continue;
        }

        // removedKEYWORDs
        if (words[i][0] == '-') {
            string stemmedword = words[i].substr(1,words[i].length() - 1);
//            Porter2Stemmer::trim(stemmedword);
//            Porter2Stemmer::stem(stemmedword);
            removedKEYWORDs.push_back(stemmedword);
            continue;
        }

        // PERSONs
        if (words[i].substr(0,7) == "PERSON:") {
            PERSONs.push_back(words[i].substr(7,words[i].length() - 7));
            continue;
        }

        // ORGs
        if (words[i].substr(0,4) == "ORG:") {
            ORGs.push_back(words[i].substr(4,words[i].length() - 4));
            continue;
        }

        // KEYWORDs
        string stemmedword = words[i];
//        Porter2Stemmer::trim(stemmedword);
//        Porter2Stemmer::stem(stemmedword);
        KEYWORDs.push_back(stemmedword);
    }
}

vector<CountingWord> queryEngine::query(string search) {
    process(search); // splits query into components
    cout << "   Searching for Orgs:  ";
    for (int i = 0 ; i < (int) ORGs.size() ; i++) {
        cout << "\"" << ORGs[i] << "\"" << " ";
    }
    cout << endl << "   Removing Orgs:  ";
    for (int i = 0 ; i < (int) removedORGs.size() ; i++) {
        cout << "\"" << removedORGs[i] << "\"" << " ";
    }
    cout << endl << "   Searching for People:  ";
    for (int i = 0 ; i < (int) PERSONs.size() ; i++) {
        cout << "\"" << PERSONs[i] << "\"" << " ";
    }
    cout << endl << "   Removing People:  ";
    for (int i = 0 ; i < (int) removedPERSONs.size() ; i++) {
        cout << "\"" << removedPERSONs[i] << "\"" << " ";
    }
    cout << endl << "   Searching for Keywords:  ";
    for (int i = 0 ; i < (int) KEYWORDs.size() ; i++) {
        cout << "\"" << KEYWORDs[i] << "\"" << " ";
    }
    cout << endl << "   Removing Keywords:  ";
    for (int i = 0 ; i < (int) removedKEYWORDs.size() ; i++) {
        cout << "\"" << removedKEYWORDs[i] << "\"" << " ";
    }
    cout << endl;


    // performs search for each element in all six vectors
    // for example, in a loop for PERSONs vector:
    // elementResults = handler->searchTree("PERSON", PERSONs[i]);
    // includeDocuments(elementResults,false);
    // for some reason, doing size() doesn't work here?s

    if ((int) ORGs.size() > 0) { numSearchTypes += 1; }
    if ((int) PERSONs.size() > 0) { numSearchTypes += 1; }
    orderedResults = searchVector("KEYWORD",KEYWORDs);
    // add common elements to orderedresults
    if (PERSONs.size() > 0) {
        vector<CountingWord> PERSONresults = searchVector("PERSON",PERSONs);
        orderedResults = saveCommonResults(orderedResults,PERSONresults);
    }
    if (ORGs.size() > 0) {
        vector<CountingWord> ORGresults = searchVector("ORG",ORGs);
        orderedResults = saveCommonResults(orderedResults,ORGresults);
    }
    removeVector(removedKEYWORDs,removedORGs,removedPERSONs);

    ORGs.clear();
    KEYWORDs.clear();
    PERSONs.clear();
    removedORGs.clear();
    removedPERSONs.clear();
    removedKEYWORDs.clear();

    return orderedResults;
}

vector<CountingWord> queryEngine::searchVector(string treeType, vector<string>& positives) {
    //this deals with the logical operators
    vector<CountingWord> totalResults;

    //only finds common terms of positive results
    if (positives.size() > 0) {
        totalResults = handler->searchTree(treeType,positives[0]);
        if (positives.size() > 1) {
            for (int i = 1 ; i < (int) positives.size() ; i++) {
                vector<CountingWord> elementResults = handler->searchTree(treeType,positives[i]); // gets documents associated with positives[i]
                totalResults = saveCommonElements(totalResults,elementResults);

            }
        }
    }

    //turning frequency into relevancy score
    if (treeType == "ORG") {
        for (int i = 0 ; i < (int) totalResults.size() ; i++) {
            totalResults[i].frequency = ((totalResults[i].frequency)/(handler->totalORGs[totalResults[i].word]))/numSearchTypes;
        }
    } else if (treeType == "PERSON") {
        for (int i = 0 ; i < (int) totalResults.size() ; i++) {
            totalResults[i].frequency = ((totalResults[i].frequency)/(handler->totalPERSONs[totalResults[i].word]))/numSearchTypes;
        }
    } else {
        for (int i = 0 ; i < (int) totalResults.size() ; i++) {
            totalResults[i].frequency = ((totalResults[i].frequency)/(handler->totalKEYWORDs[totalResults[i].word]))/numSearchTypes;
        }
    }

    return totalResults;
}

void queryEngine::removeVector(vector<string>& removedKEYWORDs, vector<string>& removedORGs, vector<string>& removedPERSONs) {
    if (orderedResults.size() == 0) {
        return;
    }

    vector<vector<CountingWord>> elementResults;
    //removes negative results
    for (int i = 0 ; i < (int) removedKEYWORDs.size() ; i++) {
        elementResults.push_back(handler->searchTree("KEYWORD",removedKEYWORDs[i]));
    }
    for (int i = 0 ; i < (int) removedPERSONs.size() ; i++) {
        elementResults.push_back(handler->searchTree("PERSON",removedPERSONs[i]));
    }
    for (int i = 0 ; i < (int) removedORGs.size() ; i++) {
        elementResults.push_back(handler->searchTree("ORG",removedORGs[i]));
    }
    //delete common elements
    if (elementResults.size() > 0) {
        for (int i = 0 ; i < (int) orderedResults.size() ; i++) {
            for (int j = 0 ; j < (int) elementResults.size() ; j++) {
                for (int k = 0 ; k < (int) elementResults[j].size() ; k++) {
                    if (orderedResults[i].word == elementResults[j][k].word) {
                        orderedResults.erase(orderedResults.begin() + i);
                    }
                }
            }
        }
    }

    return;
}

vector<CountingWord> queryEngine::saveCommonElements(vector<CountingWord>& a, vector<CountingWord>& b) {

    vector<CountingWord> returnable;

    for (int i = 0 ; i < (int) a.size() ; i++) {
        for (int j = 0 ; j < (int) b.size() ; j++) {
            if (a[i].word == b[j].word) {
                returnable.push_back(a[i]);
                returnable[returnable.size()-1].frequency = a[i].frequency + b[j].frequency;
            }
        }
    }

    return returnable;

}

vector<CountingWord> queryEngine::saveCommonResults(vector<CountingWord>& a, vector<CountingWord>& b) {

    vector<CountingWord> returnable;

    for (int i = 0 ; i < (int) a.size() ; i++) {
        for (int j = 0 ; j < (int) b.size() ; j++) {
            if (a[i].word == b[j].word) {
                returnable.push_back(a[i]);
                returnable[returnable.size()-1].frequency = a[i].frequency + b[j].frequency;
            }
        }
    }

    return returnable;

}
