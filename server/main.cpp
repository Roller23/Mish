#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>

#include <iostream>
#include <vector>

#define HEADERS_END "\r\n\r\n"
#define HTTP_200 "HTTP/1.0 200 OK"
#define HTTP_404 "HTTP/1.0 404 Not Found"

typedef struct sockaddr SA;
typedef struct sockaddr_in SA_IN;

static std::vector<std::string> tokenize(const std::string &str, char delim) {
  std::size_t start;
  std::size_t end = 0;
  std::vector<std::string> out;
  while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
    end = str.find(delim, start);
    out.push_back(str.substr(start, end - start));
  }
  return out;
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

int main(int argc, char *argv[]) {
  const int port = 8080;
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  int option = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  SA_IN address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(socket_fd, (SA *)&address, sizeof(address));
  listen(socket_fd, 1000);
  std::cout << "Listening on port " << port << "...\n";
  while (true) {
    SA client_info;
    socklen_t info_len;
    int client_fd = accept(socket_fd, (SA *)&client_info, &info_len);
    struct sockaddr_in *inaddr = (struct sockaddr_in *)&client_info;
    char *client_ip = inet_ntoa(inaddr->sin_addr);

    char buffer[1024 * 10];
    memset(buffer, 0, sizeof(buffer));
    int r = read(client_fd, buffer, sizeof(buffer) - 1);
    std::string data = buffer;
    const std::vector<std::string> &request_lines = tokenize(data, '\n');
    const std::vector<std::string> &request = tokenize(request_lines[0], ' ');
    const std::string &method = request[0];
    const std::string &path = request[1];
    const std::string &full_request = method + " " + path;
    const std::vector<std::string> &components = tokenize(full_request, '?');
    if (components.size() > 2) {
      // malformed request
      // write_404_res(client_fd);
      write_ok_res("Bye bye", client_fd);
      continue;
    }
    std::cout << "full request " << full_request << std::endl;
    write_ok_res("Hi", client_fd);
  }
  return 0;
}