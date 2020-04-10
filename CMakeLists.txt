cmake_minimum_required(VERSION 3.10)
project(Rtpsendreceive)

string(REGEX REPLACE "(.*)/" "" THIS_PACKAGE_NAME "${CMAKE_CURRENT_SOURCE_DIR}")


if (${CMAKE_GENERATOR} MATCHES "Xcode")
         if (${XCODE_VERSION} VERSION_LESS 10)
              message(STATUS "Xcode 10 or higher is required. Please install from the Mac App Store.")
            return ()
        endif ()
endif ()


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

find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT AVUTIL AVDEVICE REQUIRED) #add here the list of ffmpeg components required
# memo: building barematal ffmpeg for macos
#  ./configure --prefix=/usr/local/Cellar/ffmpeg/4.2.2_2debug  --disable-avfoundation --disable-iconv --disable-filters --disable-devices --disable-shared --enable-static  --disable-optimizations  --disable-mmx --disable-audiotoolbox --disable-videotoolbox --disable-stripping   --disable-appkit --disable-zlib --disable-coreimage  --disable-bzlib --disable-sdl2 --disable-securetransport --disable-encoder=opus --disable-decoder=opus  --pkg-config-flags=--static --cc=clang --cxx=clang++
if(FFMPEG_FOUND)
#  FFMPEG_INCLUDE_DIRS  - Include directory necessary for using the required components headers.
#  FFMPEG_LIBRARIES     - Link these to use the required ffmpeg components.
#  FFMPEG_DEFINITIONS   - Compiler switches required for using the required ffmpeg components.
    message("FFMPEG_INCLUDE_DIRS = ${FFMPEG_INCLUDE_DIRS} ")
    message("FFMPEG_LIBRARIES = ${FFMPEG_LIBRARIES} ")
    message("FFMPEG_DEFINITIONS = ${FFMPEG_DEFINITIONS} ")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/rtpsendreceive_lib)

# Generate a project for every folder in the "source/projects" folder
SUBDIRLIST(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source/projects)
foreach (project_dir ${PROJECT_DIRS})
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source/projects/${project_dir}/CMakeLists.txt")
        message("Generating: ${project_dir}")
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/${project_dir})
    endif ()
endforeach ()
