export module utempl.attributes;
export import utempl.meta_info;
export import utempl.tuple;
export import utempl.loopholes.counter;
import utempl.utils;
import utempl.type_list;

namespace utempl {

namespace impl {

export struct AttributesTag {};

export template <typename T>
struct AttributesCounterTag {};

}  // namespace impl

export template <typename T, typename..., auto f = [] {}, auto = AddTypeToTag<impl::AttributesTag, T, decltype(f)>()>
consteval auto OpenStruct() -> bool {
  return true;
};

export template <typename...,
                 auto f = [] {},
                 auto I = loopholes::CountValue<impl::AttributesTag, decltype(f)>(),
                 auto II = (I >= 2) ? I - 2 : I - 1,
                 typename T = decltype(Magic(loopholes::Getter<MetaInfoKey<II, impl::AttributesTag>{}>{}))::Type,
                 auto = AddTypeToTag<impl::AttributesTag, T, decltype(f)>()>
consteval auto CloseStruct() -> bool {
  return true;
};

export struct NoInfo {
  consteval auto operator==(const NoInfo&) const -> bool = default;
};

export template <typename T,
                 typename O = TypeList<>,
                 auto f = [] {},
                 typename Current = decltype(GetCurrentTagType<impl::AttributesTag, decltype(f)>())::Type,
                 auto = AddTypeToTag<impl::AttributesCounterTag<Current>, O, decltype(f)>()>
using FieldAttribute = T;

export template <typename T, auto f = [] {}, bool R = (loopholes::CountValue<impl::AttributesCounterTag<T>, decltype(f)>() > 0)>
concept HasAttributes = R;

export template <typename T>
concept HasMacroAttributes = requires { T::template GetAttribute<0>(); };

export template <HasAttributes T>
consteval auto GetAttributes()
  requires HasMacroAttributes<T>
{
  constexpr auto I = loopholes::CountValue<impl::AttributesCounterTag<T>>();
  return [](auto... is) {
    return Tuple{T::template GetAttribute<is>()...};
  } | kSeq<I>;
};

export template <typename T>
consteval auto GetAttributes() {
  constexpr auto I = loopholes::CountValue<impl::AttributesCounterTag<T>>();
  return
      [](auto... is) -> Tuple<typename decltype(Magic(loopholes::Getter<MetaInfoKey<*is, impl::AttributesCounterTag<T>>{}>{}))::Type...> {
        return {};
      } | kSeq<I>;
};

}  // namespace utempl
