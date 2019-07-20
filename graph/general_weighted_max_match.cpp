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

#define MYASSERT(c)

// Tested
// - Baekjoon OJ 15741
// O(V^4)
// Ref: Galil, Micali, Gabow(1983) "An O(EV log V) Algorithm for Finding a Maximal Weighted Matching in General Graphs"
// Based on Edmond's Blossom Algorithm
struct MaxWeightMatching {
  typedef long long weight_t;
  typedef int blossom_ind_t; // less than 2*n. just an alias, used for {blossom,subblossom,node} index
  typedef int node_ind_t; // less than n, only for node index.
  enum : int { weight_scale = 4 };

  struct edge_t {
    node_ind_t next;
    weight_t weight;
    weight_t cached_z;
  };

  int n;
  vector<vector<edge_t>> graph;
  vector<node_ind_t> mate; // valid only if propagated

  MaxWeightMatching(int n) : n(n), graph(n) { }

  void AddEdge(node_ind_t a, node_ind_t b, weight_t weight) {
    if (a == b) return;
    graph[a].emplace_back(edge_t{ b, weight * weight_scale, 0 });
    graph[b].emplace_back(edge_t{ a, weight * weight_scale, 0 });
  }

  void normalize_graph() {
    for (node_ind_t i = 0; i < n; i++) {
      sort(graph[i].begin(), graph[i].end(), [](const edge_t &a, const edge_t &b) { return tie(a.next, a.weight) < tie(b.next, b.weight); });
      // remove duplicate edges, pick largest weight.
      int p = -1;
      for (int j = 0; j < graph[i].size(); j++) {
        if (p == -1 || graph[i][p].next != graph[i][j].next) p++;
        graph[i][p] = graph[i][j];
      }
    }
  }

  int greedy_matching() {
    int cnt = 0;
    for (node_ind_t i = 0; i < n; i++) {
      if (mate[i] != -1) continue;
      for (const auto& edge : graph[i]) {
        if (pi(i, edge.next, edge.weight, edge.cached_z) == 0 && mate[edge.next] == -1) {
          mate[edge.next] = i;
          mate[i] = edge.next;
          cnt++;
          break;
        }
      }
    }
    return cnt;
  }

  pair<weight_t, int> Match() {
    normalize_graph();
    int cnt = 0;
    init();
    cnt += greedy_matching();
    for (;;) {
      auto result = phase();
      if (result < 0) break;
      else if (result == 0) {
        // slack updated
      }
      else { // normal matching
        cnt += result;
      }
    }
    for (int i = 0; i < subblossoms.size(); i++) {
      if (subblossoms[i].invalid) continue;
      blossom_ind_t b = i + n;
      if (blossom[b] == b) { // outermost
        propagate_lazy(b, true);
      }
    }
    weight_t weightsum = 0;
    for (int i = 0; i < n; i++) {
      if (mate[i] < i) continue;
      weight_t cur = 0;
      for (auto& edge : graph[i]) {
        if (edge.next == mate[i]) {
          cur = max(cur, edge.weight);
        }
      }
      weightsum += cur;
    }
    return make_pair(weightsum / weight_scale, cnt);
  }

  // represents both subblossom and blossom. Note: "subblossom" is not "blossom" any more.
  struct blossom_t {
    bool invalid; // true if garbage (neither subblossom or blossom)
    node_ind_t raw_base; // for faster lookup, just caching. valid only if it is a blossom and not a subblossom.
    node_ind_t pending_base; // lazy update
    node_ind_t pending_mate; // mate of pending base
    int base_ind;
    weight_t z; // dual variable
    unordered_map<blossom_ind_t, int> ielts;
    vector<blossom_ind_t> elts; // vertex if <n, subblossom if >=n
    vector<pair<node_ind_t, node_ind_t>> edges; // raw edge between (elts[i], elts[i+1])
  };
  vector<blossom_t> subblossoms; // [blossom-n] to blossom information. Could be either subblossom or blossom.
  vector<weight_t> u; // node_ind_t->weight_t: dual variable for vertex
  // pi(i,j) = u[i]+u[j]-w[i,j]+sum(z[the blossom that has both i and j])
  vector<blossom_ind_t> blossom; // [node, blossom]->blossom index
  vector<blossom_ind_t> direct_container; // [blossom] to blossom or -1

