cmake_minimum_required(VERSION 2.8)

INCLUDE_DIRECTORIES( ./src )
INCLUDE_DIRECTORIES( ${SYSROOT}/include )
file(GLOB lib_SRC
    "./src/*.h"
    "./src/*.cpp"
    "./src/lib/*.h"
    "./src/lib/*.cpp"
)
add_library (RgbClockLib ${lib_SRC})
target_include_directories(RgbClockLib PUBLIC ./src )
target_link_libraries (RgbClockLib glog)
target_link_libraries (RgbClockLib gflags)
target_link_libraries (RgbClockLib tinyxml2_static)


