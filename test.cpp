#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "AVLTree.h"
#include <string>
#include <vector>
#include "CountingWord.h"
#include "indexHandler.h"
#include "queryEngine.h"
#include "documentParser.h"
#include "Porter2Stemmer/porter2_stemmer.h"

TEST_CASE("Check that AVLTree works", "[AVLTree]")
{
indexHandler myHandler;

AVLTree<string> myTree;
string fruit = "apple";
string drink = "Coke";

myTree.makeEmpty(); // ensure makeEmpty() on empty tree does not result in an error
REQUIRE(myTree.isEmpty() == true);
REQUIRE(myTree.contains(fruit) == false);

myTree.insert(fruit);
myTree.insert(drink);
myTree.addDocument(drink, "test");
myTree.addDocument(drink, "test3");
myTree.addDocument(drink, "test5");
REQUIRE(myTree.isEmpty() == false);
REQUIRE(myTree.contains(fruit) == true);
REQUIRE(myTree.contains(drink) == true);
REQUIRE(myTree.getDocuments(fruit).empty() == 1);
REQUIRE(myTree.getDocuments(drink)[0] == "test");
REQUIRE(myTree.getDocuments(drink)[1] == "test3");
REQUIRE(myTree.getDocuments(drink)[2] == "test5");

cout << "MyTree" << endl;
REQUIRE(myTree.size() == 2);
myTree.prettyPrintTree();
myHandler.saveTree(myTree);

myTree.makeEmpty();
REQUIRE(myTree.size() == 0);
REQUIRE(myTree.isEmpty() == true);
REQUIRE(myTree.contains(fruit) == false);
REQUIRE(myTree.contains(drink) == false);

myHandler.loadTree(myTree);
cout << "Load MyTree" << endl;
myTree.prettyPrintTree();
REQUIRE(myTree.isEmpty() == false);
REQUIRE(myTree.contains(fruit) == true);
REQUIRE(myTree.contains(drink) == true);

AVLTree<string> newTree; // create second tree
string shape = "square";
string weather = "rain";
string color = "red";

newTree.insert(shape);
newTree.insert(weather);
newTree.insert(color);

cout << "NewTree" << endl;
newTree.prettyPrintTree();

REQUIRE(newTree.isEmpty() == false);
REQUIRE(newTree.contains(shape) == true);
REQUIRE(newTree.contains(weather) == true);
REQUIRE(newTree.contains(color) == true);
newTree.addDocument(color, "test");
REQUIRE(newTree.getDocuments(color)[0] == "test");

myTree = newTree; // set empty myTree to contents of newTree
REQUIRE(myTree.isEmpty() == false);
REQUIRE(newTree.contains(shape) == true);
REQUIRE(newTree.contains(weather) == true);
REQUIRE(myTree.contains(color) == true);
REQUIRE(myTree.getDocuments(color)[0] == "test");

newTree.makeEmpty();
REQUIRE(newTree.size() == 0);
REQUIRE(myTree.size() == 3);
REQUIRE(newTree.isEmpty() == true);
REQUIRE(myTree.isEmpty() == false);
REQUIRE(newTree.contains(shape) == false);
REQUIRE(myTree.contains(shape) == true);
REQUIRE(myTree.getDocuments(color)[0] == "test");
REQUIRE(newTree.getDocuments(color).empty() == 1);

cout << "MyTree" << endl;
myTree.prettyPrintTree();
cout << "NewTree" << endl;
newTree.prettyPrintTree();

myTree = newTree; // set non-empty myTree to empty newTree
REQUIRE(myTree.isEmpty() == true);
REQUIRE(myTree.contains(shape) == false);
REQUIRE(myTree.contains(weather) == false);
REQUIRE(myTree.contains(color) == false);
REQUIRE(myTree.contains(fruit) == false);

cout << "MyTree" << endl;
myTree.prettyPrintTree();
cout << "NewTree" << endl;
newTree.prettyPrintTree();
}

TEST_CASE("Perform fake search", "[indexHandler]")
{
// Let's perform a simple, fake search
indexHandler* handler = new indexHandler;

// document parser
documentParser parser(handler);
parser.read("../sample_data");
queryEngine engine(handler);
engine.parser = &parser;

string input = "planning -ORG:eu -PERSON:phillip j tyler";
vector<CountingWord> output = engine.query(input);
REQUIRE(output.size() == 1);
REQUIRE(output[0].word == "../sample_data/coll_2/news_0064570.json");

input = "planning ORG:eu -PERSON:phillip j tyler -ORG:space x ";
output = engine.query(input);
REQUIRE(output.size() == 1);
REQUIRE(output[0].word == "../sample_data/coll_1/news_0064567.json");

input = "planning";
output = engine.query(input);
REQUIRE(output.size() == 2);

//TESTING PERSISTENCE

string desiredDirectory = "../savedTree";
// directory for total maps
string mapORGDirectory = desiredDirectory + "/totalORG.txt";
string mapPERSONDirectory = desiredDirectory + "/totalPERSON.txt";
string mapKEYWORDDirectory = desiredDirectory + "/totalKEYWORD.txt";
// directory for article info maps
string titleDirectory = desiredDirectory + "/TITLE.txt";
string authorDirectory = desiredDirectory + "/AUTHOR.txt";
string dateDirectory = desiredDirectory + "/DATE.txt";

vector<string> directories;
directories.push_back(mapORGDirectory);
directories.push_back(mapPERSONDirectory);
directories.push_back(mapKEYWORDDirectory);
directories.push_back(titleDirectory);
directories.push_back(authorDirectory);
directories.push_back(dateDirectory);

// add total hash maps to vector
vector<unordered_map<string, int>> totalMaps;
totalMaps.push_back(handler->totalORGs);
totalMaps.push_back(handler->totalPERSONs);
totalMaps.push_back(handler->totalKEYWORDs);
// add article info hash maps to vector
vector<unordered_map<string, string>> infoMaps;
infoMaps.push_back(handler->titles);
infoMaps.push_back(handler->authors);
infoMaps.push_back(handler->dates);

handler->saveMaps(totalMaps, infoMaps, directories);


indexHandler* newhandler = new indexHandler;
// add total hash maps to vector
vector<unordered_map<string, int>*> totalMapsP;
totalMapsP.push_back(& newhandler->totalORGs);
totalMapsP.push_back(& newhandler->totalPERSONs);
totalMapsP.push_back(& newhandler->totalKEYWORDs);
// add article info hash maps to vector
vector<unordered_map<string, string>*> infoMapsP;
infoMapsP.push_back(& newhandler->titles);
infoMapsP.push_back(& newhandler->authors);
infoMapsP.push_back(& newhandler->dates);

newhandler->loadMaps(totalMapsP, infoMapsP, directories);

REQUIRE(handler->totalKEYWORDs.size() == newhandler->totalKEYWORDs.size());
REQUIRE(handler->totalPERSONs.size() == newhandler->totalPERSONs.size());
REQUIRE(handler->totalORGs.size() == newhandler->totalORGs.size());
REQUIRE(handler->totalORGs.at("../sample_data/coll_2/news_0064570.json") == newhandler->totalORGs.at("../sample_data/coll_2/news_0064570.json"));
REQUIRE(handler->dates.at("../sample_data/coll_2/news_0064570.json") == newhandler->dates.at("../sample_data/coll_2/news_0064570.json"));
REQUIRE(handler->titles.at("../sample_data/coll_2/news_0064570.json") == newhandler->titles.at("../sample_data/coll_2/news_0064570.json"));

delete handler;
handler = nullptr;
delete newhandler;
newhandler = nullptr;
}
