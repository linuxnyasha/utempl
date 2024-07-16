#pragma once
#include <fmt/format.h>

#include <optional>
#include <ranges>
#include <utempl/constexpr_string.hpp>
#include <utempl/overloaded.hpp>
#include <utempl/tuple.hpp>

namespace utempl {

template <auto Value>
struct Wrapper {
  static constexpr auto kValue = Value;
  inline constexpr auto operator==(auto&& arg) const {
    return arg == Value;
  };
  consteval operator decltype(Value)() const {  // NOLINT
    return Value;
  };
  consteval auto operator*() const {
    return Value;
  };
};

template <auto Value>
inline constexpr Wrapper<Value> kWrapper;

namespace impl {

template <std::size_t N>
struct kSeq {
  template <typename F>
  friend constexpr auto operator|(F&& f, const kSeq<N>&) {
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
      return std::forward<F>(f)(kWrapper<Is>...);
    }(std::make_index_sequence<N>());
  };
};

}  // namespace impl

template <std::size_t N>
inline constexpr impl::kSeq<N> kSeq;

template <ConstexprString string, typename T = std::size_t>
consteval auto ParseNumber() -> T {
  T response{};
  for(const auto& c : string) {
    if(c >= '0' && c <= '9') {
      response = response * 10 + (c - '0');  // NOLINT
    };
  };
  return response;
};
namespace literals {

template <char... cs>
consteval auto operator"" _c() {
  return Wrapper<ParseNumber<ConstexprString<sizeof...(cs)>({cs...})>()>{};
};

}  // namespace literals

template <std::size_t I, typename... Ts>
inline constexpr auto Arg(Ts&&... args) -> decltype(auto)
  requires(I < sizeof...(Ts))
{
  return [&]<auto... Is>(std::index_sequence<Is...>) -> decltype(auto) {
    return [](decltype(Caster(Is))..., auto&& response, ...) -> decltype(auto) {
      return response;
    }(std::forward<Ts>(args)...);
  }(std::make_index_sequence<I>());
};

template <std::size_t Count>
inline constexpr auto Times(auto&& f) {
  [&]<auto... Is>(std::index_sequence<Is...>) {
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
struct TupleMaker {};

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
    requires(std::same_as<std::remove_cvref_t<Arg>, std::remove_cvref_t<Args>> && ...)
  {
    return std::array{std::forward<Arg>(arg), std::forward<Args>(args)...};
  };
  static constexpr auto Make() -> std::array<T, 0> {
    return {};
  };
};

template <typename T>
struct TupleTieMaker {};

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

template <typename T = Tuple<>, typename... Args>
inline constexpr auto MakeTuple(Args&&... args) -> decltype(TupleMaker<std::remove_cvref_t<T>>::Make(std::forward<Args>(args)...)) {
  return TupleMaker<std::remove_cvref_t<T>>::Make(std::forward<Args>(args)...);
};

template <typename T = Tuple<>, typename... Args>
inline constexpr auto MakeTie(Args&... args) -> decltype(TupleTieMaker<std::remove_cvref_t<T>>::Make(args...)) {
  return TupleTieMaker<std::remove_cvref_t<T>>::Make(args...);
};

template <typename T>
concept HasMakeTie = requires(int& arg) { MakeTie<T>(arg); };

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
struct IsSafeTuple<
    T,
    bool(TrueF(Get<0>(std::move(Get<0>(Tuple{MakeTuple<T>(0, kSafeTupleChecker<T>)})))) && Magic(Getter<SafeTupleChecker<T>>{})) ? false
                                                                                                                                 : false> {
  static constexpr bool value = false;
};

}  // namespace impl

template <typename T>
inline constexpr bool kForceEnableTuple = false;

template <typename T, std::size_t N>
inline constexpr bool kForceEnableTuple<std::array<T, N>> = true;

template <typename T>
concept TupleLike = kForceEnableTuple<std::remove_cvref_t<T>> ||
                    (requires { Get<0>(MakeTuple<T>(42)); } && impl::IsSafeTuple<std::remove_cvref_t<T>>::value);  // NOLINT

