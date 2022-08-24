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

public:
    template <typename callback_t>
    void start(fifo_thpool& pool, callback_t callback, 
               bool always_async_cb) 
    {
        auto pool_job = [this, callback] (iter_t it) {
            if (!_cancelled && _expression.apply_blocked(it))
                callback(it);
            _cnt_mut.lock();
            --_cnt_running;
            _cnt_mut.unlock();
            _wait_cv.notify_all();
        }; 

        while (_begin != _end) {
            tribool_bad res = _expression.apply(_begin);
            if (res.is_uncertain()) {
                ++_cnt_running;
                pool.enqueue(pool_job, _begin);
            } else if (res == tribool_bad::True) {
                if (always_async_cb)
                    pool.enqueue(callback, _begin);
                else callback(_begin);
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
            if (std::cv_status::timeout == _cancel_cv.wait_for(__lck, timeout))
                _cancelled = true;
        }).detach();
    }

    async_job(iter_t begin, iter_t end, node<iter_t, sv_t>& expr)
        : _begin(begin), _end(end), _expression(expr)
        , _cnt_running(0), _cancelled(false) {}

    template <typename callback_t>
    async_job(iter_t begin, iter_t end, node<iter_t, sv_t>& expr,
        fifo_thpool& pool, callback_t callback, bool always_async_cb = false)
        : async_job(begin, end, expr) 
    { start<callback_t>(pool, callback, always_async_cb); }

    async_job(const async_job&) = delete;
    async_job& operator=(const async_job&) = delete;
    async_job(async_job&&) = delete;
    async_job& operator=(async_job&&) = delete;
    ~async_job() noexcept { 
        _cancelled = true;
        join(); 
        std::unique_lock __lck(_cnt_mut);
        _cancel_cv.notify_all();
    }
};

}
}