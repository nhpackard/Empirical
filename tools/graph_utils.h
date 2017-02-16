//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file provides a number of tools for manipulating graphs.

#ifndef EMP_GRAPH_UTILS_H
#define EMP_GRAPH_UTILS_H

#include <fstream>
#include <iostream>
#include <string>
#include <tuple>

#include "../base/assert.h"
#include "../base/vector.h"

#include "Graph.h"
#include "Random.h"
#include "random_utils.h"

namespace emp {

  Graph build_graph_ring(size_t v_count, Random & random) {
    Graph graph(v_count);

    emp::vector<size_t> v_map = BuildRange<size_t>(0, v_count);
    Shuffle(random, v_map);

    for (size_t i = 1; i < v_count; i++) {
      const size_t from = v_map[i];
      const size_t to = v_map[i-1];
      graph.AddEdgePair(from, to);
    }

    graph.AddEdgePair(v_map[0], v_map[v_count-1]);

    return graph;
  }

  Graph build_graph_tree(size_t v_count, Random & random) {
    Graph graph(v_count);

    emp::vector<size_t> v_map = BuildRange<size_t>(0, v_count);
    Shuffle(random, v_map);

    for (size_t i = 1; i < v_count; i++) {
      const size_t from = v_map[i];
      const size_t to = v_map[random.GetUInt(i)];
      graph.AddEdgePair(from, to);
    }

    return graph;
  }

  Graph build_graph_random(size_t v_count, size_t e_count, Random & random, bool connected=true)
  {
    const size_t max_edges = v_count * (v_count-1) / 2;
    (void) max_edges;

    emp_assert(v_count >= 2 && e_count > 0); // We need at least two vertices to support an edge.
    emp_assert(e_count <= max_edges); // Shouldn't have more edges than can fit!

    Graph graph(v_count);
    size_t e_cur = 0;           // How many edges have we added?

    // If the graph should be connected, start by building a tree.
    if (connected) {
      emp_assert(e_count >= v_count - 1);  // We need enough edges to build a connected graph.
      graph = build_graph_tree(v_count, random);
      e_cur = v_count - 1;
    }

    // @CAO -- we should do something better if we are filling in most of the edges.

    while (e_cur < e_count) {
      const size_t from = random.GetUInt(v_count);
      const size_t to = random.GetUInt(v_count);

      if (from == to || graph.HasEdge(from,to)) continue;

      graph.AddEdgePair(from, to);
      ++e_cur;
    }

    return graph;
  }

  Graph build_graph_grid(size_t width, size_t height, Random & random, double prob_use=1.0) {
    emp_assert(width > 0 && height > 0);

    const size_t v_count = width * height;
    // const size_t e_count = (width-1)*height + width*(height-1);

    Graph graph(v_count);

    emp::vector<size_t> v_map = BuildRange<size_t>(0, v_count);
    Shuffle(random, v_map);

    for (size_t x=0; x < width; ++x) {
      for (size_t y=0; y < height; ++y) {
        const size_t from = y*width + x;
        if (x != (width-1) && random.P(prob_use)) {
          graph.AddEdgePair(v_map[from], v_map[from+1]);      // Horizontal
        }
        if (y != (height-1) && random.P(prob_use)) {
          graph.AddEdgePair(v_map[from], v_map[from+width]);  // Vertical
        }
      }
    }

    return graph;
  }

  // The following method builds a set of cliques (such that one member of each can be part
  // of an independent set) and then links them together
  Graph build_graph_clique_set(size_t clique_size, size_t clique_count, Random & random,
                               double extra_prob=0.5) {
    emp_assert(clique_size > 0 && clique_count > 0);

    const size_t v_count = clique_size * clique_count;
    Graph graph(v_count);

    emp::vector<size_t> v_map = BuildRange<size_t>(0, v_count);
    Shuffle(random, v_map);

    // Fill out all of the edges within a clique
    for (size_t start_id = 0; start_id < v_count; start_id += clique_size) {
      const size_t end_id = start_id + clique_size;
      for (size_t node1 = start_id; node1 < end_id; node1++) {
        for (size_t node2 = node1+1; node2 < end_id; node2++) {
          graph.AddEdgePair(v_map[node1], v_map[node2]);
        }
      }
    }

    // Add on extra edges.
    for (size_t start1 = 0; start1 < v_count; start1 += clique_size) {
      const size_t end1 = start1 + clique_size;
      for (size_t start2 = start1+clique_size; start2 < v_count; start2 += clique_size) {
        const size_t end2 = start2 + clique_size;
        for (size_t node1 = start1; node1 < end1; node1++) {
          for (size_t node2 = start2; node2< end2; node2++) {
            if (node1 == start1 && node2 == start2) continue;  // Both part of IS.
            if (random.P(extra_prob)) graph.AddEdgePair(v_map[node1], v_map[node2]);
          }
        }
      }
    }

    return graph;
  }


  // Helper function for loading symetric graphs from an input stream.
  // sub1 indicates that verticies are numbered 1 to N instead of 0 to N-1.
  // @CAO Need some error checking here...
  Graph load_graph_sym(std::istream & is, bool sub1=false) {
    size_t n_vert, n_edge;
    is >> n_vert >> n_edge;

    Graph out_graph(n_vert);
    size_t from, to;
    for (size_t i = 0; i < n_edge; i++) {
      is >> from >> to;
      if (sub1) { from--; to--; }
      out_graph.AddEdgePair(from, to);
    }

    return out_graph;
  }

  Graph load_graph_sym(std::string filename, bool sub1=false) {
    std::ifstream ifile(filename);
    return load_graph_sym(ifile, sub1);
  }

  // Format: #vertices followed by v^2 0's or 1's
  Graph load_graph_table(std::istream & is) {
    size_t n_vert;
    is >> n_vert;

    Graph out_graph(n_vert);
    size_t val;
    for (size_t i = 0; i < n_vert; i++) {
      for (size_t j = 0; j < n_vert; j++) {
        is >> val;
        if (val) out_graph.AddEdge(i, j);
      }
    }

    return out_graph;
  }

  Graph load_graph_table(std::string filename) {
    std::ifstream ifile(filename);
    return load_graph_table(ifile);
  }
}

#endif
