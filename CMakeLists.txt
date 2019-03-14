cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(PROJECT_NAME GameEngine)
project(PROJECT_NAME)
#添加头文件
include_directories("${PROJECT_SOURCE_DIR}/Framework/Common")
include_directories("${PROJECT_SOURCE_DIR}/Framework/Interface")
#添加外部源文件
add_subdirectory(Framework)
add_subdirectory(Empty)