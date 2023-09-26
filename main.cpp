#include "ast.h"
#include <fstream>
#include <iostream>
#include <json/reader.h>
#include <json/value.h>

int main(int argc, char **argv) {
  std::ifstream fss(argv[1]);
  Json::Value json;
  Json::Reader reader;
  reader.parse(fss, json);

  auto ast = Ast::Ast::createFromJson(json);
  return 0;
}
