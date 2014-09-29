#ifndef RECORD_LIFETIME_MANAGER_TEST_H
#define RECORD_LIFETIME_MANAGER_TEST_H

#include <limits>
#include "gtest/gtest.h"
#include "record_lifetime_manager.h"
#include "cache_exceptions.h"


TEST(CacheRecordLifetimeManagerTestCase, RemoveFromEmpty) {
    concurrent_cache::CacheRecordLifetimeManager<int> mgr;
    ASSERT_THROW(mgr.getRecordToRemove(), concurrent_cache::QueueEmpty);
}


TEST(CacheRecordLifetimeManagerTestCase, MultiInsertion) {
    concurrent_cache::CacheRecordLifetimeManager<unsigned int> mgr;
    unsigned int limit = 1000;
    for(unsigned int i = 0; i < limit; ++i) {
        mgr.addRecord(i);
    }

    for(unsigned int i = 0; i < limit; ++i) {
        EXPECT_EQ(i, *mgr.getRecordToRemove());
    }
}


TEST(CacheRecordLifetimeManagerTestCase, SingleInsertion) {
    concurrent_cache::CacheRecordLifetimeManager<unsigned int> mgr;
    unsigned int testValue = 14793;
    mgr.addRecord(testValue);
    EXPECT_EQ(testValue, *mgr.getRecordToRemove());
}


#endif // RECORD_LIFETIME_MANAGER_TEST_H
