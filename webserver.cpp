//
// Created by hanshan on 2023/3/20.
//

#include "webserver.h"
//#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

using std::string;

void WebServer::Init(int port) {
  listen_port_ = port;
  // 创建监听socket
  listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd_ < 0) {
    perror("socket listen error");
    exit(-1);
  }
  // 设置非阻塞模式
  fcntl(listen_fd_, F_SETFL, fcntl(listen_fd_, F_GETFL, 0) | O_NONBLOCK);
  // 绑定IP和端口
  memset(&server_addr_, '\0', sizeof(server_addr_));
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr_.sin_port = htons(listen_port_);
  if (bind(listen_fd_, (sockaddr *) &server_addr_, sizeof(server_addr_)) < 0) {
    perror("bind error");
    exit(-1);
  }
  // 监听端口
  if (listen(listen_fd_, 10) < 0) {
    perror("listen error");
    exit(-1);
  }
  // 创建epoll
  epoll_fd_ = epoll_create(1024);
  if (epoll_fd_ < 0) {
    perror("epoll_create error");
    exit(-1);
  }
  // 添加监听socket到epoll
  epoll_event ev{};
  ev.data.fd = listen_fd_;
  ev.events = EPOLLIN | EPOLLET;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev) < 0) {
    perror("epoll_ctl error");
    exit(-1);
  }
}

void WebServer::Start() {
  while (true) {
    epoll_event events[1024];
    int nfds = epoll_wait(epoll_fd_, events, 1024, -1);
    if (nfds < 0) {
      perror("epoll_wait error");
      exit(-1);
    }

    // 处理事件
    for (int i = 0; i < nfds; i++) {
      int sockfd = events[i].data.fd;
      // 接收请求并处理
      if (sockfd == listen_fd_) {
        // 想把客户socket存储起来
        sockaddr_in clientaddr{};
        socklen_t clientaddrlen = sizeof(clientaddr);
        int connfd = accept(listen_fd_, (sockaddr *) &clientaddr, &clientaddrlen);
        if (connfd < 0) {
          perror("accept error");
          exit(-1);
        }
        // 设置非阻塞模式
        fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK);
        // 添加连接socket到epoll
        epoll_event conn_ev{};
        conn_ev.data.fd = connfd;
        conn_ev.events = EPOLLIN | EPOLLET;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, connfd, &conn_ev) < 0) {
          perror("epoll_ctl error");
          exit(-1);
        }

      } else if (events[i].events & EPOLLIN) {// 处理数据
        char buf[1024] = {0};
        int len = recv(sockfd, buf, sizeof(buf), 0);
        if (len <= 0) {// 连接已经关闭
          close(sockfd);
          continue;
        }
        // 处理请求
        string req(buf);
        string resp = "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/html; charset=UTF-8\r\n\r\n"
                      "<html><head><title>Hello World</title></head>"
                      "<body><h1>Hello World</h1></body>"
                      "</html>";
        send(sockfd, resp.c_str(), resp.length(), 0);
      }
    }
  }
}