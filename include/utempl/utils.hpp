#pragma once
#include <utempl/tuple.hpp>
#include <utempl/overloaded.hpp>
#include <utempl/constexpr_string.hpp>
#include <ranges>
#include <optional>
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
inline constexpr auto Arg(Ts&&... args) -> decltype(auto) requires (I < sizeof...(Ts)) {
  return [&]<auto... Is>(std::index_sequence<Is...>) -> decltype(auto) {
    return [](decltype(Caster(Is))..., auto&& response, ...) -> decltype(auto) {
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
struct TupleMaker {
};

template <typename... Ts>
struct TupleMaker<std::tuple<Ts...>> {
  template <typename... Args>
  static inline constexpr auto Make(Args&&... args) {
    return std::tuple{std::forward<Args>(args)...};
  };
};
template <typename... Ts>
struct TupleMaker<Tuple<Ts...>> {
  template <typename... Args>
  static inline constexpr auto Make(Args&&... args) {
    return Tuple{std::forward<Args>(args)...};
  };
};
template <typename T, std::size_t N>
struct TupleMaker<std::array<T, N>> {
  template <typename Arg, typename... Args>
  static inline constexpr auto Make(Arg&& arg, Args&&... args) 
      requires (std::same_as<std::remove_cvref_t<Arg>, std::remove_cvref_t<Args>> && ...) {
    return std::array{std::forward<Arg>(arg), std::forward<Args>(args)...};
  };
};

template <typename T>
struct TupleTieMaker {
};

template <typename... Ts>
struct TupleTieMaker<std::tuple<Ts...>> {
  template <typename... Args>
  static inline constexpr auto Make(Args&... args) -> std::tuple<Args...> {
    return {args...};
  };
};

template <typename... Ts>
struct TupleTieMaker<Tuple<Ts...>> {
  template <typename... Args>
  static inline constexpr auto Make(Args&... args) -> Tuple<Args...> {
    return {args...};
  };
};

template <typename T, std::size_t N>
struct TupleTieMaker<std::array<T, N>> {
  template <typename Arg, typename... Args>
  static inline constexpr auto Make(Arg& arg, Args&... args) -> std::array<Arg&, sizeof...(Args) + 1>
      requires (std::same_as<std::remove_cvref_t<Arg>, std::remove_cvref_t<Args>> && ...)  {
    return {arg, args...};
  };
};



template <typename T = Tuple<>, typename... Args>
inline constexpr auto MakeTuple(Args&&... args) 
    -> decltype(TupleMaker<std::remove_cvref_t<T>>::Make(std::forward<Args>(args)...)) {
  return TupleMaker<std::remove_cvref_t<T>>::Make(std::forward<Args>(args)...);
};

template <typename T = Tuple<>, typename... Args>
inline constexpr auto MakeTie(Args&... args) 
    -> decltype(TupleTieMaker<std::remove_cvref_t<T>>::Make(args...)) {
  return TupleTieMaker<std::remove_cvref_t<T>>::Make(args...);
};



template <typename T>
concept HasMakeTie = requires(int& arg) {MakeTie<T>(arg);};

namespace impl {

template <typename T>
struct Getter {
  friend consteval auto Magic(Getter<T>);
};


template <typename T, auto Insert>
struct Inserter {
  friend consteval auto Magic(Getter<T>) -> decltype(auto) {
    return Insert;
  };
};


template <typename T>
struct SafeTupleChecker {
  consteval SafeTupleChecker(SafeTupleChecker&&) {
    std::ignore = Inserter<SafeTupleChecker<T>, false>{};
  };
  consteval SafeTupleChecker(const SafeTupleChecker&) = default;
  consteval SafeTupleChecker() = default;
};
consteval auto TrueF(auto&&...) {
  return true;
};

template <typename T>
inline constexpr const SafeTupleChecker<T> kSafeTupleChecker;

template <typename T, bool = false>
struct IsSafeTuple {
  static constexpr auto value = true;
};

template <typename T>
struct IsSafeTuple<T, bool(TrueF(Get<0>(std::move(Get<0>(Tuple{MakeTuple<T>(0, kSafeTupleChecker<T>)})))) 
                           && Magic(Getter<SafeTupleChecker<T>>{})) ? false : false> {
  static constexpr bool value = false;
};

} // namespace impl

template <typename T>
inline constexpr bool kForceEnableTuple = false;

template <typename T, std::size_t N>
inline constexpr bool kForceEnableTuple<std::array<T, N>> = true;


template <typename T>
concept TupleLike = kForceEnableTuple<std::remove_cvref_t<T>> || (requires{Get<0>(MakeTuple<T>(42));} && impl::IsSafeTuple<std::remove_cvref_t<T>>::value);


template <typename F, typename Tuple>
concept TupleTransformer = requires(F f, Tuple&& tuple) {
  {f(std::move(tuple))};
};

template <std::invocable F>
struct LazyTuple {
  F f;
  using ResultType = std::invoke_result_t<F>;
  std::optional<ResultType> result{std::nullopt};
  inline constexpr auto Evaluate() {
    if(!this->result) {
      this->result.emplace(this->f());
    };
  };
  inline constexpr auto operator()() -> decltype(auto) {
    this->Evaluate();
    return *this->result;
  };
  inline constexpr auto operator()() const -> decltype(auto) {
    return this->f();
  };
  template <typename T>
  inline constexpr auto operator==(T&& other) 
      requires requires(ResultType result){result == std::forward<T>(other);} {
    return (*this)() == other;
  };

  template <typename T>
  inline constexpr auto operator==(T&& other) const 
      requires requires(ResultType result){result == std::forward<T>(other);} {
    return (*this)() == other;
  };

  inline constexpr operator std::invoke_result_t<F>() {
    return (*this)();
  };
  inline constexpr operator std::invoke_result_t<F>() const {
    return (*this)();
  };
  template <std::size_t I>
  friend inline constexpr auto Get(LazyTuple&& tuple) -> decltype(auto) requires TupleLike<ResultType> {
    return Get<I>(std::move(tuple)());
  };
  template <std::size_t I>
  friend inline constexpr auto Get(const LazyTuple&& tuple) -> decltype(auto) requires TupleLike<ResultType> {
    return Get<I>(std::move(tuple)());
  };
  template <std::size_t I>
  friend inline constexpr auto Get(LazyTuple& tuple) -> decltype(auto) requires TupleLike<ResultType> {
    return Get<I>(tuple());
  };
  template <std::size_t I>
  friend inline constexpr auto Get(const LazyTuple& tuple) -> decltype(auto) requires TupleLike<ResultType> {
    return Get<I>(tuple());
  };
  template <typename FF>
  inline constexpr auto operator|(FF&& ff) {
    auto f = [ff = std::forward<FF>(ff), self = (*this)](){
      return ff(self());
    };
    return LazyTuple<decltype(f)>{std::move(f)};
  };
};
template <std::invocable F>
struct TupleMaker<LazyTuple<F>> {
  template <typename... Ts>
  static inline constexpr auto Make(Ts&&... args) 
      requires requires {TupleMaker<typename LazyTuple<F>::ResultType>::Make(std::forward<Ts>(args)...);} {
    return TupleMaker<typename LazyTuple<F>::ResultType>::Make(std::forward<Ts>(args)...);
  };
};


template <TupleLike Tuple, TupleTransformer<Tuple> FF>
inline constexpr auto operator|(Tuple&& tuple, FF&& f) {
  return LazyTuple{
    [tuple = std::forward<Tuple>(tuple), f = std::forward<FF>(f)]() -> decltype(auto) {
      return f(std::move(tuple));
    }};
};


template <TupleLike Tuple, typename F>
inline constexpr auto Unpack(Tuple&& tuple, F&& f) -> decltype(auto) {
  return [&]<std::size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
    return f(Get<Is>(std::forward<Tuple>(tuple))...);
  }(std::make_index_sequence<kTupleSize<Tuple>>());
};

template <typename F>
inline constexpr auto Unpack(F&& f) {
  return [f = std::forward<F>(f)]<TupleLike Tuple>(Tuple&& tuple){
    return Unpack(std::forward<Tuple>(tuple), std::move(f));
  };
};



template <TupleLike Tuple, typename R = Tuple, typename F>
inline constexpr auto Transform(Tuple&& container, F&& f, TypeList<R> = {}) {
  return Unpack(std::forward<Tuple>(container), 
                [&]<typename... Ts>(Ts&&... args){
                  return MakeTuple<R>(f(std::forward<Ts>(args))...);
                });
};

template <typename F, typename R = void>
inline constexpr auto Transform(F&& f, TypeList<R> result = {}) {
  return [f = std::forward<F>(f), result]<TupleLike Tuple>(Tuple&& tuple){
    if constexpr(!std::is_same_v<R, void>) {
      return Transform(std::forward<Tuple>(tuple), std::move(f), result);
    } else {
      return Transform(std::forward<Tuple>(tuple), std::move(f));
    };
  };
};


template <TupleLike Tuple, typename R = Tuple, typename F>
inline constexpr auto Map(Tuple&& tuple, F&& f, TypeList<R> result = {}) {
  return Transform(std::forward<Tuple>(tuple), std::forward<F>(f), result);
};

template <typename F, typename R = void>
inline constexpr auto Map(F&& f, TypeList<R> result = {}) {
  return [f = std::forward<F>(f), result]<TupleLike Tuple>(Tuple&& tuple){
    if constexpr(!std::is_same_v<R, void>) {
      return Map(std::forward<Tuple>(tuple), std::move(f), result);
    } else {
      return Map(std::forward<Tuple>(tuple), std::move(f));
    };
  };
};

template <TupleLike Tuple>
inline constexpr auto Reverse(Tuple&& tuple) {
  return [&]<auto... Is>(std::index_sequence<Is...>) {
    return MakeTuple<Tuple>(Get<kTupleSize<Tuple> - Is - 1>(std::forward<Tuple>(tuple))...);
  }(std::make_index_sequence<kTupleSize<Tuple>>());
};

consteval auto Reverse() {
  return []<TupleLike Tuple>(Tuple&& tuple){
    return Reverse(std::forward<Tuple>(tuple));
  };
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
  return Unpack(std::forward<Tuple>(tuple), [&]<typename... Ts>(Ts&&... args){
    return (
      impl::LeftFold<std::remove_cvref_t<T>, std::remove_cvref_t<F>>{.data = std::forward<T>(init), .f = std::forward<F>(f)} 
      | ...
      | impl::LeftFold<std::remove_cvref_t<Ts>>{.data = std::forward<Ts>(args)}
    ).data;
  });
};

template <TupleLike Tuple, typename T, typename F>
inline constexpr auto Reduce(Tuple&& tuple, T&& init, F&& f) {
  return LeftFold(std::forward<Tuple>(tuple), std::forward<T>(init), std::forward<F>(f));
};

template <typename T, typename F>
inline constexpr auto Reduce(T&& init, F&& f) {
  return [init = std::forward<T>(init), f = std::forward<F>(f)]<TupleLike Tuple>(Tuple&& tuple){
    return Reduce(std::forward<Tuple>(tuple), std::move(init), std::move(f));
  };
};


template <TupleLike Tuple, TupleLike Tuple2>
inline constexpr auto TupleCat(Tuple&& tuple, Tuple2&& tuple2) {
  return [&]<auto... Is, auto... IIs>(std::index_sequence<Is...>, std::index_sequence<IIs...>){
    return MakeTuple<Tuple>(Get<Is>(std::forward<Tuple>(tuple))..., Get<IIs>(std::forward<Tuple2>(tuple2))...);
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

template <TupleLike... Tuples, typename F>
inline constexpr auto Unpack(Tuples&&... tuples, F&& f) {
  return Unpack(TupleCat(std::forward<Tuples>(tuples)...), std::forward<F>(f));
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

template <typename T>
inline constexpr auto FirstOf(T&& init) {
  return [init = std::forward<T>(init)]<TupleLike Tuple>(Tuple&& tuple) {
    return FirstOf(std::forward<Tuple>(tuple), std::move(init));
  };
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

template <typename F>
inline constexpr auto Filter(F&& f) {
  return [f = std::forward<F>(f)]<TupleLike Tuple>(Tuple&& tuple){
    return Filter(std::forward<Tuple>(tuple), std::move(f));
  };
}; 

template <TupleLike Tuple>
inline constexpr auto ForEach(Tuple&& tuple, auto&& f) {
  Unpack(std::forward<Tuple>(tuple), [&]<typename... Ts>(Ts&&... args){
    (f(std::forward<Ts>(args)), ...);
  });
};


template <typename F, typename... Ts>
struct Curryer {
  F f;
  Tuple<Ts...> data;
  inline constexpr operator std::invoke_result_t<F, Ts...>() const {
    return [&]<auto... Is>(std::index_sequence<Is...>){
      return this->f(Get<Is>(this->data)...);
    }(std::index_sequence_for<Ts...>());
  };
  template <typename T>
  inline constexpr auto operator()(T&& arg) const -> Curryer<F, Ts..., std::remove_cvref_t<T>>{
    return {.f = this->f, .data = this->data + Tuple{std::forward<T>(arg)}};
  };
};
template <typename F>
inline constexpr auto Curry(F&& f) -> Curryer<std::remove_cvref_t<F>> {
  return {.f = std::forward<F>(f), .data = Tuple{}};
};

template <TupleLike Tuple, typename T>
inline constexpr auto Find(Tuple&& tuple, T&& find) -> std::size_t {
  return Unpack(std::forward<Tuple>(tuple), [&]<typename... Ts>(Ts&&... args) {
    using Type = std::remove_cvref_t<T>;
    std::array<bool, kTupleSize<Tuple>> bs{Overloaded(
      [&](const Type& element) {
        return element == find;
      },
      [&](const Type&& element) {
        return element == find;
      },
      [&](Type&& element) {
        return element == find;
      },
      [&](Type& element) {
        return element == find;
      },
      [](auto&&) {
        return false;
      }
    )(std::forward<Ts>(args))...};
    return std::ranges::find(bs, true) - bs.begin();
  });
};

template <std::size_t N, TupleLike Tuple>
inline constexpr auto Take(Tuple&& tuple) {
  if constexpr(std::is_lvalue_reference_v<Tuple> && HasMakeTie<Tuple>) {
    return [&]<std::size_t... Is>(std::index_sequence<Is...>){ 
      return MakeTie<Tuple>(Get<Is>(std::forward<Tuple>(tuple))...);
    }(std::make_index_sequence<N>());   
  } else {
    return [&]<std::size_t... Is>(std::index_sequence<Is...>){ 
      return MakeTuple<Tuple>(Get<Is>(std::forward<Tuple>(tuple))...);
    }(std::make_index_sequence<N>());   
  };
};

template <std::size_t N>
consteval auto Take() {
  return [&]<TupleLike Tuple>(Tuple&& tuple){
    return Take<N>(std::forward<Tuple>(tuple));
  };
};

template <TupleLike Tuple, typename T>
inline constexpr auto operator<<(Tuple&& tuple, T&& t) {
  return Unpack(std::forward<Tuple>(tuple), [&]<typename... Ts>(Ts&&... args){
    return MakeTuple<Tuple>(std::forward<Ts>(args)..., std::forward<T>(t));
  });
};

template <std::size_t N, TupleLike Tuple = Tuple<>, typename T>
inline constexpr auto Generate(T&& value) {
  return [&]<std::size_t... Is>(std::index_sequence<Is...>){
    return MakeTuple<Tuple>((std::ignore = Is, value)...);
  }(std::make_index_sequence<N>());
};


} // namespace utempl