  void init() {
    mate.assign(n, -1);

    // the algorithm relies on same initial u
    // TODO: find better alternatives
    weight_t largest(0);
    for (int i = 0; i < n; i++) {
      for (auto& edge : graph[i]) {
        edge.cached_z = 0;
        largest = max(largest, edge.weight);
      }
    }
    u.assign(n, largest / 2);

    blossom.resize(2*n); iota(blossom.begin(), blossom.end(), 0);
    subblossoms.clear(); subblossoms.reserve(n);
    lca_marker.assign(2 * n, 0); lca_marker_value = 0;
    blossom_processed.assign(2 * n, 0); blossom_processed_marker = 0;
    direct_container.assign(2 * n, -1);
  }

  void mark_blossom(blossom_ind_t b, blossom_ind_t new_blossom) {
    blossom[b] = new_blossom;
    if (b >= n) {
      for (auto elt : subblossoms[b - n].elts) mark_blossom(elt, new_blossom);
    }
  }

  weight_t pi(node_ind_t a, node_ind_t b, weight_t weight, weight_t cached_z) const {
    if (a == b) return 0;
    weight = u[a] + u[b] - weight + cached_z;
    MYASSERT(weight >= 0);
    return weight;
  }

  node_ind_t get_base(blossom_ind_t node) const {
    node = blossom[node]; // get outermost blossom.
    if (node >= n) node = subblossoms[node - n].raw_base;
    return node;
  }
  node_ind_t get_pending_mate(blossom_ind_t b) {
    b = blossom[b]; // get outermost blossom.
    if (b < n) return mate[b];
    if (subblossoms[b - n].pending_base >= 0)
      return subblossoms[b - n].pending_mate;
    return mate[subblossoms[b - n].raw_base];
  }


  // variables calculated for every search
  vector<int> label; // blossom to -1: free, 0: even(S), 1: odd(T)
  queue<blossom_ind_t> q; // blossom, only even levels
  vector<blossom_ind_t> parent; // blossom to -1 or blossom
  vector<pair<node_ind_t, node_ind_t>> parent_edge; // {blossom} to underlying edge (parent[blossom], blossom)
  vector<blossom_ind_t> forest_root; // blossom to -1 or blossom

  blossom_ind_t get_parent(blossom_ind_t b) {
    b = parent[b];
    if (b == -1) return -1;
    return blossom[b];
  }

  vector<int> lca_marker;
  int lca_marker_value;
  blossom_ind_t getlca(blossom_ind_t a, blossom_ind_t b) {
    if (a == b) return a;
    ++lca_marker_value;
    lca_marker[a] = lca_marker[b] = lca_marker_value;
    while (a >= 0 || b >= 0) {
      if (a >= 0) {
        a = get_parent(a);
        if (a >= 0) {
          if (lca_marker[a] == lca_marker_value) return a;
          lca_marker[a] = lca_marker_value;
        }
      }
      if (b >= 0) {
        b = get_parent(b);
        if (b >= 0) {
          if (lca_marker[b] == lca_marker_value) return b;
          lca_marker[b] = lca_marker_value;
        }
      }
    }
    return -1;
  }

