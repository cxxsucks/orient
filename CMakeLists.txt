cmake_minimum_required(VERSION 3.14)
project(orient VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/fs_pred_tree FS_PRED_TREE_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/fs FS_SRCS)

option(NEED_TEST "Build Google Test for Orient Library" OFF)

if(WIN32)
add_library(orie
    ${FS_SRCS} ${FS_PRED_TREE_SRCS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dirent_win.c
)
install(FILES include/dirent_win.h DESTINATION include)
add_executable(orient ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)
target_link_libraries(orient pcre2-16)

else()
add_library(orie
    ${FS_SRCS} ${FS_PRED_TREE_SRCS}
)
add_executable(orient ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)
target_link_libraries(orient pcre2-8)
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
