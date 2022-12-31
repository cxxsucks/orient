project(orientest)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG release-1.12.1
    FIND_PACKAGE_ARGS NAMES GTest)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} TEST_SRCS)

add_executable(orientest ${TEST_SRCS})
target_include_directories(orientest PUBLIC ${GTEST_INCLUDE_DIRS})
add_dependencies(orientest orie)
target_link_libraries(orientest PRIVATE
                      orie GTest::gtest_main)
if(WIN32)
target_link_libraries(orientest PRIVATE pcre2-16 rapidfuzz::rapidfuzz)
else(WIN32)
target_link_libraries(orientest PRIVATE pcre2-8 rapidfuzz::rapidfuzz)
endif(WIN32)

include(GoogleTest)
gtest_discover_tests(orientest)
