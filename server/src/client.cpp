#include "client.hpp"
#include "map.hpp"
#include "status.hpp"

#include "../../utils/http.hpp"
#include "../../utils/utils.hpp"

#define HTTP "HTTP/1.0"
#define HEADERS_END "\r\n\r\n"

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

void Client::flush(void) {
  write(socket_fd, res.output.c_str(), res.output.length());
  res.output = "";
}

void Client::_close(void) const {
  close(socket_fd);
}

bool Client::buffer_ready(void) const {
  return req.has_body();
}

void Client::enable_cors(void) {
  const std::string &requested_headers = req.headers.get("Access-Control-Request-Headers");
  res.add_header("Access-Control-Allow-Origin", "*");
  res.add_header("Access-Control-Allow-Methods", "GET,HEAD,PUT,PATCH,POST,DELETE");
  if (!requested_headers.empty()) {
    res.add_header("Access-Control-Allow-Headers", requested_headers);
  }
  if (req.method == "OPTIONS") {
    res.add_header("Content-Length", "0");
    res.script_code = Status::NoContent;
  }
}

void Client::end(const int code, const std::string &str) {
  res.end(code, str);
  flush();
  _close();
}