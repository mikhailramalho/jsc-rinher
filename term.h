#include "ast.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#pragma once

namespace Ast {

typedef std::shared_ptr<Node> Term;

struct Int : public Node {
  int32_t value;
  Int(int32_t value) : value(value), Node({TermType, IntKind}) {}
  virtual ~Int() = default;
};

struct Str : public Node {
  std::string value;
  Str(std::string value) : value(value), Node({TermType, StrKind}) {}
  virtual ~Str() = default;
};

struct Bool : public Node {
  bool value;
  Bool(bool value) : value(value), Node({TermType, BoolKind}) {}
  virtual ~Bool() = default;
};

struct Call : public Node {
  Term callee;
  std::vector<Term> arguments;
  Call(Term callee, std::vector<Term> arguments)
      : callee(callee), arguments(arguments), Node({TermType, CallKind}) {}
  virtual ~Call() = default;
};

struct Binary : public Node {
  Term lhs;
  BinaryOp op;
  Term rhs;
  Binary(Term lhs, BinaryOp op, Term rhs)
      : lhs(lhs), op(op), rhs(rhs), Node({TermType, BinaryKind}) {}
  virtual ~Binary() = default;
};

struct Tuple : public Node {
  Term first;
  Term second;
  Tuple(Term first, Term second)
      : first(first), second(second), Node({TermType, TupleKind}) {}
  virtual ~Tuple() = default;
};

struct Var : public Node {
  std::string text;
  Var(std::string text) : text(text), Node({TermType, VarKind}) {}
  virtual ~Var() = default;
};

struct Function : public Node {
  std::vector<Parameter> parameters;
  Term value;
  Function(std::vector<Parameter> parameters, Term value)
      : parameters(parameters), value(value), Node({TermType, FunctionKind}) {}
  virtual ~Function() = default;
};

struct Let : public Node {
  Parameter name;
  Term value;
  Term next;
  Let(Parameter name, Term value, Term next)
      : name(name), value(value), next(next), Node({TermType, LetKind}){};
  virtual ~Let() = default;
};

struct If : public Node {
  Term condition, then, otherwise;
  If(Term condition, Term then, Term otherwise)
      : condition(condition), then(then), otherwise(otherwise),
        Node({TermType, IfKind}) {}
  virtual ~If() = default;
};

struct Print : public Node {
  Term value;
  Print(Term value) : value(value), Node({TermType, PrintKind}) {}
  virtual ~Print() = default;
};

struct First : public Node {
  Term value;
  First(Term value) : Node({TermType, FirstKind}), value(value) {}
  virtual ~First() = default;
};

struct Second : public Node {
  Term value;
  Second(Term value) : Node({TermType, SecondKind}), value(value) {}
  virtual ~Second() = default;
};

}; // namespace Ast
