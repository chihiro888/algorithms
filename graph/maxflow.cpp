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
#include <utility>

using namespace std;

// usage:
// MaxFlowDinic::init(n);
// MaxFlowDinic::addEdge(0, 1, 100, 100); // for bidirectional edge
// MaxFlowDinic::addEdge(1, 2, 100); // directional edge
// result = MaxFlowDinic::solve(0, 2); // source -> sink
// graph[i][edgeIndex].res -> residual
//
// in order to find out the minimum cut, use `l'.
// if l[i] == 0, i is unreachable.
struct MaxFlowDinic {
  typedef long long cap_t;

  struct Edge
  {
    int next;
    int inv; /* inverse edge index */
    cap_t res; /* residual */
    cap_t save; /* save whatever you want. TODO: just remove if you don't need */
  };

  int n;
  vector<vector<Edge>> graph;

  vector<int> q, l, start;

  void init(int _n) {
    n = _n;
    graph.resize(n);
    for (int i = 0; i < n; i++) graph[i].clear();
  }
  void addNodes(int count) {
    n += count;
    graph.resize(n);
  }

  void addEdgeSave(int s, int e, cap_t cap, cap_t save) {
    Edge forward = { e, (int)graph[e].size() + (s == e ? 1 : 0), cap, save };
    Edge reverse = { s, (int)graph[s].size(), 0, -1 };
    graph[s].push_back(forward);
    graph[e].push_back(reverse);
  }

  void addEdge(int s, int e, cap_t cap, cap_t caprev = 0) {
    Edge forward = { e, (int)graph[e].size() + (s == e ? 1 : 0), cap, -1 };
    Edge reverse = { s, (int)graph[s].size(), caprev, -1 };
    graph[s].push_back(forward);
    graph[e].push_back(reverse);
  }

  bool assignLevel(int source, int sink) {
    int t = 0;
    l.assign(n, 0);
    l[source] = 1;
    q[t++] = source;
    for (int h = 0; h < t && !l[sink]; h++) {
      int cur = q[h];
      for (const auto &edge : graph[cur]) {
        int next = edge.next;
        if (l[next]) continue;
        if (edge.res > 0) {
          l[next] = l[cur] + 1;
          q[t++] = next;
        }
      }
    }
    return l[sink] != 0;
  }

  cap_t blockFlow(int cur, int sink, cap_t currentFlow) {
    if (cur == sink) return currentFlow;
    for (auto &i = start[cur]; i < (int)graph[cur].size(); i++) {
      int next = graph[cur][i].next;
      if (graph[cur][i].res == 0 || l[next] != l[cur] + 1)
        continue;
      if (cap_t res = blockFlow(next, sink, min(graph[cur][i].res, currentFlow))) {
        int inv = graph[cur][i].inv;
        graph[cur][i].res -= res;
        graph[next][inv].res += res;
        return res;
      }
    }
    return 0;
  }

  cap_t solve(int source, int sink)
  {
    if (source == sink) {
      // invalid case
      return -1;
    }
    q.resize(n);
    l.resize(n);
    cap_t ans = 0;
    while (assignLevel(source, sink)) {
      start.assign(n, 0);
      while (cap_t flow = blockFlow(source, sink, numeric_limits<cap_t>::max())) {
        ans += flow;
      }
    }
    return ans;
  }
};

// Finding actual instance:
// (capacity - edge.res) is the actual flow that is flown in this problem.
struct MaxFlowEdgeDemand {
  typedef MaxFlowDinic::cap_t cap_t;
  MaxFlowDinic mf;

  vector<cap_t> nodeSum;

  void init(int _n) {
    mf.init(_n + 2);
    nodeSum.resize(_n + 2);
  }

  void addNodes(int count) {
    mf.addNodes(count);
    nodeSum.resize(mf.n + 2);
  }

  void addEdgeSave(int s, int e, cap_t demand, cap_t capacity, cap_t save) {
    mf.addEdgeSave(s + 2, e + 2, capacity - demand, save);
    nodeSum[s + 2] -= demand;
    nodeSum[e + 2] += demand;
  }

  void addEdge(int s, int e, cap_t demand, cap_t capacity) {
    mf.addEdge(s + 2, e + 2, capacity - demand);
    nodeSum[s + 2] -= demand;
    nodeSum[e + 2] += demand;
  }

  // true if possible
  bool solveCirculation() {
    cap_t saturation = 0;
    for (int i = 2; i < (int)nodeSum.size(); i++) {
      auto cap = nodeSum[i];
      if (cap == 0) continue;
      if (cap < 0) {
        mf.addEdge(i, 1, -cap);
      }
      else if (cap > 0) {
        mf.addEdge(0, i, cap);
        saturation += cap;
      }
    }
    cap_t result = mf.solve(0, 1);
    return result == saturation;
  }

  // -1 if it is impossible to satisfy demand
  // return max flow.
  cap_t solveMax(int source, int sink) {
    mf.addEdge(sink + 2, source + 2, numeric_limits<cap_t>::max());
    cap_t saturation = 0;
    for (int i = 2; i < (int)nodeSum.size(); i++) {
      auto cap = nodeSum[i];
      if (cap == 0) continue;
      if (cap < 0) {
        mf.addEdge(i, 1, -cap);
      }
      else if (cap > 0) {
        mf.addEdge(0, i, cap);
        saturation += cap;
      }
    }
    cap_t result = mf.solve(0, 1);
    if (result != saturation) {
      return -1;
    }
    cap_t maxFlow = mf.solve(source + 2, sink + 2);
    return maxFlow;
  }
};


