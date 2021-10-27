#include <fstream>
#include <queue>
#include <iostream>

#include "worker.hpp"
#include "mime.hpp"

#include <poll.h>
#include <cassert>

#include "../../ckript/src/interpreter.hpp"

#define REQUEST_BUFFER_SIZE (1024 * 10)
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
      res.map[header_components[0]] = ltrim(header_components[1]);
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

static Map parse_payload(const std::string &str) {
  Map res;
  const std::vector<std::string> &pairs = split(str, '&');
  for (auto &pair : pairs) {
    const std::vector<std::string> pair_components = split(pair, '=');
    if (pair_components.size() != 2) continue; // TODO: return 400 or something
    res.map[pair_components[0]] = pair_components[1];
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
  // destroy the heap to prevent memory leaks
  interpreter.VM.heap.destroy();
  return resource_str;
}

void Worker::handle_client(Client &client) {
  char buf[REQUEST_BUFFER_SIZE];
  std::memset(buf, 0, REQUEST_BUFFER_SIZE);
  int r = read(client.socket_fd, buf, REQUEST_BUFFER_SIZE - 1);
  client.buffer += buf;
  const std::vector<std::string> &request_lines = split(client.buffer, '\n');
  client.req.headers = read_headers(request_lines);
  if (client.req.headers.map.count("Content-Length") != 0) {
    client.req.length = std::stoul(client.req.headers.map["Content-Length"]);
  }
  if (client.req.length > REQUEST_BUFFER_SIZE) {
    // TODO: read the missing body parts
  }
  const std::vector<std::string> &request = split(request_lines[0], ' ');
  const std::string &request_method = request[0];
  if (request_method == "POST") {
    // read body
    client.req.raw_body = read_body(client.buffer, client.req.length);
    client.req.body = parse_payload(client.req.raw_body);
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
    client.req.query = parse_payload(components[1]);
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

void Worker::manage_clients(void) {
  while (true) {
    int res = poll(fds.data(), fds.size(), poll_timeout);
    if (res < 0) {
      std::cout << "poll returned " << res << std::endl;
      std::exit(0);
    }
    if (fds[0].fd == _pipe[0]) {
      // pipe was used to wake up poll()
      char payload;
      int r = read(_pipe[0], &payload, sizeof(payload));
      assert(r == 1 && payload == 23);
    }
    while (client_queue.size() != 0) {
      handle_client(client_queue.front());
      client_queue.pop();
    }
  }
}