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

struct StronglyConnectedComponents {
  // input
  vector<vector<int>> graph;

  // other
  vector<int> mark;
  vector<int> order;
  vector<int> low;
  int dfsind;
  vector<int> component;
  vector<vector<int>> all;
  void dfs(int cur) {
    mark[cur] = 1;
    order[cur] = dfsind++;
    low[cur] = order[cur];
    int pushed_index = component.size();
    component.push_back(cur);
    for (auto next : graph[cur]) {
      if (mark[next] == 2) continue; // cross-edge, other component, checked subtree
      else if (mark[next] == 1) { // ancestor
        low[cur] = min(low[cur], order[next]);
      } else {
        dfs(next);
        low[cur] = min(low[cur], low[next]);
      }
    }
    if (order[cur] == low[cur]) {
      all.emplace_back(component.begin() + pushed_index, component.end());
      for (auto node : all.back()) mark[node] = 2;
      component.resize(pushed_index);
    }
  }

  // return reverse topological order
  vector<vector<int>> decompose() {
    int n = graph.size();
    mark.assign(n, -1);
    order.assign(n, -1);
    low.assign(n, -1);
    dfsind = 0;
    component.clear();
    all.clear();
    for (int i = 0; i < n; i++) if (mark[i] == -1) dfs(i);
    vector<vector<int>> result; result.swap(all);
    return result;
  }
};

