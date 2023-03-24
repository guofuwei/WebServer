//
// Created by hanshan on 2023/3/21.
//

#include "threadpool.h"
#include "config.h"
#include <iostream>


ThreadPool::ThreadPool() : is_stop_(false) {
  for (size_t i = 0; i < webserverconfig::kThreadNum; i++) {
    workers_.emplace_back([this] {
      while (true) {
        Task task;
        {
          std::unique_lock<std::mutex> lock(this->queue_mutex_);
          this->condition_.wait(lock, [this] {
            return is_stop_ || !tasks_queue_.empty();
          });
          if (this->is_stop_ && tasks_queue_.empty()) {
            return;
          }
          task = std::move(tasks_queue_.front());
          tasks_queue_.pop();
        }
        task();
      }
    });
  }
}


ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    is_stop_ = true;
  }
  condition_.notify_all();
  for (std::thread &worker: workers_) {
    worker.join();
  }
}