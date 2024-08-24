#ifndef SPLITTOOL_CPPJIEBA_H
#define SPLITTOOL_CPPJIEBA_H

#include "cppjieba/Jieba.hpp"
#include "SplitTool.h"

#include <vector>
#include <string>

extern cppjieba::Jieba jieba;

using namespace std;

class SplitToolCppJieba : public SplitTool
{
public:
    virtual vector<string> cut(const string & input) override;
};

#endif