cmake_minimum_required(VERSION 3.23)

project(sym)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(dep/emlabcpp)
add_subdirectory(dep/rapidcheck)

add_executable(sym_test src/test.cpp)
target_link_libraries(sym_test PUBLIC emlabcpp rapidcheck)
