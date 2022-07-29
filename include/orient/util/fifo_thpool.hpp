#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

namespace orie {

class fifo_thpool {
public:
    fifo_thpool(size_t);
	fifo_thpool() : fifo_thpool(std::thread::hardware_concurrency()) {}

    template<class F, class... Args>
	std::future<typename std::result_of<F(Args...)>::type> 
	enqueue(F&& f, Args&&... args);

    ~fifo_thpool();
private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task queue
    std::vector< std::function<void()> > tasks;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// the constructor just launches some amount of workers
inline fifo_thpool::fifo_thpool(size_t threads)
    :   stop(false)
{
    for(size_t i = 0;i<threads;++i)
        workers.emplace_back( [this] {
            for(;;) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock,
                        [this] { return this->stop || !this->tasks.empty(); });
                    if(this->stop && this->tasks.empty())
                        return;
                    task = std::move(this->tasks.back());
                    this->tasks.pop_back();
                }

                task();
                }
            }
        );
}

// add new work item to the pool
template<class F, class... Args>
std::future<typename std::result_of<F(Args...)>::type>
fifo_thpool::enqueue(F&& f, Args&&... args) 
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped fifo_thpool");

        tasks.emplace_back([task](){ (*task)(); });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline fifo_thpool::~fifo_thpool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers)
        worker.join();
}

}

#endif
