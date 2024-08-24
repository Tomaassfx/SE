#ifndef LRUCACHETASK_H
#define LRUCACHETASK_H

#include "MyLRUCache.h"
#include "QueryProcessor.h"
#include <workflow/WFTaskFactory.h>
#include <workflow/WFResourcePool.h>
#include <workflow/SubTask.h>
#include <workflow/WFFacilities.h>
#include <wfrest/HttpServer.h>
#include <workflow/WFTaskFactory.h>
#include <workflow/WFGlobal.h>
#include <string>
#include <iostream>

using namespace std;

class LRUCacheTask : public SubTask {
public:
    LRUCacheTask(MyLRUCache *cache, const string &key, wfrest::HttpResp *resp, QueryProcessor &processor)
        : cache_(cache), key_(key), resp_(resp), processor_(processor) {
        if (cache_ == nullptr) {
            throw std::invalid_argument("Cache pointer cannot be null");
        }
        }

    void setCache(MyLRUCache *cache) {
        cache_ = cache;
    }
    
    // 实现任务逻辑
    void LRUCacheTask::dispatch() override {
        if (cache_ == nullptr) {
            cerr << "Cache is null in LRUCacheTask::dispatch" << "\n";
            return;
        }

        string result;
        if (cache_->get(key_, result)) {
            // 缓存命中，直接返回结果
            resp_->String(result);
        } else {
            // 缓存未命中，执行查询逻辑
            result = processor_.processQuery(key_);
            cache_->put(key_, result);
            resp_->String(result);
        }
        // 标记任务完成
        this->subtask_done();
    }

    SubTask* done() override {
        // 任务完成后的清理或通知逻辑
        // 示例：
        cout << "Task done: " << key_ << endl;
        return this; // 返回当前对象
    }

private:
    MyLRUCache *cache_;
    string key_;
    wfrest::HttpResp *resp_;
    QueryProcessor &processor_; // 注意：QueryProcessor 的引用
};

#endif