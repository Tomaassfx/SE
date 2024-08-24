#include "QueryHandler.h"
#include <sstream>

#include <workflow/WFResourcePool.h>
#include <workflow/WFTaskFactory.h>

using json = nlohmann::json;

//Redis 缓存机制
QueryHandler::QueryHandler(const string &cnStopWordsPath, const string &enStopWordsPath, 
                           const string &invertedIndexFilePath, const string &offsetLibFilePath)
    : processor(cnStopWordsPath, enStopWordsPath, invertedIndexFilePath, offsetLibFilePath) {}

void QueryHandler::handleRequest(const string &userInput, wfrest::HttpResp *resp, SeriesWork *series) {
    cout << "Received user input: " << userInput << "\n";

    // 创建 Redis 任务来获取缓存
    WFRedisTask *redisGetTask = WFTaskFactory::create_redis_task(
        "redis://127.0.0.1:6379", 10, 
        [this, resp, userInput, series](WFRedisTask *redisTask) {
            protocol::RedisResponse *redisResp = redisTask->get_resp();
            protocol::RedisValue value;
            redisResp->get_result(value);

            if (value.is_string()) {
                // 缓存命中，直接返回结果
                cout << "Cache hit for query: " << userInput << "\n";
                resp->String(value.string_value());
            } else {
                // 缓存未命中，执行查询并将结果存入 Redis
                string resultJson = processor.processQuery(userInput);

                WFRedisTask *redisSetTask = WFTaskFactory::create_redis_task(
                    "redis://127.0.0.1:6379", 10, nullptr
                );
                redisSetTask->get_req()->set_request("SET", {userInput, resultJson});
                series->push_back(redisSetTask);

                // 设置响应头并返回查询结果
                resp->headers["Content-Type"] = "application/json";
                resp->String(resultJson);
            }
        }
    );

    // 设置 Redis GET 请求命令
    redisGetTask->get_req()->set_request("GET", {userInput});
    series->push_back(redisGetTask);
}


// QueryHandler::QueryHandler(const string &cnStopWordsPath, const string &enStopWordsPath, 
//                            const string &invertedIndexFilePath, const string &offsetLibFilePath,
//                            size_t lruCapacity, size_t poolCapacity)
//     : processor(cnStopWordsPath, enStopWordsPath, invertedIndexFilePath, offsetLibFilePath),
//       lruCachePool(poolCapacity)
// {
//     // 在资源池中初始化 LRU 缓存对象
//     for (size_t i = 0; i < poolCapacity; ++i) {
//         auto *cache = new MyLRUCache(lruCapacity);
//         lruCachePool.post(cache);
//         std::cout << "Posted cache resource " << cache << " to pool.\n";
//     }
// }

// void QueryHandler::handleRequest(const string &userInput, wfrest::HttpResp *resp, SeriesWork *series) {
//     std::cout << "Received user input: " << userInput << "\n";

//     // 从资源池中获取缓存对象
//     void* resource = nullptr;
//     // 创建一个任务，传递给 get 方法
//     SubTask* task = new LRUCacheTask(nullptr, userInput, resp, processor);
//     WFConditional* cond = lruCachePool.get(task, &resource);

//     if (cond == nullptr) {
//         std::cerr << "Failed to get WFConditional from resource pool" << std::endl;
//         resp->String("{\"error\": \"Internal error\"}");
//         return;
//     }

//     if (resource == nullptr) {
//         std::cerr << "Failed to get cache resource from pool" << std::endl;
//         resp->String("{\"error\": \"Cache resource error\"}");
//         return;
//     }

//     // 设置任务的缓存对象
//     static_cast<LRUCacheTask*>(task)->setCache(static_cast<MyLRUCache*>(resource));

//     // 将任务添加到 SeriesWork 中
//     series->push_back(task);

//     // 启动条件任务
//     cond->start();
// }



