//
// Created by hanshan on 2023/3/21.
//

#ifndef WEBSERVER_HTTPHANDLE_H
#define WEBSERVER_HTTPHANDLE_H

#include <memory>
#include <netinet/in.h>
#include <string>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#define SUB_MAX_EVENTS 10

class HttpHandle {
public:
  HttpHandle() {
    stop_eventfd_ = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  };
  ~HttpHandle() {
    Stop();
  };

  void Run(int, sockaddr_in);
  void Stop() const;

private:
  void ParseClientAddr();
  void HandleData();
  void ReadHandle();
  void WriteHandle();
  void CloseConnection();


  int sub_epoll_fd_{};
  int stop_eventfd_;
  struct epoll_event sub_ev_ {};
  struct epoll_event sub_events[SUB_MAX_EVENTS]{};
  int client_fd_{};
  sockaddr_in client_addr_{};
  std::unique_ptr<char> client_ip_;
  int client_port_{};

  bool is_stop_{};
};


#endif//WEBSERVER_HTTPHANDLE_H
