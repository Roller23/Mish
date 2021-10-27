#if !defined(__INTERPRETER_)
#define __INTERPRETER_

#include <string>
#include <filesystem>
#include <mutex>

#include "CVM.hpp"
#include "evaluator.hpp"

#include "../../server/src/client.hpp"

class Interpreter {
  private:
    Utils utils;
    Evaluator *evaluator = nullptr;
    const std::string &source;
    std::filesystem::path source_path;
    std::filesystem::path source_dir;
    Client &client;
  public:
    CVM VM;
    void destroy();
    void process_string(const std::string &code);
    Interpreter(const std::string &_source, std::mutex &file_mut, std::mutex &stdout_mut, Client &_client) :
      source(_source),
      source_path(std::filesystem::path(_source).lexically_normal()),
      source_dir(source_path.parent_path()),
      client(_client),
      VM(source_path, file_mut, stdout_mut, _client) {}
};

#endif // __INTERPRETER_