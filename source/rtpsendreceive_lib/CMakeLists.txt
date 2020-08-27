# Copyright 2018 The Min-DevKit Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

cmake_minimum_required(VERSION 3.0)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(CMAKE_MACOSX_RPATH 1)
endif()

message("test!!! ${DEPENDENT_LIBS}")
include_directories( 
	"${FFMPEG_INCLUDE_DIR}" 
)

# add_library( 
# 	rtpsendreceive
# 	STATIC
# 	rtpsrbase.cpp rtpsender.cpp rtpreceiver.cpp
# )
add_library(rtpsr_new
	STATIC rtpsr_classes.cpp
)
# target_link_libraries(rtpsendreceive 
# 						PUBLIC ${DEPENDENT_LIBS})
target_link_libraries(rtpsr_new PUBLIC  ${DEPENDENT_LIBS})

include(rtpsendreceive_lib_test.cmake)
