#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <filesystem>
#include <string>
#include <sstream>
#include <fstream>

#include "../ckript/src/interpreter.hpp"

#define HEADERS_END "\r\n\r\n"
#define HTTP_200 "HTTP/1.0 200 OK"
#define HTTP_404 "HTTP/1.0 404 Not Found"

#define REQUEST_BUFFER_SIZE (1024 * 10)
#define MAX_CONNECTIONS 1000

typedef struct sockaddr SA;
typedef struct sockaddr_in SA_IN;

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

bool has_suffix(const std::string &str, const std::string &suffix) {
  return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
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

static std::string generate_ok_res(const std::string &content) {
  std::string response = HTTP_200;
  // for (const auto &it : headers) {
  //   response += "\n" + it.first + ": " + it.second;
  // }
  response += HEADERS_END;
  response += content;
  return response;
}

static void write_ok_res(const std::string &content, int client) {
  std::string response = generate_ok_res(content);
  const char *response_c = response.c_str();
  write(client, response_c, strlen(response_c));
  close(client);
}

static int create_server_socket(const int port) {
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  int option = 1;
  // make the socket address reusable
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  SA_IN address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(socket_fd, (SA *)&address, sizeof(address));
  return socket_fd;
}

static std::string process_code(const std::string &path) {
  std::string resource_str = read_file(path);
  Interpreter interpreter;
  while (true) {
    auto first = resource_str.find("<&");
    if (first == std::string::npos) break;
    auto last = resource_str.find("&>");
    const std::string &code = resource_str.substr(first + 2, last - first - 2);
    const std::string &code_output = interpreter.process_string(code);
    if (interpreter.VM.aborted_early) {
      resource_str = resource_str.replace(first, resource_str.length(), code_output);
      break; 
    }
    resource_str = resource_str.replace(first, last - first + 2, code_output);
  }
  return resource_str;
}

static void serve_http(const int port) {
  int socket_fd = create_server_socket(port);
  listen(socket_fd, MAX_CONNECTIONS);
  std::cout << "Listening on port " << port << "...\n";
  const std::string &current_path = std::filesystem::current_path().string();
  while (true) {
    SA client_info;
    socklen_t info_len;
    int client_fd = accept(socket_fd, (SA *)&client_info, &info_len);
    struct sockaddr_in *inaddr = (struct sockaddr_in *)&client_info;
    char *client_ip = inet_ntoa(inaddr->sin_addr);

    char buffer[REQUEST_BUFFER_SIZE];
    std::memset(buffer, 0, REQUEST_BUFFER_SIZE);
    int r = read(client_fd, buffer, REQUEST_BUFFER_SIZE - 1);
    std::string data = buffer;
    const std::vector<std::string> &request_lines = split(data, '\n');
    const std::vector<std::string> &request = split(request_lines[0], ' ');
    const std::string &method = request[0];
    const std::string &path = request[1];
    const std::string &full_request = method + " " + path;
    const std::vector<std::string> &components = split(path, '?');
    const std::size_t components_size = components.size();
    if (components_size > 2) {
      // more than two "?" found
      // malformed request
      // write_404_res(client_fd);
      write_ok_res("Bye bye", client_fd);
      continue;
    }
    const std::string &request_path = components[0];
    const std::string &requested_resource = current_path + request_path;
    bool has_query = components_size == 2;
    if (has_query) {
      // TODO
    }
    std::cout << "full request " << full_request << std::endl;
    std::cout << "requested resource " << requested_resource << std::endl;
    if (!resource_exists(requested_resource)) {
      // send 404 back
      write_ok_res("bye bye", client_fd);
      continue;
    }
    if (has_suffix(requested_resource, ".ck")) {
      // run the interpreter
      const std::string &resource_str = process_code(requested_resource);
      write_ok_res(resource_str, client_fd);
      continue;
    }
    const std::string &resource_str = read_file(requested_resource);
    write_ok_res(resource_str, client_fd);
  }
}

int main(int argc, char *argv[]) {
  const int port = 8080;
  serve_http(port);
  return 0;
}