export module utempl.meta_info;
import utempl.loopholes;
import utempl.type_list;
import std;

namespace utempl {

export struct Types {};

export template <std::size_t Id, typename Tag>
struct MetaInfoKey {};

export template <typename T, typename Tag = Types>
struct MetaInfo {
  static constexpr std::size_t kTypeId = loopholes::Counter<Tag, T>();
  using Type = T;

 private:
  static constexpr auto _ = loopholes::Injector<MetaInfoKey<kTypeId, Tag>{}, TypeList<T>{}>{};
};

export template <typename T, typename Tag = Types>
inline constexpr std::size_t kTypeId = MetaInfo<T, Tag>::kTypeId;

export template <typename Tag,
                 typename T,
                 typename... Ts,
                 typename... TTs,
                 std::size_t Id = loopholes::Counter<Tag, T, Ts..., TTs...>(),
                 auto = loopholes::Injector<MetaInfoKey<Id, Tag>{}, TypeList<T>{}>{}>
consteval auto AddTypeToTag(TTs&&...) -> std::size_t {
  return Id;
};

export template <std::size_t Id, typename Tag = Types>
using GetMetaInfo = MetaInfo<typename decltype(Magic(loopholes::Getter<MetaInfoKey<Id, Tag>{}>{}))::Type>;

export template <std::size_t Id, typename Tag = Types>
using GetType = GetMetaInfo<Id, Tag>::Type;

template <typename Tag, std::size_t I = 0, typename... Ts, typename G>
static consteval auto GetTypeListForTag(G g)
  requires(I == 0 || requires { Magic(loopholes::Getter<MetaInfoKey<I, Tag>{}>{}); })
{
  if constexpr(I == 0 && !requires { Magic(loopholes::Getter<MetaInfoKey<I, Tag>{}>{}); }) {
    return TypeList{};
  } else {
    if constexpr(requires { GetTypeListForTag<Tag, I + 1, Ts...>(g); }) {
      constexpr auto type = Magic(loopholes::Getter<MetaInfoKey<I, Tag>{}>{});
      return GetTypeListForTag<Tag, I + 1, Ts..., typename decltype(type)::Type>(g);
    } else {
      constexpr auto type = Magic(loopholes::Getter<MetaInfoKey<I, Tag>{}>{});
      return TypeList<Ts..., typename decltype(type)::Type>();
    };
  };
};

export template <typename Tag = Types, typename... Ts>
consteval auto GetTypeListForTag() {
  return GetTypeListForTag<Tag>(TypeList<Ts...>{});
};

export template <typename Tag, typename... Ts, auto I = utempl::loopholes::CountValue<Tag, Ts...>() - 1>
consteval auto GetCurrentTagType() {
  return Magic(utempl::loopholes::Getter<MetaInfoKey<I, Tag>{}>());
};

/*
static_assert(kTypeId<int> == 0);
static_assert(kTypeId<void> == 1);
static_assert(AddTypeToTag<Types, void, int>() == 2);
static_assert(std::is_same_v<decltype(GetTypeListForTag()), TypeList<int, void, void>>);

*/

}  // namespace utempl
