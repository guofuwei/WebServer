//
// Created by hanshan on 2023/3/22.
//

#include "logger.h"

#include <utility>

Logger::Logger(std::string filename) : filename_(std::move(filename)), log_level_(LOG_LEVEL::DEBUG) {
  file_stream_.open(filename_, std::ios_base::app);
}


void Logger::Log(LOG_LEVEL level, const std::string &message,
                 const std::string &file, const std::string &func) {
  if (level < log_level_) {
    return;
  }
  std::stringstream ss;
  ss << std::put_time(std::localtime(&now_), "%Y-%m-%d %H:%M:%S")
     << " [" << file << ":" << func << "] " << message;
  file_stream_ << ss.str() << std::endl;
}


Logger *Logger::GetInstance(const std::string &filename) {
  static Logger instance_(filename);
  return &instance_;
}
