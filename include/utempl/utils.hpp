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
inline constexpr std::size_t kTupleSize<const T> = kTupleSize<std::remove_cv_t<T>>;

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

namespace impl {

template <typename...>
struct LeftFold;

template <typename T>
struct LeftFold<T> {
  T data;
};

template <typename T, typename F>
struct LeftFold<T, F> {
  T data;
  const F& f;
  template <typename TT>
  inline constexpr auto operator|(LeftFold<TT>&& other) {
    using R = decltype(f(std::move(this->data), std::move(other.data)));
    return LeftFold<R, F>{.data = f(std::move(this->data), std::move(other.data)), .f = this->f};
  };
};



} // namespace impl


template <TupleLike Tuple, typename T, typename F>
inline constexpr auto LeftFold(Tuple&& tuple, T&& init, F&& f) {
  return [&]<auto... Is>(std::index_sequence<Is...>){
    return (
      impl::LeftFold<std::remove_cvref_t<T>, std::remove_cvref_t<F>>{.data = std::forward<T>(init), .f = std::forward<F>(f)} 
      | ...
      | impl::LeftFold<std::remove_cvref_t<decltype(Get<Is>(tuple))>>{.data = Get<Is>(tuple)}
    ).data;
  }(std::make_index_sequence<kTupleSize<Tuple>>());
};



template <TupleLike Tuple, TupleLike Tuple2>
inline constexpr auto TupleCat(Tuple&& tuple, Tuple2&& tuple2) {
  return [&]<auto... Is, auto... IIs>(std::index_sequence<Is...>, std::index_sequence<IIs...>){
    return MakeTuple<Tuple>(Get<Is>(tuple)..., Get<IIs>(tuple2)...);
  }(std::make_index_sequence<kTupleSize<Tuple>>(), std::make_index_sequence<kTupleSize<Tuple2>>());
};

template <TupleLike... Tuples>
inline constexpr auto TupleCat(Tuples&&... tuples) requires (sizeof...(tuples) >= 1) {
  return LeftFold(
    Tuple{std::forward<Tuples>(tuples)...},
    MakeTuple<decltype(Arg<0>(std::forward<Tuples>(tuples)...))>(),
    []<TupleLike Tup, TupleLike Tup2>(Tup&& tup, Tup2&& tup2){
      return TupleCat(std::forward<Tup>(tup), std::forward<Tup2>(tup2));
    });
};
template <typename... Ts>
inline constexpr auto Tie(Ts&... args) -> Tuple<Ts&...> {
  return {args...};
};

template <template <typename...> typename F, TupleLike Tuple>
inline constexpr bool kEveryElement = 
  []<auto... Is>(std::index_sequence<Is...>){
    return (F<decltype(Get<Is>(std::declval<Tuple>()))>::value && ...);
  }(std::make_index_sequence<kTupleSize<Tuple>>());

template <template <typename...> typename F, typename... Ts>
struct PartialCaller {
  template <typename... TTs>
  using type = F<Ts..., TTs...>;
};
template <template <typename...> typename F, typename... Ts>
consteval auto PartialCallerF(TypeList<Ts...>) {
  return []<typename... TTs>(TypeList<TTs...>){
    return F<Ts..., TTs...>{};
  };
};



template <TupleLike Tuple, typename T>
inline constexpr auto FirstOf(Tuple&& tuple, T&& init) requires kEveryElement<std::is_invocable, Tuple> {
  return LeftFold(
    std::forward<Tuple>(tuple),
    std::forward<T>(init),
    []<typename TT, typename F>(TT&& value, F&& f) -> TT {
      if(value) {
        return value;
      };
      return f();
    }
  );
};


template <TupleLike Tuple>
inline constexpr auto Filter(Tuple&& tuple, auto&& f) {
  return LeftFold(
    std::forward<Tuple>(tuple),
    MakeTuple<Tuple>(),
    [&]<TupleLike Accumulator, typename T>(Accumulator&& accumulator, T&& add) {
      if constexpr(decltype(f(std::forward<T>(add))){}) {
        return TupleCat(std::forward<Accumulator>(accumulator), std::forward<T>(add));
      } else {
        return accumulator;
      };
    }
  );
};

} // namespace utempl
