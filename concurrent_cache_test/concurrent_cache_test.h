#ifndef CONCURRENT_CACHE_TEST_H
#define CONCURRENT_CACHE_TEST_H

#include <string>
#include "gtest/gtest.h"
#include "concurrent_cache.h"


class EmptyStringCacheFixture : public ::testing::Test {
    public:
        EmptyStringCacheFixture()
            :stringCache{1000, std::chrono::milliseconds{1000}, boost::chrono::milliseconds{100}}{};

    protected:
        virtual void SetUp() {
        }

        virtual void TearDown() {

        }

    concurrent_cache::ConcurrentCache<std::string, std::string> stringCache;
};


class EmptyIntCacheFixture : public ::testing::Test {
    public:
        EmptyIntCacheFixture()
            :intCache{1000, std::chrono::milliseconds{1000}, boost::chrono::milliseconds{100}}{};

    protected:
        virtual void SetUp() {
        }

        virtual void TearDown() {

        }

    concurrent_cache::ConcurrentCache<int, int> intCache;
};

TEST(CacheTestSupport, removeDbIfExists) {
    remove("db.json");
}


void createZeroSizeCache(){
    concurrent_cache::ConcurrentCache<std::string, std::string> cache{0,
                                                                std::chrono::milliseconds{1000},
                                                                boost::chrono::milliseconds{100}};
}


TEST(ConcurrentCacheCommon, initZeroSizeCache) {
    ASSERT_THROW(createZeroSizeCache(), concurrent_cache::CacheInvalidArgument);
}


TEST(ConcurrentCacheCommon, maxSizeGetter) {
    concurrent_cache::ConcurrentCache<std::string, std::string> cache{1,
                                                                std::chrono::milliseconds{1000},
                                                                boost::chrono::milliseconds{100}};
    EXPECT_EQ(cache.maxSize(), 1);
}


TEST_F(EmptyStringCacheFixture, find) {
    EXPECT_EQ(stringCache.size(), 0);
    EXPECT_EQ(stringCache.find("Ivanov").compare(""), 0);
    EXPECT_EQ(stringCache.size(), 1);
}


TEST_F(EmptyStringCacheFixture, findConsequence) {
    EXPECT_EQ(stringCache.size(), 0);
    const std::string surname0{"Petrov"};
    const std::string surname1{"Mark"};
    EXPECT_EQ(stringCache.find(surname0).compare(""), 0);
    EXPECT_EQ(stringCache.find(surname1).compare(""), 0);
    EXPECT_EQ(stringCache.size(), 2);
}


TEST_F(EmptyStringCacheFixture, update) {
    EXPECT_EQ(stringCache.size(), 0);
    const std::string name{"Eugen"};
    const std::string surname{"Petrov"};
    stringCache.update(surname, name);
    EXPECT_EQ(stringCache.find(surname).compare((name)), 0);
    EXPECT_EQ(stringCache.size(), 1);
}


TEST_F(EmptyStringCacheFixture, updateRewrite) {
    EXPECT_EQ(stringCache.size(), 0);
    const std::string initialName{"Eugen"};
    const std::string resultName{"Stepan"};
    const std::string surname{"Petrov"};
    stringCache.update(surname, initialName);
    stringCache.update(surname, resultName);
    EXPECT_EQ(stringCache.find(surname).compare((resultName)), 0);
    EXPECT_EQ(stringCache.size(), 1);
}


TEST_F(EmptyIntCacheFixture, fillCache) {
    EXPECT_EQ(intCache.size(), 0);
    int totalValuesInserted = 0;
    for(int nextValue = 0; nextValue < intCache.maxSize(); ++nextValue){
        intCache.find(nextValue);
        ++totalValuesInserted;
    }
    EXPECT_EQ(intCache.size(), totalValuesInserted);
    EXPECT_EQ(intCache.maxSize(), totalValuesInserted);
}


TEST_F(EmptyIntCacheFixture, overfillCache) {
    EXPECT_EQ(intCache.size(), 0);
    int totalValuesInserted = 0;
    for(int nextValue = 0; nextValue < intCache.maxSize()+1; ++nextValue){
        intCache.find(nextValue);
        ++totalValuesInserted;
    }
    EXPECT_EQ(intCache.size(), intCache.maxSize());
    EXPECT_LT(intCache.maxSize(), totalValuesInserted);
}

#endif // CONCURRENT_CACHE_TEST_H
