#include "http.hpp"
#include "utils.hpp"

#include <vector>
#include <string>
#include <algorithm>

#include "../server/src/map.hpp"
#include "utils.hpp"
#include "uri.hpp"

#define HEADERS_END "\r\n\r\n"

Map Http::read_headers(const std::vector<std::string> &lines) {
  if (lines.size() <= 1) return {}; 
  Map res;
  for (std::size_t i = 1; i < lines.size(); i++) {
    if (lines[i] == "\r") break;
    const std::size_t colon_idx = lines[i].find(":");
    if (colon_idx == std::string::npos) continue;
    const std::string &header_key = lines[i].substr(0, colon_idx);
    std::string header_value = Srv::Utils::ltrim(lines[i].substr(colon_idx + 1));
    const std::size_t length = header_value.length();
    if (length > 0 && header_value[length - 1] == '\r') {
      header_value.pop_back();
    }
    res.map[Srv::Utils::to_lower(header_key)] = header_value;
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