#if !defined(__PATH_)
#define __PATH_

#include <filesystem>
#include <string>

class Path {
  public:
    static bool safe(const std::filesystem::path &path);
    static bool resource_exists(const std::string &path);
    static bool is_index_directory(const std::filesystem::path &path);
};

#endif // __PATH_