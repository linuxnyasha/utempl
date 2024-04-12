#include <utempl/utils.hpp>

struct Container {
  float data{};
};

auto main() -> int {
  using namespace utempl;
  constexpr auto value = Tuple{1, 2, 3, 4, 5, 6}
    | Take<5>()
    | Map([](int arg) {return arg + 1;})
    | Map([](int arg) -> float {return arg;})
    | Map([](float arg) -> Container {return {.data = arg};})
    | Reverse()
    | Take<3>()
    | Reduce(0.f, [](auto accumulator, Container arg) -> float {
      return accumulator + arg.data;
    }); // Lazy evavalue
  static_assert(value == 15.0f);
};
