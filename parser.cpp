#include "./parser.hpp"

namespace rpa {
char const *ps_parser::monitored[max] = {"PID", "%CPU", "%MEM", "COMMAND"};

void ps_parser::line(const std::function<void(char *, int)> &fun) const
    noexcept {
  char buff[buffer_size]; // a token should never exceed 255 (path length)
  int token_len = 0;
  int token_n = 0;
  for (int c = fgetc(source); c != EOF && c != '\n'; c = fgetc(source)) {
    if (c == ' ') {
      // split
      if (token_len != 0) {
        buff[token_len] = 0;
        fun(buff, token_n);
        token_n++;
        token_len = 0;
      }
      continue;
    }
    buff[token_len] = c;
    token_len++;
  }
  buff[token_len] = 0;
  fun(buff, token_n);
}
} // namespace rpa