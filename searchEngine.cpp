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

using namespace std;

searchEngine::searchEngine() {
    handler = new indexHandler;
    parser = new documentParser(handler);
    engine = new queryEngine(handler);
}

searchEngine::~searchEngine() {
    delete handler;
    handler = nullptr;
    delete parser;
    parser = nullptr;
    delete engine;
    engine = nullptr;
}

void searchEngine::start(int argc, char *argv[]) {
    //./supersearch ui
    //./supersearch index <directory to create from> <directory to save to>
    //./supersearch index <directory to load from> <query>

    if (argc == 1 || strcmp( argv[1], "ui") == 0) {
        interface();
        return;
    }
    if (strcmp( argv[1], "index") == 0) {
        createIndex(argv[2]);
        saveIndex(argv[3]);
        return;
    }
    if (strcmp( argv[1], "query") == 0) {
        loadIndex(argv[2]);

        string query;
        for (int i = 3; i < argc ; i++ ) {
            query += " ";
            query += argv[i];
        }
        cout << query << endl;
        performSearch(query);
        return;
    }

    return;
}

void searchEngine::interface() {
    //must allow for following options:
    //  createIndex from document directory, return success or fail
    //  saveIndex to file, return success or fail
    //  loadIndex from file, return success or fail
    //  enter query, return results
    //allow for statistics at every step

    string response = "";
    cout << "Hello and welcome to Bing II!" << endl;
    while(true) {
        cout << "Please select an option below..." << endl;
        cout << "   1. Create index from directory with documents" << endl;
        cout << "   2. Enter a search query" << endl;
        cout << "   3. Write index to file" << endl;
        cout << "   4. Read index from file" << endl;
        cout << "   5. Exit program" << endl;

        cin >> response;
        int intResponse;
        try
        {
            intResponse = stoi(response);
        }
        catch (...)
        {
            cout << endl << "Invalid response, please try again!" << endl << endl;
            response = "";
            continue;
        }

        switch(intResponse){
            case 1:
                cout << endl << "You selected \"Create index from directory with documents\"" << endl;
                createIndex();
                cout << "   Indexed " << handler->totalKEYWORDs.size() << " articles" << endl;
                cout << "   Indexed " << handler->treeKEYWORD.size() << " words" << endl << endl;
                break;
            case 2:
                cout << endl << "You selected \"Enter a search query\"" << endl;
                performSearch();
                cout << "   Searched " << handler->totalKEYWORDs.size() << " articles" << endl;
                cout << "   Searched " << handler->treeKEYWORD.size() << " words" << endl << endl;
                break;
            case 3:
                cout << endl << "You selected \"Write index to file\"" << endl;
                saveIndex();
                cout << "   Saved " << handler->totalKEYWORDs.size() << " articles" << endl;
                cout << "   Saved " << handler->treeKEYWORD.size() << " words" << endl << endl;
                break;
            case 4:
                cout << endl << "You selected \"Read index from file\"" << endl;
                loadIndex();
                cout << "   Loaded " << handler->totalKEYWORDs.size() << " articles" << endl;
                cout << "   Loaded " << handler->treeKEYWORD.size() << " words" << endl << endl;
                break;
            case 5:
                cout << endl << "Thank you for searching with us!" << endl << endl;
                return;
            default:
                cout << endl << "Invalid response, please try again!" << endl << endl;
                response = "";
                break;
        };
    }
}

void searchEngine::createIndex(string input) {
    string desiredDirectory;
    if (input == "NO_INPUT") {
        cout << "   Please enter a folder (i.e. \"../sample_data\"):" << endl;

        cin >> desiredDirectory;
    } else { desiredDirectory = input; }

    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::chrono::duration<double> elapsed_seconds = end - start;
    start = std::chrono::system_clock::now();
    parser->read(desiredDirectory);
    end = std::chrono::system_clock::now();
    elapsed_seconds = end - start;
    cout << "   Time elapsed: " << elapsed_seconds.count() << "s" << endl;
}

