#include <cstdlib>
#include <iostream>

#define ABORT(msg)                                                             \
  do {                                                                         \
    std::cout << msg << '\n';                                                  \
    std::cout << std::flush;                                                   \
    abort();                                                                   \
  } while (0)

bool has_properties(const Json::Value &) { return true; }

template <typename Arg1, typename... Args>
bool has_properties(const Json::Value &json, const Arg1 &arg1,
                    const Args &...args) {
  return json.isMember(arg1) && has_properties(json, args...);
}

template <typename Arg1, typename... Args>
void has_properties_or_abort(const Json::Value &json, const Arg1 &arg1,
                             const Args &...args) {
  if (!has_properties(json, arg1, args...)) {
    std::cout << json << '\n';
    ABORT("json ill-formed");
  }
}