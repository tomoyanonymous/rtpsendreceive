# Copyright 2018 The Min-DevKit Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

cmake_minimum_required(VERSION 3.0)

set(C74_MIN_API_DIR ${CMAKE_SOURCE_DIR}/source/min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum OS X deployment version" FORCE)


#############################################################
# MAX EXTERNAL
#############################################################


set( SOURCE_FILES
	${PROJECT_NAME}.cpp
)
# set(RTPSRLIB ${})
add_library( 
	${PROJECT_NAME} 
	MODULE
	${SOURCE_FILES}
)

set(RTPSR_LIB "rtpsendreceive")

target_include_directories(${PROJECT_NAME} PUBLIC
${C74_INCLUDES}
${CMAKE_SOURCE_DIR}/source/rtpsendreceive_lib
)

message(STATUS testlib!${RTPSR_LIB})
target_link_libraries(${PROJECT_NAME} PUBLIC ${RTPSR_LIB} ${FFMPEG_LIBRARIES})

include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)
message(STATUS ${CMAKE_SOURCE_DIR}/externals/mc.rtpsend~.mxo)
execute_process(COMMAND ln -s ${CMAKE_SOURCE_DIR}/externals/mc.rtpsend~.mxo ${CMAKE_SOURCE_DIR}/test_patch/)

#############################################################
# UNIT TEST
#############################################################

include(mc.rtpsend_tilde_test.cmake)
