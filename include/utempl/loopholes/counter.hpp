#pragma once
#include <utempl/loopholes/core.hpp>
#include <cstddef>

namespace utempl::loopholes {

namespace impl {

template <typename Tag, auto Value>
struct TagWithValue {};

template <auto I = 0, typename Tag, typename... Ts, auto = Injector<TagWithValue<Tag, I>{}>{}>
constexpr auto Counter(...) {
  return I;
};

template <auto I = 0, typename Tag, typename... Ts>
consteval auto Counter(std::size_t arg) requires Injected<TagWithValue<Tag, I>{}, Ts...> {
  return Counter<I + 1, Tag, Ts...>(arg);
};
} // namespace impl;

// For incerement counter need a unique Ts...
template <
  typename Tag,
  typename... Ts,
  std::size_t R = impl::Counter<0, Tag, Ts...>(std::size_t{})
#if defined __clang__
  - 1
#endif
>
consteval auto Counter(auto...) -> std::size_t {
  return R;
};


} // namespace utempl::loopholes