template <typename F, typename Tuple>
concept TupleTransformer = requires(F f, Tuple&& tuple) {
  { f(std::move(tuple)) };
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
    requires requires(ResultType result) { result == std::forward<T>(other); }
  {
    return (*this)() == other;
  };

  template <typename T>
  inline constexpr auto operator==(T&& other) const
    requires requires(ResultType result) { result == std::forward<T>(other); }
  {
    return (*this)() == other;
  };

  inline constexpr operator std::invoke_result_t<F>() {  // NOLINT
    return (*this)();
  };
  inline constexpr operator std::invoke_result_t<F>() const {  // NOLINT
    return (*this)();
  };
  template <std::size_t I>
  friend inline constexpr auto Get(LazyTuple&& tuple) -> decltype(auto)
    requires TupleLike<ResultType>
  {
    return Get<I>(std::move(tuple)());
  };
  template <std::size_t I>
  friend inline constexpr auto Get(const LazyTuple&& tuple) -> decltype(auto)
    requires TupleLike<ResultType>
  {
    return Get<I>(std::move(tuple)());
  };
  template <std::size_t I>
  friend inline constexpr auto Get(LazyTuple& tuple) -> decltype(auto)
    requires TupleLike<ResultType>
  {
    return Get<I>(tuple());
  };
  template <std::size_t I>
  friend inline constexpr auto Get(const LazyTuple& tuple) -> decltype(auto)
    requires TupleLike<ResultType>
  {
    return Get<I>(tuple());
  };
  template <std::invocable<std::invoke_result_t<F>> FF>
  constexpr auto operator|(FF&& ff) {
    auto f = [ff = std::forward<FF>(ff), self = (*this)] {
      return ff(self());
    };
    return LazyTuple<decltype(f)>{std::move(f)};
  };
};
template <std::invocable F>
struct TupleMaker<LazyTuple<F>> {
  template <typename... Ts>
  static inline constexpr auto Make(Ts&&... args)
    requires requires { TupleMaker<typename LazyTuple<F>::ResultType>::Make(std::forward<Ts>(args)...); }
  {
    return TupleMaker<typename LazyTuple<F>::ResultType>::Make(std::forward<Ts>(args)...);
  };
};

template <TupleLike Tuple, TupleTransformer<Tuple> FF>
inline constexpr auto operator|(Tuple&& tuple, FF&& f) {
  return LazyTuple{[tuple = std::forward<Tuple>(tuple), f = std::forward<FF>(f)]() -> decltype(auto) {
    return f(std::move(tuple));
  }};
};

namespace impl {

template <typename F, typename Tuple>
concept UnpackConcept = []<std::size_t... Is>(std::index_sequence<Is...>) {
  return std::invocable<F, decltype(Get<Is>(std::declval<Tuple>()))...>;
}(std::make_index_sequence<kTupleSize<Tuple>>());

template <typename F, typename Tuple>
concept TransformConcept = []<std::size_t... Is>(std::index_sequence<Is...>) {
  return ([] {
    if constexpr(std::invocable<F, decltype(Get<Is>(std::declval<Tuple>()))>) {
      return !std::same_as<std::invoke_result_t<F, decltype(Get<Is>(std::declval<Tuple>()))>, void>;
    };
    return false;
  }() && ...);
}(std::make_index_sequence<kTupleSize<Tuple>>());

template <typename F, typename Tuple>
concept ForEachConcept = []<std::size_t... Is>(std::index_sequence<Is...>) {
  return (std::invocable<F, decltype(Get<Is>(std::declval<Tuple>()))> && ...);
}(std::make_index_sequence<kTupleSize<Tuple>>());

}  // namespace impl

template <TupleLike Tuple, impl::UnpackConcept<Tuple> F>
inline constexpr auto Unpack(Tuple&& tuple, F&& f) -> decltype(auto) {
  return [&](auto... is) -> decltype(auto) {
    return f(Get<is>(std::forward<Tuple>(tuple))...);
  } | kSeq<kTupleSize<Tuple>>;
};

template <typename F>
constexpr auto Unpack(F&& f) {
  return [f = std::forward<F>(f)]<TupleLike Tuple>(Tuple&& tuple) -> decltype(auto)
           requires impl::UnpackConcept<F, Tuple>
  {
    return Unpack(std::forward<Tuple>(tuple), std::move(f));
  };
};

template <TupleLike Tuple, TupleLike R = Tuple, impl::TransformConcept<Tuple> F>
constexpr auto Transform(Tuple&& container, F&& f, TypeList<R> = {}) {
  return Unpack(std::forward<Tuple>(container), [&]<typename... Ts>(Ts&&... args) {
    return MakeTuple<R>(f(std::forward<Ts>(args))...);
  });
};

