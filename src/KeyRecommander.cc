#include "KeyRecommander.h"
#include <locale>
#include <codecvt>

enum class LanguageType {
    ENGLISH,
    CHINESE,
    MIXED
};

LanguageType detectLanguage0(const string &input) {
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

KeyRecommander::KeyRecommander(const string &word, Dictionary &dict)
: _sought(word), _dict(dict) 
{
    LanguageType langType = detectLanguage0(word);

    if (langType == LanguageType::ENGLISH) {
        // 将大写转小写，移除标点符号和无关字符
        string processedWord;
        for (char ch : word) {
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
            // 英文分词
            _keywords.push_back(keyword);
        }
    } else if (langType == LanguageType::CHINESE) {
        // 中文分词
        _keywords = _dict.getSplitTool()->cut(word);
    } else if (langType == LanguageType::MIXED) {
        string processedWord;
        vector<string> chineseParts;

        for (size_t pos = 0; pos < word.length();) {
            unsigned char ch = static_cast<unsigned char>(word[pos]);
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

            string character = word.substr(pos, charLen);
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

        if (!processedWord.empty()) {
            istringstream iss(processedWord);
            string keyword;
            while (iss >> keyword) {
                _keywords.push_back(keyword);
            }
        }

        if (!chineseParts.empty()) {
            string combinedChinese;
            for (const auto &part : chineseParts) {
                combinedChinese += part;
            }
            vector<string> chineseWords = _dict.getSplitTool()->cut(combinedChinese);
            _keywords.insert(_keywords.end(), chineseWords.begin(), chineseWords.end());
        }
    }
}

vector<string> KeyRecommander::doQuery() {
    vector<string> results;
    for (const auto &key : _keywords) {
        auto it = _dict._index.find(key);
        if (it != _dict._index.end()) {
            // 利用索引找到对应的词汇
            for (int idx : it->second) {
                const auto &wordFreqPair = _dict._dict[idx];
                results.push_back(wordFreqPair.first);
            }
        }
    }

    return results;
}
