#include <cstdio>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <climits>
#include <cstring>
#include <string>
#include <vector>
#include <queue>
#include <numeric>
#include <functional>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <thread>
#include <tuple>
#include <limits>

using namespace std;


// Construct a centroid decomposition tree from the input tree.
// Example:
//   CentroidDecomposition c(n);
//   for (each edge) c.AddEdge(edge);
//   c.Decompose();
// O(n log n)
struct CentroidDecomposition {
  struct edge_t {
    int next;
    int inv;
    bool cut;
  };

  struct centroid_info_t {
    int parent;
    vector<int> children;
  };

  int n;
  vector<vector<edge_t>> graph; // input must be a tree
  int decomposition_root;
  vector<centroid_info_t> decomposition;

  CentroidDecomposition(int n) : n(n), graph(n) { }

  void AddEdge(int a, int b) {
    if (a == b) return;
    edge_t forward = { b, static_cast<int>(graph[b].size()), false };
    edge_t backward = { a, static_cast<int>(graph[a].size()), false };
    graph[a].emplace_back(forward);
    graph[b].emplace_back(backward);
  }

  void Decompose() {
    DecomposeContext context;
    context.parent.assign(n, -1);
    context.submax.assign(n, 0);
    context.size.assign(n, -1);
    decomposition.assign(n, {});
    decompose_recursive(-1, 0, &context);
  }

  struct DecomposeContext {
    vector<int> size; // -1 for unvisited nodes. reusing array to save memory.
    vector<int> submax;
    vector<int> parent;
  };
  void decompose_recursive(int dec_parent, int start, DecomposeContext* c) {
    vector<int> q;
    q.push_back(start);
    c->size[start] = 0;
    c->submax[start] = 0;
    c->parent[start] = -1;
    for (int h = 0; h < q.size(); h++) {
      int cur = q[h];
      for (auto& edge : graph[cur]) {
        if (edge.cut) continue;
        if (c->size[edge.next] == 0) continue;
        q.push_back(edge.next);
        c->size[edge.next] = 0;
        c->submax[edge.next] = 0;
        c->parent[edge.next] = cur;
      }
    }
    const int component_size = static_cast<int>(q.size());
    for (int qi = component_size - 1; qi >= 0; qi--) {
      int node = q[qi];
      c->size[node] += 1;
      int par = c->parent[node];
      if (par != -1) {
        c->size[par] += c->size[node];
        c->submax[par] = max(c->submax[par], c->size[node]);
      }
    }
    const int allowed_max = component_size / 2;
    int centroid = -1;
    for (int qi = 0; qi < component_size; qi++) {
      int node = q[qi];
      int opposite_size = component_size - c->size[node];
      int max_split_size = max(opposite_size, c->submax[node]);
      if (max_split_size <= allowed_max) {
        centroid = node;
        break;
      }
    }
    for (auto node : q) c->size[node] = -1;
    decomposition[centroid].parent = dec_parent;
    if (dec_parent == -1) decomposition_root = centroid;
    else decomposition[dec_parent].children.push_back(centroid);
    for (auto& edge : graph[centroid]) {
      if (edge.cut) continue;
      int next = edge.next;
      edge.cut = true;
      graph[next][edge.inv].cut = true;
      decompose_recursive(centroid, next, c);
    }
  }
};


