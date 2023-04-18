#ifndef _GRAPH_PARTITION_H_  // [
#define _GRAPH_PARTITION_H_

#include "graph.h"
class GraphPartition {
 public:
  GraphPartition(std::shared_ptr<Graph> g, uint64_t num_parts = 64)
      : graph(g), ngraph(std::make_unique<Graph>()), nparts(num_parts) {}
  void newGraphByPart();
  void printPartition();

  std::unique_ptr<Graph> ngraph;
  std::vector<index_t> nodeidx;
  std::vector<index_t> N2Oidx;
  std::vector<index_t> O2Nidx;

 private:
  void FixGraph();
  void partition();
  void sortNodesByPart();
  std::shared_ptr<Graph> graph;
  std::vector<index_t> parts;
  index_t nparts;
};
#endif