cmake_minimum_required(VERSION 3.0)
set(TEST_NAME rtpsr_classes_test)
# project(${PROJECT_NAME})
set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum OS X deployment version" FORCE)
enable_testing()

include_directories( 
	"${C74_INCLUDES}"
	"${C74_MIN_API_DIR}"
	"${C74_MIN_API_DIR}/test"
	# "${C74_MIN_API_DIR}/test/mock"
)


add_executable(${TEST_NAME} ${TEST_NAME}.cpp )
set_property(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
target_compile_definitions(
	${TEST_NAME}
	PUBLIC 
	-DMIN_TEST
)

target_link_libraries(${TEST_NAME} PUBLIC "mock_kernel" rtpsendreceive)

add_test(NAME ${TEST_NAME}
        COMMAND ${TEST_NAME})