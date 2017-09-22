//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Load input from standard in that begins with a value "N" and then contains N pairs of strings.
//  output will be the edit distances between each string pair.

#include <iostream>
#include <string>

#include "../../../source/base/assert.h"
#include "../../../source/base/vector.h"
#include "../../../source/config/command_line.h"
#include "../../../source/tools/functions.h"
#include "../../../source/tools/sequence_utils.h"

int main(int argc, char* argv[])
{
  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_flag(args, "-v");

  int N;
  std::cin >> N;
  std::string in1, in2;
  for (int i = 0; i < N; i++) {
    std::cin >> in1 >> in2;
    std::cout << emp::calc_edit_distance(in1, in2) << std::endl;
  }

  return 0;
}
