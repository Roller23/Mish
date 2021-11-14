#include <fstream>
#include <queue>
#include <iostream>
#include <vector>
#include <queue>

#include <poll.h>
#include <cassert>

#include "worker.hpp"
#include "mime.hpp"
#include "status.hpp"

#include "../../ckript/src/interpreter.hpp"
#include "../../utils/path.hpp"
#include "../../utils/utils.hpp"
#include "../../utils/http.hpp"
#include "../../utils/file.hpp"

#define CKRIPT_START "<&"
#define CKRIPT_END "&>"

const std::vector<std::string> Worker::valid_methods = {
  "GET", "POST", "DELETE", "PUT", "PATCH", "HEAD", "OPTIONS"
};

const std::vector<std::string> Worker::methods_containing_bodies = {
  "POST", "PUT", "PATCH"
};

const std::string Worker::allowed_methods = "GET, POST, DELETE, PUT, PATCH, HEAD, OPTIONS";
const std::string Worker::ckript_abort_message = "ckript abort()";

static std::queue<std::uint64_t> get_lines_offsets(const std::string &str) {
  std::queue<std::uint64_t> res;
  std::uint64_t line = 1;
  std::uint64_t endings = 0;
  const std::size_t length = str.length();
  for (std::size_t i = 0; i < length; i++) {
    if (str[i] == '\n') line++;
    if (str[i] == '<' && str[i + 1] == '&') {
      res.push(line);
    }
    if (str[i] == '&' && str[i + 1] == '>') {
      endings++;
    }
  }
  if (endings != res.size()) {
    throw std::runtime_error("Mismatched Ckript tags");
  }
  return res;
}

