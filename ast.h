#include <cmath>
#include <json/json.h>
#include <json/value.h>
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
  VarKind,
  NoneKind
};

enum BinaryOp { Add, Sub, Mul, Div, Rem, Eq, Neq, Lt, Gt, Lte, Gte, And, Or };

struct Parameter {
  std::string text;
};

enum NodeType { FileType, TermType };

struct NodeSpecification {
  NodeType type;
  Kind kind;
};

struct Node {
  NodeSpecification specification;
  Node(NodeSpecification specification) : specification(specification) {}
  virtual ~Node() = default;
};

struct File : public Node {
  std::string name;
  std::shared_ptr<Node> expression;
  File(std::string name, std::shared_ptr<Node> expression)
      : name(name), expression(expression), Node({FileType, NoneKind}) {}
  virtual ~File() = default;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
};

class Ast {
private:
  std::shared_ptr<Node> head;
  Ast(std::shared_ptr<Node> head) : head(head) {}

public:
  static Ast createFromJson(const Json::Value &json);
};

}; // namespace Ast
