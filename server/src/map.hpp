#if !defined(__MAP_)
#define __MAP_

#include <unordered_map>
#include <string>

typedef std::unordered_map<std::string, std::string> StrMap;

class Map {
  public:
    StrMap map;
    bool has(const std::string &key) const;
    std::string get(const std::string &key) const;
};

#endif // __MAP_