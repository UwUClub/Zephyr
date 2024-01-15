cmake_minimum_required(VERSION 3.15...3.23)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Systems)
add_subdirectory(Events)
add_subdirectory(Libraries)
