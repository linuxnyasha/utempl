export module utempl.utils;

import std;
import fmt;
import utempl.string;
import utempl.tuple;
import utempl.type_list;
import utempl.overloaded;

namespace utempl {

template <typename T>
using ForwardType = decltype(std::forward<T>(std::declval<T>()));

export template <auto Value>
constexpr Wrapper<Value> kWrapper;

export template <typename T>
  requires std::same_as<T, void> || requires { T{}; }
constexpr auto kDefaultCreator = [] {
  return T{};
};

export template <>
constexpr auto kDefaultCreator<void> = [] {};

template <std::size_t N>
struct kSeqType {
  template <typename F>
  friend constexpr auto operator|(F&& f, const kSeqType<N>&) -> decltype(auto) {
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
      return std::forward<F>(f)(kWrapper<Is>...);
    }(std::make_index_sequence<N>());
  };
};

template <typename From, typename To>
concept ImplicitConvertibleTo = std::same_as<From, To> || requires(From from) { [](To) {}(from); };

template <typename F, typename Sig>
concept Function = []<typename R, typename... Ts>(TypeList<R(Ts...)>) {
  if constexpr(std::invocable<F, Ts...>) {
    return std::same_as<R, void> || ImplicitConvertibleTo<std::invoke_result_t<F, Ts...>, R>;
  };
  return false;
}(kType<Sig>);

static_assert(Function<decltype([]() {}), void()>);

export template <std::size_t N>
constexpr kSeqType<N> kSeq;

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

export template <char... cs>
consteval auto operator"" _c() {
  return Wrapper<ParseNumber<ConstexprString<sizeof...(cs)>({cs...})>()>{};
};

}  // namespace literals

export template <std::size_t I, typename... Ts>
constexpr auto Arg(Ts&&... args) -> decltype(auto)
  requires(I < sizeof...(Ts))
{
  return [&]<auto... Is>(std::index_sequence<Is...>) -> decltype(auto) {
    return [](decltype(Caster(Is))..., auto&& response, ...) -> decltype(auto) {
      return response;
    }(std::forward<Ts>(args)...);
  }(std::make_index_sequence<I>());
};

export template <std::size_t Count>
constexpr auto Times(auto&& f) {
  [&]<auto... Is>(std::index_sequence<Is...>) {
    (Arg<0>(f, Is)(), ...);
  }(std::make_index_sequence<Count>());
};

export template <typename T>
constexpr std::size_t kTupleSize = []() -> std::size_t {
  static_assert(!sizeof(T), "Not Found");
  return 0;
}();

export template <typename T>
constexpr std::size_t kTupleSize<T&&> = kTupleSize<std::remove_reference_t<T>>;

export template <typename T>
constexpr std::size_t kTupleSize<T&> = kTupleSize<std::remove_reference_t<T>>;

export template <typename T>
constexpr std::size_t kTupleSize<const T> = kTupleSize<std::remove_cv_t<T>>;

export template <template <typename...> typename M, typename... Ts>
constexpr std::size_t kTupleSize<M<Ts...>> = sizeof...(Ts);

export template <template <typename, std::size_t> typename Array, typename T, std::size_t N>
constexpr std::size_t kTupleSize<Array<T, N>> = N;

export template <typename T>
struct TupleMaker {};

export template <typename... Ts>
struct TupleMaker<std::tuple<Ts...>> {
  template <typename... Args>
  static constexpr auto Make(Args&&... args) {
    return std::tuple{std::forward<Args>(args)...};
  };
};
export template <typename... Ts>
struct TupleMaker<Tuple<Ts...>> {
  template <typename... Args>
  static constexpr auto Make(Args&&... args) {
    return Tuple{std::forward<Args>(args)...};
  };
};
export template <typename T, std::size_t N>
struct TupleMaker<std::array<T, N>> {
  template <typename Arg, typename... Args>
  static constexpr auto Make(Arg&& arg, Args&&... args)
    requires(std::same_as<std::remove_cvref_t<Arg>, std::remove_cvref_t<Args>> && ...)
  {
    return std::array{std::forward<Arg>(arg), std::forward<Args>(args)...};
  };
  static constexpr auto Make() -> std::array<T, 0> {
    return {};
  };
};

export template <typename T>
struct TupleTieMaker {};

