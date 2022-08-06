#if !defined(__SERVER_)
#define __SERVER_

#include <cstdlib>

#include <mutex>
#include <thread>
#include <vector>
#include <filesystem>
#include <unordered_map>

#include "client.hpp"
#include "worker.hpp"
#include "map.hpp"
#include "config.hpp"

class Server {
  private:
    Config config;
    Map mime_types;
    std::mutex file_mutex;
    std::mutex stdout_mutex;
    std::vector<Worker> threadpool;
    int _pipe[2];
    int socket_fd;
    void create_server_socket(const int port);
    void accept_connections();
    void serve(const int port);
    Server(void) {
      if (pipe(_pipe) < 0) {
        std::cout << "Coudln't create server pipe\n";
        exit(EXIT_FAILURE);
      }
    }
  public:
    void generate_threadpool(void);
    void load_config_args(int argc, char *argv[]);
    void load_config_file(void);
    Worker &get_optimal_worker(int *err = nullptr);
    static void load_session(const std::string &id);
    static void destroy_session(const std::string &id);
    static bool check_session_id(const std::string &id);
    static bool session_has(const std::string &id, const std::string &key);
    static std::string session_get(const std::string &id, const std::string &key);
    static void session_set(const std::string &id, const std::string &key, const std::string &value);
    static void serve_http() {
      Server &srv = get();
      return srv.serve(srv.config.port);
    }
    Server(const Server &) = delete;
    static Server &get() {
      static Server server;
      return server;
    }
};

#endif // __SERVER_