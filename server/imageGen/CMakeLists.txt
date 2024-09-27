add_library(imageGenerator ./src/imageGen.cpp)

target_include_directories(imageGenerator PUBLIC include)
target_include_directories(imageGenerator PRIVATE ${CMAKE_SOURCE_DIR}/include)