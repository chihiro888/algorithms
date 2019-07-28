#include <array>
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

uint32_t ptri[11][11];

void init_ptri() {
  for (int i = 0; i <= 10; i++) {
    ptri[i][0] = ptri[i][i] = 1;
    for (int j = 1; j < i; j++) {
      ptri[i][j] = ptri[i - 1][j - 1] + ptri[i - 1][j];
    }
  }
}

struct RankTreapRangeQuery {
  struct val_t {
    int cnt;
    // TODO: change the rest of the struct.
    uint32_t value;
    array<uint32_t, 11> poly; // [p] -> sum_i {i^p * elt[i]} (i=1..)
    val_t() : cnt(0), value(0), poly({0,0,0,0,0,0,0,0,0,0,0}) {}
    void shift_add(uint32_t off, uint32_t* dest) const {
      uint32_t poff[11];
      poff[0] = 1;
      for (int i = 1; i <= 10; i++) poff[i] = poff[i - 1] * off;
      for (int k = 0; k <= 10; k++) {
        for (int i = 0; i <= k; i++) {
          dest[k] += ptri[k][i] * poly[i] * poff[k - i];
        }
      }
    }
  };

  unsigned long long lastval;
  unsigned int myrand() {
    lastval *= 25214903917;
    lastval += 11;
    return (unsigned int)(lastval >> 13);
  }

  struct node {
    unsigned int heap_val;

    val_t val;
    int par, left, right;

    node() : par(-1), left(-1), right(-1) {}
  };

  int rootid;
  vector<node> nodes;

  void refresh(int ind) {
    // TODO: change the whole function
    val_t& val = nodes[ind].val;
    val.cnt = 1;
    val.poly.fill(val.value);
    if (nodes[ind].left != -1) {
      const val_t& left = nodes[nodes[ind].left].val;
      val.poly = left.poly;
      uint32_t pcnt = 1;
      for (int i = 0; i <= 10; i++) {
        val.poly[i] += val.value * pcnt;
        pcnt *= left.cnt + 1;
      }
      val.cnt += left.cnt;
    }
    if (nodes[ind].right != -1) {
      const val_t& right = nodes[nodes[ind].right].val;
      right.shift_add(val.cnt, &val.poly[0]);
      val.cnt += right.cnt;
    }
  }

  RankTreapRangeQuery() : rootid(-1) {}

  val_t getRangeRec(int id, int l, int r, int offset) {
    if (id == -1) return val_t();
    int tl = offset, tr = offset + nodes[id].val.cnt - 1;
    int tm = (nodes[id].left == -1) ? tl : (tl + nodes[nodes[id].left].val.cnt);
    if (l <= tl && tr <= r) return nodes[id].val;
    if (r < tl || tr < l) return val_t();
    if (r < tm) return getRangeRec(nodes[id].left, l, r, tl);
    if (l > tm) return getRangeRec(nodes[id].right, l, r, tm + 1);
    val_t first = getRangeRec(nodes[id].left, l, r, tl);
    val_t second = getRangeRec(nodes[id].right, l, r, tm + 1);
    // TODO: change the rest of the function
    {
      uint32_t pcnt = 1;
      for (int i = 0; i <= 10; i++) {
        first.poly[i] += nodes[id].val.value * pcnt;
        pcnt *= first.cnt + 1;
      }
      first.cnt++;
    }
    if (second.cnt > 0) {
      second.shift_add(first.cnt, &first.poly[0]);
      first.cnt += second.cnt;
    }
    return first;
  }
  array<uint32_t, 11> getRange(int l, int r) { // inclusive range
    // TODO: change the return type
    val_t result;
    result.cnt = 0;
    if (rootid == -1)
      return result.poly;
    int id = rootid;
    return getRangeRec(id, l, r, 0).poly;
  }

