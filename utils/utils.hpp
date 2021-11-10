#if !defined(__UTILS_SRV_)
#define __UTILS_SRV_

#include <string>
#include <vector>

namespace Srv {
  class Utils {
    public:
      static std::vector<std::string> split(const std::string &str, char delim);
      static std::string ltrim(std::string str, const char *whitespace = " \t");
      static bool vector_contains(const std::vector<std::string> &v, const std::string &n);
  };
};

#endif // __UTILS_SRV_