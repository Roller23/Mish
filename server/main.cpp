#include "src/server.hpp"

int main(int argc, char *argv[]) {
  Server &server = Server::get();
  server.load_config_args(argc, argv);
  server.load_config_file();
  server.generate_threadpool();
  server.serve_http();
  return 0;
}