#pragma once
#include <optional>

namespace utempl {

template <typename T>
struct Optional {
  bool flag = false;
  union {
    char null;
    T _value;
  };
  inline constexpr Optional() = default;
  inline constexpr Optional(const Optional&) = default;
  inline constexpr Optional(Optional&&) = default;
  inline constexpr Optional(T&& arg) : _value(std::move(arg)), flag(true) {};
  inline constexpr Optional(const T& arg) : _value(arg), flag(true) {};
  inline constexpr Optional(std::nullopt_t) : flag(false), null(0) {};
  inline constexpr auto has_value() const -> bool {
    return this->flag;
  };
  inline constexpr auto value() -> T& {
    return this->_value;
  };
  inline constexpr auto operator*() -> T& {
    return this->value();
  };
  inline constexpr auto operator->() -> T* {
    return &this->value();
  };
  inline constexpr auto value() const -> const T& {
    return this->_value;
  };
  inline constexpr auto operator*() const -> const T& {
    return this->value();
  };
  inline constexpr auto operator->() const -> const T* {
    return &this->value();
  };
  inline constexpr explicit operator bool() const {
    return this->has_value();
  };
};

} // namespace utempl
