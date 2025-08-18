// Copyright (c) 2025 Alex Li
// ThreadPool.hpp
// Thread pool class to manage a pool of threads for parallel execution.

#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>
#include <type_traits>
#include <vector>

namespace utils {

class ThreadPool {

public:
  explicit ThreadPool(size_t numThreads);
  ~ThreadPool();

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;

  ///@brief Enqueue a task to be executed by the thread pool.
  ///@param f Function to be executed.
  ///@param args Arguments to be passed to the function.
  ///@return A future that will hold the result of the function execution.
  template <typename F, typename... Args>
  auto enqueue(F &&f, Args &&...args)
      -> std::future<std::invoke_result_t<F, Args...>> {
    using ReturnType = std::invoke_result_t<F, Args...>;
    using Fn_t = std::decay_t<F>;
    using ArgsTuple_t = std::tuple<std::decay_t<Args>...>;

    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        [f = Fn_t(std::forward<F>(f)),
         argsTuple =
             ArgsTuple_t(std::forward<Args>(args)...)]() mutable -> ReturnType {
          // Unpack the tuple and call the function
          return std::apply(std::move(f), std::move(argsTuple));
        });
    std::future<ReturnType> res = task->get_future();
    {
      std::unique_lock<std::mutex> lock(queueMutex);
      // Don't allow enqueueing after stopping the pool
      if (stop) {
        throw std::runtime_error("Enqueue on stopped ThreadPool");
      }
      tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one(); // Notify one worker thread that a task is available
    // Return the future to the caller
    return res;
  }

private:
  std::vector<std::thread> workers;        // Vector of worker threads
  std::queue<std::function<void()>> tasks; // Queue of tasks to be executed
  std::mutex queueMutex; // Mutex to protect access to the task queue
  std::condition_variable condition; // Condition variable for task notification
  bool stop;                         // Flag to indicate if the pool is stopping
};
} // namespace utils
