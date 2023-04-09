
struct graph {
  // Number of edges in the graph
  int num_edges;
  // Number of vertices in the graph
  int num_nodes;

  // The node reached by vertex i's first outgoing edge is given by
  // outgoing_edges[outgoing_starts[i]].  To iterate over all
  // outgoing edges, please see the top-down bfs implementation.
  int* outgoing_starts;
  Vertex* outgoing_edges;

  int* incoming_starts;
  Vertex* incoming_edges;
};

class ParallelPagerank {
  uint64_t num_part;
}