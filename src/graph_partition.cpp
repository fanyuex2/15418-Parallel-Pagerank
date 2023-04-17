#include "graph_partition.h"

#include <cassert>
#include <iostream>

#include "gk_defs.h"
#include "gk_macros.h"
#include "gk_mkblas.h"
#include "gk_mkmemory.h"
#include "gk_mkpqueue.h"
#include "gk_mkrandom.h"
#include "gk_mksort.h"
#include "gk_proto.h"
#include "metis.h"

typedef struct {
  index_t u, v, w; /*!< Edge (u,v) with weight w */
} uvw_t;

void uvwsorti(size_t n, uvw_t *base) {
#define uvwkey_lt(a, b) \
  ((a)->u < (b)->u || ((a)->u == (b)->u && (a)->v < (b)->v))
  GK_MKQSORT(uvw_t, base, n, uvwkey_lt);
#undef uvwkey_lt
}

/*************************************************************************/
/*! This function creates a graph whose topology is consistent with
    Metis' requirements that:
    - There are no self-edges.
    - It is undirected; i.e., (u,v) and (v,u) should be present and of the
      same weight.
    - The adjacency list should not contain multiple edges to the same
      other vertex.

    Any of the above errors are fixed by performing the following operations:
    - Self-edges are removed.
    - The undirected graph is formed by the union of edges.
    - One of the duplicate edges is selected.

    The routine does not change the provided vertex weights.
*/
/*************************************************************************/
std::shared_ptr<Graph> GraphPartition::FixGraph(std::shared_ptr<Graph> graph) {
  index_t i, j, k, l, nvtxs, nedges;
  index_t *xadj, *adjncy, *adjwgt;
  index_t *nxadj, *nadjncy;
  std::shared_ptr<Graph> ngraph;
  uvw_t *edges;

  nvtxs = graph->nvtxs;
  xadj = graph->xadj.data();
  adjncy = graph->adjncy.data();

  ngraph = Graph::createMetisGraphEmpty();

  ngraph->nvtxs = nvtxs;

  /* fix graph by sorting the "superset" of edges */
  edges =
      (uvw_t *)gk_malloc(sizeof(uvw_t) * 2 * xadj[nvtxs], "FixGraph: edges");

  for (nedges = 0, i = 0; i < nvtxs; i++) {
    for (j = xadj[i]; j < xadj[i + 1]; j++) {
      /* keep only the upper-trianglular part of the adjacency matrix */
      if (i < adjncy[j]) {
        edges[nedges].u = i;
        edges[nedges].v = adjncy[j];
        nedges++;
      } else if (i > adjncy[j]) {
        edges[nedges].u = adjncy[j];
        edges[nedges].v = i;
        nedges++;
      }
    }
  }

  uvwsorti(nedges, edges);

  /* keep the unique subset */
  for (k = 0, i = 1; i < nedges; i++) {
    if (edges[k].v != edges[i].v || edges[k].u != edges[i].u) {
      edges[++k] = edges[i];
    }
  }
  nedges = k + 1;

  /* allocate memory for the fixed graph */
  ngraph->xadj.resize(nvtxs + 1);
  ngraph->adjncy.resize(2 * nedges);
  nxadj = ngraph->xadj.data();
  nadjncy = ngraph->adjncy.data();

  /* create the adjacency list of the fixed graph from the upper-triangular
     part of the adjacency matrix */
  for (k = 0; k < nedges; k++) {
    nxadj[edges[k].u]++;
    nxadj[edges[k].v]++;
  }
  MAKECSR(i, nvtxs, nxadj);

  for (k = 0; k < nedges; k++) {
    nadjncy[nxadj[edges[k].u]] = edges[k].v;
    nadjncy[nxadj[edges[k].v]] = edges[k].u;
    nxadj[edges[k].u]++;
    nxadj[edges[k].v]++;
  }
  SHIFTCSR(i, nvtxs, nxadj);

  gk_free((void **)&edges, LTERM);
  ngraph->parts.resize(nvtxs);

  // update vertex weight according to the nunmber of incoming edges
  ngraph->vwgt.resize(nvtxs);
  for (k = 0; k < nvtxs; k++) {
    ngraph->vwgt[k] = xadj[k + 1] - xadj[k];
  }
  return ngraph;
}

std::vector<std::shared_ptr<Graph>> GraphPartition::DivideGraph(
    std::shared_ptr<Graph> graph, index_t nparts) {
  std::vector<std::shared_ptr<Graph>> subgraphs(nparts);
  for (int i = 0; i < nparts; i++) {
    subgraphs[i] = Graph::createMetisGraphEmpty();
  }
  for (int i = 0; i < graph->nvtxs; i++) {
  }
}

std::shared_ptr<Graph> Graph::createMetisGraph(const std::string file_name) {
  SNAPFile snap_file(file_name);
  int rc = 0;
  index_t nvtxs;
  index_t nedges;
  std::shared_ptr<Graph> graph = std::make_shared<Graph>();

  snap_file.getInfo(graph->nvtxs, graph->nedges);
  graph->xadj.resize(graph->nvtxs + 1);
  graph->adjncy.resize(graph->nedges);
  graph->parts.resize(graph->nvtxs);
  for (index_t i = 0; i < graph->nvtxs; i++) {
    graph->parts[i] = -1;
  }

  snap_file.read(graph->xadj.data(), graph->adjncy.data());
  return graph;
}

void Graph::printPartition() {
  std::vector<index_t> partition(nvtxs, 0);
  int part = 0;
  for (int i = 0; i < nvtxs; i++) {
    partition[parts[i]] += adjncy[xadj[i + 1] - xadj[i]];
    part = std::max(part, parts[i]);
  }
  for (int i = 0; i < part; i++) {
    std::cout << partition[i] << ' ';
  }
  std::cout << std::endl;
}