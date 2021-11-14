#if !defined(__WORKER_)
#define __WORKER_

#include <unistd.h>
#include <poll.h>

#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <filesystem>

#include "client.hpp"
#include "config.hpp"

class Worker {
  friend class Server;
  private:
    std::mutex &file_mutex;
    std::mutex &stdout_mutex;
    std::unordered_map<int, Client> clients;
    Client *pending_client = nullptr;
    pollfd *pfds = new pollfd[PFDS_SIZE];
    int clients_polled = 0;
    std::thread *thread;
    static const int PFDS_SIZE = 1000;
    static const int poll_timeout = -1;
    static const int TEMP_BUFFER_SIZE = 1024 * 8; // 8KB
    static const char PIPE_PAYLOAD = 23;
    static const int PIPE_READ = 0;
    static const int PIPE_WRITE = 1;
    int *server_pipe;
    static const std::vector<std::string> valid_methods;
    static const std::vector<std::string> methods_containing_bodies;
    static const std::string allowed_methods;
    static const std::string ckript_abort_message;
    char temp_buffer[TEMP_BUFFER_SIZE];
    const std::string current_path = std::filesystem::current_path();
    const Config &config;
    
    void read_pipe(void) const;
    int handle_client(Client &client);
    int read_client(Client &client);
    void remove_client(Client &client, pollfd &pfd);
    void manage_clients(void);
    static inline bool can_read_fd(const pollfd &pfd);
    static inline bool fd_hung_up(const pollfd &pfd);
    static inline bool can_write_fd(const pollfd &pfd);
    void report_back(void) const;
    std::string process_code(const std::string &full_path, const std::string &relative_path, Client &client);
  public:
    int _pipe[2];
    bool add_client(Client &client);
    void start_thread(void);
    Worker(std::mutex &file_mut, std::mutex &stdout_mut, const Config &cfg) :
      file_mutex(file_mut),
      stdout_mutex(stdout_mut),
      config(cfg) {
        if (pipe(_pipe) < 0) {
          std::cout << "Coudln't create worker pipe\n";
          exit(EXIT_FAILURE);
        }
        clients.reserve(PFDS_SIZE);
        for (int i = 0; i < PFDS_SIZE; i++) {
          pfds[i].fd = -1;
          pfds[i].events = POLLIN | POLLHUP | POLLOUT;
          pfds[i].revents = 0;
        }
        // listen to writes to pipe on first poll fd
        pfds[0].fd = _pipe[PIPE_READ];
        pfds[0].events = POLLIN;
      }
};

#endif // __WORKER_