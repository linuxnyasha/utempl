#pragma once
#include <utempl/module.hpp>

#ifdef UTEMPL_MODULE
export module utempl.attributes;
export import utempl.meta_info;
export import utempl.tuple;
export import utempl.loopholes.counter;
import utempl.utils;
import utempl.type_list;
#else

#include <utempl/loopholes/counter.hpp>
#include <utempl/meta_info.hpp>
#include <utempl/tuple.hpp>
#include <utempl/type_list.hpp>
#include <utempl/utils.hpp>
#endif

namespace utempl {

namespace impl {

UTEMPL_EXPORT struct AttributesTag {};

UTEMPL_EXPORT template <typename T>
struct AttributesCounterTag {};

}  // namespace impl

UTEMPL_EXPORT template <typename T, typename..., auto f = [] {}, auto = AddTypeToTag<impl::AttributesTag, T, decltype(f)>()>
consteval auto OpenStruct() -> bool {
  return true;
};

UTEMPL_EXPORT template <typename...,
                        auto f = [] {},
                        auto I = loopholes::CountValue<impl::AttributesTag, decltype(f)>(),
                        auto II = (I >= 2) ? I - 2 : I - 1,
                        typename T = decltype(Magic(loopholes::Getter<MetaInfoKey<II, impl::AttributesTag>{}>{}))::Type,
                        auto = AddTypeToTag<impl::AttributesTag, T, decltype(f)>()>
consteval auto CloseStruct() -> bool {
  return true;
};

UTEMPL_EXPORT struct NoInfo {
  consteval auto operator==(const NoInfo&) const -> bool = default;
};

UTEMPL_EXPORT template <typename T,
                        typename O = TypeList<>,
                        auto f = [] {},
                        typename Current = decltype(GetCurrentTagType<impl::AttributesTag, decltype(f)>())::Type,
                        auto = AddTypeToTag<impl::AttributesCounterTag<Current>, O, decltype(f)>()>
using FieldAttribute = T;

UTEMPL_EXPORT template <typename T, auto f = [] {}, bool R = (loopholes::CountValue<impl::AttributesCounterTag<T>, decltype(f)>() > 0)>
concept HasAttributes = R;

UTEMPL_EXPORT template <typename T>
concept HasMacroAttributes = requires { T::template GetAttribute<0>(); };

UTEMPL_EXPORT template <HasAttributes T>
consteval auto GetAttributes()
  requires HasMacroAttributes<T>
{
  constexpr auto I = loopholes::CountValue<impl::AttributesCounterTag<T>>();
  return [](auto... is) {
    return Tuple{T::template GetAttribute<is>()...};
  } | kSeq<I>;
};

UTEMPL_EXPORT template <typename T>
consteval auto GetAttributes() {
  constexpr auto I = loopholes::CountValue<impl::AttributesCounterTag<T>>();
  return
      [](auto... is) -> Tuple<typename decltype(Magic(loopholes::Getter<MetaInfoKey<*is, impl::AttributesCounterTag<T>>{}>{}))::Type...> {
        return {};
      } | kSeq<I>;
};

}  // namespace utempl
