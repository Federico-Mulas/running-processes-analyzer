#ifndef CATCH_TEST_UTILS_HPP
#define CATCH_TEST_UTILS_HPP
#include <algorithm>
#include <cctype>
#include <string>

namespace rpa {
namespace {
/**
 * @brief wrapper for disambiguate tolower
 *
 * @param c the char to lower
 * @return char the lowered char
 */
inline char to_lower_resolver(char c) noexcept { return std::tolower(c); }
} // namespace

/**
 * @brief lower down the entair passed string
 *
 * @param s the string to lower, it works inplace
 * @return the string passed as parameter to permit chaining
 */
inline std::string &to_lower(std::string &s) noexcept {
  std::transform(s.begin(), s.end(), s.begin(), to_lower_resolver);
  return s;
}

/**
 * @brief search the end of the string for the given ending string
 *
 * @param search_string the string in which the search will be performed
 * @param ending the string to search
 * @return true if \p ending is at the end of \p search_string
 * @return false otherwise
 */
bool end_with(const std::string &search_string, const std::string &ending) {
  if (search_string.length() >= ending.length()) {
    return (0 == search_string.compare(search_string.length() - ending.length(),
                                       ending.length(), ending));
  }
  return false;
}

} // namespace rpa
#endif
