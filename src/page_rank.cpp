#include "page_rank.h"

#include <omp.h>
#include <stdlib.h>

#include <cmath>
#include <iostream>
#include <utility>

#include "CycleTimer.h"
#include "graph.h"
#include "graph_partition.h"
#include "profile/papi.h"
#define AVGITER 100

double PageRank::earlyConvergePageRank(std::vector<double>* score_new) {
  double start = CycleTimer::currentSeconds();
  int numNodes = original_graph->nvtxs;
  double equal_prob = 1.0 / numNodes;
  bool converged = false;
  double broadcastScore = 0.0, broadcastScore_old = 0.0;
  double globalDiff = 0.0;
  int iter = 0;
  std::vector<bool>* Nodeconverged = new std::vector<bool>(numNodes);

  #pragma omp parallel for default(shared) reduction(+ : broadcastScore_old)
  for (int i = 0; i < numNodes; ++i) {
    score_old[i] = equal_prob;
    (*Nodeconverged)[i] = false;
    if (original_graph->outgoing_sizes[i] == 0) {
      broadcastScore_old = equal_prob + broadcastScore_old;
    }
  }

  while (!converged && iter < max_iter_) {
    iter++;
    broadcastScore = 0.0;
    globalDiff = 0.0;

    converged = true;
    #pragma omp parallel for schedule(dynamic, 16) reduction(+ : broadcastScore)
    for (int i = 0; i < numNodes; ++i) {
      if ((*Nodeconverged)[i] == false) {
        double score = 0.0;
        for (index_t v = original_graph->xadj[i]; v < original_graph->xadj[i + 1];
            ++v) {
          score +=
            score_old[original_graph->adjncy[v]] /
            original_graph->outgoing_sizes[original_graph->adjncy[v]];
        }
        score =
            damping_ * score + (1.0 - damping_) * equal_prob;
        score += damping_ * broadcastScore_old * equal_prob;
        (*score_new)[i] = score;
        double diff = score - score_old[i];
        if (original_graph->outgoing_sizes[i] == 0) {
            broadcastScore += diff;
        }
        if (std::abs(diff) < convergence_) {
          (*Nodeconverged)[i] = true;
        }else{
          converged = false;
        }
      } else{
        (*score_new)[i]  = score_old[i];
      }
    }

    broadcastScore_old+=broadcastScore;
    
    std::swap((*score_new), score_old);
  }
  double pagerank_time = CycleTimer::currentSeconds() - start;
  delete Nodeconverged;
  return pagerank_time;
}

double PageRank::dynamicPageRank(std::vector<double>* score_new) {
  double start = CycleTimer::currentSeconds();
  int numNodes = original_graph->nvtxs;
  double equal_prob = 1.0 / numNodes;
  bool converged = false;
  double broadcastScore = 0.0;
  double globalDiff = 0.0;
  int iter = 0;

  #pragma omp parallel for default(shared)
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

#pragma omp parallel for default(shared) reduction(max : globalDiff)
    for (int i = 0; i < numNodes; ++i) {
      (*score_new)[i] += damping_ * broadcastScore * equal_prob;
      globalDiff = std::max((*score_new)[i] - score_old[i], globalDiff);
    }
    converged = (globalDiff < convergence_);
    std::swap((*score_new), score_old);
  }
  double pagerank_time = CycleTimer::currentSeconds() - start;
  return pagerank_time;
}

double PageRank::naivePageRank(std::vector<double>* score_new, int avg_iter,
                               bool early) {
  double pagerank_time = 0.0;
  for (int i = 0; i < avg_iter; i++) {
    pagerank_time +=
        early ? earlyConvergePageRank(score_new) : dynamicPageRank(score_new);
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

  #pragma omp parallel for default(shared)
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

#pragma omp parallel for default(shared) reduction(max : globalDiff)
    for (int i = 0; i < numNodes; ++i) {
      (*score_new)[i] += damping_ * broadcastScore * equal_prob;
      globalDiff = std::max((*score_new)[i] - score_old[i], globalDiff);
    }

    converged = (globalDiff < convergence_);
    std::swap((*score_new), score_old);
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

  int retval;
  retval = PAPI_hl_region_begin("metis");
  assert(retval == PAPI_OK);
  double pagerank_time = 0.0;
  for (int i = 0; i < avg_iter; i++) {
    pagerank_time +=
        staticPageRank(partition->ngraph.get(), score_new, partition->nodeidx);
  }
  retval = PAPI_hl_region_end("metis");
  assert(retval == PAPI_OK);
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