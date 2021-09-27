cmake_minimum_required(VERSION 3.20)
project(SpriteSheetSplitter)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lodepng)

add_executable(SpriteSheetSplitter main.cpp splitterOptions.h IO/SpriteSheetIO.cpp IO/SpriteSheetIO.h Splitter.cpp Splitter.h SpriteSheetData.h)

target_link_libraries(SpriteSheetSplitter PRIVATE lodepng)