template <typename F, typename R = void>
constexpr auto Transform(F&& f, TypeList<R> result = {}) {
  return [f = std::forward<F>(f), result]<TupleLike TTuple, typename RR = decltype([] {
                                                              if constexpr(std::same_as<R, void>) {
                                                                return kType<TTuple>;
                                                              } else {
                                                                return kType<R>;
                                                              };
                                                            }())::Type>(TTuple&& tuple)
    requires impl::TransformConcept<F, TTuple>
  {
    return Transform(std::forward<TTuple>(tuple), std::move(f), kType<RR>);
  };
};

template <TupleLike Tuple, TupleLike R = Tuple, impl::TransformConcept<Tuple> F>
constexpr auto Map(Tuple&& tuple, F&& f, TypeList<R> result = {}) {
  return Transform(std::forward<Tuple>(tuple), std::forward<F>(f), result);
};

template <typename F, typename R = void>
constexpr auto Map(F&& f, TypeList<R> result = {}) -> decltype(Transform(std::forward<F>(f), result)) {
  return Transform(std::forward<F>(f), result);
};

template <auto Tuple, TupleLike To = decltype(Tuple)>
consteval auto PackConstexprWrapper()
  requires TupleLike<decltype(Tuple)>
{
  return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    return MakeTuple<To>(kWrapper<Get<Is>(Tuple)>...);
  }(std::make_index_sequence<kTupleSize<decltype(Tuple)>>());
};

template <TupleLike Tuple>
constexpr auto Reverse(Tuple&& tuple) {
  return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    return MakeTuple<Tuple>(Get<kTupleSize<Tuple> - Is - 1>(std::forward<Tuple>(tuple))...);
  }(std::make_index_sequence<kTupleSize<Tuple>>());
};

consteval auto Reverse() {
  return []<TupleLike Tuple>(Tuple&& tuple) {
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
  F f;
  template <typename TT>
  constexpr auto operator|(LeftFold<TT>&& other) {
    using R = decltype(f(std::move(this->data), std::move(other.data)));
    return LeftFold<R, F>{.data = f(std::move(this->data), std::move(other.data)), .f = this->f};
  };
};

template <typename F, typename T, typename Tuple>
concept LeftFoldConcept = decltype(Unpack(std::declval<Tuple>(), []<typename... Ts>(Ts&&...) {
  return kWrapper<([] {
    if constexpr(std::invocable<F, T, Ts>) {
      return std::convertible_to<std::invoke_result_t<F, T, Ts>, T>;
    };
    return false;
  }() && ...)>;
}))::kValue;

}  // namespace impl

template <TupleLike Tuple, std::move_constructible T, impl::LeftFoldConcept<T, Tuple> F>
constexpr auto LeftFold(Tuple&& tuple, T&& init, F&& f) {
  return Unpack(std::forward<Tuple>(tuple), [&]<typename... Ts>(Ts&&... args) {
    return (impl::LeftFold<std::remove_cvref_t<T>, F>{.data = std::forward<T>(init), .f = std::forward<F>(f)} | ... |
            impl::LeftFold<std::remove_cvref_t<Ts>>{.data = std::forward<Ts>(args)})
        .data;
  });
};

template <TupleLike Tuple, std::move_constructible T, impl::LeftFoldConcept<T, Tuple> F>
constexpr auto Reduce(Tuple&& tuple, T&& init, F&& f) {
  return LeftFold(std::forward<Tuple>(tuple), std::forward<T>(init), std::forward<F>(f));
};

template <typename T, typename F>
constexpr auto Reduce(T&& init, F&& f) {
  return [init = std::forward<T>(init), f = std::forward<F>(f)]<TupleLike Tuple>(Tuple&& tuple)
    requires impl::LeftFoldConcept<F, T, Tuple>
  {
    return Reduce(std::forward<Tuple>(tuple), std::move(init), std::move(f));
  };
};

template <TupleLike Tuple, TupleLike Tuple2>
inline constexpr auto TupleCat(Tuple&& tuple, Tuple2&& tuple2) {
  return [&]<auto... Is, auto... IIs>(std::index_sequence<Is...>, std::index_sequence<IIs...>) {
    return MakeTuple<Tuple>(Get<Is>(std::forward<Tuple>(tuple))..., Get<IIs>(std::forward<Tuple2>(tuple2))...);
  }(std::make_index_sequence<kTupleSize<Tuple>>(), std::make_index_sequence<kTupleSize<Tuple2>>());
};

