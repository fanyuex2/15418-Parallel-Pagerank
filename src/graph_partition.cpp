#include "graph_partition.h"

#include <algorithm>  // std::sort, std::stable_sort
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#include "common.h"
#include "gk_defs.h"
#include "gk_macros.h"
#include "gk_mkblas.h"
#include "gk_mkmemory.h"
#include "gk_mkpqueue.h"
#include "gk_mkrandom.h"
#include "gk_mksort.h"
#include "gk_proto.h"
#include "mtmetis.h"

std::shared_ptr<Graph> Graph::createMetisGraph(const std::string file_name) {
  SNAPFile snap_file(file_name);
  int rc = 0;
  index_t nvtxs;
  index_t nedges;
  std::shared_ptr<Graph> graph = std::make_shared<Graph>();

  snap_file.getInfo(graph->nvtxs, graph->nedges);
  graph->xadj.resize(graph->nvtxs + 1);
  graph->adjncy.resize(graph->nedges);

  snap_file.read(graph->xadj.data(), graph->adjncy.data());
  graph->outgoingSize();
  return graph;
}

std::shared_ptr<Graph> Graph::createSavedGraph(const std::string file_name) {
  std::cout << "Entering create saved graph with graph: " << file_name << std::endl;
  
  ifstream myfile;
  std::string buffer;
  myfile.open(file_name);
  
  std::shared_ptr<Graph> ngraph = std::make_shared<Graph>();
  // read nvtxs
  do {
     std::getline(myfile, buffer);
  } while (buffer.size() == 0 || buffer[0] == '#');
  ngraph->nvtxs = atoi(buffer.c_str());
  buffer.clear();
  std::cout << "nvtxs: " << ngraph->nvtxs << std::endl;

  // read nedges
  do {
     std::getline(myfile, buffer);
  } while (buffer.size() == 0 || buffer[0] == '#');
  ngraph->nedges = atoi(buffer.c_str());
  buffer.clear();
  std::cout << "nedges: " << ngraph->nedges << std::endl;

  ngraph->xadj.resize(ngraph->nvtxs + 1);
  ngraph->nedges = 2 * ngraph->nedges;
  ngraph->adjncy.resize(2 * ngraph->nedges);
  ngraph->nodeidx.resize(64 + 1, 0);

  // read xadj
  int tmp = 0;
  std::getline(myfile, buffer);
  std::getline(myfile, buffer);
  ngraph->xadj[tmp] = atoi(buffer.c_str());
  tmp += 1;
  while (buffer[0] != '#') {
   // std::cout << buffer << std::endl;
    std::getline(myfile, buffer);
    ngraph->xadj[tmp] = atoi(buffer.c_str());
    tmp += 1;
  }

  // read adjncy
  int tmp1 = 0;
  std::getline(myfile, buffer);
  ngraph->adjncy[tmp1] = atoi(buffer.c_str());
  tmp1 += 1;
  while (buffer[0] != '#') {
    std::getline(myfile, buffer);
    ngraph->adjncy[tmp1] = atoi(buffer.c_str());
    tmp1 += 1;
  }
  
  // read nodeidx
  int tmp2 = 0;
  std::getline(myfile, buffer);
  ngraph->nodeidx[tmp2] = atoi(buffer.c_str());
  tmp2 += 1;
  while (tmp2 <= 64 + 1) {
    std::getline(myfile, buffer);
    ngraph->nodeidx[tmp2] = atoi(buffer.c_str());
    tmp2 += 1;
  }

  ngraph->outgoingSize();

  myfile.close();

  std::cout << "Finish create saved graph" << std::endl;
  
  return ngraph;
}

void Graph::outgoingSize() {
  outgoing_sizes.resize(nvtxs, 0);
  for (int i = 0; i < nvtxs; i++) {
    for (int j = xadj[i]; j < xadj[i + 1]; j++) {
      outgoing_sizes[adjncy[j]]++;
    }
  }
}

