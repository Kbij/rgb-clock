cmake_minimum_required(VERSION 2.8)

INCLUDE_DIRECTORIES( ./src )
INCLUDE_DIRECTORIES( ${SYSROOT}/include )
file(GLOB app_SRC
    "./src/*.h"
    "./src/*.cpp"
)
add_executable (RgbTest ${app_SRC})

target_link_libraries (RgbTest RgbClockLib)
target_link_libraries (RgbTest pthread)
target_link_libraries (RgbTest rt)
target_link_libraries (RgbTest gtest)
if("${RGBCLOCK_PLATFORM}" STREQUAL "arm")
    add_custom_command(TARGET RgbTest POST_BUILD COMMAND scp $<TARGET_FILE:RgbTest> pi@musicbox.lan:/home/pi )
endif()    