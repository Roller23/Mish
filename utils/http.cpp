#include "http.hpp"
#include "utils.hpp"

#include <vector>
#include <string>
#include <algorithm>

#include "../server/src/map.hpp"
#include "utils.hpp"
#include "uri.hpp"

#define HEADERS_END "\r\n\r\n"

Map Http::read_headers(const std::vector<std::string> &lines, int *err) {
  if (lines.size() <= 1) return {}; 
  Map res;
  for (std::size_t i = 1; i < lines.size(); i++) {
    if (lines[i].find(":") != std::string::npos) {
      std::vector<std::string> header_components = Srv::Utils::split(lines[i], ':');
      const std::size_t components_size = header_components.size();
      if (components_size < 2) {
        if (err != nullptr) *err = 1;
        return {};
      }
      if (components_size > 2) {
        for (std::size_t j = 2; j < components_size; j++) {
          header_components[1] += ":" + header_components[j];
        }
      }
      std::string header_value = Srv::Utils::ltrim(header_components[1]);
      const std::size_t length = header_value.length();
      if (length > 0 && header_value[length - 1] == '\r') {
        header_value.pop_back();
      }
      res.map[Srv::Utils::to_lower(header_components[0])] = header_value;
    }
    if (lines[i] == "\r") break;
  }
  return res;
}

Map Http::parse_payload(const std::string &str) {
  Map res;
  const std::vector<std::string> &pairs = Srv::Utils::split(str, '&');
  for (auto &pair : pairs) {
    const std::vector<std::string> pair_components = Srv::Utils::split(pair, '=');
    if (pair_components.size() != 2) continue; // TODO: return 400 or something
    std::string value = pair_components[1];
    std::replace(value.begin(), value.end(), '+', ' ');
    res.map[pair_components[0]] = Uri::decode_component(value);
  }
  return res;
}

std::string Http::read_body(const std::string &str, const std::size_t n) {
  const std::string headers_end = HEADERS_END;
  std::size_t pos = str.find(headers_end);
  if (pos == std::string::npos) return "";
  const std::size_t idx = pos + headers_end.length();
  if (idx + n > str.length()) return "";
  if (n != 0) {
    return str.substr(idx, n);
  }
  return str.substr(idx);
}