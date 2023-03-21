//
// Created by hanshan on 2023/3/20.
//

#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H
#include "threadpool.h"
#include <netinet/in.h>

class WebServer {
private:
  WebServer() = default;
  ~WebServer() = default;

  void Init(int port);

  int listen_port_;
  int listen_fd_;
  sockaddr_in server_addr_{};
  int epoll_fd_;
  ThreadPool *thread_pool_;

public:
  inline static WebServer *GetInstance(int port = 3000) {
    static WebServer web_server_;
    web_server_.Init(port);
    return &web_server_;
  }
  void Start();
};


#endif//WEBSERVER_WEBSERVER_H
