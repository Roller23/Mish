#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"
#include "evaluator.hpp"

void Interpreter::process_string(const std::string &code) {
  TokenList tokens = Lexer().tokenize(code);
  Parser parser(tokens, Token::TokenType::NONE, "", utils);
  Node AST = parser.parse(NULL);
  if (evaluator == nullptr) {
    evaluator = new Evaluator(AST, VM, utils);
  } else {
    evaluator->AST = AST;
  }
  evaluator->stack.reserve(100);
  evaluator->start();
}