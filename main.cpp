
#include <cassert>
#include <fstream>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>

#include "ast.h"

int main(int argc, char **argv) {
  assert(argc == 2);
  std::ifstream fss(argv[1]);

  Json::Value json;
  Json::Reader reader;
  reader.parse(fss, json);

  auto ast = Ast::createNodeFromJson(json);
  return ast->dumpToFile("gen_main.cpp");
}
