#对Empty这个可执行文件添加了这个源文件
add_executable(Empty EmptyApplication.cpp)
#生成Empty可执行文件，链接了Common这个库
target_link_libraries(Empty Common)