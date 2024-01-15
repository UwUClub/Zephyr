cmake_minimum_required(VERSION 3.15...3.23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O -ldl")

add_subdirectory(Core)

target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32> ${Boost_LIBRARIES})
