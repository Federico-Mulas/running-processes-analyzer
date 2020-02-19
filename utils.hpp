#ifndef CATCH_TEST_UTILS_HPP
#define CATCH_TEST_UTILS_HPP
#include <algorithm>
#include <cctype>
#include <string>

namespace rpa {
namespace {
inline char to_lower_resolver(char c) { return std::tolower(c); }
} // namespace

inline void to_lower(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(), to_lower_resolver);
}
} // namespace rpa
#endif
