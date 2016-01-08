cmake_minimum_required (VERSION 2.8.11)
INCLUDE_DIRECTORIES(${SYSROOT}/include)
INCLUDE_DIRECTORIES( ./ )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHOSTBUILD")
file(GLOB exe_SRC
    "./*.h"
    "./*.cpp"
    "./lib/*.h"
    "./lib/*.cpp"
)
link_directories(${SYSROOT}/lib)
add_executable (RgbClock ${exe_SRC})
target_link_libraries(RgbClock TinyXml)
target_link_libraries(RgbClock libglog.a)
target_link_libraries(RgbClock libgflags.a)

target_link_libraries (RgbClock pthread)
target_link_libraries (RgbClock rt)
