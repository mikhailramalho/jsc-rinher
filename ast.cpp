#include "ast.h"
#include "term.h"
#include "utils.h"

#include <bits/fs_fwd.h>
#include <fstream>
#include <iostream>
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
  has_properties_or_abort(json, "text");
  return {json["text"].asString()};
}

std::shared_ptr<Ast::Node> createTermFromJson(const Json::Value &json) {
  has_properties_or_abort(json, "kind", "location");

  std::string kind = json["kind"].asString();
  if (!termLookupTable.count(kind))
    ABORT("Term kind not recognized");

  Ast::Kind termKind = termLookupTable.find(kind)->second;

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
    return std::make_shared<Ast::Int>(json["value"].asInt());

  case Ast::StrKind:
    error = !json.isMember("value");
    if (error)
      break;
    return std::make_shared<Ast::Str>(json["value"].asString());

  case Ast::CallKind:
    error = !json.isMember("arguments");
    error |= !json.isMember("callee");
    if (error)
      break;
    for (const Json::Value &js : json["arguments"]) {
      argVec.push_back(createTermFromJson(js));
    }
    return std::make_shared<Ast::Call>(createTermFromJson(json["callee"]),
                                       argVec);

  case Ast::BinaryKind:
    error = !json.isMember("lhs");
    error |= !json.isMember("op");
    error |= !json.isMember("rhs");
    if (error)
      break;
    return std::make_shared<Ast::Binary>(createTermFromJson(json["lhs"]),
                                         createBinaryOpFromJson(json["op"]),
                                         createTermFromJson(json["rhs"]));

  case Ast::FunctionKind:
    error = !json.isMember("parameters");
    error |= !json.isMember("value");
    error |= !json.isMember("location");
    if (error)
      break;
    for (Json::Value js : json["parameters"]) {
      paramVec.push_back(createParameterFromJson(js));
    }
    return std::make_shared<Ast::Function>(paramVec,
                                           createTermFromJson(json["value"]));

  case Ast::LetKind:
    error = !json.isMember("name");
    error |= !json.isMember("value");
    error |= !json.isMember("next");
    if (error)
      break;
    return std::make_shared<Ast::Let>(createParameterFromJson(json["name"]),
                                      createTermFromJson(json["value"]),
                                      createTermFromJson(json["next"]));

  case Ast::IfKind:
    error = !json.isMember("condition");
    error |= !json.isMember("then");
    error |= !json.isMember("otherwise");
    if (error)
      break;
    return std::make_shared<Ast::If>(createTermFromJson(json["condition"]),
                                     createTermFromJson(json["then"]),
                                     createTermFromJson(json["otherwise"]));

  case Ast::PrintKind:
    error = !json.isMember("value");
    if (error)
      break;
    return std::make_shared<Ast::Print>(createTermFromJson(json["value"]));

  case Ast::FirstKind:
    error = !json.isMember("value");
    if (error)
      break;
    return std::make_shared<Ast::First>(createTermFromJson(json["value"]));

  case Ast::SecondKind:
    error = !json.isMember("value");
    if (error)
      break;
    return std::make_shared<Ast::Second>(createTermFromJson(json["value"]));

  case Ast::BoolKind:
    error = !json.isMember("value");
    if (error)
      break;
    return std::make_shared<Ast::Bool>(json["value"].asBool());

  case Ast::TupleKind:
    error = !json.isMember("first");
    error |= !json.isMember("second");
    if (error)
      break;
    return std::make_shared<Ast::Tuple>(createTermFromJson(json["first"]),
                                        createTermFromJson(json["second"]));

  case Ast::VarKind:
    error = !json.isMember("text");
    if (error)
      break;
    return std::make_shared<Ast::Var>(json["text"].asString());

  default:
    if (error)
      ABORT("Term illformed on " + json["location"]["file"].asString() + "(" +
            std::to_string(json["location"]["start"].asInt()) + ", " +
            std::to_string(json["location"]["end"].asInt()) + ")");
  }

  __builtin_unreachable();
}

std::shared_ptr<Ast::File> Ast::createNodeFromJson(const Json::Value &json) {
  has_properties_or_abort(json, "name", "expression", "location");
  return std::make_shared<Ast::File>(json["name"].asString(),
                                     createTermFromJson(json["expression"]));
}

std::string getOpString(Ast::BinaryOp op) {
  switch (op) {
  case Ast::Add:
    return "add";
  case Ast::Sub:
    return "sub";
  case Ast::Mul:
    return "mul";
  case Ast::Div:
    return "Div";
  case Ast::Rem:
    return "rem";
  case Ast::Eq:
    return "eq";
  case Ast::Neq:
    return "Not eq";
  case Ast::Lt:
    return "<";
  case Ast::Gt:
    return ">";
  case Ast::Lte:
    return "<=";
  case Ast::Gte:
    return ">=";
  case Ast::And:
    return "&";
  case Ast::Or:
    return "|";
  }
  return "";
}

std::string getStringValueOfTerm(const Ast::Term &value) {
  std::string response = "";
  bool first = true;
  switch (value->kind) {
  case Ast::IntKind:
    response = std::to_string(std::static_pointer_cast<Ast::Int>(value)->value);
    break;
  case Ast::BoolKind:
    response =
        std::static_pointer_cast<Ast::Bool>(value)->value ? "true" : "false";
    break;
  case Ast::StrKind:
    response = "\"" + std::static_pointer_cast<Ast::Str>(value)->value + "\"";
    break;
  case Ast::TupleKind:
    response = "(";
    response += getStringValueOfTerm(
        std::static_pointer_cast<Ast::Tuple>(value)->first);
    response += ", ";
    response += getStringValueOfTerm(
        std::static_pointer_cast<Ast::Tuple>(value)->second);
    response += ")";
    break;
  case Ast::FunctionKind:
    response = "<#";
    for (Ast::Parameter par :
         std::static_pointer_cast<Ast::Function>(value)->parameters) {
      if (!first)
        response += ", ";
      first = false;
      response += par.text;
    }
    response += ">";
    response += '\n';
    break;
  case Ast::CallKind:
    response = "Call(" +
               getStringValueOfTerm(
                   std::static_pointer_cast<Ast::Call>(value)->callee) +
               ")";
    response += '\n';
    break;
  case Ast::BinaryKind:
    response +=
        getStringValueOfTerm(std::static_pointer_cast<Ast::Binary>(value)->lhs);
    response += getOpString(std::static_pointer_cast<Ast::Binary>(value)->op);
    response +=
        getStringValueOfTerm(std::static_pointer_cast<Ast::Binary>(value)->rhs);
    response += '\n';
    break;
  case Ast::LetKind:
    response +=
        "auto " + std::static_pointer_cast<Ast::Let>(value)->name.text + " = " +
        getStringValueOfTerm(std::static_pointer_cast<Ast::Let>(value)->value);
    response += ";\n";
    response +=
        getStringValueOfTerm(std::static_pointer_cast<Ast::Let>(value)->next);
    break;
  case Ast::PrintKind:
    response += "print(" +
                getStringValueOfTerm(
                    std::static_pointer_cast<Ast::Print>(value)->value) +
                ")";
  }
  return response;
}

int Ast::File::dumpToFile(std::string filename) {
  std::ofstream file;
  file.open(filename);

  file << "int gen_main() {\n";
  file << getStringValueOfTerm(term) << ";\n";
  file << "return 0;\n";
  file << "}\n";
  file.close();
  return 0;
}