add_library("communication" ./src/socketCommunication.cpp)

target_include_directories(communication PUBLIC include)
target_include_directories(communication PRIVATE ${CMAKE_SOURCE_DIR}/include)
