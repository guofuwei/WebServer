//
// Created by hanshan on 2023/3/21.
//

#include "httphandle.h"
#include "logger.h"
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>

const std::string FILE_NAME = "httphandle.cpp";

void HttpHandle::ParseClientAddr() {
  client_ip_ = new char[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(client_addr_.sin_addr), client_ip_, INET_ADDRSTRLEN);
  client_port_ = ntohs(client_addr_.sin_port);
}

void HttpHandle::Run(int client_fd, sockaddr_in client_addr) {
  client_fd_ = client_fd;
  client_addr_ = client_addr;
  ParseClientAddr();
  sub_epoll_fd_ = epoll_create(1);
  if (sub_epoll_fd_ < 0) {
    perror("epoll_create error");
    exit(-1);
  }
  // 将客户端套接字添加到 epoll 中
  memset(&sub_ev_, 0, sizeof(sub_ev_));
  sub_ev_.events = EPOLLIN;
  sub_ev_.data.fd = client_fd_;
  if (epoll_ctl(sub_epoll_fd_, EPOLL_CTL_ADD, client_fd_, &sub_ev_) == -1) {
    perror("epoll_ctl EPOLL_CTL_ADD connfd");
    return;
  }

  std::stringstream ss;
  ss << client_ip_ << ":" << client_port_ << " "
     << "join";
  Logger::GetInstance()->Log(LOG_LEVEL::INFO, ss.str(), FILE_NAME, "Run");
  is_stop_ = false;
  while (!is_stop_) {
    int nfds = epoll_wait(sub_epoll_fd_, sub_events, SUB_MAX_EVENTS, -1);
    if (nfds == -1) {
      perror("sub_epoll_wait");
      continue;
    }
    // 处理事件
    for (int i = 0; i < nfds; i++) {
      if (sub_events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
        CloseConnection();
      } else if (sub_events[i].events & EPOLLIN) {
        HandleData();
      } else {
        CloseConnection();
      }
    }
  }
  ss << client_ip_ << ":" << client_port_ << " "
     << "leave";
  Logger::GetInstance()->Log(LOG_LEVEL::INFO, ss.str(), FILE_NAME, "Run");
}

void HttpHandle::CloseConnection() {
  is_stop_ = true;
  epoll_ctl(sub_epoll_fd_, EPOLL_CTL_DEL, client_fd_, &sub_ev_);
  close(client_fd_);
}

void HttpHandle::HandleData() {
  ReadHandle();
  if (!is_stop_) {
    WriteHandle();
  }
}

void HttpHandle::ReadHandle() {
  char buf[1024] = {0};
  int len = recv(client_fd_, buf, sizeof(buf), 0);
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
