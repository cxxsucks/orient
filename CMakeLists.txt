cmake_minimum_required(VERSION 3.14)
project(orient VERSION 0.3.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ORIE_TEST "Build Google Test for Orient Library" OFF)
option(ORIE_SYSTEM_PCRE2 "Use System PCRE2 Library" OFF)
option(ORIE_SYSTEM_RAPIDFUZZ "Use System rapidfuzz Library" OFF)
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
    # By default PCRE2 build static lib only
    FetchContent_MakeAvailable(pcre2)
endif(NOT ORIE_SYSTEM_PCRE2)
set(orie_pcre2_libname pcre2-)

if (NOT ORIE_SYSTEM_RAPIDFUZZ)
    FetchContent_Declare(rapidfuzz
        GIT_REPOSITORY https://github.com/maxbachmann/rapidfuzz-cpp.git
        GIT_TAG main)
    FetchContent_MakeAvailable(rapidfuzz)
else (NOT ORIE_SYSTEM_RAPIDFUZZ)
    find_package(rapidfuzz REQUIRED)
endif (NOT ORIE_SYSTEM_RAPIDFUZZ)

# List common sources
set(s ${CMAKE_CURRENT_SOURCE_DIR}/src/fs)
set(orie_src
    ${s}/app.cpp ${s}/data_iter.cpp
    ${s}/dumper_fs.cpp ${s}/dumper_mem.cpp)
set(s ${CMAKE_CURRENT_SOURCE_DIR}/src/fs_pred_tree)
set(orie_src ${orie_src}
    ${s}/action_node.cpp ${s}/content_node.cpp
    ${s}/iter_manip_node.cpp ${s}/name_node.cpp 
    ${s}/stat_node.cpp ${s}/fs_expr_builder.cpp
    src/util/file_mem_chunk.cpp
)
unset(s)

# Platform specific settings
if(WIN32)
    set(orie_src ${orie_src}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/util/dirent_win.c)
    install(FILES include/dirent_win.h DESTINATION include)
    set(orie_pcre2_libname ${orie_pcre2_libname}16)
else(WIN32)
    set(orie_pcre2_libname ${orie_pcre2_libname}8)
endif(WIN32)

add_library(orie ${orie_src})
add_library(orie::orie ALIAS orie)
add_executable(orient ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)
target_link_libraries(orie PUBLIC ${orie_pcre2_libname} rapidfuzz::rapidfuzz)
target_link_libraries(orient PRIVATE orie)
target_include_directories(orie PUBLIC 
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_include_directories(orient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(ORIE_TEST)
add_subdirectory(test)
endif(ORIE_TEST)
unset(orie_pcre2_libname)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
install(TARGETS orient DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp")

# Install liborie as CMake Module
set(ORIE_CMAKE_CONFIG_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/orie)
# Group all targets into a single exported target (only 1 in this case)
install(TARGETS orie EXPORT orieTargs DESTINATION ${CMAKE_INSTALL_LIBDIR})
# Generate *Config.cmake for each target (only 1 in this case)
configure_package_config_file(orieConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/orieConfig.cmake
    INSTALL_DESTINATION ${ORIE_CMAKE_CONFIG_DESTINATION})
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/orieConfigVersion.cmake
    COMPATIBILITY SameMinorVersion)

install(EXPORT orieTargs NAMESPACE orie:: 
        DESTINATION ${ORIE_CMAKE_CONFIG_DESTINATION})
install(TARGETS orie DESTINATION ${CMAKE_INSTALL_LIBDIR})
# Manually install *Config.cmake for each target (only 1 in this case)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/orieConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/orieConfigVersion.cmake
    DESTINATION ${ORIE_CMAKE_CONFIG_DESTINATION})

if(ORIE_LINK_STATIC)
    set_target_properties(orie PROPERTIES LINK_SEARCH_START_STATIC ON)
    set_target_properties(orie PROPERTIES LINK_SEARCH_END_STATIC ON)
    target_link_options(orie PRIVATE -static-libgcc -static-libstdc++ -static)
    set_target_properties(orient PROPERTIES LINK_SEARCH_START_STATIC ON)
    set_target_properties(orient PROPERTIES LINK_SEARCH_END_STATIC ON)
    target_link_options(orient PRIVATE -static-libgcc -static-libstdc++ -static)
endif(ORIE_LINK_STATIC)

target_compile_options(orie PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
