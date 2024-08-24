#ifndef PAGELIBPREPROCESSOR_H
#define PAGELIBPREPROCESSOR_H

#include <unordered_map>
#include <vector>
#include <string>
#include <utility> 

using namespace std;

class PageLibPreprocessor {
public:
    PageLibPreprocessor();          // 构造函数
    void doProcess();               // 执行预处理
    void readInfoFromFile();        // 根据配置信息读取网页库和位置偏移库的内容
    void cutRedundantPages();       // 对冗余的网页进行去重
    void buildInvertIndexTable();   // 创建倒排索引表
    void storeOnDisk();             // 将经过预处理之后的网页库、位置偏移库和倒排索引表写回到磁盘上
private:
    unordered_map<int, pair<int, int> > _offsetLib;                     // 网页偏移库对象
    unordered_map<string, vector<pair<int, double>>> _invertIndexTable; // 倒排索引表对象
};


#endif