#include "indexHandler.h"
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include "CountingWord.h"

using namespace std;

vector<CountingWord> indexHandler::searchTree(string treeType, string keyword) {
    // Processes keyword and searches the AVL Tree
    // Documents associated with keyword are output, along with frequencies

    AVLTree<string>* mytree;
    if (treeType == "PERSON") {
        mytree = &treePERSON;
    } else if (treeType == "ORG") {
        mytree = &treeORG;
    } else {
        mytree = &treeKEYWORD;
    }

    vector<string> rawDocuments = mytree->getDocuments(keyword);
    vector<CountingWord> documents;

    for (size_t n = 0 ; n < rawDocuments.size() ; n++ ) {
        //for each string in rawDocuments

        if (documents.size() > 0) {

            //searches through document vector's word values for rawDocument[i]
            // if found: add one to document[i]'s frequency values
            // if not found: push back string of document[i] with a frequency of 1

            bool isIncluded = false;
            for (size_t i = 0 ; i < documents.size() ; i++) {
                if (documents[i].word == rawDocuments[n]) {
                    isIncluded = true;
                    // found at document[i]
                    documents[i].frequency += 1;
                }
            }


            if (!isIncluded) {
                documents.push_back(CountingWord(rawDocuments[n],1));
            }
        } else {
            documents.push_back(CountingWord(rawDocuments[n],1));
        }

    }

    return documents;
}

void indexHandler::saveTree(AVLTree<string>& myTree, string filePath)
{
    myTree.saveTree(filePath);
}

void indexHandler::loadTree(AVLTree<string>& myTree, string filePath)
{
    fstream file (filePath, ios::in);
    if(!file.is_open())
    {
        cerr << "File not open: " << filePath << endl;
    }

    myTree.makeEmpty(); // clear tree before filling it with new information

    string word, numDocs, doc;
    int counter = 1; // keeps track of line number in read file incase of error
    while (!file.eof())
    {
        // get the word
        getline(file, word, '^');
        if (word != "" && word.size() > 0)
        {
            myTree.insert(word);
        }
        else
        {
            getline(file, doc, '\n');
            continue;
        }

        // get the number of documents
        int Docs = 0;
        getline(file, numDocs, '^');
        try
        {
            Docs = stoi(numDocs);
        }
        catch (...)
        {
            cerr << "stoi conversion exception: " << numDocs << " -> " << filePath << " -> line-" << counter << endl;
        }
        counter++;

        if (Docs == 0)
        {
            getline(file, doc, '\n');
        }
        for (int i = 0; i < Docs; ++i)
        {
            if (i + 1 != Docs)
            {
                getline(file, doc, '^');
                myTree.addDocument(word, doc);
            }
            else
            {
                getline(file, doc, '\n');
                myTree.addDocument(word, doc);
            }
        }
    }
    file.close();
}

void indexHandler::saveMaps(vector<unordered_map<string, int>>& totalMaps, vector<unordered_map<string, string>>& infoMaps, vector<string>& paths)
{
    fstream file;
    int tracker = 0;
    for(size_t i = 0; i < paths.size(); ++i)
    {
        file.open(paths[i], ios::out);
        if(!file.is_open())
        {
            cerr << "File not open: " << paths[i] << endl;
        }

        if (i < 3) // first three are total frequency maps
        {
            for(const auto& pair: totalMaps[i])
            {
                file << pair.first << ";" << pair.second << "\n";
            }
            tracker++;
        }
        else // last three are article information maps including title, author, and date
        {
            for(const auto& pair: infoMaps[i - tracker])
            {
                file << pair.first << ";" << pair.second << "\n";
            }
        }

        file.close();
    }
}

void indexHandler::loadMaps(vector<unordered_map<string, int>*> totalMaps, vector<unordered_map<string, string>*> infoMaps, vector<string>& paths)
{
    fstream file;
    int tracker = 0;
    string val1, val2;
    for(size_t i = 0; i < paths.size(); ++i)
    {
        file.open(paths[i], ios::in);
        if(!file.is_open())
        {
            cerr << "File not open: " << paths[i] << endl;
        }

        if (i < 3)
        {
            totalMaps[i]->erase(totalMaps[i]->begin(), totalMaps[i]->end());

            while (!file.eof())
            {
                getline(file, val1, ';');
                getline(file, val2, '\n');
                if (val1.size() > 0) { (*totalMaps[i])[val1] = stoi(val2); }
            }
            tracker++;
        }
        else
        {
            infoMaps[i - tracker]->erase(infoMaps[i - tracker]->begin(), infoMaps[i - tracker]->end());

            while (!file.eof())
            {
                getline(file, val1, ';');
                getline(file, val2, '\n');
                if (val1.size() > 0) { (*infoMaps[i - tracker])[val1] = val2; }
            }
        }

        file.close();
    }
}
