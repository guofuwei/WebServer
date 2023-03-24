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
  ThreadPool();
  ~ThreadPool();
  std::vector<std::thread> workers_;
  std::queue<Task> tasks_queue_;

  std::mutex queue_mutex_;
  std::condition_variable condition_;
  std::atomic<bool> is_stop_{false};


public:
  static ThreadPool *GetInstance();
  // 禁用拷贝和移动构造函数
  ThreadPool(const ThreadPool &) = delete;
  ThreadPool(ThreadPool &&) = delete;

  template<class T, typename... Args>
  void EnQueue(T &&task, Args &&...args) {
    if (is_stop_) {
      throw std::runtime_error("ThreadPool has been stopped.");
    }
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      auto f = std::bind(std::forward<T>(task), std::forward<Args>(args)...);
      tasks_queue_.push(f);
    }
    condition_.notify_one();
  }
};


#endif//WEBSERVER_THREADPOOL_H
