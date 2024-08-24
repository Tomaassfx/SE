#include "WebRemoveDuplicates.h"
#include <fstream>
#include <iostream>

Simhasher simhasher("../conf/dictS/jieba.dict.utf8", "../conf/dictS/hmm_model.utf8", 
                    "../conf/dictS/idf.utf8", "../conf/dictS/stop_words.utf8");


WebRemoveDuplicates::WebRemoveDuplicates() {}
WebRemoveDuplicates::~WebRemoveDuplicates() {}

uint64_t WebRemoveDuplicates::make(const string& text, int topN) const {
    uint64_t hashValue = 0;
    simhasher.make(text, topN, hashValue);
    return hashValue;
}

bool WebRemoveDuplicates::isEqual(uint64_t hash1, uint64_t hash2, size_t distance = 3) const {
    return Simhasher::isEqual(hash1, hash2, distance);
}

/// @brief 
/// @param webFilePath 
/// @param offsetFilePath 
void WebRemoveDuplicates::readInfoFromFile(const string& webFilePath, const string& offsetFilePath) {
    ifstream webFile(webFilePath);
    if (!webFile.is_open()) {
        std::cerr << "Failed to open web file.\n";
        return;
    }

    _files.clear();
    _docMap.clear();

    string line;
    stringstream ss;
    while (getline(webFile, line)) {
        if (line.find("<doc>") != string::npos) {
            ss.str("");
            ss << line << "\n";
            while (getline(webFile, line) && line.find("</doc>") == string::npos) {
                ss << line << "\n";
            }
            ss << line; // 添加 </doc>
            
            // 解析文档 ID 和内容
            string docContent = ss.str();
            stringstream docStream(docContent);
            string docLine;
            int docId = -1;
            while (getline(docStream, docLine)) {
                if (docLine.find("<docid>") != string::npos) {
                    size_t startPos = docLine.find('>') + 1;
                    size_t endPos = docLine.find("</docid>");
                    docId = stoi(docLine.substr(startPos, endPos - startPos));
                }
            }
            if (docId != -1) {
                _docMap[docId] = docContent; // 存储文档 ID 和内容
            }
        }
    }
    webFile.close();

    // 读取偏移库内容
    ifstream offsetFile(offsetFilePath);
    if (!offsetFile.is_open()) {
        std::cerr << "Failed to open offset file.\n";
        return;
    }

    int docId, startOffset, length;
    while (offsetFile >> docId >> startOffset >> length) {
        _offsetLib[docId] = {startOffset, length};
    }
    offsetFile.close();
}

void WebRemoveDuplicates::cutRedundantPages() {
    vector<string> uniqueFiles;
    unordered_map<uint64_t, int> uniqueHashes; // 哈希值到文档ID的映射
    unordered_map<uint64_t, string> hashToContent; // 哈希值到文档内容的映射

    for (const auto& [docId, docContent] : _docMap) {
        stringstream ss(docContent);
        string line;
        string description, content;

        while (getline(ss, line)) {
            if (line.find("<description>") != string::npos) {
                size_t startPos = line.find('>') + 1;
                size_t endPos = line.find("</description>");
                description = line.substr(startPos, endPos - startPos);
            } else if (line.find("<content>") != string::npos) {
                size_t startPos = line.find('>') + 1;
                size_t endPos = line.find("</content>");
                content = line.substr(startPos, endPos - startPos);
            }
        }

        string combinedText = description + " " + content;
        uint64_t hashValue = make(combinedText, 5);  // topN 设置为 5

        bool isRedundant = false;

        for (const auto& [existingHash, existingContent] : hashToContent) {
            if (isEqual(hashValue, existingHash, 3)) { // 这里的 3 是允许的哈希距离
                isRedundant = true;
                break;
            }
        }

        if (!isRedundant) {
            uniqueHashes[hashValue] = docId;
            uniqueFiles.push_back(docContent);
            hashToContent[hashValue] = docContent; // 记录内容以供后续比较
        }
    }

    // 更新去重后的网页库
    _files = uniqueFiles;
    _docMap.clear();
    for (const auto& docContent : _files) {
        stringstream ss(docContent);
        string line;
        int docId = -1;
        while (getline(ss, line)) {
            if (line.find("<docid>") != string::npos) {
                size_t startPos = line.find('>') + 1;
                size_t endPos = line.find("</docid>");
                docId = stoi(line.substr(startPos, endPos - startPos));
            }
        }
        if (docId != -1) {
            _docMap[docId] = docContent;
        }
    }

    hashToPageMap.clear();
    for (const auto& [docId, docContent] : _docMap) {
        stringstream ss(docContent);
        string line;
        string description, content;

        while (getline(ss, line)) {
            if (line.find("<description>") != string::npos) {
                size_t startPos = line.find('>') + 1;
                size_t endPos = line.find("</description>");
                description = line.substr(startPos, endPos - startPos);
            } else if (line.find("<content>") != string::npos) {
                size_t startPos = line.find('>') + 1;
                size_t endPos = line.find("</content>");
                content = line.substr(startPos, endPos - startPos);
            }
        }

        uint64_t hashValue = make(description + " " + content, 5);
        hashToPageMap[hashValue] = description; // 或者根据需要存储其他字段
    }
}

void WebRemoveDuplicates::writeBackToFile(const string& webFilePath, const string& offsetFilePath) {
    // 打开网页库文件进行写入
    ofstream webOfs(webFilePath);
    if (!webOfs.is_open()) {
        cerr << "Failed to open web file for writing.\n";
        return;
    }

    // 打开偏移文件进行写入
    ofstream offsetOfs(offsetFilePath);
    if (!offsetOfs.is_open()) {
        cerr << "Failed to open offset file for writing.\n";
        return;
    }

    // 写入网页库文件内容
    webOfs << "<docs>\n";
    for (const auto& [docId, content] : _docMap) {
        // 根据需要解析每个文档的内容，例如提取 title, link, description 等字段
        // 这里假设文档内容包含 <title>, <link>, <description>, <content> 等标签
        webOfs << "<doc>\n";
        webOfs << "    <docid> " << docId << " </docid>\n";

        size_t titleStart = content.find("<title>") + 7;
        size_t titleEnd = content.find("</title>");
        string title = content.substr(titleStart, titleEnd - titleStart);

        size_t linkStart = content.find("<link>") + 6;
        size_t linkEnd = content.find("</link>");
        string link = content.substr(linkStart, linkEnd - linkStart);

        size_t descriptionStart = content.find("<description>") + 13;
        size_t descriptionEnd = content.find("</description>");
        string description = content.substr(descriptionStart, descriptionEnd - descriptionStart);

        size_t contentStart = content.find("<content>") + 9;
        size_t contentEnd = content.find("</content>");
        string mainContent = content.substr(contentStart, contentEnd - contentStart);

        webOfs << "    <title> " << title << " </title>\n";
        webOfs << "    <link> " << link << " </link>\n";
        webOfs << "    <description> " << description << " \n</description>\n";
        webOfs << "    <content> " << mainContent << " \n</content>\n";
        webOfs << "</doc>\n";
    }
    webOfs << "</docs>\n";

    // 写入偏移文件内容
    for (const auto& [docId, offsetInfo] : _offsetLib) {
        offsetOfs << docId << " " << offsetInfo.first << " " << offsetInfo.second << "\n";
    }

    // 关闭文件流
    webOfs.close();
    offsetOfs.close();
}
