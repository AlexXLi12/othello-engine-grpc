// Copyright (c) Alex Li, 2025
// ThreadPool.cpp
// Implementation of a generic thread pool class

#include "utils/ThreadPool.hpp"

#include <mutex>

namespace utils {
ThreadPool::ThreadPool(size_t num_threads) : stop(false) {
  for (size_t i = 0; i < num_threads; ++i) {
    workers.emplace_back([this] {
      while (true) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(this->queueMutex);
          // Wait until there is a task or the pool is stopping
          this->condition.wait(
              lock, [this] { return this->stop || !this->tasks.empty(); });
          // If stopping and no tasks, exit the thread
          if (this->stop && this->tasks.empty()) {
            return;
          }
          // Get the next task from the queue
          task = std::move(this->tasks.front());
          this->tasks.pop();
        }
        // Execute the task once the lock is released
        task();
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(queueMutex);
    stop = true;
  }
  condition.notify_all();
  for (std::thread &worker : workers) {
    worker.join();
  }
}
} // namespace utils

