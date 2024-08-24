#include "QueryProcessor.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cctype> 
#include <locale> 

// 帮助函数：转义特殊字符（用于 JSON）
string escapeJson(const string& input) {
    // 转义 JSON 中的特殊字符
    string escaped = input;
    // 替换换行符
    size_t pos = 0;
    while ((pos = escaped.find("\n", pos)) != string::npos) {
        escaped.replace(pos, 1, "\\n");
        pos += 2;
    }
    // 替换回车符
    pos = 0;
    while ((pos = escaped.find("\r", pos)) != string::npos) {
        escaped.replace(pos, 1, "\\r");
        pos += 2;
    }
    // 替换双引号
    pos = 0;
    while ((pos = escaped.find("\"", pos)) != string::npos) {
        escaped.replace(pos, 1, "\\\"");
        pos += 2;
    }
    return escaped;
}

// 解析文档内容
void parseDocument(const string& content, int docId, json& docJson) {
    size_t docStart = content.find("<doc>");
    size_t docEnd = content.find("</doc>");
    
    while (docStart != string::npos && docEnd != string::npos) {
        string doc = content.substr(docStart, docEnd - docStart + 6);  // +6 for "</doc>"
        
        size_t idStart = doc.find("<docid>");
        size_t idEnd = doc.find("</docid>");
        if (idStart != string::npos && idEnd != string::npos) {
            int currentDocId = stoi(doc.substr(idStart + 7, idEnd - idStart - 7));
            if (currentDocId == docId) {
                size_t titleStart = doc.find("<title>");
                size_t titleEnd = doc.find("</title>");
                if (titleStart != string::npos && titleEnd != string::npos) {
                    string title = doc.substr(titleStart + 7, titleEnd - titleStart - 7);
                    docJson["title"] = escapeJson(title);
                } else {
                    docJson["title"] = "No title found";
                }

                size_t descStart = doc.find("<description>");
                size_t descEnd = doc.find("</description>");
                if (descStart != string::npos && descEnd != string::npos) {
                    string description = doc.substr(descStart + 13, descEnd - descStart - 13);
                    docJson["summary"] = escapeJson(description);
                } else {
                    docJson["summary"] = "No summary found";
                }

                return;  // 找到目标文档后退出函数
            }
        }

        docStart = content.find("<doc>", docEnd + 6);
        docEnd = content.find("</doc>", docStart);
    }
    
    docJson["title"] = "No title found";
    docJson["summary"] = "No summary found";
}

enum class LanguageType {
    ENGLISH,
    CHINESE,
    MIXED
};

LanguageType detectLanguage(const string &input) {
    int chineseCount = 0;
    int englishCount = 0;

    for (char ch : input) {
        if ((unsigned char)ch > 0x80) {
            // 高位字符是中文字符
            chineseCount++;
        } else if (isalpha(ch)) {
            // 低位的英文字母
            englishCount++;
        }
    }

    if (chineseCount > 0 && englishCount > 0) {
        return LanguageType::MIXED;
    } else if (chineseCount > 0) {
        return LanguageType::CHINESE;
    } else {
        return LanguageType::ENGLISH;
    }
}

string remove(const string &str) {
    string result = str;
    size_t pos;
    while ((pos = result.find("&nbsp;")) != string::npos) {
        result.erase(pos, 6); // 去掉 &nbsp; 的长度为 6
    }
    return result;
}

const char* const DICT_PATH = "../conf/dict/jieba.dict.utf8";
const char* const HMM_PATH = "../conf/dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "../conf/dict/user.dict.utf8";
const char* const IDF_PATH = "../conf/dict/idf.utf8";
const char* const STOP_WORD_PATH = "../conf/dict/stop_words.utf8";
  
// 构造函数
QueryProcessor::QueryProcessor(const string &cnStopWordsPath, const string &enStopWordsPath, 
                               const string &invertedIndexFilePath, const string &offsetLibFilePath)
    : jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH) 
{
    loadStopWords(cnStopWordsPath, cnStopWords);
    loadStopWords(enStopWordsPath, enStopWords);
    loadInvertedIndex(invertedIndexFilePath);
    loadOffsets(offsetLibFilePath);
}

void QueryProcessor::loadInvertedIndex(const string &filename) {
    ifstream indexFile(filename);
    if (indexFile.is_open()) {
        string line;

        // 读取总文档数
        getline(indexFile, line);
        int totalDocs = stoi(line);
        _totalDocs = totalDocs; //后面算TF-IDF用得上

        while (getline(indexFile, line)) {
            string word = line;  // 读取词条

            // 读取文档数量
            getline(indexFile, line);
            int docCount = stoi(line);

            set<pair<int, double>> docSet;

            // 读取文档 ID 和权重
            for (int i = 0; i < docCount; ++i) {
                getline(indexFile, line);
                stringstream ss(line);
                int docId;
                double weight;
                ss >> docId >> weight;
                docSet.emplace(docId, weight);
            }

            // 将词条及其对应的文档集合插入到倒排索引中
            invertedIndex[word] = docSet;
        }
    } else {
        cerr << "Failed to open inverted index file: " << filename << "\n";
    }
}

