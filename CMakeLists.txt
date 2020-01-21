cmake_minimum_required(VERSION 3.8)
project(array_of_actions VERSION 1.0.0 DESCRIPTION "A collection of benchmarks on the different ways to implement an array of actions" LANGUAGES CXX)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/array_of_pure_actions")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/array_of_actions_with_data")
