#if !defined (__CONFIG_)
#define __CONFIG_

#include <thread>

class Config {
  public:
    int max_connections = 1000;
    int max_body_size = 1024 * 1024 * 8; // 8MB
    int max_headers_size = 1024 * 16; // 16KB
    unsigned int max_threads = std::thread::hardware_concurrency();
};

#endif // __CONFIG_