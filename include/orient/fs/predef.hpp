#pragma once
#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif // _MSC_VER


extern "C" {
#if _WIN32
#include <orient/dirent_win.h>
#define FS_TEXT(str) L##str
// #define NATIVE_STDOUT std::wcout
// #define NATIVE_STDERR std::wcerr
#ifdef max
#undef max
#undef min
#endif
#else

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif // _DEFAULT_SOURCE

#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#define FS_TEXT(str) str
// #define NATIVE_STDOUT std::cout
// #define NATIVE_STDERR std::cerr
#endif
}

#include <cstring>
#include <cstdlib>
#include <iostream>

namespace orie {
#ifdef _WIN32
    using value_type = wchar_t;
    using dir_t = ::_WDIR;
    using dirent_t = ::_wdirent;
    typedef struct _stat stat_t; // using does not work on MSVC :(

    constexpr value_type seperator = L'\\';
    constexpr value_type reverse_sep = L'/';
    constexpr size_t path_max = 256;

    inline std::wostream& NATIVE_STDOUT = std::wcout;
    inline std::wostream& NATIVE_STDOUT = std::wcerr;

#else 
    using value_type = char;
    using dir_t = ::DIR;
    using dirent_t = ::dirent;
    using stat_t = struct stat;

    constexpr value_type seperator = '/';
    constexpr value_type reverse_sep = '\\';
    constexpr size_t path_max = PATH_MAX;

    inline std::ostream& NATIVE_STDOUT = std::cout;
    inline std::ostream& NATIVE_STDERR = std::cerr;

#endif
    constexpr value_type root_path_str[] = FS_TEXT("root_paths");
    constexpr value_type pruned_path_str[] = FS_TEXT("pruned_paths");
    constexpr value_type db_path_str[] = FS_TEXT("database_path");

    enum _category_tag : value_type {
        unknown_tag = 0,
        file_tag = 'f', dir_tag = 'd',
        dir_pop_tag = 'p', link_tag = 'l', 
        /*fifo_tag = 251, blk_tag = 250, sock_tag = 249*/
    };

#ifdef _WIN32
    //! Get file attributes for @c path and put them in @c buf
    inline int stat(const value_type* path, stat_t* buf) {
        return ::_wstat(path, buf);
    }

    //! @brief Open a directory stream.
    //! @param path The directory to be opened.
    //! @return A dir stream on the directory
    //! @retval NULL if it could not be opened.
    inline dir_t* opendir(const value_type* path) {
        return ::wopendir(path);
    }

    // Read next directory entry.
    inline dirent_t* readdir(dir_t* d) {
        return ::wreaddir(d);
    }

    //! @brief close a directory stream.
    //! @retval 0 successful @retval 1 failed
    inline int closedir(dir_t* d) {
        return ::wclosedir(d);
    }

    //! @brief Get the absolute name of @p src.
    //! @param [out] resolv Non-null buffer holding resolved path.
    //! @return @p resolv
    inline value_type* realpath(const value_type* src,
                                value_type* resolv, size_t buf_len) 
    {
        DWORD _buf_len = static_cast<DWORD>(buf_len);
        DWORD saved = ::GetFullPathNameW(src, _buf_len, resolv, nullptr);
        if (saved > _buf_len)
            return nullptr;
        if (resolv[saved - 1] == orie::seperator)
            resolv[saved - 1] = '\0';
        return resolv;
    }

#else
    //! Get file attributes for @c path and put them in @c buf
    inline int stat(const value_type* path, stat_t* buf) {
        return ::stat(path, buf);
    }

    //! @brief Open a directory stream.
    //! @param path The directory to be opened.
    //! @return A dir stream on the directory
    //! @retval NULL if it could not be opened.
    inline dir_t* opendir(const value_type* path) {
        return ::opendir(path);
    }

    // Read next directory entry.
    inline dirent_t* readdir(dir_t* d) {
        return ::readdir(d);
    }

    //! @brief close a directory stream.
    //! @retval 0 successful @retval 1 failed
    inline int closedir(dir_t* d) {
        return ::closedir(d);
    }

    //! @brief Get the absolute name of @p src.
    //! @param [out] resolv Non-null buffer holding resolved path.
    //! @return @p resolv
    inline value_type* realpath(const value_type* src,
                                value_type* resolv, size_t buf_len) 
    {
        if (buf_len >= PATH_MAX - 2)
            return ::realpath(src, resolv);
        char tmp_dest[PATH_MAX];
        if (::realpath(src, tmp_dest) == nullptr)
            return nullptr;
        ::strncpy(resolv, tmp_dest, buf_len);
        return resolv;
    }
#endif
};