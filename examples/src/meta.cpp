import utempl.meta_info;
import std;

auto main() -> int {
  constexpr std::array types = {utempl::kTypeId<int>, utempl::kTypeId<void>};
  static_assert(std::is_same_v<utempl::GetMetaInfo<types[0]>::Type, int>);
  static_assert(std::is_same_v<utempl::GetMetaInfo<types[1]>::Type, void>);
};
