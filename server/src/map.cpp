#include "map.hpp"

bool Map::has(const std::string &key) const {
  return map.count(key) != 0;
}

std::string Map::get(const std::string &key) const {
  if (!has(key)) return "";
  return map.at(key);
}