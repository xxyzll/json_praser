cmake_minimum_required(VERSION 3.0.0)
project(json_praser VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(json_praser main.cpp json_parser/json_parser.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
