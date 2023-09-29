#include <algorithm>
#include <cassert>
#include <fstream>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <unordered_map>

#ifndef NDEBUG
#include <iostream>
#endif

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
  if (!termLookupTable.contains(kind))
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

  case Ast::ProgramKind:;
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

int anon_counter = 0;

std::unordered_map<Ast::Term::pointer, std::string> functionNameCache;

static inline std::string getStringValueOfTerm(const Ast::Term &value,
                                               const Ast::Term &parent,
                                               std::ofstream &file) {
  std::string response;
  switch (value->kind) {
  case Ast::IntKind: {
    response.append(
        std::to_string(static_cast<Ast::Int *>(value.get())->value));
    return response;
  }

  case Ast::BoolKind: {
    response.append(static_cast<Ast::Bool *>(value.get())->value ? "true"
                                                                 : "false");
    return response;
  }

  case Ast::StrKind: {
    response.append("\"")
        .append(static_cast<Ast::Str *>(value.get())->value)
        .append("\"");
    return response;
  }

  case Ast::TupleKind: {
    auto const &first_str = getStringValueOfTerm(
        static_cast<Ast::Tuple *>(value.get())->first, value, file);
    auto const &second_str = getStringValueOfTerm(
        static_cast<Ast::Tuple *>(value.get())->second, value, file);

    response.append("__tuple<")
        .append("decltype(")
        .append(first_str)
        .append(")")
        .append(", ")
        .append("decltype(")
        .append(second_str)
        .append(")>{")
        .append(first_str)
        .append(", ")
        .append(second_str)
        .append("}");
    return response;
  }

  case Ast::VarKind: {
    response.append(static_cast<Ast::Var *>(value.get())->text);
    return response;
  }

  case Ast::FunctionKind: {
    if (functionNameCache.contains(value.get()))
      return functionNameCache[value.get()];

    std::string name = (parent->kind == Ast::LetKind)
                           ? static_cast<Ast::Let *>(parent.get())->name
                           : "__anon_fn_" + (std::to_string(anon_counter++));

    // We only care about functions that are either set to a variable or that
    // are immediately called
    bool const generate_def =
        (parent->kind == Ast::LetKind) || (parent->kind == Ast::CallKind);

    auto const &f = static_cast<Ast::Function *>(value.get());
    std::size_t const numParams = f->parameters.size();

    std::string function_def;
    if (generate_def) {
      if (numParams) {
        function_def.append("template <");
        for (std::size_t i = 0; i < numParams; i++) {
          function_def.append("typename T").append(std::to_string(i));
          if (i < (numParams - 1))
            function_def.append(", ");
        }
        function_def.append(">");
      }

      function_def.append("auto ").append(name).append("(");
      for (std::size_t i = 0; i < numParams; i++) {
        function_def.append("T")
            .append(std::to_string(i))
            .append(" ")
            .append(f->parameters[i]);
        if (i < (numParams - 1))
          function_def.append(", ");
      }
      function_def.append(") {");

      // If the body doesn't start with let, function, or if, it's a return
      bool const must_return = f->value->kind != Ast::LetKind &&
                               f->value->kind != Ast::IfKind &&
                               f->value->kind != Ast::FunctionKind;
      if (must_return)
        function_def.append("return ");

      function_def.append(getStringValueOfTerm(f->value, value, file));

      if (must_return)
        function_def.append(";");

      function_def.append("}");

      // If the parent was a call, we need to return the name of the function
      // with the template instantiated
      if (parent->kind == Ast::CallKind) {
        auto const &c = static_cast<Ast::Call *>(parent.get());
        std::size_t const numArgs = c->arguments.size();
        if (numArgs) {
          name.append("<");
          for (std::size_t i = 0; i < numArgs; i++) {

            // We won't be adding the current function to the name
            if (c->arguments[i] == value)
              continue;

            name.append("decltype(")
                .append(getStringValueOfTerm(c->arguments[i], value, file))
                .append(")");
            if (i < (numArgs - 1))
              name.append(", ");
          }
          name.append(">");
        }
      }
    } else {
      // Anon function: generate as lambda
      function_def.append("void ").append(name).append("() {};");
    }

    file << function_def;

    functionNameCache[value.get()] = name;
    return name;
  }

  case Ast::CallKind: {
    response
        .append(getStringValueOfTerm(
            static_cast<Ast::Call *>(value.get())->callee, value, file))
        .append("(");

    auto const &c = static_cast<Ast::Call *>(value.get());
    std::size_t const numArgs = c->arguments.size();
    for (std::size_t i = 0; i < numArgs; i++) {
      response.append(getStringValueOfTerm(c->arguments[i], value, file));
      if (i < (numArgs - 1))
        response.append(", ");
    }

    return response.append(")");
  }

  case Ast::BinaryKind: {
    response.append(getOpString(static_cast<Ast::Binary *>(value.get())->op))
        .append("(")
        .append(getStringValueOfTerm(
            static_cast<Ast::Binary *>(value.get())->lhs, value, file))
        .append(", ")
        .append(getStringValueOfTerm(
            static_cast<Ast::Binary *>(value.get())->rhs, value, file))
        .append(")");
    return response;
  }

  case Ast::LetKind: {
    auto rhs = getStringValueOfTerm(static_cast<Ast::Let *>(value.get())->value,
                                    value, file);

    // Special case functions
    auto const &v = static_cast<Ast::Let *>(value.get())->value;
    auto const &name = static_cast<Ast::Let *>(value.get())->name;
    if (v->kind != Ast::FunctionKind) {
      if (name != "_")
        response.append("auto ").append(name).append(" = ");
      response.append(rhs).append(";\n");
    }

    auto const &next = static_cast<Ast::Let *>(value.get())->next;
    auto const &body = getStringValueOfTerm(next, parent, file);

    {
      bool const must_return = (next->kind != Ast::LetKind &&
                                next->kind != Ast::IfKind && parent != nullptr);
      add_return_if_needed;
      response.append(body);
      add_semicolon_if_needed;
    }

    return response;
  }

  case Ast::IfKind: {
    auto const &cond = getStringValueOfTerm(
        static_cast<Ast::If *>(value.get())->condition, value, file);

    bool const print_as_ternary =
        !((parent == nullptr) || (parent->kind == Ast::FunctionKind));
    if (print_as_ternary)
      response.append(cond).append(" ? ");
    else
      response.append("if (").append(cond).append(") {\n");

    {
      auto const &then = static_cast<Ast::If *>(value.get())->then;
      auto const &then_str = getStringValueOfTerm(then, value, file);

      bool const must_return =
          !print_as_ternary && (then->kind != Ast::LetKind);
      add_return_if_needed;
      response.append(then_str);
      add_semicolon_if_needed;
    }

    if (print_as_ternary)
      response.append(" : ");
    else
      response.append(" } else {");

    {
      auto const &otherwise = static_cast<Ast::If *>(value.get())->otherwise;
      auto const &otherwise_str = getStringValueOfTerm(otherwise, value, file);

      bool const must_return =
          !print_as_ternary && (otherwise->kind != Ast::LetKind);

      add_return_if_needed;
      response.append(otherwise_str);
      add_semicolon_if_needed;
    }

    if (!print_as_ternary)
      response.append("}");

    return response;
  }

  case Ast::PrintKind: {
    response.append("print(")
        .append(getStringValueOfTerm(
            static_cast<Ast::Print *>(value.get())->value, value, file))
        .append(")");
    return response;
  }

  case Ast::FirstKind: {
    response.append("__first(")
        .append(getStringValueOfTerm(
            static_cast<Ast::Print *>(value.get())->value, value, file))
        .append(")");
    return response;
  }

  case Ast::SecondKind: {
    response.append("__second(")
        .append(getStringValueOfTerm(
            static_cast<Ast::Print *>(value.get())->value, value, file))
        .append(")");
    return response;
  }

  case Ast::ProgramKind:;
  }

  ABORT(std::string("Missing support for term ")
            .append(std::to_string(value->kind)));
  __builtin_unreachable();
}

