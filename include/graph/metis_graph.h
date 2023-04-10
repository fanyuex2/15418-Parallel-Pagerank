#include "common.h"
#include "graph/SNAPFile.hpp"

class MetisGraph {
 public:
  uint64_t nvtxs;
  uint64_t nedges;
  std::vector<uint64_t> xadj;
  std::vector<uint64_t> adjncy;
  std::vector<uint64_t> outedges;
};

std::unique_ptr<MetisGraph> createMetisGraph(const std::string file_name) {
  SNAPFile snap_file(file_name);
  int rc = 0;
  uint64_t nvtxs;
  uint64_t nedges;
  std::unique_ptr<MetisGraph> graph = std::make_unique<MetisGraph>();

  snap_file.getInfo(graph->nvtxs, graph->nedges);
  graph->xadj.resize(graph->nvtxs + 1);
  graph->adjncy.resize(graph->nedges);
  graph->outedges.resize(graph->nvtxs);

  snap_file.read(graph->xadj.data(), graph->adjncy.data(),
                 graph->outedges.data());
  return std::move(graph);
}