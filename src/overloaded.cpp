#pragma once
#include <utempl/module.hpp>

#ifdef UTEMPL_MODULE
export module utempl.overloaded;
import std;

#else
#include <type_traits>
#endif

namespace utempl {
UTEMPL_EXPORT template <typename... Fs>
constexpr auto Overloaded(Fs&&... fs) {
  struct Overloaded : public std::remove_cvref_t<Fs>... {
    using Fs::operator()...;
  };
  return Overloaded{std::forward<Fs>(fs)...};
};

}  // namespace utempl
