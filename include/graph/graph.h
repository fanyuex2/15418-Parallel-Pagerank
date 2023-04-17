#ifndef _GRAPH_H_  // [
#define _GRAPH_H_

#define index_t int

#include "common.h"
#include "graph/SNAPFile.hpp"

class Graph {
 public:
  index_t nvtxs;
  index_t nedges;
  std::vector<index_t> xadj;
  std::vector<index_t> adjncy;
  std::vector<index_t> vwgt;
  std::vector<index_t> parts;
  static std::shared_ptr<Graph> createMetisGraph(const std::string file_name);
  static std::shared_ptr<Graph> createMetisGraphEmpty() {
    return std::make_shared<Graph>();
  }
  void printPartition();
};

#endif