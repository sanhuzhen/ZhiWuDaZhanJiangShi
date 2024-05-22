cmake_minimum_required(VERSION 3.27)
project(untitled)
set(CMAKE_CXX_STANDARD 17)
link_libraries(libeasyx.a)
link_libraries(winmm.lib)
add_executable(untitled main.cpp
        tools.cpp
vector2.cpp)
