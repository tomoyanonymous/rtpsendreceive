cmake_minimum_required(VERSION 3.0)
set(PROJECT_NAME rtpsendreceive_lib_test)
project(${PROJECT_NAME})

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../min-api)

enable_testing()

include_directories( 
	"${C74_INCLUDES}"
	"${CMAKE_CURRENT_LIST_DIR}/../../min-api/test"
#	"${CMAKE_CURRENT_LIST_DIR}/mock"
)

add_definitions(
	-DMIN_TEST
)
add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC "mock_kernel")

add_test(NAME ${PROJECT_NAME}
        COMMAND ${PROJECT_NAME})