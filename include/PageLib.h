#ifndef PAGELIB_H
#define PAGELIB_H

#include <vector>
#include <map>
#include <string>
#include <regex>
#include <filesystem>
#include <tinyxml2.h>

using namespace std;
namespace fs = std::filesystem;
using namespace tinyxml2;

using namespace std;

struct RssItem {
    string title;
    string link;
    string description;
    string content;
};

class PageLib {
public:
    PageLib();
    void create(); // 创建网页库
    void store(); // 存储网页库和网页偏移库
private:
    void parseRss(const string &filename);
private:
    vector<string> _files; // 存放格式化之后的网页的容器
    map<int, pair<int, int>> _offsetLib; // 存放每篇文档在网页库的位置信息
    vector<RssItem> _rss;
};

#endif