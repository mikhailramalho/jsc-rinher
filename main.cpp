
#include <cassert>

#include "generate.h"

int main(int argc, char **argv) {
  assert(argc == 3);
  return generateFromJson(argv[1], argv[2]);
}
