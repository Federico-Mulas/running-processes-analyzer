#ifndef CATCH_TEST_PARSER_HPP
#define CATCH_TEST_PARSER_HPP
#include <cstring>
#include <functional>
#include <stdio.h>
#include <string>
#include <system_error>

namespace rpa {

struct info {
  unsigned int pid;
  float cpu;
  float mem;
  std::string command;
};

class ps_parser {
  enum positions { pid, cpu, mem, command, max };

  FILE *out;
  static const size_t buffer_size = 1024;
  int positions[max];
  static char const *monitored[max];

public:
  ps_parser() {}

  void setSource(FILE *file) {
    out = file;
    if (out == nullptr || ferror(out))
      throw std::system_error(
          std::error_code(ferror(out), std::system_category()),
          "setSource error");
    // get first line to get positions
    line([this](char *token, int index) {
      for (int i = 0; i < max; ++i) {
        if (std::strcmp(token, ps_parser::monitored[i]) == 0)
          this->positions[i] = index;
      }
    });
  }

  info line() {
    info res;
    line([this, &res](char *token, int index) {
      if (index == this->positions[pid]) {
        res.pid = std::stoi(token);
        return;
      }
      if (index == this->positions[cpu]) {
        res.cpu = std::stof(token);
        return;
      }

      if (index == this->positions[mem]) {
        res.mem = std::stof(token);
        return;
      }

      if (index == this->positions[command]) {
        res.command = token;
        return;
      }
    });
    return res;
  }

  explicit operator bool() { return feof(out) == 0; }

  ~ps_parser() { pclose(out); }

private:
  void line(const std::function<void(char *, int)> &fun) {
    char buff[buffer_size]; // a token should never exceed 255 (path length)
    int i = 0;
    int token_n = 0;
    for (int c = fgetc(out); c != EOF && c != '\n'; c = fgetc(out)) {
      if (c == ' ') {
        if (i != 0) {
          buff[i] = 0;
          fun(buff, token_n);
          token_n++;
          i = 0;
        }
        continue;
      }
      buff[i] = c;
      i++;
    }
    buff[i] = 0;
    fun(buff, token_n);
  }
};

char const *ps_parser::monitored[max] = {"PID", "%CPU", "%MEM", "COMMAND"};
} // namespace rpa
#endif
