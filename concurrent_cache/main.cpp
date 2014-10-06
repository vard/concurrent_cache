#include <cstdlib>
#include "concurrent_cache.h"
#include "simple_db.h"


std::atomic<bool> doWork;
concurrent_cache::ConcurrentCache<std::string, std::string> cache(100, std::chrono::milliseconds(1000), boost::chrono::microseconds(100));

std::string randomStrGen(int length, unsigned int *seed) {
    static std::string charset = "abcdefghijklmnopqrstuvwxyz";
    std::string result;
    result.resize(length);


    for (int i = 0; i < length; i++){
        result[i] = charset[rand_r(seed) % charset.length()];
    }

    return result;
}

void workThread(){
    unsigned int seed = time(NULL);
    while(doWork.load()){
        try{
        auto key1 = randomStrGen(2, &seed);
        auto key2 = randomStrGen(2, &seed);
        auto value = randomStrGen(10, &seed);
        auto val = cache.find(key1);
        cache.update(key2, value);
        } catch (const concurrent_cache::CacheTimeoutException& ex){
            //std::cout << ex.what() << std::endl;

        } catch (const std::exception& ex){
            std::cout << ex.what() << std::endl;
        }
    }
}

int main()
{
    try{
        srand(time(NULL));
        doWork.store(true);
        std::vector<std::thread> threads;
        threads.resize(8);
        std::for_each(std::begin(threads), std::end(threads), [](std::thread& thisThread){
            thisThread = std::thread{workThread};
        });



        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        doWork.store(false);
        std::for_each(std::begin(threads), std::end(threads), [](std::thread& thisThread){
            thisThread.join();
        });






    } catch (const std::exception& ex){
        std::cout << "ex: " << ex.what() << std::endl;
    }

    return 0;
}

