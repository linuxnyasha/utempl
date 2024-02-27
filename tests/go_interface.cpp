#include <gtest/gtest.h>
#include <utempl/go_interface.hpp>

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
  utempl::GoInterface<SomeInterface> obj(SomeStruct{1});
  EXPECT_EQ(obj.field, 1);
};

TEST(GoInterface, Equal) {
  utempl::GoInterface<SomeInterface> obj(SomeStruct{1});
  EXPECT_EQ(obj, utempl::GoInterface{SomeInterface{1}});
  EXPECT_EQ(obj, SomeInterface{1});
  EXPECT_NE(obj, SomeStruct{1});
  EXPECT_EQ(obj, SomeStruct2{1});
};