export template <typename... Ts>
struct TupleTieMaker<std::tuple<Ts...>> {
  template <typename... Args>
  static constexpr auto Make(Args&... args) -> std::tuple<Args...> {
    return {args...};
  };
};

export template <typename... Ts>
struct TupleTieMaker<Tuple<Ts...>> {
  template <typename... Args>
  static constexpr auto Make(Args&... args) -> Tuple<Args...> {
    return {args...};
  };
};

export template <typename T = Tuple<>, typename... Args>
constexpr auto MakeTuple(Args&&... args) -> decltype(TupleMaker<std::remove_cvref_t<T>>::Make(std::forward<Args>(args)...)) {
  return TupleMaker<std::remove_cvref_t<T>>::Make(std::forward<Args>(args)...);
};

export template <typename T = Tuple<>, typename... Args>
constexpr auto MakeTie(Args&... args) -> decltype(TupleTieMaker<std::remove_cvref_t<T>>::Make(args...)) {
  return TupleTieMaker<std::remove_cvref_t<T>>::Make(args...);
};

template <typename T>
concept HasMakeTie = requires(int& arg) { MakeTie<T>(arg); };

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
constexpr const SafeTupleChecker<T> kSafeTupleChecker;

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

export template <typename T>
constexpr bool kForceEnableTuple = false;

export template <typename T, std::size_t N>
constexpr bool kForceEnableTuple<std::array<T, N>> = true;

export template <typename T>
concept TupleLike = kForceEnableTuple<std::remove_cvref_t<T>> ||
                    (requires { Get<0>(MakeTuple<T>(42)); } && IsSafeTuple<std::remove_cvref_t<T>>::value);  // NOLINT

export template <typename F, typename Tuple>
concept TupleTransformer = requires(F f, Tuple&& tuple) {
  { f(std::move(tuple)) };
};

template <std::invocable F>
struct LazyTuple {
  F f;
  using ResultType = std::invoke_result_t<F>;
  std::optional<ResultType> result{std::nullopt};
  constexpr auto Evaluate() {
    if(!this->result) {
      this->result.emplace(this->f());
    };
  };
  constexpr auto operator()() -> decltype(auto) {
    this->Evaluate();
    return *this->result;
  };
  constexpr auto operator()() const -> decltype(auto) {
    return this->f();
  };
  template <typename T>
  constexpr auto operator==(T&& other)
    requires requires(ResultType result) { result == std::forward<T>(other); }
  {
    return (*this)() == other;
  };

  template <typename T>
  constexpr auto operator==(T&& other) const
    requires requires(ResultType result) { result == std::forward<T>(other); }
  {
    return (*this)() == other;
  };

  constexpr operator std::invoke_result_t<F>() {  // NOLINT
    return (*this)();
  };
  constexpr operator std::invoke_result_t<F>() const {  // NOLINT
    return (*this)();
  };
  template <std::size_t I>
  friend constexpr auto Get(LazyTuple&& tuple) -> decltype(auto)
    requires TupleLike<ResultType>
  {
    return Get<I>(std::move(tuple)());
  };
  template <std::size_t I>
  friend constexpr auto Get(const LazyTuple&& tuple) -> decltype(auto)
    requires TupleLike<ResultType>
  {
    return Get<I>(std::move(tuple)());
  };
  template <std::size_t I>
  friend constexpr auto Get(LazyTuple& tuple) -> decltype(auto)
    requires TupleLike<ResultType>
  {
    return Get<I>(tuple());
  };
  template <std::size_t I>
  friend constexpr auto Get(const LazyTuple& tuple) -> decltype(auto)
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
  static constexpr auto Make(Ts&&... args)
    requires requires { TupleMaker<typename LazyTuple<F>::ResultType>::Make(std::forward<Ts>(args)...); }
  {
    return TupleMaker<typename LazyTuple<F>::ResultType>::Make(std::forward<Ts>(args)...);
  };
};

export template <TupleLike Tuple, TupleTransformer<Tuple> FF>
constexpr auto operator|(Tuple&& tuple, FF&& f) {
  return LazyTuple{[tuple = std::forward<Tuple>(tuple), f = std::forward<FF>(f)]() -> decltype(auto) {
    return f(std::move(tuple));
  }};
};

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

