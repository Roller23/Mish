#if !defined(__INTERPRETER_)
#define __INTERPRETER_

#include "CVM.hpp"
#include "evaluator.hpp"

#include <string>

class Interpreter {
  private:
    Utils utils;
    Evaluator *evaluator = nullptr;
    const std::string &source;
  public:
    CVM VM;
    void process_string(const std::string &code);
    Interpreter(const std::string &_source) : source(_source) {}
};

#endif // __INTERPRETER_