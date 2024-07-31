import utempl.utils;
import std;

auto main() -> int {
  constexpr auto f = utempl::Curry([](auto... args) {
    return (0 + ... + args);
  });
  std::cout << f(1)(2)(3) << std::endl;  // Call on cast to return type
};
