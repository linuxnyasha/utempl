#pragma once
#include <cstddef>
#include <tuple>
#include <utempl/loopholes/core.hpp>

namespace utempl::loopholes {

namespace impl {

template <typename Tag, std::size_t Value>
struct TagWithValue {};

template <bool Add, typename Tag, std::size_t I, typename... Ts>
consteval auto Counter() -> std::size_t {
  if constexpr(requires { Magic(Getter<TagWithValue<Tag, I>{}>{}); }) {
    return Counter<Add, Tag, I + 1, Ts...>();
  };
  if constexpr(Add) {
    return (std::ignore = Injector<TagWithValue<Tag, I>{}, 0>{}, I);
  } else {
    return I;
  };
};

}  // namespace impl

// For incerement counter need a unique Ts...
template <typename Tag, typename... Ts, std::size_t R = impl::Counter<true, Tag, 0, Ts...>()>
consteval auto Counter(auto...) -> std::size_t {
  return R;
};

// Without increment
template <typename Tag, typename... Ts, std::size_t R = impl::Counter<false, Tag, 0, Ts...>()>
consteval auto CountValue(auto...) -> std::size_t {
  return R;
};

/*
static_assert(Counter<void>() == 0);
static_assert(Counter<void, void>() == 1);
static_assert(CountValue<void>() == 2);
static_assert(CountValue<void, void>() == 2);
*/

}  // namespace utempl::loopholes
