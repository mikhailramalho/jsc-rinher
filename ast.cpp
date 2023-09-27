#include <algorithm>
#include <cassert>
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
    return "__add";
  case Ast::Sub:
    return "__sub";
  case Ast::Mul:
    return "__mul";
  case Ast::Div:
    return "__div";
  case Ast::Rem:
    return "__rem";
  case Ast::Eq:
    return "__eq";
  case Ast::Neq:
    return "__noteq";
  case Ast::Lt:
    return "__lt";
  case Ast::Gt:
    return "__gt";
  case Ast::Lte:
    return "__lte";
  case Ast::Gte:
    return "__gte";
  case Ast::And:
    return "__and";
  case Ast::Or:
    return "__or";
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
  }

  ABORT(std::string("Term ill-formed on ")
            .append(json["location"]["file"].asString())
            .append("(")
            .append(std::to_string(json["location"]["start"].asInt()))
            .append(", ")
            .append(std::to_string(json["location"]["end"].asInt()))
            .append(")"));
  __builtin_unreachable();
}

enum Scope : int {
  PROGRAM = 1,
  FUNCTION = 1 << 1,
  LET = 1 << 2,
  IFCOND = 1 << 3,
  IFBODY = 1 << 4,
  CALL = 1 << 5,
  BINARY = 1 << 6,
  TUPLE = 1 << 7,
};

static inline std::string getStringValueOfTerm(const Ast::Term &value,
                                               const Ast::Term &parent,
                                               std::ofstream &file,
                                               Scope scope) {
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

  case Ast::TupleKind: {
    SetForScope(scope, scope | TUPLE);

    return response.append("(")
        .append(getStringValueOfTerm(
            static_cast<Ast::Tuple *>(value.get())->first, value, file, scope))
        .append(", ")
        .append(getStringValueOfTerm(
            static_cast<Ast::Tuple *>(value.get())->second, value, file, scope))
        .append(")");
  }

  case Ast::FunctionKind: {
    SetForScope(scope, scope | FUNCTION);

    std::string function_def;
    function_def.append("template <");

    auto const &f = static_cast<Ast::Function *>(value.get());
    std::size_t numParams = f->parameters.size();
    for (std::size_t i = 0; i < numParams; i++) {
      function_def.append("typename T").append(std::to_string(i));
      if (i < (numParams - 1))
        function_def.append(", ");
    }

    assert(parent->kind == Ast::LetKind);
    auto const &p = static_cast<Ast::Let *>(parent.get());

    function_def.append("> auto ").append(p->name).append("(");
    for (std::size_t i = 0; i < numParams; i++) {
      function_def.append("T")
          .append(std::to_string(i))
          .append(" ")
          .append(f->parameters[i]);
      if (i < (numParams - 1))
        function_def.append(", ");
    }
    function_def.append(") {")
        .append(getStringValueOfTerm(f->value, value, file, scope))
        .append("}");

    file << function_def;
    return "";
  }

  case Ast::CallKind: {
    SetForScope(scope, scope | CALL);

    response
        .append(getStringValueOfTerm(
            static_cast<Ast::Call *>(value.get())->callee, value, file, scope))
        .append("(");

    auto const &c = static_cast<Ast::Call *>(value.get());
    std::size_t numParams = c->arguments.size();
    for (std::size_t i = 0; i < numParams; i++) {
      response.append(
          getStringValueOfTerm(c->arguments[i], value, file, scope));
      if (i < (numParams - 1))
        response.append(", ");
    }
    return response.append(")");
  }

  case Ast::BinaryKind: {
    SetForScope(scope, scope | BINARY);

    return response
        .append(getOpString(static_cast<Ast::Binary *>(value.get())->op))
        .append("(")
        .append(getStringValueOfTerm(
            static_cast<Ast::Binary *>(value.get())->lhs, value, file, scope))
        .append(", ")
        .append(getStringValueOfTerm(
            static_cast<Ast::Binary *>(value.get())->rhs, value, file, scope))
        .append(")");
  }

  case Ast::LetKind: {
    SetForScope(scope, scope | LET);

    auto rhs = getStringValueOfTerm(static_cast<Ast::Let *>(value.get())->value,
                                    value, file, scope);

    // Special case functions
    auto const &v = static_cast<Ast::Let *>(value.get())->value;
    if (v->kind != Ast::FunctionKind) {
      response.append("auto ")
          .append(static_cast<Ast::Let *>(value.get())->name)
          .append(" = ")
          .append(rhs)
          .append(";\n");
    }

    return response.append(getStringValueOfTerm(
        static_cast<Ast::Let *>(value.get())->next, value, file, scope));
  }

  case Ast::PrintKind: {
    SetForScope(scope, scope | CALL);

    return response.append("print(")
        .append(getStringValueOfTerm(
            static_cast<Ast::Print *>(value.get())->value, value, file, scope))
        .append(")");
  }

  case Ast::IfKind: {
    {
      SetForScope(scope, scope | IFCOND);
      response.append("if (").append(getStringValueOfTerm(
          static_cast<Ast::If *>(value.get())->condition, value, file, scope));
    }
    {
      SetForScope(scope, scope | IFBODY);
      return response.append(") { return \n")
          .append(getStringValueOfTerm(
              static_cast<Ast::If *>(value.get())->then, value, file, scope))
          .append(";}\nelse {return \n")
          .append(getStringValueOfTerm(
              static_cast<Ast::If *>(value.get())->otherwise, value, file,
              scope))
          .append(";}");
    }
  }

  case Ast::VarKind:
    return response.append(static_cast<Ast::Var *>(value.get())->text);
  }

  ABORT(std::string("Missing support for term ")
            .append(std::to_string(value->kind)));
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
  file << "#include \"out.h\"\n\n";

  auto main_body = getStringValueOfTerm(ast, nullptr, file, PROGRAM);

  file << "int main() {\n";
  file << main_body << ";\n";
  file << "return 0;\n";
  file << "}\n";
  file.close();
  return 0;
}