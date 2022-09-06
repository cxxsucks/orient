#pragma once
#include <orient/pred_tree/async_job.hpp>
#include <orient/fs/data_iter.hpp>
#include <shared_mutex>
#include <cassert>

namespace orie {

using fsearch_expr = orie::pred_tree::node<fs_data_iter, sv_t>;

class app {
    bool _conf_path_valid = false, 
         _auto_update_stopped = false;

    str_t _conf_path, _db_path;
    std::vector<str_t> _ignored_paths,
                       _start_paths;
    std::vector<std::pair<str_t, bool>> _root_paths;
    std::unique_ptr<std::byte[]> _data_dumped;

    fifo_thpool& _pool;
    std::unique_ptr<std::thread> _auto_update_thread;
    std::shared_mutex _data_dumped_mut;
    std::condition_variable _auto_update_cv;

public:
    // Magic number of database file. 32bit and 64bit versions are different.
    static constexpr uint64_t magic_num = 0x44f8a1ef44f8a1ef + sizeof(long);

    static app os_default(fifo_thpool& pool);
    static int main(int argc_exe, const char_t* const* argv_exe) noexcept;

    // Set path to database and read from it
    app& read_db(str_t path = str_t());
    // Scan filesystem, update database and write to the db file
    app& update_db(str_t path = str_t());

    // Auto updating
    template <class Rep, class Period>
    app& start_auto_update(
        const std::chrono::duration<Rep, Period>& interval);
    app& stop_auto_update();

    // Set pruned paths, i.e., paths that are skipped
    // THREAD UNSAFE
    app& add_ignored_path(str_t path);
    app& erase_ignored_path(const str_t& path);
    app& add_root_path(str_t path, bool multithreaded = false);
    app& erase_root_path(const str_t& path);
    app& add_start_path(str_t path);
    app& erase_start_path(const str_t& path);

    // Read or write config files. Use the most recently passed
    // parameter if it is empty. THREAD UNSAFE
    app& read_conf(str_t path = str_t());
    app& write_conf(str_t path = str_t());
    // Returns whether the config path is valid
    operator bool() const noexcept { return _conf_path_valid; }

    template <class callback_t, class Rep, class Period> 
    void run_pooled(fsearch_expr& expr, callback_t callback,
                    const std::chrono::duration<Rep, Period>& timeout = 
                    std::chrono::hours(1));
    template <class callback_t>
    void run(fsearch_expr& expr, callback_t callback);

    app(fifo_thpool& pool);
    app(app&&);
    app& operator=(app&&);
    app(const app&) = delete;
    app& operator=(const app&) = delete;
    ~app() { 
        stop_auto_update(); 
        std::unique_lock __lck(_data_dumped_mut);
    }
};

template <class cb_t>
void app::run(fsearch_expr& expr, cb_t callback) {
    for (sv_t p : _start_paths) {
        fs_data_iter it(_data_dumped.get(), p);
        if (it == it.end())
            continue;
        while (it != it.end()) {
            if (expr.apply_blocked(it))
                callback(it);
            ++it;
        }
    }
}

template <class callback_t, class Rep, class Period> 
void app::run_pooled(fsearch_expr& expr, callback_t callback,
                     const std::chrono::duration<Rep, Period>& timeout)
{
    std::vector<
        std::unique_ptr< pred_tree::async_job<fs_data_iter, sv_t>>
    > jobs;
    jobs.reserve(_start_paths.size());
    std::shared_lock __lck(_data_dumped_mut);

    // Construct jobs
    for (sv_t p : _start_paths) {
        fs_data_iter it(_data_dumped.get(), p);
        if (it == it.end()) // Invalid starting path
            continue;

        jobs.emplace_back(std::make_unique<pred_tree::async_job<fs_data_iter, sv_t>>(
            it, it.end(), expr, _pool, callback, false
        ))->cancel(timeout);
        // Would quickly finish after timeout reached
    }

    for (auto& j : jobs)
        j->join();
}

template <class Rep, class Period>
app& app::start_auto_update(
    const std::chrono::duration<Rep, Period>& interval)
{
    if (_auto_update_thread != nullptr)
        stop_auto_update();
    _auto_update_stopped = false;
    assert(_auto_update_thread == nullptr);

    _auto_update_thread.reset(new std::thread( [this, interval] () {
        while (!_auto_update_stopped) {
            update_db();
            std::unique_lock __lck(_data_dumped_mut);
            if (!_auto_update_stopped)
                _auto_update_cv.wait_for(__lck, interval);
        }
    }));
    return *this;
}

}