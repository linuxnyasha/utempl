import fmt;
import utempl.go_interface;
import std;

struct SomeInterfaceImpl {
  int a;
  int b;
};
using SomeInterface = utempl::GoInterface<SomeInterfaceImpl>;
struct SomeStruct {
  std::int16_t a;
  std::int16_t b;
};

inline auto Func(SomeInterface arg) {
  fmt::print("{} {}\n", arg.a, arg.b);
};

auto main() -> int {
  Func(SomeStruct{42, 300});  // NOLINT
};
