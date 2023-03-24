//
// Created by hanshan on 2023/3/24.
//

#ifndef WEBSERVER_CONFIG_H
#define WEBSERVER_CONFIG_H


namespace webserverconfig {
  const int kMaxConnections = 1024;
  const int kPort = 3000;
  const int kThreadNum = 32;
  const int kMaxEvents = 1024;
};// namespace webserverconfig

#endif//WEBSERVER_CONFIG_H
