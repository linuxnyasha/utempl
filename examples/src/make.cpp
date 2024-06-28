#include <utempl/utils.hpp>

auto main() -> int {
  constexpr auto arr = utempl::TupleMaker<std::array<void*, 0>>::Make();
};
