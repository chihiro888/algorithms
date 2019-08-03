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

// Tested: BOJ 13546
constexpr int BLOCK_SIZE = 600;
struct SqrtBatchSolver {
  typedef int elt_t;
  typedef int answer_t;

  vector<elt_t> A;
  SqrtBatchSolver(vector<elt_t> A) : A(move(A)) { }

  // ========== CHANGE FROM HERE =========
  // TODO: define state of the accumulator
  struct acc_t {
    int first, last;
    int marker;
    acc_t() : first(-1), last(-1), marker(-1) {}
  };
  vector<acc_t> key_to_save, key_to_save2;
  int current_marker, marker2;
  int best_so_far;
  void init_accumulator() { // called once
    // TODO: replace this part
    key_to_save.assign(100'000 + 1, acc_t());
    key_to_save2 = key_to_save;
    marker2 = 0;
  }
  void reset_accumulator(int start_index) { // called O(N / BLOCK_SIZE) times
    // TODO: replace this part
    current_marker = start_index;
    best_so_far = 0;
  }
  void add_tail(int ind) { // called O(N^2 / BLOCK_SIZE) times
    // TODO: replace this part
    int key = A[ind];
    auto& save = key_to_save[key];
    if (save.marker != current_marker) save.first = ind;
    save.last = ind;
    save.marker = current_marker;
    best_so_far = max(best_so_far, save.last - save.first);
  }
  answer_t prepend_solve(int l, int r) { // [l, r], called O(Q) times and r-l < BLOCK_SIZE
    answer_t ans;
    // TODO: set initial ans
    ans = best_so_far;
    ++marker2;
    for (int i = l; i <= r; i++) {
      // TODO: update ans
      int key = A[i];
      const auto& save = key_to_save[key];
      if (save.marker == current_marker) {
        ans = max(ans, save.last - i);
      }
      auto& save2 = key_to_save2[key];
      if (save2.marker != marker2) {
        save2.first = i;
        save2.marker = marker2;
      }
      ans = max(ans, i - save2.first);
    }
    return ans;
  }
  // ========== CHANGE UNTIL HERE =========

  // input range is inclusive [l, r]
  vector<answer_t> Solve(const vector<pair<int, int>> &input_range) {
    int n = A.size();
    // TODO: put more stuff if you need more information
    vector<vector<tuple<int, int, int>>> query_blocks((n + BLOCK_SIZE - 1) / BLOCK_SIZE);
    for (int i = 0; i < input_range.size(); i++) {
      int l, r;
      tie(l, r) = input_range[i];
      query_blocks[l / BLOCK_SIZE].emplace_back(r, l, i);
    }
    vector<answer_t> ans(input_range.size());
    init_accumulator();
    for (int block = 0; block < query_blocks.size(); block++) {
      sort(query_blocks[block].begin(), query_blocks[block].end());
      int s = (block + 1) * BLOCK_SIZE;
      //           s    t ->
      // [ block ][......]->
      //     l          r
      int t = s;
      reset_accumulator(s);
      for (auto query : query_blocks[block]) {
        int r, l, ind;
        tie(r, l, ind) = query;
        if (r < s) {
          ans[ind] = prepend_solve(l, r);
        } else {
          for (; t <= r; t++) add_tail(t);
          ans[ind] = prepend_solve(l, s - 1);
        }
      }
    }
    return ans;
  }
};

int main() {
  int n, K;
  scanf("%d%d", &n, &K);
  vector<int> A(n);
  for (int i = 0; i < n; i++) scanf("%d", &A[i]);
  int q;
  scanf("%d", &q);
  vector<pair<int, int>> queries;
  while (q-- > 0) {
    int l, r;
    scanf("%d%d", &l, &r);
    l--, r--;
    queries.emplace_back(l, r);
  }
  SqrtBatchSolver solver(A);
  auto ans = solver.Solve(queries);
  for (auto a : ans) {
    printf("%d\n", a);
  }
  return 0;
}

