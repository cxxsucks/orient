#include <orient/app.hpp>
#include <orient/fs_pred_tree/fs_expr_builder.hpp>

#ifdef _MSC_VER
int wmain(int exe_argc, wchar_t** exe_argv) 
#else
int main(int exe_argc, char** exe_argv) 
#endif
{
    int expr_since = 1;
    orie::fifo_thpool pool;
    orie::app app(pool);
    while (expr_since < exe_argc) {
        // -conf is the only global option implemented :(
        // More would be added if a non-bloated argparser is found :(
        // (No hashmap, set, string or vector; only string_view, array, ...)
        if (NATIVE_SV("-conf") == exe_argv[expr_since]) {
            if (expr_since + 1 == exe_argc || 
                !app.read_conf(exe_argv[expr_since + 1])) {
                orie::NATIVE_STDOUT << "Unable to read configuration.\n";
                return 3;
            } else {
                expr_since += 2;
                continue;
            }
        }

        // TODO: Test existence and type of a starting path
        app.add_start_path(exe_argv[expr_since]);
        ++expr_since;
    }

    orie::pred_tree::fs_expr_builder builder;
    builder.build(exe_argc - expr_since + 1, exe_argv + expr_since - 1);
    bool has_action = builder.has_action();
    auto callback = [has_action] (orie::fs_data_iter& it) {
        static std::mutex out_mut;
        if (!has_action) {
            std::lock_guard __lck(out_mut);
            orie::NATIVE_STDOUT << it.path() << '\n';
        }
    };

    if (builder.has_async()) {
        // TODO: -content-timeout field
        app.run_pooled(*builder.get(), callback, std::chrono::hours(1));
    } else {
        app.run(*builder.get(), callback);
    }
    return 0;
}
