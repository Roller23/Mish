#include <fstream>
#include <queue>
#include <iostream>
#include <vector>
#include <queue>

#include <poll.h>
#include <cassert>

#include "worker.hpp"
#include "mime.hpp"

#include "../../ckript/src/interpreter.hpp"
#include "../../utils/path.hpp"
#include "../../utils/utils.hpp"
#include "../../utils/http.hpp"

#define CKRIPT_START "<&"
#define CKRIPT_END "&>"

const std::vector<std::string> Worker::valid_methods = {
  "GET", "POST", "DELETE", "PUT", "PATCH"
};

static std::string read_file(const std::string &path) {
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

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
  std::string resource_str = read_file(full_path);
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

void Worker::read_client(Client &client) {
  std::memset(temp_buffer, 0, TEMP_BUFFER_SIZE);
  int r = read(client.socket_fd, temp_buffer, TEMP_BUFFER_SIZE - 1);
  client.req.buffer += temp_buffer;
}

void Worker::handle_client(Client &client) {
  const std::vector<std::string> &request_lines = Srv::Utils::split(client.req.buffer, '\n');
  client.req.headers = Http::read_headers(request_lines);
  const std::string &content_length_str = client.req.headers.get("Content-Length");
  if (content_length_str != "") {
    client.req.length = std::stoul(content_length_str);
  }
  if (client.req.length > max_body_size) {
    return client.end(Status::RequestEntityTooLarge);
  }
  if (client.req.length > TEMP_BUFFER_SIZE) {
    // TODO: read the missing body parts
  }
  const std::vector<std::string> &request = Srv::Utils::split(request_lines[0], ' ');
  client.req.method = request[0];
  if (std::find(valid_methods.begin(), valid_methods.end(), client.req.method) == valid_methods.end()) {
    // TODO: set the correct status code
    return client.end(Status::BadRequest);
  }
  if (client.req.method == "POST") {
    // read body
    client.req.raw_body = Http::read_body(client.req.buffer, client.req.length);
    client.req.body = Http::parse_payload(client.req.raw_body);
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
  const auto &path = std::filesystem::path(requested_resource).lexically_normal();
  bool has_query = components_size == 2;
  if (has_query) {
    client.req.query = Http::parse_payload(components[1]);
  }
  std::cout << "full request " << full_request << std::endl;
  if (!Path::safe(path)) {
    return client.end(Status::NotFound);
  }
  if (!Path::resource_exists(requested_resource)) {
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
  // pollfd client_pfd;
  // client_pfd.fd = client.socket_fd;
  // client_pfd.events = POLLIN | POLLHUP;
  // client_pfd.revents = 0;
  // pfds.push_back(client_pfd);
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
      std::exit(EXIT_FAILURE);
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
      Client &client = client_queue.front();
      read_client(client);
      if (!client.buffer_ready()) {
        if (client.req.buffer.length() > max_headers_size) {
          client.end(Status::RequestHeaderFieldsTooLarge);
          client_queue.pop();
        }
        continue;
      }
      handle_client(client);
      client_queue.pop();
    }
  }
}