#include "date.hpp"

#include <iomanip>
#include <ctime>

#include <string>
#include <sstream>

std::string Date::format(const std::string &fmt) {
  std::time_t t = std::time(nullptr);
  std::tm tm = *std::localtime(&t);
  const auto &time = std::put_time(&tm, fmt.c_str());
  std::stringstream ss;
  ss << time;
  return ss.str();
}