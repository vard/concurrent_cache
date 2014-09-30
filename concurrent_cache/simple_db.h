#ifndef SIMPLE_DB_H
#define SIMPLE_DB_H

#include <string>
#include <fstream>
#include <iostream>
#include "boost/noncopyable.hpp"
#include "string_conv.h"
#include "json/json.h"
#include "cache_exceptions.h"

namespace concurrent_cache{

template<typename Key, typename Value>
class SimpleDB : private boost::noncopyable {
    public:
        SimpleDB(const std::string& dbFileName);
        ~SimpleDB();

        void update(const Key& key, const Value& value);
        Value find(const Key& key);

    private:

        static const char* rootKeyName(){
            return "records";
        }

        bool dbInited_;

        void initDb();
        void createNewDbDumpFile();
        void loadDbFromDump(std::fstream& dumpFile);
        std::string dbFileName_;

        Json::Value db_;
        std::unique_ptr<Json::Writer> writer_;
};


template<typename Key, typename Value>
SimpleDB<Key, Value>::SimpleDB(const std::string& dbFileName)
    :dbInited_(false),
     dbFileName_{dbFileName},
     writer_{new Json::StyledWriter}{
    initDb();
}


template<typename Key, typename Value>
SimpleDB<Key, Value>::~SimpleDB() {
    try{
        if(dbInited_){
            std::fstream  dbDumpFile;
            dbDumpFile.open(dbFileName_, std::ios::out | std::ios::trunc);
            dbDumpFile << writer_->write(db_);
            // std::cout << "dump to file: " << writer_->write(db_) << std::endl;
        }
    } catch (const std::exception& ex){
        std::cerr << ex.what() <<std::endl;
    }
}


template<typename Key, typename Value>
void SimpleDB<Key, Value>::update(const Key& key, const Value& value) {
    db_[this->rootKeyName()][key] =  toString(value);
}


template<typename Key, typename Value>
Value SimpleDB<Key, Value>::find(const Key& key) {
    Value val;
    fromString(db_[this->rootKeyName()][key].asString(), val);
    return val;
}


template<typename Key, typename Value>
void SimpleDB<Key, Value>::initDb() {
    std::fstream  dbDumpFile;
    dbDumpFile.open(dbFileName_, std::ios::out | std::ios::in);
    if (!dbDumpFile) {
        this->createNewDbDumpFile();
        db_[this->rootKeyName()] = Json::Value();
    } else {
        this->loadDbFromDump(dbDumpFile);
    }

    dbInited_ = true;
}


template<typename Key, typename Value>
void SimpleDB<Key, Value>::createNewDbDumpFile() {
    std::fstream  dbDumpFile;
    dbDumpFile.open(dbFileName_, std::ios::out);
    if(!dbDumpFile){
        throw DbInitException();
    }
    dbDumpFile.close();
    dbDumpFile.open(dbFileName_, std::ios::out | std::ios::in | std::ios::app);
    if(!dbDumpFile){
        throw DbInitException();
    }
}


template<typename Key, typename Value>
void SimpleDB<Key, Value>::loadDbFromDump(std::fstream& dumpFile) {
    std::string buf;
    dumpFile.seekg(0, std::ios::end);
    buf.reserve(dumpFile.tellg());
    dumpFile.seekg(0, std::ios::beg);

    buf.assign((std::istreambuf_iterator<char>(dumpFile)),
                std::istreambuf_iterator<char>());

    Json::Reader reader;
    std::cout << "dump file contains: " << buf << std::endl;

    auto parsingSuccessful = reader.parse(buf, db_);
    if (!parsingSuccessful) {
        throw DbParseException(reader.getFormattedErrorMessages().c_str());
    }

    // std::cout << "Parsed from file:" << std::endl << writer_->write(db_) << std::endl;
}


} // namespace

#endif // SIMPLE_DB_H
