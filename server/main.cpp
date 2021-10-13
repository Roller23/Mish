#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>

#include "../ckript/src/interpreter.hpp"
#include "src/server.hpp"

int main(int argc, char *argv[]) {
  Server().serve_http(8080);
  return 0;
}