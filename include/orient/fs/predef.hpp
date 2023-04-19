#pragma once

extern "C" {
#if _WIN32
#include <shlwapi.h>
#ifdef _MSC_VER
#pragma comment(lib, "Shlwapi.lib")
#pragma warning(disable: 4996)
#endif

#include <orient/util/dirent_win.h>
#define NATIVE_PATH(str) L##str
#ifdef max
#undef max
#undef min
#endif

#else
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <fnmatch.h>
#define NATIVE_PATH(str) str
#endif
}

#define NATIVE_SV(str) orie::sv_t(NATIVE_PATH(str))
#ifndef __likely
#ifdef __GNUC__
#define __likely(x) __builtin_expect(!!(x), 1)
#define __unlikely(x) __builtin_expect(!!(x), 0)
#else
#define __likely(x)
#define __unlikely(x)
#endif
#endif

#include <cstring>
#include <cstdlib>
#include <iostream>

namespace orie {
#ifdef _WIN32
    #define PCRE2_CODE_UNIT_WIDTH 16
    using dir_t = ::_WDIR;
    using dirent_t = ::_wdirent;
    typedef struct _stat stat_t; // using does not work on MSVC :(
    using ssize_t = ptrdiff_t;

    using fs_char_traits = std::char_traits<char>;
    using sv_t = std::wstring_view;
    using str_t = std::wstring;
    using char_t = wchar_t;

    constexpr char_t separator = L'\\';
    constexpr char_t reverse_sep = L'/';
    constexpr size_t path_max = 2048;

    inline std::wostream& NATIVE_STDOUT = std::wcout;
    inline std::wostream& NATIVE_STDERR = std::wcerr;

// For _waccess
    enum { R_OK = 4, W_OK = 2, X_OK = 1 };

#else 
    #define PCRE2_CODE_UNIT_WIDTH 8
    using dir_t = ::DIR;
    using dirent_t = ::dirent;
    using stat_t = struct stat;
    using ssize_t = ssize_t;

    using fs_char_traits = std::char_traits<char>;
    using sv_t = std::string_view;
    using str_t = std::string;
    using char_t = char;
    constexpr char_t separator = '/';
    constexpr char_t reverse_sep = '\\';
    constexpr size_t path_max = PATH_MAX;

    inline std::ostream& NATIVE_STDOUT = std::cout;
    inline std::ostream& NATIVE_STDERR = std::cerr;
#endif

    using uid_t = decltype(stat_t::st_uid);
    using gid_t = decltype(stat_t::st_gid);
    using ino_t = decltype(stat_t::st_ino);
    using mode_t = decltype(stat_t::st_mode);

    // https://en.cppreference.com/w/cpp/string/char_traits
    // Used in case-ignored comparison
    struct fs_icase_traits : public std::char_traits<char_t> {
        static char_t to_upper(char_t ch) {
            if constexpr (sizeof(char_t) == 1)
                return ::toupper(ch);
            else return ::towupper(ch);
        }
        static bool eq(char_t c1, char_t c2) {
            return to_upper(c1) == to_upper(c2);
        }
        static bool lt(char_t c1, char_t c2) {
            return to_upper(c1) < to_upper(c2);
        }
        static int compare(const char_t *s1, const char_t *s2, size_t n) {
            while (n-- != 0) {
                if (to_upper(*s1) < to_upper(*s2)) return -1;
                if (to_upper(*s1) > to_upper(*s2)) return 1;
                ++s1; ++s2;
            }
            return 0;
        }
        static const char_t* find(const char_t* s, size_t n, char_t a) {
            auto const ua(to_upper(a));
            while (n-- != 0) {
                if (to_upper(*s) == ua)
                    return s;
                ++s;
            }
            return nullptr;
        }
    };
    using icase_sv_t = std::basic_string_view<char_t, fs_icase_traits>;

