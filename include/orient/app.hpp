#pragma once
#include <orient/pred_tree/async_job.hpp>
#include <orient/fs/data_iter.hpp>
#include <cassert>

namespace orie {

using fsearch_expr = orie::pred_tree::node<fs_data_iter, sv_t>;

class app {
    bool _auto_update_stopped = false;

    str_t _conf_path;
    std::vector<str_t> _start_paths;
    std::shared_ptr<dmp::dumper> _dumper;

public:
    // Since _pool is a reference which cannot be modified to point to
    // another object, instead of a getter, it is made public.
    fifo_thpool& _pool;

private:
    std::thread _auto_update_thread;
    // Mutex of special paths
    std::mutex _paths_mut;
    std::condition_variable _auto_update_cv;

    void handle_connection_impl(int connfd);

public:
    static app os_default(fifo_thpool& pool);
    static void rectify_path(orie::str_t& path);

    // Set path to database and read from it
    // 0.4: No more read_db; reading is handled on the fly by dumper
    // app& read_db(str_t path = str_t());

    // Scan filesystem, update database and write to the db file
    app& update_db();
    app& set_db_path(const char_t* path);

    // Auto updating
    template <class Rep, class Period>
    app& start_auto_update(const std::chrono::duration<Rep, Period>& interval,
                           bool update_immediate);
    // CANNOT BE CALLED SIMUTANEOUSLY ON MULTIPLE THREADS!
    app& stop_auto_update();

    // Set special paths; thread safe
    app& add_ignored_path(str_t path);
    app& erase_ignored_path(const str_t& path);
    app& add_slow_path(str_t path);
    app& erase_slow_path(const str_t& path);
    app& set_root_path(str_t path);
    app& add_start_path(str_t path);
    app& erase_start_path(const str_t& path);

    // Getters
    const str_t& db_path() const noexcept {
        return _dumper->fwdidx_path();
    }
    const str_t& root_path() const noexcept { return _dumper->_root_path; }
    const str_t& conf_path() const noexcept { return _conf_path; }

    const std::vector<str_t>& 
    slow_paths() const noexcept { return _dumper->_noconcur_paths; }
    const std::vector<str_t>& 
    ignored_paths() const noexcept { return _dumper->_pruned_paths; }
    const std::vector<str_t>& 
    start_paths() const noexcept { return _start_paths; }

    // Read or write config files. Use the most recently passed
    // parameter if it is empty. THREAD UNSAFE
    app& read_conf(str_t path = str_t());
    app& write_conf(str_t path = str_t());

    // Returns whether the app object is in valid state
    // Paths can only be set in valid state
    operator bool() const noexcept { return _dumper != nullptr; }
    // Returns whether the app object is in valid state
    // Paths can only be set in valid state
    bool valid() const noexcept { return _dumper != nullptr; }
    bool has_data() const noexcept {
        return _dumper != nullptr && _dumper->chunk_count() != 0;
    }

    // To keep jobs safe after updatedb, which resets dumped data in
    // app class, shared pointer to previous data is stored along with
    // the job itself.
    typedef std::vector<std::pair<
        std::shared_ptr<dmp::dumper>, 
        // Use unique_ptr simply because async_job is not movable
        std::unique_ptr<pred_tree::async_job<fs_data_iter, sv_t>>
    >> job_list;

    template <class callback_t>
    void run(fsearch_expr& expr, callback_t callback);
    job_list get_jobs(fsearch_expr& expr);

    app(fifo_thpool& pool);
    app(app&&) noexcept;
    app& operator=(app&&) noexcept;
    app(const app&) = delete;
    app& operator=(const app&) = delete;
    ~app();
};

template <class callback_t> 
void app::run(fsearch_expr& expr, callback_t callback) {
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
    stop_auto_update();
    _auto_update_stopped = false;
    assert(!_auto_update_thread.joinable());

    _auto_update_thread = std::thread( [this, interval, immediate] () {
        bool not_first = false;
        while (!_auto_update_stopped) {
            // Do not update if first time inside the loop and
            // an immediate updatedb is not requested
            if (not_first || immediate) {
                try {
                    if (!_conf_path.empty())
                        read_conf();
                    update_db();
                } catch(const std::runtime_error& e) {
                    // Cannot throw here
                    NATIVE_STDERR << "Error during orient auto updatedb: "
                                  << e.what() << NATIVE_PATH('\n');
                }
            }
            not_first = true;
            std::unique_lock __lck(_paths_mut);
            _auto_update_cv.wait_for(__lck, interval, 
                [this] () {return _auto_update_stopped;});
        }
    });
    return *this;
}

}