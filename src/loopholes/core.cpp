export module utempl.loopholes.core;
import std;

namespace utempl::loopholes {

export {
  template <auto I>
  struct Getter {
    friend constexpr auto Magic(Getter<I>);
  };
  template <auto I, auto Value = 0>
  struct Injector {
    friend constexpr auto Magic(Getter<I>) {
      return Value;
    };
  };
}
template <auto, typename = void, typename... Ts>
struct InjectedImpl {
  static constexpr bool value = false;
};
template <auto V, typename... Ts>
struct InjectedImpl<V, std::void_t<decltype(Magic(Getter<V>{}))>, Ts...> {
  static constexpr bool value = true;
};

export template <auto I, typename... Ts>
concept Injected = InjectedImpl<I, void, Ts...>::value;

}  // namespace utempl::loopholes
