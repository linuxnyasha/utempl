#pragma once
#include <utempl/loopholes/counter.hpp>
#include <array>
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

template <std::size_t Id, typename Tag = impl::Types>
using GetMetaInfo = MetaInfo<typename decltype(Magic(loopholes::Getter<MetaInfoKey<Id, Tag>{}>{}))::Type>;

template <std::size_t Id, typename Tag = impl::Types>
using GetType = GetMetaInfo<Id, Tag>::Type;

} // namespace utempl
