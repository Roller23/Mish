#if !defined(__INTERPRETER_)
#define __INTERPRETER_

#include "CVM.hpp"
#include "evaluator.hpp"

#include <string>
#include <filesystem>

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
    Interpreter(const std::string &_source) :
      source(_source),
      source_path(std::filesystem::path(_source).lexically_normal()),
      source_dir(source_path.parent_path()),
      VM(source_path) {} // might be dangerous
};

#endif // __INTERPRETER_