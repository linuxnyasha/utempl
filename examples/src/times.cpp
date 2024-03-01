#include <utempl/utils.hpp>
#include <iostream>

auto main() -> int {
  std::size_t i = 0;
  utempl::Times<10>([&](){
    ++i;
    std::cout << i << std::endl;
  });
};
