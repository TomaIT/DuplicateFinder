cmake_minimum_required(VERSION 3.14)
project(DuplicateFinder)

set(CMAKE_CXX_STANDARD 17)

add_executable(DuplicateFinder main.cpp)
find_package(OpenSSL REQUIRED)
target_link_libraries(DuplicateFinder OpenSSL::SSL)