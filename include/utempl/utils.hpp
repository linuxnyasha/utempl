#pragma once
#include <utempl/tuple.hpp>
#include <utempl/overloaded.hpp>
#include <utempl/constexpr_string.hpp>
#include <fmt/format.h>
namespace utempl {

template <auto Value>
struct Wrapper {
  static constexpr auto kValue = Value;
  inline constexpr auto operator==(auto&& arg) {
    return arg == Value;
  };
  consteval operator decltype(Value)() {
    return Value;
  };
};

template <ConstexprString string, typename T = std::size_t>
consteval auto ParseNumber() -> T {
  T response{};
  for(const auto& c : string) {
    if (c >= '0' && c <= '9') {
      response = response * 10 + (c - '0');
    };
  };
  return response;
};
namespace literals {

template <char... cs>
consteval auto operator"" _c() {
  return Wrapper<ParseNumber<ConstexprString<sizeof...(cs)>({cs...})>()>{};
};

} // namespace literals

template <std::size_t I, typename... Ts>
inline constexpr auto Arg(Ts&&... args) requires (I < sizeof...(Ts)) {
  return [&]<auto... Is>(std::index_sequence<Is...>){
    return [](decltype(Caster(Is))..., auto&& response, ...){
      return response;
    }(std::forward<Ts>(args)...);
  }(std::make_index_sequence<I>());
};


template <std::size_t Count>
inline constexpr auto Times(auto&& f) {
  [&]<auto... Is>(std::index_sequence<Is...>){
    (Arg<0>(f, Is)(), ...);
  }(std::make_index_sequence<Count>());
};

template <typename T>
inline constexpr std::size_t kTupleSize = []() -> std::size_t {
  static_assert(!sizeof(T), "Not Found");
  return 0;
}();


template <typename T>
inline constexpr std::size_t kTupleSize<T&&> = kTupleSize<std::remove_reference_t<T>>;

template <typename T>
inline constexpr std::size_t kTupleSize<T&> = kTupleSize<std::remove_reference_t<T>>;

template <typename T>
inline constexpr std::size_t kTupleSize<const T&> = kTupleSize<std::remove_cvref_t<T>>;

template <typename T>
inline constexpr std::size_t kTupleSize<const T&&> = kTupleSize<std::remove_cvref_t<T>>;

template <template <typename...> typename M, typename... Ts>
inline constexpr std::size_t kTupleSize<M<Ts...>> = sizeof...(Ts);

template <template <typename, std::size_t> typename Array, typename T, std::size_t N>
inline constexpr std::size_t kTupleSize<Array<T, N>> = N;


template <typename T>
concept TupleLike = kTupleSize<T> == 0 || requires(T t){Get<0>(t);};

template <typename T>
concept IsTypeList = Overloaded(
  []<typename... Ts>(TypeList<TypeList<Ts...>>) {return true;},
  [](auto&&) {return false;}
)(kType<std::remove_cvref_t<T>>);


template <TupleLike T = Tuple<>, typename... Args>
inline constexpr auto MakeTuple(Args&&... args) {
  return Overloaded(
    [&]<template <typename...> typename M, typename... Ts>(TypeList<M<Ts...>>){
      return M{std::forward<Args>(args)...};
    },
    [&]<template <typename, std::size_t> typename Array, typename TT, std::size_t N>(TypeList<Array<TT, N>>) {
      return Array{std::forward<Args>(args)...};
    }
  )(kType<std::remove_cvref_t<T>>);
};

template <TupleLike Tuple>
inline constexpr auto Transform(Tuple&& container, auto&& f) {
  return [&]<auto... Is>(std::index_sequence<Is...>){
    return MakeTuple<Tuple>(f(Get<Is>(container))...);
  }(std::make_index_sequence<kTupleSize<Tuple>>());
};

template <TupleLike Tuple>
inline constexpr auto Reverse(Tuple&& tuple) {
  return [&]<auto... Is>(std::index_sequence<Is...>) {
    return MakeTuple<Tuple>(Get<kTupleSize<Tuple> - Is - 1>(tuple)...);
  }(std::make_index_sequence<kTupleSize<Tuple>>());
};

template <TupleLike Tuple, TupleLike Tuple2>
inline constexpr auto TupleCat(Tuple&& tuple, Tuple2&& tuple2) {
  return [&]<auto... Is, auto... IIs>(std::index_sequence<Is...>, std::index_sequence<IIs...>){
    return MakeTuple<Tuple>(Get<Is>(tuple)..., Get<IIs>(tuple2)...);
  }(std::make_index_sequence<kTupleSize<Tuple>>(), std::make_index_sequence<kTupleSize<Tuple2>>());
};
namespace impl {
template <TupleLike Tuple>
struct TupleCater {
  Tuple tuple;
  template <TupleLike Other>
  inline constexpr auto operator+(TupleCater<Other>&& other) {
    using ResultType = decltype(TupleCat(std::move(this->tuple), other.tuple));
    return TupleCater<ResultType>{TupleCat(std::move(this->tuple), std::move(other.tuple))};
  };
};
} // namespace impl

template <TupleLike... Tuples>
inline constexpr auto TupleCat(Tuples&&... tuples) {
  return (impl::TupleCater{std::forward<Tuples>(tuples)} + ...).tuple;
};

template <TupleLike Tuple>
inline constexpr auto Filter(Tuple&& tuple, auto&& f) {
  return [&]<auto... Is>(std::index_sequence<Is...>){
    return TupleCat([&]<auto I>(Wrapper<I>){
      constexpr bool flag = decltype(f(Get<I>(tuple)))::kValue;
      if constexpr(flag) {
        return MakeTuple<Tuple>(Get<I>(tuple));
      } else {
        return MakeTuple<Tuple>();
      };
    }(Wrapper<Is>{})...);
  }(std::make_index_sequence<kTupleSize<Tuple>>());
};

};
