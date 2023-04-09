#ifndef __GRAPH_H__
#define __GRAPH_H__

using Vertex = int;

class graph {
  // Number of edges in the graph
  uint64_t num_edges;
  // Number of vertices in the graph
  uint64_t num_nodes;
  uint64_t* incoming_starts;
  uint64_t* incoming_edges;

 public:
};
