cmake_minimum_required(VERSION 3.15...3.23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O")

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Core)