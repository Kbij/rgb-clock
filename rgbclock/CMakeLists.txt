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
set (SYSROOT ${CMAKE_CURRENT_SOURCE_DIR}/sysroot-${RGBCLOCK_PLATFORM})
message(STATUS "Sysroot: " ${SYSROOT})
set(BUILD_STATIC_LIBS ON)

#No glog testing
set(BUILD_TESTING OFF)
#install libraries in the sysroot directory (for eclipse)
set(CMAKE_INSTALL_PREFIX ${SYSROOT})

#gflags settings
set(GFLAGS_IS_SUBPROJECT TRUE)
set(INSTALL_HEADERS TRUE)
set(GFLAGS_INSTALL_STATIC_LIBS TRUE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
set(GFLAGS_INCLUDE_DIR "gflags")

#for tinyxml2:
SET(BUILD_SHARED_LIBS OFF)
SET(BUILD_STATIC_LIBS ON)
SET(BUILD_TESTS OFF)

add_subdirectory (./libraries/gflags)
add_subdirectory (./libraries/glog)
add_subdirectory (./libraries/googletest)
add_subdirectory (./libraries/tinyxml2)

add_subdirectory (./RgbLib)
add_subdirectory (./RgbApp)
