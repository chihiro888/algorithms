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

struct RankSplayTree {
  typedef int val_t;
  struct node_t {
    int l, r, p;
    int cnt;
    val_t val;
    node_t() :l(-1), r(-1), p(-1), cnt(1) {}
  };
  int root;
  vector<node_t> tree;

  RankSplayTree() : root(-1) {}
  void refresh(int v) {
    int cnt = 1;
    int l = tree[v].l, r = tree[v].r;
    if (l != -1) cnt += tree[l].cnt;
    if (r != -1) cnt += tree[r].cnt;
    tree[v].cnt = cnt;
  }
  void reserve(size_t n) { tree.reserve(n); }
  void rotateUp(int v) {
    int p = tree[v].p;
    bool left = (tree[p].l == v);
    int s = left ? tree[v].r : tree[v].l;
    int g = tree[p].p;
    tree[v].p = g;
    tree[p].p = v;
    if (left) {
      tree[v].r = p;
      tree[p].l = s;
    } else {
      tree[v].l = p;
      tree[p].r = s;
    }
    if (g == -1) root = v;
    else if (tree[g].l == p) tree[g].l = v;
    else tree[g].r = v;
    if (s != -1) tree[s].p = p;
  }
  void splay(int x) {
    while (tree[x].p != -1) {
      int p = tree[x].p;
      int g = tree[p].p;
      if (g == -1) {
        rotateUp(x);
        refresh(p);
      } else if ((tree[g].l == p) == (tree[p].l == x)) {
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
  int getKth(int k, bool do_splay = true) { // 0-indexed, returns index of the node in `tree`
    if (root == -1 || k >= tree[root].cnt) return -1;
    int x = root;
    for (;;) {
      int l = tree[x].l;
      int r = tree[x].r;
      if (l != -1) {
        if (tree[l].cnt > k) {
          x = l;
          continue;
        }
        k -= tree[l].cnt;
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
  void addKth(int k, val_t val) {
    int total_cnt = (root == -1) ? 0 : tree[root].cnt;
    int newid = static_cast<int>(tree.size());
    tree.emplace_back();
    tree[newid].val = val;
    int rmost;
    if (total_cnt == 0) {
      root = newid;
      splay(root);
      return;
    } else if (k == total_cnt) {
      rmost = root;
    } else {
      int orig = getKth(k, false);
      if (tree[orig].l == -1) {
        tree[orig].l = newid;
        tree[newid].p = orig;
        splay(newid);
        return;
      }
      rmost = tree[orig].l;
    }
    while (tree[rmost].r != -1) rmost = tree[rmost].r;
    tree[rmost].r = newid;
    tree[newid].p = rmost;
    splay(newid);
    return;
  }
  void removeId(const int id) {
    const int par = tree[id].p;
    const int left = tree[id].l;
    const int right = tree[id].r;
    if (left == -1) {
      if (right != -1) tree[right].p = par;
      if (par == -1) root = right;
      else if (tree[par].l == id) tree[par].l = right;
      else tree[par].r = right;
      if (par != -1) splay(par);
      return;
    } else if (right == -1) {
      tree[left].p = par;
      if (par == -1) root = left;
      else if (tree[par].l == id) tree[par].l = left;
      else tree[par].r = left;
      if (par != -1) splay(par);
      return;
    }
    int replace = left;
    while (tree[replace].r != -1) replace = tree[replace].r;
    swap(tree[id].val, tree[replace].val);
    removeId(replace);
  }
};

