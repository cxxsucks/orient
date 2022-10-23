#pragma once
#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif // _MSC_VER

extern "C" {
#if _WIN32
#include <orient/util/dirent_win.h>
#define NATIVE_PATH(str) L##str
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
#define NATIVE_SV(str) orie::sv_t(NATIVE_PATH(str))

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

    constexpr char_t separator = L'\\';
    constexpr char_t reverse_sep = L'/';
    constexpr size_t path_max = 2048;

    inline std::wostream& NATIVE_STDOUT = std::wcout;
    inline std::wostream& NATIVE_STDOUT = std::wcerr;

#else 
    #define PCRE2_CODE_UNIT_WIDTH 8
    using dir_t = ::DIR;
    using dirent_t = ::dirent;
    using stat_t = struct stat;

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
    using icase_sv_t = std::basic_string_view<char_t, fs_icase_traits>;

    constexpr char_t root_path_str[] = NATIVE_PATH("root_paths");
    constexpr char_t pruned_path_str[] = NATIVE_PATH("pruned_paths");
    constexpr char_t db_path_str[] = NATIVE_PATH("database_path");

    enum category_tag : char_t {
        unknown_tag = 0,
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
    //! A redundant '\\' will be placed in @p resolv[0]
    //! @return @p resolv
    char_t* realpath(const char_t* src,
                            char_t* resolv, size_t buf_len) 
    {
        // -1 for the '\\' in output
        DWORD _buf_len = static_cast<DWORD>(buf_len) - 1;
        while (*src == separator)
            ++src;
        DWORD saved = ::GetFullPathNameW(src, _buf_len, resolv + 1, nullptr);
        if (saved > _buf_len)
            return nullptr;
        // Strip '\\'s at the end of resolved path
        if (resolv[saved] == orie::separator)
            resolv[saved] = '\0';
        resolv[0] = separator;
        return resolv;
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

    //! @brief Get the absolute name of @p src.
    //! @param [out] resolv Non-null buffer holding resolved path.
    //! @return @p resolv
    inline char_t* realpath(const char_t* src,
                            char_t* resolv, size_t buf_len) 
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