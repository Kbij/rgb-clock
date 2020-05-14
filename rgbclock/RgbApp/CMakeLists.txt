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

if("${RGBCLOCK_PLATFORM}" STREQUAL "arm")
    add_custom_command(TARGET RgbClock POST_BUILD COMMAND scp $<TARGET_FILE:RgbClock> pi@musicbox.lan:/home/pi )
endif()    





