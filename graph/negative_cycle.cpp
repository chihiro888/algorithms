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
#include <iostream>
#include <tuple>
#include <limits>

using namespace std;

/*
  edge_t will look like this:
  struct edge_t {
    int next;
    weight_t get_weight() const;
    bool is_valid() const;
  };
	can be used for cycle canceling
*/
template<typename edge_t>
vector<pair<int, int>> negative_cycle_search(const vector<vector<edge_t>>& graph) {
  typedef decltype(graph[0][0].get_weight()) weight_t;
  vector<pair<int, int>> answer; // (node, edge_index)..
  int n = static_cast<int>(graph.size());
  vector<weight_t> dist(n, 0);
  vector<pair<int, int>> from(n, make_pair(-1, -1)); // node, edge index
  vector<pair<int, weight_t>> cycle_check(n, make_pair(0, 0)); // vcnt, distance
  int vcnt = 1;
  for (int loop = 0; loop < n - 1; loop++) {
    bool relaxed = false;
    // relax
    for (int i = 0; i < n; i++) {
      for (int e = 0; e < static_cast<int>(graph[i].size()); e++) {
        const auto& edge = graph[i][e];
        if (!edge.is_valid()) continue;
        int next = edge.next;
        weight_t ndist = dist[i] + edge.get_weight();
        if (dist[next] > ndist) {
          dist[next] = ndist;
          from[next] = make_pair(i, e);
          relaxed = true;
        }
      }
    }
    // finished
    if (!relaxed) return answer;
    // cycle check
    int check_starter = vcnt;
    for (int i = 0; i < n; i++) {
      if (cycle_check[i].first >= check_starter) continue;
      const int now = vcnt++;
      weight_t sum = 0;
      cycle_check[i] = make_pair(now, sum);
      for (int p = i;;) {
        int prev, pe;
        tie(prev, pe) = from[p];
        if (prev < 0) break;
        sum += graph[prev][pe].get_weight();
        if (cycle_check[prev].first == now) {
          if (sum >= cycle_check[prev].second) break;
          // found negative cycle
          int base = p;
          do {
            int np = from[p].first;
            answer.emplace_back(from[p]);
            p = np;
          } while (p != base);
          return answer;
        } else if (cycle_check[prev].first >= check_starter) break;
        cycle_check[prev] = make_pair(now, sum);
        p = prev;
      }
    }
  } // end of loop
  return answer;
}

