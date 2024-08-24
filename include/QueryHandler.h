#ifndef QUERY_HANDLER_H
#define QUERY_HANDLER_H

#include "MyLRUCache.h"
#include "QueryProcessor.h"

#include <string>
#include <nlohmann/json.hpp>
#include <workflow/WFFacilities.h>
#include <wfrest/HttpServer.h>
#include <workflow/WFTaskFactory.h>
#include <workflow/WFGlobal.h>

using namespace std;

class QueryHandler {
public:
    QueryHandler(const string &cnStopWordsPath, const string &enStopWordsPath, 
                 const string &invertedIndexFilePath, const string &offsetLibFilePath);

    void handleRequest(const string &userInput, wfrest::HttpResp *resp, SeriesWork *series);

private:
    QueryProcessor processor;
};


// class QueryHandler {
// public:
//     QueryHandler(const string &cnStopWordsPath, const string &enStopWordsPath, 
//                  const string &invertedIndexFilePath, const string &offsetLibFilePath,
//                  size_t lruCapacity, size_t poolCapacity);

//     void handleRequest(const string &userInput, wfrest::HttpResp *resp, SeriesWork *series);

// private:
//     QueryProcessor processor;
//     WFResourcePool lruCachePool;
// };

#endif