void searchEngine::performSearch(string input2) {
    string input;
    if (input2 == "NO_INPUT") {
        cout << "   Please enter a query (i.e. \"planning governmnent  spending   ORG:eu PERSON:phillip j tyler -ORG:space x   \"):" << endl;

        cin.ignore();
        getline(cin, input);
    } else { input = input2; }

    engine->parser = parser;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::chrono::duration<double> elapsed_seconds = end - start;
    start = std::chrono::system_clock::now();
    vector<CountingWord> output = engine->query(input);
    vector<CountingWord> rankedOutputs;
    double maximumRelevancy = 0;
    double oldMaximumRelevancy = 10;

    // Ordering outputs to get up to 15 top relevant articles
    if (output.size() > 0) {
        while(rankedOutputs.size() < 15 && output.size() > 0) {
            maximumRelevancy = 0;

            for (size_t i = 0 ; i < output.size() ; i++) {
                if (output[i].frequency > maximumRelevancy && output[i].frequency < oldMaximumRelevancy) {
                    maximumRelevancy = output[i].frequency;

                    bool theSame = false;
                    for (size_t j = 0 ; j < rankedOutputs.size() ; j++) {
                        if (handler->titles.at(rankedOutputs[j].word) == handler->titles.at(output[i].word) && handler->dates.at(rankedOutputs[j].word) == handler->dates.at(output[i].word)) {
                            theSame = true;
                        }
                    }
                    if (!theSame) { rankedOutputs.push_back(output[i]); }


                    output.erase(output.begin() + i);
                }
            }

            oldMaximumRelevancy = maximumRelevancy;
        }
    }
    end = std::chrono::system_clock::now();
    elapsed_seconds = end - start;

    if (rankedOutputs.size() > 0) {
        cout << endl << "Top Results:" << endl;
        for (int i = 0 ; i < min(15,(int) rankedOutputs.size()) ; i++) {
            cout << (i + 1) << ") " << handler->titles.at(rankedOutputs[i].word) << endl;
            cout << "   ~ " << handler->dates.at(rankedOutputs[i].word) << " by " << handler->authors.at(rankedOutputs[i].word) << endl;
            cout << "   ~ " << rankedOutputs[i].word << endl;
        }
        cout << endl;

        string response;
        int intResponse;
        while (true) {
            cout << "Select the ranking number of the article you wish to read, or enter 0 to continue:" << endl;
            cin >> response;
            try
            {
                intResponse = stoi(response);
            }
            catch (...)
            {
                cout << endl << "Invalid response, please try again!" << endl << endl;
                response = "";
                continue;
            }

            if (intResponse == 0) {
                cout << endl;
                break;
            } else if (intResponse <= 0 || intResponse > (int) rankedOutputs.size()) {
                cout << endl << "Invalid response, please try again!" << endl << endl;
                continue;
            }

            cout << endl << "Article: " << handler->titles.at(rankedOutputs[intResponse-1].word) << endl;
            cout << "   ~ " << handler->dates.at(rankedOutputs[intResponse-1].word) << " by " << handler->authors.at(rankedOutputs[intResponse-1].word) << endl;

            // Reading in the file text
            fstream file (rankedOutputs[intResponse-1].word, ios::in);
            if (!file.is_open())
            {
                cerr << "Can't open file: " << rankedOutputs[intResponse-1].word << endl;
                break;
            }
            // Create a RapidJSON IStreamWrapper using the file input stream above.
            rapidjson::IStreamWrapper isw(file);

            // Create a RapidJSON Document object and use it to parse the IStreamWrapper object above.
            rapidjson::Document doc;
            doc.ParseStream(isw);

            auto text = doc["text"].GetString();

            cout << text << endl << endl;

            break;
        }
    } else {
        cout << endl << "No results found :(" << endl << endl;
    }

    cout << "   Time elapsed: " << elapsed_seconds.count() << "s" << endl;
}

