cmake_minimum_required(VERSION 3.1)
project(LearningC)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
find_package(SDL3 REQUIRED)
include_directories(${SDL3_INCLUDE_DIRS})

add_executable(main src/main.c src/util.c src/fileIO.c src/inflate/decoder.c src/pngChunks.c src/pngDecoder.c /opt/homebrew/include/SDL3/SDL.h)

target_link_libraries(main ${SDL3_LIBRARIES})
