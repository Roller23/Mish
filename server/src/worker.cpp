#include <fstream>
#include <queue>
#include <iostream>

#include "worker.hpp"
#include "mime.hpp"

#include <poll.h>
#include <cassert>

#include "../../ckript/src/interpreter.hpp"
#include "../../utils/uri.hpp"

#define CKRIPT_START "<&"
#define CKRIPT_END "&>"
#define HEADERS_END "\r\n\r\n"

static std::string read_file(const std::string &path) {
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
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

static std::string &ltrim(std::string &str, const char *whitespace = " \t") {
  str.erase(0, str.find_first_not_of(whitespace));
  return str;
}

static Map read_headers(const std::vector<std::string> &lines) {
  if (lines.size() <= 1) return {}; 
  Map res;
  for (std::size_t i = 1; i < lines.size(); i++) {
    if (lines[i].find(":") != std::string::npos) {
      std::vector<std::string> header_components = split(lines[i], ':');
      if (header_components.size() != 2) {
        continue; // TODO
      }
      std::string header_value = ltrim(header_components[1]);
      const auto length = header_value.length();
      if (length > 0 && header_value[length - 1] == '\r') {
        header_value.pop_back();
      }
      res.map[header_components[0]] = header_value;
    }
    if (lines[i] == "\r") break;
  }
  return res;
}

static std::string read_body(const std::string &str, const std::size_t n = 0) {
  const std::string headers_end = HEADERS_END;
  std::size_t pos = str.find(headers_end);
  if (pos == std::string::npos) return "";
  const std::size_t idx = pos + headers_end.length();
  if (idx + n > str.length()) return "";
  if (n != 0) {
    return str.substr(idx, n);
  }
  return str.substr(idx);
}

static Map parse_payload(const std::string &str, bool is_urlencoded = false) {
  Map res;
  const std::vector<std::string> &pairs = split(str, '&');
  for (auto &pair : pairs) {
    const std::vector<std::string> pair_components = split(pair, '=');
    if (pair_components.size() != 2) continue; // TODO: return 400 or something
    std::string value = pair_components[1];
    if (is_urlencoded) {
      std::replace(value.begin(), value.end(), '+', ' ');
    }
    res.map[pair_components[0]] = Uri::decode_component(value);
  }
  return res;
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

std::string Worker::process_code(const std::string &full_path, const std::string &relative_path, Client &client) {
  std::string resource_str = read_file(full_path);
  Interpreter interpreter(relative_path, file_mutex, stdout_mutex, client);
  const auto tag_size = sizeof(CKRIPT_START) - 1;
  while (true) {
    auto first = resource_str.find(CKRIPT_START);
    if (first == std::string::npos) break;
    auto last = resource_str.find(CKRIPT_END);
    const std::string &code = resource_str.substr(first + tag_size, last - first - tag_size);
    try {
      interpreter.process_string(code);
      resource_str = resource_str.replace(first, last - first + tag_size, interpreter.VM.output_buffer);
    } catch (const std::runtime_error &e) {
      if (e.what() == ckript_abort_message) {
        resource_str = resource_str.replace(first, resource_str.length(), interpreter.VM.output_buffer);
      } else {
        resource_str = "<body>" + interpreter.VM.error_buffer + "</body>";
      }
      break;
    }
  }
  // destroy the interpreter to prevent memory leaks
  interpreter.destroy();
  return resource_str;
}

void Worker::handle_client(Client &client) {
  std::memset(temp_buffer, 0, TEMP_BUFFER_SIZE);
  int r = read(client.socket_fd, temp_buffer, TEMP_BUFFER_SIZE - 1);
  client.buffer += temp_buffer;
  const std::vector<std::string> &request_lines = split(client.buffer, '\n');
  client.req.headers = read_headers(request_lines);
  const bool is_urlencoded = client.req.headers.get("Content-Type") == "application/x-www-form-urlencoded";
  if (client.req.headers.has("Content-Length")) {
    client.req.length = std::stoul(client.req.headers.get("Content-Length"));
  }
  if (client.req.length > TEMP_BUFFER_SIZE) {
    // TODO: read the missing body parts
  }
  const std::vector<std::string> &request = split(request_lines[0], ' ');
  const std::string &request_method = request[0];
  if (request_method == "POST") {
    // read body
    client.req.raw_body = read_body(client.buffer, client.req.length);
    client.req.body = parse_payload(client.req.raw_body, is_urlencoded);
  }
  const std::string &full_request_path = request[1];
  const std::string &full_request = request_method + " " + full_request_path;
  const std::vector<std::string> &components = split(full_request_path, '?');
  const std::size_t components_size = components.size();
  if (components_size > 2) {
    // malformed request
    return client.end(Status::BadRequest);
  }
  const std::string &request_path = components[0];
  const std::string &requested_resource = current_path + request_path;
  const auto &path = std::filesystem::path(requested_resource).lexically_normal();
  bool has_query = components_size == 2;
  if (has_query) {
    client.req.query = parse_payload(components[1], is_urlencoded);
  }
  std::cout << "full request " << full_request << std::endl;
  if (!safe_path(path)) {
    return client.end(Status::NotFound);
  }
  if (!resource_exists(requested_resource)) {
    return client.end(Status::NotFound);
  }
  const std::string ext = path.extension();
  if (ext == ".ck") {
    // run the interpreter
    const std::string &code_output = process_code(requested_resource, request_path, client);
    return client.end(client.res.script_code, code_output);
  }
  const std::string &mime_type = Mime::ext_to_mime(ext);
  client.res.add_header("Content-Type", mime_type);
  client.end(Status::OK, read_file(requested_resource));
}

void Worker::add_client(Client &client) {
  client_queue.push(client);
}

void Worker::start_thread(void) {
  thread = new std::thread(&Worker::manage_clients, this);
}

void Worker::read_pipe(void) const {
  char payload;
  int r = read(_pipe[PIPE_READ], &payload, sizeof(payload));
  assert(r == sizeof(payload) && payload == PIPE_PAYLOAD);
}

bool inline Worker::can_read_fd(const pollfd &pfd) {
  return pfd.revents & POLLIN;
}

void Worker::manage_clients(void) {
  while (true) {
    int res = poll(pfds.data(), pfds.size(), poll_timeout);
    if (res < 0) {
      std::perror("poll()");
      std::exit(0);
    }
    for (const pollfd &pfd : pfds) {
      if (can_read_fd(pfd)) {
        if (pfd.fd == _pipe[PIPE_READ]) {
          // pipe was used to wake up poll()
          read_pipe();
        }
      }
    }
    while (client_queue.size() != 0) {
      handle_client(client_queue.front());
      client_queue.pop();
    }
  }
}