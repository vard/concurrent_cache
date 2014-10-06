#ifndef CONCURRENT_CACHE_H
#define CONCURRENT_CACHE_H

#include <cstdint>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <future>
#include <memory>
#include <functional>
#include "boost/noncopyable.hpp"
#include "boost/thread/locks.hpp"
#include "boost/thread/shared_mutex.hpp"
#include "cache_exceptions.h"
#include "record_lifetime_manager.h"
#include "simple_db.h"

namespace concurrent_cache{

template<typename Lock>
void checkLock(Lock& lock){
    if(!lock.owns_lock()){
        throw CacheTimeoutException();
    }
}


template<typename Key, typename Value, typename Hasher = std::hash<Key>>
class ConcurrentCache : private boost::noncopyable {
    public:
        ConcurrentCache(std::uint64_t maxSize,
                        const std::chrono::milliseconds& syncPeriodMs,
                        const boost::chrono::microseconds& getAccessTimeoutUs);
        ~ConcurrentCache();
        Value find(const Key& key);
        void update(const Key& key, const Value& value);
        std::uint64_t size();
        std::uint64_t maxSize();
        static const char* dbName(){
            return "db.json";
        }

    private:
        struct ValueRecord{
                Value value;
                std::shared_ptr<boost::timed_mutex> mtx; // wrap with pointer to make this struct copyble
                ValueRecord(const Value& val)
                    :value{val},
                     mtx{std::make_shared<boost::timed_mutex>()}{}


        };


        // use value_type reference to store records links in lifetime manager, value_type ref doesn't invalidate when
        // hashmap reallocating and doing rehash, while iterators does
        typedef typename std::reference_wrapper<typename std::unordered_map<Key, ValueRecord, Hasher>::value_type> RecordRef;

        static const char* unexpectedException(){
            return "Unexpected exception";
        }

        void sync();
        void syncTask();
        ValueRecord& loadFromDb(const Key& key);
        void removeRecords();


        std::uint64_t maxSize_;
        std::uint64_t currentSize_;
        std::chrono::milliseconds syncPeriodMs_;
        boost::chrono::microseconds getAccessTimeoutUs_;

        std::future<void> syncThreadRes_;
        std::atomic<bool> stopSync_;
        boost::shared_mutex globalSharedMtx_;

