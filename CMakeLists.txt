cmake_minimum_required(VERSION 3.20)
project(SpriteSheetSplitter)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenMP REQUIRED)

add_subdirectory(libraries)

add_executable(SpriteSheetSplitter main.cpp util/SplitterOptions.h IO/SpriteSheetIO.cpp IO/SpriteSheetIO.h Splitter.cpp Splitter.h util/SpriteSheetPNGData.h util/SpriteSheetType.h util/CharSheetInfo.h util/SpriteSplittingStatus.h util/SimpleTimer.h util/SpriteSplittingData.h IO/IOOptions.hpp logging/LoggerTags.cpp logging/LoggerTags.hpp IO/JSONConfigParser.cpp IO/JSONConfigParser.hpp)

target_include_directories(SpriteSheetSplitter PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libraries/struct_mapping")

target_link_libraries(SpriteSheetSplitter PRIVATE lodepng OpenMP::OpenMP_CXX)