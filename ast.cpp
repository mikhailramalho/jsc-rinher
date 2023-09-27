#include <algorithm>
#include <fstream>
#include <iostream>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <unordered_map>

#include "ast.h"
#include "utils.h"

namespace {

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
    return "&&";
  case Ast::Or:
    return "||";
  }
  __builtin_unreachable();
}

Ast::BinaryOp createBinaryOpFromJson(const Json::Value &json) {
  return binaryOpLookupTable[json.asString()];
}

Ast::Parameter createParameterFromJson(const Json::Value &json) {
  has_properties_or_abort(json, "text", "location");
  return {json["text"].asString()};
}

std::unique_ptr<Ast::Node> createTermFromJson(const Json::Value &json) {
  has_properties_or_abort(json, "kind", "location");

  const std::string &kind = json["kind"].asString();
  if (termLookupTable.count(kind) == 0U)
    ABORT("Term kind not recognized");

  Ast::Kind const termKind = termLookupTable.find(kind)->second;

  switch (termKind) {
  case Ast::IntKind:
    has_properties_or_abort(json, "value");
    return std::make_unique<Ast::Int>(json["value"].asInt());

  case Ast::StrKind:
    has_properties_or_abort(json, "value");
    return std::make_unique<Ast::Str>(json["value"].asString());

  case Ast::CallKind: {
    has_properties_or_abort(json, "arguments", "callee");

    auto const &jsonArgs = json["arguments"];
    std::vector<std::unique_ptr<Ast::Node>> args;
    args.reserve(jsonArgs.size());

    std::for_each(jsonArgs.begin(), jsonArgs.end(), [&](auto &&item) {
      args.push_back(createTermFromJson(item));
    });

    return std::make_unique<Ast::Call>(createTermFromJson(json["callee"]),
                                       std::move(args));
  }

  case Ast::BinaryKind:
    has_properties_or_abort(json, "lhs", "op", "rhs");
    return std::make_unique<Ast::Binary>(createTermFromJson(json["lhs"]),
                                         createBinaryOpFromJson(json["op"]),
                                         createTermFromJson(json["rhs"]));

  case Ast::FunctionKind: {
    has_properties_or_abort(json, "parameters", "value", "location");

    auto const &jsonParams = json["parameters"];
    std::vector<Ast::Parameter> params;
    params.reserve(jsonParams.size());

    std::for_each(jsonParams.begin(), jsonParams.end(), [&](auto &&item) {
      params.push_back(createParameterFromJson(item));
    });

    return std::make_unique<Ast::Function>(params,
                                           createTermFromJson(json["value"]));
  }

  case Ast::LetKind:
    has_properties_or_abort(json, "name", "value", "next");
    return std::make_unique<Ast::Let>(createParameterFromJson(json["name"]),
                                      createTermFromJson(json["value"]),
                                      createTermFromJson(json["next"]));

  case Ast::IfKind:
    has_properties_or_abort(json, "condition", "then", "otherwise");
    return std::make_unique<Ast::If>(createTermFromJson(json["condition"]),
                                     createTermFromJson(json["then"]),
                                     createTermFromJson(json["otherwise"]));

  case Ast::PrintKind:
    has_properties_or_abort(json, "value");
    return std::make_unique<Ast::Print>(createTermFromJson(json["value"]));

  case Ast::FirstKind:
    has_properties_or_abort(json, "value");
    return std::make_unique<Ast::First>(createTermFromJson(json["value"]));

  case Ast::SecondKind:
    has_properties_or_abort(json, "value");
    return std::make_unique<Ast::Second>(createTermFromJson(json["value"]));

  case Ast::BoolKind:
    has_properties_or_abort(json, "value");
    return std::make_unique<Ast::Bool>(json["value"].asBool());

  case Ast::TupleKind:
    has_properties_or_abort(json, "first", "second");
    return std::make_unique<Ast::Tuple>(createTermFromJson(json["first"]),
                                        createTermFromJson(json["second"]));

  case Ast::VarKind:
    has_properties_or_abort(json, "text");
    return std::make_unique<Ast::Var>(json["text"].asString());

  default:
    ABORT(std::string("Term ill-formed on ")
              .append(json["location"]["file"].asString())
              .append("(")
              .append(std::to_string(json["location"]["start"].asInt()))
              .append(", ")
              .append(std::to_string(json["location"]["end"].asInt()))
              .append(")"));
  }

  __builtin_unreachable();
}

std::string getStringValueOfTerm(const Ast::Term &value) {
  std::string response;

  switch (value->kind) {
  case Ast::IntKind:
    return response.append(
        std::to_string(static_cast<Ast::Int *>(value.get())->value));

  case Ast::BoolKind:
    return response.append(
        static_cast<Ast::Bool *>(value.get())->value ? "true" : "false");

  case Ast::StrKind:
    return response.append("\"")
        .append(static_cast<Ast::Str *>(value.get())->value)
        .append("\"");

  case Ast::TupleKind:
    return response.append("(")
        .append(
            getStringValueOfTerm(static_cast<Ast::Tuple *>(value.get())->first))
        .append(", ")
        .append(getStringValueOfTerm(
            static_cast<Ast::Tuple *>(value.get())->second))
        .append(")");

  case Ast::FunctionKind: {
    response.append("<#");
    bool first = true;
    for (const auto &param :
         static_cast<Ast::Function *>(value.get())->parameters) {
      if (!first)
        response.append(", ");
      first = false;
      response.append(param);
    }
    return response.append(">\n");
  }

  case Ast::CallKind:
    return response.append(
        "Call(" +
        getStringValueOfTerm(static_cast<Ast::Call *>(value.get())->callee) +
        ")\n");

  case Ast::BinaryKind:
    return response
        .append(
            getStringValueOfTerm(static_cast<Ast::Binary *>(value.get())->lhs))
        .append(getOpString(static_cast<Ast::Binary *>(value.get())->op))
        .append(
            getStringValueOfTerm(static_cast<Ast::Binary *>(value.get())->rhs))
        .append("\n");

  case Ast::LetKind:
    return response.append("auto ")
        .append(static_cast<Ast::Let *>(value.get())->name)
        .append(" = ")
        .append(
            getStringValueOfTerm(static_cast<Ast::Let *>(value.get())->value))
        .append(";\n")
        .append(
            getStringValueOfTerm(static_cast<Ast::Let *>(value.get())->next));

  case Ast::PrintKind:
    return response.append("print(")
        .append(
            getStringValueOfTerm(static_cast<Ast::Print *>(value.get())->value))
        .append(")");
  }
  __builtin_unreachable();
}

} // namespace

int generateFromJson(const char *pathToJson) {
  std::ifstream fss(pathToJson);

  Json::Value json;
  Json::Reader reader;
  reader.parse(fss, json);

  has_properties_or_abort(json, "name", "expression", "location");
  auto ast = createTermFromJson(json["expression"]);

  std::ofstream file;
  file.open("generated_main.cpp");

  file << "int gen_main() {\n";
  file << getStringValueOfTerm(ast) << ";\n";
  file << "return 0;\n";
  file << "}\n";
  file.close();
  return 0;
}