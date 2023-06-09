//
// Created by hanshan on 2023/3/20.
//

#include "webserver.h"
#include "httphandle.h"
#include "logger.h"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

using std::string;


WebServer::WebServer() {
  // 初始化端口
  listen_port_ = webserverconfig::kPort;
  // 初始化子连接
  http_handle_queue_ = std::vector<HttpHandle>(webserverconfig::kMaxConnections);
  // 初始化线程池
  thread_pool_ = ThreadPool::GetInstance();
  // 创建监听socket
  listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd_ < 0) {
    // 记录日志
    Logger::GetInstance()->Log(LOG_LEVEL::ERROR, "create listen socket error", "webserver.cpp", "WebServer");
    exit(-1);
  }
  // 立即释放端口
  int optval = 1;
  setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  // 设置非阻塞模式
  fcntl(listen_fd_, F_SETFL, fcntl(listen_fd_, F_GETFL, 0) | O_NONBLOCK);
  // 绑定IP和端口
  memset(&server_addr_, '\0', sizeof(server_addr_));
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr_.sin_port = htons(listen_port_);
  if (bind(listen_fd_, (sockaddr *) &server_addr_, sizeof(server_addr_)) < 0) {
    // 记录日志
    Logger::GetInstance()->Log(LOG_LEVEL::ERROR, "bind error", "webserver.cpp", "WebServer");
    exit(-1);
  }
  // 监听端口
  if (listen(listen_fd_, 10) < 0) {
    // 记录日志
    Logger::GetInstance()->Log(LOG_LEVEL::ERROR, "listen error", "webserver.cpp", "WebServer");
    exit(-1);
  }
  // 创建epoll
  epoll_fd_ = epoll_create(1);
  if (epoll_fd_ < 0) {
    // 记录日志
    Logger::GetInstance()->Log(LOG_LEVEL::ERROR, "epoll_create error", "webserver.cpp", "WebServer");
    exit(-1);
  }
  // 添加监听socket到epoll
  epoll_event ev{};
  ev.data.fd = listen_fd_;
  ev.events = EPOLLIN | EPOLLET;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev) < 0) {
    // 记录日志
    Logger::GetInstance()->Log(LOG_LEVEL::ERROR, "epoll_ctl EPOLL_CTL_ADD listen_fd_", "webserver.cpp", "WebServer");
    exit(-1);
  }
}

void WebServer::HandleNewConnection() {
  // 想把客户socket存储起来
  sockaddr_in clientaddr{};
  socklen_t clientaddrlen = sizeof(clientaddr);
  int connfd = accept(listen_fd_, (sockaddr *) &clientaddr, &clientaddrlen);
  if (connfd < 0) {
    // 记录日志
    Logger::GetInstance()->Log(LOG_LEVEL::WARN, "accept error", "webserver.cpp", "HandleNewConnection");
    return;
  } else if (connfd >= webserverconfig::kMaxConnections) {
    close(connfd);
    // 记录日志
    Logger::GetInstance()->Log(LOG_LEVEL::WARN, "Too many connections", "webserver.cpp", "HandleNewConnection");
    return;
  }
  // 设置非阻塞模式
  fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK);
  // 添加连接socket到epoll
  thread_pool_->EnQueue(&HttpHandle::Run, &http_handle_queue_[connfd], connfd, clientaddr);
}

void WebServer::Start() {
  Logger::GetInstance()->Log(LOG_LEVEL::INFO, "Server Start", "webserver.cpp", "Start");
  try {
    while (!is_stop_) {
      epoll_event events[webserverconfig::kMaxEvents];
      int nfds = epoll_wait(epoll_fd_, events, webserverconfig::kMaxEvents, -1);
      if (nfds < 0) {
        if (errno == EINTR) {
          continue;
        }
        // 记录日志
        Logger::GetInstance()->Log(LOG_LEVEL::ERROR, "epoll_wait error", "webserver.cpp", "Start");
        exit(-1);
      }
      // 处理事件
      for (int i = 0; i < nfds; i++) {
        int sockfd = events[i].data.fd;
        // 接收请求并处理
        if (sockfd == listen_fd_) {
          HandleNewConnection();
        }
      }
    }
  } catch (std::exception &e) {
    // 记录日志
    Logger::GetInstance()->Log(LOG_LEVEL::ERROR, "Exception caught", "webserver.cpp", "Start");
  }
}

void WebServer::Stop() {
  is_stop_ = true;
  epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, listen_fd_, nullptr);
  close(listen_fd_);
  close(epoll_fd_);
}