export template <TupleLike Tuple, UnpackConcept<Tuple> F>
constexpr auto Unpack(Tuple&& tuple, F&& f) -> decltype(auto) {
  return [&](auto... is) -> decltype(auto) {
    return f(Get<is>(std::forward<Tuple>(tuple))...);
  } | kSeq<kTupleSize<Tuple>>;
};

export template <typename F>
constexpr auto Unpack(F&& f) {
  return [f = std::forward<F>(f)]<TupleLike Tuple>(Tuple&& tuple) -> decltype(auto)
           requires UnpackConcept<F, Tuple>
  {
    return Unpack(std::forward<Tuple>(tuple), std::move(f));
  };
};

export template <TupleLike Tuple, TupleLike R = Tuple, TransformConcept<Tuple> F>
constexpr auto Transform(Tuple&& container, F&& f, TypeList<R> = {}) {
  return Unpack(std::forward<Tuple>(container), [&]<typename... Ts>(Ts&&... args) {
    return MakeTuple<R>(f(std::forward<Ts>(args))...);
  });
};

export template <typename F, typename R = void>
constexpr auto Transform(F&& f, TypeList<R> result = {}) {
  return [f = std::forward<F>(f), result]<TupleLike TTuple, typename RR = decltype([] {
                                                              if constexpr(std::same_as<R, void>) {
                                                                return kType<TTuple>;
                                                              } else {
                                                                return kType<R>;
                                                              };
                                                            }())::Type>(TTuple&& tuple)
    requires TransformConcept<F, TTuple>
  {
    return Transform(std::forward<TTuple>(tuple), std::move(f), kType<RR>);
  };
};

export template <TupleLike Tuple, TupleLike R = Tuple, TransformConcept<Tuple> F>
constexpr auto Map(Tuple&& tuple, F&& f, TypeList<R> result = {}) {
  return Transform(std::forward<Tuple>(tuple), std::forward<F>(f), result);
};

export template <typename F, typename R = void>
constexpr auto Map(F&& f, TypeList<R> result = {}) -> decltype(Transform(std::forward<F>(f), result)) {
  return Transform(std::forward<F>(f), result);
};

export template <auto Tuple, TupleLike To = decltype(Tuple)>
consteval auto PackConstexprWrapper()
  requires TupleLike<decltype(Tuple)>
{
  return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    return MakeTuple<To>(kWrapper<Get<Is>(Tuple)>...);
  }(std::make_index_sequence<kTupleSize<decltype(Tuple)>>());
};

export template <TupleLike Tuple>
constexpr auto Reverse(Tuple&& tuple) {
  return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    return MakeTuple<Tuple>(Get<kTupleSize<Tuple> - Is - 1>(std::forward<Tuple>(tuple))...);
  }(std::make_index_sequence<kTupleSize<Tuple>>());
};

export consteval auto Reverse() {
  return []<TupleLike Tuple>(Tuple&& tuple) {
    return Reverse(std::forward<Tuple>(tuple));
  };
};

template <typename...>
struct LeftFoldHelper;

template <typename T>
struct LeftFoldHelper<T> {
  T data;
};

template <typename T, typename F>
struct LeftFoldHelper<T, F> {
  T data;
  F f;
  template <typename TT>
  constexpr auto operator|(LeftFoldHelper<TT>&& other) -> LeftFoldHelper<std::invoke_result_t<F, T, TT>, F> {
    return {.data = std::forward<F>(this->f)(std::forward<T>(this->data), std::forward<TT>(other.data)), .f = std::forward<F>(this->f)};
  };
};

struct LeftFoldIgnorer {
  static constexpr bool value = false;
  consteval auto operator|(auto&&) const -> LeftFoldIgnorer {
    return {};
  };
};

template <typename F, typename T>
struct LeftFoldIsOk {
  static constexpr bool value = true;
  template <typename TT>
  consteval auto operator|(LeftFoldIsOk<F, TT>&& other) -> LeftFoldIsOk<F, std::invoke_result_t<F, T, TT>> {
    return {};
  };
  consteval auto operator|(auto&&) -> LeftFoldIgnorer {
    return {};
  };
};

template <typename F, typename T, typename Tuple>
concept LeftFoldConcept = decltype(Unpack(std::declval<Tuple>(), []<typename... Ts>(Ts&&...) {
  return kWrapper<decltype((LeftFoldIsOk<F, T>{} | ... | LeftFoldIsOk<F, Ts>{}))::value>;
}))::kValue;

