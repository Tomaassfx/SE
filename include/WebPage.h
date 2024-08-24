#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <cmath>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include "SplitTool.h"
#include "SplitToolCppJieba.h"

using namespace std;

class WebPage {
public:
    WebPage(const string &cnStopWordsPath, const string &enStopWordsPath, SplitTool *splitTool);
    int getDocId();                     // 获取文档的 docid
    string getDoc();                    // 获取文档
    void processAllPages(const string& processedFilePath); // 处理所有去重后的网页库
    map<string, int> &getWordsMap();                       // 获取文档的词频统计 map
    const map<string, set<int>>& getWordToDocs() const;    // 获取词到文档的映射
    const map<string, int>& getWordToDocCount() const;     // 获取包含该词的文档数量
    const unordered_map<string, set<pair<int, double>>>& getInvertedIndex() const; // 获取倒排索引

private:
    void loadStopWords(const string &filePath, set<string> &stopWordsSet);
    void calcWordFreq();        // 计算词频
    void updateWordDocMaps();   // 更新词与文档的映射
    void parseDoc(const string &doc); // 解析文档
    void saveInvertedIndex(const string &filename); // 保存倒排索引到文件
    void loadInvertedIndex(const string &filename); // 从文件加载倒排索引

private:
    string _doc;                // 整篇文档，包含 xml 在内
    int _docId;                 // 文档 id
    string _docTitle;           // 文档标题
    string _docUrl;             // 文档 URL
    string _docContent;         // 文档内容
    string _docDescription;     // 文档描述
    map<string, int> _wordMap;  // 保存每篇文档的所有词语和词频，不包括停用词

    vector<pair<int, map<string, int>>> _wordMaps;  // 保存每篇文章的 _wordMap
    
    set<string> _cnStopWords;   // 中文停用词集合
    set<string> _enStopWords;   // 英文停用词集合
    SplitTool *_splitTool;      // 中文分词
    int _totalDocs;             // 总文档数

    string trim(const string &s);
    map<string, set<int>> _wordToDocs;   // 记录每个词出现在哪些文档中
    map<string, int> _wordToDocCount;    // 记录包含该词的文档数量
    unordered_map<string, set<pair<int, double>>> _invertedIndex; // 倒排索引
};

#endif


