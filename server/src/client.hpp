#if !defined(__CLIENT_)
#define __CLIENT_

#include <sys/socket.h>
#include <arpa/inet.h>

class Client {
  friend class Server;
  protected:
    char *ip_addr;
    int socket_fd;
    sockaddr_in info;
    socklen_t info_len;
  public:

};

#endif // __CLIENT_