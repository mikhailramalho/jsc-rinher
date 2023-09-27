#pragma once

#ifndef NDEBUG
#define ABORT(msg)                                                             \
  do {                                                                         \
    std::cout << msg << '\n';                                                  \
    std::cout << std::flush;                                                   \
    abort();                                                                   \
  } while (0)
#else
#define ABORT(msg)                                                             \
  do {                                                                         \
    abort();                                                                   \
  } while (0)
#endif

static inline bool has_properties(const Json::Value & /*unused*/) {
  return true;
}

#define add_return_if_needed                                                   \
  do {                                                                         \
    if (must_return)                                                           \
      response.append("return ");                                              \
  } while (0)

#define add_semicolon_if_needed                                                \
  do {                                                                         \
    if (must_return)                                                           \
      response.append(";");                                                    \
  } while (0)

template <typename Arg1, typename... Args>
static inline bool has_properties(const Json::Value &json, const Arg1 &arg1,
                                  const Args &...args) {
  return json.isMember(arg1) && has_properties(json, args...);
}

template <typename Arg1, typename... Args>
static inline void has_properties_or_abort(const Json::Value &json,
                                           const Arg1 &arg1,
                                           const Args &...args) {
  if (!has_properties(json, arg1, args...)) {
#ifndef NDEBUG
    std::cout << json.asString() << '\n';
#endif
    ABORT("json ill-formed");
  }
}
