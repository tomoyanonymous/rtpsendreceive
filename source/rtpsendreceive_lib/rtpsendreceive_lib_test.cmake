cmake_minimum_required(VERSION 3.0)
set(TEST_NAME rtpsr_classes_test)
# project(${PROJECT_NAME})
set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../min-api)

enable_testing()

include_directories( 
	"${C74_INCLUDES}"
	"${C74_MIN_API_DIR}/test"
	# "${C74_MIN_API_DIR}/test/mock"
)

add_definitions(
	-DMIN_TEST
)
add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
set_property(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)


target_link_libraries(${TEST_NAME} PUBLIC "mock_kernel" rtpsendreceive)

add_test(NAME ${TEST_NAME}
        COMMAND ${TEST_NAME})