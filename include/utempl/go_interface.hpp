#pragma once
#include <utempl/constexpr_string.hpp>
#include <boost/pfr.hpp>
#include <type_traits>

namespace utempl {

template <ConstexprString name, typename T>
consteval auto FindField() -> std::size_t {
  constexpr auto names = boost::pfr::names_as_array<std::remove_cvref_t<T>>();
  return std::ranges::find(names, static_cast<std::string_view>(name)) - names.begin();
};

template <ConstexprString name, typename T>
inline constexpr auto Get(T&& arg) {
  return boost::pfr::get<FindField<name, T>()>(std::forward<T>(arg));
};

template <auto...>
struct EmptyField {};

namespace impl {


template <ConstexprString name, typename T>
inline constexpr auto TryGet(T&& arg) 
      -> decltype(boost::pfr::get<FindField<name, T>()>(std::forward<T>(arg)))
      requires(FindField<name, T>() < boost::pfr::tuple_size_v<std::remove_cvref_t<T>>) {
  constexpr auto I = FindField<name, T>();
  return boost::pfr::get<I>(std::forward<T>(arg));
};
template <ConstexprString name, typename T>
inline constexpr auto TryGet(T&& arg) {
  return EmptyField<name>{};
};


template <typename To, typename Transformer, typename From>
inline constexpr auto Transform(Transformer&& transformer, From&& from) {
  return [&]<auto... Is>(std::index_sequence<Is...>){
    return To{transformer(TryGet<ConstexprString<boost::pfr::get_name<Is, To>().size() + 1>{boost::pfr::get_name<Is, To>().begin()}>(from))...};
  }(std::make_index_sequence<boost::pfr::tuple_size_v<To>>());
};

} // impl

struct DefaultFieldTransformer {
  inline constexpr auto operator()(auto&& arg) -> auto&& {
    return arg;
  };
  inline constexpr auto operator()(auto& arg) -> auto& {
    return arg;
  };
  template <ConstexprString str>
  inline constexpr auto operator()(EmptyField<str> arg) {
    static_assert(str == "This Field Not Found");
  };
};

template <typename Value, typename Transformer = DefaultFieldTransformer>
struct GoInterface : Value {
  inline constexpr GoInterface(Value&& value) : 
      Value(std::move(value)) {};
  inline constexpr GoInterface(const Value& value) : 
      Value(value) {};
  template <typename T>
  inline constexpr GoInterface(T&& value) :
    Value(impl::Transform<Value>(Transformer{}, std::forward<T>(value))) {};
  inline constexpr auto operator=(const GoInterface&) -> GoInterface& = default;
  inline constexpr auto operator=(GoInterface&&) -> GoInterface& = default;
  inline constexpr auto operator==(const Value& other) const -> bool 
      requires requires(const Value& a){a == a;} {
    return static_cast<const Value&>(*this) == other;
  };
  inline constexpr auto operator==(const GoInterface& other) const -> bool {
    return *this == static_cast<const Value&>(other);
  };
  template <typename T>
  inline constexpr auto operator==(T&& other) const -> bool {
    return [&]<auto... Is>(std::index_sequence<Is...>){
      using Type = std::remove_cvref_t<T>;
      Transformer transformer;
      // + 1 - NULL Terminator
      return ((boost::pfr::get<Is>(static_cast<const Value&>(*this)) == 
        transformer(impl::TryGet<ConstexprString<boost::pfr::get_name<Is, Type>().size() + 1>{boost::pfr::get_name<Is, Type>().begin()}>(other))) && ...);
    }(std::make_index_sequence<boost::pfr::tuple_size_v<Value>>());
  };
  template <typename T>
  inline constexpr auto operator==(T&& other) const -> bool 
        requires (requires(const Value& value){value == other;})  {
    return static_cast<const Value&>(*this) == other;
  };
};

} // namespace utempl