  blossom_ind_t form_blossom(blossom_ind_t a, blossom_ind_t b, blossom_ind_t lca, pair<node_ind_t, node_ind_t> bridge_a_to_b) {
    // lca is the current base
    blossom_t builder;
    builder.invalid = false;
    builder.raw_base = ((lca < n) ? lca : subblossoms[lca - n].raw_base);
    builder.pending_base = ((lca < n) ? -1 : subblossoms[lca - n].pending_base); // for lazy update
    builder.pending_mate = ((lca < n) ? -1 : subblossoms[lca - n].pending_mate); // for lazy update
    builder.base_ind = 0; // let's use the first one for the current base
    builder.z = 0;
    builder.elts.push_back(lca);
    vector<blossom_ind_t> apath, bpath;
    for (blossom_ind_t p = a; p != lca; p = get_parent(p)) { apath.push_back(p); }
    for (blossom_ind_t p = b; p != lca; p = get_parent(p)) { bpath.push_back(p); }
    builder.elts.reserve(1 + apath.size() + bpath.size());
    builder.edges.reserve(1 + apath.size() + bpath.size());
    for (int i = static_cast<int>(apath.size()); i-- > 0;) {
      builder.elts.push_back(apath[i]);
      builder.edges.emplace_back(parent_edge[apath[i]]); // (parent, node) and parent is going to lca
    }
    builder.edges.emplace_back(bridge_a_to_b);
    for (int i = 0; i < bpath.size(); i++) {
      builder.elts.push_back(bpath[i]);
      const auto &rb = parent_edge[bpath[i]]; // (parent, node), need to flip.
      builder.edges.emplace_back(rb.second, rb.first);
    }
    builder.ielts.reserve(builder.elts.size());
    for (int i = 0; i < builder.elts.size(); i++) builder.ielts[builder.elts[i]] = i;
    // finished building info, update blossom markers
    blossom_ind_t new_blossom_index = n + static_cast<blossom_ind_t>(subblossoms.size());
    for (int i = 0; i < subblossoms.size(); i++) {
      if (subblossoms[i].invalid) {
        new_blossom_index = n + i;
        break;
      }
    }
    for (blossom_ind_t elt : builder.elts) {
      direct_container[elt] = new_blossom_index;
      mark_blossom(elt, new_blossom_index);
    }
    blossom[new_blossom_index] = new_blossom_index;
    direct_container[new_blossom_index] = -1;
    if (new_blossom_index - n < subblossoms.size()) {
      subblossoms[new_blossom_index - n] = move(builder);
    } else {
      subblossoms.emplace_back(move(builder));
    }
    return new_blossom_index;
  }

  void expand_blossom(blossom_ind_t a) {
    if (a < n) return;
    propagate_lazy(a, false);
    blossom_t& cur = subblossoms[a - n];
    MYASSERT(cur.z == 0);
    cur.invalid = true;
    for (auto elt : cur.elts) {
      direct_container[elt] = -1;
      mark_blossom(elt, elt);
    }
    cur.ielts.clear(); cur.ielts.rehash(0);
    cur.elts.clear(); cur.elts.shrink_to_fit();
    cur.edges.clear(); cur.edges.shrink_to_fit();
  }


