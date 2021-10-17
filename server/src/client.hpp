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
    QueryMap params;
    bool has(const std::string &key) const;
    std::string get(const std::string &key) const;
};

class Request {
  public:
    Query query;
    HeadersMap headers;
};

class Response {
  friend class Worker;
  private:
    std::string buffer = "";
    HeadersMap headers;
  public:
    int script_code = Status::OK;
    std::string output = "";
    void append(const std::string &str);
    void add_header(const std::string &key, const std::string &value);
    const std::string &get_header(const std::string &key);
    void end(const int code = Status::OK, const std::string &str = "");
    Response() {
      headers["Content-Type"] = "text/html; charset=utf-8";
      headers["Content-Length"] = "0";
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