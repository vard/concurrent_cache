#ifndef CONCURRENT_CACHE_H
#define CONCURRENT_CACHE_H

#include <cstdint>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <memory>
#include <functional>
#include "boost/noncopyable.hpp"
#include "boost/thread/locks.hpp"
#include "boost/thread/shared_mutex.hpp"
#include "cache_exceptions.h"
#include "record_lifetime_manager.h"

namespace concurrent_cache{


template<typename Key, typename Value, typename Hasher = std::hash<Key>>
class ConcurrentCache : private boost::noncopyable {
    public:
        ConcurrentCache(std::uint64_t maxSize, const std::chrono::milliseconds& syncPeriodMs, const std::chrono::microseconds& getAccessTimeoutUs);
        ~ConcurrentCache();

        Value find(const Key& key);
        void update(const Key& key, const Value& value);


    private:

        struct ValueRecord{
                std::timed_mutex mtx;
                Value value;
        };

        typedef typename std::reference_wrapper<typename std::unordered_map<Key, ValueRecord, Hasher>::value_type> RecordRef;

        std::uint64_t maxSize_;
        std::uint64_t currentSize_;
        std::chrono::milliseconds syncPeriodMs_;
        std::chrono::microseconds getAccessTimeoutUs_;

        std::atomic<bool> stopSync_;
        boost::shared_mutex globalSharedMtx_;
        std::thread syncThread;

        std::unordered_map<Key, ValueRecord, Hasher> hashMap_;
        CacheRecordLifetimeManager<RecordRef> recordLifetimeManager_;
        void syncTask();

};


template<typename Key, typename Value, typename Hasher>
ConcurrentCache<Key, Value, Hasher>::ConcurrentCache(std::uint64_t maxSize, const std::chrono::milliseconds& syncPeriodMs, const std::chrono::microseconds& getAccessTimeoutUs)
    :maxSize_{maxSize},
     currentSize_{0},
     syncPeriodMs_{syncPeriodMs},
     getAccessTimeoutUs_{getAccessTimeoutUs},
     stopSync_{false},
     syncThread{&ConcurrentCache<Key, Value, Hasher>::syncTask, this}
{
    if(0 == maxSize_){
        throw CacheInvalidArgument("Zero max cache size");
    }
}


template<typename Key, typename Value, typename Hasher>
ConcurrentCache<Key, Value, Hasher>::~ConcurrentCache() {
    if(syncThread.joinable()){
        stopSync_.store(true);
        try{
            syncThread.join();
        } catch (const std::exception& ex){
            std::cerr << ex.what(); // log somewhere
        }
    }
}


template<typename Key, typename Value, typename Hasher>
Value ConcurrentCache<Key, Value, Hasher>::find(const Key& key) {
    boost::shared_lock<boost::shared_mutex> readLock(globalSharedMtx_, getAccessTimeoutUs_);
    if(!readLock.owns_lock()){
        throw CacheTimeoutException();
    }

    auto keyFound = hashMap_.find(key);
    if(hashMap_.end() == keyFound){
        // insert
    } else {
        std::unique_lock<std::timed_mutex> lock(keyFound.second.mtx, getAccessTimeoutUs_);
        if(!lock.owns_lock()){
            throw CacheTimeoutException();
        }
        return keyFound.second.value;
    }
}


template<typename Key, typename Value, typename Hasher>
void ConcurrentCache<Key, Value, Hasher>::syncTask() {
    do {
        int a;
        a++;
    } while(!stopSync_.load());
}



} // namespace

#endif // CONCURRENT_CACHE_H
