//
// Created by hanshan on 2023/3/22.
//

#ifndef WEBSERVER_LOGGER_H
#define WEBSERVER_LOGGER_H
#include <condition_variable>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>


enum class LOG_LEVEL {
  TRACE,
  DEBUG,
  INFO,
  WARN,
  ERROR
};

class Logger {
public:
  static Logger *GetInstance(const std::string &filename = "log.txt");
  void set_log_level(LOG_LEVEL level) {
    log_level_ = level;
  }
  void Log(LOG_LEVEL level, const std::string &message,
           const std::string &file = "",
           const std::string &func = "");
  void Stop();

private:
  explicit Logger(std::string filename);
  ~Logger() {
    Stop();
  };
  void LogWorker();

  std::string filename_;
  LOG_LEVEL log_level_;
  std::time_t now_{std::time(nullptr)};
  std::ofstream file_stream_;

  std::queue<std::string> log_buffer_;
  std::mutex log_mutex_;
  std::condition_variable cv_;
  std::thread log_thread_;
  bool is_stop_{false};
};


#endif//WEBSERVER_LOGGER_H
