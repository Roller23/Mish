#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <thread>

#include "server.hpp"
#include "client.hpp"
#include "worker.hpp"

void Server::create_server_socket(const int port) {
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  int option = 1;
  // make the socket address reusable
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(socket_fd, (sockaddr *)&address, sizeof(address));
}

void Server::generate_threadpool(void) {
  for (unsigned int i = 0; i < max_threads; i++) {
    threadpool.emplace_back(file_mutex, stdout_mutex);
  }
  for (auto &thread : threadpool) {
    thread.start_thread();
  }
}

void Server::accept_connections() {
  while (true) {
    Client client;
    client.socket_fd = accept(socket_fd, (sockaddr *)&client.info, &client.info_len);
    client.ip_addr = inet_ntoa(client.info.sin_addr);
    threadpool[0].add_client(client);
    char payload = 23;
    int w = write(threadpool[0]._pipe[1], &payload, sizeof(payload));
    std::cout << "sent payload to pipe " << w << std::endl;
  }
}

void Server::serve(const int port) {
  std::cout << max_threads << " cores detected\n";
  create_server_socket(port);
  listen(socket_fd, max_connections);
  std::cout << "Listening on port " << port << "...\n";
  accept_connections();
}