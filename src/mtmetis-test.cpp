#include <stdio.h>

#include <cstddef>
#include <ctime>
#include <iostream>
#include <vector>

#include "graph/graph.h"
#include "graph_partition.h"
#include "metis/wildriver.h"

using namespace std;

int main();
void partition(int nvtxs, const int *xadj, const int *adjncy, int *where);

//****************************************************************************80

int main() {
  // wildriver_graph_handle * handle;
  std::unique_ptr<Graph> graph =
      Graph::createMetisGraph("../test/hollins.snap");
  std::unique_ptr<GraphPartition> par =
      std::make_unique<GraphPartition>(std::move(graph), 128);
  par->partition();
  par->sortNodesByPart();
  // partition(graph->nvtxs, graph->xadj.data(),
  // graph->adjncy.data(),graph->parts.data());
  return 0;
}

/*
void partition(uint64_t nvtxs, const uint64_t *xadj, const uint64_t *adjncy,
               uint64_t *where) {
  mtmetis_pid_type nparts = 2;
  long int r_edgecut;

  cout << "\n";
  cout << "PARTGRAPHRECURSIVE_TEST:\n";
  cout << "  METIS_PartGraphRecursive partitions a graph into K parts\n";
  cout << "  using multilevel recursive bisection.\n";

  double *options = mtmetis_init_options();

options[MTMETIS_OPTION_VERBOSITY] = MTMETIS_VERBOSITY_HIGH;
options[MTMETIS_OPTION_NPARTS] = 64.0;
options[MTMETIS_OPTION_PTYPE] = MTMETIS_PTYPE_KWAY;
options[MTMETIS_OPTION_NTHREADS] = 4.0;

int ret = mtmetis_partition_explicit(nvtxs, xadj, adjncy, NULL, NULL, options,
                                     where, &r_edgecut);
cout << "\n";
cout << "  Return code = " << ret << "\n";
cout << "  Edge cuts for partition = " << r_edgecut << "\n";

cout << "\n";
cout << "  Partition vector:\n";
cout << "\n";
cout << "  Node  Part\n";
cout << "\n";
for (unsigned part_i = 0; part_i < nvtxs; part_i++) {
  cout << "     " << part_i << "     " << where[part_i] << endl;
}

FILE *fout = fopen("test.part", "w");
for (int i = 0; i < nvtxs; ++i) {
  fprintf(fout, "%lu\n", where[i]);
}
fclose(fout);

return;
}
*/