  void augment_blossom(const blossom_ind_t b, const node_ind_t raw, const node_ind_t raw_mate) {
    blossom_ind_t end = raw;
    mate[raw] = raw_mate;
    while (b != end) {
      const blossom_ind_t container_ind = direct_container[end];
      MYASSERT(container_ind != -1);
      auto& container = subblossoms[container_ind - n];
      const int blossom_size = static_cast<int>(container.elts.size());
      MYASSERT(container.ielts.count(end) != 0);
      const int end_index = container.ielts[end];
      const int base_index = container.base_ind;
      if (end_index == base_index) {
        container.raw_base = raw;
        container.pending_base = -1;
        container.pending_mate = -1;
        end = container_ind;
        continue;
      }
      int right_distance = (end_index < base_index ? blossom_size : 0) + end_index - base_index;
      if (right_distance % 2 == 0) {
        // base_index <- end_index
        for (int p = end_index, nnp; p != base_index; p = nnp) {
          // base_index ... (nnp) ..eg.. (np) --- (p) ... end_index
          int np = (p == 0 ? blossom_size : p) - 1;
          nnp = (np == 0 ? blossom_size : np) - 1;
          blossom_ind_t np_blossom = container.elts[np];
          blossom_ind_t nnp_blossom = container.elts[nnp];
          auto eg = container.edges[nnp];
          augment_blossom_lazy(np_blossom, eg.second, eg.first);
          augment_blossom_lazy(nnp_blossom, eg.first, eg.second);
        }
      }
      else {
        // end_index -> base_index
        for (int p = end_index, nnp; p != base_index; p = nnp) {
          // end_index ... (p) --- (np) ..eg.. (nnp) ... base_index
          int np = ((p + 1 == blossom_size) ? 0 : (p + 1));
          nnp = ((np + 1 == blossom_size) ? 0 : (np + 1));
          blossom_ind_t np_blossom = container.elts[np];
          blossom_ind_t nnp_blossom = container.elts[nnp];
          auto eg = container.edges[np];
          augment_blossom_lazy(np_blossom, eg.first, eg.second);
          augment_blossom_lazy(nnp_blossom, eg.second, eg.first);
        }
      }
      // new base
      container.base_ind = end_index;
      container.raw_base = raw;
      container.pending_base = -1;
      container.pending_mate = -1;
      end = container_ind;
    }
  }
  void augment_blossom_lazy(const blossom_ind_t b, const node_ind_t raw, const node_ind_t raw_mate) {
    if (b < n) {
      mate[raw] = raw_mate;
      return;
    }
    auto& container = subblossoms[b - n];
    container.raw_base = raw;
    container.pending_base = raw;
    container.pending_mate = raw_mate;
  }
  void propagate_lazy(const blossom_ind_t b, bool deep) {
    if (b < n) return;
    auto& container = subblossoms[b - n];
    if (container.pending_base >= 0) {
      augment_blossom(b, container.pending_base, container.pending_mate);
    }
    if (deep) {
      for (auto elt : container.elts) {
        if (elt >= n) propagate_lazy(elt, deep);
      }
    }
  }
  void augment(blossom_ind_t cur, node_ind_t raw_end, node_ind_t raw_end_mate) {
    for (;;) {
      //  (cur' ) ..pedge.. ( par ) ---- ( cur raw_end)..
      augment_blossom_lazy(cur, raw_end, raw_end_mate);
      blossom_ind_t par = get_parent(cur);
      if (par == -1) break;
      auto pedge = parent_edge[par];
      augment_blossom_lazy(par, pedge.second, pedge.first);
      tie(cur, raw_end, raw_end_mate) = make_tuple(get_parent(par), pedge.first, pedge.second);
    }
  }

  vector<int> blossom_processed;
  int blossom_processed_marker;
  bool process_node(const blossom_ind_t blossomind) {
    if (blossom_processed[blossomind] == blossom_processed_marker) return false;
    blossom_processed[blossomind] = blossom_processed_marker;
    if (blossomind < n) {
      node_ind_t cur_raw = blossomind;
      for (const auto& edge : graph[cur_raw]) {
        if (pi(cur_raw, edge.next, edge.weight, edge.cached_z) != 0) continue;
        blossom_ind_t cur = blossom[cur_raw]; // may change every loop
        blossom_ind_t next = blossom[edge.next];
        if (cur == next) continue;
        if (label[next] == -1) {
          // expand 0 T-blossom
          while (next >= n && subblossoms[next - n].z == 0) {
            expand_blossom(next);
            next = blossom[edge.next];
          }
          // not free vertex because frees are added to q.
          label[next] = 1;
          parent[next] = cur;
          parent_edge[next] = make_pair(cur_raw, edge.next);
          forest_root[next] = forest_root[cur];
          node_ind_t next_base = get_base(next);
          node_ind_t next_base_mate = get_pending_mate(next);
          blossom_ind_t next_mate = blossom[next_base_mate];
          label[next_mate] = 0;
          MYASSERT(next_mate != next);
          parent[next_mate] = next;
          parent_edge[next_mate] = make_pair(next_base, next_base_mate);
          forest_root[next_mate] = forest_root[cur];
          q.push(next_mate);
          continue;
        }
        else if (label[next] == 1) { /* not interesting */ }
        else if (label[next] == 0) {
          blossom_ind_t lca = getlca(cur, next);
          if (lca == -1) {
            augment(cur, cur_raw, edge.next);
            augment(next, edge.next, cur_raw);
            return true;
          }
          else {
            blossom_ind_t newcur = form_blossom(cur, next, lca, make_pair(blossomind, edge.next));
            label[newcur] = 0; // always S blossom is formed
            parent[newcur] = parent[lca];
            parent_edge[newcur] = parent_edge[lca];
            forest_root[newcur] = forest_root[cur];
            q.push(newcur);
          }
        }
      }
    }
    else {
      for (auto sub : subblossoms[blossomind - n].elts) {
        if (process_node(sub)) return true;
      }
    }
    return false;
  }

