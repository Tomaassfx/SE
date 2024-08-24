#ifndef SPLITTOOL_H
#define SPLITTOOL_H

#include <vector>
#include <string>

using std::string;
using std::vector;

class SplitTool
{
public:
    virtual vector<string> cut(const string & input) = 0;
};

#endif