#pragma once

#include <jsoncpp/json/value.h>
#include <memory>
#include <string>
#include <vector>

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
  explicit Node(Kind k) : kind(k) {}
  virtual ~Node() = default;
} __attribute__((aligned(4)));

using Term = std::unique_ptr<Node>;

struct Int : public Node {
  int32_t value{};
  explicit Int(int32_t value) : Node(IntKind), value(value) {}
  ~Int() override = default;
} __attribute__((aligned(4)));

struct Str : public Node {
  const std::string value{};
  explicit Str(const std::string &&value) : Node(StrKind), value(value) {}
  ~Str() override = default;
} __attribute__((aligned(32)));

struct Bool : public Node {
  bool value;
  explicit Bool(bool value) : Node(BoolKind), value(value) {}
  ~Bool() override = default;
} __attribute__((aligned(1)));

struct Call : public Node {
  Term callee{};
  std::vector<Term> arguments{};
  Call(Term callee, std::vector<Term> arguments)
      : Node(CallKind), callee(std::move(callee)),
        arguments(std::move(arguments)) {}
  ~Call() override = default;
} __attribute__((aligned(32)));

struct Binary : public Node {
  Term lhs{};
  BinaryOp op;
  Term rhs{};
  Binary(Term lhs, BinaryOp op, Term rhs)
      : Node(BinaryKind), lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}
  ~Binary() override = default;
} __attribute__((aligned(32)));

struct Tuple : public Node {
  Term first{};
  Term second{};
  Tuple(Term first, Term second)
      : Node(TupleKind), first(std::move(first)), second(std::move(second)) {}
  ~Tuple() override = default;
} __attribute__((aligned(16)));

struct Var : public Node {
  const std::string text{};
  explicit Var(std::string text) : Node(VarKind), text(std::move(text)) {}
  ~Var() override = default;
} __attribute__((aligned(32)));

using Parameter = std::string;

struct Function : public Node {
  std::vector<Parameter> parameters{};
  Term value{};
  Function(std::vector<Parameter> parameters, Term value)
      : Node(FunctionKind), parameters(std::move(parameters)),
        value(std::move(value)) {}
  ~Function() override = default;
} __attribute__((aligned(32)));

struct Let : public Node {
  const Parameter name{};
  Term value{};
  Term next{};
  Let(const Parameter &&name, Term value, Term next)
      : Node(LetKind), name(name), value(std::move(value)),
        next(std::move(next)){};
  ~Let() override = default;
} __attribute__((aligned(64)));

struct If : public Node {
  Term condition{}, then{}, otherwise{};
  If(Term condition, Term then, Term otherwise)
      : Node(IfKind), condition(std::move(condition)), then(std::move(then)),
        otherwise(std::move(otherwise)) {}
  ~If() override = default;
} __attribute__((aligned(32)));

struct Print : public Node {
  Term value{};
  explicit Print(Term value) : Node(PrintKind), value(std::move(value)) {}
  ~Print() override = default;
};

struct First : public Node {
  Term value{};
  explicit First(Term value) : Node(FirstKind), value(std::move(value)) {}
  ~First() override = default;
};

struct Second : public Node {
  Term value{};
  explicit Second(Term value) : Node(SecondKind), value(std::move(value)) {}
  ~Second() override = default;
};

}; // namespace Ast
