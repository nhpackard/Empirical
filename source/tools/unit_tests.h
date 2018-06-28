/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file unit_tests.h
 *  @brief Macros to facilitate unit testing.
 *  @note Status: technically DEPRECATED (now using Catch, but may revert back)
 */


#ifndef EMP_UNIT_TESTS_H
#define EMP_UNIT_TESTS_H

#include <iostream>
#include <sstream>

#include "../config/command_line.h"
#include "../tools/string_utils.h"

namespace emp {

  // Unit tests verbosity levels:
  enum class UnitTestOutput {
    SILENT = 0,    // Just return error code.
    NORMAL = 1,    // Print errors and summary.
    VERBOSE = 2    // Print results for each test performed.
  };

  static size_t & UnitTestVerbose() {
    static size_t verbose = 1;
    return verbose;
  }

  static size_t & UnitTestVerbose(size_t in_verbose) {
    return (UnitTestVerbose() = in_verbose);
  }

  static size_t & UnitTestErrors() {
    static size_t errors = 0;
    return errors;
  }


  void ResolveUnitTest(bool pass, const std::string & test_input,
                   const std::string & result, const std::string & exp_result,
                   const std::string & filename, size_t line_num) {
    const size_t verbose = emp::UnitTestVerbose();
    if (verbose == 2 || !pass) {
      std::cout << filename << ", line " << line_num << ": "
                << test_input << " == " << result << std::endl;
    }
    if (verbose >= 1 && !pass) {
      std::cout << "-> \033[1;31mMATCH FAILED!  Expected: "
                << exp_result << "\033[0m" << std::endl;
      emp::UnitTestErrors()++;
    } else if (verbose == 2) {
      std::cout << "-> \033[1;32mPASSED!" << "\033[0m" << std::endl;
    }
  }
}

///  Input:  A macro call and a strings indicating the expected result.
///  Output: Code that tests of the macro result matches the expected results, and optionally
///          print it (if in verbose mode or if the macro fails to produce the expected result.)

#define EMP_TEST_MACRO( MACRO, EXP_RESULT )                                       \
  do {                                                                            \
    std::string result = std::string(EMP_STRINGIFY( MACRO ));                     \
    bool match = (result == EXP_RESULT);                                          \
    emp::ResolveUnitTest(match, #MACRO, result, #EXP_RESULT, __FILE__, __LINE__); \
  } while (false)


///  Take in an expression and a string representing the expected result; verify that the
///  stringifyied result is the expected string AND print this info if -v flag is set.
///     Input:  An expression and an expected evaluation.
///     Output: Code to evaluate the expression and optionally print it (if either in verbose mode
///             or the macro does not produce the expected result).

#define EMP_TEST_VALUE( VALUE, EXP_RESULT )                                           \
  do {                                                                                \
    auto result = VALUE;                                                              \
    bool match = (result == (EXP_RESULT));                                            \
    std::string result_str = emp::to_string(result);                                  \
    emp::ResolveUnitTest(match, #VALUE, result_str, #EXP_RESULT, __FILE__, __LINE__); \
  } while (false)


#define emp_test_main                                                        \
  emp_main_function();                                                       \
  int main(int argc, char * argv[]) {                                        \
    auto args = emp::cl::args_to_strings(argc, argv);                        \
    size_t & verbose = emp::UnitTestVerbose();                               \
    if (emp::cl::use_arg(args, "--help")) {                                  \
      std::cout << "Usage: \033[1;36m" << args[0] << " [args]\033[0m\n"      \
        << "  \033[1m--help\033[0m    : This message.\n"                           \
        << "  \033[1m--silent\033[0m  : Produce no output except result code.\n"   \
        << "  \033[1m--verbose\033[0m : Produce detailed output for each test.\n"; \
      exit(0);                                                               \
    }                                                                        \
    if (emp::cl::use_arg(args, "--verbose")) verbose = 2;                    \
    if (emp::cl::use_arg(args, "--silent")) verbose = 0;                     \
                                                                             \
    emp_main_function();                                                     \
    int num_errors = emp::UnitTestErrors();                                  \
    if (verbose>=1) {                                                        \
      if (emp::UnitTestErrors()) {                                           \
        std::cout << "\033[1;31mRESULT: " << num_errors << " tests failed!"  \
                  << "\033[0m" << std::endl;                                 \
      } else {                                                               \
        std::cout << "\033[1;32mRESULT: all tests PASSED!"                   \
                  << "\033[0m" << std::endl;                                 \
      }                                                                      \
    }                                                                        \
    return num_errors;                                                       \
  }                                                                          \
  int emp_main_function

#endif
