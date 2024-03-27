#pragma once
#include <utempl/loopholes/counter.hpp>
#include <array>
#include <utempl/type_list.hpp>

namespace utempl {

namespace impl {

struct Types {};
} // namespace impl


template <std::size_t Id>
struct MetaInfoKey {

};


template <typename T>
struct MetaInfo {
  static constexpr std::size_t kTypeId = loopholes::Counter<impl::Types, T>();
  using Type = T;
private: 
  static constexpr auto _ = loopholes::Injector<MetaInfoKey<kTypeId>{}, TypeList<T>{}>{};

};

template <typename T>
inline constexpr std::size_t kTypeId = MetaInfo<T>::kTypeId;

template <std::size_t Id>
using GetMetaInfo = MetaInfo<typename decltype(Magic(loopholes::Getter<MetaInfoKey<Id>{}>{}))::Type>;

template <std::size_t Id>
using GetType = GetMetaInfo<Id>::Type;

} // namespace utempl
