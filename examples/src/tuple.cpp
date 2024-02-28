#include <utempl/tuple.hpp>
#include <utempl/utils.hpp>
#include <iostream>
#include <cassert>

auto main() -> int {
  using utempl::literals::operator""_c;
  constexpr utempl::Tuple tuple = utempl::Tuple{42, 3.141500, "Hello World"};
  utempl::Tuple<int> tuple2{{}}; 
  std::ignore = tuple2;
  std::cout << utempl::kTupleSize<decltype(tuple)> << std::endl; // Get tuple size
  std::cout << tuple[0_c] << std::endl; // Get element using [] with literal
  auto newTuple = utempl::Transform(tuple, utempl::Overloaded(
    [](auto arg){
      return std::to_string(arg);
    },
    [](const char* arg) {
      return std::string(arg);
    }
  ));
  auto flag = newTuple == utempl::Tuple<std::string, std::string, std::string>{"42", "3.141500", "Hello World"};
  assert(flag);
};
