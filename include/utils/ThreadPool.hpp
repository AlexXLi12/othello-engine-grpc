// Copyright (c) 2025 Alex Li
// ThreadPool.hpp
// Thread pool class to manage a pool of threads for parallel execution.

#pragma once

#include <thread>
#include <vector>
#include <functional>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>

class ThreadPool {

public:
  ThreadPool(size_t numThreads);
  ~ThreadPool();

  template<typename F, typename... Args>
  auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;

private:
  std::vector<std::thread> workers; // Vector of worker threads
  std::queue<std::function<void()>> tasks; // Queue of tasks to be executed
  std::mutex queueMutex; // Mutex to protect access to the task queue
  std::condition_variable condition; // Condition variable for task notification
  bool stop; // Flag to indicate if the pool is stopping

  void workerThread(); // Worker thread function
};
