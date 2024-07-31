module;
#include <gtest/gtest.h>
export module tests.go_interface;
import utempl.go_interface;

namespace utempl {

struct SomeInterface {
  int field;
};

struct SomeStruct {
  int field;
  friend auto operator==(const SomeInterface& a, const SomeStruct& b) -> bool {
    return false;
  };
};

struct SomeStruct2 {
  int field;
};

TEST(GoInterface, Basic) {
  GoInterface<SomeInterface> obj(SomeStruct{1});
  EXPECT_EQ(obj.field, 1);
};

TEST(GoInterface, Equal) {
  GoInterface<SomeInterface> obj(SomeStruct{1});
  EXPECT_EQ(obj, GoInterface{SomeInterface{1}});
  EXPECT_EQ(obj, SomeInterface{1});
  EXPECT_NE(obj, SomeStruct{1});
  EXPECT_EQ(obj, SomeStruct2{1});
};
}  // namespace utempl
