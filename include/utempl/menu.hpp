#include <iostream>
#include <array>
#include <algorithm>
#include <optional>
#include <concepts>
#include <fmt/format.h>
#include <fmt/compile.h>


namespace utempl::utils {
  template <std::size_t>
  struct ConstexprString;
};

template <std::size_t Size>
struct fmt::formatter<utempl::utils::ConstexprString<Size>> : public fmt::formatter<std::string_view> {
  constexpr auto parse(format_parse_context& ctx) const {
    return ctx.begin();
  };
  inline constexpr auto format(const utempl::utils::ConstexprString<Size>& str, auto& ctx) const {
    return fmt::formatter<std::string_view>::format({str.data.begin()}, ctx);
  };
};

namespace utempl {

namespace utils {

template <std::size_t Size>
struct ConstexprString {
  std::array<char, Size> data;
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
  inline constexpr explicit operator std::string_view() const {
    return {this->data.begin()};
  };
  inline constexpr bool operator==(std::string_view other) const {
    return static_cast<std::string_view>(*this) == other;
  };
  template <std::size_t SSize>
  inline constexpr auto operator+(const ConstexprString<SSize>& other) -> ConstexprString<Size + SSize - 1> {
    ConstexprString<Size + SSize - 1> response;
    std::copy_n(this->data.begin(), Size - 1, response.data.begin());
    std::copy_n(other.data.begin(), SSize, response.data.begin() + Size - 1);
    return response;
  };
  inline constexpr ConstexprString(const ConstexprString&) = default; 
  inline constexpr ConstexprString(ConstexprString&&) = default; 
};

template <std::size_t Count>
consteval auto createStringWith(char c) {
  ConstexprString<Count + 1> str = {};
  for(std::size_t i = 0; i < Count; i++) {
    str.data[i] = c;
  }; 
  str.data[Count] = '\0';
  return str;
};


template <std::size_t Size>
ConstexprString(const char (&data)[Size]) -> ConstexprString<Size>;


template <auto>
struct Wrapper {};

constexpr std::size_t countDigits(std::size_t num) {
  std::size_t count = 0;
  do {
    ++count;
    num /= 10;
  } while (num != 0);
  return count;
};

constexpr std::size_t getDigit(std::size_t num, std::size_t index) {
  for (std::size_t i = 0; i < index; ++i) {
    num /= 10;
  }
  return num % 10;
};

template <std::size_t num>
consteval auto toString() {
  constexpr std::size_t digits = countDigits(num);
  return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    return utils::ConstexprString{std::array{static_cast<char>('0' + getDigit(num, digits - 1 - Is))..., '\0'}};
  }(std::make_index_sequence<digits>());
};

template <typename Range>
constexpr auto getMax(Range&& range) {
  std::remove_cvref_t<decltype(range[0])> response = 0;
  for(const auto& element : range) {
    response = element > response ? element : response;
  };
  return response;
};

struct Caster {
  constexpr Caster(auto&&) {};
};


template <typename... Ts>
struct TypeList {
};
template <typename... Ts, typename... TTs>
consteval auto operator==(const TypeList<Ts...>& first, const TypeList<TTs...>& second) -> bool {
  return std::is_same_v<decltype(first), decltype(second)>;
};
template <std::size_t... Is, typename T>
consteval auto get(std::index_sequence<Is...>, decltype(Caster(Is))..., T, ...) -> T; 


template <std::size_t I, typename... Ts>
consteval auto get(const TypeList<Ts...>&) -> decltype(get(std::make_index_sequence<I>(), std::declval<Ts>()...)) requires (I < sizeof...(Ts));




template <auto, typename T>
struct TupleLeaf {
  T value;
  template <typename TT>
  inline constexpr TupleLeaf(TT&& arg) : value(std::forward<TT>(arg)) {};
  inline constexpr TupleLeaf(const TupleLeaf&) = default;
  inline constexpr TupleLeaf(TupleLeaf&&) = default;
  inline constexpr bool operator==(const TupleLeaf&) const = default;
};

template <std::size_t I, typename... Ts>
struct TupleHelper {
  consteval TupleHelper() = default;
  inline constexpr TupleHelper(const TupleHelper&) = default;
  inline constexpr TupleHelper(TupleHelper&&) = default;
  inline constexpr bool operator==(const TupleHelper&) const = default;
};

template <std::size_t I, typename T, typename... Ts>
struct TupleHelper<I, T, Ts...> : public TupleLeaf<I, T> , public TupleHelper<I + 1, Ts...> {
  template <typename TT, typename... TTs>
  inline constexpr TupleHelper(TT&& arg, TTs&&... args) : 
    TupleLeaf<I, T>{std::forward<TT>(arg)},
    TupleHelper<I + 1, Ts...>{std::forward<TTs>(args)...} {};
  inline constexpr TupleHelper(const TupleHelper&) = default;
  inline constexpr TupleHelper(TupleHelper&&) = default;
  inline constexpr bool operator==(const TupleHelper&) const = default;
};

template <typename... Ts>
struct Tuple;

template <std::size_t I, typename... Ts>
inline constexpr auto get(Tuple<Ts...>& tuple) -> auto& requires (I < sizeof...(Ts)) {
  using Type = decltype(get<I>(TypeList<Ts...>{}));
  return static_cast<TupleLeaf<I, Type>&>(tuple).value;
};

template <std::size_t I, typename... Ts>
inline constexpr auto get(const Tuple<Ts...>& tuple) -> const auto& requires (I < sizeof...(Ts)) {
  using Type = decltype(get<I>(TypeList<Ts...>{}));
  return static_cast<const TupleLeaf<I, Type>&>(tuple).value;
};

template <std::size_t I, typename... Ts>
inline constexpr auto get(Tuple<Ts...>&& tuple) -> auto&& requires (I < sizeof...(Ts)) {
  using Type = decltype(get<I>(TypeList<Ts...>{}));
  return std::move(static_cast<TupleLeaf<I, Type>&&>(tuple).value);
};

template <typename... Ts>
struct Tuple : public TupleHelper<0, Ts...> {
  template <typename... TTs>
  inline constexpr Tuple(TTs&&... args) : 
    TupleHelper<0, Ts...>(std::forward<TTs>(args)...) {};
  inline constexpr Tuple(const Tuple&) = default;
  inline constexpr Tuple(Tuple&&) = default;
  inline constexpr bool operator==(const Tuple&) const = default;
  template <typename... TTs>
  inline constexpr auto operator+(const Tuple<TTs...>& other) const -> Tuple<Ts..., TTs...> {
    return [&]<auto... Is, auto... IIs>(std::index_sequence<Is...>, std::index_sequence<IIs...>) -> Tuple<Ts..., TTs...> {
      return {get<Is>(*this)..., get<IIs>(other)...};
    }(std::make_index_sequence<sizeof...(Ts)>(), std::make_index_sequence<sizeof...(TTs)>());
  };
};

template <typename... Ts>
Tuple(Ts&&...) -> Tuple<std::remove_cvref_t<Ts>...>;


template <typename... Ts>
consteval auto listFromTuple(const utils::Tuple<Ts...>&) -> utils::TypeList<Ts...> {
  return {};
};
template <typename>
struct TupleSize {};

template <typename... Ts>
struct TupleSize<Tuple<Ts...>> {
  static constexpr auto value = sizeof...(Ts);
};
template <typename Tuple>
inline constexpr auto kTupleSize = TupleSize<Tuple>::value;



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



} // namespace utils

