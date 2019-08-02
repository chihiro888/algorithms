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

// usage
// TwoSat ts;
// ts.init(n);
// for(each implication) ts.add(p,q); (both p => q and ~q => ~p are added)
// for(each implication) ts.add(p+n,q); (both ~p => q and ~q => p are added)
// for(each implication) ts.add(p,q+n); (both p => ~q and q => ~p are added)
// ts.negation(p) => p+n
// ts.negation(p+n) => p
//
// vector<bool> result = ts.solve();
// if (n > 0 && result.empty()): NOT satisfiable. ***n > 0*** check is IMPORTANT
// if (result[p] == true): variable p is true
// if (result[p] == false): variable p is false
//
// caution: n != TwoSat.n

struct TwoSatMatrix {
  int n;
  int var;
  vector<vector<char>> graph;
  vector<vector<char>> grev;

  int vcnt;
  vector<int> v;

  void init(int var_) {
    var = var_;
    n = var_ * 2;
    graph.clear(); graph.assign(n, vector<char>(n));
    grev = graph;

    vcnt = 0;
    v.clear(); v.resize(n);
  }

  int negation(int nod) {
    return nod >= var ? nod - var : nod + var;
  }

  // p implies q. p -> q
  void add(int p, int q) {
    graph[p][q] = 1;
    graph[negation(q)][negation(p)] = 1;
    grev[q][p] = 1;
    grev[negation(p)][negation(q)] = 1;
  }

  // and (p or q)
  void addCNF(int p, int q) {
    add(negation(p), q);
  }

  vector<int> emit;
  void dfs(int nod) {
    v[nod] = vcnt;
    for (int next = 0; next < n; next++) {
      if (graph[nod][next] == 0) continue;
      if (v[next] == vcnt) continue;
      dfs(next);
    }
    emit.push_back(nod);
  }
  void dfsrev(int nod) {
    v[nod] = vcnt;
    for (int next = 0; next < n; next++) {
      if (grev[nod][next] == 0) continue;
      if (v[next] == vcnt) continue;
      dfsrev(next);
    }
    emit.push_back(nod);
  }

  vector<bool> solve() {
    vector<bool> solution(var);
    vector<int> scc_check(n);
    int scc_index = 0;

    ++vcnt;
    emit.clear();
    for (int i = 0; i < n; i++) {
      if (v[i] == vcnt) continue;
      dfs(i);
    }

    ++vcnt;
    for (auto start : vector<int>(emit.rbegin(), emit.rend())) {
      if (v[start] == vcnt) continue;
      emit.clear();
      dfsrev(start);
      ++scc_index;
      for (auto node : emit) {
        scc_check[node] = scc_index;
        if (scc_check[negation(node)] == scc_index) {
          // contradiction found
          solution.clear();
          return solution;
        }
        solution[node >= var ? node - var : node] = (node < var);
      }
    }
    return solution;
  }
};

