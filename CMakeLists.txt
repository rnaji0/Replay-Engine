cmake_minimum_required(VERSION 3.18)
project(ReplayEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(src/include)


file(GLOB_RECURSE SRC_FILES
    src/src/*.cpp
    tests/*.cpp
)


add_executable(replay_engine src/main.cpp ${SRC_FILES})


find_package(Threads REQUIRED)
target_link_libraries(replay_engine PRIVATE Threads::Threads)
