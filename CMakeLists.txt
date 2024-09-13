cmake_minimum_required(VERSION 3.20)
project(SpriteSheetSplitter)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenMP REQUIRED)

add_subdirectory(libraries)

set(SOURCES
        main.cpp
        Splitter.cpp
        IO/SpriteSheetIO.cpp
        IO/JSONConfigParser.cpp
        logging/LoggerTags.cpp
)

add_executable(SpriteSheetSplitter ${SOURCES})

target_include_directories(SpriteSheetSplitter PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libraries/struct_mapping")

target_link_libraries(SpriteSheetSplitter PRIVATE lodepng OpenMP::OpenMP_CXX)