#ifndef _GRAPH_H_  // [
#define _GRAPH_H_

#define index_t int

#include "common.h"
#include "graph/SNAPFile.hpp"

class Graph {
 public:
  Graph() : xadj(0), adjncy(0), vwgt(0), adjwgt(0), outgoing_sizes(0) {}
  index_t nvtxs;
  index_t nedges;

  std::vector<index_t> xadj;
  std::vector<index_t> adjncy;
  std::vector<index_t> vwgt;
  std::vector<index_t> adjwgt;
  std::vector<index_t> outgoing_sizes;
  static std::shared_ptr<Graph> createMetisGraph(const std::string file_name);
  void outgoingSize();
  void printGraph();
};

#endif