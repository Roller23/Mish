#include "src/server.hpp"

int main(int argc, char *argv[]) {
  Server::serve_http(8080);
  return 0;
}