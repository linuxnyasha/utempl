#include <utempl/utils.hpp>

auto main() -> int {
  static_assert(utempl::Switch<int>(utempl::Tuple{2, 1, 0}, utempl::Tuple{0, 1, 2}, 0, [](int value) {
                  return value + 1;
                }) == 3);
  static_assert(utempl::Switch<int>(utempl::Tuple{2, 1, 0}, utempl::Tuple{0, 1, 2}, 3, [](int value) {
                  return value + 1;
                }) == 0);
  static_assert(utempl::Switch<std::optional<int>>(utempl::Tuple{2, 1, 0}, utempl::Tuple{0, 1, 2}, 3, [](int value) {
                  return value + 1;
                }) == std::nullopt);
  static_assert(utempl::Switch<int>(
                    utempl::Tuple{2, 1, 0},
                    utempl::Tuple{0, 1, 2},
                    3,
                    [](int value) {
                      return value + 1;
                    },
                    [] {
                      return 42;
                    }) == 42);
};
