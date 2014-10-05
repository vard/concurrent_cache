#ifndef SIMPLE_DB_TEST_H
#define SIMPLE_DB_TEST_H
#include <cstdio>
#include <string>
#include "gtest/gtest.h"
#include "simple_db.h"


class SimpleDbFixture : public ::testing::Test {
    public:
        SimpleDbFixture()
            :simpleDb{"test_db_str"}{};

    protected:
        virtual void SetUp() {
        }

        virtual void TearDown() {

        }

  concurrent_cache::SimpleDB<std::string, std::string> simpleDb;
};


TEST_F(SimpleDbFixture, FindInEmpty) {
    EXPECT_EQ(simpleDb.find("pseudo_random_key").compare(std::string()), 0);
}


TEST_F(SimpleDbFixture, Insertion) {
    std::string key{"KeyName"};
    std::string value{"ValueName"};
    simpleDb.update(key, value);
    auto valueRead = simpleDb.find(key);
    EXPECT_EQ(valueRead.compare(value), 0);
}


TEST(TestSupport, removeDbIfExistsAfter) {
    remove("test_db_str");
}


#endif // SIMPLE_DB_TEST_H
