#include <omp.h>
#include <stdio.h>

#include <cstddef>
#include <ctime>
#include <iostream>
#include <vector>

#include "graph.h"
#include "graph_partition.h"
#include "page_rank.h"

using namespace std;
#define PageRankDampening 0.3d
#define PageRankConvergence 1e-9d
#define EPSILON 0.00000000001
#define THREADNUM 8

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

//****************************************************************************80

int main(int argc, char **argv) {
  if (argc < 5) {
    std::cerr << "Usage: ./pagerank <path_to_graph> <num_workers> <method> <runs>\n"
                 "<method>: "
                 "naive, metis, static\n"
              << "<runs>: to compute the average time across all runs\n";
    exit(1);
  }

  //graph->saveGraph();

  int thread_count = -1;
  thread_count = atoi(argv[2]);
  if (thread_count <= 0) {
    std::cerr << "invalid number of workers\n";
    exit(1);
  }

  int avg_iter = -1;
  avg_iter = atoi(argv[4]);
  if (avg_iter <= 0) {
    std::cerr << "<invalid number of runs\n";
    exit(1);
  }

  std::string method = argv[3];
  omp_set_num_threads(thread_count);

  std::shared_ptr<Graph> graph;
  std::unique_ptr<PageRank> par;
  
  if (method.compare("naive") == 0) {
    graph = Graph::createMetisGraph(argv[1]);
    par = std::make_unique<PageRank>(graph, PageRankDampening, PageRankConvergence, 500);
  } else {
    graph = Graph::createSavedGraph(argv[1]);
    par = std::make_unique<PageRank>(graph, PageRankDampening, PageRankConvergence, 500);
  }

  std::vector<double> *vec = new std::vector<double>(graph->nvtxs, 0.0);
  double time;
  if (method.compare("naive") == 0)
    time = par->naivePageRank(vec, avg_iter);
  else if (method.compare("metis") == 0)
    time = par->partitionPageRank(vec, avg_iter, true);
  else if (method.compare("static") == 0)
    time = par->partitionPageRank(vec, avg_iter, false);
  else {
    std::cerr << "invalid method\n";
    exit(1);
  }
  std::cout << time << std::endl;

  std::vector<double> *vec2 = new std::vector<double>(graph->nvtxs, 0.0);
  par->dynamicPageRank(vec2);
  // assert(compareApprox((*vec).data(), (*vec2).data(), graph->nvtxs));
  return 0;
}
