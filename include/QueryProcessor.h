#ifndef QUERY_PROCESSOR_H
#define QUERY_PROCESSOR_H
#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <queue>
// #include <tinyxml2.h>
#include <nlohmann/json.hpp> 

#include "cppjieba/Jieba.hpp"

using namespace std;
using json = nlohmann::json;
// using namespace tinyxml2;

class QueryProcessor {
public:
    QueryProcessor(const string &cnStopWordsPath, const string &enStopWordsPath, 
                   const string &invertedIndexFilePath, const string &offsetLibFilePath);

    string processQuery(const string &query);  // 处理查询请求，并返回JSON结果

private:
    void loadStopWords(const string &filePath, set<string> &stopWordsSet);
    vector<string> tokenize(const string &text);  // 分词并去除停用词
    map<string, double> calculateTFIDF(const vector<string> &terms);  // 计算TF-IDF
    vector<pair<int, double>> searchDocuments(const vector<string> &terms);  // 倒排索引查询
    double calculateCosineSimilarity(const map<string, double> &queryTFIDF, 
                                     const map<string, double> &docTFIDF);  // 计算余弦相似度
    string generateJSON(const vector<pair<int, double>> &sortedResults);  // 生成JSON结果

    cppjieba::Jieba jieba;  // 用于中文分词
    set<string> cnStopWords;  // 中文停用词集合
    set<string> enStopWords;  // 英文停用词集合
    unordered_map<string, set<pair<int, double>>> invertedIndex;  // 倒排索引
    unordered_map<int, string> docOffsets;  // 网页偏移库
    int _totalDocs;  // 总文档数

    void loadInvertedIndex(const string &filename);  // 加载倒排索引
    void loadOffsets(const string &filename);  // 加载网页偏移库
};

#endif