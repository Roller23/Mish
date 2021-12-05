#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <cassert>
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

#include "../../utils/utils.hpp"
#include "../../utils/file.hpp"

void Server::load_config_args(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    const auto &arg_components = Srv::Utils::split(argv[i], '=');
    if (arg_components.size() != 2) {
      std::cout << "Invalid argument: " << argv[i] << std::endl;
      std::exit(EXIT_FAILURE);
    }
    if (arg_components[0].substr(0, 2) != "--") {
      std::cout << "Unknown argument: " << arg_components[0] << std::endl;
      std::exit(EXIT_FAILURE);
    }
    config.load_option(arg_components[0].substr(2), arg_components[1]);
  }
}

void Server::load_config_file(void) {
  if (!std::filesystem::exists(config.config_file)) return;
  const std::string &file_str = File::read(config.config_file);
  const auto &lines = Srv::Utils::split(file_str, '\n');
  for (auto &line : lines) {
    const auto &arg_components = Srv::Utils::split(line, ':');
    if (arg_components.size() != 2) {
      std::cout << "Invalid argument: " << line << std::endl;
      std::exit(EXIT_FAILURE);
    }
    const std::string &value = Srv::Utils::ltrim(arg_components[1]);
    config.load_option(arg_components[0], value);
  }
}

void Server::create_server_socket(const int port) {
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    perror("socket()");
    std::exit(EXIT_FAILURE);
  }
  int option = 1;
  // make the socket address reusable
  int err = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  if (err) {
    perror("setsockopt()");
    std::exit(EXIT_FAILURE);
  }
  sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  err = bind(socket_fd, (sockaddr *)&address, sizeof(address));
  if (err) {
    perror("bind()");
    std::exit(EXIT_FAILURE);
  }
}

void Server::generate_threadpool(void) {
  for (unsigned int i = 0; i < config.max_threads; i++) {
    threadpool.emplace_back(file_mutex, stdout_mutex, config);
  }
  for (auto &worker : threadpool) {
    worker.server_pipe = _pipe;
    worker.start_thread();
  }
}

Worker &Server::get_optimal_worker(int *err) {
  std::size_t idx = 0;
  int min = threadpool[idx].clients_polled;
  for (std::size_t i = 0; i < threadpool.size(); i++) {
    if (threadpool[i].clients_polled < min) {
      min = threadpool[i].clients_polled;
      idx = i;
    }
  }
  if (threadpool[idx].clients_polled > Worker::PFDS_SIZE && err != nullptr) {
    *err = 1;
  }
  return threadpool[idx];
}

void Server::accept_connections() {
  while (true) {
    Client *client = new Client;
    client->socket_fd = accept(socket_fd, nullptr, nullptr);
    if (client->socket_fd == -1) {
      perror("Could't accept a new connection");
      continue;
    }
    int err = 0;
    Worker &worker = get_optimal_worker(&err);
    if (err) {
      close(client->socket_fd);
      std::cout << "All workers are busy";
      continue;
    }
    worker.pending_client = client;
    // notify the thread about the client
    char payload = Worker::PIPE_PAYLOAD;
    int w = write(worker._pipe[Worker::PIPE_WRITE], &payload, sizeof(payload));
    assert(w == sizeof(payload));
    // wait for the thread to report back
    payload = 0;
    int r = read(_pipe[Worker::PIPE_READ], &payload, sizeof(payload));
    assert(r == sizeof(payload));
    assert(payload == Worker::PIPE_PAYLOAD);
  }
}

void Server::serve(const int port) {
  std::cout << config.max_threads << " cores detected\n";
  create_server_socket(port);
  int err = listen(socket_fd, config.max_connections);
  if (err) {
    perror("listen()");
    std::exit(EXIT_FAILURE);
  }
  std::cout << "Listening on port " << port << "...\n";
  accept_connections();
}