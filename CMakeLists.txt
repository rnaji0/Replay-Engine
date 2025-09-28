cmake_minimum_required(VERSION 3.18)
project(ReplayEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src/include)
include_directories(testEngine/include)

file(GLOB_RECURSE SRC_FILES
    src/src/*.cpp
)

file(GLOB_RECURSE TEST_FILES
    testEngine/src/*.cpp
)

add_executable(replay_engine src/main.cpp ${SRC_FILES} ${TEST_FILES})

find_package(Threads REQUIRED)
target_link_libraries(replay_engine PRIVATE Threads::Threads)
