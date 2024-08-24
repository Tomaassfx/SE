#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include "Dictionary.h"
#include "KeyRecommander.h"
#include "CandidateResult.h"
#include <wfrest/HttpServer.h>
#include <nlohmann/json.hpp> 
#include <memory>

class RequestHandler {
public:
    RequestHandler(const string &dictFile, const string &indexFile)
    : _candidateresult("", 0, 0) 
    {
        _dict.loadDict(dictFile);
        _dict.loadIndex(indexFile);
    }

    void handleRequest(const string &userInput, wfrest::HttpResp *resp) {
        json recommendations = _candidateresult.getRecommendations(userInput, _dict);
        resp->headers["Content-Type"] = "application/json";
        resp->String(recommendations.dump(4));
    }

private:
    Dictionary _dict;
    CandidateResult _candidateresult;
};

#endif