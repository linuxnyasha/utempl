#pragma once
#include <utempl/loopholes/counter.hpp>
#include <utempl/type_list.hpp>

namespace utempl {

namespace impl {

struct Types {};
} // namespace impl


template <std::size_t Id, typename Tag>
struct MetaInfoKey {};


template <typename T, typename Tag = impl::Types>
struct MetaInfo {
  static constexpr std::size_t kTypeId = loopholes::Counter<Tag, T>();
  using Type = T;
private: 
  static constexpr auto _ = loopholes::Injector<MetaInfoKey<kTypeId, Tag>{}, TypeList<T>{}>{};
};

template <typename T, typename Tag = impl::Types>
inline constexpr std::size_t kTypeId = MetaInfo<T, Tag>::kTypeId;

template <
  typename Tag,
  typename T,
  typename... Ts,
  typename... TTs,
  std::size_t Id = loopholes::Counter<Tag, T, Ts..., TTs...>(),
  auto = loopholes::Injector<MetaInfoKey<Id, Tag>{}, TypeList<T>{}>{}
>
consteval std::size_t AddTypeToTag(TTs&&...) {
  return Id;
};


template <std::size_t Id, typename Tag = impl::Types>
using GetMetaInfo = MetaInfo<typename decltype(Magic(loopholes::Getter<MetaInfoKey<Id, Tag>{}>{}))::Type>;

template <std::size_t Id, typename Tag = impl::Types>
using GetType = GetMetaInfo<Id, Tag>::Type;

namespace impl {

template <typename Tag, std::size_t I = 0, typename... Ts, typename G>
static consteval auto GetTypeListForTag(G g) requires (I == 0 || 
    requires {Magic(loopholes::Getter<MetaInfoKey<I, Tag>{}>{});}) {
  if constexpr(I == 0 && !requires {Magic(loopholes::Getter<MetaInfoKey<I, Tag>{}>{});}) {
    return TypeList{};
  } else {
    if constexpr(requires{GetTypeListForTag<Tag, I + 1, Ts...>(g);}) {
      constexpr auto type = Magic(loopholes::Getter<MetaInfoKey<I, Tag>{}>{});
      return GetTypeListForTag<Tag, I + 1, Ts..., typename decltype(type)::Type>(g);
    } else {
      constexpr auto type = Magic(loopholes::Getter<MetaInfoKey<I, Tag>{}>{});
      return TypeList<Ts..., typename decltype(type)::Type>();
    };
  };
};


} // namespace impl

template <typename Tag = impl::Types, typename... Ts>
consteval auto GetTypeListForTag() {
  return impl::GetTypeListForTag<Tag>(TypeList<Ts...>{});
};

/*
static_assert(kTypeId<int> == 0);
static_assert(kTypeId<void> == 1);
static_assert(AddTypeToTag<impl::Types, void, int>() == 2);
static_assert(std::is_same_v<decltype(GetTypeListForTag()), TypeList<int, void, void>>);

*/


} // namespace utempl
