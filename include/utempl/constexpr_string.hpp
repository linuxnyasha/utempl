#pragma once
#include <fmt/format.h>

#include <algorithm>

namespace utempl {
template <std::size_t>
struct ConstexprString;
}  // namespace utempl

template <std::size_t Size>
struct fmt::formatter<utempl::ConstexprString<Size>> : public fmt::formatter<std::string_view> {
  constexpr auto parse(format_parse_context& ctx) const {
    return ctx.begin();
  };
  constexpr auto format(const utempl::ConstexprString<Size>& str, auto& ctx) const {
    return fmt::formatter<std::string_view>::format({str.begin()}, ctx);
  };
};

namespace utempl {

template <std::size_t Size>
struct ConstexprString {
  std::array<char, Size> data;
  constexpr auto begin() -> char* {
    return this->data.begin();
  };
  [[nodiscard]] constexpr auto begin() const -> const char* {
    return this->data.begin();
  };
  [[nodiscard]] constexpr auto end() -> char* {
    return this->data.end();
  };
  [[nodiscard]] constexpr auto end() const -> const char* {
    return this->data.end();
  };
  static constexpr auto kSize = Size == 0 ? 0 : Size - 1;
  explicit constexpr ConstexprString() = default;
  constexpr ConstexprString(const char (&data)[Size]) : data{} {  // NOLINT
    std::ranges::copy_n(data, Size, this->data.begin());
  };
  explicit constexpr ConstexprString(std::string data) : data{} {
    std::ranges::copy_n(data.begin(), Size, this->data.begin());
  };
  explicit constexpr ConstexprString(std::array<char, Size> data) : data(std::move(data)) {};
  constexpr auto size() const {
    return Size == 0 ? 0 : Size - 1;
  };
  constexpr operator std::string_view() const& {  // NOLINT
    return {this->begin()};
  };
  constexpr auto operator==(std::string_view other) const -> bool {
    return static_cast<std::string_view>(*this) == other;
  };
  constexpr auto operator==(const ConstexprString<Size>& other) const {
    return static_cast<std::string_view>(*this) == static_cast<std::string_view>(other);
  };
  template <std::size_t SSize>
  constexpr auto operator==(const ConstexprString<SSize>& other) const -> bool {
    return false;
  };
  constexpr auto operator==(const std::string& other) const -> bool {
    return static_cast<std::string_view>(*this) == other;
  };
  template <std::size_t SSize>
  constexpr auto operator+(const ConstexprString<SSize>& other) -> ConstexprString<Size + SSize - 1> {
    ConstexprString<Size + SSize - 1> response;
    std::ranges::copy_n(this->begin(), Size - 1, response.begin());
    std::ranges::copy_n(other.begin(), SSize, response.begin() + Size - 1);
    return response;
  };
  constexpr ConstexprString(const ConstexprString&) = default;
  constexpr ConstexprString(ConstexprString&&) = default;
};

template <std::size_t N>
constexpr auto operator<<(std::ostream& stream, const ConstexprString<N>& str) -> std::ostream& {
  stream << static_cast<std::string_view>(str);
  return stream;
};

template <std::size_t Count>
constexpr auto CreateStringWith(char c) {
  ConstexprString<Count + 1> str{};
  for(std::size_t i = 0; i < Count; i++) {
    str.data[i] = c;
  };
  str.data[Count] = '\0';
  return str;
};

template <std::size_t Size>
ConstexprString(const char (&data)[Size]) -> ConstexprString<Size>;  // NOLINT
}  // namespace utempl
