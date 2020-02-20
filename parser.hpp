#ifndef CATCH_TEST_PARSER_HPP
#define CATCH_TEST_PARSER_HPP
#include <cstring>
#include <functional>
#include <stdio.h>
#include <string>
#include <system_error>

namespace rpa {
/**
 * @brief structure that will contain the parsed values, members are self
 * explanatory
 *
 */
struct info {
  unsigned int pid;
  float cpu;
  float mem;
  std::string command;
};

/**
 * @brief parser tailored for ps command.
 *
 */
class ps_parser {
  /**
   * @brief enum utilized within class to enhance readability for positions
   *
   */
  enum positions { pid, cpu, mem, command, max };

  FILE *source;
  static const size_t buffer_size = 1024;
  int positions[max];
  static char const *monitored[max];

public:
  /**
   * @brief set source input, the first line is read to set positions.
   *
   * @param file the source input. suggested input pipe output from ps or a file
   * for testing
   */
  void setSource(FILE *file) {
    source = file;
    if (source == nullptr || ferror(source))
      throw std::system_error(
          std::error_code(ferror(source), std::system_category()),
          "setSource error");
    // get first line to get positions
    line([this](char *token, int index) {
      for (int i = 0; i < max; ++i) {
        if (std::strcmp(token, ps_parser::monitored[i]) == 0)
          this->positions[i] = index;
      }
    });
  }

  /**
   * @brief read the next line of of source parsing the output
   *
   * @return info data from the line (see \ref info struct)
   */
  info line() const noexcept {
    info res;
    // parse the line elements accordingly to their type
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

  /**
   * @brief return false if the source is in a invalid state
   *
   * @return true if source is valid hence another line can be read
   * @return false if reached end of file or an error occurred
   */
  explicit operator bool() const noexcept {
    return ferror(source) == 0 && feof(source) == 0;
  }

  void close(const std::function<void(FILE *)> &close_f) noexcept {
    close_f(source);
  }

  ~ps_parser() {}

private:
  /**
   * @brief parse the next line applaying fun to each token, token are divided
   * by ' '
   *
   * @param fun a function to apply to each token
   */
  void line(const std::function<void(char *, int)> &fun) const noexcept;
};
} // namespace rpa
#endif
