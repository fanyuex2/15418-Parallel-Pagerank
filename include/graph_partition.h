#ifndef _GRAPH_PARTITION_H_  // [
#define _GRAPH_PARTITION_H_

#include "graph/graph.h"
class GraphPartition {
 public:
  GraphPartition(std::unique_ptr<Graph> g, uint64_t num_parts = 64)
      : graph(std::move(g)),
        ngraph(std::make_unique<Graph>()),
        nparts(num_parts) {}
  void partition();
  void printPartition();
  void sortNodesByPart();
  static std::vector<std::shared_ptr<Graph>> DivideGraph(
      std::shared_ptr<Graph> graph, index_t nparts);

 private:
  void FixGraph();
  std::unique_ptr<Graph> graph;
  std::unique_ptr<Graph> ngraph;
  std::vector<index_t> parts;
  std::vector<index_t> N2Oidx; 
  std::vector<index_t> O2Nidx;
  index_t nparts;
};
#endif