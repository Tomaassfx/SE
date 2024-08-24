#include "QueryHandler.h"
#include "RequestHandler.h"
#include <signal.h>
#include <iostream>
#include <workflow/WFFacilities.h>
#include <wfrest/HttpServer.h>

using std::cout;
using std::map;
using std::string;

static WFFacilities::WaitGroup waitGroup(1);

void sighandler(int signum){
    cout << "done!\n"; // 尽量避免使用endl
    waitGroup.done();
}

// 测试Redis缓存
int main() {
    signal(SIGINT, sighandler);
    wfrest::HttpServer server;

    RequestHandler handler0("../data/dict.dat", "../data/index.dat");

    server.GET("/", [](const wfrest::HttpReq *req, wfrest::HttpResp *resp) {
        resp->File("../conf/html/rem-or-sea.html");
    });

    server.POST("/recommend", [&handler0](const wfrest::HttpReq *req, wfrest::HttpResp *resp) {
        string userInput = req->body(); 
        handler0.handleRequest(userInput, resp);
    });

    QueryHandler handler1("../conf/yuliao/stop_words_zh.txt", "../conf/yuliao/stop_words_eng.txt", 
                         "../data/invertedIndex.dat", "../data/deduplicatedOffsetData.dat");


    server.POST("/search", [&handler1](const wfrest::HttpReq *req, wfrest::HttpResp *resp, SeriesWork *series) {
        string userInput = req->body();
        handler1.handleRequest(userInput, resp, series);
    });

    if (server.track().start(8080) == 0) {
        server.list_routes();
        waitGroup.wait();
        cout << "finished!\n";
        server.stop();
        return 0;
    } else {
        perror("server start fail!");
        return -1;
    }
}

// int main() {
//     signal(SIGINT, sighandler); // 注册信号处理函数
//     wfrest::HttpServer server; // 创建 HTTP 服务器

//     // 创建 QueryHandler 实例，LRU 缓存容量设置为 10，资源池容量也设置为 10
//     QueryHandler handler("../conf/yuliao/stop_words_zh.txt", "../conf/yuliao/stop_words_eng.txt", 
//                          "../data/invertedIndex.dat", "../data/deduplicatedOffsetData.dat", 10, 10);

//     // 处理 GET 请求，返回 HTML 文件
//     server.GET("/", [](const wfrest::HttpReq *req, wfrest::HttpResp *resp) {
//         resp->File("../conf/html/rem-or-sea.html");
//     });

//     // 处理 POST 请求，处理搜索请求
//     server.POST("/search", [&handler](const wfrest::HttpReq *req, wfrest::HttpResp *resp, SeriesWork *series) {
//         string userInput = req->body();
//         handler.handleRequest(userInput, resp, series);
//     });

//     // 启动服务器并开始监听
//     if (server.track().start(8080) == 0) {
//         server.list_routes(); // 列出所有路由
//         waitGroup.wait(); // 等待信号处理
//         cout << "Server finished!" << endl;
//         server.stop(); // 停止服务器
//         return 0;
//     } else {
//         perror("Server start failed!"); // 启动失败
//         return -1;
//     }
// }