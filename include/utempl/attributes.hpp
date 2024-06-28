#include <utempl/loopholes/counter.hpp>
#include <utempl/meta_info.hpp>


namespace utempl {



namespace impl {

struct AttributesTag {};

template <typename T>
struct AttributesCounterTag {};


template <typename Current, auto f = []{}, auto = AddTypeToTag<impl::AttributesTag, Current, decltype(f)>()>
struct CurrentSetter {};


} // namespace impl





#define ATTRIBUTE_STRUCT(name, ...) struct name { \
static_assert((::utempl::impl::CurrentSetter<name>(), true)); \
template <std::size_t N> \
static consteval auto GetAttribute(); __VA_ARGS__ }

struct NoInfo {};

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


} // namespace utempl
