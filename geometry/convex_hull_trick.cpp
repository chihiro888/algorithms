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

namespace offline_trick {

// tuple<slope, base, auxiliary>: f(x) = slope * x + base
template<typename TAux>
void calc_upper_convex(vector<tuple<long long, long long, TAux>> *lines) {
  if (lines->empty()) return;
  vector<tuple<long long, long long, TAux>> stck;
  sort(lines->begin(), lines->end());
  for (auto &sbi : *lines) {
    long long s = get<0>(sbi), b = get<1>(sbi);
    while (stck.size() >= 2) {
      if (get<0>(stck.back()) == s) {
        stck.pop_back();
        continue;
      }
      long long s0 = get<0>(stck[stck.size() - 2]), b0 = get<1>(stck[stck.size() - 2]);
      long long s1 = get<0>(stck[stck.size() - 1]), b1 = get<1>(stck[stck.size() - 1]);
      if ((b - b0) * (s1 - s0) >= (b1 - b0) * (s - s0)) {
        stck.pop_back();
        continue;
      }
      break;
    }
    stck.emplace_back(move(sbi));
  }
  lines->swap(stck);
}

template<typename TAux>
pair<long long, TAux> evaluate(tuple<long long, long long, TAux> line, long long x) {
  return make_pair(get<0>(line)*x + get<1>(line), move(get<2>(line)));
}
// tuple<slope, base, auxiliary>: f(x) = slope * x + base
template<typename TAux>
pair<long long, TAux> query_upper_convex(const vector<tuple<long long, long long, TAux>> &lines, long long x) {
  int l = 1, r = static_cast<int>(lines.size()) - 1;
  pair<long long, TAux> best = evaluate(lines[0], x);
  while (l <= r) {
    int m = (l + r) / 2;
    pair<long long, TAux> em = evaluate(lines[m], x);
    if (m == r) {
      best = max(best, em);
      r = m - 1;
      continue;
    }
    pair<long long, TAux> em2 = evaluate(lines[m + 1], x);
    if (em.first < em2.first) {
      best = max(best, em2);
      l = m + 1;
    } else {
      best = max(best, em);
      r = m - 1;
    }
  }
  return best;
}

}  // offline_trick

struct UpperHull {
    typedef long long coord_t;
    struct Line {
        // y = mx + b
        coord_t b;
        coord_t m;

        bool isEvaluation;
        mutable coord_t s, e; // [s, e)

        Line() : b(0), m(0), isEvaluation(false), s(numeric_limits<coord_t>::lowest()), e(numeric_limits<coord_t>::max()) { }

        bool operator <(const Line &other) const {
            if (isEvaluation) {
                coord_t x = b;
                if (other.s <= x && x < other.e) {
                    return false; // equals
                } else {
                    return x < other.e;
                }
            }
            if (other.isEvaluation) {
                coord_t x = other.b;
                if (s <= x && x < e) {
                    return false; // equals
                } else {
                    return s <= x;
                }
            }
            return m < other.m;
        }

        coord_t evaluate(coord_t xp) const {
            return b + m * xp;
        }

        // ceil division
        coord_t cross_point(const Line &other) const {
            // b1 + m1 * x == b2 + m2 * x
            // (b1 - b2) / (m2 - m1) = x
            coord_t up = b - other.b;
            coord_t down = other.m - m;
            if (down < 0) { up = -up; down = -down; }
            if (up < 0) return up / down;
            return (up + (down - 1)) / down;
        }
    };

    set<Line> hull;

    bool ok(const Line &a, const Line &b, const Line &c) const {
        coord_t x1 = a.cross_point(b);
        coord_t x2 = b.cross_point(c);
        return x1 < x2;
    }

    coord_t evaluate(coord_t x) const {
        Line evaluation;
        evaluation.b = x;
        evaluation.m = x;
        evaluation.isEvaluation = true;
        return hull.find(evaluation)->evaluate(x);
    }

    void addLine(coord_t x, coord_t y, coord_t incline) {
        Line cand;
        cand.b = y - x * incline;
        cand.m = incline;
        auto Next = hull.lower_bound(cand);
        // same incline, so higher b wins
        if (Next != hull.end() && Next->m == cand.m) {
            if (Next->b >= cand.b) {
                // higher one is there.
                return;
            } else {
                // lower one, remove first.
                Next = hull.erase(Next);
            }
        }
        if (Next != hull.begin() && Next != hull.end()) {
            auto Prev = Next; --Prev;
            if (ok(*Prev, cand, *Next)) {
                // can be added
            } else {
                return;
            }
        }
        for (;;) {
            auto Prev = Next;
            if (Prev == hull.begin()) break;
            --Prev;
            auto PPrev = Prev;
            if (PPrev == hull.begin()) break;
            --PPrev;
            if (ok(*PPrev, *Prev, cand)) {
                break;
            }
            hull.erase(Prev);
        }
        for (;;) {
            if (Next == hull.end()) break;
            auto NNext = Next; ++NNext;
            if (NNext == hull.end()) break;
            if (ok(cand, *Next, *NNext)) {
                break;
            }
            Next = hull.erase(Next);
        }
        insertAndAssign(cand);
    }

private:
    void insertAndAssign(const Line &l) {
        auto iter = hull.insert(l).first;
        auto N = iter; ++N;
        if (N != hull.end()) {
            coord_t x = iter->cross_point(*N);
            iter->e = x;
            N->s = x;
        }
        auto P = iter;
        if (P != hull.begin()) {
            --P;
            coord_t x = P->cross_point(*iter);
            iter->s = x;
            P->e = x;
        }
    }
};

int main() {
    UpperHull up;
    up.addLine(0, 15, -1);
    for (int x = -30; x <= 30; x++) { printf("%d,", (int)up.evaluate(x)); } printf("\n");
    up.addLine(0, 3, 0);
    for (int x = -30; x <= 30; x++) { printf("%d,", (int)up.evaluate(x)); } printf("\n");
    up.addLine(0, 4, 0);
    for (int x = -30; x <= 30; x++) { printf("%d,", (int)up.evaluate(x)); } printf("\n");
    up.addLine(0, 2, 0);
    for (int x = -30; x <= 30; x++) { printf("%d,", (int)up.evaluate(x)); } printf("\n");
    up.addLine(10, 15, 1);
    for (int x = -30; x <= 30; x++) { printf("%d,", (int)up.evaluate(x)); } printf("\n");
    up.addLine(15, 12, 2);
    for (int x = -30; x <= 30; x++) { printf("%d,", (int)up.evaluate(x)); } printf("\n");
    up.addLine(0, 12, 0);
    for (int x = -30; x <= 30; x++) { printf("%d,", (int)up.evaluate(x)); } printf("\n");
    up.addLine(0, 30, 5);
    for (int x = -30; x <= 30; x++) { printf("%d,", (int)up.evaluate(x)); } printf("\n");
    return 0;
}
