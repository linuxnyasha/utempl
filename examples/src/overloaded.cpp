import utempl.overloaded;
import std;

auto main() -> int {
  utempl::Overloaded([](auto&& arg) {},
                     [](int arg) {
                       std::cout << arg << std::endl;
                     })(42);
};
