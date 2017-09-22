//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Build graphs of various types in the standard format.
//  NOTE: All questions can be answered by providing command-line arguements.

#include <iostream>

#include "../../../source/base/assert.h"
#include "../../../source/base/vector.h"
#include "../../../source/config/command_line.h"
#include "../../../source/tools/Graph.h"
#include "../../../source/tools/graph_utils.h"
#include "../../../source/tools/string_utils.h"

int GetValue(const std::string & query,
	     emp::vector<std::string> & args,
	     uint32_t & cur_arg,
	     int max_val=-1)
{
  if (cur_arg < args.size()) {  // We already have value from the command line!
    return std::stoi( args[cur_arg++] );
  }

  std::cout << query;
  if (max_val >= 0) std::cout << " [max=" << max_val << "]";
  std::cout << ":" << std::endl;

  int out_val;
  std::cin >> out_val;
  return out_val;
}

int main(int argc, char* argv[])
{
  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);

  uint32_t cur_arg = 1;
  bool print_file = true;

  // First, determine what type of graph we need to make.
  int graph_type = 0;
  if (cur_arg < args.size()) {  // Specified on command line.
    graph_type = std::stoi( args[cur_arg++] );
  }
  else { // No command-line arg; request input from user.
    std::cout << "What type of graph?" << std::endl
              << " 0 - Random" << std::endl
              << " 1 - Chain" << std::endl
              << " 2 - Ring" << std::endl
              << " 3 - Tree" << std::endl
              << " 4 - Grid" << std::endl
              << " 5 - Lossy Grid" << std::endl
              << " 6 - Linked Cliques" << std::endl
	      << " 7 - Hamiltonion Cycle (with solution)" << std::endl;
    std::cin >> graph_type;
  }

  emp::Graph graph;
  emp::Random random;
  std::string filename;

  if (graph_type == 0) {
    std::cout << "Generating a Random Graph." << std::endl;
    int nodes = GetValue("How many vertices?", args, cur_arg, 1000);
    int edges = GetValue("How many edges?", args, cur_arg, nodes*(nodes-1)/2);
    graph = build_graph_random(nodes, edges, random);
    filename = emp::to_string("rand-", nodes, '-', edges);
  }
  else if (graph_type == 1) {
    std::cout << "Generating a Chain Graph." << std::endl;
    int nodes = GetValue("How many vertices?", args, cur_arg, 1000);
    graph = build_graph_grid(nodes, 1, random);
    filename = emp::to_string("chain-", nodes, '-', nodes-1);
  }
  else if (graph_type == 2) {
    std::cout << "Generating a Ring Graph." << std::endl;
    int nodes = GetValue("How many vertices?", args, cur_arg, 1000);
    graph = build_graph_ring(nodes, random);
    filename = emp::to_string("ring-", nodes, '-', nodes);
  }
  else if (graph_type == 3) {
    std::cout << "Generating a Tree Graph." << std::endl;
    int nodes = GetValue("How many vertices?", args, cur_arg, 1000);
    graph = build_graph_tree(nodes, random);
    filename = emp::to_string("tree-", nodes, '-', nodes-1);
  }
  else if (graph_type == 4) {
    std::cout << "Generating a Grid Graph." << std::endl;
    int rows = GetValue("How many rows?", args, cur_arg, 100);
    int cols = GetValue("How many columns?", args, cur_arg, 100);
    graph = build_graph_grid(rows, cols, random);
    filename = emp::to_string("grid-", rows*cols, '-', rows*(cols-1)+cols*(rows-1));
  }
  else if (graph_type == 5) {
    std::cout << "Generating a Lossy Grid Graph." << std::endl;
    int rows = GetValue("How many rows?", args, cur_arg, 100);
    int cols = GetValue("How many columns?", args, cur_arg, 100);
    int max_edges = rows*(cols-1) + cols*(rows-1);
    int edges = GetValue("How many active edges?", args, cur_arg, max_edges);
    double edge_frac = ((double) edges) / ((double) max_edges);
    graph = build_graph_grid(rows, cols, random, edge_frac);
    filename = emp::to_string("lgrid-", rows*cols, '-', graph.GetEdgeCount()/2);
  }
  else if (graph_type == 6) {
    std::cout << "Generating a Linked Cliques Graph." << std::endl;
    int clique_count = GetValue("How many cliques?", args, cur_arg, 100);
    int clique_size = GetValue("How big is each clique?", args, cur_arg, 100);
    int v_count = clique_count * clique_size;
    int max_edges = v_count * (v_count-1) / 2;
    int edges = GetValue("How many extra edges?", args, cur_arg, max_edges);
    double edge_frac = ((double) edges) / ((double) max_edges);
    graph = build_graph_clique_set(clique_size, clique_count, random, edge_frac);
    filename = emp::to_string("cliqueset-", v_count, '-', graph.GetEdgeCount()/2);
  }
  else if (graph_type == 7) {
    std::cout << "Generating a Random Graph (with hamiltonian cycle and solution)." << std::endl;
    int nodes = GetValue("How many vertices?", args, cur_arg, 1000);
    int edges = GetValue("How many edges?", args, cur_arg, nodes*(nodes-1)/2);

    // Generate the Hamiltonian Cycle
    emp::vector<size_t> v_map = emp::BuildRange<size_t>(0, nodes);
    emp::Shuffle(random, v_map);
    graph.Resize(nodes);
    for (size_t i = 1; i < nodes; i++) {
      const size_t from = v_map[i];
      const size_t to = v_map[i-1];
      graph.AddEdgePair(from, to);
    }
    graph.AddEdgePair(v_map[0], v_map[nodes-1]);

    // Add in extra edges.
    size_t e_cur = nodes;
    while (e_cur < edges) {
      const size_t from = random.GetUInt(nodes);
      const size_t to = random.GetUInt(nodes);
      if (from == to || graph.HasEdge(from,to)) continue;
      graph.AddEdgePair(from, to);
      ++e_cur;
    }

    // Print the file.
    filename = emp::to_string("hcycle-", nodes, '-', edges);
    std::ofstream of(filename);
    graph.PrintSym(of);
    for (size_t i = 0; i < nodes; i++) {
      if (i > 0) of << " ";
      of << v_map[i];
    }
    print_file = false;
  }
  else {
    std::cout << "Unknown Graph type '" << graph_type << "'. Aborting." << std::endl;
    return 0;
  }


  if (print_file) {
    std::ofstream of(filename);
    graph.PrintSym(of);
  }

  std::cout << "Printed to file '" << filename << "'." << std::endl;
}
