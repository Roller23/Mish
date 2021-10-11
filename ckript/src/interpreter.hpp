#if !defined(__INTERPRETER_)
#define __INTERPRETER_

#include "CVM.hpp"
#include "evaluator.hpp"

#include <string>

class Interpreter {
  private:
    Utils utils;
    Evaluator *evaluator = nullptr;
  public:
    CVM VM;
    void process_string(const std::string &code);
};

#endif // __INTERPRETER_