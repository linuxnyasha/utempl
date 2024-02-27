#include <gtest/gtest.h>
#include <utempl/menu.hpp>

namespace utempl {

TEST(Menu, Basic) {
  testing::internal::CaptureStdout();
  std::istringstream stream("t\n");
  int value = 0;
  menu::Menu{}
    .With<{"t", "This is t"}>([&]{
      std::cout << "Success!" << std::endl;
      value = 1;
    })
  .Run<"[{0}]{2} - ({1})\n">(stream);
  auto captured = testing::internal::GetCapturedStdout();
  EXPECT_EQ(captured, "[t] - (This is t)\n|> Success!\n");
  EXPECT_EQ(value, 1);
};

} // namespace utempl
