#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"
#include "evaluator.hpp"

void Interpreter::process_string(const std::string &code) {
  Lexer lexer(VM);
  lexer.set_filename(source);
  TokenList tokens = lexer.tokenize(code);
  Parser parser(tokens, Token::TokenType::NONE, "", utils, VM);
  Node AST = parser.parse(NULL);
  if (evaluator == nullptr) {
    evaluator = new Evaluator(AST, VM, utils);
  } else {
    VM.output_buffer = "";
    evaluator->AST = AST;
  }
  evaluator->stack.reserve(100);
  evaluator->start();
}