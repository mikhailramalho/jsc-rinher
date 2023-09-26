#include <cmath>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include <memory>
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

struct Node {
  Kind kind;
  Node(Kind k) : kind(k) {}
  virtual ~Node() = default;
};

typedef std::unique_ptr<Node> Term;

struct File {
  const std::string name;
  Term term;
  File(const std::string &_name, Term t) : name(_name), term(std::move(t)) {}
  int dumpToFile(const std::string &filename);
};

struct Int : public Node {
  int32_t value;
  Int(int32_t value) : Node(IntKind), value(value) {}
  ~Int() override = default;
};

struct Str : public Node {
  const std::string value;
  Str(const std::string &value) : Node(StrKind), value(value) {}
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
      : Node(CallKind), callee(std::move(callee)), arguments(std::move(arguments)) {}
  ~Call() override = default;
};

struct Binary : public Node {
  Term lhs;
  BinaryOp op;
  Term rhs;
  Binary(Term lhs, BinaryOp op, Term rhs)
      : Node(BinaryKind), lhs(std::move(lhs)), op(std::move(op)),
        rhs(std::move(rhs)) {}
  ~Binary() override = default;
};

struct Tuple : public Node {
  Term first;
  Term second;
  Tuple(Term first, Term second)
      : Node(TupleKind), first(std::move(first)), second(std::move(second)) {}
  ~Tuple() override = default;
};

struct Var : public Node {
  const std::string text;
  Var(const std::string &text) : Node(VarKind), text(text) {}
  ~Var() override = default;
};

typedef std::string Parameter;

struct Function : public Node {
  std::vector<Parameter> parameters;
  Term value;
  Function(const std::vector<Parameter> &parameters, Term value)
      : Node(FunctionKind), parameters(std::move(parameters)),
        value(std::move(value)) {}
  ~Function() override = default;
};

struct Let : public Node {
  Parameter name;
  Term value;
  Term next;
  Let(Parameter name, Term value, Term next)
      : Node(LetKind), name(name), value(std::move(value)),
        next(std::move(next)){};
  ~Let() override = default;
};

struct If : public Node {
  Term condition, then, otherwise;
  If(Term condition, Term then, Term otherwise)
      : Node(IfKind), condition(std::move(condition)), then(std::move(then)),
        otherwise(std::move(otherwise)) {}
  ~If() override = default;
};

struct Print : public Node {
  Term value;
  Print(Term value) : Node(PrintKind), value(std::move(value)) {}
  ~Print() override = default;
};

struct First : public Node {
  Term value;
  First(Term value) : Node(FirstKind), value(std::move(value)) {}
  ~First() override = default;
};

struct Second : public Node {
  Term value;
  Second(Term value) : Node(SecondKind), value(std::move(value)) {}
  ~Second() override = default;
};

std::unique_ptr<Ast::File> createNodeFromJson(const Json::Value &json);

}; // namespace Ast
