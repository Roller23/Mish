#if !defined(__FILE_)
#define __FILE_

#include <string>

class File {
  public:
    static std::string read(const std::string &path);
};

#endif // __FILE_