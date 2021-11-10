#if !defined(__WORKER_)
#define __WORKER_

#include <unistd.h>
#include <poll.h>

#include <thread>
#include <iostream>
#include <queue>
#include <vector>

#include "client.hpp"

class Worker {
  friend class Server;
  private:
    std::mutex &file_mutex;
    std::mutex &stdout_mutex;
    std::queue<Client> client_queue;
    std::vector<pollfd> pfds;
    const int poll_timeout = -1;
    std::thread *thread;
    static const int TEMP_BUFFER_SIZE = 1024 * 8; // 8KB
    static const char PIPE_PAYLOAD = 23;
    static const int PIPE_READ = 0;
    static const int PIPE_WRITE = 1;
    static const std::vector<std::string> valid_methods;
    static const std::vector<std::string> methods_containing_bodies;
    char temp_buffer[TEMP_BUFFER_SIZE];
    const std::string current_path = std::filesystem::current_path();
    const std::string ckript_abort_message = "ckript abort()";
    int max_body_size = 1024 * 1024 * 8; // 8MB
    int max_headers_size = 1024 * 16; // 16KB
    
    void read_pipe(void) const;
    void handle_client(Client &client);
    void read_client(Client &client);
    void manage_clients(void);
    static inline bool can_read_fd(const pollfd &pfd);
    std::string process_code(const std::string &full_path, const std::string &relative_path, Client &client);
  public:
    int _pipe[2];
    void add_client(Client &client);
    void start_thread(void);
    Worker(std::mutex &file_mut, std::mutex &stdout_mut) :
      file_mutex(file_mut),
      stdout_mutex(stdout_mut) {
        if (pipe(_pipe) < 0) {
          std::cout << "Coudln't create worker pipe\n";
          exit(EXIT_FAILURE);
        }
        pollfd pfd;
        pfd.fd = _pipe[PIPE_READ];
        pfd.events = POLLIN;
        pfd.revents = 0;
        pfds.push_back(pfd);
      }
};

#endif // __WORKER_