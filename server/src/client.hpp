#if !defined(__CLIENT_)
#define __CLIENT_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>
#include <unordered_map>

#include "status.hpp"

#define HTTP "HTTP/1.0"
#define HEADERS_END "\r\n\r\n"

typedef std::unordered_map<std::string, std::string> HeadersMap;
typedef std::unordered_map<std::string, std::string> QueryMap;

class Query {
  public:
    QueryMap query;
    bool has(const std::string &key) const;
    std::string get(const std::string &key) const;
};

class Request {
  public:
    Query query;
};

class Response {
  friend class Server;
  private:
    std::string buffer = "";
    HeadersMap headers;
  public:
    std::string output = "";
    void append(const std::string &str);
    void add_header(const std::string &key, const std::string &value);
    void end(const int code = Status::OK, const std::string &str = "");
    Response() {
      headers["Content-Type"] = "text/html; charset=utf-8";
      headers["Content-Length"] = "0";
    }
};

class Client {
  friend class Server;
  protected:
    char *ip_addr;
    int socket_fd;
    sockaddr_in info;
    socklen_t info_len;
    Request req;
    Response res;
  private:
    void flush(void) const;
  public:
    void end(const int code = Status::OK, const std::string &str = "");
};

#endif // __CLIENT_