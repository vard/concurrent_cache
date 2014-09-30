set(PROJECT_NAME_STR concurrent_cache)
project(${PROJECT_NAME_STR})
cmake_minimum_required(VERSION 2.8)

# set main project source list
set(SRC_LIST
         main.cpp
         concurrent_cache.h
         cache_exceptions.h
         record_lifetime_manager.h
         simple_db.h
         string_conv.h
         json/json.h
         json/jsoncpp.cpp

)

include_directories(${PROJECT_SOURCE_DIR})

# append compilation flags
list( APPEND CMAKE_CXX_FLAGS "-std=c++0x -std=c++1y ${CMAKE_CXX_FLAGS} -g -Wall -ftest-coverage")

add_executable(${PROJECT_NAME_STR} ${SRC_LIST})

find_package(Boost COMPONENTS system-mt REQUIRED)
if(NOT Boost_FOUND)
    message(SEND_ERROR "Failed to find boost::system_category-mt.")
    return()
else()
    include_directories(${Boost_INCLUDE_DIRS})
endif()


target_link_libraries(
    ${PROJECT_NAME_STR}
    ${Boost_LIBRARIES}
)










