cmake_minimum_required(VERSION 2.8)

INCLUDE_DIRECTORIES( ./src )
INCLUDE_DIRECTORIES( ${SYSROOT}/include )
file(GLOB app_SRC
    "./src/*.h"
    "./src/*.cpp"
)
add_executable (RgbClock ${app_SRC})

target_link_libraries (RgbClock RgbClockLib)
target_link_libraries (RgbClock pthread)
target_link_libraries (RgbClock rt)







