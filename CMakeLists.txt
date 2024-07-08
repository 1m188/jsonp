cmake_minimum_required(VERSION 3.0.0)
project(test CXX)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} PRIVATE jsonp_shared)