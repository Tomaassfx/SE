#include "PageLib.h"
#include <iostream>
#include <fstream>

PageLib::PageLib(){}

void PageLib::create() {
    string directoryPath = "../conf/人民网语料";

    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.path().extension() == ".xml") {
            parseRss(entry.path().string());
        }
    }
}

void PageLib::parseRss(const string &filename) {
    XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != XML_SUCCESS) {
        cerr << "Failed to load file: " << filename << endl;
        return;
    }

    XMLElement *rss = doc.FirstChildElement("rss");
    if (rss) {
        XMLElement *channel = rss->FirstChildElement("channel");
        if (channel) {
            XMLElement *item = channel->FirstChildElement("item");
            regex html_tag("<.*?>");
            stringstream ss;

            while (item) {
                RssItem rssItem;

                XMLElement *title = item->FirstChildElement("title");
                if (title && title->GetText()) {
                    rssItem.title = title->GetText();
                }

                XMLElement *link = item->FirstChildElement("link");
                if (link && link->GetText()) {
                    rssItem.link = link->GetText();
                }

                XMLElement *description = item->FirstChildElement("description");
                if (description && description->GetText()) {
                    rssItem.description = regex_replace(description->GetText(), html_tag, "");
                }

                XMLElement *contentE = item->FirstChildElement("content:encoded");
                if (contentE && contentE->GetText()) {
                    rssItem.content = regex_replace(contentE->GetText(), html_tag, "");
                }

                XMLElement *content = item->FirstChildElement("content");
                if (content && content->GetText()) {
                    rssItem.content = regex_replace(content->GetText(), html_tag, "");
                }
                /* *************************************************** */

                _rss.push_back(rssItem);

                // 格式化内容并存储到 _files
                ss.str("");  // 清空字符串流
                ss << "<doc>\n";
                ss << "    <docid> " << _rss.size() << " </docid>\n";
                ss << "    <title> " << rssItem.title << " </title>\n";
                ss << "    <link> " << rssItem.link << " </link>\n";
                ss << "    <description> " << rssItem.description << " \n</description>\n";
                ss << "    <content> " << rssItem.content << " \n</content>\n";
                ss << "</doc>\n\n";

                _files.push_back(ss.str());

                item = item->NextSiblingElement("item");
            }
        }
    }
}

void PageLib::store() {
    string outputDir = "../data"; // 输出目录
    string contentFileName = "/newripepage.dat"; // 存储网页内容的文件名
    string offsetFileName = "/newoffset.dat"; // 存储偏移信息的文件名

    // 创建 data 目录（如果不存在）
    if (!fs::exists(outputDir)) {
        fs::create_directory(outputDir);
    }

    // 打开内容文件
    ofstream ofs(outputDir + contentFileName);
    if (!ofs.is_open()) {
        cerr << "Failed to open " << outputDir + contentFileName << " for writing" << "\n";
        return;
    }

    // 打开偏移信息文件
    ofstream offsetOfs(outputDir + offsetFileName);
    if (!offsetOfs.is_open()) {
        cerr << "Failed to open " << outputDir + offsetFileName << " for writing" << "\n";
        return;
    }

    // 写入 _files 容器中的内容到文件中，并记录偏移信息
    ofs << "<docs>\n";
    for (size_t i = 0; i < _files.size(); ++i) {
        int docStartOffset = ofs.tellp(); // 记录文档起始位置
        ofs << _files[i]; // 写入文档内容
        int docEndOffset = ofs.tellp(); // 记录文档结束位置
        int docLength = docEndOffset - docStartOffset; // 计算文档长度

        // 将文档偏移信息保存到 _offsetLib 中
        _offsetLib[i + 1] = {docStartOffset, docLength};

        // 将偏移信息写入 offset 文件
        offsetOfs << i + 1 << " " << docStartOffset << " " << docLength << "\n";
    }
    ofs << "</docs>\n";
    

    ofs.close();
    offsetOfs.close();
}


int main() {
    PageLib lib;
    lib.create();
    lib.store();
    return 0;
}


