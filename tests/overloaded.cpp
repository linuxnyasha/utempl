#include <gtest/gtest.h>
#include <utempl/overloaded.hpp>

namespace utempl {

TEST(Overloaded, Basic) {
  constexpr auto f = utempl::Overloaded([](int){
    return 1;
  }, [](auto&&){
    return 2;
  });
  EXPECT_EQ(f(1), 1);
  EXPECT_EQ(f(""), 2);
};

} // namespace utempl
