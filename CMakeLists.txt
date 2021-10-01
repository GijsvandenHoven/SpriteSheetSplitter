cmake_minimum_required(VERSION 3.20)
project(SpriteSheetSplitter)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenMP REQUIRED)

add_subdirectory(lodepng)

add_executable(SpriteSheetSplitter main.cpp util/splitterOptions.h IO/SpriteSheetIO.cpp IO/SpriteSheetIO.h Splitter.cpp Splitter.h util/SpriteSheetData.h util/SpriteSheetType.h)

target_link_libraries(SpriteSheetSplitter PRIVATE lodepng OpenMP::OpenMP_CXX)