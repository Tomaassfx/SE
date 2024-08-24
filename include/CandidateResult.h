#ifndef CANDIDATERESULT_H
#define CANDIDATERESULT_H


#include "Dictionary.h"
#include "KeyRecommander.h"
#include <queue>
#include <vector>
#include <string>
#include <../include/nlohmann/json.hpp> 

using namespace std;
using json = nlohmann::json;

class CandidateResult {
public:
    CandidateResult(const string &w, int f, int d) : word(w), freq(f), dist(d) {}
    int editDistance(const string & lhs, const string &rhs);
    json getRecommendations(const string &input, Dictionary &dict);

    bool operator<(const CandidateResult& other) const {
        if (dist == other.dist) {
            return freq < other.freq;  // 热度大的优先
        }
        return dist > other.dist;  // 编辑距离小的优先
    }

private:
    string word;
    int freq;
    int dist;
};

#endif