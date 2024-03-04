#pragma once
#include <fmt/format.h>

namespace utempl {
template <std::size_t>
struct ConstexprString;
} // namespace utempl

template <std::size_t Size>
struct fmt::formatter<utempl::ConstexprString<Size>> : public fmt::formatter<std::string_view> {
  constexpr auto parse(format_parse_context& ctx) const {
    return ctx.begin();
  };
  inline constexpr auto format(const utempl::ConstexprString<Size>& str, auto& ctx) const {
    return fmt::formatter<std::string_view>::format({str.begin()}, ctx);
  };
};

namespace utempl {

template <std::size_t Size>
struct ConstexprString {
  std::array<char, Size> data;
  inline constexpr auto begin() -> char* {
    return this->data.begin();
  };
  inline constexpr auto begin() const -> const char* {
    return this->data.begin();
  };
  inline constexpr auto end() -> char* {
    return this->data.end();
  };
  inline constexpr auto end() const -> const char* {
    return this->data.end();
  };
  static constexpr auto kSize = Size == 0 ? 0 : Size - 1;
  inline constexpr ConstexprString() = default;
  inline constexpr ConstexprString(const char (&data)[Size]) : data{} {
    std::ranges::copy_n(data, Size, this->data.begin());
  };
  inline constexpr ConstexprString(std::string data) : data{} {
    std::ranges::copy_n(data.begin(), Size, this->data.begin());
  };
  inline constexpr ConstexprString(std::array<char, Size> data) : data(std::move(data)) {};
  inline constexpr auto size() const {
    return Size == 0 ? 0 : Size - 1;
  };
  inline constexpr operator std::string_view() const & {
    return {this->begin()};
  };
  inline constexpr bool operator==(std::string_view other) const {
    return static_cast<std::string_view>(*this) == other;
  };
  template <std::size_t SSize>
  inline constexpr auto operator+(const ConstexprString<SSize>& other) -> ConstexprString<Size + SSize - 1> {
    ConstexprString<Size + SSize - 1> response;
    std::ranges::copy_n(this->begin(), Size - 1, response.begin());
    std::ranges::copy_n(other.begin(), SSize, response.begin() + Size - 1);
    return response;
  };
  inline constexpr ConstexprString(const ConstexprString&) = default; 
  inline constexpr ConstexprString(ConstexprString&&) = default; 
};

template <std::size_t N>
inline constexpr auto operator<<(std::ostream& stream, const ConstexprString<N>& str) -> std::ostream& {
  stream << static_cast<std::string_view>(str);
  return stream;
};

template <std::size_t Count>
inline constexpr auto CreateStringWith(char c) {
  ConstexprString<Count + 1> str = {};
  for(std::size_t i = 0; i < Count; i++) {
    str.data[i] = c;
  }; 
  str.data[Count] = '\0';
  return str;
};


template <std::size_t Size>
ConstexprString(const char (&data)[Size]) -> ConstexprString<Size>;
} // namespace utempl

