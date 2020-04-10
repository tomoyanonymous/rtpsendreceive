# Copyright 2018 The Min-DevKit Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

cmake_minimum_required(VERSION 3.0)
set(PROJECT_NAME rtpsendreceive_lib)

project(rtpsendreceive_lib)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(CMAKE_MACOSX_RPATH 1)
endif()

include_directories( 
	"${FFMPEG_INCLUDE_DIR}" 
)

add_library( 
	rtpsendreceive
	SHARED
	rtpsendreceive_lib.cpp
)
target_link_directories(rtpsendreceive PUBLIC "/usr/local/lib")
target_link_libraries(rtpsendreceive ${FFMPEG_LIBRARIES})
include(rtpsendreceive_lib_test.cmake)
