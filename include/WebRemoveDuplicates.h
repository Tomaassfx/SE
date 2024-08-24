#ifndef WEBREMOVEDUPLICATES_H
#define WEBREMOVEDUPLICATES_H

#include "simhash1/simhash/Simhasher.hpp"
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;
using namespace simhash;

class WebRemoveDuplicates {
public:
    WebRemoveDuplicates();
    ~WebRemoveDuplicates();

    void readInfoFromFile(const string& webFilePath, const string& offsetFilePath);
    void writeBackToFile(const string& webFilePath, const string& offsetFilePath);
    void cutRedundantPages();
    bool isEqual(uint64_t hash1, uint64_t hash2, size_t distance) const;

private:
    vector<string> _files; // 存储文档内容
    unordered_map<int, pair<int, int>> _offsetLib; // 存储文档偏移信息
    map<int, string> _docMap; // 存储文档ID和内容

    uint64_t make(const string& text, int topN) const;
    unordered_map<uint64_t, string> hashToPageMap;
};

#endif