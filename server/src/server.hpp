#if !defined(__SERVER_)
#define __SERVER_

#include <mutex>

#include "../../ckript/src/interpreter.hpp"

class Server {
  private:
    std::mutex file_mutex;

  public:
    std::string process_code(const std::string &full_path, const std::string &relative_path);
    void serve_http(const int port);
};

#endif // __SERVER_