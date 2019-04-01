cmake_minimum_required(VERSION 3.1)
#为了在window下编译
set(CMAKE_C_COMPILER "clang-cl")
set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_C_FLAGS_DEBUG "/Debug")
set(CMAKE_C_FLAGS_MINSIZEREL  "-Os -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE  "-O4 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO  "-O2 /Debug")
set(CMAKE_C_STANDARD  11)
set(CMAKE_CXX_COMPILER "clang-cl")
set(CMAKE_CXX_FLAGS "-Wall -Xclang -std=gnu++14")
set(CMAKE_CXX_FLAGS_DEBUG  "/Debug")
set(CMAKE_CXX_FLAGS_MINSIZEREL  "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE  "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "-O2 /Debug")

set(PROJECT_NAME GameEngine)
project(PROJECT_NAME)
#添加头文件
include_directories("${PROJECT_SOURCE_DIR}/Framework/Common")
include_directories("${PROJECT_SOURCE_DIR}/Framework/Interface")
#添加外部源文件
add_subdirectory(Framework)
add_subdirectory(Empty)