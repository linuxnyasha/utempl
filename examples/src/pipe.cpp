#include <utempl/utils.hpp>

auto main() -> int {
  using namespace utempl;
  constexpr TupleLike auto tuple = Tuple{1, 2, 3, 4, 5, 6}
    | Take<5>()
    | Map([](int arg){return arg + 1;})
    | Map([](int arg) -> float {return arg;})
    | Reverse()
    | Take<3>(); // Lazy evaluate
  static_assert(tuple == Tuple{6.0f, 5.0f, 4.0f});
};
