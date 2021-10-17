#if !defined(__CLIENT_)
#define __CLIENT_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>

#include "map.hpp"
#include "status.hpp"

class Request {
  public:
    Map query;
    Map headers;
    Map body;
    std::string raw_body = "";
    std::size_t length = 0;
};

class Response {
  friend class Worker;
  private:
    std::string buffer = "";
    Map headers;
  public:
    int script_code = Status::OK;
    std::string output = "";
    void append(const std::string &str);
    void add_header(const std::string &key, const std::string &value);
    const std::string &get_header(const std::string &key);
    void end(const int code = Status::OK, const std::string &str = "");
    Response() {
      headers.map["Content-Type"] = "text/html; charset=utf-8";
      headers.map["Content-Length"] = "0";
    }
};

class Client {
  friend class Server;
  friend class Worker;
  protected:
    char *ip_addr;
    int socket_fd;
    sockaddr_in info;
    socklen_t info_len;
  private:
    void flush(void) const;
  public:
    Request req;
    Response res;
    void end(const int code = Status::OK, const std::string &str = "");
};

#endif // __CLIENT_