void Graph::printGraph() {
  for (int i = 0; i < nvtxs; i++) {
    std::cout << i << ": ";
    for (int j = xadj[i]; j < xadj[i + 1]; j++) {
      std::cout << adjncy[j] << ' ';
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

/*
Format:
# nvtxs
# nedges
# xadj
# adjncy
*/

void Graph::saveGraph() {
  std::cout << "Entering save graph" << std::endl;
  ofstream myfile;
  myfile.open ("../graph-files/test.txt");
  myfile << "# " << nvtxs << "\n";
  myfile << "# " << nedges << "\n";
  
  myfile << "# ";
  for (int i = 0; i <= nvtxs; i++) {
    myfile << xadj[i] << " ";
  }
  myfile << "\n";

  myfile << "# ";
  for (int j = 0; j < nedges; j++) {
    myfile << adjncy[j] << " ";
  }
  myfile << "\n";

  myfile.close();

  std::cout << "Finish save graph" << std::endl;
}

/*
Format:
#
nvtxs
#
nedges
# 
xadj
# 
adjncy
#
nodeidx
*/
void Graph::savePartitionGraph(std::vector<index_t>& nodeidx) {
  std::cout << "Entering save partition graph" << std::endl;
  ofstream myfile;
  myfile.open ("../static-partition-graphs/test.txt");

  // store num vertex and num edges
  myfile << "# nvtxs\n";
  myfile << nvtxs << "\n";
  myfile << "# nedges\n";
  myfile << nedges << "\n";
  
  // store xadj
  myfile << "# xadj\n";
  for (int i = 0; i <= nvtxs; i++) {
    myfile << xadj[i] << "\n";
  }

  // store adjncy
  myfile << "# adjncy\n";
  for (int j = 0; j < nedges; j++) {
    myfile << adjncy[j] << "\n";
  }

  // store nodeidx
  myfile << "# nodeidx\n";
  for (int i = 0; i < nodeidx.size(); i++) {
    myfile << nodeidx[i] << " \n";
  }

  myfile.close();

  std::cout << "Finish save partition graph" << std::endl;
}

/*----------------Functions for graph_partition.h------------------------*/
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
void GraphPartition::FixGraph() {
  index_t i, j, k, l, nvtxs, nedges;
  index_t *xadj, *adjncy, *adjwgt;
  index_t *nxadj, *nadjncy;
  uvw_t *edges;

  nvtxs = graph->nvtxs;
  xadj = graph->xadj.data();
  adjncy = graph->adjncy.data();

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
  ngraph->nedges = 2 * nedges;
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

  // update vertex weight according to the nunmber of incoming edges
  ngraph->vwgt.resize(nvtxs);
  for (k = 0; k < nvtxs; k++) {
    ngraph->vwgt[k] = xadj[k + 1] - xadj[k];
  }
}

void GraphPartition::sortNodesByPart() {
  N2Oidx.resize(graph->nvtxs);
  O2Nidx.resize(graph->nvtxs);
  ngraph->nvtxs = graph->nvtxs;

  for (int i = 0; i < graph->nvtxs; i++) N2Oidx[i] = i;
  std::stable_sort(N2Oidx.begin(), N2Oidx.end(), [this](size_t i1, size_t i2) {
    return parts[i1] < parts[i2];
  });

  // split information will be stored in the parts field of ngraph
  nodeidx.resize(nparts + 1, 0);
  for (int i = 0; i < graph->nvtxs; i++) {
    O2Nidx[N2Oidx[i]] = i;
    nodeidx[parts[i] + 1]++;
  }

  for (int i = 1; i <= nparts; i++) {
    nodeidx[i] = nodeidx[i] + nodeidx[i - 1];
  }

  ngraph->xadj.resize(0);
  ngraph->adjncy.resize(0);
  for (int i = 0; i < graph->nvtxs; i++) {
    ngraph->xadj.push_back(ngraph->adjncy.size());
    for (int j = graph->xadj[N2Oidx[i]]; j < graph->xadj[N2Oidx[i] + 1]; j++) {
      ngraph->adjncy.push_back(O2Nidx[graph->adjncy[j]]);
    }
  }
  ngraph->xadj.push_back(ngraph->adjncy.size());
  ngraph->nedges = graph->nedges;
}

void GraphPartition::partition() {
  FixGraph();
  parts.resize(graph->nvtxs);
  mtmetis_vtx_type ncon = 1;
  mtmetis_wgt_type edgecut;

  typedef uint64_t mtmetis_vtx_type;
  typedef uint64_t mtmetis_adj_type;
  typedef int64_t mtmetis_wgt_type;
  typedef uint64_t mtmetis_pid_type;
  typedef double mtmetis_real_type;

  mtmetis_vtx_type mt_nvtxs = ngraph->nvtxs;
  std::vector<mtmetis_adj_type> *mt_xadj =
      new std::vector<mtmetis_adj_type>(ngraph->xadj.size());
  for (int i = 0; i < ngraph->xadj.size(); i++) {
    (*mt_xadj)[i] = ngraph->xadj[i];
  }
  std::vector<mtmetis_vtx_type> *mt_adjncy =
      new std::vector<mtmetis_vtx_type>(ngraph->adjncy.size());
  for (int i = 0; i < ngraph->adjncy.size(); i++) {
    (*mt_adjncy)[i] = ngraph->adjncy[i];
  }
  std::vector<mtmetis_wgt_type> *mt_vwgt =
      new std::vector<mtmetis_wgt_type>(ngraph->vwgt.size());
  for (int i = 0; i < ngraph->vwgt.size(); i++) {
    (*mt_vwgt)[i] = ngraph->vwgt[i];
  }
  mtmetis_pid_type mt_nparts = nparts;

  std::vector<mtmetis_pid_type> *mt_parts =
      new std::vector<mtmetis_pid_type>(parts.size());

  double *opts = mtmetis_init_options();
  opts[MTMETIS_OPTION_NTHREADS] = nparts;
  MTMETIS_PartGraphRecursive(
      &mt_nvtxs, &ncon, mt_xadj->data(), mt_adjncy->data(), mt_vwgt->data(),
      NULL, NULL, &mt_nparts, NULL, NULL, opts, &edgecut, mt_parts->data());

  for (int i = 0; i < parts.size(); i++) {
    parts[i] = (int)((*mt_parts)[i]);
  }

  delete mt_xadj;
  delete mt_adjncy;
  delete mt_vwgt;
  delete mt_parts;
}

void GraphPartition::newFromPartition() {
  partition();
  sortNodesByPart();
}

void GraphPartition::newFromStatic() {
  std::vector<index_t> partition(nparts, 0);
  parts.resize(graph->nvtxs);

  std::vector<index_t> *index = new std::vector<index_t>(graph->nvtxs);
  for (int i = 0; i < graph->nvtxs; i++) {
    (*index)[i] = i;
  }
  std::stable_sort((*index).begin(), (*index).end(),
                   [this](size_t i1, size_t i2) {
                     return (graph->xadj[i1 + 1] - graph->xadj[i1]) >
                            (graph->xadj[i2 + 1] - graph->xadj[i2]);
                   });

  for (int i = 0; i < graph->nvtxs; i++) {
    int min_index = 0;

    for (int j = 1; j < nparts; j++) {
      if (partition[j] < partition[min_index]) min_index = j;
    }

    parts[(*index)[i]] = min_index;
    partition[min_index] +=
        graph->xadj[(*index)[i] + 1] - graph->xadj[(*index)[i]];
  }
  sortNodesByPart();
}

void GraphPartition::printPartition() {
  std::cout << "Partition" << std::endl;
  std::vector<index_t> partition(nparts, 0);
  for (int i = 0; i < graph->nvtxs; i++) {
    std::cout << parts[i] << ' ';
    partition[parts[i]] += graph->xadj[i + 1] - graph->xadj[i];
  }
  std::cout << std::endl;
  for (int i = 0; i < nparts; i++) {
    std::cout << partition[i] << ' ';
  }
  std::cout << std::endl;
}