std::string Worker::process_code(const std::string &full_path, const std::string &relative_path, Client &client) {
  std::string resource_str = File::read(full_path);
  std::queue<std::uint64_t> lines_offsets;
  try {
    lines_offsets = get_lines_offsets(resource_str);
  } catch(const std::runtime_error &e) {
    return "<body>Parsing error: Mismatched Ckript tags number</body>";
  }
  Interpreter interpreter(relative_path, file_mutex, stdout_mutex, client);
  const auto tag_size = sizeof(CKRIPT_START) - 1;
  while (true) {
    auto first = resource_str.find(CKRIPT_START);
    if (first == std::string::npos) break;
    auto last = resource_str.find(CKRIPT_END);
    const std::string &code = resource_str.substr(first + tag_size, last - first - tag_size);
    try {
      if (lines_offsets.empty()) {
        return "<body>Parsing error: Mismatched Ckript tags number</body>";
      }
      interpreter.process_string(code, lines_offsets.front());
      lines_offsets.pop();
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

int Worker::read_client(Client &client) {
  std::memset(temp_buffer, 0, TEMP_BUFFER_SIZE);
  int r = read(client.socket_fd, temp_buffer, TEMP_BUFFER_SIZE - 1);
  client.req.buffer += temp_buffer;
  return r;
}

void Worker::remove_client(Client &client, pollfd &pfd) {
  clients.erase(client.socket_fd);
  pfd.fd = -1;
  clients_polled--;
}

void Worker::handle_client(Client &client) {
  const std::vector<std::string> &request_lines = Srv::Utils::split(client.req.buffer, '\n');
  client.req.headers = Http::read_headers(request_lines);
  const std::string &content_length_str = client.req.headers.get("Content-Length");
  if (content_length_str != "") {
    client.req.length = std::stoul(content_length_str);
  }
  if (client.req.length > config.max_body_size) {
    return client.end(Status::RequestEntityTooLarge);
  }
  const std::vector<std::string> &request = Srv::Utils::split(request_lines[0], ' ');
  client.req.method = request[0];
  if (Srv::Utils::vector_contains(valid_methods, client.req.method)) {
    // TODO: set the correct status code
    return client.end(Status::BadRequest);
  }
  const bool is_options = client.req.method == "OPTIONS";
  const bool headers_only = client.req.method == "HEAD" || is_options;
  if (Srv::Utils::vector_contains(methods_containing_bodies, client.req.method)) {
    // read body
    client.req.raw_body = Http::read_body(client.req.buffer, client.req.length);
    if (client.req.headers.get("Content-Type") == "application/x-www-form-urlencoded") {
      client.req.body = Http::parse_payload(client.req.raw_body);
    }
  }
  const std::string &full_request_path = request[1];
  const std::string &full_request = client.req.method + " " + full_request_path;
  const std::vector<std::string> &components = Srv::Utils::split(full_request_path, '?');
  const std::size_t components_size = components.size();
  if (components_size > 2) {
    // malformed request
    return client.end(Status::BadRequest);
  }
  const std::string &request_path = components[0];
  const std::string &requested_resource = current_path + request_path;
  auto path = std::filesystem::path(requested_resource).lexically_normal();
  bool has_query = components_size == 2;
  if (has_query) {
    client.req.query = Http::parse_payload(components[1]);
  }
  std::cout << "Request received: " << full_request << std::endl;
  if (!Path::safe(path)) {
    return client.end(Status::NotFound);
  }
  const bool is_index_dir = Path::is_index_directory(path);
  if (is_index_dir && request_path.back() != '/') {
    // redirect
    client.res.add_header("Location", request_path + "/");
    return client.end(Status::TemporaryRedirect);
  }
  if (!Path::resource_exists(path) && !is_index_dir) {
    return client.end(Status::NotFound);
  }
  if (is_index_dir) {
    path = path / "index.ck";
  }
  const std::string &ext = path.extension();
  if (ext == ".ck") {
    // run the interpreter
    const std::string &code_output = process_code(path, request_path, client);
    if (client.should_enable_cors || config.global_cors_enabled) {
      client.enable_cors();
    }
    if (is_options) {
      client.res.add_header("Allow", allowed_methods);
    }
    return client.end(client.res.script_code, code_output, headers_only);
  }
  // non-ckript resource request
  const std::string &mime_type = Mime::ext_to_mime(ext);
  client.res.add_header("Content-Type", mime_type);
  if (config.global_cors_enabled) {
    client.enable_cors();
  }
  if (is_options) {
    client.res.add_header("Allow", allowed_methods);
  }
  client.end(Status::OK, File::read(requested_resource), headers_only);
}

bool Worker::add_client(Client &client) {
  for (int i = 1; i < PFDS_SIZE; i++) {
    if (pfds[i].fd == -1) {
      pfds[i].fd = client.socket_fd;
      pfds[i].revents = 0;
      clients[client.socket_fd] = client;
      clients_polled++;
      return true;
    }
  }
  return false;
}

void Worker::start_thread(void) {
  thread = new std::thread(&Worker::manage_clients, this);
}

void Worker::read_pipe(void) const {
  char payload;
  int r = read(_pipe[PIPE_READ], &payload, sizeof(payload));
  assert(r == sizeof(payload) && payload == PIPE_PAYLOAD);
}

void Worker::report_back(void) const {
  // reports back to the server after being woken up
  char payload = Worker::PIPE_PAYLOAD;
  int w = write(server_pipe[Worker::PIPE_WRITE], &payload, sizeof(payload));
  assert(w == sizeof(payload));
}

inline bool Worker::can_read_fd(const pollfd &pfd) {
  return pfd.revents & POLLIN;
}

inline bool Worker::fd_hung_up(const pollfd &pfd) {
  return pfd.revents & POLLHUP;
}

inline bool Worker::can_write_fd(const pollfd &pfd) {
  return pfd.revents & POLLOUT;
}

void Worker::manage_clients(void) {
  while (true) {
    int res = poll(pfds, PFDS_SIZE, poll_timeout);
    if (res < 0) {
      std::perror("poll()");
      std::exit(EXIT_FAILURE);
    }
    for (int i = 0; i < PFDS_SIZE; i++) {
      pollfd &pfd = pfds[i];
      if (pfd.fd == _pipe[PIPE_READ]) {
        if (can_read_fd(pfd)) {
          // pipe was used to wake up poll()
          read_pipe();
          if (pending_client != nullptr) {
            Client client = *pending_client;
            delete pending_client;
            pending_client = nullptr;
            bool success = add_client(client);
            report_back();
            if (!success) {
              client.end(Status::ServiceUnavailable);
            }
          } else {
            report_back();
          }
        }
        continue;
      }
      auto it = clients.find(pfd.fd);
      if (it == clients.end()) {
        // client not found
        pfd.fd = -1;
        continue;
      }
      Client &client = it->second;
      if (fd_hung_up(pfd)) {
        // client disconnected
        remove_client(client, pfd);
        continue;
      }
      if (can_read_fd(pfd)) {
        int r = read_client(client);
        if (r == -1) {
          // an error occured while trying to read from socket
          perror("read()");
          close(client.socket_fd);
          remove_client(client, pfd);
          continue;
        }
        if (r == 0) {
          // client disconnected while trying to read from socket
          remove_client(client, pfd);
          continue;
        }
      }
      if (!client.buffer_ready()) {
        if (client.req.buffer.length() > config.max_headers_size) {
          client._close();
          remove_client(client, pfd);
        }
        continue;
      }
      if (!client.request_processed) {
        handle_client(client);
        if (client.closed) {
          remove_client(client, pfd);
        }
        continue;
      }
      if (can_write_fd(pfd)) {
        client.attempt_close();
      }
      if (client.closed) {
        remove_client(client, pfd);
      }
    }
  }
}