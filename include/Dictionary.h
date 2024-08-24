#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "SplitTool.h"
#include "SplitToolCppJieba.h"
#include <map>
#include <vector>
#include <set>
#include <string>
#include <utility> 
#include <algorithm>

using namespace std;

class Dictionary {
public:
    Dictionary();
    void loadDict(const string &dictFile);
    void loadIndex(const string &indexFile);
    int getFrequency(const std::string &word);

    SplitTool * getSplitTool() {
        return _cuttor; 
    }
    friend class KeyRecommander;
private:
    vector<pair<string, int>> _dict;
    map<string, set<int>> _index;
    SplitTool * _cuttor;
};

#endif