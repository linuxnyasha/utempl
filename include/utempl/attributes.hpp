#include <utempl/loopholes/counter.hpp>
#include <utempl/meta_info.hpp>
#include <utempl/utils.hpp>


namespace utempl {



namespace impl {

struct AttributesTag {};

template <typename T>
struct AttributesCounterTag {};



} // namespace impl


template <
  typename T,
  typename...,
  auto f = []{},
  auto = AddTypeToTag<impl::AttributesTag, T, decltype(f)>()
>
consteval auto OpenStruct() -> bool {
  return true;
};

template <
  typename...,
  auto f = []{},
  auto I = loopholes::CountValue<impl::AttributesTag, decltype(f)>(),
  auto II = (I > 2) ? I - 2 : I - 1,
  typename T = decltype(Magic(loopholes::Getter<MetaInfoKey<II, impl::AttributesTag>{}>{}))::Type,
  auto = AddTypeToTag<impl::AttributesTag, T, decltype(f)>()
>
consteval auto CloseStruct() -> bool {
  return true;
};



struct NoInfo {
  consteval auto operator==(const NoInfo&) const -> bool = default;
};


template <typename T>
struct FieldType {
  using Type = T;
};


namespace impl {

template <
  typename T,
  typename... Ts,
  auto f = []{},
  typename Current = decltype(GetCurrentTagType<impl::AttributesTag, decltype(f)>())::Type,
  auto = AddTypeToTag<impl::AttributesCounterTag<Current>, TypeList<Ts...>, decltype(f)>()
>
consteval auto FieldAttribute() -> T::Type;

} // namespace impl

template <typename... Ts>
using FieldAttribute = decltype(impl::FieldAttribute<Ts...>());


#define ATTRIBUTE_STRUCT(name, ...) struct name { \
static_assert(::utempl::OpenStruct<name>()); \
template <std::size_t N> \
static consteval auto GetAttribute(); \
  __VA_ARGS__ \
  static_assert(::utempl::CloseStruct());\
}


#define GENERIC_ATTRIBUTE(value) \
  template <> \
  consteval auto GetAttribute< \
      ::utempl::loopholes::Counter< \
        ::utempl::impl::AttributesCounterTag<decltype(::utempl::GetCurrentTagType< \
          ::utempl::impl::AttributesTag, \
          decltype([]{}) \
        >())::Type \
      >, \
      decltype([]{}) >()>() { return value; }

#define SKIP_ATTRIBUTE() GENERIC_ATTRIBUTE(::utempl::NoInfo{})


template <
  typename T,
  auto f = []{},
  bool R = (loopholes::CountValue<impl::AttributesCounterTag<T>, decltype(f)>() > 0)
>
concept HasAttributes = R;

template <typename T>
concept HasMacroAttributes = requires {T::template GetAttribute<0>();};



template <HasAttributes T>
consteval auto GetAttributes() requires HasMacroAttributes<T> {
  constexpr auto I = loopholes::CountValue<impl::AttributesCounterTag<T>>();
  return [](auto... is) {
    return Tuple{T::template GetAttribute<is>()...};
  } | kSeq<I>;
};

template <typename T>
consteval auto GetAttributes() {
  constexpr auto I = loopholes::CountValue<impl::AttributesCounterTag<T>>();
  return [](auto... is) {
    return utempl::Tuple{typename decltype(Magic(loopholes::Getter<MetaInfoKey<is, impl::AttributesCounterTag<T>>{}>{}))::Type{}...};
  } | kSeq<I>;
};




} // namespace utempl
