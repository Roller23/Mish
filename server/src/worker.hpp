#if !defined(__WORKER_)
#define __WORKER_

#include <mutex>
#include <condition_variable>
#include <queue>
#include <iostream>

#include "client.hpp"
#include "server.hpp"

class Worker {
  public:
    void process(void) {
      std::cout << "worker started\n";
      std::unique_lock<std::mutex> lock(mutex);
      while (true) {
        cond_var.wait(lock);
        while (client_queue.size() != 0) {
          Client &client = client_queue.front();
          client_queue.pop();
          server->handle_client(client);
        }
      }
    }
  private:
    std::mutex mutex;
    std::condition_variable cond_var;
    std::queue<Client> client_queue;
    Server *server = nullptr;
    std::thread thread;
  public:
    void add_client(Client &client) {
      client_queue.push(client);
    }
    Worker(Server *_server) : server(_server) {
      thread = std::thread(&Worker::process, this);
    }
};

#endif // __WORKER_