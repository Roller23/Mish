#include "client.hpp"

#define HTTP "HTTP/1.0"
#define HEADERS_END "\r\n\r\n"

void Response::append(const std::string &str) {
  buffer += str;
  headers.map["Content-Length"] = std::to_string(buffer.size());
}

void Response::add_header(const std::string &key, const std::string &value) {
  headers.map[key] = value;
}

const std::string &Response::get_header(const std::string &key) {
  return headers.map[key];
}

void Response::end(const int code, const std::string &str) {
  append(str);
  output = HTTP;
  output += " " + std::to_string(code) + " " + Status::to_string(code);
  for (const auto &it : headers.map) {
    output += "\n" + it.first + ": " + it.second;
  }
  output += HEADERS_END;
  output += this->buffer;
}

void Client::flush(void) const {
  write(socket_fd, res.output.c_str(), res.output.length());
  close(socket_fd);
}

bool Client::buffer_ready(void) const {
  return true;
}

void Client::end(const int code, const std::string &str) {
  this->res.end(code, str);
  this->flush();
}