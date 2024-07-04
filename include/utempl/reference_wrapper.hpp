#pragma once

namespace utempl {
template <typename T>
struct ReferenceWrapper;

template <typename T>
struct ReferenceWrapper<T&&> {
  constexpr auto operator*() -> T& {
    return this->value;
  };
  constexpr auto operator->() -> T* {
    return &this->value;
  };
  T&& value;
};

template <typename T>
struct ReferenceWrapper<const T&> {
  constexpr auto operator*() -> const T& {
    return this->value;
  };
  constexpr auto operator->() -> const T* {
    return &this->value;
  };
  const T& value;
};

template <typename T>
struct ReferenceWrapper<T&> {
  constexpr auto operator*() -> T& {
    return this->value;
  };
  constexpr auto operator->() -> T* {
    return &this->value;
  };
  T& value;
};

}  // namespace utempl
