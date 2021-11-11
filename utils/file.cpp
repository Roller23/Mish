#include "file.hpp"

#include <string>
#include <fstream>
#include <sstream>

std::string File::read(const std::string &path) {
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}