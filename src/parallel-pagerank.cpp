
#include <stdint.h>

class graph {
  // Number of edges in the graph
  uint64_t num_edges;
  // Number of vertices in the graph
  uint64_t num_nodes;

  // The node reached by vertex i's first outgoing edge is given by
  // outgoing_edges[outgoing_starts[i]].  To iterate over all
  // outgoing edges, please see the top-down bfs implementation.

  uint64_t* incoming_starts;
  uint64_t* incoming_edges;
};

class ParallelPagerank {
  uint64_t num_part;
};