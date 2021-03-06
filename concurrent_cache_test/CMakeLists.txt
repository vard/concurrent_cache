set(PROJECT_NAME concurrent_cache_test)

project(${PROJECT_NAME})
cmake_minimum_required(VERSION 2.8)

include_directories("../concurrent_cache/")

list( APPEND CMAKE_CXX_FLAGS "-std=c++0x -std=c++1y ${CMAKE_CXX_FLAGS} -g -Wall -ftest-coverage")

# gtest
set(GTEST_INCLUDE_DIR /Users/vard/lib/gtest/gtest-1.7.0/include)
set(GTEST_LIBS_DIR /Users/vard/lib/gtest/gtest-1.7.0/build)
include_directories(${GTEST_INCLUDE_DIR})


# generate list of files and store it in variable

set(SRC_LIST
    test_main.cpp
    record_lifetime_manager_test.h
    simple_db_test.h
    concurrent_cache_test.h
    json/jsoncpp.cpp
)

find_package(Boost COMPONENTS system-mt chrono thread REQUIRED)
if(NOT Boost_FOUND)
    message(SEND_ERROR "Failed to find boost libraries.")
    return()
else()
    include_directories(${Boost_INCLUDE_DIRS})
endif()


add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME}
                      ${GTEST_LIBS_DIR}/libgtest.a
                      ${GTEST_LIBS_DIR}/libgtest_main.a
                      ${Boost_LIBRARIES}
)