void searchEngine::saveIndex(string input) {
    string desiredDirectory;
    if (input == "NO_INPUT") {
        cout << "   Enter a folder to save index to (i.e. \"../savedTree\" will save \"../savedTree/treeORGS.txt\", \"../savedTree/totalPERSON.txt\", etc.):" << endl;

        cin >> desiredDirectory;
    } else { desiredDirectory = input; }

    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::chrono::duration<double> elapsed_seconds = end - start;
    start = std::chrono::system_clock::now();
    // directory for AVL trees
    string treeORGDirectory = desiredDirectory + "/treeORGs.txt";
    string treePERSONDirectory = desiredDirectory + "/treePERSONs.txt";
    string treeKEYWORDDirectory = desiredDirectory + "/treeKEYWORDs.txt";
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

    // save trees
    handler->saveTree(handler->treeORG, treeORGDirectory);
    cout << "   Successfully saved " << treeORGDirectory << endl;

    handler->saveTree(handler->treePERSON, treePERSONDirectory);
    cout << "   Successfully saved " << treePERSONDirectory << endl;

    handler->saveTree(handler->treeKEYWORD, treeKEYWORDDirectory);
    cout << "   Successfully saved " << treeKEYWORDDirectory << endl;

    // save hash maps
    handler->saveMaps(totalMaps, infoMaps, directories);
    cout << "   Successfully saved maps: " << endl;
    for (int i = 0 ; i < (int) directories.size() ; i++) {
        cout << "     " << directories[i] << endl;
    }
    cout << endl;
    end = std::chrono::system_clock::now();
    elapsed_seconds = end - start;
    cout << "   Time elapsed: " << elapsed_seconds.count() << "s" << endl;

}

void searchEngine::loadIndex(string input) {
    string desiredDirectory;
    if (input == "NO_INPUT") {
        cout << "   Enter a folder to load index from (i.e. \"../savedTree\" will load \"../savedTree/treeORGS.txt\", \"../savedTree/totalPERSON.txt\", etc.):" << endl;

        cin >> desiredDirectory;
    } else { desiredDirectory = input; }

    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::chrono::duration<double> elapsed_seconds = end - start;
    start = std::chrono::system_clock::now();
    // directory for AVL trees
    string treeORGDirectory = desiredDirectory + "/treeORGs.txt";
    string treePERSONDirectory = desiredDirectory + "/treePERSONs.txt";
    string treeKEYWORDDirectory = desiredDirectory + "/treeKEYWORDs.txt";
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
    vector<unordered_map<string, int>*> totalMaps;
    totalMaps.push_back(& handler->totalORGs);
    totalMaps.push_back(& handler->totalPERSONs);
    totalMaps.push_back(& handler->totalKEYWORDs);
    // add article info hash maps to vector
    vector<unordered_map<string, string>*> infoMaps;
    infoMaps.push_back(& handler->titles);
    infoMaps.push_back(& handler->authors);
    infoMaps.push_back(& handler->dates);

    // save trees
    handler->loadTree(handler->treeORG, treeORGDirectory);
    cout << "   Successfully loaded " << treeORGDirectory << endl;

    handler->loadTree(handler->treePERSON, treePERSONDirectory);
    cout << "   Successfully loaded " << treePERSONDirectory << endl;

    handler->loadTree(handler->treeKEYWORD, treeKEYWORDDirectory);
    cout << "   Successfully loaded " << treeKEYWORDDirectory << endl;

    // save hash maps
    handler->loadMaps(totalMaps, infoMaps, directories);
    cout << "   Successfully loaded maps: " << endl;
    for (int i = 0 ; i < (int) directories.size() ; i++) {
        cout << "     " << directories[i] << endl;
    }
    cout << endl;
    end = std::chrono::system_clock::now();
    elapsed_seconds = end - start;
    cout << "   Time elapsed: " << elapsed_seconds.count() << "s" << endl;

}
