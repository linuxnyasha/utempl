#pragma once
#include <utempl/optional.hpp>
#include <utempl/constexpr_string.hpp>
#include <utempl/tuple.hpp>
#include <utempl/utils.hpp>
#include <iostream>
#include <array>
#include <fmt/format.h>
#include <fmt/compile.h>


namespace utempl {

constexpr std::size_t CountDigits(std::size_t num) {
  std::size_t count = 0;
  do {
    ++count;
    num /= 10;
  } while (num != 0);
  return count;
};

constexpr std::size_t GetDigit(std::size_t num, std::size_t index) {
  for (std::size_t i = 0; i < index; ++i) {
    num /= 10;
  }
  return num % 10;
};

template <std::size_t num>
consteval auto ToString() {
  constexpr std::size_t digits = CountDigits(num);
  return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
    return ConstexprString{std::array{static_cast<char>('0' + GetDigit(num, digits - 1 - Is))..., '\0'}};
  }(std::make_index_sequence<digits>());
};

template <typename Range>
constexpr auto GetMax(Range&& range) {
  std::remove_cvref_t<decltype(range[0])> response = 0;
  for(const auto& element : range) {
    response = element > response ? element : response;
  };
  return response;
};


namespace menu {

namespace impl {

template <std::size_t N1, std::size_t N2>
struct CallbackMessage {
  ConstexprString<N1> message;
  Optional<ConstexprString<N2>> need;
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



template <Tuple storage = Tuple{}, typename... Fs>
struct Menu {
  Tuple<Fs...> functionStorage;

  static consteval auto GetMaxSize() -> std::size_t {
    return [&]<auto... Is>(std::index_sequence<Is...>){
      constexpr auto list = ListFromTuple(storage); 
      return GetMax(std::array{(std::remove_cvref_t<decltype(*std::declval<decltype(Get<Is>(list))>().need)>::kSize != 0 ? std::remove_cvref_t<decltype(*std::declval<decltype(Get<Is>(list))>().need)>::kSize : CountDigits(Is))...});
    }(std::index_sequence_for<Fs...>());
  };
  template <impl::CallbackMessage message, std::invocable F>
  constexpr auto With(F&& f) const {
    return Menu<storage + Tuple{message}, Fs..., std::remove_cvref_t<F>>{.functionStorage = this->functionStorage + Tuple(std::forward<F>(f))};
  };
  template <ConstexprString fmt, ConstexprString enter = "|> ">
  inline constexpr auto Run(std::istream& in = std::cin, std::FILE* out = stdout) const {
    using Cleared = std::remove_cvref_t<decltype(*this)>;
    constexpr auto maxSize = Cleared::GetMaxSize();
    [&]<auto... Is>(std::index_sequence<Is...>){
      constexpr auto message = ([&]<auto I>(Wrapper<I>){
        constexpr auto message = Get<I>(storage);
        constexpr std::size_t s = maxSize - (message.need ? message.need->size() : CountDigits(I));
        constexpr auto str3 = CreateStringWith<s>(' ');
        constexpr auto str2 = message.message; 
        constexpr auto str1 = [&] {
          if constexpr(message.need) {
            return *message.need;
          } else {
            return ToString<I>();            
          };
        }();
        // + 1 - NULL Terminator
        constexpr auto size = fmt::formatted_size(FMT_COMPILE(fmt.data.begin())
          ,str1
          ,str2
          ,str3) + 1;
        char data[size] = {};
        fmt::format_to(data, FMT_COMPILE(fmt.begin())
          ,str1
          ,str2
          ,str3);
        return ConstexprString<size>(data);
      }(Wrapper<Is>{}) + ...) + enter;

      std::fwrite(message.begin(), 1, message.size(), out);
      std::fflush(out);
      std::string input;
      std::getline(in, input);
      ([&]<auto I, impl::CallbackMessage message = Get<I>(storage)>(Wrapper<I>) {
        if constexpr(message.need) {
          if(*message.need == input) {
            Get<I>(this->functionStorage)();
          };
        } else {
          if(ToString<I>() == input) {
            Get<I>(this->functionStorage)();
          };
        }; 
      }(Wrapper<Is>{}), ...);
    }(std::index_sequence_for<Fs...>());
  };
};

} // namespace menu
} // namespace utempl


