#include "ast.h"
#include "term.h"
#include "utils.h"

#include <bits/fs_fwd.h>
#include <exception>
#include <iostream>
#include <json/value.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

std::unordered_map<std::string, Ast::Kind> termLookupTable = {
    {"Int", Ast::IntKind},
    {"Str", Ast::StrKind},
    {"Call", Ast::CallKind},
    {"Binary", Ast::BinaryKind},
    {"Function", Ast::FunctionKind},
    {"Let", Ast::LetKind},
    {"If", Ast::IfKind},
    {"Print", Ast::PrintKind},
    {"First", Ast::FirstKind},
    {"Second", Ast::SecondKind},
    {"Bool", Ast::BoolKind},
    {"Tuple", Ast::TupleKind},
    {"Var", Ast::VarKind}};

std::unordered_map<std::string, Ast::BinaryOp> binaryOpLookupTable = {
    {"Add", Ast::Add}, {"Sub", Ast::Sub}, {"Mul", Ast::Mul}, {"Div", Ast::Div},
    {"Rem", Ast::Rem}, {"Eq", Ast::Eq},   {"Neq", Ast::Neq}, {"Lt", Ast::Lt},
    {"Gt", Ast::Gt},   {"Lte", Ast::Lte}, {"Gte", Ast::Gte}, {"And", Ast::And},
    {"Or", Ast::Or}};

Ast::BinaryOp createBinaryOpFromJson(const Json::Value &json) {
  return binaryOpLookupTable[json.asString()];
}

Ast::Parameter createParameterFromJson(const Json::Value &json) {
  if (!json.isMember("text"))
    ABORT("Parameter illformed");
  return {json["text"].asString()};
}

std::shared_ptr<Ast::Node> createTermFromJson(const Json::Value &json) {
  if (!json.isMember("kind") || !json.isMember("location"))
    ABORT("Term Illformed");
  std::string kind = json["kind"].asString();
  if (termLookupTable.count(kind) == 0)
    ABORT("Term kind not recognized");

  Ast::Kind termKind = termLookupTable.find(kind)->second;

  std::shared_ptr<Ast::Node> node;
  // Auxiliar vector to get argument list
  std::vector<std::shared_ptr<Ast::Node>> argVec;
  // Auxiliar vector to get Parameter list
  std::vector<Ast::Parameter> paramVec;
  bool error = false;

  switch (termKind) {
  case Ast::IntKind:
    error = !json.isMember("value");
    if (error)
      break;
    node = std::make_shared<Ast::Int>(json["value"].asInt());
    break;

  case Ast::StrKind:
    error = !json.isMember("value");
    if (error)
      break;
    node = std::make_shared<Ast::Str>(json["value"].asString());
    break;

  case Ast::CallKind:
    error = !json.isMember("arguments");
    error |= !json.isMember("callee");
    if (error)
      break;
    for (const Json::Value &js : json["arguments"]) {
      argVec.push_back(createTermFromJson(js));
    }
    node =
        std::make_shared<Ast::Call>(createTermFromJson(json["callee"]), argVec);
    break;

  case Ast::BinaryKind:
    error = !json.isMember("lhs");
    error |= !json.isMember("op");
    error |= !json.isMember("rhs");
    if (error)
      break;
    node = std::make_shared<Ast::Binary>(createTermFromJson(json["lhs"]),
                                         createBinaryOpFromJson(json["op"]),
                                         createTermFromJson(json["rhs"]));
    break;

  case Ast::FunctionKind:
    error = !json.isMember("parameters");
    error |= !json.isMember("value");
    error |= !json.isMember("location");
    if (error)
      break;
    for (Json::Value js : json["parameters"]) {
      paramVec.push_back(createParameterFromJson(js));
    }
    node = std::make_shared<Ast::Function>(paramVec,
                                           createTermFromJson(json["value"]));
    break;

  case Ast::LetKind:
    error = !json.isMember("name");
    error |= !json.isMember("value");
    error |= !json.isMember("next");
    if (error)
      break;
    node = std::make_shared<Ast::Let>(createParameterFromJson(json["name"]),
                                      createTermFromJson(json["value"]),
                                      createTermFromJson(json["next"]));
    break;

  case Ast::IfKind:
    error = !json.isMember("condition");
    error |= !json.isMember("then");
    error |= !json.isMember("otherwise");
    if (error)
      break;
    node = std::make_shared<Ast::If>(createTermFromJson(json["condition"]),
                                     createTermFromJson(json["then"]),
                                     createTermFromJson(json["otherwise"]));
    break;

  case Ast::PrintKind:
    error = !json.isMember("value");
    if (error)
      break;
    node = std::make_shared<Ast::Print>(createTermFromJson(json["value"]));
    break;

  case Ast::FirstKind:
    error = !json.isMember("value");
    if (error)
      break;
    node = std::make_shared<Ast::First>(createTermFromJson(json["value"]));
    break;

  case Ast::SecondKind:
    error = !json.isMember("value");
    if (error)
      break;
    node = std::make_shared<Ast::Second>(createTermFromJson(json["value"]));
    break;

  case Ast::BoolKind:
    error = !json.isMember("value");
    if (error)
      break;
    node = std::make_shared<Ast::Bool>(json["value"].asBool());
    break;

  case Ast::TupleKind:
    error = !json.isMember("first");
    error |= !json.isMember("second");
    if (error)
      break;
    node = std::make_shared<Ast::Tuple>(createTermFromJson(json["first"]),
                                        createTermFromJson(json["second"]));
    break;

  case Ast::VarKind:
    error = !json.isMember("text");
    if (error)
      break;
    node = std::make_shared<Ast::Var>(json["text"].asString());
    break;

  default:
    ABORT("Term not found");
  }

  if (error)
    ABORT("Term illformed on " + json["location"]["file"].asString() + "(" +
          std::to_string(json["location"]["start"].asInt()) + ", " +
          std::to_string(json["location"]["end"].asInt()) + ")");
  return node;
}

std::shared_ptr<Ast::Node> createNodeFromJson(const Json::Value &json) {
  if (!(json.isMember("name") && json.isMember("expression") &&
        json.isMember("location")))
    ABORT("Node illformed");
  return std::make_shared<Ast::File>(json["name"].asString(),
                                     createTermFromJson(json["expression"]));
}

Ast::Ast Ast::Ast::createFromJson(const Json::Value &json) {
  return Ast(createNodeFromJson(json));
}
