//
// Created by hanshan on 2023/3/20.
//

#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H
#include "logger.h"
#include "threadpool.h"
#include <netinet/in.h>

class WebServer {
private:
  WebServer() = default;
  ~WebServer() = default;

  void Init(int port);
  void HandleNewConnection();

  int listen_port_{0};
  int listen_fd_{0};
  sockaddr_in server_addr_{};
  int epoll_fd_{0};
  ThreadPool *thread_pool_{nullptr};

public:
  inline static WebServer *GetInstance(int port = 3000) {
    static WebServer web_server_;
    web_server_.Init(port);
    return &web_server_;
  }
  void Start();
};


#endif//WEBSERVER_WEBSERVER_H
