//
// Created by hanshan on 2023/3/22.
//

#include "logger.h"

#include <utility>

Logger::Logger(std::string filename) : filename_(std::move(filename)), log_level_(LOG_LEVEL::DEBUG) {
  file_stream_.open(filename_, std::ios_base::app);
  log_thread_ = std::thread(&Logger::LogWorker, this);
}

void Logger::LogWorker() {
  while (!is_stop_) {
    std::unique_lock<std::mutex> lock(log_mutex_);
    cv_.wait(lock, [this] { return is_stop_ || !log_buffer_.empty(); });
    while (!log_buffer_.empty()) {
      file_stream_ << log_buffer_.front() << std::endl;
      log_buffer_.pop();
    }
  }
}


void Logger::Log(LOG_LEVEL level, const std::string &message,
                 const std::string &file, const std::string &func) {
  if (level < log_level_) {
    return;
  }
  std::unique_lock<std::mutex> lock(log_mutex_);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&now_), "%Y-%m-%d %H:%M:%S")
     << " [" << file << ":" << func << "] " << message;
  log_buffer_.push(ss.str());
  cv_.notify_one();
  ss.str(std::string());
}


Logger *Logger::GetInstance(const std::string &filename) {
  static Logger instance_(filename);
  return &instance_;
}
void Logger::Stop() {
  Log(LOG_LEVEL::INFO, "logger stop", "logger.cpp", "Stop");
  is_stop_ = true;
  cv_.notify_all();
  log_thread_.join();
}
