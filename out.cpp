#include <cstdio>
#include <cstdlib>

int print(int arg, bool append_newline = true) {
  printf("%d", arg);
  printf(append_newline ? "\n" : "");
  return arg;
}

const char *print(const char *arg, bool append_newline = true) {
  printf("%s", arg);
  printf(append_newline ? "\n" : "");
  return arg;
}

bool print(bool arg, bool append_newline = true) {
  printf("%s", arg ? "true" : "false");
  printf(append_newline ? "\n" : "");
  return arg;
}

// For functions
template <typename T> T print(T arg, bool append_newline = true) {
  printf("<#closure>");
  printf(append_newline ? "\n" : "");
  return arg;
}

template <typename T1, typename T2> struct tuple {
  T1 first;
  T2 second;
};

template <typename T1, typename T2>
struct tuple<T1, T2> print(struct tuple<T1, T2> arg) {
    printf("(");
    print(arg.first, false);
    printf(", ");
    print(arg.second, false);
    printf(")\n");
    return arg;
}

int gen_main();

int main() { return gen_main(); }

#include "gen_main.cpp"