//
// Created by hanshan on 2023/3/21.
//

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>


class ThreadPool {
  using Task = std::function<void()>;

private:
  std::vector<std::thread> workers_;
  std::queue<Task> tasks_queue_;

  std::mutex queue_mutex_;
  std::condition_variable condition_;
  std::atomic<bool> is_stop_;
  explicit ThreadPool(size_t thread_count);
  ~ThreadPool() {
    is_stop_ = true;
    condition_.notify_all();
    for (std::thread &worker: workers_) {
      worker.join();
    }
  }

public:
  static ThreadPool *GetInstance(size_t thread_count = 10) {
    static ThreadPool instance_(thread_count);
    return &instance_;
  }

  template<class T, typename... Args>
  void EnQueue(T &&task, Args &&...args) {
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      auto f = std::bind(std::forward<T>(task), std::forward<Args>(args)...);
      tasks_queue_.push(f);
    }
    condition_.notify_one();
  }
};


#endif//WEBSERVER_THREADPOOL_H
