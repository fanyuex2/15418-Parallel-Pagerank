#include "metis/mtmetis.h"

#include <stdio.h>
#include <stdint.h>

#include <cstddef>
#include <ctime>
#include <iostream>
#include <vector>

#include "graph/SNAPFile.hpp"
#include "common/graph.h"

using namespace std;

// graph* snap_load_graph(std::string s);

//****************************************************************************80

graph* snap_load_graph(std::string s) {
  // wildriver_graph_handle * handle;
  // if (argc < 2) {
  //       std::cerr << "Usage: ./metmetis-test.cpp <path/to/graph/file>\n";
  //       exit(1);
  //   }
  // string s = argv[1];
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

  graph* graph = (struct graph*)(malloc(sizeof(struct graph)));
  int* incoming_starts = (int*)malloc(sizeof(int) * nvtxs);
  int* incoming_edegs = (int*)malloc(sizeof(int)*nedges);
  int* outgoing_count = (int*)malloc(sizeof(int)*nvtxs);
  
  for (int i = 0; i < nvtxs; i++) {
    incoming_starts[i] = xadj[i];
  }
  for (int i = 0; i < nedges; i++) {
    incoming_edegs[i] = adjncy[i];
  }
  for (int i = 0; i < nvtxs; i++) {
    outgoing_count[i] = outedges[i];
  }

  // cout << "----------incoming start idx--------" << endl;
  // for (int i = 0; i < nvtxs; i++) {
  //   cout << xadj[i] << ' ';
  // }
  // cout << endl;
  // cout << "----------incoming edges' vertex--------" << endl;
  // for (int i = 0; i < nedges; i++) {
  //   cout << adjncy[i] << ' ';
  // }
  // cout << endl;
  // cout << "----------num of outgoing edges per vertex--------" << endl;
  // for (int i = 0; i < nvtxs; i++) {
  //   cout << outedges[i] << ' ';
  // }
  // cout << endl;

  graph->num_nodes = nvtxs;
  graph->num_edges = nedges;
  graph->incoming_starts = incoming_starts;
  graph->incoming_edges = incoming_edegs;
  graph->outgoing_count = outgoing_count;

  return graph;
}
