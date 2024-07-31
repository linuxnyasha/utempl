import utempl.utils;
import utempl.tuple;
import std;

auto main() -> int {
  auto tuple = Reverse(utempl::Tuple{4, 3, 2, 1});
  ForEach(tuple, [](auto arg) {
    std::cout << arg << std::endl;
  });
};
