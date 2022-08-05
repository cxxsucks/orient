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
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#define NATIVE_PATH(str) str
#endif
}
#define NATIVE_PATH_SV(str) orie::sv_t(NATIVE_PATH(str))

#include <cstring>
#include <cstdlib>
#include <iostream>

namespace orie {
#ifdef _WIN32
    #define PCRE2_CODE_UNIT_WIDTH 16
    using dir_t = ::_WDIR;
    using dirent_t = ::_wdirent;
    typedef struct _stat stat_t; // using does not work on MSVC :(

    using fs_char_traits = std::char_traits<char>;
    using sv_t = std::wstring_view;
    using str_t = std::wstring;
    using char_t = wchar_t;

    constexpr value_type seperator = L'\\';
    constexpr value_type reverse_sep = L'/';
    constexpr size_t path_max = 256;

    inline std::wostream& NATIVE_STDOUT = std::wcout;
    inline std::wostream& NATIVE_STDOUT = std::wcerr;

#else 
    #define PCRE2_CODE_UNIT_WIDTH 8
    using value_type = char;
    using dir_t = ::DIR;
    using dirent_t = ::dirent;
    using stat_t = struct stat;

    using fs_char_traits = std::char_traits<char>;
    using sv_t = std::string_view;
    using str_t = std::string;
    using char_t = char;
    constexpr value_type seperator = '/';
    constexpr value_type reverse_sep = '\\';
    constexpr size_t path_max = PATH_MAX;

    inline std::ostream& NATIVE_STDOUT = std::cout;
    inline std::ostream& NATIVE_STDERR = std::cerr;
#endif

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
        static const char_t* find(const char* s, size_t n, char a) {
            auto const ua(to_upper(a));
            while (n-- != 0) {
                if (to_upper(*s) == ua)
                    return s;
                ++s;
            }
            return nullptr;
        }
    };

    constexpr value_type root_path_str[] = NATIVE_PATH("root_paths");
    constexpr value_type pruned_path_str[] = NATIVE_PATH("pruned_paths");
    constexpr value_type db_path_str[] = NATIVE_PATH("database_path");

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