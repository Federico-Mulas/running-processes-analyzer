#ifndef CATCH_TEST_ARGUMENTS_HPP
#define CATCH_TEST_ARGUMENTS_HPP
#include <gflags/gflags.h>
#include <string>
#include <filesystem>

DEFINE_int32(probe_frequency, 5, "probe frequency of processes details in seconds (default 5s)");
DEFINE_int32(config_probe_frequency, 60, "probe frequency of configs from endpoints in seconds (default 60s)");
DEFINE_string(config_file, "", "file containing endpoints to query for configs. It must contain 1 endpoint per line, if more then one line is present the endpoint are queried circurally (eg. if 2 are provided: 1st, 2nd, 1st, ...)");
DEFINE_string(output_file, "", "output file, by default print on standard output");

inline bool file_validator(const char *, const std::string &value)
{
    // empty string is the default value
    return value == "" || std::filesystem::exists(std::filesystem::path(value));
}
DEFINE_validator(config_file, &file_validator);

#endif
