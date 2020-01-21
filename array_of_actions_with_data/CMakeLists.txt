cmake_minimum_required(VERSION 3.11)
project(array_of_pure_actions VERSION 1.0.0 DESCRIPTION "Benchmarks on the different ways to implement an array of pure actions" LANGUAGES CXX)

add_executable(array_of_pure_actions)
target_compile_features(array_of_pure_actions PRIVATE cxx_std_17)
target_include_directories(array_of_pure_actions PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_sources(array_of_pure_actions PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp")
