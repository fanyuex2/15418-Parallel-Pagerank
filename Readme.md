## Input Graph
Graph should follow below format, nodes should index from 0

```
# Directed graph (or Undirected graph)
# Nodes: 6 Edges: 5
0 1 
0 2
0 3
0 4
0 5
```

## Installation 
This project relies on mtmetis and GKlib libraries. Their binary files should already be included in this directory and added as dependency. If not, please install mt-metis and metis (which include GKlib) to this directory.

I haven't figured out how to install mt-metis with 32bits. The project assumes default configuration of those two libraries, meaning mt-metis(64 bits) and metis(32 bits). 

## Compile and run
```
mkdir build
cd build && cmake ..
./pagerank <num_workers> <method> <runs>
<method>: naive, metis, static
<runs>: to compute the average time across all runs
```
The scores will be compared with the sequential version to ensure correctness and the time of only pagerank iterations will be the output.