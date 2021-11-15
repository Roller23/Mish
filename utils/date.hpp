#if !defined(__DATE_)
#define __DATE_

#include <string>
#include <ctime>

class Date {
  public:
    static std::string format(const std::string &fmt, std::time_t t = std::time(nullptr));
};

#endif // __DATE_