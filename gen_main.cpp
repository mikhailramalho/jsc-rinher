#include <functional>

template <typename T> 
auto fib_fn(T n) {
  if (n < 2)
    return n;
  return fib_fn(n - 1) + fib_fn(n - 2);
}

// template <class F>
// using fn_type_t = std::remove_pointer_t<decltype(+std::declval<F>())>;

// template<typename F, typename ... Args>
// using fn_type_t = decltype(std::declval<F>().print(std::declval<Args>() ...)) (F::*)(Args ...);

int gen_main() {
  return 0;
}
