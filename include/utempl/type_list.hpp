#include <concepts>
#include <utility>
#include <array>
#include <ranges>

namespace utempl {

namespace impl {

struct Caster {
  constexpr Caster(auto&&) {};
};

};

template <typename... Ts>
struct TypeList {
};
template <typename... Ts, typename... TTs>
consteval auto operator==(const TypeList<Ts...>& first, const TypeList<TTs...>& second) -> bool {
  return std::same_as<decltype(first), decltype(second)>;
};
template <std::size_t... Is, typename T>
consteval auto Get(std::index_sequence<Is...>, decltype(impl::Caster(Is))..., T, ...) -> T; 

template <std::size_t I, typename... Ts>
consteval auto Get(const TypeList<Ts...>&) -> decltype(Get(std::make_index_sequence<I>(), std::declval<Ts>()...)) requires (I < sizeof...(Ts));

template <typename T, typename... Ts>
consteval auto Find(TypeList<Ts...>) -> std::size_t {
  std::array arr{std::same_as<T, Ts>...};
  return std::ranges::find(arr, true) - arr.begin();
};

} // namespace utempl
