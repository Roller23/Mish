#if !defined(__UTILS_SRV_)
#define __UTILS_SRV_

#include <string>
#include <vector>

namespace Srv {
  class Utils {
    public:
      static std::vector<std::string> split(const std::string &str, char delim);
      static std::string ltrim(std::string str, const char *whitespace = " \t");
  };
};

#endif // __UTILS_SRV_