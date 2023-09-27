#pragma once

#include <cstdlib>
#include <iostream>
#include <jsoncpp/json/value.h>
#include <utility>

#define ABORT(msg)                                                             \
  do {                                                                         \
    std::cout << msg << '\n';                                                  \
    std::cout << std::flush;                                                   \
    abort();                                                                   \
  } while (0)

static inline bool has_properties(const Json::Value & /*unused*/) {
  return true;
}

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
    std::cout << json.asString() << '\n';
    ABORT("json ill-formed");
  }
}

template<typename T>
class SetForScope {
public:
    SetForScope(T& scopedVariable)
        : m_scopedVariable(scopedVariable)
        , m_valueToRestore(scopedVariable)
    {
    }

    template<typename U>
    SetForScope(T& scopedVariable, U&& newValue)
        : SetForScope(scopedVariable)
    {
        m_scopedVariable = static_cast<T>(std::forward<U>(newValue));
    }

    ~SetForScope()
    {
        m_scopedVariable = std::move(m_valueToRestore);
    }

private:
    T& m_scopedVariable;
    T m_valueToRestore;
};
