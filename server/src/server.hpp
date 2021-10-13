#if !defined(__SERVER_)
#define __SERVER_

#include <mutex>

#include "../../ckript/src/interpreter.hpp"

class Server {
  private:
    std::mutex file_mutex;

  public:
    void serve_http(const int port);
};

#endif // __SERVER_