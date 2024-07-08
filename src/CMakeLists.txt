cmake_minimum_required(VERSION 3.0.0)
project(jsonp CXX)

set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 11)

file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${PROJECT_NAME}_shared SHARED ${sources})
target_include_directories(${PROJECT_NAME}_shared INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(${PROJECT_NAME}_static STATIC ${sources})
target_include_directories(${PROJECT_NAME}_static INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})