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
    std::mutex _wait_mut;
    std::condition_variable _wait_cv; //, _cancel_cv;
    bool _cancelled;

    // Needed for implementing pause when enough async results produced
    std::atomic<int> _result_left;
    std::vector<iter_t> _stashed_async_iters;
    std::mutex _stash_mut;

public:
    // Start or resume the job, stopping when `min_result_sync` iters
    // are found synchorously OR `min_result_async` iters are found
    // asynchorously, OR end reached.
    template <typename callback_t>
    void start(fifo_thpool& pool, callback_t callback,
               int min_result = 2147483647) 
    {
        if (_cancelled)
            return;
        auto pool_job = [this, callback] (iter_t it) {
            if (_cancelled)
                ;
            else if (_result_left <= 0) {
                // Async execution paused; stash the iterator
                std::lock_guard __lck(_stash_mut);
                _stashed_async_iters.emplace_back(std::move(it));
            } else if (_expression.apply_blocked(it)) {
                --_result_left;
                if constexpr (std::is_invocable_v<callback_t, bool, iter_t&>)
                    callback(true, it);
                else callback(it);
            }

            --_cnt_running;
            _wait_cv.notify_all();
        }; // End of local function pool_job

        _result_left = min_result;
    {   // First resume stashed jobs
        std::lock_guard __lck(_stash_mut);
        _cnt_running += _stashed_async_iters.size();
        for (iter_t& it : _stashed_async_iters)
            pool.enqueue(pool_job, std::move(it));
        _stashed_async_iters.clear();
    }

        // Produce synchorous results
        // If `next` is faster, use it.
        if (_expression.faster_with_next(true)) {
            while (_result_left > 0 && !_cancelled) {
                bool uncertain =
                    _expression.next_or_uncertain(_begin, _end, true);
                if (_begin == _end)
                    return;
                if (uncertain) {
                    ++_cnt_running;
                    pool.enqueue(pool_job, _begin);
                } else {
                    if constexpr (std::is_invocable_v<callback_t, bool, iter_t&>)
                        callback(false, _begin);
                    else callback(_begin);
                    --_result_left;
                }
            }
            return;
        }

        // Fall back on iterative approach if `next` is not faster
        while (_begin != _end && _result_left > 0 && !_cancelled) {
            tribool_bad res;
            try {
                res = _expression.apply(_begin);
            } catch (pred_tree::quitted&) {
                _cancelled = true;
            }
            if (res.is_uncertain()) {
                ++_cnt_running;
                pool.enqueue(pool_job, _begin);
            } else if (res == tribool_bad::True) {
                // TODO: Async callback?
                if constexpr (std::is_invocable_v<callback_t, bool, iter_t&>)
                    callback(false, _begin);
                else callback(_begin);
                --_result_left;
            }
            ++_begin;
        }
    }

    void join() noexcept {
        std::unique_lock __lck(_wait_mut);
        // Noexcept if pred is noexcept.
        _wait_cv.wait(__lck, [this] () { return _cnt_running == 0; });
    }

    // Once cancelled, the job cannot be started again
    void cancel() noexcept { _cancelled = true; }
    bool is_finished() const noexcept { 
        return _cnt_running == 0 && _stashed_async_iters.empty() &&
               _begin == _end; 
    }

    async_job(iter_t begin, iter_t end, node<iter_t, sv_t>& expr)
        : _begin(begin), _end(end), _expression(expr)
        , _cnt_running(0), _cancelled(false) {}

    // template <typename callback_t>
    // async_job(iter_t begin, iter_t end, node<iter_t, sv_t>& expr,
    //           fifo_thpool& pool, callback_t callback,
    //           int min_result = 2147483647)
    //     : async_job(begin, end, expr) 
    // { start<callback_t>(pool, callback, min_result_sync, min_result_async); }

    async_job(const async_job&) = delete;
    async_job& operator=(const async_job&) = delete;
    async_job(async_job&&) = delete;
    async_job& operator=(async_job&&) = delete;
    ~async_job() noexcept { 
        _cancelled = true;
        join(); // cnt_running == 0 after joining
    }
};

}
}