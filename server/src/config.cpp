#include "config.hpp"

#include <cstdlib>

#include <iostream>
#include <string>
#include <filesystem>

static void abort_loading(const std::string &msg) {
  std::cout << "Error loading config: " << msg << std::endl;
  std::exit(EXIT_FAILURE);
}

void Config::load_option(const std::string &option, const std::string &value) {
  if (option == "port") {
    int selected_port = std::stoi(value);
    if (selected_port < 0x1 || selected_port > 0xffff) {
      abort_loading("port value must be between 1 and 65535");
    }
    port = selected_port;
  } else if (option == "max_connections") {
    int selected_connections = std::stoi(value);
    if (selected_connections < 1) {
      abort_loading("max connections value can't be negative or zero");
    }
    max_connections = selected_connections;
  } else if (option == "request_size") {
    // value * megabytes
    max_body_size = std::stoi(value) * 1024 * 1024;
  } else if (option == "headers_size") {
    // value * kilobytes
    max_headers_size = std::stoi(value) * 1024;
  } else if (option == "enable_cors") {
    global_cors_enabled = true;
  } else if (option == "threads") {
    int threads = std::stoi(value);
    if (threads < 1) {
      abort_loading("threads number must be positive");
    }
    max_threads = threads;
  } else if (option == "config_file") {
    config_file = value;
  } else if (option == "root") {
    std::filesystem::current_path(value);
  } else {
    abort_loading("unknown option '" + option + "'");
  }
}