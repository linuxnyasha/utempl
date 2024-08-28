#pragma once
#define ATTRIBUTE_STRUCT(name, ...) /* NOLINT */ \
                                                 \
  struct name {                                  \
    static_assert(::utempl::OpenStruct<name>()); \
    template <std::size_t N>                     \
    static consteval auto GetAttribute();        \
    __VA_ARGS__                                  \
    static_assert(::utempl::CloseStruct());      \
  }

#define GENERIC_ATTRIBUTE(value) /* NOLINT */                                                                                              \
  template <>                                                                                                                              \
  consteval auto GetAttribute<::utempl::loopholes::Counter<                                                                                \
      ::utempl::impl::AttributesCounterTag<decltype(::utempl::GetCurrentTagType<::utempl::impl::AttributesTag, decltype([] {})>())::Type>, \
      decltype([] {})>()>() {                                                                                                              \
    return value;                                                                                                                          \
  }

#define SKIP_ATTRIBUTE() /* NOLINT */ GENERIC_ATTRIBUTE(::utempl::NoInfo{})