  int phase() {
    label.assign(2 * n, -1);
    queue<blossom_ind_t>().swap(q);
    forest_root.resize(2 * n);
    parent.resize(2 * n); parent_edge.resize(2 * n);
    for (int i = 0; i < n; i++) {
      blossom_ind_t b = blossom[i];
      if (label[b] != -1) continue;
      if (get_pending_mate(b) != -1) continue;
      q.push(b);
      label[b] = 0;
      parent[b] = -1;
      parent_edge[b] = make_pair(-1, -1);
      forest_root[b] = b;
    }
    if (q.empty()) { // already perfect matching
      return -1;
    }
    blossom_processed_marker++;
    while (!q.empty()) {
      blossom_ind_t cur = q.front(); q.pop();
      if (process_node(cur)) {
        return 1;
      }
    }

    weight_t weightlimit = numeric_limits<weight_t>::max();
    weight_t delta_1 = weightlimit;
    weight_t delta_2 = weightlimit;
    weight_t delta_3 = weightlimit;
    weight_t delta_4 = weightlimit;

    for (int i = 0; i < n; i++) {
      if (label[blossom[i]] == 0) {
        delta_1 = min(delta_1, u[i]);
        for (const auto& edge : graph[i]) {
          node_ind_t j = edge.next;
          if (blossom[i] == blossom[j]) continue;
          int labelj = label[blossom[j]];
          if (labelj == -1) {
            delta_2 = min(delta_2, pi(i, j, edge.weight, edge.cached_z));
          }
          else if (labelj == 0) {
            delta_3 = min(delta_3, pi(i, j, edge.weight, edge.cached_z) / 2);
          }
        }
      }
      else if (label[blossom[i]] == 1) {
        if (blossom[i] >= n) {
          delta_4 = min(delta_4, subblossoms[blossom[i] - n].z / 2);
        }
      }
    }

    weight_t delta = min(min(delta_1, delta_2), min(delta_3, delta_4));
    // no more update.
    if (delta == delta_1 || delta == 0) {
      return -1;
    }
    for (node_ind_t i = 0; i < n; i++) {
      blossom_ind_t curb = blossom[i];
      int l = label[curb];
      if (l == 0) u[i] -= delta;
      else if (l == 1) u[i] += delta;
      for (auto& edge : graph[i]) {
        blossom_ind_t nextb = blossom[edge.next];
        if (nextb == curb) {
          if (label[curb] == 0) edge.cached_z += 2 * delta;
          else if (label[curb] == 1) edge.cached_z -= 2 * delta;
        }
      }
    }
    vector<blossom_ind_t> to_expand;
    for (blossom_ind_t i = 0; i < subblossoms.size(); i++) {
      if (blossom[n + i] != n + i) continue; // subblossom
      if (label[n + i] == 0) subblossoms[i].z += 2 * delta;
      else if (label[n + i] == 1) subblossoms[i].z -= 2 * delta;
      if (subblossoms[i].z == 0) to_expand.push_back(n + i);
    }
    for (blossom_ind_t bind : to_expand) {
      expand_blossom(bind);
    }
    return 0;
  }
};

