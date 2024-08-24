#include <iostream>
#include <fstream>
#include <map>
#include <utility> // for std::pair
#include <string>
#include "WebPage.h"// 包含 WebPage 类定义
#include <set>


using namespace std;

int main() {
    // 假设 cnStopWordsPath, enStopWordsPath 和 splitTool 已经初始化
    string cnStopWordsPath = "../conf/yuliao/stop_words_zh.txt";
    string enStopWordsPath = "../conf/yuliao/stop_words_eng.txt";
    SplitToolCppJieba toolJieba;
    WebPage webPage(cnStopWordsPath, enStopWordsPath, &toolJieba);

    // 处理测试文件
    // string processedFilePath = "../data/test.dat";
    string processedFilePath = "../data/deduplicatedWebData.dat";
    webPage.processAllPages(processedFilePath);

    // 输出倒排索引
    // cout << "\nInverted Index:\n";
    // for (const auto &entry : webPage.getInvertedIndex()) {
    //     cout << "Word: " << entry.first << "\n";
    //     for (const auto &docInfo : entry.second) {
    //         cout << "Doc ID: " << docInfo.first << ", Weight: " << docInfo.second << "\n";
    //     }
    // }
    return 0;
}

