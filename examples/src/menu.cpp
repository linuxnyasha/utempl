import utempl.menu;
import std;

auto main() -> int {
  utempl::menu::Menu{}
      .With<{"This is 0"}>([] {
        std::cout << "You entered 0" << std::endl;
      })
      .With<{"Some Long", "S"}>([] {
        std::cout << "It aligns the output to the longest element" << std::endl;
      })
      .Run<"[{0}]{2} - |{1}|\n">();
};
