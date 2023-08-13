cmake_minimum_required(VERSION 3.5)

project(du-clone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package (Boost COMPONENTS program_options thread REQUIRED)

add_executable(du-clone main.cpp du.hpp du.cpp)

if (Boost_FOUND)
    target_include_directories	(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIR})
    target_link_libraries 	(${PROJECT_NAME} ${Boost_LIBRARIES})
endif ()

install(TARGETS du-clone
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
