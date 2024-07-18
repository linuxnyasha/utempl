#include <utempl/utils.hpp>

auto main() -> int {
  constexpr auto value = utempl::LeftFold(
      utempl::GetIndexesTuple<4>(), utempl::kWrapper<std::size_t{}>, []<std::size_t I>(utempl::Wrapper<I>, auto) -> utempl::Wrapper<I + 1> {
        return {};
      });
  static_assert(std::is_same_v<decltype(value), const utempl::Wrapper<static_cast<std::size_t>(4)>>);
};
