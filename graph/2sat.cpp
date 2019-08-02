#include <algorithm>
#include <vector>
#include <stack>
#include <tuple>

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

struct TwoSat
{
  int n;
  int var;
  vector<vector<int>> graph;
  vector<vector<int>> grev;

  int vcnt;
  vector<int> v;

  void init(int var_) {
    var = var_;
    n = var_*2;
    graph.clear(); graph.resize(n);
    grev.clear(); grev.resize(n);

    vcnt = 0;
    v.clear(); v.resize(n);
  }

  int negation(int nod) {
    return nod >= var ? nod - var : nod + var;
  }

  // p implies q. p -> q
  void add(int p, int q) {
    graph[p].push_back(q);
    graph[negation(q)].push_back(negation(p));
    grev[q].push_back(p);
    grev[negation(p)].push_back(negation(q));
  }

  // and (p or q)
  void addCNF(int p, int q) {
    add(negation(p), q);
  }

  vector<int> emit;
  void dfs(int start, vector<vector<int>> &graph) {
    stack<pair<int, int>> s;
    v[start] = vcnt;
    s.emplace(start, 0);
    while (!s.empty()) {
      int nod = s.top().first;
      int &i = s.top().second;
      int ecnt = graph[nod].size();
      for (; i < ecnt; i++) {
        int next = graph[nod][i];
        if (v[next] == vcnt) continue;
        v[next] = vcnt;
        s.emplace(next, 0);
        break;
      }
      if (i >= ecnt) {
        emit.push_back(nod);
        s.pop();
      }
    }
  }

  vector<bool> solve() {
    vector<bool> solution(var);
    vector<int> scc_check(n);
    int scc_index = 0;

    ++vcnt;
    emit.clear();
    for(int i = 0; i < n; i++) {
      if (v[i] == vcnt) continue;
      dfs(i, graph);
    }

    ++vcnt;
    for(auto start : vector<int>(emit.rbegin(),emit.rend())) {
      if (v[start] == vcnt) continue;
      emit.clear();
      dfs(start, grev);
      ++scc_index;
      for(auto node : emit) {
        scc_check[node] = scc_index;
        if (scc_check[negation(node)] == scc_index) {
          // contradiction found
          solution.clear();
          return solution;
        }
        solution[node >= var ? node-var : node] = (node < var);
      }
    }

    return solution;
  }
};
