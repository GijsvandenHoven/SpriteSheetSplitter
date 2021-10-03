add_library(
        lodepng
        src/lodepng.cpp
        src/lodepng.h
)

target_include_directories(lodepng PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")