template <TupleLike... Tuples>
inline constexpr auto TupleCat(Tuples&&... tuples)
  requires(sizeof...(tuples) >= 1)
{
  return LeftFold(Tuple{std::forward<Tuples>(tuples)...},
                  MakeTuple<decltype(Arg<0>(std::forward<Tuples>(tuples)...))>(),
                  []<TupleLike Tup, TupleLike Tup2>(Tup&& tup, Tup2&& tup2) {
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
inline constexpr bool kEveryElement = [](auto... is) {
  return (F<std::decay_t<decltype(Get<is>(std::declval<Tuple>()))>>::value && ...);
} | kSeq<kTupleSize<Tuple>>;

template <template <typename...> typename F, typename... Ts>
struct PartialCaller {
  template <typename... TTs>
  using type = F<Ts..., TTs...>;
};
template <template <typename...> typename F, typename... Ts>
consteval auto PartialCallerF(TypeList<Ts...>) {
  return []<typename... TTs>(TypeList<TTs...>) {
    return F<Ts..., TTs...>{};
  };
};

template <TupleLike Tuple, std::move_constructible T>
inline constexpr auto FirstOf(Tuple&& tuple, T&& init)
  requires kEveryElement<std::is_invocable, Tuple>
{
  return LeftFold(std::forward<Tuple>(tuple), std::forward<T>(init), []<typename TT, typename F>(TT&& value, F&& f) -> TT {
    if(value) {
      return std::forward<TT>(value);
    };
    return std::forward<F>(f)();
  });
};

template <typename T>
inline constexpr auto FirstOf(T&& init) {
  return [init = std::forward<T>(init)]<TupleLike Tuple>(Tuple&& tuple) {
    return FirstOf(std::forward<Tuple>(tuple), std::move(init));
  };
};

namespace impl {

template <typename F, typename Tuple>
concept FilterConcept = decltype(Unpack(std::declval<Tuple>(), []<typename... Ts>(Ts&&...) {
  return kWrapper<([] {
    if constexpr(std::invocable<F, Ts>) {
      return std::convertible_to<std::invoke_result_t<F, Ts>, bool>;
    };
  }() && ...)>;
}))::kValue;

}  // namespace impl

template <TupleLike Tuple, impl::FilterConcept<Tuple> F>
constexpr auto Filter(Tuple&& tuple, F&& f) {
  return LeftFold(
      std::forward<Tuple>(tuple), MakeTuple<Tuple>(), [&]<TupleLike Accumulator, typename T>(Accumulator&& accumulator, T&& add) {
        if constexpr(decltype(f(std::forward<T>(add))){}) {
          return TupleCat(std::forward<Accumulator>(accumulator), std::forward<T>(add));
        } else {
          return accumulator;
        };
      });
};

template <typename F>
constexpr auto Filter(F&& f) {
  return [f = std::forward<F>(f)]<TupleLike Tuple>(Tuple&& tuple)
    requires impl::FilterConcept<F, Tuple>
  {
    return Filter(std::forward<Tuple>(tuple), std::move(f));
  };
};

template <TupleLike Tuple, impl::ForEachConcept<Tuple> F>
constexpr auto ForEach(Tuple&& tuple, F&& f) {
  Unpack(std::forward<Tuple>(tuple), [&]<typename... Ts>(Ts&&... args) {
    (f(std::forward<Ts>(args)), ...);
  });
};

template <typename F, typename... Ts>
struct Curryer {
  F f;
  Tuple<Ts...> data;
  inline constexpr operator std::invoke_result_t<F, Ts...>() const {  // NOLINT
    return [&]<auto... Is>(std::index_sequence<Is...>) {
      return this->f(Get<Is>(this->data)...);
    }(std::index_sequence_for<Ts...>());
  };
  template <typename T>
  inline constexpr auto operator()(T&& arg) const -> Curryer<F, Ts..., std::remove_cvref_t<T>> {
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
        })(std::forward<Ts>(args))...};
    return std::ranges::find(bs, true) - bs.begin();
  });
};

template <std::size_t N, TupleLike Tuple>
inline constexpr auto Take(Tuple&& tuple) {
  if constexpr(std::is_lvalue_reference_v<Tuple> && HasMakeTie<Tuple>) {
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
      return MakeTie<Tuple>(Get<Is>(std::forward<Tuple>(tuple))...);
    }(std::make_index_sequence<N>());
  } else {
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
      return MakeTuple<Tuple>(Get<Is>(std::forward<Tuple>(tuple))...);
    }(std::make_index_sequence<N>());
  };
};

template <std::size_t N>
consteval auto Take() {
  return [&]<TupleLike Tuple>(Tuple&& tuple) {
    return Take<N>(std::forward<Tuple>(tuple));
  };
};

