#pragma once
#include <utempl/type_list.hpp>
#include <utempl/overloaded.hpp>
namespace utempl {

template <auto>
struct Wrapper;
namespace impl {

template <auto, typename T>
struct TupleLeaf {
  T value;
  inline constexpr bool operator==(const TupleLeaf&) const = default;
};


template <typename, typename...>
struct TupleHelper;

template <std::size_t... Is, typename... Ts>
struct TupleHelper<std::index_sequence<Is...>, Ts...> : public impl::TupleLeaf<Is, Ts>... {
  inline constexpr bool operator==(const TupleHelper&) const = default;
};

template <typename T>
struct Process {
  using type = decltype(Overloaded(
    []<typename TT>(TT&&) -> std::remove_cvref_t<TT> {},
    []<std::size_t N>(const char(&)[N]) -> const char* {}
  )(std::declval<T>()));
};

} // namespace impl

template <typename... Ts>
struct Tuple;

template <std::size_t I, typename... Ts>
inline constexpr auto Get(Tuple<Ts...>& tuple) -> auto& requires (I < sizeof...(Ts)) {
  using Type = decltype(Get<I>(TypeList<Ts...>{}));
  return static_cast<impl::TupleLeaf<I, Type>&>(tuple).value;
};

template <std::size_t I, typename... Ts>
inline constexpr auto Get(const Tuple<Ts...>& tuple) -> const auto& requires (I < sizeof...(Ts)) {
  using Type = decltype(Get<I>(TypeList<Ts...>{}));
  return static_cast<const impl::TupleLeaf<I, Type>&>(tuple).value;
};

template <std::size_t I, typename... Ts>
inline constexpr auto Get(Tuple<Ts...>&& tuple) -> decltype(auto) requires (I < sizeof...(Ts)) {
  using Type = decltype(Get<I>(TypeList<Ts...>{}));
  return std::move(static_cast<impl::TupleLeaf<I, Type>&&>(tuple).value);
};

template <std::size_t I, typename T>
inline constexpr auto Get(T&& arg) -> decltype(get<I>(std::forward<T>(arg))) {
  return get<I>(std::forward<T>(arg));
};

template <typename... Ts>
struct Tuple : impl::TupleHelper<std::index_sequence_for<Ts...>, Ts...> {
  template <typename... TTs>
  inline constexpr Tuple(TTs&&... args) : 
    impl::TupleHelper<std::index_sequence_for<Ts...>, Ts...>{{std::forward<TTs>(args)}...} {};
  template <std::invocable... Fs>
  inline constexpr Tuple(Fs&&... fs) requires (!std::invocable<Ts> || ...) :
    impl::TupleHelper<std::index_sequence_for<Ts...>, Ts...>{{fs()}...} {};
  inline constexpr Tuple(Ts&&... args) :
    impl::TupleHelper<std::index_sequence_for<Ts...>, Ts...>{{args}...} {};
  inline constexpr Tuple(const Tuple&) = default;
  inline constexpr Tuple(Tuple&&) = default;
  inline constexpr Tuple(Tuple&) = default;
  inline constexpr Tuple& operator=(const Tuple&) = default;
  inline constexpr Tuple& operator=(Tuple&&) = default;
  inline constexpr Tuple& operator=(Tuple&) = default;
  inline constexpr Tuple() : 
    impl::TupleHelper<std::index_sequence_for<Ts...>, Ts...>() {};
  inline constexpr bool operator==(const Tuple<Ts...>& other) const {
    return [&]<std::size_t... Is>(std::index_sequence<Is...>){
      return ((Get<Is>(*this) == Get<Is>(other)) && ...);
    }(std::index_sequence_for<Ts...>());
  }; 
  template <typename... TTs>
  inline constexpr auto operator+(const Tuple<TTs...>& other) const -> Tuple<Ts..., TTs...> {
    return [&]<std::size_t... Is, std::size_t... IIs>(std::index_sequence<Is...>, std::index_sequence<IIs...>) -> Tuple<Ts..., TTs...> {
      return {Get<Is>(*this)..., Get<IIs>(other)...};
    }(std::index_sequence_for<Ts...>(), std::index_sequence_for<TTs...>());
  };
  
  template <auto I>
  inline constexpr auto operator[](Wrapper<I>) const -> const auto& {
    return Get<I>(*this);
  };
  
  template <auto I>
  inline constexpr auto operator[](Wrapper<I>) -> auto& {
    return Get<I>(*this);
  };
};

template <>
struct Tuple<> {
  template <typename... Ts>
  inline constexpr auto operator+(const Tuple<Ts...>& other) const -> Tuple<Ts...> {
    return other;
  };
  inline constexpr auto operator==(const Tuple<>&) const {
    return true;
  };
};
template <typename... Ts>
Tuple(Ts&&...) -> Tuple<typename impl::Process<Ts>::type...>;

template <typename... Ts>
consteval auto ListFromTuple(Tuple<Ts...>) -> TypeList<Ts...> {
  return {};
};

} // namespace utempl
