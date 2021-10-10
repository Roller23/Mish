#if !defined(__INTERPRETER_)
#define __INTERPRETER_

#include "CVM.hpp"
#include "evaluator.hpp"

#include <string>

class Interpreter {
  private:
    CVM VM;
    Utils utils;
    Evaluator *evaluator = nullptr;
  public:
    std::string process_string(const std::string &code);
};

#endif // __INTERPRETER_