INCLUDE(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER(gcc GNU)
CMAKE_FORCE_CXX_COMPILER(g++ GNU)

cmake_minimum_required(VERSION 3.7)
project(player)

set(CMAKE_CXX_FLAGS "-std=c++14")

set(SOURCE_FILES
        greedyC.cpp
        raceState.hpp
        raceState.cpp reinforce.cc.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/Users/albicilla/programming/samurAI/samurai17/player/beamZ/")

add_executable(player ${SOURCE_FILES})