export template <TupleLike Tuple, std::move_constructible T, LeftFoldConcept<T, Tuple> F = decltype(kDefaultCreator<void>)&>
constexpr auto LeftFold(Tuple&& tuple, T&& init, F&& f = kDefaultCreator<void>) -> decltype(auto) {
  return Unpack(std::forward<Tuple>(tuple), [&]<typename... Ts>(Ts&&... args) -> decltype(auto) {
    return (LeftFoldHelper<ForwardType<T>, ForwardType<F>>{.data = std::forward<T>(init), .f = std::forward<F>(f)} | ... |
            LeftFoldHelper<ForwardType<Ts>>{.data = std::forward<Ts>(args)})
        .data;
  });
};

export template <TupleLike Tuple, std::move_constructible T, LeftFoldConcept<T, Tuple> F>
constexpr auto Reduce(Tuple&& tuple, T&& init, F&& f) -> decltype(auto) {
  return LeftFold(std::forward<Tuple>(tuple), std::forward<T>(init), std::forward<F>(f));
};

export template <typename T, typename F>
constexpr auto Reduce(T&& init, F&& f) -> decltype(auto) {
  return [init = std::forward<T>(init), f = std::forward<F>(f)]<TupleLike Tuple>(Tuple&& tuple) -> decltype(auto)
           requires LeftFoldConcept<F, T, Tuple>
  {
    return Reduce(std::forward<Tuple>(tuple), std::move(init), std::move(f));
  };
};

export template <TupleLike Tuple, TupleLike Tuple2>
constexpr auto TupleCat(Tuple&& tuple, Tuple2&& tuple2) {
  return [&]<auto... Is, auto... IIs>(std::index_sequence<Is...>, std::index_sequence<IIs...>) {
    return MakeTuple<Tuple>(Get<Is>(std::forward<Tuple>(tuple))..., Get<IIs>(std::forward<Tuple2>(tuple2))...);
  }(std::make_index_sequence<kTupleSize<Tuple>>(), std::make_index_sequence<kTupleSize<Tuple2>>());
};

export template <TupleLike... Tuples>
constexpr auto TupleCat(Tuples&&... tuples)
  requires(sizeof...(tuples) >= 1)
{
  return LeftFold(Tuple{std::forward<Tuples>(tuples)...},
                  MakeTuple<decltype(Arg<0>(std::forward<Tuples>(tuples)...))>(),
                  []<TupleLike Tup, TupleLike Tup2>(Tup&& tup, Tup2&& tup2) {
                    return TupleCat(std::forward<Tup>(tup), std::forward<Tup2>(tup2));
                  });
};

export template <TupleLike... Tuples, typename F>
constexpr auto Unpack(Tuples&&... tuples, F&& f) -> decltype(Unpack(TupleCat(std::forward<Tuples>(tuples)...), std::forward<F>(f))) {
  return Unpack(TupleCat(std::forward<Tuples>(tuples)...), std::forward<F>(f));
};

export template <typename... Ts>
constexpr auto Tie(Ts&... args) -> Tuple<Ts&...> {
  return {args...};
};

