#pragma once

#include <utempl/module.hpp>

#ifdef UTEMPL_MODULE

export module utempl.loopholes.core;
import std;

#else

#include <type_traits>

#endif

namespace utempl::loopholes {

UTEMPL_EXPORT_BEGIN
template <auto I>
struct Getter {
  friend constexpr auto Magic(Getter<I>);
};
template <auto I, auto Value = 0>
struct Injector {
  friend constexpr auto Magic(Getter<I>) {
    return Value;
  };
};
UTEMPL_EXPORT_END
template <auto, typename = void, typename... Ts>
struct InjectedImpl {
  static constexpr bool value = false;
};
template <auto V, typename... Ts>
struct InjectedImpl<V, std::void_t<decltype(Magic(Getter<V>{}))>, Ts...> {
  static constexpr bool value = true;
};

UTEMPL_EXPORT template <auto I, typename... Ts>
concept Injected = InjectedImpl<I, void, Ts...>::value;

}  // namespace utempl::loopholes
