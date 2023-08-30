#ifndef INDEXHANDLER_H
#define INDEXHANDLER_H

#include "AVLTree.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "CountingWord.h"

class indexHandler {
public:
    // avltrees have nodes of words with all documents listed below
    AVLTree<string> treeORG;
    AVLTree<string> treePERSON;
    AVLTree<string> treeKEYWORD;
    // unordered maps match strings to total number of words (or orgs or person)
    unordered_map<string, int> totalORGs = {};
    unordered_map<string, int> totalPERSONs = {};
    unordered_map<string, int> totalKEYWORDs = {};
    // unordered maps match document path to article info including title, author, and date
    unordered_map<string, string> titles = {};
    unordered_map<string, string> authors = {};
    unordered_map<string, string> dates = {};

    vector<CountingWord> searchTree(string, string);

    // Persistency
    void saveTree(AVLTree<string>&, string = "../savedTree.txt");
    void loadTree(AVLTree<string>&, string = "../savedTree.txt");
    void saveMaps(vector<unordered_map<string, int>>&, vector<unordered_map<string, string>>&, vector<string>&);
    void loadMaps(vector<unordered_map<string, int>*>, vector<unordered_map<string, string>*>, vector<string>&);
};

#endif
