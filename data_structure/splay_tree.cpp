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

struct SplayTree {
  typedef int val_t;
  struct node_t {
    int l, r, p;
    int cnt;
    int key;
    val_t val;
    node_t() :l(-1), r(-1), p(-1), cnt(1) {}
  };
  vector<node_t> nodes;
  vector<int> gc;
  int root;

  SplayTree() : root(-1) {}

  void refresh(int v) {
    int cnt = 1;
    int l = nodes[v].l, r = nodes[v].r;
    if (l != -1) cnt += nodes[l].cnt;
    if (r != -1) cnt += nodes[r].cnt;
    nodes[v].cnt = cnt;
  }
  void reserve(size_t n) { nodes.reserve(n); gc.reserve(n); }

  void rotateUp(int v) {
    int p = nodes[v].p;
    bool left = (nodes[p].l == v);
    int s = left ? nodes[v].r : nodes[v].l;
    int g = nodes[p].p;
    nodes[v].p = g;
    nodes[p].p = v;
    if (left) {
      nodes[v].r = p;
      nodes[p].l = s;
    } else {
      nodes[v].l = p;
      nodes[p].r = s;
    }
    if (g == -1) root = v;
    else if (nodes[g].l == p) nodes[g].l = v;
    else nodes[g].r = v;
    if (s != -1) nodes[s].p = p;
  }
  void splay(int x) {
    while (nodes[x].p != -1) {
      int p = nodes[x].p;
      int g = nodes[p].p;
      if (g == -1) {
        rotateUp(x);
        refresh(p);
      } else if ((nodes[g].l == p) == (nodes[p].l == x)) {
        rotateUp(p); rotateUp(x);
        refresh(g);
        refresh(p);
      } else {
        rotateUp(x); rotateUp(x);
        refresh(g); refresh(p);
      }
    }
    refresh(x);
  }
  int lower_bound(int key, bool do_splay = true) { // returns index of the node in `nodes` or -1 if end()
    int chosen = -1;
    int x = root;
    int last = -1;
    while (x != -1) {
      last = x;
      if (nodes[x].key >= key) {
        chosen = x;
        x = nodes[x].l;
      } else {
        x = nodes[x].r;
      }
    }
    if (do_splay && last != -1) splay(last);
    return chosen;
  }
  int addKey(int key, val_t val) {
    int total_cnt = (root == -1) ? 0 : nodes[root].cnt;
    int newid;
    if (gc.empty()) {
      newid = static_cast<int>(nodes.size());
      nodes.emplace_back();
    } else {
      newid = gc.back(); gc.pop_back();
      nodes[newid] = node_t();
    }
    nodes[newid].key = key;
    nodes[newid].val = val;
    int rmost;
    if (total_cnt == 0) {
      root = newid;
      splay(root);
      return newid;
    } else {
      int orig = lower_bound(key, false);
      if (orig == -1) {
        rmost = root;
      } else if (nodes[orig].l == -1) {
        nodes[orig].l = newid;
        nodes[newid].p = orig;
        splay(newid);
        return newid;
      } else {
        rmost = nodes[orig].l;
      }
    }
    while (nodes[rmost].r != -1) rmost = nodes[rmost].r;
    nodes[rmost].r = newid;
    nodes[newid].p = rmost;
    splay(newid);
    return newid;
  }
  int getRank(const int id) {
    splay(id);
    int rank = 0;
    int subl = nodes[id].l;
    if (subl != -1) rank = nodes[subl].cnt;
    return rank;
  }
  int getKth(int k, bool do_splay = true) { // 0-indexed, returns index of the node in `nodes`
    if (k < 0 || root == -1 || k >= nodes[root].cnt) return -1;
    int x = root;
    for (;;) {
      int l = nodes[x].l;
      int r = nodes[x].r;
      if (l != -1) {
        if (nodes[l].cnt > k) {
          x = l;
          continue;
        }
        k -= nodes[l].cnt;
      }
      if (k == 0) {
        if (do_splay) splay(x);
        return x;
      }
      k--;
      x = r;
    }
    return -1;
  }
  void removeId(const int id) {
    const int par = nodes[id].p;
    const int left = nodes[id].l;
    const int right = nodes[id].r;
    if (left == -1) {
      if (right != -1) nodes[right].p = par;
      if (par == -1) root = right;
      else if (nodes[par].l == id) nodes[par].l = right;
      else nodes[par].r = right;
      if (par != -1) splay(par);
      gc.push_back(id);
      return;
    } else if (right == -1) {
      nodes[left].p = par;
      if (par == -1) root = left;
      else if (nodes[par].l == id) nodes[par].l = left;
      else nodes[par].r = left;
      if (par != -1) splay(par);
      gc.push_back(id);
      return;
    }
    int replace = left;
    while (nodes[replace].r != -1) replace = nodes[replace].r;
    std::swap(nodes[id].val, nodes[replace].val);
    removeId(replace);
  }
};

