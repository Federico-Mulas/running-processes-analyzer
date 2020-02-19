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

bool end_with(std::string const &search_string, std::string const &ending) {
    if (search_string.length() >= ending.length()) {
        return (0 == search_string.compare (search_string.length() - ending.length(), ending.length(), ending));
    }
    return false;
}

} // namespace rpa
#endif
