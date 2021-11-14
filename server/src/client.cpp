#include "client.hpp"
#include "map.hpp"
#include "status.hpp"

#include <cassert>

#include "../../utils/http.hpp"
#include "../../utils/utils.hpp"

#define HTTP "HTTP/1.0"
#define HEADERS_END "\r\n\r\n"
#define HEADER_END "\r\n"

bool Request::has_headers() const {
  return buffer.find(HEADERS_END) != std::string::npos;
}

bool Request::has_body() const {
  if (!has_headers()) return false;
  const std::vector<std::string> &buffer_lines = Srv::Utils::split(buffer, '\n');
  const Map &req_headers = Http::read_headers(buffer_lines);
  if (!req_headers.has("Content-Length")) return true;
  const std::string headers_end = HEADERS_END;
  const std::size_t headers_length = buffer.find(headers_end) + headers_end.length();
  return buffer.length() - headers_length == std::stoi(req_headers.get("Content-Length"));
}

void Response::append(const std::string &str) {
  buffer += str;
  headers.map["Content-Length"] = std::to_string(buffer.length());
}

void Response::add_header(const std::string &key, const std::string &value) {
  headers.map[key] = value;
}

const std::string &Response::get_header(const std::string &key) {
  return headers.map[key];
}

void Response::end(const int code, const std::string &str, bool ignore_buffer) {
  append(str);
  output = HTTP;
  output += " " + std::to_string(code) + " " + Status::to_string(code);
  for (const auto &it : headers.map) {
    output += HEADER_END + it.first + ": " + it.second;
  }
  output += HEADERS_END;
  if (!ignore_buffer) {
    output += this->buffer;
  }
}

void Client::flush(void) {
  int w = write(socket_fd, res.output.c_str(), res.output.length());
  if (w == -1) {
    perror("write()");
    return;
  }
  res.output = res.output.erase(0, w);
}

void Client::_close(void) {
  close(socket_fd);
  closed = true;
}

bool Client::buffer_ready(void) const {
  return req.has_body();
}

void Client::enable_cors(void) {
  const std::string &requested_headers = req.headers.get("Access-Control-Request-Headers");
  res.add_header("Access-Control-Allow-Origin", "*");
  res.add_header("Access-Control-Allow-Methods", "GET, HEAD, PUT, PATCH, POST, DELETE");
  if (!requested_headers.empty()) {
    res.add_header("Access-Control-Allow-Headers", requested_headers);
  }
  if (req.method == "OPTIONS") {
    res.add_header("Content-Length", "0");
    res.script_code = Status::NoContent;
  }
}

void Client::attempt_close(void) {
  flush();
  if (res.output.empty()) {
    _close();
  }
}

void Client::end(const int code, const std::string &str, bool ignore_buffer) {
  assert(!request_processed);
  request_processed = true;
  res.end(code, str, ignore_buffer);
  attempt_close();
}