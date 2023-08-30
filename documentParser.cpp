#include "documentParser.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/document.h"
#include "Porter2Stemmer/porter2_stemmer.h"
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <cstring>
#include <unordered_map>
#include "indexHandler.h"

using namespace rapidjson;
using namespace std;

documentParser::documentParser(indexHandler* pointer) {
    handler = pointer;
}

void documentParser::setIndexHandler(indexHandler* pointer) {
    handler = pointer;
}

void documentParser::read(const string& path)
{
    auto files = filesystem::recursive_directory_iterator(path);

    // loop over all the entries.
    for (const auto &f : files)
    {
        // We only want to attempt to parse files that end with .json
        if (f.is_regular_file() && f.path().extension().string() == ".json")
        {
            string pathway = f.path().string();
            process(pathway);

            // adding documents to avl tree and hash map - vectors have all included words
            for (int i = 0 ; i < (int) documentKEYWORD.size() ; i++) {
                handler->treeKEYWORD.addDocument(documentKEYWORD[i],pathway);
            }
            for (int i = 0 ; i < (int) documentPERSON.size() ; i++) {
                handler->treePERSON.addDocument(documentPERSON[i],pathway);
            }
            for (int i = 0 ; i < (int) documentORG.size() ; i++) {
                handler->treeORG.addDocument(documentORG[i],pathway);
            }
            handler->totalORGs[pathway] = (int) documentORG.size();
            handler->totalPERSONs[pathway]= (int) documentPERSON.size();
            handler->totalKEYWORDs[pathway] = (int) documentKEYWORD.size();
            documentKEYWORD.clear();
            documentPERSON.clear();
            documentORG.clear();
        }
    }
}

void documentParser::process(const string& fileName)
{
    // open an ifstream on the file of interest and check that it could be opened.
    fstream file (fileName, ios::in);
    if (!file.is_open())
    {
        cerr << "Can't open file: " << fileName << endl;
        return;
    }

    // Create a RapidJSON IStreamWrapper using the file input stream above.
    IStreamWrapper isw(file);

    // Create a RapidJSON Document object and use it to parse the IStreamWrapper object above.
    Document doc;
    doc.ParseStream(isw);

    auto title = doc["title"].GetString();
    auto author = doc["author"].GetString();
    auto text = doc["text"].GetString();
    auto persons = doc["entities"]["persons"].GetArray();
    auto orgs = doc["entities"]["organizations"].GetArray();
    auto locations = doc["entities"]["locations"].GetArray();

    auto rawDate = doc["published"].GetString();
    string publishedDate = "";
    for (size_t i = 0; i < strlen(rawDate); ++i)
    {
        if (rawDate[i] != 'T')
        {
            publishedDate += rawDate[i];
        }
        else
        {
            break;
        }
    }

    // add article information to hash maps
    handler->titles[fileName] = title;
    handler->authors[fileName] = author;
    handler->dates[fileName] = publishedDate;

    getTerms(title);
    getTerms(text);

    if (strlen(author) > 0)
    {
        documentPERSON.push_back(author);
    }
    for (auto &p : persons)
    {
        string person(p["name"].GetString());
        if (person.size() > 0) { documentPERSON.push_back(person); }
    }
    for (auto &o : orgs)
    {
        string org(o["name"].GetString());
        documentORG.push_back(org);
    }
    for (auto &l : locations)
    {
        string location(l["name"].GetString());
        Porter2Stemmer::trim(location);
        Porter2Stemmer::stem(location);
        documentKEYWORD.push_back(location);
    }

    file.close();
}

// parses article text and stem each word
void documentParser::getTerms(const char* str)
{
    int len = strlen(str);
    string word = "";

    for (int i = 0; i < len; ++i)
    {
        if (str[i] == ' ' || i == (len - 1))
        {
            word += str[i];
            Porter2Stemmer::trim(word);
            Porter2Stemmer::stem(word);
            if (!isStopWord(word) && word.size() > 0)
            {
                documentKEYWORD.push_back(word);
            }
            word = "";
        }
        else
        {
            word += str[i];
        }
    }
}

// checks to see if word is a stop-word by comparing it to a word-list from stopwords.txt
bool documentParser::isStopWord(const string& word)
{
    if (stopList.empty())
    {
        fstream file ("../stopwords.txt", ios::in);
        if (!file.is_open())
        {
            cerr << "Can't open file: stopwords.txt" << endl;
            return true;
        }

        string stopWord;
        while (!file.eof())
        {
            getline(file, stopWord, '\n');
            Porter2Stemmer::trim(stopWord);
            Porter2Stemmer::stem(stopWord);
            stopList.push_back(stopWord);
        }
        file.close();
    }

    if (find(stopList.begin(), stopList.end(), word) != stopList.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}
