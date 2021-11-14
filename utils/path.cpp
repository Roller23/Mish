#include "path.hpp"

#include <algorithm>
#include <filesystem>
#include <string>

bool Path::safe(const std::filesystem::path &path) {
  const std::filesystem::path &curr = std::filesystem::current_path();
  return std::search(path.begin(), path.end(), curr.begin(), curr.end()) != path.end();
}

bool Path::resource_exists(const std::string &path) {
  if (!std::filesystem::exists(path)) return false;
  if (std::filesystem::is_directory(path)) return false;
  return true;
}

bool Path::is_index_directory(const std::filesystem::path &path) {
  if (!std::filesystem::exists(path)) return false;
  if (!std::filesystem::is_directory(path)) return false;
  return std::filesystem::exists(path / "index.ck");
}