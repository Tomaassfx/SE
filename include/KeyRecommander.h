#ifndef KEYRECOMMANDER_H
#define KEYRECOMMANDER_H

#include "Dictionary.h"
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class KeyRecommander {
public:
    KeyRecommander(const string &word, Dictionary &dict);
    vector<string> doQuery();

private:
    string _sought;                    // 用户输入的查询词
    vector<string> _keywords;          // 分词后的关键词
    Dictionary &_dict;                 // 字典和索引
};

#endif