import utempl;
import std;

auto main() -> int {
  constexpr auto arr = utempl::TupleMaker<std::array<void*, 0>>::Make();
};