namespace menu {

namespace impl {

template <std::size_t N1, std::size_t N2>
struct CallbackMessage {
  utils::ConstexprString<N1> message;
  utils::Optional<utils::ConstexprString<N2>> need;
  consteval CallbackMessage(const char (&need)[N2], const char (&message)[N1]) : 
    message(std::move(message))
    ,need(std::move(need)) {};
  consteval CallbackMessage(const char (&message)[N1]) : 
    message(std::move(message))
    ,need(std::nullopt) {};
};
template <std::size_t N1, std::size_t N2>
CallbackMessage(const char(&)[N1], const char(&)[N2]) -> CallbackMessage<N2, N1>;

template <std::size_t N1>
CallbackMessage(const char(&)[N1]) -> CallbackMessage<N1, 0>;

} // namespace impl



template <utils::Tuple storage = utils::Tuple{}, typename... Fs>
struct Menu {
  utils::Tuple<Fs...> functionStorage;
  static constexpr auto kMessages = storage;
  static consteval auto getMaxSize() -> std::size_t {
    return [&]<auto... Is>(std::index_sequence<Is...>){
      constexpr auto list = utils::listFromTuple(storage); 
      return utils::getMax(std::array{(std::remove_cvref_t<decltype(*std::declval<decltype(get<Is>(list))>().need)>::kSize != 0 ? std::remove_cvref_t<decltype(*std::declval<decltype(get<Is>(list))>().need)>::kSize : utils::countDigits(Is))...});
    }(std::make_index_sequence<sizeof...(Fs)>());
  };
  template <impl::CallbackMessage message, std::invocable F>
  constexpr auto With(F&& f) const {
    return Menu<storage + utils::Tuple{message}, Fs..., std::remove_cvref_t<F>>{.functionStorage = this->functionStorage + utils::Tuple(std::forward<F>(f))};
  };
};

template <utils::ConstexprString fmt, utils::ConstexprString enter = "|> ", typename Menu>
inline auto Run(Menu&& menu) {
  using Cleared = std::remove_cvref_t<Menu>;
  constexpr auto maxSize = Cleared::getMaxSize();
  constexpr auto messagesCount = utils::kTupleSize<std::remove_cv_t<decltype(Cleared::kMessages)>>;
  [&]<auto... Is, auto messages = Cleared::kMessages>(std::index_sequence<Is...>){
    constexpr auto message = ([&]<auto I>(utils::Wrapper<I>){
      constexpr auto message = get<I>(messages);
      constexpr std::size_t s = maxSize - (message.need ? message.need->size() : utils::countDigits(I));
      constexpr auto str3 = utils::createStringWith<s>(' ');
      constexpr auto str2 = message.message; 
      constexpr auto str1 = [&] {
        if constexpr(message.need) {
          return *message.need;
        } else {
          return utils::toString<I>();            
        };
      }();
      // + 1 - NULL Terminator
      constexpr auto size = fmt::formatted_size(FMT_COMPILE(fmt.data.begin())
        ,str1
        ,str2
        ,str3) + 1;
      char data[size] = {};
      fmt::format_to(data, FMT_COMPILE(fmt.data.begin())
        ,str1
        ,str2
        ,str3);
      return utils::ConstexprString<size>(data);
    }(utils::Wrapper<Is>{}) + ...) + enter;

    std::fwrite(message.data.data(), 1, message.size(), stdout);
    std::fflush(stdout);
    std::string input;
    std::getline(std::cin, input);
    ([&]<auto I, impl::CallbackMessage message = get<I>(messages)>(utils::Wrapper<I>) {
      if constexpr(message.need) {
        if(input == std::string_view(*message.need)) {
          get<I>(menu.functionStorage)();
        };
      } else {
        if(input == std::string_view(utils::toString<I>())) {
          get<I>(menu.functionStorage)();
        };
      }; 
    }(utils::Wrapper<Is>{}), ...);
  }(std::make_index_sequence<messagesCount>());

};


} // namespace menu
} // namespace utempl


