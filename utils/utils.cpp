#include "utils.hpp"

#include <string>
#include <vector>

std::vector<std::string> Srv::Utils::split(const std::string &str, char delim) {
  std::size_t start;
  std::size_t end = 0;
  std::vector<std::string> out;
  while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
    end = str.find(delim, start);
    out.push_back(str.substr(start, end - start));
  }
  return out;
}

std::string Srv::Utils::ltrim(std::string str, const char *whitespace) {
  return str.erase(0, str.find_first_not_of(whitespace));
}