template <TupleLike Tuple, std::move_constructible T>
inline constexpr auto operator<<(Tuple&& tuple, T&& t) {
  return Unpack(std::forward<Tuple>(tuple), [&]<typename... Ts>(Ts&&... args) {
    return MakeTuple<Tuple>(std::forward<Ts>(args)..., std::forward<T>(t));
  });
};

template <std::size_t N, TupleLike Tuple = Tuple<>, typename T>
inline constexpr auto Generate(T&& value)
  requires(std::copyable<T> || N == 1 && std::move_constructible<T>)
{
  if constexpr(N == 1) {
    return MakeTuple<Tuple>(std::forward<T>(value));
  } else {
    return [&](auto... is) {
      return MakeTuple<Tuple>((std::ignore = is, value)...);
    } | kSeq<N>;
  };
};

template <TupleLike Tuple>
constexpr auto Enumerate(Tuple&& tuple) {
  return Unpack(std::forward<Tuple>(tuple), [](auto&&... vs) {
    return [&](auto... is) {
      return MakeTuple<Tuple>(std::pair{*is, std::forward<decltype(vs)>(vs)}...);
    } | kSeq<sizeof...(vs)>;
  });
};

template <typename T>
  requires std::same_as<T, void> || requires { T{}; }
constexpr auto kDefaultCreator = [] {
  return T{};
};

template <>
constexpr auto kDefaultCreator<void> = [] {};

namespace impl {

template <typename Key, typename KeysTuple>
concept ComparableSwitchConcept = decltype(Unpack(std::declval<KeysTuple>(), []<typename... Ts>(Ts&&... args) {
  return kWrapper<(requires { args == std::declval<Key>(); } && ...)>;
}))::kValue;

template <typename F, typename ValuesTuple, typename R>
concept CallableSwitchConcept = std::same_as<R, void> || decltype(Unpack(std::declval<ValuesTuple>(), []<typename... Ts>(Ts&&...) {
                                  return kWrapper<([] {
                                    if constexpr(std::invocable<F, Ts>) {
                                      if constexpr(!std::same_as<std::invoke_result_t<F, Ts>, void>) {
                                        return std::convertible_to<std::invoke_result_t<F, Ts>, std::optional<R>>;
                                      };
                                    };
                                    return false;
                                  }() && ...)>;
                                }))::kValue;

template <typename F, typename R>
concept DefaultSwitchConcept = [] {
  if constexpr(std::invocable<F>) {
    if constexpr(!std::same_as<R, void>) {
      return std::convertible_to<std::invoke_result_t<F>, std::optional<R>>;
    };
    return true;
  };
  return false;
}();

}  // namespace impl

template <typename R = void,
          TupleLike KeysTuple,
          TupleLike ValuesTuple,
          impl::ComparableSwitchConcept<KeysTuple> Key,
          impl::CallableSwitchConcept<ValuesTuple, R> F,
          impl::DefaultSwitchConcept<R> Default = decltype(kDefaultCreator<R>)>
constexpr auto Switch(KeysTuple&& keysTuple, ValuesTuple&& valuesTuple, Key&& key, F&& f, Default&& def = {}) -> R
  requires(std::move_constructible<R> || std::same_as<R, void>) && (kTupleSize<KeysTuple> == kTupleSize<ValuesTuple>)
{
  return Unpack(std::forward<KeysTuple>(keysTuple), [&]<typename... Keys>(Keys&&... keys) {
    return Unpack(std::forward<ValuesTuple>(valuesTuple), [&]<typename... Values>(Values&&... values) {
      if constexpr(std::same_as<R, void>) {
        FirstOf(Tuple{[&] {
                  return std::forward<Keys>(keys) == std::forward<Key>(key) ? (std::forward<F>(f)(std::forward<Values>(values)), true)
                                                                            : false;
                }...},
                false)
            ? void(std::ignore)
            : [&] {
                if constexpr(!std::same_as<Default, decltype(kDefaultCreator<R>)>) {
                  std::forward<Default>(def)();
                };
              }();
      } else {
        return *FirstOf(Tuple{[&] {
                          return std::forward<Keys>(keys) == std::forward<Key>(key) ? std::forward<F>(f)(std::forward<Values>(values))
                                                                                    : std::optional<R>{};
                        }...},
                        std::optional<R>{})
                    .or_else([&] -> std::optional<R> {
                      return std::forward<Default>(def)();
                    });
      };
    });
  });
};

template <std::size_t N, typename R = Tuple<>>
consteval auto GetIndexesTuple() {
  return [](auto... is) {
    return MakeTuple<R>(*is...);
  } | kSeq<N>;
};
}  // namespace utempl
