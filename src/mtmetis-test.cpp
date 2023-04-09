#include "metis/mtmetis.h"

#include <stdio.h>

#include <cstddef>
#include <ctime>
#include <iostream>
#include <vector>

#include "graph/SNAPFile.hpp"
#include "metis/wildriver.h"

using namespace std;

int main();
void partition(uint64_t nvtxs, const uint64_t *xadj, const uint64_t *adjncy);

//****************************************************************************80

int main() {
  // wildriver_graph_handle * handle;
  string s = "../test/test.snap";
  const char *filename = s.c_str();
  SNAPFile snap_file(s);
  int rc = 0;
  uint64_t nvtxs;
  uint64_t nedges;
  snap_file.getInfo(nvtxs, nedges);
  uint64_t adjncy[nedges];
  uint64_t xadj[nvtxs + 1];
  uint64_t outedges[nvtxs] = {0};
  snap_file.read(xadj, adjncy, outedges);

  for (int i = 0; i < nvtxs + 1; i++) {
    cout << xadj[i] << ' ';
  }
  cout << endl;
  for (int i = 0; i < nedges; i++) {
    cout << adjncy[i] << ' ';
  }
  cout << endl;
  for (int i = 0; i < nvtxs; i++) {
    cout << outedges[i] << ' ';
  }
  cout << endl;
  partition(nvtxs, xadj, adjncy);
  return rc;
}

void partition(uint64_t nvtxs, const uint64_t *xadj, const uint64_t *adjncy) {
  mtmetis_pid_type nparts = 2;
  uint64_t r_edgecut;
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