#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <list>
#include <unordered_map>
#include <utility>
#include <string>
#include <iostream>

using namespace std;

class MyLRUCache {
public:
    MyLRUCache(size_t capacity) : _capacity(capacity) {}

    // 从缓存获取值
    bool get(const string &key, string &value) {
        cout << "Get key: " << key << "\n";
        auto it = _cacheMap.find(key);
        if (it == _cacheMap.end()) {
            cout << "Key not found: " << key << "\n";
            return false;
        }
        _cacheList.splice(_cacheList.begin(), _cacheList, it->second);
        value = it->second->second;
        return true;
    }

    // 将值放入缓存
    void put(const string &key, const string &value) {
        auto it = _cacheMap.find(key);
        if (it != _cacheMap.end()) {
            _cacheList.splice(_cacheList.begin(), _cacheList, it->second);
            it->second->second = value;
            return;
        }

        if (_cacheList.size() == _capacity) {
            auto del = _cacheList.back(); // 获取最久未使用的项
            _cacheMap.erase(del.first);  
            _cacheList.pop_back();        
        }

        _cacheList.emplace_front(key, value);
        // 更新映射，键值对映射到列表的迭代器
        _cacheMap[key] = _cacheList.begin();
    }

private:
    size_t _capacity;
    list<pair<string, string>> _cacheList; // 存储缓存项的双向链表，最近使用在前面
    unordered_map<string, typename list<pair<string, string>>::iterator> _cacheMap; // 存储键和列表位置对应关系
};

#endif
