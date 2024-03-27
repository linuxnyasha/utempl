#pragma once

namespace utempl::loopholes {

template <auto I>
struct Getter {
  friend constexpr auto Magic(Getter<I>);
};
template <auto I, auto Value = 0>
struct Injector {
  friend constexpr auto Magic(Getter<I>) {return Value;};
};


template <auto I, typename...>
concept Injected = requires{Magic(Getter<I>{});};



} // namespace utempl::loopholes
