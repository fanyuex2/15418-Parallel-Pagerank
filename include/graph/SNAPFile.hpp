/**
* Adapted from below source. modified interface to produce incoming/outgoing
@file SNAPFile.hpp
* @brief The SNAPFile class.
* @author Dominique LaSalle <dominique@solidlake.com>
* Copyright 2018
* @version 1
* @date 2018-04-07
*/

#include <stdint.h>

#include "graph/TextFile.hpp"

/**
 * @brief A class for reading and writing files in the SNAP graph text format.
 * SNAP is an unweighted IJ graph format.
 *
 * ```
 * # Directed graph (each unordered pair of nodes is saved once): example.txt
 * # Description of the graph.
 * # Nodes: 6 Edges: 5
 * # FromNodeId	ToNodeId
 * 0	1
 * 0	2
 * 0	3
 * 0	4
 * 0	5
 * ```
 * The first line must either start with `# Undirected graph`, or
 * `# Directed graph`. Undirected graphs get their edges counted in both
 * directions, whereas directed graphs get the counted in only one.
 *
 * @misc{snapnets,
 *   author       = {Jure Leskovec and Andrej Krevl},
 *   title        = {{SNAP Datasets}: {Stanford} Large Network Dataset
 * Collection}, howpublished = {\url{http://snap.stanford.edu/data}}, month =
 * jun, year         = 2014
 * }
 */
class SNAPFile {
 public:
  /**
   * @brief Create a new MetisFile for reading and writing.
   *
   * @param fname The filename/path.
   */
  SNAPFile(std::string const& fname);

  /**
   * @brief Close file and free any memory.
   */
  virtual ~SNAPFile();

  /**
   * @brief Read the CSR structure of the graph.
   *
   * @param xadj The adjacency list pointer (length nvtxs+1).
   * @param adjncy The adjacency list (length nedges).
   */
  virtual void read(int* xadj, int* adjncy);

  /**
   * @brief Get information about the graph.
   *
   * @param nvtxs The number of vertices.
   * @param nedges The number of edges (directed).
   * @param nvwgt The number of vertex weights (constraints).
   * @param ewgts Whether or not edge weights are specified.
   */
  virtual void getInfo(int& nvtxs, int& nedges);

 private:
  /**
   * @brief Whether or not the graph information has been set.
   */
  bool m_infoSet;

  /**
   * @brief The number of vertices in the graph.
   */
  int m_numVertices;

  /**
   * @brief The number of non-zeros in the matrix.
   */
  int m_numEdges;

  /**
   * @brief Whether or not the graph file stores edge weights.
   */
  bool m_hasEdgeWeights;

  /**
   * @brief Whether or not the graph is directed.
   */
  bool m_directed;

  /**
   * @brief Line buffer.
   */
  std::string m_line;

  /**
   * @brief The underlying text file.
   */
  TextFile m_file;

  /**
   * @brief Get the next non-comment line from the file.
   *
   * @param line The line to fill.
   *
   * @return True if the line was filled.
   */
  bool nextNoncommentLine(std::string& line);

  /**
   * @brief Read the header of this matrix file. Populates internal fields
   * with the header information.
   */
  virtual void readHeader();
};
