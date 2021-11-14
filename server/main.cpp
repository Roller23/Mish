#include "src/server.hpp"

#include <signal.h>

int main(int argc, char *argv[]) {
  signal(SIGPIPE, SIG_IGN); // ignore SIGPIPE for safety reasons

  Server &server = Server::get();
  server.load_config_args(argc, argv);
  server.load_config_file();
  server.generate_threadpool();
  server.serve_http();
  return 0;
}