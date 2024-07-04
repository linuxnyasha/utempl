#pragma once
#include <utempl/overloaded.hpp>
#include <utempl/type_list.hpp>
namespace utempl {

template <auto>
struct Wrapper;
namespace impl {

template <auto, typename T>
struct TupleLeaf {
  T value;
  constexpr auto operator==(const TupleLeaf&) const -> bool = default;
};

template <typename, typename...>
struct TupleHelper;

template <std::size_t... Is, typename... Ts>
struct TupleHelper<std::index_sequence<Is...>, Ts...> : public impl::TupleLeaf<Is, Ts>... {
  constexpr auto operator==(const TupleHelper&) const -> bool = default;
};

}  // namespace impl

template <typename... Ts>
struct Tuple;

template <std::size_t I, typename... Ts>
constexpr auto Get(Tuple<Ts...>& tuple) -> auto&
  requires(I < sizeof...(Ts))
{
  using Type = decltype(Get<I>(TypeList<Ts...>{}));
  return static_cast<impl::TupleLeaf<I, Type>&>(tuple).value;
};

template <std::size_t I, typename... Ts>
constexpr auto Get(const Tuple<Ts...>& tuple) -> const auto&
  requires(I < sizeof...(Ts))
{
  using Type = decltype(Get<I>(TypeList<Ts...>{}));
  return static_cast<const impl::TupleLeaf<I, Type>&>(tuple).value;
};

template <std::size_t I, typename... Ts>
constexpr auto Get(Tuple<Ts...>&& tuple) -> decltype(auto)
  requires(I < sizeof...(Ts))
{
  using Type = decltype(Get<I>(TypeList<Ts...>{}));
  return std::move(static_cast<impl::TupleLeaf<I, Type>&&>(tuple).value);
};

template <std::size_t I, typename T>
constexpr auto Get(T&& arg) -> decltype(get<I>(std::forward<T>(arg))) {
  return get<I>(std::forward<T>(arg));
};

template <typename... Ts>
struct Tuple : impl::TupleHelper<std::index_sequence_for<Ts...>, Ts...> {
  template <typename... TTs>
  constexpr Tuple(TTs&&... args) /* NOLINT */ : impl::TupleHelper<std::index_sequence_for<Ts...>, Ts...>{{std::forward<TTs>(args)}...} {};
  template <std::invocable... Fs>
  constexpr Tuple(Fs&&... fs)  // NOLINT
    requires(!std::invocable<Ts> || ...)
      : impl::TupleHelper<std::index_sequence_for<Ts...>, Ts...>{{fs()}...} {};
  constexpr Tuple(Ts&&... args) : impl::TupleHelper<std::index_sequence_for<Ts...>, Ts...>{{args}...} {};  // NOLINT
  constexpr Tuple(const Tuple&) = default;
  constexpr Tuple(Tuple&&) = default;
  constexpr Tuple(Tuple&) = default;
  constexpr auto operator=(const Tuple&) -> Tuple& = default;
  constexpr auto operator=(Tuple&&) -> Tuple& = default;
  constexpr auto operator=(Tuple&) -> Tuple& = default;
  constexpr Tuple() : impl::TupleHelper<std::index_sequence_for<Ts...>, Ts...>() {};
  constexpr auto operator==(const Tuple<Ts...>& other) const -> bool {
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
      return ((Get<Is>(*this) == Get<Is>(other)) && ...);
    }(std::index_sequence_for<Ts...>());
  };
  template <typename... TTs>
  constexpr auto operator+(const Tuple<TTs...>& other) const -> Tuple<Ts..., TTs...> {
    return [&]<std::size_t... Is, std::size_t... IIs>(std::index_sequence<Is...>, std::index_sequence<IIs...>) -> Tuple<Ts..., TTs...> {
      return {Get<Is>(*this)..., Get<IIs>(other)...};
    }(std::index_sequence_for<Ts...>(), std::index_sequence_for<TTs...>());
  };

  template <auto I>
  constexpr auto operator[](Wrapper<I>) const -> const auto& {
    return Get<I>(*this);
  };

  template <auto I>
  constexpr auto operator[](Wrapper<I>) -> auto& {
    return Get<I>(*this);
  };
};

template <>
struct Tuple<> {
  template <typename... Ts>
  constexpr auto operator+(const Tuple<Ts...>& other) const -> Tuple<Ts...> {
    return other;
  };
  constexpr auto operator==(const Tuple<>&) const {
    return true;
  };
};
template <typename... Ts>
Tuple(Ts&&...) -> Tuple<std::decay_t<Ts>...>;

template <typename... Ts>
consteval auto ListFromTuple(Tuple<Ts...>) -> TypeList<Ts...> {
  return {};
};

}  // namespace utempl
