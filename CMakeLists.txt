cmake_minimum_required(VERSION 3.14)
project(orient)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(NEED_TEST "Build Google Test for Orient Library" OFF)
option(SYSTEM_PCRE2 "Use System PCRE2 Library" OFF)

# Unfortunately PCRE2 provides no package name therefore whether to use system
# library can only be manually toggled unlike GTest.
if (NOT SYSTEM_PCRE2)
    include(FetchContent)
    FetchContent_Declare(pcre2
        GIT_REPOSITORY https://github.com/PCRE2Project/pcre2
        GIT_TAG pcre2-10.42)
    set(PCRE2_BUILD_PCRE2GREP CACHE INTERNAL OFF)
    set(PCRE2_BUILD_TESTS CACHE INTERNAL OFF)
    set(PCRE2_DISABLE_PERCENT_ZT CACHE INTERNAL ON)
    if (WIN32)
        set(PCRE2_BUILD_PCRE2_8 CACHE INTERNAL OFF)
        set(PCRE2_BUILD_PCRE2_16 CACHE INTERNAL ON)
    endif(WIN32)
    FetchContent_MakeAvailable(pcre2)
endif(NOT SYSTEM_PCRE2)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/fs_pred_tree FS_PRED_TREE_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/fs FS_SRCS)

if(WIN32)
add_library(orie
    ${FS_SRCS} ${FS_PRED_TREE_SRCS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dirent_win.c
)
install(FILES include/dirent_win.h DESTINATION include)
add_executable(orient ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)
target_link_libraries(orie pcre2-16)

else()
add_library(orie
    ${FS_SRCS} ${FS_PRED_TREE_SRCS}
)
add_executable(orient ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)
target_link_libraries(orie pcre2-8)
endif()

install(TARGETS orie DESTINATION lib)
install(DIRECTORY include/ DESTINATION include
        FILES_MATCHING PATTERN "*.hpp")

target_link_libraries(orient orie)
install(TARGETS orient DESTINATION bin)

if(NEED_TEST)
add_subdirectory(test)
endif()

target_compile_options(orie PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
