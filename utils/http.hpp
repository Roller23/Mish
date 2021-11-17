#if !defined(__HTTP_)
#define __HTTP_

#include <vector>
#include <string>

#include "../server/src/map.hpp"

class Http {
  public:
    static Map read_headers(const std::vector<std::string> &lines, int *err = nullptr);
    static Map parse_payload(const std::string &str);
    static std::string read_body(const std::string &str, const std::size_t n = 0);
};

#endif // __HTTP_