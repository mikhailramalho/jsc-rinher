#include <cstdio>
#include <cstdlib>
#include <functional>
#include <string>

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

std::string print(std::string arg, bool append_newline = true) {
  printf("%s", arg.c_str());
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

template <typename T0, typename T1> struct __tuple {
  T0 first;
  T1 second;
};

template <typename T0, typename T1>
struct __tuple<T0, T1> print(struct __tuple<T0, T1> arg) {
  printf("(");
  print(arg.first, false);
  printf(", ");
  print(arg.second, false);
  printf(")\n");
  return arg;
}

template <typename T0, typename T1>
T0 __first(struct __tuple<T0, T1> arg) {
  return arg.first;
}

template <typename T0, typename T1>
T1 __second(struct __tuple<T0, T1> arg) {
  return arg.second;
}


static inline int
__add_impl(int a, int b) {
  return a + b;
}
static inline std::string __add_impl(std::string a, std::string b) {
  return a + b;
}
static inline std::string __add_impl(std::string a, int b) {
  return a + std::to_string(b);
}
static inline std::string __add_impl(int a, std::string b) {
  return std::to_string(a) + b;
}

template <
    typename T0, typename T1,
    typename = std::enable_if_t<
        (std::is_integral_v<T0> || std::is_same_v<T0, const char *> ||
         std::is_same_v<T0, std::string> &&
             !std::is_same_v<T0, bool>)&&(std::is_integral_v<T1> ||
                                          std::is_same_v<T1, const char *> ||
                                          std::is_same_v<T1, std::string> &&
                                              !std::is_same_v<T1, bool>)>>
static inline auto __add(T0 a, T1 b) {
  return __add_impl(a, b);
}

template <typename T, typename = std::enable_if_t<std::is_integral_v<T> &&
                                                  !std::is_same_v<T, bool>>>
static inline auto __sub(T a, T b) {
  return a - b;
}

template <typename T, typename = std::enable_if_t<std::is_integral_v<T> &&
                                                  !std::is_same_v<T, bool>>>
static inline auto __mul(T a, T b) {
  return a * b;
}

template <typename T, typename = std::enable_if_t<std::is_integral_v<T> &&
                                                  !std::is_same_v<T, bool>>>
static inline auto __div(T a, T b) {
  return a / b;
}

template <typename T, typename = std::enable_if_t<std::is_integral_v<T> &&
                                                  !std::is_same_v<T, bool>>>
static inline auto __rem(T a, T b) {
  return a % b;
}

template <typename T,
          typename = std::enable_if_t<std::is_integral_v<T> ||
                                      std::is_same_v<T, const char *> ||
                                      std::is_same_v<T, bool>>>
static inline auto __eq(T a, T b) {
  return a == b;
}

template <typename T,
          typename = std::enable_if_t<std::is_integral_v<T> ||
                                      std::is_same_v<T, const char *> ||
                                      std::is_same_v<T, bool>>>
static inline auto __noteq(T a, T b) {
  return a != b;
}

template <typename T, typename = std::enable_if_t<std::is_integral_v<T> &&
                                                  !std::is_same_v<T, bool>>>
static inline auto __lt(T a, T b) {
  return a < b;
}

template <typename T, typename = std::enable_if_t<std::is_integral_v<T> &&
                                                  !std::is_same_v<T, bool>>>
static inline auto __lte(T a, T b) {
  return a <= b;
}

template <typename T, typename = std::enable_if_t<std::is_integral_v<T> &&
                                                  !std::is_same_v<T, bool>>>
static inline auto __gt(T a, T b) {
  return a > b;
}

template <typename T, typename = std::enable_if_t<std::is_integral_v<T> &&
                                                  !std::is_same_v<T, bool>>>
static inline auto __gte(T a, T b) {
  return a >= b;
}

template <typename T, typename = std::enable_if_t<std::is_same_v<T, bool>>>
static inline auto __and(T a, T b) {
  return a && b;
}

template <typename T, typename = std::enable_if_t<std::is_same_v<T, bool>>>
static inline auto __or(T a, T b) {
  return a || b;
}