static inline std::string
getJulia(const Ast::Term &value, const Ast::Term &parent, std::ofstream &file) {
  std::string response;
  switch (value->kind) {
  case Ast::IntKind:
    return response.append("Int32(")
        .append(std::to_string(static_cast<Ast::Int *>(value.get())->value))
        .append(")");

  case Ast::BoolKind:
    return (static_cast<Ast::Bool *>(value.get())->value ? "true" : "false");

  case Ast::StrKind:
    return response.append("\"")
        .append(static_cast<Ast::Str *>(value.get())->value)
        .append("\"");

  case Ast::VarKind:
    return static_cast<Ast::Var *>(value.get())->text;

  case Ast::TupleKind:
    return response.append("(")
        .append(getJulia(static_cast<Ast::Tuple *>(value.get())->first, value,
                         file))
        .append(", ")
        .append(getJulia(static_cast<Ast::Tuple *>(value.get())->second, value,
                         file))
        .append(")");

  case Ast::BinaryKind:
    return response
        .append(getOpString(static_cast<Ast::Binary *>(value.get())->op))
        .append("(")
        .append(
            getJulia(static_cast<Ast::Binary *>(value.get())->lhs, value, file))
        .append(", ")
        .append(
            getJulia(static_cast<Ast::Binary *>(value.get())->rhs, value, file))
        .append(")");

  case Ast::PrintKind:
    return response.append("__print(")
        .append(getJulia(static_cast<Ast::Print *>(value.get())->value, value,
                         file))
        .append(")\n");

  case Ast::CallKind: {
    response
        .append(getJulia(static_cast<Ast::Call *>(value.get())->callee, value,
                         file))
        .append("(");

    auto const &c = static_cast<Ast::Call *>(value.get());
    std::size_t const numArgs = c->arguments.size();
    for (std::size_t i = 0; i < numArgs; i++) {
      response.append(getJulia(
          static_cast<Ast::Call *>(value.get())->arguments[i], value, file));
      if (i < (numArgs - 1))
        response.append(", ");
    }
    return response.append(") ");
  }

  case Ast::FunctionKind: {
    auto const &name = "__anon_fn_" + (std::to_string(anon_counter++));

    file << "function " << name << "(";

    auto const &f = static_cast<Ast::Function *>(value.get());
    std::size_t const numParams = f->parameters.size();

    for (std::size_t i = 0; i < numParams; i++) {
      file << f->parameters[i];
      if (i < (numParams - 1))
        file << ", ";
    }
    file << ")\n";
    file << getJulia(static_cast<Ast::Function *>(value.get())->value, value,
                     file);
    file << "end\n";
    return name;
  }

  case Ast::LetKind:
    return response.append(static_cast<Ast::Let *>(value.get())->name)
        .append(" = ")
        .append(
            getJulia(static_cast<Ast::Let *>(value.get())->value, value, file))
        .append("\n")
        .append(
            getJulia(static_cast<Ast::Let *>(value.get())->next, value, file))
        .append("\n");

  case Ast::FirstKind:
    return getJulia(static_cast<Ast::First *>(value.get())->value, value, file)
        .append("[1]");

  case Ast::SecondKind:
    return getJulia(static_cast<Ast::First *>(value.get())->value, value, file)
        .append("[2]");

  case Ast::IfKind:
    return response.append("if ")
        .append(getJulia(static_cast<Ast::If *>(value.get())->condition, value,
                         file))
        .append("\n")
        .append(
            getJulia(static_cast<Ast::If *>(value.get())->then, value, file))
        .append("\nelse\n")
        .append(getJulia(static_cast<Ast::If *>(value.get())->otherwise, value,
                         file))
        .append("\nend\n");

  case Ast::ProgramKind:;
  }

  ABORT(std::string("Missing support for term ")
            .append(std::to_string(value->kind)));
  __builtin_unreachable();
}

} // namespace

int generateFromJson(const char *pathToJson, const char *mode) {
  std::ifstream fss(pathToJson);

  Json::Value json;
  Json::Reader reader;
  reader.parse(fss, json);

  has_properties_or_abort(json, "name", "expression", "location");
  auto ast = createTermFromJson(json["expression"]);

  std::ofstream file;
  if (atoi(mode)) {
    file.open("generated_main.cpp");
    file << "#include \"out.h\"\n\n";

    auto main_body = getStringValueOfTerm(ast, nullptr, file);

    file << "int main() {\n";
    file << main_body << ";\n";
    file << "return 0;\n";
    file << "}\n";
    file.close();
  } else {
    file.open("generated_main.jl");
    file << "include(\"builtin.jl\")\n\n";
    file << getJulia(ast, nullptr, file);
    file.close();
  }
  return 0;
}