# Copyright 2018 The Min-DevKit Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

cmake_minimum_required(VERSION 3.0)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(CMAKE_MACOSX_RPATH 1)
endif()

include_directories( 
	"${FFMPEG_INCLUDE_DIR}" 
)

add_library( 
	rtpsendreceive 
	STATIC
	rtpsendreceive_lib
)
