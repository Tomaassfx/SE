#include "WebPage.h"

// 辅助函数：去除字符串中的特殊字符，如 &nbsp;
string remove(const string &str) {
    string result = str;
    size_t pos;
    while ((pos = result.find("&nbsp;")) != string::npos) {
        result.erase(pos, 6); // 去掉 &nbsp; 的长度为 6
    }
    return result;
}

WebPage::WebPage(const string &cnStopWordsPath, const string &enStopWordsPath, SplitTool *splitTool)
    : _splitTool(splitTool), _totalDocs(0) {
    loadStopWords(cnStopWordsPath, _cnStopWords);
    loadStopWords(enStopWordsPath, _enStopWords);
}

int WebPage::getDocId(){
    return _docId;
}

string WebPage::getDoc() {
    return _doc;
}

map<string, int> & WebPage::getWordsMap() {
    return _wordMap;
}

const map<string, set<int>>& WebPage::getWordToDocs() const {
    return _wordToDocs;
}

const map<string, int>& WebPage::getWordToDocCount() const {
    return _wordToDocCount;
}

const unordered_map<string, set<pair<int, double>>>& WebPage::getInvertedIndex() const {
    return _invertedIndex;
}

string WebPage::trim(const string &s) {
    // Trim leading and trailing spaces
    size_t start = s.find_first_not_of(" \t\n\r\f\v");
    size_t end = s.find_last_not_of(" \t\n\r\f\v");
    if (start == string::npos || end == string::npos)
        return "";
    return s.substr(start, end - start + 1);
}

void WebPage::parseDoc(const string &doc) {
    _doc = doc;

    auto extractContent = [&](const string &tag) {
        size_t start = _doc.find("<" + tag + ">");
        if (start != string::npos) {
            start += tag.length() + 2;
            size_t end = _doc.find("</" + tag + ">", start);
            if (end != string::npos) {
                string content = _doc.substr(start, end - start);
                content = trim(content);
                if (!content.empty()) {
                    return content;
                }
            }
        }
        return string(""); 
    };

    string docIdStr = extractContent("docid");
    if (!docIdStr.empty()) {
        try {
            _docId = stoi(docIdStr);
            // 只在成功解析时输出
            // cout << "Extracted docid: " << _docId << "\n";
        } catch (const invalid_argument &e) {
            cerr << "Invalid docid format: " << docIdStr << "\n";
        }
    }

    _docTitle = extractContent("title");
    // if (!_docTitle.empty()) {
    //     cout << "Extracted title: " << _docTitle << "\n";
    // }

    _docUrl = extractContent("link");
    // if (!_docUrl.empty()) {
    //     cout << "Extracted link: " << _docUrl << "\n";
    // }

    _docDescription = extractContent("description");
    // if (!_docDescription.empty()) {
    //     cout << "Extracted description: " << _docDescription << "\n";
    // }

    _docContent = extractContent("content");
    // if (!_docContent.empty()) {
    //     cout << "Extracted content: " << _docContent << "\n";
    // }

    calcWordFreq();

    // cout << "Word Map After Parsing Doc: " << "\n";
    // for (const auto& pair : _wordMap) {
    //     cout << "Word: " << pair.first << ", Frequency: " << pair.second << "\n";
    // }
}

void WebPage::calcWordFreq() {
    _wordMap.clear();
    vector<string> words;

    string combinedContent = _docTitle + " " + _docUrl + " " + _docDescription + " " + _docContent;

    if (_splitTool) {
        vector<string> chineseWords = _splitTool->cut(combinedContent);

        //调试
        for (const string& word : words) {
            cout << "Word: " << word << "\n";
        }

        for (const auto &word : chineseWords) {
            if (!word.empty() && word.find_first_not_of(' ') != string::npos) {
                words.push_back(word);
            }
        }
    }

    istringstream contentStream(combinedContent);
    string word;
    while (contentStream >> word) {
        transform(word.begin(), word.end(), word.begin(), ::tolower);
        word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
        if (!word.empty()) {
            words.push_back(word);
        }
    }

    for (const auto &word : words) {
        string cleanedWord = word;
        
        // 去除标点符号
        cleanedWord.erase(remove_if(cleanedWord.begin(), cleanedWord.end(), ::ispunct), cleanedWord.end());
        
        // 检查 cleanedWord 是否为空
        if (cleanedWord.empty()) {
            continue; // 跳过空词
        }

        // 检查 cleanedWord 是否在停用词集合中
        if (_cnStopWords.find(cleanedWord) == _cnStopWords.end() &&
            _enStopWords.find(cleanedWord) == _enStopWords.end()) 
        {
            _wordMap[cleanedWord]++; // 更新词频
        }
    }
}

