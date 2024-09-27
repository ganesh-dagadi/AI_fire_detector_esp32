cmake_minimum_required(VERSION 3.0)

project("fireCameraServer")
add_subdirectory(./communication)
add_subdirectory(./imageGen)

add_executable("fireCameraServer" ./src/fireCameraServer.cpp ./src/threadManager.cpp)
target_include_directories("fireCameraServer" PRIVATE include)
target_link_libraries("fireCameraServer" communication)
target_link_libraries("fireCameraServer" imageGenerator)