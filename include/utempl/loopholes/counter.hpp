#pragma once
#include <utempl/loopholes/core.hpp>
#include <cstddef>
#include <tuple>

namespace utempl::loopholes {

namespace impl {

template <typename Tag, std::size_t Value>
struct TagWithValue {};

template <typename Tag, std::size_t I, typename... Ts>
consteval auto Counter() -> std::size_t {
  if constexpr(requires{Magic(Getter<TagWithValue<Tag, I>{}>{});}) {
    return Counter<Tag, I + 1, Ts...>();
  };
  return (std::ignore = Injector<TagWithValue<Tag, I>{}, 0>{}, I);
};
} // namespace impl;

// For incerement counter need a unique Ts...
template <
  typename Tag,
  typename... Ts,
  std::size_t R = impl::Counter<Tag, 0, Ts...>()
>
consteval auto Counter(auto...) -> std::size_t {
  return R;
};


} // namespace utempl::loopholes
