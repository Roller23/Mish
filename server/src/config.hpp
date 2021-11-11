#if !defined (__CONFIG_)
#define __CONFIG_

#include <thread>
#include <string>

class Config {
  friend class Server;
  friend class Worker;
  private:
    int port = 8080;
    int max_connections = 1000;
    int max_body_size = 1024 * 1024 * 8; // 8MB
    int max_headers_size = 1024 * 16; // 16KB
    bool global_cors_enabled = false;
    unsigned int max_threads = std::thread::hardware_concurrency();
    std::string config_file = "server.config";
  public:
    void load_option(const std::string &option, const std::string &value = "");
};

#endif // __CONFIG_