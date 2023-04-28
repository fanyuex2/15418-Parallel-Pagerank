#ifndef __PAGE_RANK_H__
#define __PAGE_RANK_H__

#include "common.h"
#include "graph.h"

class PageRank {
 public:
  PageRank(std::shared_ptr<Graph> g, double damping, double convergence,
           int max_iter)
      : original_graph(g),
        score_old(original_graph->nvtxs, 0),
        damping_(damping),
        convergence_(convergence),
        max_iter_(max_iter) {}
  double partitionPageRank(std::vector<double>* score_new, int avg_iter,
                           bool graph_partition);
  double naivePageRank(std::vector<double>* score_new, int avg_iter,
                       bool early);
  double dynamicPageRank(std::vector<double>* score_new);
  double serialPageRank(std::vector<double>* score_new);
  double earlyConvergePageRank(std::vector<double>* score_new);

 private:
  double staticPageRank(Graph* graph, std::vector<double>* score_new,
                        std::vector<index_t>& nodeidx);
  std::shared_ptr<Graph> original_graph;
  std::vector<double> score_old;
  double damping_;
  double convergence_;
  int max_iter_;
};
#endif /* __PAGE_RANK_H__ */
