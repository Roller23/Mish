#if !defined(__SERVER_)
#define __SERVER_

#include <mutex>
#include <thread>
#include <vector>
#include <filesystem>

#include "client.hpp"
#include "worker.hpp"
#include "map.hpp"

class Server {
  private:
    unsigned int max_threads = std::thread::hardware_concurrency();
    int max_connections = 1000;
    int max_body_size = 1024 * 1024 * 8; // 8MB

    Map mime_types;
    std::mutex file_mutex;
    std::mutex stdout_mutex;
    std::vector<Worker> threadpool;
    int socket_fd;
    void create_server_socket(const int port);
    void accept_connections();
    void serve(const int port);
    void generate_threadpool(void);
    Server(void) {
      generate_threadpool();
    }
  public:
    Worker &get_optimal_worker(void);
    static void serve_http(const int port) {
      return get().serve(port);
    }
    Server(const Server &) = delete;
    static Server &get() {
      static Server server;
      return server;
    }
};

#endif // __SERVER_