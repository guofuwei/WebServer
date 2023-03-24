//
// Created by hanshan on 2023/3/20.
//

#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H
#include "httphandle.h"
#include "logger.h"
#include "threadpool.h"
#include <netinet/in.h>

class WebServer {
private:
  explicit WebServer();
  ~WebServer() {
    Stop();
  };

  void HandleNewConnection();

  std::atomic<bool> is_stop_{false};
  int listen_port_{0};
  int listen_fd_{0};
  sockaddr_in server_addr_{};
  int epoll_fd_{0};
  ThreadPool *thread_pool_{nullptr};         // 线程池
  std::vector<HttpHandle> http_handle_queue_;// http处理队列
  int http_count_{0};

public:
  static WebServer *GetInstance() {
    static WebServer web_server_;
    return &web_server_;
  }
  void Start();
  void Stop();
};


#endif//WEBSERVER_WEBSERVER_H
