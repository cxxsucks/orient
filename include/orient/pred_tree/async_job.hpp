#pragma once
#include "./node.hpp"
#include <orient/util/fifo_thpool.hpp>

// void f(iter_t&)

namespace orie {
namespace pred_tree {

template <class iter_t, class sv_t>
class async_job {
    iter_t _begin, _end;
    node<iter_t, sv_t>& _expression;

    std::atomic<size_t> _cnt_running;
    std::mutex _cnt_mut;
    bool _cancelled;
    std::condition_variable _wait_cv, _cancel_cv;
    
    // Needed for implementing pause when enough async results produced
    std::atomic<int> _async_result_left;
    std::vector<iter_t> _stashed_async_iters;
    std::mutex _stash_mut;

public:
    // Start or resume the job, stopping when `min_result_sync` iters
    // are found synchorously OR `min_result_async` iters are found
    // asynchorously, OR end reached.
    template <typename callback_t>
    void start(fifo_thpool& pool, callback_t callback,
               int min_result_sync = 2147483647,
               int min_result_async = 2147483647) 
    {
        auto pool_job = [this, callback] (iter_t it) {
            if (_cancelled)
                return;
            if (_async_result_left <= 0) {
                // Async execution paused; stash the iterator
                std::lock_guard __lck(_stash_mut);
                _stashed_async_iters.emplace_back(std::move(it));
                return;
            }
            if (_expression.apply_blocked(it)) {
                --_async_result_left;
                callback(it);
            }

            _cnt_mut.lock();
            --_cnt_running;
            _cnt_mut.unlock();
            _wait_cv.notify_all();
        }; 

        _cancelled = false;
        _async_result_left = min_result_async;
    {   // First resume stashed jobs
        std::lock_guard __lck(_stash_mut);
        for (iter_t& it : _stashed_async_iters)
            pool_job(std::move(it));
        _stashed_async_iters.clear();
    }

        // Produce `min_result_sync` synchorous results
        while (_begin != _end && min_result_sync > 0) {
            tribool_bad res = _expression.apply(_begin);
            if (res.is_uncertain()) {
                ++_cnt_running;
                pool.enqueue(pool_job, _begin);
            } else if (res == tribool_bad::True) {
                // TODO: Async callback?
                callback(_begin);
                --min_result_sync;
            }
            ++_begin;
        }
    }

    void join() noexcept {
        std::unique_lock __lck(_cnt_mut);
        // Noexcept if pred is noexcept.
        _wait_cv.wait(__lck, [this] () { return _cnt_running == 0; });
    }

    void cancel() noexcept { _cancelled = true; }
    template <class Rep, class Period> 
    void cancel(const std::chrono::duration<Rep, Period>& timeout) noexcept {
        std::thread([this, timeout] () {
            std::unique_lock __lck(_cnt_mut);
            if (_cancelled)
                return;
            if (std::cv_status::timeout == _cancel_cv.wait_for(__lck, timeout))
                _cancelled = true;
        }).detach();
    }

    async_job(iter_t begin, iter_t end, node<iter_t, sv_t>& expr)
        : _begin(begin), _end(end), _expression(expr)
        , _cnt_running(0), _cancelled(false) {}

    template <typename callback_t>
    async_job(iter_t begin, iter_t end, node<iter_t, sv_t>& expr,
              fifo_thpool& pool, callback_t callback,
              int min_result_sync = 2147483647, int min_result_async = 2147483647)
        : async_job(begin, end, expr) 
    { start<callback_t>(pool, callback, min_result_sync, min_result_async); }

    async_job(const async_job&) = delete;
    async_job& operator=(const async_job&) = delete;
    async_job(async_job&&) = delete;
    async_job& operator=(async_job&&) = delete;
    ~async_job() noexcept { 
    {
        std::lock_guard __lck(_cnt_mut);
        _cancelled = true;
    }
        join(); // cnt_running == 0 after joining
        _cancel_cv.notify_all(); // Stop timed cancelling thread
    }
};

}
}