#include <cmath>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

#pragma once

namespace Ast {

enum Kind {
  IntKind,
  StrKind,
  CallKind,
  BinaryKind,
  FunctionKind,
  LetKind,
  IfKind,
  PrintKind,
  FirstKind,
  SecondKind,
  BoolKind,
  TupleKind,
  VarKind
};

enum BinaryOp { Add, Sub, Mul, Div, Rem, Eq, Neq, Lt, Gt, Lte, Gte, And, Or };

struct Parameter {
  std::string text;
};

struct Node {
  Kind kind;
  Node(Kind k) : kind(k) {}
  virtual ~Node() = default;
};

typedef std::shared_ptr<Node> Term;

struct File {
  std::string name;
  Term term;
  File(const std::string &_name, Term t) : name(_name), term(t) {}
  int dumpToFile(std::string filename);
};

struct Int : public Node {
  int32_t value;
  Int(int32_t value) : Node(IntKind), value(value) {}
  ~Int() override = default;
};

struct Str : public Node {
  std::string value;
  Str(std::string value) : Node(StrKind), value(value) {}
  ~Str() override = default;
};

struct Bool : public Node {
  bool value;
  Bool(bool value) : Node(BoolKind), value(value) {}
  ~Bool() override = default;
};

struct Call : public Node {
  Term callee;
  std::vector<Term> arguments;
  Call(Term callee, std::vector<Term> arguments)
      : Node(CallKind), callee(callee), arguments(arguments) {}
  ~Call() override = default;
};

struct Binary : public Node {
  Term lhs;
  BinaryOp op;
  Term rhs;
  Binary(Term lhs, BinaryOp op, Term rhs)
      : Node(BinaryKind), lhs(lhs), op(op), rhs(rhs) {}
  ~Binary() override = default;
};

struct Tuple : public Node {
  Term first;
  Term second;
  Tuple(Term first, Term second)
      : Node(TupleKind), first(first), second(second) {}
  ~Tuple() override = default;
};

struct Var : public Node {
  std::string text;
  Var(std::string text) : Node(VarKind), text(text) {}
  ~Var() override = default;
};

struct Function : public Node {
  std::vector<Parameter> parameters;
  Term value;
  Function(std::vector<Parameter> parameters, Term value)
      : Node(FunctionKind), parameters(parameters), value(value) {}
  ~Function() override = default;
};

struct Let : public Node {
  Parameter name;
  Term value;
  Term next;
  Let(Parameter name, Term value, Term next)
      : Node(LetKind), name(name), value(value), next(next){};
  ~Let() override = default;
};

struct If : public Node {
  Term condition, then, otherwise;
  If(Term condition, Term then, Term otherwise)
      : Node(IfKind), condition(condition), then(then), otherwise(otherwise) {}
  ~If() override = default;
};

struct Print : public Node {
  Term value;
  Print(Term value) : Node(PrintKind), value(value) {}
  ~Print() override = default;
};

struct First : public Node {
  Term value;
  First(Term value) : Node(FirstKind), value(value) {}
  ~First() override = default;
};

struct Second : public Node {
  Term value;
  Second(Term value) : Node(SecondKind), value(value) {}
  ~Second() override = default;
};

std::shared_ptr<Ast::File> createNodeFromJson(const Json::Value &json);

}; // namespace Ast
