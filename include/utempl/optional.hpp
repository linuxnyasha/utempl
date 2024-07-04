#pragma once
#include <optional>

namespace utempl {

template <typename T>
struct Optional {  // NOLINT
  bool flag = false;
  union {
    char null;
    T _value;
  };
  constexpr Optional() = default;
  constexpr Optional(const Optional&) = default;
  constexpr Optional(Optional&&) = default;
  explicit constexpr Optional(T&& arg) : _value(std::move(arg)), flag(true) {};
  explicit constexpr Optional(const T& arg) : _value(arg), flag(true) {};
  explicit constexpr Optional(std::nullopt_t) : null(0) {};
  [[nodiscard]] constexpr auto has_value() const -> bool {
    return this->flag;
  };
  constexpr auto value() -> T& {
    return this->_value;
  };
  constexpr auto operator*() -> T& {
    return this->value();
  };
  constexpr auto operator->() -> T* {
    return &this->value();
  };
  constexpr auto value() const -> const T& {
    return this->_value;
  };
  constexpr auto operator*() const -> const T& {
    return this->value();
  };
  constexpr auto operator->() const -> const T* {
    return &this->value();
  };
  constexpr explicit operator bool() const {
    return this->has_value();
  };
};

}  // namespace utempl
