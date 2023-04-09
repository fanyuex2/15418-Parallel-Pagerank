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
void partition(mtmetis_vtx_type nvtxs, const mtmetis_adj_type *xadj,
               const mtmetis_vtx_type *adjncy);

//****************************************************************************80

int main() {
  // wildriver_graph_handle * handle;
  string s = "../test/test.snap";
  const char *filename = s.c_str();
  SNAPFile snap_file(s);
  int rc = 0;
  mtmetis_vtx_type nvtxs;
  mtmetis_adj_type nedges;
  snap_file.getInfo(nvtxs, nedges);
  mtmetis_vtx_type adjncy[nedges];
  mtmetis_adj_type xadj[nvtxs + 1];
  mtmetis_adj_type outedges[nvtxs] = {0};
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

void partition(mtmetis_vtx_type nvtxs, const mtmetis_adj_type *xadj,
               const mtmetis_vtx_type *adjncy) {
  mtmetis_pid_type nparts = 2;
  mtmetis_wgt_type r_edgecut;
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