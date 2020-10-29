# Copyright 2018 The Min-DevKit Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

cmake_minimum_required(VERSION 3.0)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum OS X deployment version" FORCE)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(CMAKE_MACOSX_RPATH 1)
endif()

message("test!!! ${DEPENDENT_LIBS}")


add_library( 
	rtpsendreceive
	STATIC
	rtpsrbase.cpp rtpsender.cpp rtpreceiver.cpp
	)
target_include_directories(rtpsendreceive 
PUBLIC
${FFMPEG_INCLUDE_DIRS})

target_compile_features(rtpsendreceive PUBLIC cxx_std_17)
target_link_libraries(rtpsendreceive PUBLIC  ${DEPENDENT_LIBS})

include(rtpsendreceive_lib_test.cmake)
