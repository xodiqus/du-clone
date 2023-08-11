cmake_minimum_required(VERSION 3.5)

project(du-clone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(du-clone main.cpp)

install(TARGETS du-clone
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
