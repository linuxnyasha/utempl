#pragma once

namespace utempl {
template <typename T>
struct ReferenceWrapper;

template <typename T>
struct ReferenceWrapper<T&&> {
  inline constexpr auto operator*() -> T& {
    return this->value;
  };
  inline constexpr auto operator->() -> T* {
    return &this->value;
  };
  T&& value;
};

template <typename T>
struct ReferenceWrapper<const T&> {
  inline constexpr auto operator*() -> const T& {
    return this->value;
  };
  inline constexpr auto operator->() -> const T* {
    return &this->value;
  };
  const T& value;
};

template <typename T>
struct ReferenceWrapper<T&> {

  inline constexpr auto operator*() -> T& {
    return this->value;
  };
  inline constexpr auto operator->() -> T* {
    return &this->value;
  };
  T& value;
};

} // namespace utempl
