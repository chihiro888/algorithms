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

struct TreePath {
  typedef long long cost_t;
  struct edge_t {
    int next;
    cost_t cost;
  };
  int n;
  vector<vector<edge_t>> graph;
  vector<int> depth;
  vector<vector<pair<int, cost_t>>> jump;
  int jumplen;

  TreePath(int n) : n(n), graph(n) {
    for (jumplen = 1; (1ll << jumplen) - 1 < (n - 1); jumplen++);
  }

  void AddEdge(int a, int b, cost_t l) {
    graph[a].emplace_back(edge_t{b, l});
    graph[b].emplace_back(edge_t{a, l});
  }

  void Init() {
    jump.assign(jumplen, vector<pair<int, cost_t>>(n));
    depth.assign(n, -1);
    queue<int> q;
    q.push(0);
    jump[0][0].first = 0;
    jump[0][0].second = 0;
    depth[0] = 0;
    while (!q.empty()) {
      int cur = q.front(); q.pop();
      for (const auto& e : graph[cur]) {
        int next = e.next;
        if (depth[next] == -1) {
          q.push(next);
          jump[0][next].first = cur;
          jump[0][next].second = e.cost;
          depth[next] = depth[cur] + 1;
        }
      }
    }
    for (int j = 1; j < jumplen; j++) {
      for (int i = 0; i < n; i++) {
        int par = jump[j-1][i].first;
        cost_t cost = jump[j-1][i].second;
        cost += jump[j-1][par].second;
        par = jump[j-1][par].first;
        jump[j][i] = make_pair(par, cost);
      }
    }
  }
  // return (lca, number of edges, cost sum)
  tuple<int, int, cost_t> GetPathLengthAndCost(int a, int b) const {
    cost_t sum = 0;
    int cnt = 0;
    if (depth[a] < depth[b]) swap(a, b);
    for (int j = jumplen - 1; j >= 0 && depth[a] > depth[b]; j--) {
      if (depth[a] - depth[b] >= (1 << j)) {
        sum += jump[j][a].second;
        a = jump[j][a].first;
        cnt += (1 << j);
      }
    }
    if (a == b) return make_tuple(a, cnt, sum);
    for (int j = jumplen - 1; j >= 0; j--) {
      if (jump[j][a].first == jump[j][b].first) continue;
      sum += jump[j][a].second;
      sum += jump[j][b].second;
      a = jump[j][a].first;
      b = jump[j][b].first;
      cnt += (2 << j);
    }
    sum += jump[0][a].second;
    sum += jump[0][b].second;
    cnt += 2;
    return make_tuple(jump[0][a].first, cnt, sum);
  }
};

