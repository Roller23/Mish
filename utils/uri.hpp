#if !defined(__URI_)
#define __URI_

#include <string>

class Uri {
  public:
    static std::string decode_component(const std::string &str);
};

#endif // __URI_