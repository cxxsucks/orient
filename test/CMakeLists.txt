project(orientest)
set(CMAKE_CXX_STANDARD 17)

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} TEST_SRCS)

add_executable(orientest ${TEST_SRCS})
add_dependencies(orientest orie)
target_link_libraries(orientest orie ${GTEST_BOTH_LIBRARIES})

include(GoogleTest)
gtest_discover_tests(orientest)
