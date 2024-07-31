# uTempl - Modern C++ Template Library

## Features
### Ranges Like Interface For TupleLike
```cpp
import utempl;

auto main() -> int {
  using namespace utempl;
  constexpr TupleLike auto tuple = Tuple{1, 2, 3, 4, 5, 6}
    | Take<5>()
    | Map([](int arg){return arg + 1;})
    | Map([](int arg) -> float {return arg;})
    | Reverse()
    | Take<3>(); // Lazy evaluate
  static_assert(tuple == Tuple{6.0f, 5.0f, 4.0f});
};
```
### Storage types in array 
```cpp
import utempl;
import std;

auto main() -> int {
  using namespace utempl;
  constexpr std::array types{kTypeId<int>, kTypeId<void>};
  static_assert(std::is_same_v<GetMetaInfo<types[0]>::Type, int>);
  static_assert(std::is_same_v<GetMetaInfo<types[1]>::Type, void>);
  static_assert(std::is_same_v<decltype(GetTypeListForTag()), TypeList<int, void>>);
};
```

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