    enum category_tag : uint8_t {
        unknown_tag = 0, data_end_tag = unknown_tag,
        next_chunk_tag = '+', next_group_tag = '*',
        file_tag = 'f', dir_tag = 'd',
        link_tag = 'l', char_tag = 'c',
        fifo_tag = 'p', blk_tag = 'b', sock_tag = 's',
        dir_pop_tag = '-'
    };

// Syscall Wrappers. 
// In orient's context, Windows paths may start with a redundant '\\'
// which is dropped when invoking the "syscall".
#ifdef _WIN32
    //! Get file attributes for @c path and put them in @c buf
    inline int stat(const char_t* path, stat_t* buf) {
        while (*path == separator)
            ++path;
        return ::_wstat(path, buf);
    }

    //! @brief Open a directory stream.
    //! @param path The directory to be opened, with an optional starting '\\'
    //! @return A dir stream on the directory
    //! @retval NULL if it could not be opened.
    inline dir_t* opendir(const char_t* path) {
        while (*path == separator)
            ++path;
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

    //! @brief Get the absolute name of @p src, with an optional starting '\\'.
    //! @param [out] resolv Non-null buffer holding resolved path.
    //! @b NO redundant '\\' will be placed in @p resolv[0]
    //! @return Size written to @p resolv not including '\0'
    //! @retval -1 Failure or buffer too small
    inline ssize_t realpath(const char_t* src,
                            char_t* resolv, size_t buf_len) 
    {
        DWORD _buf_len = static_cast<DWORD>(buf_len);
        while (*src == separator)
            ++src;
        if (*src == L'\0') { // orient's fake root path
            resolv[0] = separator;
            resolv[1] = L'\0';
            return 1;
        }

        HANDLE hdl = ::CreateFileW(src, GENERIC_READ, FILE_SHARE_READ, nullptr,
                                   OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS,
                                   nullptr);
        if (hdl == INVALID_HANDLE_VALUE)
            return -1;
        DWORD saved = ::GetFinalPathNameByHandleW(hdl, resolv, _buf_len,
                                                  VOLUME_NAME_DOS);
        ::CloseHandle(hdl);
        if (saved >= _buf_len || saved == 0)
            return -1;

        // On success GetFullPathNameW does not include '\0'
        // Strip the "\\\\?\\" part
        saved -= 4;
        // The '\0' also have to be memmoved
        ::memmove(resolv, resolv + 4, saved * sizeof(wchar_t) + sizeof(wchar_t));
        // Strip '\\' at the end of resolved path
        if (resolv[saved - 1] == orie::separator)
            resolv[--saved] = L'\0';
        return static_cast<ssize_t>(saved);
    }

    //! @brief Unix fnmatch(3) and Windows PathMatchSpecW wrapper.
    inline bool glob_match(const char_t* needle, const char_t* haystack, bool) {
        return *haystack == separator ? PathMatchSpecW(haystack + 1, needle)
                                      : PathMatchSpecW(haystack, needle);
    }

#else // Not Windows. Then these functions are just wrappers.
    //! Get file attributes for @c path and put them in @c buf
    inline int stat(const char_t* path, stat_t* buf) {
        return ::stat(path, buf);
    }

    //! @brief Open a directory stream.
    //! @param path The directory to be opened.
    //! @return A dir stream on the directory
    //! @retval NULL if it could not be opened.
    inline dir_t* opendir(const char_t* path) {
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

    //! @brief Get the absolute name of @p src, but return path length.
    //! @param [out] resolv Non-null buffer holding resolved path.
    //! @param buf_len Length of @p resolv including '\0'
    //! @return Size written ro @p resolv not including '\0'
    //! @retval ~size_t() on fail
    inline ssize_t realpath(const char_t* src,
                            char_t* resolv, size_t buf_len) 
    {
        if (buf_len >= PATH_MAX) {
            if (::realpath(src, resolv) == nullptr)
                return -1;
        } else {
            char tmp_dest[PATH_MAX];
            if (::realpath(src, tmp_dest) == nullptr)
                return -1;
            // -1 because buf_len includes NULL
            ::strncpy(resolv, tmp_dest, buf_len - 1);
            resolv[buf_len - 1] = '\0';
        }
        return ::strlen(resolv);
    }

    inline bool glob_match(const char_t* needle, const char_t* haystack, bool icase) {
        return ::fnmatch(needle, haystack, icase ? FNM_CASEFOLD : 0) == 0;
    }
#endif
};