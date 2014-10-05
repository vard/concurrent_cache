#include "concurrent_cache.h"
#include "simple_db.h"


int main()
{
    try{
        std::cout << "init cache" << std::endl;
        concurrent_cache::ConcurrentCache<std::string, std::string> cache(100, std::chrono::milliseconds(1000), boost::chrono::microseconds(100));

        //std::cout << cache.find("Ivanov");
        cache.find("Petrov");
        cache.update("Ivanov", "Kozel");
        cache.update("Petrov", "Eugen");
        cache.update("Sidorov", "Andrew");


    /*
    std::cout << "init db" << std::endl;
    concurrent_cache::SimpleDB<std::string, std::string> db("db.json");
    db.update("name", "Anton");
    db.update("surname", "Skornyakov");

    std::cout << "name: " << db.find("name") << std::endl;
    std::cout << "surname: " << db.find("surname") << std::endl;

    std::cout << "update surname" <<std::endl;
    db.update("surname", "Ivanov");
    std::cout << "surname: " << db.find("surname") << std::endl;
    */

    } catch (const std::exception& ex){
        std::cout << "ex: " << ex.what() << std::endl;
    }

    return 0;
}

