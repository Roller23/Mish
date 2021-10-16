#include "client.hpp"

bool Query::has(const std::string &key) const {
  return map.count(key) != 0;
}

std::string Query::get(const std::string &key) const {
  if (!has(key)) return "";
  return map.at(key);
}

void Response::append(const std::string &str) {
  buffer += str;
  headers["Content-Length"] = std::to_string(buffer.size());
}

void Response::add_header(const std::string &key, const std::string &value) {
  headers[key] = value;
}

const std::string &Response::get_header(const std::string &key) {
  return headers[key];
}

void Response::end(const int code, const std::string &str) {
  append(str);
  output = HTTP;
  output += " " + std::to_string(code) + " " + Status::to_string(code);
  for (const auto &it : headers) {
    output += "\n" + it.first + ": " + it.second;
  }
  output += HEADERS_END;
  output += this->buffer;
}

void Client::flush(void) const {
  write(socket_fd, res.output.c_str(), res.output.length());
  close(socket_fd);
}

void Client::end(const int code, const std::string &str) {
  this->res.end(code, str);
  this->flush();
}