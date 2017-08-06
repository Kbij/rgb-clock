cmake_minimum_required (VERSION 2.8.11)
#set( CMAKE_VERBOSE_MAKEFILE on )
project (RGBCLOCK)
message(STATUS "CMAKE platform: " ${RGBCLOCK_PLATFORM})

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin-${RGBCLOCK_PLATFORM})
#set (SYSROOT ${CMAKE_CURRENT_SOURCE_DIR}/libraries/sysroot-${RGBCLOCK_PLATFORM})
#message(STATUS "Sysroot: " ${SYSROOT})
set(BUILD_STATIC_LIBS ON)

list(INSERT CMAKE_MODULE_PATH 0 "./libraries/gflags")
message(STATUS "gflags: " ${gflags_DIR})

#add_subdirectory (./libraries/buildlibs/ticpp-master)
#Needed for the generation of the log include files
find_package (glog 0.3.5 REQUIRED)
add_subdirectory (./libraries/gflags)
add_subdirectory (./libraries/glog)
add_subdirectory (./libraries/googletest)
add_subdirectory (./libraries/tinyxml2)
#find_package (gflags)
#add_subdirectory (./libraries/glog)

add_subdirectory (./RgbLib)
add_subdirectory (./RgbApp)
