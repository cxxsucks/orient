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

add_executable(orientest 
    test_action_node.cc test_app.cc test_builder.cc
    test_content_node.cc test_data_iter.cc test_fs_builder.cc
    test_file_mem.cc test_node_base.cc test_stat_node.cc
    test_tokenize.cc test_cmprslib.cc test_arr2d.cc
    test_trigram.cc
)

target_include_directories(orientest PRIVATE ${GTEST_INCLUDE_DIRS})
add_dependencies(orientest orie)
target_link_libraries(orientest PRIVATE
    orie GTest::gtest
    ${orie_pcre2_libname} rapidfuzz::rapidfuzz
)

include(GoogleTest)
gtest_discover_tests(orientest)
