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

#include "server.hpp"
#include "client.hpp"

#define HEADERS_END "\r\n\r\n"
#define HTTP_200 "HTTP/1.0 200 OK"
#define HTTP_404 "HTTP/1.0 404 Not Found"

#define CKRIPT_START "<&"
#define CKRIPT_END "&>"

#define REQUEST_BUFFER_SIZE (1024 * 10)
#define MAX_CONNECTIONS 1000

static int create_server_socket(const int port) {
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  int option = 1;
  // make the socket address reusable
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(socket_fd, (sockaddr *)&address, sizeof(address));
  return socket_fd;
}

static std::vector<std::string> split(const std::string &str, char delim) {
  std::size_t start;
  std::size_t end = 0;
  std::vector<std::string> out;
  while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
    end = str.find(delim, start);
    out.push_back(str.substr(start, end - start));
  }
  return out;
}

static std::string read_file(const std::string &path) {
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

static bool resource_exists(const std::string &path) {
  if (!std::filesystem::exists(path)) return false;
  if (std::filesystem::is_directory(path)) return false;
  return true;
}

static bool safe_path(const std::filesystem::path &path) {
  const std::filesystem::path &curr = std::filesystem::current_path();
  return std::search(path.begin(), path.end(), curr.begin(), curr.end()) != path.end();
}

static std::string generate_ok_res(const std::string &content) {
  std::string response = HTTP_200;
  response += HEADERS_END;
  response += content;
  return response;
}

static void write_ok_res(const std::string &content, int client) {
  std::string response = generate_ok_res(content);
  const char *response_c = response.c_str();
  write(client, response_c, response.length());
  close(client);
}

std::string Server::process_code(const std::string &full_path, const std::string &relative_path) {
  std::string resource_str = read_file(full_path);
  Interpreter interpreter(relative_path, file_mutex, stdout_mutex);
  const auto tag_size = sizeof(CKRIPT_START) - 1;
  while (true) {
    auto first = resource_str.find(CKRIPT_START);
    if (first == std::string::npos) break;
    auto last = resource_str.find(CKRIPT_END);
    const std::string &code = resource_str.substr(first + tag_size, last - first - tag_size);
    try {
      interpreter.process_string(code);
      resource_str = resource_str.replace(first, last - first + tag_size, interpreter.VM.output_buffer);
    } catch (const std::runtime_error& e) {
      if (std::string(e.what()) == "ckript abort()") {
        resource_str = resource_str.replace(first, resource_str.length(), interpreter.VM.output_buffer);
      } else {
        resource_str = "<body>" + interpreter.VM.error_buffer + "</body>";
      }
      break;
    }
  }
  return resource_str;
}

void Server::handle_client(const Client &client) {
  char buffer[REQUEST_BUFFER_SIZE];
  std::memset(buffer, 0, REQUEST_BUFFER_SIZE);
  int r = read(client.socket_fd, buffer, REQUEST_BUFFER_SIZE - 1);
  std::string data = buffer;
  const std::vector<std::string> &request_lines = split(data, '\n');
  const std::vector<std::string> &request = split(request_lines[0], ' ');
  const std::string &request_method = request[0];
  const std::string &full_request_path = request[1];
  const std::string &full_request = request_method + " " + full_request_path;
  const std::vector<std::string> &components = split(full_request_path, '?');
  const std::size_t components_size = components.size();
  if (components_size > 2) {
    // more than two "?" found
    // malformed request
    // write_404_res(client_fd);
    write_ok_res("Bye bye", client.socket_fd);
    return;
  }
  const std::string &request_path = components[0];
  const std::string &requested_resource = current_path + request_path;
  const auto &path = std::filesystem::path(requested_resource).lexically_normal();
  bool has_query = components_size == 2;
  if (has_query) {
    // TODO
  }
  std::cout << "full request " << full_request << std::endl;
  // TODO: make it more robust
  if (!safe_path(path)) {
    // send 404 back
    write_ok_res("illegal path", client.socket_fd);
    return;
  }
  if (!resource_exists(requested_resource)) {
    // send 404 back
    write_ok_res("bye bye", client.socket_fd);
    return;
  }
  if (path.extension() == ".ck") {
    // run the interpreter
    const std::string &code_output = process_code(requested_resource, request_path);
    write_ok_res(code_output, client.socket_fd);
    return;
  }
  write_ok_res(read_file(requested_resource), client.socket_fd);
}

void Server::accept_connections() {
  while (true) {
    Client client;
    client.socket_fd = accept(socket_fd, (sockaddr *)&client.info, &client.info_len);
    client.ip_addr = inet_ntoa(client.info.sin_addr);
    new std::thread(&Server::handle_client, this, client);
  }
}

void Server::serve(const int port) {
  std::cout << max_threads << " cores detected\n";
  socket_fd = create_server_socket(port);
  listen(socket_fd, MAX_CONNECTIONS);
  std::cout << "Listening on port " << port << "...\n";
  accept_connections();
}