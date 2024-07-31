export module utempl.type_list;

import std;
import utempl.overloaded;

namespace utempl {

export template <typename... Ts>
struct TypeList {};

export template <typename T>
struct TypeList<T> {
  using Type = T;
};

export template <typename T>
inline constexpr auto kType = TypeList<T>{};

export template <typename... Ts>
inline constexpr auto kTypeList = TypeList<Ts...>{};

export template <typename T>
concept IsTypeList = Overloaded(
    []<typename... Ts>(TypeList<TypeList<Ts...>>) {
      return true;
    },
    [](auto&&) {
      return false;
    })(kType<std::remove_cvref_t<T>>);

template <std::size_t I, typename T>
struct IndexedType {};

template <typename...>
struct Caster {};

template <std::size_t... Is, typename... Ts>
struct Caster<std::index_sequence<Is...>, Ts...> : IndexedType<Is, Ts>... {};

export template <typename... Ts, typename... TTs>
consteval auto operator==(const TypeList<Ts...>& first, const TypeList<TTs...>& second) -> bool {
  return std::same_as<decltype(first), decltype(second)>;
};

export template <typename... Ts, typename... TTs>
consteval auto operator+(const TypeList<Ts...>&, const TypeList<TTs...>&) -> TypeList<Ts..., TTs...> {
  return {};
};

export template <std::size_t I, typename... Ts>
consteval auto Get(TypeList<Ts...>) -> decltype([]<typename T>(IndexedType<I, T>&&) -> T {
}(Caster<std::index_sequence_for<Ts...>, Ts...>{}));

export template <typename T, typename... Ts>
consteval auto Find(TypeList<Ts...>) -> std::size_t {
  std::array<bool, sizeof...(Ts)> arr{std::same_as<T, Ts>...};
  return std::ranges::find(arr, true) - arr.begin();
};

export template <template <typename...> typename F, typename... Ts>
consteval auto Find(TypeList<Ts...>) -> std::size_t {
  std::array<bool, sizeof...(Ts)> arr{F<Ts>::value...};
  return std::ranges::find(arr, true) - arr.begin();
};

export template <typename... Ts>
consteval auto Reverse(TypeList<Ts...> list) {
  return [&]<auto... Is>(std::index_sequence<Is...>) -> TypeList<decltype(Get<sizeof...(Ts) - Is - 1>(list))...> {
    return {};
  }(std::index_sequence_for<Ts...>());
};

export template <typename... Ts>
consteval auto Transform(TypeList<Ts...>, auto&& f) -> TypeList<decltype(f(TypeList<Ts>{}))...> {
  return {};
};

export template <typename... Ts>
consteval auto FilterTypeList(TypeList<Ts...>, auto&& f) {
  return ((kTypeList<> +
           [](auto&& list) {
             if constexpr(decltype(f(list))::kValue) {
               return list;
             } else {
               return kTypeList<>;
             }
           }(kType<Ts>)) +
          ...);
};

export template <typename... Ts>
consteval auto Size(TypeList<Ts...>) -> std::size_t {
  return sizeof...(Ts);
};

export template <std::size_t N, std::size_t From = 0, typename... Ts>
consteval auto TakeFrom(TypeList<Ts...> list)
  requires(N + From <= sizeof...(Ts))
{
  return [&]<std::size_t... Is>(std::index_sequence<Is...>) -> TypeList<decltype(Get<Is + From>(list))...> {
    return {};
  }(std::make_index_sequence<N>());
};

export template <typename T, std::size_t N, typename... Ts>
consteval auto PushTo(TypeList<Ts...> list) {
  return TakeFrom<N>(list) + kType<T> + TakeFrom<sizeof...(Ts) - N, N>(list);
};

export template <std::size_t N, typename... Ts>
consteval auto Erase(TypeList<Ts...> list) {
  return TakeFrom<N>(list) + TakeFrom<sizeof...(Ts) - N - 1, N + 1>(list);
};

}  // namespace utempl
