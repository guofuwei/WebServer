//
// Created by hanshan on 2023/3/21.
//

#include "httphandle.h"
#include "logger.h"
#include <arpa/inet.h>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>


void HttpHandle::ParseClientAddr() {
  client_ip_.reset(new char[INET_ADDRSTRLEN]);
  inet_ntop(AF_INET, &(client_addr_.sin_addr), client_ip_.get(), INET_ADDRSTRLEN);
  client_port_ = ntohs(client_addr_.sin_port);
}

void HttpHandle::Run(int client_fd, sockaddr_in client_addr) {
  client_fd_ = client_fd;
  client_addr_ = client_addr;
  // 解析客户端地址
  ParseClientAddr();
  // 创建 epoll
  sub_epoll_fd_ = epoll_create(2);
  if (sub_epoll_fd_ < 0) {
    // 记录日志
    Logger::GetInstance()->Log(LOG_LEVEL::ERROR, "sub_epoll_create error.", "httphandle.cpp", "Run");
    return;
  }
  // 将客户端套接字添加到 epoll 中
  epoll_event sub_ev{EPOLLIN, {}};
  sub_ev.data.fd = client_fd_;
  if (epoll_ctl(sub_epoll_fd_, EPOLL_CTL_ADD, client_fd_, &sub_ev) == -1) {
    // 记录日志
    Logger::GetInstance()->Log(LOG_LEVEL::ERROR, "epoll_ctl EPOLL_CTL_ADD connfd", "httphandle.cpp", "Run");
    return;
  }
  // 将 stop_eventfd_ 添加到 epoll 中
  epoll_event stop_eventfd_event{EPOLLIN, {}};
  stop_eventfd_event.data.fd = stop_eventfd_;
  epoll_ctl(sub_epoll_fd_, EPOLL_CTL_ADD, stop_eventfd_, &stop_eventfd_event);

  // 记录日志
  std::stringstream ss;
  ss << client_ip_.get() << ":" << client_port_ << " join";
  Logger::GetInstance()->Log(LOG_LEVEL::INFO, ss.str(), "httphandle.cpp", "Run");
  ss.str(std::string());
  //  开始处理请求
  is_stop_ = false;
  while (!is_stop_) {
    int nfds = epoll_wait(sub_epoll_fd_, sub_events, webserverconfig::kMaxEvents, -1);
    if (nfds == -1) {
      if (errno == EINTR) {
        continue;
      }
      // 记录日志
      Logger::GetInstance()->Log(LOG_LEVEL::ERROR, "sub_epoll_wait error.", "httphandle.cpp", "Run");
      return;
    }
    // 处理事件
    for (int i = 0; i < nfds; i++) {
      // 如果是 stop_eventfd_ 触发的事件
      if (sub_events[i].data.fd == stop_eventfd_) {
        uint64_t temp = 0;
        ssize_t len = read(stop_eventfd_, &temp, sizeof(temp));
        if (len != sizeof(temp)) {
          // 记录日志
          Logger::GetInstance()->Log(LOG_LEVEL::ERROR, "read stop_eventfd_ error.", "httphandle.cpp", "Run");
          return;
        }
        CloseConnection();
        //  如果是客户端套接字可读事件
      } else if (sub_events[i].events & EPOLLIN) {
        HandleData();
        //  如果是客户端套接字其他事件
      } else {
        CloseConnection();
      }
    }
  }
  // 记录日志
  ss << client_ip_.get() << ":" << client_port_ << " leave";
  Logger::GetInstance()->Log(LOG_LEVEL::INFO, ss.str(), "httphandle.cpp", "Run");
  ss.str(std::string());
}

void HttpHandle::CloseConnection() {
  is_stop_ = true;
  close(client_fd_);
  close(stop_eventfd_);
  close(sub_epoll_fd_);
}

void HttpHandle::HandleData() {
  ReadHandle();
  if (!is_stop_) {
    WriteHandle();
  }
}

void HttpHandle::ReadHandle() {
  char buf[1024] = {0};
  auto len = recv(client_fd_, buf, sizeof(buf), 0);
  if (len <= 0) {// 连接已经关闭
    CloseConnection();
  }
}

void HttpHandle::WriteHandle() {
  // 处理请求
  std::string resp = "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html\r\n"
                     "Content-Length: 11\r\n"
                     "\r\n"
                     "Hello World";
  send(client_fd_, resp.c_str(), resp.length(), 0);
}
void HttpHandle::Stop() const {
  uint64_t temp = 1;
  ssize_t len = write(stop_eventfd_, &temp, sizeof(temp));
  if (len != sizeof(temp)) {
    // 记录日志
    Logger::GetInstance()->Log(LOG_LEVEL::ERROR, "write stop_eventfd_ error.", "httphandle.cpp", "Stop");
  }
}
