#if !defined(__DATE_)
#define __DATE_

#include <string>

class Date {
  public:
    static std::string format(const std::string &fmt);
};

#endif // __DATE_