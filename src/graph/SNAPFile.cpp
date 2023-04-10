/**
 * @file SNAPFile.cpp
 * @brief Implementation of the SNAPFile class.
 * @author Dominique LaSalle <dominique@solidlake.com>
 * Copyright 2018
 * @version 1
 * @date 2018-04-09
 */

#include "graph/SNAPFile.hpp"

#include <stdint.h>

#include <cassert>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

#include "Exception.hpp"

/******************************************************************************
 * TYPES ***********************************************************************
 ******************************************************************************/

namespace {

struct edge_struct {
  uint64_t src;
  uint64_t dst;
};

}  // namespace

/******************************************************************************
 * CONSTANTS *******************************************************************
 ******************************************************************************/

namespace {

const std::string DIRECTED_GRAPH_HEADER("# Directed graph");
const std::string UNDIRECTED_GRAPH_HEADER("# Undirected graph");
const std::string NODES_HEADER("# Nodes: ");

}  // namespace

/******************************************************************************
 * HELPER FUNCTIONS ************************************************************
 ******************************************************************************/

namespace {

bool startsWith(std::string const line, std::string const start) {
  return line.size() >= start.size() &&
         line.compare(0, start.size(), start) == 0;
}

std::vector<std::string> split(std::string const line) {
  std::string const whitespace(" \t");
  std::vector<std::string> chunks;

  size_t start = line.find_first_not_of(whitespace, 0);
  while (start != std::string::npos) {
    size_t next = line.find_first_of(whitespace, start);
    if (next == std::string::npos) {
      next = line.size();
    }

    chunks.emplace_back(line.substr(start, next - start));

    start = line.find_first_not_of(whitespace, next);
  }

  return chunks;
}

std::vector<edge_struct> readEdges(TextFile* file,
                                   uint64_t const numEdges = 0) {
  std::string line;
  std::vector<edge_struct> edges;
  if (numEdges != 0) {
    edges.reserve(numEdges);
  }
  while (file->nextLine(line)) {
    if (line.size() == 0) {
      throw BadFileException("Hit empty line.");
    } else if (line[0] == '#') {
      // skip comment line
    } else {
      edge_struct edge{0, 0};
      char* eptr = (char*)line.data();
      char* sptr = eptr;
      edge.dst = static_cast<uint64_t>(std::strtoull(sptr, &eptr, 10))-1;
      if (eptr == sptr) {
        throw BadFileException("Unable to parse line: '" + line + "'");
      }

      sptr = eptr;
      edge.src = static_cast<uint64_t>(std::strtoull(sptr, &eptr, 10))-1;
      if (eptr == sptr) {
        throw BadFileException("Unable to parse line: '" + line + "'");
      }

      sptr = eptr;

      edges.emplace_back(edge);
    }
  }

  return edges;
}

/**
 * @brief Count the number of vertices and edges in this file.
 *
 * @param file The file.
 * @param numVerticesOut The memory address to write the number of vertices to.
 * @param numEdgesOut The memory address to write the number of edges to.
 */
void countVerticesAndEdges(TextFile* const file, uint64_t* const numVerticesOut,
                           uint64_t* const numEdgesOut) {
  std::unordered_set<uint64_t> vertices;
  std::string line;
  uint64_t numEdges = 0;
  while (file->nextLine(line)) {
    char* eptr = (char*)line.data();
    char* sptr = eptr;
    uint64_t const src = std::strtoull(sptr, &eptr, 10);
    if (eptr == sptr) {
      throw BadFileException("Unable to parse line: '" + line + "'");
    }

    sptr = eptr;
    uint64_t const dst = std::strtoull(sptr, &eptr, 10);
    if (eptr == sptr) {
      throw BadFileException("Unable to parse line: '" + line + "'");
    }

    vertices.emplace(src);
    vertices.emplace(dst);

    ++numEdges;
  }

  *numVerticesOut = static_cast<uint64_t>(vertices.size());
  *numEdgesOut = numEdges;
}

}  // namespace

/******************************************************************************
 * PUBLIC STATIC FUNCTIONS *****************************************************
 ******************************************************************************/

/******************************************************************************
 * PRIVATE METHODS *************************************************************
 ******************************************************************************/
