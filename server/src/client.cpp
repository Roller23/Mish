#include "client.hpp"
#include "map.hpp"
#include "status.hpp"
#include "server.hpp"

#include <cassert>
#include <unistd.h>

#if defined(__APPLE__)
  #include <uuid/uuid.h>
#endif

#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <unordered_map>

#include "../../utils/http.hpp"
#include "../../utils/utils.hpp"
#include "../../utils/date.hpp"
#include "../../utils/utils.hpp"

#define HTTP "HTTP/1.0"
#define HEADERS_END "\r\n\r\n"
#define HEADER_END "\r\n"

#define UUID_SIZE 36

static std::string uuid_to_str(unsigned char *uuid) {
  char str[UUID_SIZE + 1] = {};
  uint32_t data1 = *reinterpret_cast<uint32_t *>(uuid);
  uint16_t data2 = *reinterpret_cast<uint16_t *>(uuid + 4);
  uint16_t data3 = *reinterpret_cast<uint16_t *>(uuid + 6);
  snprintf(str,
    UUID_SIZE + 1,
    "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", 
    data1, data2, data3,
    uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]
  );
  return str;
}

static std::string generate_uuid(void) {
  unsigned char uuid[16];
  #if defined(__APPLE__)
    uuid_generate(uuid);
  #else
    uint64_t *long_uuid = (uint64_t *)uuid;
    static std::random_device rd;
    static std::default_random_engine generator(rd());
    static std::uniform_int_distribution<std::uint64_t> distribution(0, -1);
    long_uuid[0] = distribution(generator);
    long_uuid[1] = distribution(generator);
  #endif
  return uuid_to_str(uuid);
}

bool Request::has_headers() const {
  return buffer.find(HEADERS_END) != std::string::npos;
}

bool Request::has_body() const {
  if (!has_headers()) return false;
  const std::vector<std::string> &buffer_lines = Srv::Utils::split(buffer, '\n');
  const Map &req_headers = Http::read_headers(buffer_lines);
  if (!req_headers.has("content-length")) return true;
  const std::string headers_end = HEADERS_END;
  const std::size_t headers_length = buffer.find(headers_end) + headers_end.length();
  return buffer.length() - headers_length == std::stoi(get_header("Content-Length"));
}

void Request::parse_cookies(const std::string &cookie_header) {
  const auto &cookies = Srv::Utils::split(cookie_header, ';');
  for (const std::string &cookie_str : cookies) {
    const auto &entries = Srv::Utils::split(Srv::Utils::ltrim(cookie_str), '=');
    if (entries.size() != 2) continue;
    this->cookies.map[entries[0]] = entries[1];
  }
}

std::string Request::get_header(const std::string &key) const {
  const auto it = headers.map.find(Srv::Utils::to_lower(key));
  if (it == headers.map.end()) {
    return "";
  }
  return it->second;
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
  add_header("Date", Date::format("%c %Z"));
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

int Client::flush(void) {
  int w = write(socket_fd, res.output.c_str(), res.output.length());
  if (w == -1) {
    perror("write()");
    return w;
  }
  res.output = res.output.erase(0, w);
  return w;
}

void Client::_close(void) {
  close(socket_fd);
  closed = true;
}

bool Client::buffer_ready(void) const {
  return req.has_body();
}

void Client::enable_cors(void) {
  const std::string &requested_headers = req.get_header("Access-Control-Request-Headers");
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

int Client::write_and_attempt_close(void) {
  int w = flush();
  if (w == -1) return w;
  if (res.output.empty()) {
    _close();
  }
  return w;
}

int Client::end(const int code, const std::string &str, bool ignore_buffer) {
  assert(!request_processed);
  request_processed = true;
  res.end(code, str, ignore_buffer);
  return write_and_attempt_close();
}

void Client::start_session(void) {
  this->session.load();
}

void Client::end_session(void) {
  return this->session.destroy();
}

std::string Client::get_session_token(void) {
  return this->session.id;
}

void Session::load(void) {
  if (this->id != "" && !Server::check_session_id(this->id)) {
    this->id = "";
  }
  if (this->id == "") {
    this->id = generate_uuid();
  }
  Server::load_session(this->id);
}

void Session::destroy(void) {
  Server::destroy_session(this->id);
}

void Client::session_set(const std::string &key, const std::string &value) {
  Server::session_set(this->session.id, key, value);
}

void Client::session_unset(const std::string &key) {
  Server::session_unset(this->session.id, key);
}

std::string Client::session_get(const std::string &key) const {
  return Server::session_get(this->session.id, key);
}

bool Client::session_has(const std::string &key) const {
  return Server::session_has(this->session.id, key);
}