#include "client.hpp"

bool Query::has(const std::string &key) const {
  return query.count(key) != 0;
}

std::string Query::get(const std::string &key) const {
  if (!has(key)) return "";
  return query.at(key);
}

void Response::append(const std::string &str) {
  buffer += str;
  headers["Content-Length"] = std::to_string(buffer.size());
}

void Response::add_header(const std::string &key, const std::string &value) {
  headers[key] = value;
}

void Response::end(const std::string &str, const int code) {
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

void Client::end(const std::string &str, const int code) {
  this->res.end(str, code);
  this->flush();
}