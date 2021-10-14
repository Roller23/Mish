#if !defined(__INTERPRETER_)
#define __INTERPRETER_

#include "CVM.hpp"
#include "evaluator.hpp"

#include <string>
#include <filesystem>
#include <mutex>

class Interpreter {
  private:
    Utils utils;
    Evaluator *evaluator = nullptr;
    const std::string &source;
    std::filesystem::path source_path;
    std::filesystem::path source_dir;
  public:
    CVM VM;
    void process_string(const std::string &code);
    Interpreter(const std::string &_source, std::mutex &file_mut, std::mutex &stdout_mut) :
      source(_source),
      source_path(std::filesystem::path(_source).lexically_normal()),
      source_dir(source_path.parent_path()),
      VM(source_path, file_mut, stdout_mut) {}
};

#endif // __INTERPRETER_