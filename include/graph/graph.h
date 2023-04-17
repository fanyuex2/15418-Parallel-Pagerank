#ifndef _GRAPH_H_  // [
#define _GRAPH_H_

#define index_t int

#include "common.h"
#include "graph/SNAPFile.hpp"

class Graph {
 public:
  index_t nvtxs;
  index_t nedges;

  std::vector<index_t> vidx;
  std::vector<index_t> xadj;
  std::vector<index_t> adjncy;
  std::vector<index_t> vwgt;
  static std::unique_ptr<Graph> createMetisGraph(const std::string file_name);
  void printGraph();
};

#endif