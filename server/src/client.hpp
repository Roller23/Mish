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
    std::string buffer = "";
    std::string raw_body = "";
    std::size_t length = 0;
    std::string method;

    bool has_headers() const;
    bool has_body() const;
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
    void end(const int code = Status::OK, const std::string &str = "", bool ignore_buffer = false);
    Response() {
      headers.map["Server"] = "Mish/1.0";
      headers.map["Content-Type"] = "text/html; charset=utf-8";
      headers.map["Content-Length"] = "0";
      headers.map["Connection"] = "close";
    }
};

class Client {
  friend class Server;
  friend class Worker;
  protected:
    char ip_addr[20];
    int socket_fd;
    sockaddr_in info;
    socklen_t info_len;
    bool request_processed = false;
    bool closed = false;
  private:
    void flush(void);
    void _close(void);
    bool buffer_ready(void) const;
    void enable_cors(void);
  public:
    bool should_enable_cors = false;
    Request req;
    Response res;
    void end(const int code = Status::OK, const std::string &str = "", bool ignore_buffer = false);
    void attempt_close(void);
};

#endif // __CLIENT_