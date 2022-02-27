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
#include <iostream>

using namespace std;

template<typename TNode>
struct NodeAllocator {
  vector<TNode> nodes;

  TNode& operator [](int index) { return nodes[index]; }
  const TNode& operator [](int index) const { return nodes[index]; }
};

template<typename TKey>
struct HeapNode {
  TKey key;
  // Big child first, small child later.
  vector<int> children; // order == children.size()
};

// Persistent Min BinomialHeap.
// Usage example:
//  NodeAllocator<HeapNode<int>> alloc;
//  PersistentHeap<int> heap(&alloc);
//  heap = heap.insert(3);
//  heap = heap.insert(4);
//  PersistentHeap<int> heap2(&alloc);
//  heap2 = heap2.insert(5);
//  heap2 = heap2.insert(2);
//  heap2 = heap2.insert(7);
//  heap2 = heap2.insert(17);
//  PersistentHeap<int> heap3 = heap.mergeHeap(heap2);
//  if (!heap3.empty())
//    heap3 = heap3.deleteMin();
template<typename TKey>
struct PersistentHeap {
  typedef PersistentHeap<TKey> Heap;
  typedef HeapNode<TKey> Node;

  PersistentHeap(NodeAllocator<HeapNode<TKey>> *alloc) : alloc(alloc) { }

  // returns node id of the merged tree. Takes node id.
  int internalMergeTree(int ai, int bi) {
    const Node &a = (*alloc)[ai], &b = (*alloc)[bi];
    // assumption: a.order == b.order
    Node new_node;
    if (a.key <= b.key) {
      new_node.key = a.key;
      new_node.children.reserve(a.children.size() + 1);
      new_node.children.push_back(bi);
      new_node.children.insert(new_node.children.end(), a.children.begin(), a.children.end());
    } else {
      new_node.key = b.key;
      new_node.children.reserve(b.children.size() + 1);
      new_node.children.push_back(ai);
      new_node.children.insert(new_node.children.end(), b.children.begin(), b.children.end());
    }
    alloc->nodes.emplace_back(move(new_node));
    return alloc->nodes.size() - 1;
  }

  Heap mergeHeap(const Heap& other) {
    // assumption: other.alloc == alloc
    Heap result(alloc);
    auto& A = *alloc;
    int i = 0, j = 0;
    int pending_tree = -1;
    vector<int> cmps;
    cmps.reserve(3);
    while (i < roots.size() || j < other.roots.size() || pending_tree != -1) {
      size_t min_order = numeric_limits<size_t>::max();
      cmps.clear();
      int ri = i < roots.size() ? roots[i] : -1, rj = j < other.roots.size() ? other.roots[j] : -1;
      if (ri != -1) min_order = min(min_order, A[ri].children.size());
      if (rj != -1) min_order = min(min_order, A[rj].children.size());
      if (pending_tree != -1) min_order = min(min_order, A[pending_tree].children.size());
      if (ri != -1 && min_order == A[ri].children.size()) cmps.push_back(ri);
      if (rj != -1 && min_order == A[rj].children.size()) cmps.push_back(rj);
      if (pending_tree != -1 && min_order == A[pending_tree].children.size()) cmps.push_back(pending_tree);
      if (cmps.size() == 1) {
        int r = cmps[0];
        result.roots.push_back(r);
        if (r == ri) i++;
        if (r == rj) j++;
        if (r == pending_tree) pending_tree = -1;
        continue;
      }
      if (cmps.size() == 3) {
        int r = cmps[2];
        result.roots.push_back(r);
        if (r == ri) i++;
        if (r == rj) j++;
        if (r == pending_tree) pending_tree = -1;
      }
      if (cmps[0] == ri || cmps[1] == ri) i++;
      if (cmps[0] == rj || cmps[1] == rj) j++;
      if (cmps[0] == pending_tree || cmps[1] == pending_tree) pending_tree = -1;
      pending_tree = internalMergeTree(cmps[0], cmps[1]);
    }
    return result;
  }

  Heap insert(TKey key) {
    Heap tmp(alloc);
    alloc->nodes.push_back({move(key), {}});
    tmp.roots.push_back(alloc->nodes.size() - 1);
    return mergeHeap(tmp);
  }

  // returns the index of roots that contains the min key.
  int internalMinSibling() const {
    if (roots.empty()) return -1;
    int ind = 0;
    for (int i = 1; i < roots.size(); i++) {
      const TKey& minkey = (*alloc)[roots[ind]].key;
      const TKey& curkey = (*alloc)[roots[i]].key;
      if (minkey > curkey) {
        ind = i;
      }
    }
    return ind;
  }

  bool empty() const {
    return roots.empty();
  }

  TKey findMin() const {
    int ind = internalMinSibling();
    if (ind == -1) return TKey{};
    return (*alloc)[roots[ind]].key;
  }

  Heap deleteMin() {
    int ind = internalMinSibling();
    Heap rest(alloc);
    rest.roots = roots;
    rest.roots.erase(rest.roots.begin() + ind);
    Heap sub(alloc);
    sub.roots = (*alloc)[roots[ind]].children;
    reverse(sub.roots.begin(), sub.roots.end());
    return rest.mergeHeap(sub);
  }

  // smaller order to greater order
  vector<int> roots;
  NodeAllocator<HeapNode<TKey>> *alloc;
};


