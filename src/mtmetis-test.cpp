#include "metis/mtmetis.h"

#include <stdio.h>
#include <stdint.t>

#include <cstddef>
#include <ctime>
#include <iostream>
#include <vector>

#include "graph/SNAPFile.hpp"

using namespace std;

int main();

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
  
  return rc;
}
