
#include <cassert>

#include "generate.h"

int main(int argc, char **argv) {
  assert(argc == 2);
  return generateFromJson(argv[1]);
}
