#include "page_rank.h"

#include <omp.h>
#include <stdlib.h>

#include <cmath>
#include <iostream>
#include <utility>

#include "CycleTimer.h"
#include "graph.h"
#include "graph_partition.h"
#define AVGITER 100

double PageRank::dynamicPageRank(std::vector<double>* score_new) {
  double start = CycleTimer::currentSeconds();
  int numNodes = original_graph->nvtxs;
  double equal_prob = 1.0 / numNodes;
  bool converged = false;
  double broadcastScore = 0.0;
  double globalDiff = 0.0;
  int iter = 0;

  for (int i = 0; i < numNodes; ++i) {
    score_old[i] = equal_prob;
  }

  while (!converged && iter < max_iter_) {
    iter++;
    broadcastScore = 0.0;
    globalDiff = 0.0;
#pragma omp parallel for reduction(+ : broadcastScore) schedule(dynamic, 16)
    for (int i = 0; i < numNodes; ++i) {
      (*score_new)[i] = 0.0;

      if (original_graph->outgoing_sizes[i] == 0) {
        broadcastScore += score_old[i];
      }
      for (index_t v = original_graph->xadj[i]; v < original_graph->xadj[i + 1];
           ++v) {
        (*score_new)[i] +=
            score_old[original_graph->adjncy[v]] /
            original_graph->outgoing_sizes[original_graph->adjncy[v]];
      }
      (*score_new)[i] =
          damping_ * (*score_new)[i] + (1.0 - damping_) * equal_prob;
    }

#pragma omp parallel for default(shared) reduction(+ : globalDiff)
    for (int i = 0; i < numNodes; ++i) {
      (*score_new)[i] += damping_ * broadcastScore * equal_prob;
      globalDiff += std::abs((*score_new)[i] - score_old[i]);
    }
    converged = (globalDiff < convergence_);
    std::swap((*score_new), score_old);
  }
  double pagerank_time = CycleTimer::currentSeconds() - start;
  return pagerank_time;
}

double PageRank::naivePageRank(std::vector<double>* score_new, int avg_iter) {
  double pagerank_time = 0.0;
  for (int i = 0; i < avg_iter; i++) {
    pagerank_time += dynamicPageRank(score_new);
  }
  return pagerank_time / avg_iter;
}

double PageRank::staticPageRank(Graph* graph, std::vector<double>* score_new,
                                std::vector<index_t>& nodeidx) {
  int nparts = nodeidx.size() - 1;

  double start = CycleTimer::currentSeconds();
  int numNodes = original_graph->nvtxs;

  double equal_prob = 1.0 / numNodes;
  bool converged = false;
  double broadcastScore = 0.0;
  double globalDiff = 0.0;
  int iter = 0;

  for (int i = 0; i < numNodes; ++i) {
    score_old[i] = equal_prob;
  }

  while (!converged && iter < max_iter_) {
    iter++;
    broadcastScore = 0.0;
    globalDiff = 0.0;
#pragma omp parallel default(shared) reduction(+ : broadcastScore)
    {
      index_t start_vex = nodeidx[omp_get_thread_num()],
              end_vex = nodeidx[omp_get_thread_num() + 1];
      double localScore = 0.0;
      for (int i = start_vex; i < end_vex; ++i) {
        (*score_new)[i] = 0.0;

        if (graph->outgoing_sizes[i] == 0) {
          localScore += score_old[i];
        }
        for (index_t v = graph->xadj[i]; v < graph->xadj[i + 1]; ++v) {
          (*score_new)[i] += score_old[graph->adjncy[v]] /
                             graph->outgoing_sizes[graph->adjncy[v]];
        }
        (*score_new)[i] =
            damping_ * (*score_new)[i] + (1.0 - damping_) * equal_prob;
      }
      broadcastScore += localScore;
    }

#pragma omp parallel for default(shared) reduction(+ : globalDiff)
    for (int i = 0; i < numNodes; ++i) {
      (*score_new)[i] += damping_ * broadcastScore * equal_prob;
      globalDiff += std::abs((*score_new)[i] - score_old[i]);
      std::swap((*score_new)[i], score_old[i]);
    }

    converged = (globalDiff < convergence_);
  }

  double pagerank_time = CycleTimer::currentSeconds() - start;
  return pagerank_time;
}

double PageRank::partitionPageRank(std::vector<double>* score_new, int avg_iter,
                                   bool graph_partition) {
  std::unique_ptr<GraphPartition> partition =
      std::make_unique<GraphPartition>(original_graph, omp_get_max_threads());

  if (graph_partition)
    partition->newFromPartition();
  else
    partition->newFromStatic();

  partition->ngraph->outgoingSize();

  double pagerank_time = 0.0;
  for (int i = 0; i < avg_iter; i++) {
    pagerank_time +=
        staticPageRank(partition->ngraph.get(), score_new, partition->nodeidx);
  }

  /* for (int i = 0; i < original_graph->nvtxs; i++) {
     std::cout << ' ' << (*score_new)[i] << ' ' << std::endl;
   }*/

  for (int i = 0; i < original_graph->nvtxs; i++) {
    // std::cout << ' ' << partition->O2Nidx[i] << ' ' << std::endl;
    score_old[partition->N2Oidx[i]] = (*score_new)[i];
  }
  std::swap((*score_new), score_old);
  return pagerank_time / avg_iter;
}