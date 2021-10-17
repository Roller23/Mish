#if !defined(__MIME_)
#define __MIME_

#include <unordered_map>
#include <string>

class Mime {
  private:
    static const std::unordered_map<std::string, std::string> mime_types;
  public:
    static const std::string &ext_to_mime(const std::string &ext);
};

#endif // __MIME_