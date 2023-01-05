cmake_minimum_required(VERSION 3.19)

project(rtpsendreceive
        LANGUAGES CXX)

string(REGEX REPLACE "(.*)/" "" THIS_PACKAGE_NAME "${CMAKE_CURRENT_SOURCE_DIR}")


set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

elseif(CMAKE_EXPORT_COMPILE_COMMANDS)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

if (APPLE)
    if (${CMAKE_GENERATOR} MATCHES "Xcode")
            if (${XCODE_VERSION} VERSION_LESS 10)
                message(STATUS "Xcode 10 or higher is required. Please install from the Mac App Store.")
                return ()
            elseif(${XCODE_VERSION} VERSION_GREATER_EQUAL 12)
                set(C74_BUILD_FAT YES)
            endif ()
    endif ()

    if (NOT CMAKE_OSX_ARCHITECTURES)
        if(C74_BUILD_FAT)
            set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "macOS architecture" FORCE)
        else()
            set(CMAKE_OSX_ARCHITECTURES ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "macOS architecture" FORCE)
        endif()
        message("CMAKE_OSX_ARCHITECTURES set to ${CMAKE_OSX_ARCHITECTURES}")
    endif()
endif()

# Fetch the correct version of the min-api
message(STATUS "Updating Git Submodules")
execute_process(
    COMMAND				git submodule update --init --recursive
    WORKING_DIRECTORY	"${CMAKE_CURRENT_SOURCE_DIR}"
)


# Misc setup and subroutines
include(${CMAKE_CURRENT_SOURCE_DIR}/source/min-api/script/min-package.cmake)


# Add unit tests for the API
enable_testing()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/min-api)


# Add the Lib, if it exists
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source/min-lib/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/min-lib)
endif ()


### PROJECT_SPECIFIC LIBRARY(FFMPEG)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(NOT DEFINED VCPKG_TARGET_TRIPLET)
if(APPLE)
set(VCPKG_TARGET_TRIPLET x64-osx)
elseif(WIN32)
set(VCPKG_TARGET_TRIPLET x64-windows-static)
endif()
endif()

set(VCPKG_PATH 
${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/${VCPKG_TARGET_TRIPLET})
message("vcpkg path: ${VCPKG_PATH}")
find_package(FFmpeg 
REQUIRED
) #add here the list of ffmpeg components required
# memo: building barematal ffmpeg for macos

if(FFMPEG_FOUND)
#  FFMPEG_INCLUDE_DIRS  - Include directory necessary for using the required components headers.
#  FFMPEG_LIBRARIES     - Link these to use the required ffmpeg components.
#  FFMPEG_DEFINITIONS   - Compiler switches required for using the required ffmpeg components.
    message("FFMPEG_INCLUDE_DIRS = ${FFMPEG_INCLUDE_DIRS} ")
    message("FFMPEG_LIBRARIES = ${FFMPEG_LIBRARIES} ")
    message("FFMPEG_DEFINITIONS = ${FFMPEG_DEFINITIONS} ")
endif()
# find_library(OPUS_LIB
#     NAMES libopus.a opus
# 	PATHS /usr/lib /usr/local/lib /usr/local/opt/opus/lib C:/ProgramData/chocolatey/lib/opus-tools/tools/lib
#     NO_DEFAULT_PATH
#     # REQUIRED
#     )
# message("OPUS_LIB =${OPUS_LIB}")

set(DEPENDENT_LIBS 
${FFMPEG_LIBRARIES} 
# ${OPUS_LIB}
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/rtpsendreceive_lib)

# Generate a project for every folder in the "source/projects" folder
SUBDIRLIST(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source/projects)
foreach (project_dir ${PROJECT_DIRS})
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source/projects/${project_dir}/CMakeLists.txt")
        message("Generating: ${project_dir}")
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/${project_dir})
    endif ()
endforeach ()
