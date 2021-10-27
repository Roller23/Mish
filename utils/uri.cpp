#include "uri.hpp"

#include <string>
#include <cstring>

std::string Uri::decode_component(const std::string &str) {
  std::string str_copy = str;
  char hex[] = "00";
  bool decoded = false;
  while (!decoded) {
    decoded = true;
    for (std::size_t i = 0; i < str_copy.length(); i++) {
      if (str_copy[i] == '%') {
        if (str_copy[i+1] == 0) return str_copy;
        if (std::isxdigit(str_copy[i + 1]) && std::isxdigit(str_copy[i + 2])) {
          decoded = false;

          hex[0] = str_copy[i + 1];
          hex[1] = str_copy[i + 2];

          char x = (char)std::strtol(hex, NULL, 16);
          std::memmove(&str_copy[i + 1], &str_copy[i + 3], std::strlen(&str_copy[i + 3]) + 1);
          str_copy[i] = x;
        }
      }
    }
  }
  return str_copy;
}