  int getKth(int k) { // 0 indexed
    if (rootid == -1) return -1;
    int id = rootid;
    for (;;) {
      int lid = nodes[id].left;
      int rid = nodes[id].right;
      if (lid != -1) {
        if (nodes[lid].val.cnt > k) {
          id = lid;
          continue;
        }
        k -= nodes[lid].val.cnt;
      }
      if (k == 0) return id;
      k--;
      if (rid != -1) {
        if (nodes[rid].val.cnt > k) {
          id = rid;
          continue;
        }
        k -= nodes[rid].val.cnt;
      }
      return -1;
    }
  }
  void addKth(int k, uint32_t val) { // k in [0, total_cnt]
    int total_cnt = 0;
    if (rootid != -1) total_cnt = nodes[rootid].val.cnt;
    int curid = static_cast<int>(nodes.size());
    nodes.emplace_back();
    nodes[curid].heap_val = myrand();
    nodes[curid].val.cnt = 1;
    // TODO: change from here
    nodes[curid].val.value = val;
    nodes[curid].val.poly.fill(val);
    // TODO: change until here
    if (total_cnt == 0) {
      rootid = curid;
      nodes[curid].par = -1;
    } else if (k == total_cnt) {
      int subid = rootid;
      while (nodes[subid].right != -1) subid = nodes[subid].right;
      nodes[subid].right = curid;
      nodes[curid].par = subid;
    }
    else {
      int origid = getKth(k);
      if (nodes[origid].left == -1) {
        nodes[origid].left = curid;
        nodes[curid].par = origid;
      }
      else {
        int subid = nodes[origid].left;
        while (nodes[subid].right != -1) subid = nodes[subid].right;
        nodes[subid].right = curid;
        nodes[curid].par = subid;
      }
    }
    for (int p = curid; p != -1; p = nodes[p].par) {
      refresh(p);
    }
    adjustUp(curid);
  }
  void modifyKth(int k, uint32_t val) { // k in [0, total_cnt)
    int id = getKth(k);
    // TODO: change from here
    nodes[id].val.value = val;
    // TODO: change until here
    for (int p = id; p != -1; p = nodes[p].par) {
      refresh(p);
    }
  }
  void removeId(const int id) {
    const int par = nodes[id].par;
    const int left = nodes[id].left;
    const int right = nodes[id].right;
    if (left == -1) {
      if (right != -1) nodes[right].par = par;
      if (par == -1) rootid = right;
      else if (nodes[par].left == id) nodes[par].left = right;
      else nodes[par].right = right;
      if (par != -1) refresh(par);
      for (int p = par; p != -1; p = nodes[p].par) refresh(p);
      return;
    } else if (right == -1) {
      nodes[left].par = par;
      if (par == -1) rootid = left;
      else if (nodes[par].left == id) nodes[par].left = left;
      else nodes[par].right = left;
      for (int p = par; p != -1; p = nodes[p].par) refresh(p);
      return;
    }
    int replace = left;
    while (nodes[replace].right != -1) replace = nodes[replace].right;
    swap(nodes[id].heap_val, nodes[replace].heap_val);
    swap(nodes[id].val, nodes[replace].val);
    removeId(replace);
    adjustDown(id);
  }
  void removeKth(const int k) { // k in [0, total_cnt)
    const int id = getKth(k);
    removeId(id);
  }
  void rotateLeft(int pos) {
    int par = nodes[pos].par;
    int right = nodes[pos].right;
    if (right == -1) return;
    int sub = nodes[right].left;
    nodes[pos].par = right;
    nodes[pos].right = sub;
    nodes[right].left = pos;
    nodes[right].par = par;
    if (par == -1) rootid = right;
    else if (nodes[par].right == pos) nodes[par].right = right;
    else nodes[par].left = right;
    if (sub != -1) nodes[sub].par = pos;
  }
  void rotateRight(int pos) {
    int par = nodes[pos].par;
    int left = nodes[pos].left;
    if (left == -1) return;
    int sub = nodes[left].right;
    nodes[pos].par = left;
    nodes[pos].left = sub;
    nodes[left].right = pos;
    nodes[left].par = par;
    if (par == -1) rootid = left;
    else if (nodes[par].right == pos) nodes[par].right = left;
    else nodes[par].left = left;
    if (sub != -1) nodes[sub].par = pos;
  }
  void adjustDown(int pos) {
    for (;;) {
      int left = nodes[pos].left;
      int right = nodes[pos].right;
      if ((left == -1 || nodes[left].heap_val <= nodes[pos].heap_val) &&
        (right == -1 || nodes[right].heap_val <= nodes[pos].heap_val)) {
        return;
      }
      if (left == -1) {
        rotateLeft(pos);
      } else if (right == -1) {
        rotateRight(pos);
      } else if (nodes[left].heap_val <= nodes[right].heap_val) {
        rotateLeft(pos);
      } else {
        rotateRight(pos);
      }
      refresh(pos);
      refresh(nodes[pos].par);
    }
  }
  void adjustUp(int pos) {
    for (;;) {
      int par = nodes[pos].par;
      if (par == -1)
        return;
      if (nodes[par].heap_val >= nodes[pos].heap_val)
        return;
      if (nodes[par].left == pos) rotateRight(par);
      else rotateLeft(par);
      refresh(par);
      refresh(pos);
    }
  }
};

// Tested with https://www.acmicpc.net/problem/13543
int main() {
  init_ptri();
  int n;
  RankTreapRangeQuery tr;
  scanf("%d", &n);
  tr.nodes.reserve(200001);
  for (int i = 0; i < n; i++) {
    unsigned int a;
    scanf("%u", &a);
    tr.addKth(i, a);
  }
  int Q;
  scanf("%d", &Q);
  while (Q-->0) {
    int cmd;
    scanf("%d", &cmd);
    if (cmd == 1) {
      int p; unsigned int v;
      scanf("%d%u", &p, &v);
      tr.addKth(p, v);
    } else if (cmd == 2) {
      int p;
      scanf("%d", &p);
      tr.removeKth(p);
    } else if (cmd == 3) {
      int p; unsigned int v;
      scanf("%d%u", &p, &v);
      tr.modifyKth(p, v);
    } else {
      int l, r, k;
      scanf("%d%d%d", &l, &r, &k);
      uint32_t result = 0;
      if (l <= r) {
        result = tr.getRange(l, r)[k];
      }
      printf("%u\n", (unsigned int)result);
    }
  }
  return 0;
}

