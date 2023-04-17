#ifndef _GRAPH_PARTITION_H_  // [
#define _GRAPH_PARTITION_H_

#include "graph/graph.h"
class GraphPartition {
 public:
  GraphPartition(std::shared_ptr<Graph> g, double ubfactor = 1.3,
                 uint64_t num_parts = 64)
      : g_(g),
        ubfactor_(ubfactor),
        num_parts_(num_parts),
        part_idx_(0),
        edge_num_(0),
        edge_num_target_((uint64_t)((double)g->nedges / num_parts_)),
        edge_num_threshold_((uint64_t)(edge_num_target_ * ubfactor_)),
        visited_(g->nvtxs, 0) {}
  void partition();
  void assign_node(uint64_t node);
  static std::shared_ptr<Graph> FixGraph(std::shared_ptr<Graph>);
  static std::vector<std::shared_ptr<Graph>> DivideGraph(
      std::shared_ptr<Graph> graph, index_t nparts);

 private:
  std::shared_ptr<Graph> g_;
  /* unbalanced factor for number of incoming edges to each parts*/
  double ubfactor_ = 1.3;
  /* unbalanced factor for number of incoming edges to each parts*/
  uint64_t extra_node_edge_threshold_ = 10;
  uint64_t num_parts_ = 64;
  uint64_t part_idx_ = 0;
  uint64_t edge_num_ = 0;
  uint64_t edge_num_target_;
  uint64_t edge_num_threshold_;
  std::vector<bool> visited_;
};
#endif