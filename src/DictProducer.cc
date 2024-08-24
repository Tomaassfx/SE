#include "DictProducer.h"

#include <utility> 
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cctype>
#include <unordered_set>
#include <algorithm>


using namespace std;
namespace fs = std::filesystem;  // C17

/// @brief 
/// @param str 
/// @return 
string remove(const string &str) {
    auto start = str.find_first_not_of(" \t\r\n");
    auto end = str.find_last_not_of(" \t\r\n");
    return (start == string::npos || end == string::npos) ? "" : str.substr(start, end - start + 1);
}

/// @brief 
/// @param filePath 
/// @param tool 
DictProducer::DictProducer(const string & filePath, SplitTool * tool)
: _dict(), _index(), _cuttor(tool)
{
    //读目录/文件
    fs::path path(filePath);

    try {
        if (fs::is_directory(path)) {
            // 处理目录
            for (const auto& entry : fs::directory_iterator(path)) {
                if (fs::is_regular_file(entry.status())) {
                    _files.push_back(entry.path().string());
                }
            }
        } else if (fs::is_regular_file(path)) {
            // 处理单个文件
            _files.push_back(path.string());
        } else {
            cerr << "Invalid path: " << path << '\n';
        }
    } catch (const fs::filesystem_error& e) {
        cerr << "Filesystem error: " << e.what() << '\n';
    } catch (const std::exception& e) {
        cerr << "Error: " << e.what() << '\n';
    }
}

void DictProducer::buildEnDict() {
    map<string, int> wordCount;
    unordered_set<string> stopWords;

    // 读取停用词文件
    ifstream stopWordsFile("../conf/yuliao/stop_words_eng.txt");
    if (stopWordsFile.is_open()) {
        string stopWord;
        while (stopWordsFile >> stopWord) {
            stopWord = remove(stopWord);
            stopWords.insert(stopWord);
        }
        stopWordsFile.close();
    } else {
        cerr << "Failed to open stop words file.\n";
    }

    // 处理每个文件
    for (const auto& filePath : _files) {
        ifstream file(filePath);
        if (!file.is_open()) {
            cerr << "Failed to open file: " << filePath << "\n";
            continue;
        }

        string line;
        while (getline(file, line)) {
            string lowerInput = line;
            transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);

            string processedInput;
            for (char c : lowerInput) {
                if (ispunct(c) || c == '\r' || c == '\n') {
                    processedInput += ' ';
                } else {
                    processedInput += c;
                }
            }

            istringstream iss(processedInput);
            string word;
            while (iss >> word) {
                if (!word.empty() && stopWords.find(word) == stopWords.end()) { // 过滤停用词
                    wordCount[word]++;
                }
            }
        }
        file.close();
    }

    _dict.clear();
    for (const auto& pair : wordCount) {
        _dict.emplace_back(pair.first, pair.second);
    }
}

void DictProducer::buildCnDict() {
    map<string, int> wordCount;
    unordered_set<string> stopWords;

    // 读取停用词文件
    ifstream stopWordsFile("../conf/yuliao/stop_words_zh.txt");
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
        cerr << "Failed to open stop words file.\n";
    }

    // 处理每个文件
    for (const auto &filePath : _files) {
        ifstream file(filePath);
        if (!file.is_open()) {
            cerr << "Failed to open file: " << filePath << "\n";
            continue;
        }

        string line;
        while (getline(file, line)) {
            vector<string> words = _cuttor->cut(line);
            
            for (const auto &word : words) {
                if (!word.empty() && stopWords.find(word) == stopWords.end()) { // 过滤停用词
                    wordCount[word]++;
                }
            }
        }
        file.close();
    }

    _dict.clear();
    for (const auto &pair : wordCount) {
        _dict.emplace_back(pair.first, pair.second);
    }
}

void DictProducer::createIndex() {
    _index.clear();

    for (size_t i = 0; i < _dict.size(); ++i) {
        const string &word = _dict[i].first;
        size_t len = word.length();
        size_t pos = 0;

        while (pos < len) {
            unsigned char ch = static_cast<unsigned char>(word[pos]);
            // 根据首字节确定字符的实际长度
            size_t charLen;
            if (ch < 0x80) {
                charLen = 1; // 英文字母、数字等
            } else if ((ch & 0xE0) == 0xC0) {
                charLen = 2; // 2 字节字符
            } else if ((ch & 0xF0) == 0xE0) {
                charLen = 3; // 3 字节字符，中文汉字
            } else if ((ch & 0xF8) == 0xF0) {
                charLen = 4; // 4 字节字符
            } else {
                ++pos;
                continue;
            }

            // 处理字符并插入索引
            string character = word.substr(pos, charLen);
            _index[character].insert(i);
            pos += charLen; // 移动到下一个字符
        }
    }
}

void DictProducer::store() {
    // 追加模式打开词典文件
    // ofstream dictFile("../data/dict.dat", std::ios::app);
    ofstream dictFile("../data/dict.dat");
    if (!dictFile.is_open()) {
        cerr << "Failed to open ../data/dict.dat for writing" << "\n";
        return;
    }

    // 保存词典数据
    for (const auto& readIn : _dict) {
        dictFile << readIn.first << ":" << readIn.second << "\n";
    }
    dictFile.close();

    // 追加模式打开索引文件
    // ofstream indexFile("../data/index.dat", std::ios::app);
    ofstream indexFile("../data/index.dat");
    if (!indexFile.is_open()) {
        cerr << "Failed to open ../data/index.dat for writing" << "\n";
        return;
    }

    // 保存索引数据
    for (const auto& readIn : _index) {
        indexFile << readIn.first << ":";
        for (const auto& fileId : readIn.second) {
            indexFile << fileId << " ";
        }
        indexFile << "\n";
    }
    indexFile.close();
}



int main() {
    SplitTool * tool;
    // DictProducer dict1("../conf/yuliao/english.txt", tool);
    // dict1.buildEnDict(); 
    // dict1.createIndex();
    // dict1.store(); 

    SplitToolCppJieba toolJieba;
    DictProducer dict2("../conf/yuliao/art", &toolJieba);
    dict2.buildCnDict(); 
    dict2.createIndex();
    dict2.store();
}



//C11读取目录下所有文件
// #include <iostream>
// #include <dirent.h>
// #include <sys/types.h>

// void listFiles(const string& directory) {
//     DIR* dir = opendir(directory.c_str());
//     if (dir == nullptr) {
//         cerr << "Directory not found: " << directory << "\n";
//         return;
//     }

//     struct dirent* entity;
//     while ((entity = readdir(dir)) != nullptr) {
//         const string fileOrDirName = entity->d_name;
//         if (fileOrDirName != "." && fileOrDirName != "..") {
//             cout << fileOrDirName << "\n";
//         }
//     }
//     closedir(dir);
// }

// int main() {
//     listFiles("../conf/yuliao/art");
//     return 0;
// }