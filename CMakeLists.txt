cmake_minimum_required(VERSION 3.14)
project(orient VERSION 0.2.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ORIE_TEST "Build Google Test for Orient Library" OFF)
option(ORIE_SYSTEM_PCRE2 "Use System PCRE2 Library" OFF)
option(ORIE_LINK_STATIC "Statically link orient executable" OFF)

# Unfortunately PCRE2 provides no package name therefore whether to use system
# library can only be manually toggled unlike GTest.
include(FetchContent)
if (NOT ORIE_SYSTEM_PCRE2)
    FetchContent_Declare(pcre2
        GIT_REPOSITORY https://github.com/PCRE2Project/pcre2
        GIT_TAG pcre2-10.42)
    set(PCRE2_BUILD_PCRE2GREP OFF CACHE INTERNAL "")
    set(PCRE2_BUILD_TESTS OFF CACHE INTERNAL "")
    set(PCRE2_DISABLE_PERCENT_ZT ON CACHE INTERNAL "")
    if (WIN32)
        set(PCRE2_NEWLINE "CRLF" CACHE INTERNAL "")
        set(PCRE2_BUILD_PCRE2_8 OFF CACHE INTERNAL "")
        set(PCRE2_BUILD_PCRE2_16 ON CACHE INTERNAL "")
    endif(WIN32)
    FetchContent_MakeAvailable(pcre2)
endif(NOT ORIE_SYSTEM_PCRE2)

if (NOT ORIE_SYSTEM_RAPIDFUZZ)
    FetchContent_Declare(rapidfuzz
        GIT_REPOSITORY https://github.com/maxbachmann/rapidfuzz-cpp.git
        GIT_TAG main)
    FetchContent_MakeAvailable(rapidfuzz)
endif (NOT ORIE_SYSTEM_RAPIDFUZZ)

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
    target_link_libraries(orie PRIVATE pcre2-16 rapidfuzz::rapidfuzz)

else(WIN32)
    add_library(orie ${FS_SRCS} ${FS_PRED_TREE_SRCS})
    add_executable(orient ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)
    target_link_libraries(orie PRIVATE pcre2-8 rapidfuzz::rapidfuzz)
endif(WIN32)

install(TARGETS orie DESTINATION lib)
install(DIRECTORY include/ DESTINATION include
        FILES_MATCHING PATTERN "*.hpp")

if(ORIE_LINK_STATIC)
    set_target_properties(orie PROPERTIES LINK_SEARCH_START_STATIC ON)
    set_target_properties(orie PROPERTIES LINK_SEARCH_END_STATIC ON)
    target_link_options(orie PRIVATE -static-libgcc -static-libstdc++ -static)
    set_target_properties(orient PROPERTIES LINK_SEARCH_START_STATIC ON)
    set_target_properties(orient PROPERTIES LINK_SEARCH_END_STATIC ON)
    target_link_options(orient PRIVATE -static-libgcc -static-libstdc++ -static)
endif(ORIE_LINK_STATIC)

target_link_libraries(orient orie)
install(TARGETS orient DESTINATION bin)

if(ORIE_TEST)
add_subdirectory(test)
endif(ORIE_TEST)

target_compile_options(orie PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
