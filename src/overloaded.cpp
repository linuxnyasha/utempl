export module utempl.overloaded;
import std;

namespace utempl {
export template <typename... Fs>
constexpr auto Overloaded(Fs&&... fs) {
  struct Overloaded : public std::remove_cvref_t<Fs>... {
    using Fs::operator()...;
  };
  return Overloaded{std::forward<Fs>(fs)...};
};

}  // namespace utempl
