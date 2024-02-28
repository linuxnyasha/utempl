#pragma once
#include <concepts>
#include <utility>
#include <array>
#include <ranges>


namespace utempl {

namespace impl {

struct Caster {
  constexpr Caster(auto&&) {};
};

};

template <typename... Ts>
struct TypeList {
};

template <typename T>
inline constexpr auto kType = TypeList<T>{};

template <typename... Ts>
inline constexpr auto kTypeList = TypeList<Ts...>{};


template <typename... Ts, typename... TTs>
consteval auto operator==(const TypeList<Ts...>& first, const TypeList<TTs...>& second) -> bool {
  return std::same_as<decltype(first), decltype(second)>;
};

template <typename... Ts, typename... TTs>
consteval auto operator+(const TypeList<Ts...>&, const TypeList<TTs...>&) -> TypeList<Ts..., TTs...> {
  return {};
};

template <std::size_t... Is, typename T>
consteval auto Get(std::index_sequence<Is...>, decltype(impl::Caster(Is))..., T, ...) -> T; 

template <std::size_t I, typename... Ts>
consteval auto Get(const TypeList<Ts...>&) -> decltype(Get(std::make_index_sequence<I>(), std::declval<Ts>()...)) requires (I < sizeof...(Ts));

template <typename T, typename... Ts>
consteval auto Find(TypeList<Ts...>) -> std::size_t {
  std::array arr{std::same_as<T, Ts>...};
  return std::ranges::find(arr, true) - arr.begin();
};

template <typename... Ts>
consteval auto Reverse(TypeList<Ts...> list) {
  return [&]<auto... Is>(std::index_sequence<Is...>) -> TypeList<decltype(Get<sizeof...(Ts) - Is - 1>(list))...> {
    return {};
  }(std::index_sequence_for<Ts...>());
};

template <typename... Ts>
consteval auto Transform(TypeList<Ts...>, auto&& f) -> TypeList<decltype(f(TypeList<Ts>{}))...> {
  return {};
};
template <typename... Ts>
consteval auto Filter(TypeList<Ts...>, auto&& f) {
  return ([](auto&& list){if constexpr(decltype(f(list))::kValue) {return list;} else {return kTypeList<>;}}(kType<Ts>) + ...);
};


} // namespace utempl
