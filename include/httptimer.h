//
// Created by hanshan on 2023/3/24.
//

#ifndef WEBSERVER_HTTPTIMER_H
#define WEBSERVER_HTTPTIMER_H


#include <chrono>
#include <functional>
#include <iostream>
#include <thread>
#include <utility>

class HttpTimer {
private:
  std::function<void()> func;
  std::chrono::milliseconds interval;
  bool is_running = false;
  std::thread timer_thread;

public:
  HttpTimer(std::function<void()> func, std::chrono::milliseconds interval)
      : func(std::move(func)), interval(interval) {}
  ~HttpTimer() {
    std::cout << "~HttpTimer" << std::endl;
    stop();
  }

  void start() {
    is_running = true;
    timer_thread = std::thread([&]() {
      while (is_running) {
        std::this_thread::sleep_for(interval);
        func();
      }
    });
  }

  void stop() {
    is_running = false;
    if (timer_thread.joinable()) {
      timer_thread.join();
    }
  }
  void refresh() {
    stop();
    start();
  }
};

#endif//WEBSERVER_HTTPTIMER_H
