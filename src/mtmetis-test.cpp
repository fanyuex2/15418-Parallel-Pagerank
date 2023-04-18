#include <omp.h>
#include <stdio.h>

#include <cstddef>
#include <ctime>
#include <iostream>
#include <vector>

#include "graph.h"
#include "graph_partition.h"
#include "metis/wildriver.h"
#include "page_rank.h"

using namespace std;
#define PageRankDampening 0.3d
#define PageRankConvergence 1e-9d
#define EPSILON 0.00000000001
#define THREADNUM 2

bool compareApprox(double *ref, double *stu, int nvtxs) {
  for (int i = 0; i < nvtxs; i++) {
    if (fabs(ref[i] - stu[i]) > EPSILON) {
      std::cerr << "*** Results disagree at " << i << " expected " << ref[i]
                << " found " << stu[i] << std::endl;
      return false;
    }
  }
  return true;
}

int main();
void partition(int nvtxs, const int *xadj, const int *adjncy, int *where);

//****************************************************************************80

int main() {
  // wildriver_graph_handle * handle;

  omp_set_num_threads(THREADNUM);
  std::shared_ptr<Graph> graph =
      Graph::createMetisGraph("../test/hollins.snap");

  std::unique_ptr<GraphPartition> partition =
      std::make_unique<GraphPartition>(graph, 2);
  partition->newGraphByPart();


  std::unique_ptr<PageRank> par = std::make_unique<PageRank>(
      graph, PageRankDampening, PageRankConvergence, 500);
  std::vector<double> *vec_1 = new std::vector<double>(graph->nvtxs, 0.0);
  std::cout << par->dynamicPageRank(vec_1) << std::endl;

  std::vector<double> *vec_2 = new std::vector<double>(graph->nvtxs, 0.0);
  std::cout << par->partitionPageRank(vec_2,THREADNUM) << std::endl;

  assert(compareApprox((*vec_1).data(), (*vec_2).data(), graph->nvtxs));
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