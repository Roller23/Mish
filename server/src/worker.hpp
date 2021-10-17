#if !defined(__WORKER_)
#define __WORKER_

#include <unistd.h>
#include <poll.h>

#include <queue>
#include <thread>
#include <iostream>

#include "client.hpp"

class Worker {
  friend class Server;
  private:
    std::mutex &file_mutex;
    std::mutex &stdout_mutex;
    std::queue<Client> client_queue;
    std::vector<pollfd> fds;
    const int poll_timeout = -1;
    std::thread *thread;
    const std::string current_path = std::filesystem::current_path();

    void handle_client(Client &client);
    void manage_clients(void);
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
        struct pollfd pfd;
        pfd.fd = _pipe[0];
        pfd.events = POLLIN;
        pfd.revents = 0;
        fds.push_back(pfd);
      }
};

#endif // __WORKER_