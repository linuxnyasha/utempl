#pragma once
#include <utempl/type_list.hpp>

namespace utempl {

template <auto>
struct Wrapper;
namespace impl {

template <auto, typename T>
struct TupleLeaf {
  T value;
  template <typename TT>
  inline constexpr TupleLeaf(TT&& arg) : value(std::forward<TT>(arg)) {};
  inline constexpr TupleLeaf(const TupleLeaf&) = default;
  inline constexpr TupleLeaf(TupleLeaf&&) = default;
  inline constexpr bool operator==(const TupleLeaf&) const = default;
};

template <std::size_t I, typename... Ts>
struct TupleHelper {
  consteval TupleHelper() = default;
  inline constexpr TupleHelper(const TupleHelper&) = default;
  inline constexpr TupleHelper(TupleHelper&&) = default;
  inline constexpr bool operator==(const TupleHelper&) const = default;
};

template <std::size_t I, typename T, typename... Ts>
struct TupleHelper<I, T, Ts...> : public TupleLeaf<I, T> , public TupleHelper<I + 1, Ts...> {
  template <typename TT, typename... TTs>
  inline constexpr TupleHelper(TT&& arg, TTs&&... args) : 
    TupleLeaf<I, T>{std::forward<TT>(arg)},
    TupleHelper<I + 1, Ts...>{std::forward<TTs>(args)...} {};
  inline constexpr TupleHelper(const TupleHelper&) = default;
  inline constexpr TupleHelper(TupleHelper&&) = default;
  inline constexpr bool operator==(const TupleHelper&) const = default;
};

} // namespace impl

template <typename... Ts>
struct Tuple;

template <std::size_t I, typename... Ts>
inline constexpr auto Get(Tuple<Ts...>& tuple) -> auto& requires (I < sizeof...(Ts)) {
  using Type = decltype(Get<I>(TypeList<Ts...>{}));
  return static_cast<impl::TupleLeaf<I, Type>&>(tuple).value;
};

template <std::size_t I, typename... Ts>
inline constexpr auto Get(const Tuple<Ts...>& tuple) -> const auto& requires (I < sizeof...(Ts)) {
  using Type = decltype(Get<I>(TypeList<Ts...>{}));
  return static_cast<const impl::TupleLeaf<I, Type>&>(tuple).value;
};

template <std::size_t I, typename... Ts>
inline constexpr auto Get(Tuple<Ts...>&& tuple) -> auto&& requires (I < sizeof...(Ts)) {
  using Type = decltype(Get<I>(TypeList<Ts...>{}));
  return std::move(static_cast<impl::TupleLeaf<I, Type>&&>(tuple).value);
};

template <std::size_t I, typename T>
inline constexpr auto Get(T&& arg) -> decltype(get<I>(std::forward<T>(arg))) {
  return get<I>(std::forward<T>(arg));
};

template <typename... Ts>
struct Tuple : public impl::TupleHelper<0, Ts...> {
  template <typename... TTs>
  inline constexpr Tuple(TTs&&... args) : 
    impl::TupleHelper<0, Ts...>(std::forward<TTs>(args)...) {};
  inline constexpr Tuple(const Tuple&) = default;
  inline constexpr Tuple(Tuple&&) = default;
  inline constexpr bool operator==(const Tuple&) const = default;
  template <typename... TTs>
  inline constexpr auto operator+(const Tuple<TTs...>& other) const -> Tuple<Ts..., TTs...> {
    return [&]<auto... Is, auto... IIs>(std::index_sequence<Is...>, std::index_sequence<IIs...>) -> Tuple<Ts..., TTs...> {
      return {Get<Is>(*this)..., Get<IIs>(other)...};
    }(std::make_index_sequence<sizeof...(Ts)>(), std::make_index_sequence<sizeof...(TTs)>());
  };
  template <auto I>
  inline constexpr auto operator[](Wrapper<I>) const -> const auto& {
    return Get<I>(*this);
  };
  template <auto I>
  inline constexpr auto operator[](Wrapper<I>) -> auto& {
    return Get<I>(*this);
  };
};

template <typename... Ts>
Tuple(Ts&&...) -> Tuple<std::remove_cvref_t<Ts>...>;

template <typename... Ts>
consteval auto ListFromTuple(Tuple<Ts...>) -> TypeList<Ts...> {
  return {};
};


} // namespace utempl