        std::unordered_map<Key, ValueRecord, Hasher> hashMap_;
        CacheRecordLifetimeManager<RecordRef> recordLifetimeManager_;
        SimpleDB<Key, Value> db_;

};


template<typename Key, typename Value, typename Hasher>
ConcurrentCache<Key, Value, Hasher>::ConcurrentCache(std::uint64_t maxSize,
                                                     const std::chrono::milliseconds& syncPeriodMs,
                                                     const boost::chrono::microseconds& getAccessTimeoutUs)
    :maxSize_{maxSize},
     currentSize_{0},
     syncPeriodMs_{syncPeriodMs},
     getAccessTimeoutUs_{getAccessTimeoutUs},
     stopSync_{false},
     db_{this->dbName()} {
    if(0 == maxSize_){
        throw CacheInvalidArgument("Zero max cache size");
    }

    syncThreadRes_ = std::async(&ConcurrentCache::syncTask, this);
}


template<typename Key, typename Value, typename Hasher>
ConcurrentCache<Key, Value, Hasher>::~ConcurrentCache() {
    try{
        // get exceptions occured in sync thread
        try{
            stopSync_.store(true);
            syncThreadRes_.get();
        } catch (const std::exception& ex){
            std::cerr << ex.what(); // log somewhere
        } catch(...){
            std::cerr << unexpectedException(); // log somewhere
        }
    } catch (const std::exception& ex){
        std::cerr << ex.what(); // log somewhere
    } catch(...){
        std::cerr << unexpectedException(); // log somewhere
    }
}


template<typename Key, typename Value, typename Hasher>
Value ConcurrentCache<Key, Value, Hasher>::find(const Key& key) {
    boost::shared_lock<boost::shared_mutex> readLock{globalSharedMtx_, getAccessTimeoutUs_};
    checkLock(readLock);

    auto keyFound = hashMap_.find(key);
    if(hashMap_.end() == keyFound){
        readLock.unlock();
        boost::unique_lock<boost::shared_mutex> globalWriteLock{globalSharedMtx_, getAccessTimeoutUs_};
        checkLock(globalWriteLock);
        // another thread could load such key since this thread unlocked shared mutex, need to check it
        auto keyFound = hashMap_.find(key);
        if(hashMap_.end() == keyFound){
            return (this->loadFromDb(key)).value;
        } else {
            return (*keyFound).second.value;
        }

    } else {
        boost::unique_lock<boost::timed_mutex> recordLock{*((*keyFound).second.mtx), getAccessTimeoutUs_};
        checkLock(recordLock);
        return (*keyFound).second.value;
    }
}


template<typename Key, typename Value, typename Hasher>
void ConcurrentCache<Key, Value, Hasher>::update(const Key& key, const Value& value) {
    boost::shared_lock<boost::shared_mutex> readLock{globalSharedMtx_, getAccessTimeoutUs_};
    checkLock(readLock);

    auto keyFound = hashMap_.find(key);
    if(hashMap_.end() == keyFound){
        readLock.unlock();
        boost::unique_lock<boost::shared_mutex> globalWriteLock{globalSharedMtx_, getAccessTimeoutUs_};
        checkLock(globalWriteLock);
        // another thread could load such key since this thread unlocked shared mutex, need to check it
        auto keyFound = hashMap_.find(key);
        if(hashMap_.end() == keyFound){
            this->loadFromDb(key).value = value;
        } else {
            (*keyFound).second.value = value;
        }

    } else {
        boost::unique_lock<boost::timed_mutex> recordLock{*((*keyFound).second.mtx), getAccessTimeoutUs_};
        checkLock(recordLock);
        (*keyFound).second.value = value;
    }
}


template<typename Key, typename Value, typename Hasher>
std::uint64_t ConcurrentCache<Key, Value, Hasher>::size() {
    boost::shared_lock<boost::shared_mutex> globalReadLock{globalSharedMtx_, getAccessTimeoutUs_};
    checkLock(globalReadLock);
    return currentSize_;
}


template<typename Key, typename Value, typename Hasher>
std::uint64_t ConcurrentCache<Key, Value, Hasher>::maxSize() {
    return maxSize_; // readonly value, no need sync
}


template<typename Key, typename Value, typename Hasher>
void ConcurrentCache<Key, Value, Hasher>::sync() {
    std::for_each(std::begin(hashMap_), std::end(hashMap_), [this](auto& thisRecord){
        boost::unique_lock<boost::timed_mutex> recordLock{*((thisRecord).second.mtx)};
        db_.update(thisRecord.first, thisRecord.second.value);
    });
}


template<typename Key, typename Value, typename Hasher>
void ConcurrentCache<Key, Value, Hasher>::syncTask() {
    auto startPoint = std::chrono::system_clock::now();
    auto endPoint = startPoint;
    while(1) {
        if(std::chrono::duration_cast<std::chrono::milliseconds>(endPoint - startPoint) < syncPeriodMs_){
            auto waitForMs = syncPeriodMs_- std::chrono::duration_cast<std::chrono::milliseconds>(endPoint - startPoint);
            std::this_thread::sleep_for(waitForMs);
        }

        startPoint = std::chrono::system_clock::now();
        boost::shared_lock<boost::shared_mutex> globalReadLock{globalSharedMtx_};
        // here, internally we access db under reader lock only, this method shouldn't be called from multiple threads
        // (syncronization thread only)
        this->sync();

        if(stopSync_.load()) {
            break;
        }
        endPoint = std::chrono::system_clock::now();
    }
}


template<typename Key, typename Value, typename Hasher>
typename ConcurrentCache<Key, Value, Hasher>::ValueRecord& ConcurrentCache<Key, Value, Hasher>::loadFromDb(const Key& key) {

    if(currentSize_ >= maxSize_){
        removeRecords();
    }
    auto value = db_.find(key);
    auto insertionRes = hashMap_.insert(std::make_pair<Key, ValueRecord>(Key(key), value));
    auto insertedSuccessfully = insertionRes.second;
    if(!insertedSuccessfully){
        throw CacheInternalException("Error inserting record in hashmap");
    }
    auto iter = insertionRes.first;
    try{
        recordLifetimeManager_.addRecord(*iter);
    } catch (const std::exception& ex){
        // underlying recordLifetimeManager_ std::queue<std::deque> gives us strong exception
        // safety guarantee, so we need to remove recently inserted record from hashmap to keep
        // hasmap and queue in consistency
        hashMap_.erase(iter);
        throw ex;
    }

    ++currentSize_;
    return (*iter).second;
}


template<typename Key, typename Value, typename Hasher>
void ConcurrentCache<Key, Value, Hasher>::removeRecords() {
   auto recordToRemove = recordLifetimeManager_.getRecordToRemove();
   // at this moment record already removed from lifetime manager queue, but still contains in hashmap
   // erase method doesn't throw exception other than those thrown by the hash object ot equality predicate,
   // so need to be careful using own hasher and equality predicate
   auto deleted = hashMap_.erase((*recordToRemove).get().first);
   if(1!=deleted){
       throw CacheInternalException("Record in lifetime manager haven't appropriate record in hashmap");
   }
   --currentSize_;

}



} // namespace

#endif // CONCURRENT_CACHE_H
