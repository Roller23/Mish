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
    void process_string(const std::string &code, int argc, char *argv[]);
};

#endif // __INTERPRETER_