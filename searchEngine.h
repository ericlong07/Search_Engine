#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include "searchEngine.h"
#include "indexHandler.h"
#include "queryEngine.h"
#include "documentParser.h"
#include <chrono>
#include <sstream>
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/document.h"
#include <filesystem>
#include <iomanip>

class searchEngine {
private:
    indexHandler* handler;
    documentParser* parser;
    queryEngine* engine;

    void createIndex(string = "NO_INPUT");
    void saveIndex(string = "NO_INPUT");
    void loadIndex(string = "NO_INPUT");
    void performSearch(string = "NO_INPUT");
public:
    searchEngine();
    ~searchEngine();
    void start(int, char *[]);
    void interface();
};

#endif
