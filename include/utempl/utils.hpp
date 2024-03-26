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
  template <typename... Args>
  static inline constexpr auto Make(Args&&... args) {
    return std::array{std::forward<Args>(args)...};
  };
};



template <typename T = Tuple<>, typename... Args>
inline constexpr auto MakeTuple(Args&&... args) {
  return TupleMaker<std::remove_cvref_t<T>>::Make(std::forward<Args>(args)...);
};

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


template <TupleLike Tuple, typename R = Tuple, typename F>
inline constexpr auto Transform(Tuple&& container, F&& f, TypeList<R> = {}) {
  return [&]<auto... Is>(std::index_sequence<Is...>){
    return MakeTuple<R>(f(Get<Is>(std::forward<Tuple>(container)))...);
  }(std::make_index_sequence<kTupleSize<Tuple>>());
};

template <TupleLike Tuple, typename R = Tuple, typename F>
inline constexpr auto Map(Tuple&& tuple, F&& f, TypeList<R> result = {}) {
  return Transform(std::forward<Tuple>(tuple), std::forward<F>(f), result);
};


template <TupleLike Tuple>
inline constexpr auto Reverse(Tuple&& tuple) {
  return [&]<auto... Is>(std::index_sequence<Is...>) {
    return MakeTuple<Tuple>(Get<kTupleSize<Tuple> - Is - 1>(std::forward<Tuple>(tuple))...);
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
      | impl::LeftFold<std::remove_cvref_t<decltype(Get<Is>(std::forward<Tuple>(tuple)))>>{.data = Get<Is>(std::forward<Tuple>(tuple))}
    ).data;
  }(std::make_index_sequence<kTupleSize<Tuple>>());
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

template <TupleLike Tuple>
inline constexpr auto ForEach(Tuple&& tuple, auto&& f) {
  [&]<auto... Is>(std::index_sequence<Is...>){
    (f(Get<Is>(std::forward<Tuple>(tuple))), ...);
  }(std::make_index_sequence<kTupleSize<Tuple>>());
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
  return [&]<std::size_t... Is>(std::index_sequence<Is...>) -> std::size_t {
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
    )(Get<Is>(std::forward<Tuple>(tuple)))...};
    return std::ranges::find(bs, true) - bs.begin();
  }(std::make_index_sequence<kTupleSize<Tuple>>());
};
} // namespace utempl
