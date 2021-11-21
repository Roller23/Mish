#if !defined(__CVM_)
#define __CVM_

#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>
#include <memory>
#include <filesystem>
#include <algorithm>
#include <mutex>

#include "utils.hpp"
#include "AST.hpp"

#include "../../server/src/client.hpp"

class Value {
  public:
    Utils::VarType type = Utils::UNKNOWN;
    bool boolean_value = false;
    double float_value = 0.0;
    std::string string_value = "";
    std::int64_t number_value = 0;
    std::string reference_name = "";
    std::int64_t heap_reference = -1;
    std::int64_t this_ref = -1;
    FuncExpression func;
    std::string func_name = "";
    ParamList members;
    std::map<std::string, Value> member_values;
    std::vector<Value> array_values;
    std::string array_type = "int";
    std::string class_name = "";
    std::string member_name = "";
    bool is_lvalue() const;
    Value(void) : type(Utils::UNKNOWN) {};
    Value(Utils::VarType _type) : type(_type) {};
    Value(const FuncExpression &fn) : type(Utils::FUNC), func(fn) {}
};

class Variable {
  public:
    std::string type;
    Value val;
    bool constant = false;
    bool is_allocated() const;
};

class Chunk {
  public:
    Value *data = nullptr;
    std::int64_t heap_reference = -1;
    bool used = false;
    bool marked = false;
};

class Cache {
  private:
    static const int CACHE_SIZE = 10000;
    int index = -1;
  public:
    std::vector<std::int64_t> refs;
    void push(std::int64_t ref);
    std::int64_t pop(void);
    Cache(void) {
      refs.resize(CACHE_SIZE);
    }
};

class Heap {
  public:
    std::vector<Chunk> chunks;
    Cache cache;
    Chunk &allocate();
    void free(std::int64_t ref);
    void destroy(void) const;
};

typedef std::unordered_map<std::string, std::shared_ptr<Variable>> CallStack;

class Call {
  public:
    std::uint64_t line;
    std::string name;
    std::string source = "";
    Call(const std::uint64_t &__l, const std::string &__n, const std::string &__s) {
      this->line = __l;
      this->name = __n;
      this->source = __s;
    }
};

class StackTrace {
  public:
    std::vector<Call> stack;
    void pop(void);
    void push(const std::string &_name, const std::uint64_t &_line, const std::string &_source);
    StackTrace(void) {
      stack.reserve(1000);
    }
};

class NativeFunction;
class Evaluator;

class CVM {
  private:
    void mark_chunk(Chunk &chunk);
    void mark_all(void);
    std::size_t sweep(void);
    std::size_t run_gc(void);

    void load_stdlib(void);
    std::size_t allocated_chunks = 0;
    std::size_t chunks_limit = 5;
    const std::size_t limit_scale_factor = 2;
  public:
    std::string actual_path(const std::string &filename) const;
    std::string stringify(Value &val) const;
    std::unordered_map<std::string, NativeFunction *> globals;
    Heap heap;
    StackTrace trace;
    std::string output_buffer = "";
    std::string error_buffer = "";
    std::string abort_message = "";
    const std::filesystem::path &source_path;
    std::mutex &file_mutex;
    std::mutex &stdout_mutex;
    Client &client;
    std::vector<Evaluator *> active_evaluators;
    void destroy_globals(void);
    Chunk &allocate(void);
    void check_chunks(void);
    void throw_syntax_error(const std::string &cause, std::uint32_t line = 0);
    void throw_runtime_error(const std::string &cause, std::uint32_t line = 0);
    void throw_file_error(const std::string &cause);
    void throw_generic_error(const std::string &cause, std::uint32_t line = 0);
    CVM(const std::filesystem::path &_source_path, std::mutex &file_mut, std::mutex &stdout_mut, Client &_client) :
      source_path(_source_path),
      file_mutex(file_mut),
      stdout_mutex(stdout_mut),
      client(_client) {
        load_stdlib();
    }
};

class NativeFunction {
  public:
    virtual Value execute(std::vector<Value> &args, std::int64_t line, CVM &VM) = 0;
    virtual ~NativeFunction() = default;
};

#endif // __CVM_