void WebPage::loadStopWords(const string &filePath, set<string> &stopWords) {
    ifstream stopWordsFile(filePath);
    if (stopWordsFile.is_open()) {
        string stopWord;
        while (getline(stopWordsFile, stopWord)) {
            stopWord = remove(stopWord); // 去除首尾空白字符
            if (!stopWord.empty()) {
                stopWords.insert(stopWord);
            }
        }
        stopWordsFile.close();
    } else {
        cerr << "Failed to open stop words file: " << filePath << "\n";
    }
}

void WebPage::updateWordDocMaps() {
    // 假设 _wordMap 包含当前文档的词频
    for (const auto &wordFreq : _wordMap) {
        const string &word = wordFreq.first;

        // 更新词到文档的映射
        _wordToDocs[word].insert(_docId);
        _wordToDocCount[word] = _wordToDocs[word].size();
    }

    // 将当前文档的词频映射添加到 _wordMaps
    _wordMaps.push_back({_docId, _wordMap});
}

void WebPage::saveInvertedIndex(const string &filename) {
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cerr << "Failed to open file for writing inverted index.\n";
        return;
    }

    // 写入总文档数
    outFile << _totalDocs << "\n";

    // 遍历倒排索引并写入文件
    for (const auto &entry : _invertedIndex) {
        const string &word = entry.first;
        const auto &docSet = entry.second;

        // 写入词
        outFile << word << "\n";

        // 写入文档数量
        outFile << docSet.size() << "\n";

        // 写入每个文档的 ID 和权重
        for (const auto &docEntry : docSet) {
            int docId = docEntry.first;
            double weight = docEntry.second;
            outFile << docId << " " << weight << "\n";
        }
    }

    outFile.close();
    cout << "Inverted index saved to " << filename << "\n";
}

void WebPage::loadInvertedIndex(const string &filename) {
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cerr << "Failed to open file for reading inverted index.\n";
        return;
    }

    // 读取总文档数
    inFile >> _totalDocs;
    inFile.ignore(numeric_limits<streamsize>::max(), '\n'); // 跳过换行符

    _invertedIndex.clear();
    while (inFile.peek() != EOF) {
        string word;
        getline(inFile, word); // 读取词

        size_t docCount;
        inFile >> docCount;
        inFile.ignore(numeric_limits<streamsize>::max(), '\n'); // 跳过换行符

        set<pair<int, double>> docSet;
        for (size_t i = 0; i < docCount; ++i) {
            int docId;
            double weight;
            inFile >> docId >> weight;
            inFile.ignore(numeric_limits<streamsize>::max(), '\n'); // 跳过换行符
            docSet.insert({docId, weight});
        }

        _invertedIndex[word] = docSet;
    }

    inFile.close();
}

void WebPage::processAllPages(const string& processedFilePath) {
    ifstream webFile(processedFilePath);
    if (!webFile.is_open()) {
        cerr << "Failed to open processed web pages file.\n";
        return;
    }

    string doc;
    while (getline(webFile, doc)) {
        parseDoc(doc); // 解析文档内容
        calcWordFreq(); // 计算词频
        updateWordDocMaps(); // 更新词与文档的映射
        _totalDocs++;
    }

    webFile.close();

    // 计算 IDF（逆文档频率）
    unordered_map<string, double> idfMap;
    for (const auto &entry : _wordToDocCount) {
        const string &word = entry.first;
        double idf = 0;
        if (_totalDocs > 0 && entry.second > 0) {
            idf = log(static_cast<double>(_totalDocs) / entry.second) + 1;
        }
        idfMap[word] = idf;
    }

    // 计算每个文档的 TF-IDF
    unordered_map<int, map<string, double>> docTfIdfMap;
    for (const auto &docEntry : _wordMaps) {
        int docId = docEntry.first;
        const map<string, int>& wordMap = docEntry.second;

        for (const auto &wordFreq : wordMap) {
            const string &word = wordFreq.first;
            double tf = static_cast<double>(wordFreq.second) / wordMap.size(); // 使用文档的词频
            double idf = idfMap[word];
            double tfIdf = tf * idf;

            docTfIdfMap[docId][word] = tfIdf;
        }
    }

    // 归一化 TF-IDF 权重
    for (auto &docEntry : docTfIdfMap) {
        int docId = docEntry.first;
        auto &wordMap = docEntry.second;

        double norm = 0.0;
        for (const auto &pair : wordMap) {
            norm += pair.second * pair.second;
        }
        norm = sqrt(norm);

        if (norm > 0) {
            for (auto &pair : wordMap) {
                pair.second /= norm;
            }
        } else {
            for (auto &pair : wordMap) {
                pair.second = 0;
            }
        }
    }

    // 更新倒排索引
    for (const auto &docEntry : docTfIdfMap) {
        int docId = docEntry.first;
        for (const auto &pair : docEntry.second) {
            const string &word = pair.first;
            double weight = pair.second;
            _invertedIndex[word].insert({docId, weight});
        }
    }

    saveInvertedIndex("../data/invertedIndex.dat");
}