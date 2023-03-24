#include "webserver.h"
#include <csignal>

// SigInt handler
void SigIntHandler(int sig) {
  printf("sig");
  exit(0);
}
int main() {
  // Register signal handler
  signal(SIGINT, SigIntHandler);
  WebServer *web_server = WebServer::GetInstance();
  web_server->Start();
}
