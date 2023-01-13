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
    std::shared_ptr<std::byte[]> _data_dumped;

public:
    // Since _pool is a reference which cannot be modified to point to
    // another object, instead of a getter, it is made public.
    fifo_thpool& _pool;

private:
    std::unique_ptr<std::thread> _auto_update_thread;
    // Mutex of all members, mainly for "_data_dumped"
    std::shared_mutex _member_mut;
    std::condition_variable_any _auto_update_cv;

public:
    // Magic number of database file. 32bit and 64bit versions are different.
    static constexpr uint64_t magic_num = 0x44f8a1ef44f8a1ef + sizeof(long);

    static app os_default(fifo_thpool& pool);

    // Set path to database and read from it
    app& read_db(str_t path = str_t());
    // Scan filesystem, update database and write to the db file
    app& update_db(str_t path = str_t());
    app& set_db_path(str_t path) {
        std::unique_lock __lck(_member_mut);
        _db_path = path;
        return *this;
    }

    // Auto updating
    template <class Rep, class Period>
    app& start_auto_update(const std::chrono::duration<Rep, Period>& interval,
                           bool update_immediate);
    // CANNOT BE CALLED SIMUTANEOUSLY ON MULTIPLE THREADS!
    app& stop_auto_update();

    // Set pruned paths, i.e., paths that are skipped; thread safe
    app& add_ignored_path(str_t path);
    app& erase_ignored_path(const str_t& path);
    app& add_root_path(str_t path, bool multithreaded = false);
    app& erase_root_path(const str_t& path);
    app& add_start_path(str_t path);
    app& erase_start_path(const str_t& path);

    // Getters
    const str_t& db_path() const noexcept { return _db_path; }
    const str_t& conf_path() const noexcept { return _conf_path; }

    const std::vector<str_t>& 
    ignored_paths() const noexcept { return _ignored_paths; }
    const std::vector<str_t>& 
    start_paths() const noexcept { return _start_paths; }
    const std::vector<std::pair<str_t, bool>>&
    root_paths() const noexcept { return _root_paths; }

    // Read or write config files. Use the most recently passed
    // parameter if it is empty. THREAD UNSAFE
    app& read_conf(str_t path = str_t());
    app& write_conf(str_t path = str_t());
    // Returns whether the config path is valid
    operator bool() const noexcept { return _conf_path_valid; }

    // To keep jobs safe after updatedb, which resets dumped data in
    // app class, shared pointer to originally dumped data is stored
    // along with the job itself.
    typedef std::vector<std::pair<
        std::shared_ptr<std::byte[]>, 
        // Use unique_ptr simply because async_job is not movable
        std::unique_ptr<pred_tree::async_job<fs_data_iter, sv_t>>
    >> job_list;
    template <class callback_t>
    void run(fsearch_expr& expr, callback_t callback);
    template <class callback_t> 
    void run_pooled(fsearch_expr& expr, callback_t callback);
    job_list get_jobs(fsearch_expr& expr);

    app(fifo_thpool& pool);
    app(app&&) noexcept;
    app& operator=(app&&) noexcept;
    app(const app&) = delete;
    app& operator=(const app&) = delete;
    ~app();
};

template <class cb_t>
void app::run(fsearch_expr& expr, cb_t callback) {
    std::shared_lock __lck(_member_mut);
    for (sv_t p : _start_paths) {
        fs_data_iter it(_data_dumped.get(), p);
        if (it == it.end())
            continue;
        try {
            while (it != it.end()) {
                if (expr.apply_blocked(it))
                    callback(it);
                ++it;
            }
        } catch(const pred_tree::quitted&) { }
    }
}

template <class callback_t> 
void app::run_pooled(fsearch_expr& expr, callback_t callback) {
    job_list jobs = get_jobs(expr);
    for (auto& j : jobs)
        j.second->start(_pool, callback);
    for (auto& j : jobs)
        j.second->join();
}

template <class Rep, class Period>
app& app::start_auto_update(const std::chrono::duration<Rep, Period>& interval,
                            bool immediate)
{
    if (_auto_update_thread != nullptr)
        stop_auto_update();
    _auto_update_stopped = false;
    assert(_auto_update_thread == nullptr);

    _auto_update_thread.reset(new std::thread( [this, interval, immediate] () {
        bool not_first = false;
        while (!_auto_update_stopped) {
            // Do not update if first time inside the loop and
            // an immediate updatedb is not requested
            if (not_first || immediate) {
                read_conf();
                update_db();
            }
            not_first = true;
            std::unique_lock __lck(_member_mut);
            _auto_update_cv.wait_for(__lck, interval, 
                [this] () {return _auto_update_stopped;});
        }
    }));
    return *this;
}

}