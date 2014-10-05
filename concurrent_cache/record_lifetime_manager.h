#ifndef RECORD_LIFETIME_MANAGER_H
#define RECORD_LIFETIME_MANAGER_H

#include <queue>
#include <memory>
#include "boost/noncopyable.hpp"
#include "cache_exceptions.h"


namespace concurrent_cache{


template<typename Record>
class CacheRecordLifetimeManager : boost::noncopyable  {
    public:
        CacheRecordLifetimeManager() = default;
        void addRecord(const Record& record);
        // use shared_ptr to provide exception safety keeping in mind Record copy constructors can rise such
        std::shared_ptr<Record> getRecordToRemove();

    private:
        // simple FIFO
        std::queue<Record> queue_;
};


template<typename Record>
void CacheRecordLifetimeManager<Record>::addRecord(const Record& record) {
    queue_.push(record);
}


template<typename Record>
std::shared_ptr<Record> CacheRecordLifetimeManager<Record>::getRecordToRemove(){
    if(queue_.empty()){
        throw QueueEmpty();
    }
    // create shared_ptr before pop element to provide strong exveption safety
    std::shared_ptr<Record> recordToRemove{std::make_shared<Record>(queue_.front())};
    queue_.pop();
    return recordToRemove; // no exception
}


} // namespace
#endif // RECORD_LIFETIME_MANAGER_H
