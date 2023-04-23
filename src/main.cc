#include <orient/app.hpp>
#include <orient/fs_pred_tree/fs_expr_builder.hpp>
#define ERR_N_DIE(msg, retval) do { \
    orie::NATIVE_STDOUT<<NATIVE_PATH(msg); \
    return retval; \
} while (false)

#ifdef _WIN32
#ifdef _MSC_VER
int wmain(int argc, const wchar_t* const* argv) noexcept {
#else // Win32 not MSVC
extern "C" {
#include <shellapi.h>
}
int main() noexcept {
    int argc;
    wchar_t** argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
#endif
#else // Not Win32
int main(int argc, const char* const* argv) noexcept {
#endif
    // No C-style buffered write to stdout in all of the code
    std::ios_base::sync_with_stdio(false);

try {
    int expr_since = 1;
    bool updatedb_flag = false, startpath_flag = false;
    orie::fifo_thpool pool;
    orie::app app(orie::app::os_default(pool));
    while (expr_since < argc) {
        // -conf is the only global option implemented :(
        // More would be added if a non-bloated argparser is found :(
        // (No hashmap, set, string or vector; only string_view, array, ...)
        if (NATIVE_SV("-conf") == argv[expr_since]) {
            if (expr_since + 1 == argc || !app.read_conf(argv[expr_since + 1])) {
                ERR_N_DIE("Unable to read configuration.\n", 3);
            } else {
                expr_since += 2;
                continue;
            }
        } else if (NATIVE_SV("-updatedb") == argv[expr_since]) {
            updatedb_flag = true;
            ++expr_since;
            continue;
        }

        // Expression start with either '-' or one of "(" and "!"
        if (argv[expr_since][0] == orie::char_t('-') ||
            argv[expr_since] == NATIVE_SV("!") ||
            argv[expr_since] == NATIVE_SV("("))
            break;
        orie::char_t realpath_buf[orie::path_max] = {};
        orie::ssize_t realpath_len =
            orie::realpath(argv[expr_since], realpath_buf, orie::path_max);
        if (realpath_len < 0 || realpath_len >= orie::path_max)
            orie::NATIVE_STDERR << argv[expr_since]
                                << NATIVE_PATH(": No such directory\n");
        else app.add_start_path(realpath_buf);
        startpath_flag = true;
        ++expr_since;
    }

    // Parsing arguments finished; start doing stuff
    if (updatedb_flag)
        app.update_db();
    if (!app.has_data())
        ERR_N_DIE("Database not initialized. Please run with "
                  "-updatedb first.\n", 4);

    orie::pred_tree::fs_expr_builder builder;
    if (expr_since == argc)
        builder.build(updatedb_flag ? 
            NATIVE_SV("-false") : NATIVE_SV("-true"));
    else builder.build(argc - expr_since + 1, argv + expr_since - 1);
    bool has_action = builder.has_action();

    auto callback = [has_action] (orie::fs_data_iter& it) {
        static std::mutex out_mut;
        if (!has_action) {
            std::lock_guard __lck(out_mut);
#ifdef _WIN32
            // Do not print trailing '\\'
            orie::NATIVE_STDOUT << it.path().c_str() + 1 << '\n';
#else
            orie::NATIVE_STDOUT << it.path() << '\n';
#endif
        }
    };

    if (!startpath_flag) {
        orie::char_t cwd_buf[orie::path_max];
#ifdef _WIN32
        // TODO: cwd is longer than path_max
        ::GetCurrentDirectoryW(orie::path_max, cwd_buf);
        app.add_start_path(cwd_buf);
#else
        app.add_start_path(::getcwd(cwd_buf, orie::path_max));
#endif
    }
    app.run(*builder.get(), callback);

} catch (std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
}
    return 0;
}