void SNAPFile::readHeader() {
  if (!m_file.isOpenRead()) {
    // open our file for reading if not already
    m_file.openRead();
  }

  m_directed = true;

  m_numVertices = 0;
  m_numEdges = 0;

  // default weight information incase its not preset
  m_hasEdgeWeights = false;

  // parse the header search for 'Directed'/'Undirected' and 'Nodes: Edges:'
  std::string line;
  while (m_file.nextLine(line) && line.size() > 0 && line[0] == '#') {
    if (startsWith(line, DIRECTED_GRAPH_HEADER)) {
      m_directed = true;
    } else if (startsWith(line, UNDIRECTED_GRAPH_HEADER)) {
      m_directed = false;
    } else if (startsWith(line, NODES_HEADER)) {
      // parse estimated size '# Nodes: 10 Edges: 15'
      std::vector<std::string> const chunks = split(line);
      // first make sure this line looks sane
      if (chunks.size() != 5 || chunks[3].compare("Edges:") != 0) {
        throw BadFileException(std::string("Badly formed header line: ") +
                               line);
      }

      try {
        m_numVertices = static_cast<uint64_t>(std::stoull(chunks[2]));
        m_numEdges = static_cast<uint64_t>(std::stoull(chunks[4]));
      } catch (std::exception const& e) {
        throw BadFileException(std::string("Failed to parse vertices and "
                                           "edges from header line: ") +
                               e.what());
      }
    } else {
      // do nothing
    }
  }

  // count undirected edges twice
  if (!m_directed && m_numEdges != 0) {
    m_numEdges *= 2;
  }

  // if node and edge counts weren't found, we can still proceed, but need
  // to parse the whole file, counting unique vertices and edges
  if (m_numVertices == 0 || m_numEdges == 0) {
    // sets m_numVertices and m_numEdges
    countVerticesAndEdges(&m_file, &m_numVertices, &m_numEdges);
  }

  // move back to the start of the file
  m_file.resetStream();
}

/******************************************************************************
 * CONSTRUCTORS / DESTRUCTOR ***************************************************
 ******************************************************************************/

SNAPFile::SNAPFile(std::string const& filename)
    : m_infoSet(false),
      m_numVertices(0),
      m_numEdges(0),
      m_hasEdgeWeights(false),
      m_directed(true),
      m_line(),
      m_file(filename) {
  // do nothing
}

SNAPFile::~SNAPFile() {
  // do nothing
}

/******************************************************************************
 * PUBLIC METHODS **************************************************************
 ******************************************************************************/

void SNAPFile::read(uint64_t* const xadj, uint64_t* const adjncy,
                    uint64_t* out_edges) {
  if (m_numVertices == 0) {
    return;
  }

  uint64_t const interval = m_numEdges * 2 > 100 ? m_numEdges * 2 / 100 : 1;
  double const increment = 1.0 / 100.0;

  std::string line;

  // read in all edges
  const std::vector<edge_struct> edges = readEdges(&m_file, m_numEdges);

  // zero out xadj
  for (uint64_t i = 0; i < m_numVertices; ++i) {
    xadj[i] = 0;
  }

  // populate xadj
  size_t edgesProcessed = 0;
  for (edge_struct const& edge : edges) {
    if (edge.src >= m_numVertices) {
      throw BadFileException(std::string("Invalid vertex: ") +
                             std::to_string(edge.src));
    } else if (edge.dst >= m_numVertices) {
      throw BadFileException(std::string("Invalid vertex: ") +
                             std::to_string(edge.src));
    }
    ++xadj[edge.src + 1];
    ++out_edges[edge.dst];
    if (!m_directed) {
      ++xadj[edge.dst + 1];
    }

    ++edgesProcessed;
  }

  // shift xadj and prefixsum
  xadj[0] = 0;
  for (uint64_t v = 1; v <= m_numVertices; ++v) {
    xadj[v] += xadj[v - 1];
  }
  for (uint64_t v = m_numVertices; v > 0; --v) {
    xadj[v] = xadj[v - 1];
  }
  assert(xadj[0] == 0);

  // fill in edges
  for (edge_struct const& edge : edges) {
    uint64_t const srcIdx = xadj[edge.src + 1];
    adjncy[srcIdx] = edge.dst;
    ++xadj[edge.src + 1];

    if (!m_directed) {
      uint64_t const dstIdx = xadj[edge.dst + 1];
      adjncy[dstIdx] = edge.src;
      ++xadj[edge.dst + 1];
    }

    ++edgesProcessed;
  }
  assert(xadj[m_numVertices] == m_numEdges);
}

void SNAPFile::getInfo(uint64_t& nvtxs, uint64_t& nedges) {
  readHeader();

  nvtxs = m_numVertices;
  nedges = m_numEdges;

  m_infoSet = true;
}