// maximize sum(selected profit) - sum(selected cost)
// -> maximize sum(all profit) - sum(unselected profit) - sum(selected cost)
// -> minimize sum(unselected profit) + sum(selected cost)
// -> minimize sum(unselected profit) + sum(selected cost) + infinity * sum(selected project needs unselected project)
// -> mincut on graph!
// let A={unselected profit}, B={selected cost}.
// Solution: connect profit projects to all dependent cost projects with infinity capacity. -> this is expensive,
//        but anyway this forces all dependent cost projects of selected profit projects will be cut.
// Solution improvement: no need to connect all. It just requires transitive path.
// after connecting them, you need to cut profit or cut( = accept) cost
struct ProjectSelection {
    typedef MaxFlowDinic::cap_t profit_t;
    // profit can be negative if it induces costs
    MaxFlowDinic mf;
    int source, sink;

    vector<profit_t> profits;

    ProjectSelection(vector<profit_t> profits) : profits(move(profits)) {
        int n = (int)profits.size();
        mf.init(n + 2);
        source = n;
        sink = n + 1;
        for (int i = 0; i < n; i++) {
            auto p = profits[i];
            if (p > 0) {
                // profit!
                mf.addEdge(source, i, p);
            } else if (p < 0) {
                // costly
                mf.addEdge(i, sink, -p);
            }
        }
    }

    // "a" depends on "b". To do "a", you need "b" before.
    void AddDependency(int a, int b) {
        mf.addEdge(a, b, numeric_limits<MaxFlowDinic::cap_t>::max());
    }

    // return the best profit
    //
    // To get the actual selection,
    // 1. get all selected profit projects (selected = not cut edges, so still reachable -> mf.l can be used)
    // 2. find all dependencies by BFS
    profit_t Solve() {
        profit_t sum = 0;
        for (int i = 0; i < (int)profits.size(); i++) {
            auto p = profits[i];
            if (p > 0) {
                sum += p;
            }
        }
        sum -= mf.solve(source, sink);
        return sum;
    }
};


int main() {
    // CF366 D
    int n, m;
    scanf("%d%d", &n, &m);
    int r, b;
    scanf("%d%d", &r, &b);
    vector<pair<int, int>> points; points.reserve(n);
    map<int, int> X, Y; // x -> count
    map<int, int> Xi, Yi; // x -> count
    for (int i = 0; i < n; i++) {
        int x, y;
        scanf("%d%d", &x, &y);
        points.emplace_back(x, y);
        X[x]++;
        Y[y]++;
    }
    vector<pair<int, int>> rangexy(X.size() + Y.size());
    {

        int i = 0;
        for (auto kv : X) {
            rangexy[i].second = kv.second;
            Xi[kv.first] = i;
            i++;
        }
        for (auto kv : Y) {
            rangexy[i].second = kv.second;
            Yi[kv.first] = i;
            i++;
        }
    }
    MaxFlowEdgeDemand mf;
    int source = (int)(X.size() + Y.size());
    int sink = (int)(X.size() + Y.size() + 1);
    mf.init((int)(X.size() + Y.size() + 2));
    for (int i = 0; i < n; i++) {
        int xi = Xi[points[i].first];
        int yi = Yi[points[i].second];
        mf.addEdgeSave(xi, yi, 0, 1, i);
    }
    for (int i = 0; i < m; i++) {
        int t, l, d;
        scanf("%d%d%d", &t, &l, &d);
        if (t == 1) {
            auto I = X.find(l);
            if (I == X.end()) continue;
            int cnt = I->second;
            int lower = (-d + cnt); if (lower % 2) lower++; lower /= 2;
            int upper = (d + cnt); if (upper % 2) upper--; upper /= 2;
            rangexy[Xi[l]].first = max(rangexy[Xi[l]].first, lower);
            rangexy[Xi[l]].second = min(rangexy[Xi[l]].second, upper);
        } else if (t == 2) {
            auto I = Y.find(l);
            if (I == Y.end()) continue;
            int cnt = I->second;
            int lower = (-d + cnt); if (lower % 2) lower++; lower /= 2;
            int upper = (d + cnt); if (upper % 2) upper--; upper /= 2;
            rangexy[Yi[l]].first = max(rangexy[Yi[l]].first, lower);
            rangexy[Yi[l]].second = min(rangexy[Yi[l]].second, upper);
        }
    }
    for (int i = 0; i < (int)rangexy.size(); i++) {
        if (rangexy[i].first > rangexy[i].second) {
            printf("-1\n");
            return 0;
        }
        if (i < (int)X.size()) {
            mf.addEdge(source, i, rangexy[i].first, rangexy[i].second);
        } else {
            mf.addEdge(i, sink, rangexy[i].first, rangexy[i].second);
        }
    }

    auto result = mf.solveFeasibility(source, sink);
    if (result < 0) {
        printf("-1\n");
        return 0;
    }

    bool swapped = false;
    if (r > b) {
        swap(r, b);
        swapped = true;
    }

    long long cost = (long long)r * n + (long long)(n-result) * (b-r);
    printf("%lld\n", cost);
    string ans(n, '?');
    for (int i = 0; i < (int)X.size(); i++) {
        for (const auto &edge : mf.mf.graph[i + 2]) {
            if (edge.save >= 0) {
                bool filled = (edge.res == 0);
                ans[edge.save] = ((swapped == filled) ? 'b' : 'r');
            }
        }
    }
    printf("%s\n", ans.c_str());
    return 0;
}


