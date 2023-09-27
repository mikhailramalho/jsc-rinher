#include <functional>

// template <typename T0> auto sum_fn(T0 n) {
//   if (n == 1) {
//     return n;
//   } else {
//     return n + sum_fn(n - 1);
//   }
// }

// std::function<decltype(sum_fn<decltype(5)>)> sum = sum_fn<decltype(5)>;

template <typename T0> auto fib_fn(T0 n) {
  if (n < 2)
    return n;
  return fib_fn(n - 1) + fib_fn(n - 2);
}

int generated_main() {
  std::function<decltype(fib_fn<decltype(46)>)> f = fib_fn<decltype(46)>;
  print(f(46));
  // print(sum(46));
  return 0;
}