void QueryProcessor::loadOffsets(const string &filename) {
    ifstream offsetFile(filename);
    if (offsetFile.is_open()) {
        string line;
        while (getline(offsetFile, line)) {
            stringstream ss(line);
            int docId;
            int offset;
            int length;
            ss >> docId >> offset >> length;
            docOffsets[docId] = to_string(offset) + " " + to_string(length);
        }
    } else {
        cerr << "Failed to open offset library file: " << filename << "\n";
    }
}

// 加载停用词
void QueryProcessor::loadStopWords(const string &filePath, set<string> &stopWords) {
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

// 分词与去除停用词
vector<string> QueryProcessor::tokenize(const string &text) {
    vector<string> _keywords;
    vector<string> chineseParts;
    string processedWord;

    LanguageType langType = detectLanguage(text);

    if (langType == LanguageType::ENGLISH) {
        // 处理英文文本
        for (char ch : text) {
            if (isalpha(ch)) {
                // 转换为小写并添加到处理后的字符串中
                processedWord += tolower(ch);
            } else if (isspace(ch)) {
                // 保留空格
                processedWord += ch;
            }
            // 其他字符（标点符号等）将被忽略
        }

        istringstream iss(processedWord);
        string keyword;
        while (iss >> keyword) {
            _keywords.push_back(keyword);
        }
    } else if (langType == LanguageType::CHINESE) {
        // 处理中文文本
        jieba.Cut(text, _keywords);
    } else if (langType == LanguageType::MIXED) {
        // 处理中英文混合文本
        for (size_t pos = 0; pos < text.length();) {
            unsigned char ch = static_cast<unsigned char>(text[pos]);
            size_t charLen;

            // 根据首字节确定字符的实际长度
            if (ch < 0x80) {
                charLen = 1;
            } else if ((ch & 0xE0) == 0xC0) {
                charLen = 2;
            } else if ((ch & 0xF0) == 0xE0) {
                charLen = 3;
            } else if ((ch & 0xF8) == 0xF0) {
                charLen = 4;
            } else {
                ++pos; // 无效字符，跳过
                continue;
            }

            string character = text.substr(pos, charLen);
            pos += charLen;

            if (charLen == 1) {
                // 英文字符部分
                if (isalpha(character[0])) {
                    processedWord += tolower(character[0]);
                } else if (isspace(character[0])) {
                    processedWord += ' ';
                }
                // 其他字符（标点符号等）将被忽略
            } else {
                // 中文字符部分
                if (!processedWord.empty()) {
                    istringstream iss(processedWord);
                    string keyword;
                    while (iss >> keyword) {
                        _keywords.push_back(keyword);
                    }
                    processedWord.clear();
                }
                chineseParts.push_back(character);
            }
        }

        // 处理剩余的英文字符
        if (!processedWord.empty()) {
            istringstream iss(processedWord);
            string keyword;
            while (iss >> keyword) {
                _keywords.push_back(keyword);
            }
        }

        // 处理中文字符
        if (!chineseParts.empty()) {
            string combinedChinese;
            for (const auto &part : chineseParts) {
                combinedChinese += part;
            }
            vector<string> chineseWords;
            jieba.Cut(combinedChinese, chineseWords);
            _keywords.insert(_keywords.end(), chineseWords.begin(), chineseWords.end());
        }
    }

    // 去除停用词
    vector<string> result;
    for (const auto &word : _keywords) {
        if (cnStopWords.find(word) == cnStopWords.end() && 
            enStopWords.find(word) == enStopWords.end()) {
            result.push_back(word);
        }
    }

    return result;
}

// 计算TF-IDF
map<string, double> QueryProcessor::calculateTFIDF(const vector<string> &terms) {
    map<string, double> tfidfMap;

    // 计算词频 TF
    map<string, int> termFreq;
    for (const auto &term : terms) {
        termFreq[term]++;
    }

    // 计算逆文档频率 IDF
    map<string, double> idfMap;
    for (const auto &term : termFreq) {
        const string &termStr = term.first;
        int df = 0;

        // 查找包含该词条的文档数
        auto it = invertedIndex.find(termStr);
        if (it != invertedIndex.end()) {
            df = it->second.size();
        }

        // 计算 IDF
        double idf = (df > 0) ? log(static_cast<double>(_totalDocs) / df) : 0;
        idfMap[termStr] = idf;
    }

    // 计算 TF-IDF
    for (const auto &term : termFreq) {
        const string &termStr = term.first;
        int tf = term.second;
        double idf = idfMap[termStr];
        double tfidf = tf * idf;
        tfidfMap[termStr] = tfidf;
    }

    return tfidfMap;
}

// 查询包含关键词的文档
vector<pair<int, double>> QueryProcessor::searchDocuments(const vector<string> &terms) {
    vector<pair<int, double>> results;
    set<int> docIds;

    // 先收集所有包含查询词的文档 ID
    for (const auto &term : terms) {
        auto it = invertedIndex.find(term);
        if (it != invertedIndex.end()) {
            const auto &docSet = it->second;
            for (const auto &docEntry : docSet) {
                docIds.insert(docEntry.first);
            }
        }
    }

    // 计算每个文档的权重总和
    for (int docId : docIds) {
        double weightSum = 0.0;
        for (const auto &term : terms) {
            auto it = invertedIndex.find(term);
            if (it != invertedIndex.end()) {
                // 在 set 中查找包含 docId 的元素
                auto docSet = it->second;
                auto docIt = find_if(docSet.begin(), docSet.end(),
                    [docId](const pair<int, double>& p) { return p.first == docId; });
                if (docIt != docSet.end()) {
                    weightSum += docIt->second;
                }
            }
        }
        results.emplace_back(docId, weightSum);
    }

    return results;
}

// 计算余弦相似度
double QueryProcessor::calculateCosineSimilarity(const map<string, double> &queryTFIDF, const map<string, double> &docTFIDF) {
    double dotProduct = 0.0;
    double queryNorm = 0.0;
    double docNorm = 0.0;

    for (const auto &term : queryTFIDF) {
        const string &termStr = term.first;
        double queryTFIDFVal = term.second;
        double docTFIDFVal = docTFIDF.count(termStr) ? docTFIDF.at(termStr) : 0.0;
        dotProduct += queryTFIDFVal * docTFIDFVal;
        queryNorm += queryTFIDFVal * queryTFIDFVal;
    }

    for (const auto &term : docTFIDF) {
        double docTFIDFVal = term.second;
        docNorm += docTFIDFVal * docTFIDFVal;
    }

    queryNorm = sqrt(queryNorm);
    docNorm = sqrt(docNorm);

    return (queryNorm > 0 && docNorm > 0) ? (dotProduct / (queryNorm * docNorm)) : 0.0;
}

string QueryProcessor::generateJSON(const vector<pair<int, double>>& sortedResults) {
    json resultJson;
    const string webpageContentFile = "../data/deduplicatedWebData.dat";

    ifstream file(webpageContentFile);
    if (!file.is_open()) {
        cerr << "Failed to open webpage content file: " << webpageContentFile << "\n";
        return "{}";
    }

    stringstream buffer;
    buffer << file.rdbuf();
    string fileContent = buffer.str();
    file.close();

    for (const auto& item : sortedResults) {
        int docId = item.first;
        double score = item.second;
        json docJson;
        docJson["docId"] = docId;
        docJson["score"] = score;

        parseDocument(fileContent, docId, docJson);

        resultJson.push_back(docJson);
    }
    return resultJson.dump();
}

// 处理查询请求
string QueryProcessor::processQuery(const string& query) {
    // 对用户输入的查询请求进行分词，并去除停用词
    vector<string> terms = tokenize(query);

    if (terms.empty()) {
        return "{}";  // 如果没有找到任何有效的词语，则返回空的 JSON 对象
    }

    // 计算查询词的 TF-IDF 权重，并组成基准向量
    map<string, double> queryTFIDF = calculateTFIDF(terms);

    // 查找包含查询词的网页
    vector<pair<int, double>> searchResults = searchDocuments(terms);

    // 使用 priority_queue 对结果进行排序
    auto compare = [](const pair<double, int>& lhs, const pair<double, int>& rhs) {
        return lhs.first < rhs.first;  // 按照相似度从高到低排序
    };
    priority_queue<pair<double, int>, vector<pair<double, int>>, decltype(compare)> sortedResults(compare);

    const string webpageContentFile = "../data/deduplicatedWebData.dat";

    for (const auto& [docId, weight] : searchResults) {
        // 获取网页的位置信息
        auto offsetIt = docOffsets.find(docId);
        if (offsetIt != docOffsets.end()) {
            stringstream ss(offsetIt->second);
            int offset, length;
            ss >> offset >> length;

            ifstream file(webpageContentFile);
            if (file.is_open()) {
                file.seekg(offset);
                string docContent(length, '\0');
                file.read(&docContent[0], length);
                file.close();

                vector<string> docTerms = tokenize(docContent);
                map<string, double> docTFIDF = calculateTFIDF(docTerms);
                double cosineSim = calculateCosineSimilarity(queryTFIDF, docTFIDF);

                // 将计算出的相似度和文档 ID 存储到 priority_queue 中
                sortedResults.push({cosineSim, docId});
            } else {
                cerr << "Failed to open webpage content file: " << webpageContentFile << "\n";
            }
        }
    }

    // 将排序结果转为向量，并生成 JSON 字符串
    vector<pair<int, double>> sortedResultsVec;
    while (!sortedResults.empty()) {
        sortedResultsVec.push_back({sortedResults.top().second, sortedResults.top().first});
        sortedResults.pop();
    }

    return generateJSON(sortedResultsVec);
}