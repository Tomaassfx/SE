#ifndef DICTPRODUCER_H
#define DICTPRODUCER_H

#include <vector>
#include <map>
#include <string>
#include <set>
#include <utility>

#include "SplitTool.h"
#include "SplitToolCppJieba.h"

using namespace std;

string remove(const string &str);

class DictProducer
{
public:
    DictProducer(const string & filePath, SplitTool * tool);
    void buildEnDict();
    void buildCnDict();
    void createIndex();
    void store();
private:
    vector<string> _files;
    vector<pair<string, int>> _dict;
    map<string, set<int>> _index;
    SplitTool * _cuttor;
};

#endif