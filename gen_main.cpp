#include "out.h"

template <typename T0> auto fibbo(T0 n) {
  if (__lt(n, 2)) {
    return n;
  } else {
    return __add(fibbo(__sub(n, 1)), fibbo(__sub(n, 2)));
  };
}
int main() {
  auto _ = print("@!compile::");
  ;
  print(__add("@!fibbo::", __add(fibbo(15), "")));
  return 0;
}
