export module utempl.tuple;

import utempl.type_list;
import utempl.overloaded;
import std;

namespace utempl {

export template <auto Value>
struct Wrapper {
  static constexpr auto kValue = Value;
  static constexpr auto value = Value;
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

template <auto, typename T>
struct TupleLeaf {
  T value;
  constexpr auto operator==(const TupleLeaf&) const -> bool = default;
};

template <typename, typename...>
struct TupleHelper;

template <std::size_t... Is, typename... Ts>
struct TupleHelper<std::index_sequence<Is...>, Ts...> : public TupleLeaf<Is, Ts>... {
  constexpr auto operator==(const TupleHelper&) const -> bool = default;
};

export template <typename... Ts>
struct Tuple;

export template <std::size_t I, typename... Ts>
constexpr auto Get(Tuple<Ts...>& tuple) -> auto&
  requires(I < sizeof...(Ts))
{
  using Type = decltype(Get<I>(TypeList<Ts...>{}));
  return static_cast<TupleLeaf<I, Type>&>(tuple).value;
};

export template <std::size_t I, typename... Ts>
constexpr auto Get(const Tuple<Ts...>& tuple) -> const auto&
  requires(I < sizeof...(Ts))
{
  using Type = decltype(Get<I>(TypeList<Ts...>{}));
  return static_cast<const TupleLeaf<I, Type>&>(tuple).value;
};

export template <std::size_t I, typename... Ts>
constexpr auto Get(Tuple<Ts...>&& tuple) -> decltype(auto)
  requires(I < sizeof...(Ts))
{
  using Type = decltype(Get<I>(TypeList<Ts...>{}));
  return std::move(static_cast<TupleLeaf<I, Type>&&>(tuple).value);
};

export template <std::size_t I, typename T>
constexpr auto Get(T&& arg) -> decltype(get<I>(std::forward<T>(arg))) {
  return get<I>(std::forward<T>(arg));
};

export template <typename... Ts>
struct Tuple : TupleHelper<std::index_sequence_for<Ts...>, Ts...> {
  template <typename... TTs>
  constexpr Tuple(TTs&&... args) /* NOLINT */ : TupleHelper<std::index_sequence_for<Ts...>, Ts...>{{std::forward<TTs>(args)}...} {};
  template <std::invocable... Fs>
  constexpr Tuple(Fs&&... fs)  // NOLINT
    requires(!std::invocable<Ts> || ...)
      : TupleHelper<std::index_sequence_for<Ts...>, Ts...>{{fs()}...} {};
  constexpr Tuple(Ts&&... args) : TupleHelper<std::index_sequence_for<Ts...>, Ts...>{{args}...} {};  // NOLINT
  constexpr Tuple(const Tuple&) = default;
  constexpr Tuple(Tuple&&) = default;
  constexpr Tuple(Tuple&) = default;
  constexpr auto operator=(const Tuple&) -> Tuple& = default;
  constexpr auto operator=(Tuple&&) -> Tuple& = default;
  constexpr auto operator=(Tuple&) -> Tuple& = default;
  constexpr Tuple() : TupleHelper<std::index_sequence_for<Ts...>, Ts...>() {};
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

export template <>
struct Tuple<> {
  template <typename... Ts>
  constexpr auto operator+(const Tuple<Ts...>& other) const -> Tuple<Ts...> {
    return other;
  };
  constexpr auto operator==(const Tuple<>&) const {
    return true;
  };
};
export template <typename... Ts>
Tuple(Ts&&...) -> Tuple<std::decay_t<Ts>...>;

export template <typename... Ts>
consteval auto ListFromTuple(Tuple<Ts...>) -> TypeList<Ts...> {
  return {};
};

}  // namespace utempl
