#include "metis/mtmetis.h"

#include <stdio.h>

#include <cstddef>
#include <ctime>
#include <iostream>
#include <vector>

#include "graph/metis_graph.h"
#include "metis/wildriver.h"

using namespace std;

int main();
void partition(uint64_t nvtxs, const uint64_t *xadj, const uint64_t *adjncy);

//****************************************************************************80

int main() {
  // wildriver_graph_handle * handle;
  std::unique_ptr<MetisGraph> graph = createMetisGraph("../test/hollins.snap");

  for (int i = 0; i < graph->nvtxs + 1; i++) {
    cout << graph->xadj[i] << ' ';
  }
  cout << endl;
  for (int i = 0; i < graph->nedges; i++) {
    cout << graph->adjncy[i] << ' ';
  }
  cout << endl;
  for (int i = 0; i < graph->nvtxs; i++) {
    cout << graph->outedges[i] << ' ';
  }
  cout << endl;
  // partition(nvtxs, xadj, adjncy);
  return 0;
}

void partition(uint64_t nvtxs, const uint64_t *xadj, const uint64_t *adjncy) {
  mtmetis_pid_type nparts = 2;
  long int r_edgecut;
  //
  //  On return, the partition vector for the graph.
  //
  mtmetis_pid_type where[nvtxs];

  cout << "\n";
  cout << "PARTGRAPHRECURSIVE_TEST:\n";
  cout << "  METIS_PartGraphRecursive partitions a graph into K parts\n";
  cout << "  using multilevel recursive bisection.\n";

  double *options = mtmetis_init_options();

  /* set default verbosity to low */
  options[MTMETIS_OPTION_VERBOSITY] = MTMETIS_VERBOSITY_LOW;
  options[MTMETIS_OPTION_NPARTS] = 2.0;
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