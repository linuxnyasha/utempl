module;
#include <gtest/gtest.h>
export module tests.overloaded;
import utempl.overloaded;

namespace utempl {

TEST(Overloaded, Basic) {
  constexpr auto f = Overloaded(
      [](int) {
        return 1;
      },
      [](auto&&) {
        return 2;
      });
  EXPECT_EQ(f(1), 1);
  EXPECT_EQ(f(""), 2);
};

}  // namespace utempl