template <template <typename...> typename F, TupleLike Tuple>
constexpr bool kEveryElement = [](auto... is) {
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

export template <TupleLike Tuple, std::move_constructible T>
constexpr auto FirstOf(Tuple&& tuple, T&& init)
  requires kEveryElement<std::is_invocable, Tuple>
{
  return LeftFold(std::forward<Tuple>(tuple), std::forward<T>(init), []<typename TT, typename F>(TT&& value, F&& f) -> TT {
    if(value) {
      return std::forward<TT>(value);
    };
    return std::forward<F>(f)();
  });
};

export template <typename T>
constexpr auto FirstOf(T&& init) {
  return [init = std::forward<T>(init)]<TupleLike Tuple>(Tuple&& tuple) {
    return FirstOf(std::forward<Tuple>(tuple), std::move(init));
  };
};

template <typename F, typename Tuple>
concept FilterConcept = decltype(Unpack(std::declval<Tuple>(), []<typename... Ts>(Ts&&...) {
  return kWrapper<(Function<F, bool(Ts)> && ...)>;
}))::kValue;

export template <TupleLike Tuple, FilterConcept<Tuple> F>
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

export template <typename F>
constexpr auto Filter(F&& f) {
  return [f = std::forward<F>(f)]<TupleLike Tuple>(Tuple&& tuple)
    requires FilterConcept<F, Tuple>
  {
    return Filter(std::forward<Tuple>(tuple), std::move(f));
  };
};

export template <TupleLike Tuple, ForEachConcept<Tuple> F>
constexpr auto ForEach(Tuple&& tuple, F&& f) {
  Unpack(std::forward<Tuple>(tuple), [&]<typename... Ts>(Ts&&... args) {
    (f(std::forward<Ts>(args)), ...);
  });
};

template <typename F, typename... Ts>
struct Curryer {
  F f;
  Tuple<Ts...> data;
  constexpr operator std::invoke_result_t<F, Ts...>() const {  // NOLINT
    return [&]<auto... Is>(std::index_sequence<Is...>) {
      return this->f(Get<Is>(this->data)...);
    }(std::index_sequence_for<Ts...>());
  };
  template <typename T>
  constexpr auto operator()(T&& arg) const -> Curryer<F, Ts..., std::remove_cvref_t<T>> {
    return {.f = this->f, .data = this->data + Tuple{std::forward<T>(arg)}};
  };
};
export template <typename F>
constexpr auto Curry(F&& f) -> Curryer<std::remove_cvref_t<F>> {
  return {.f = std::forward<F>(f), .data = Tuple{}};
};

export template <TupleLike Tuple, typename T>
constexpr auto Find(Tuple&& tuple, T&& find) -> std::size_t {
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

export template <std::size_t N, TupleLike Tuple>
constexpr auto Take(Tuple&& tuple) {
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

export template <std::size_t N>
consteval auto Take() {
  return [&]<TupleLike Tuple>(Tuple&& tuple) {
    return Take<N>(std::forward<Tuple>(tuple));
  };
};

export template <TupleLike Tuple, std::move_constructible T>
constexpr auto operator<<(Tuple&& tuple, T&& t) {
  return Unpack(std::forward<Tuple>(tuple), [&]<typename... Ts>(Ts&&... args) {
    return MakeTuple<Tuple>(std::forward<Ts>(args)..., std::forward<T>(t));
  });
};

export template <std::size_t N, TupleLike Tuple = Tuple<>, typename T>
constexpr auto Generate(T&& value)
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

export template <TupleLike Tuple>
constexpr auto Enumerate(Tuple&& tuple) {
  return Unpack(std::forward<Tuple>(tuple), [](auto&&... vs) {
    return [&](auto... is) {
      return MakeTuple<Tuple>(std::pair{*is, std::forward<decltype(vs)>(vs)}...);
    } | kSeq<sizeof...(vs)>;
  });
};

template <typename Key, typename KeysTuple>
concept ComparableSwitchConcept = decltype(Unpack(std::declval<KeysTuple>(), []<typename... Ts>(Ts&&... args) {
  return kWrapper<(requires { args == std::declval<Key>(); } && ...)>;
}))::kValue;

template <typename F, typename ValuesTuple, typename R>
concept CallableSwitchConcept = std::same_as<R, void> || decltype(Unpack(std::declval<ValuesTuple>(), []<typename... Ts>(Ts&&...) {
                                  return kWrapper<(Function<F, std::optional<R>(Ts)> && ...)>;
                                }))::kValue;

export template <typename R = void,
                 TupleLike KeysTuple,
                 TupleLike ValuesTuple,
                 ComparableSwitchConcept<KeysTuple> Key,
                 CallableSwitchConcept<ValuesTuple, R> F,
                 Function<R()> Default = std::add_lvalue_reference_t<decltype(kDefaultCreator<R>)>>
constexpr auto Switch(KeysTuple&& keysTuple, ValuesTuple&& valuesTuple, Key&& key, F&& f, Default&& def = kDefaultCreator<R>) -> R
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

export template <std::size_t N, typename R = Tuple<>>
consteval auto GetIndexesTuple() {
  return [](auto... is) {
    return MakeTuple<R>(*is...);
  } | kSeq<N>;
};

export template <typename R = Tuple<>, typename... Ts>
consteval auto GetTuple(TypeList<Ts...>, TypeList<R> = {}) {
  return MakeTuple<R>(kType<Ts>...);
};

}  // namespace utempl
