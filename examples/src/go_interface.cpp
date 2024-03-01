#include <utempl/go_interface.hpp>
#include <fmt/core.h>

struct SomeInterfaceImpl {
  int a;
  int b;
};
using SomeInterface = utempl::GoInterface<SomeInterfaceImpl>;
struct SomeStruct {
  short a;
  short b;
};

inline auto Func(SomeInterface arg) {
  fmt::print("{} {}\n", arg.a, arg.b);
};


auto main() -> int {
  Func(SomeStruct{42, 300});
};
