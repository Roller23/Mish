#if !defined(__SERVER_)
#define __SERVER_

#include <mutex>
#include <thread>
#include <vector>
#include <filesystem>

#include "../../ckript/src/interpreter.hpp"

class Server {
  private:
    std::mutex file_mutex;
    std::mutex stdout_mutex;
    const unsigned int max_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threadpool;
    int socket_fd;
    const std::string current_path = std::filesystem::current_path();
  public:
    std::string process_code(const std::string &full_path, const std::string &relative_path);
    void accept_connections();
    void handle_client(const int client_fd);
    void serve_http(const int port);
};

#endif // __SERVER_