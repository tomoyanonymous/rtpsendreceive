# Copyright 2018 The Min-DevKit Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

cmake_minimum_required(VERSION 3.0)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)


#############################################################
# MAX EXTERNAL
#############################################################


include_directories( 
	"${C74_INCLUDES}" 
	"${FFMPEG_INCLUDE_DIR}" 
	${CMAKE_CURRENT_SOURCE_DIR}/../../rtpsendreceive_lib
)

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

message(STATUS testlib!${RTPSR_LIB})
target_link_libraries(${PROJECT_NAME} PUBLIC ${RTPSR_LIB})
target_link_libraries(${PROJECT_NAME} PUBLIC  ${FFMPEG_LIBRARIES})

include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)
message(STATUS ${CMAKE_SOURCE_DIR}/externals/rtpsendreceive.mc.rtpsend~.mxo)
execute_process(COMMAND ln -s ${CMAKE_SOURCE_DIR}/externals/rtpsendreceive.mc.rtpsend~.mxo ${CMAKE_SOURCE_DIR}/test_patch/)

#############################################################
# UNIT TEST
#############################################################

include(rtpsendreceive.mc.rtpsend_tilde_test.cmake)
