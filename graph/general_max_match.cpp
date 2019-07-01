// based on Tarjan, "Data Structures and Network Algorithms"
// time complexity: O(M |E| alpha(|V|,|E|)) where M = |maximum matching| <= |V|/2

// tested
// http://acm.timus.ru/problem.aspx?space=1&num=1099
// https://www.acmicpc.net/problem/15737
// https://www.acmicpc.net/problem/13447

#include <cstdio>
#include <tuple>
#include <queue>
#include <set>
#include <vector>
#include <algorithm>

using namespace std;

struct DisjointSet {
	vector<int> parent, cnt;

	DisjointSet() { }

	DisjointSet(int n) : cnt(n, 1) {
		parent.reserve(n);
		for(int i = 0; i < n; i++) parent.push_back(i);
	}
	
	void reset(int sz) {
		parent.clear(); cnt.clear();
		for(int i = 0; i < sz; i++) {
			parent.push_back(i);
			cnt.push_back(1);
		}
	}

	void increase(int sz) {
		int base = parent.size();
		for(int i = base; i < base+sz; i++) {
			parent.push_back(i);
			cnt.push_back(1);
		}
	}

	int find(int p) {
		int np = p;
		while(p != parent[p]) p = parent[p];
		while(np != parent[np]) {
			int tmp = parent[np];
			parent[np] = p;
			np = tmp;
		}
		return p;
	}

	void merge(int a, int b) {
		a = find(a), b = find(b);
		if (a == b)
			return;
		if (cnt[a] > cnt[b]) {
			cnt[a] += cnt[b];
			parent[b] = a;
		} else {
			cnt[b] += cnt[a];
			parent[a] = b;
		}
	}
};

// sample usage
// MaxMatching matcher(n);
// for (auto edge : edges) matcher.AddEdge(edge.start, edge.end);
// int max_match = matcher.Match();
// for (int i = 0; i < n; i++) {
//	 if (matched[i] > i) reportMatch(i, matched[i]);
// }
struct MaxMatching {
	int n;
	vector<vector<int>> gnext;
	vector<int> matched;

	int vcnt;

	MaxMatching(int n) :
		n(n),
		gnext(n),
		matched(n, -1)
	{
	}

	void AddEdge(int a, int b) {
		gnext[a].push_back(b);
		gnext[b].push_back(a);
	}

	// to find actual match, use matched array. if (u,v) is a match, matched[u] = v and matched[v] = u
	// Returns the number of matched edges.
	int Match() {
		int ans = 0;
		for (int i = 0; i < n; i++) {
			if (matched[i] != -1) continue;
			for (auto next : gnext[i]) {
				if (matched[next] != -1) continue;
				ans++;
				matched[i] = next;
				matched[next] = i;
				break;
			}
		}
		while (findAugment()) {
			ans++;
		}
		return ans;
	}

	// even: 자유정점으로부터 거리가 짝수 (원래 매칭의 끝)
	// odd: 자유정점으로부터 거리가 홀수 (원래 매칭의 시작)
	// base: 블라섬 내부에서 바깥과 매칭된 내부 정점
	// stem: 자유정점으로부터 base 까지의 경로
	// predecessor: augment path에서 매칭된 점 말고 반대쪽으로 있는 점
	// mate: 매칭된 점
	// origin: blossom이면 base가 되는 정점을 계속 추적했을 때 나오는 정점
	//
	// augmenting 한 번마다 새로 forest를 키운다.
	// 초기에 각 트리의 루트는 자유 정점이다.
	// forest의 even에서 연결된 간선을 확인하면서 붙여나간다. augment path를 키워나가는 것이다.
	//
	// even인 v에서 확인했을 때, 새 정점 w의 경우수
	// 1) forest에 있었던 odd 정점 -> ignore
	// 2) 처음 보는데, 매칭되어 있다 -> 매칭된 정점도 함께 추가
	// 3) forest에 있었던 even 정점, 다른 트리 -> 경로를 찾음
	// 4) forest에 있었던 even 정점, 같은 트리 -> blossom.
	//
	vector<int> parent; // 정점 -> 트리의 parent 관계, 즉 predecessor 및 mate
	vector<int> forest; // 정점 -> forest에 산입된 경우, 트리 번호. unreached이면 -1
	vector<int> level;  // 정점 -> 트리 상 깊이 (0부터 시작)
	vector<pair<int,int>> bridge; // odd 정점 -> 트리 내에서 blossom이 만들어진다면 그 원인 간선. 방향은 반대쪽
	queue<int> q;
	DisjointSet blossomSet;
	vector<int> origin; // 정점 -> 재귀적으로 blossom이라면 base의 원본 vertex
	vector<int> ancestorChecker;
	int ancestorCheckerValue;


	vector<int> marker;
	void markBlossomPath(int vv, pair<int,int> vu, int ancestor){
		int p = vv;
		marker.clear();
		while(p != ancestor) {
			int np;
			np = origin[blossomSet.find(parent[p])];
			marker.push_back(p);
			p = np;

			np = origin[blossomSet.find(parent[p])];
			marker.push_back(p);
			bridge[p] = vu; // need original vertex number
			q.push(p); // odd level edges were not considered
			p = np;
		}
		for(auto x : marker) blossomSet.merge(ancestor, x);
		origin[blossomSet.find(ancestor)] = ancestor;
	}

	void mergeBlossom(int vv, int uu, int v, int u){
		if (uu == vv) return;
		++ancestorCheckerValue;
		int p1 = uu, p2 = vv;
		int ancestor = -1;
		for(;;) {
			if (p1 >= 0) {
				if (ancestorChecker[p1] == ancestorCheckerValue) {
					ancestor = p1;
					break;
				}
				ancestorChecker[p1] = ancestorCheckerValue;
				if (parent[p1] >= 0) p1 = origin[blossomSet.find(parent[p1])]; else p1 = -1;
			}
			if (p2 >= 0) {
				if (ancestorChecker[p2] == ancestorCheckerValue) {
					ancestor = p2;
					break;
				}
				ancestorChecker[p2] = ancestorCheckerValue;
				if (parent[p2] >= 0) p2 = origin[blossomSet.find(parent[p2])]; else p2 = -1;
			}
		}
		markBlossomPath(uu, make_pair(u, v), ancestor);
		markBlossomPath(vv, make_pair(v, u), ancestor);
	}

	vector<int> augmentPathLink;

	void getRootPath(int v, int w, bool reversed){
		if (v == w) return;
		if (level[v] & 1) {
			// odd. use bridge
			int x, y, mate = matched[v];
			tie(x, y) = make_pair(bridge[v].first, bridge[v].second);
			getRootPath(x, mate, !reversed);
			getRootPath(y, w, reversed);
			if (reversed) {
				augmentPathLink[y] = x;
				augmentPathLink[mate] = v;
			} else {
				augmentPathLink[v] = mate;
				augmentPathLink[x] = y;
			}
		} else {
			// even
			int mate = matched[v];
			getRootPath(parent[mate], w, reversed);
			if (reversed) {
				augmentPathLink[parent[mate]] = mate;
				augmentPathLink[mate] = v;
			} else {
				augmentPathLink[v] = mate;
				augmentPathLink[mate] = parent[mate];
			}
		}
	}

	void augmentPath(int v, int w) {
		augmentPathLink = vector<int>(n,-1);
		int x = forest[v];
		int y = forest[w];
		getRootPath(v,x,true);
		getRootPath(w,y,false);
		augmentPathLink[v] = w;
		int p = x;
		for(;;) {
			int q = augmentPathLink[p];
			matched[p] = q;
			matched[q] = p;
			if (q == y) break;
			p = augmentPathLink[q];
		}
	}

	bool findAugment() {
		parent.assign(n,-1);
		forest.assign(n,-1);
		level.assign(n, 0);
		bridge.assign(n,make_pair(-1,-1));
		q = queue<int>();
		blossomSet.reset(n);
		origin.assign(n, 0);
		ancestorChecker.assign(n, 0);
		ancestorCheckerValue = 0;

		for(int i = 0; i < n; i++) {
			origin[i] = i;
			if (matched[i] == -1) {
				forest[i] = i;
				q.push(i);
			}
		}
		bool foundPath = false;
		while(!q.empty() && !foundPath) {
			int v = q.front(); q.pop();
			for(auto u : gnext[v]) {
				int vv = origin[blossomSet.find(v)];
				int uu = origin[blossomSet.find(u)];
				if (forest[uu] == -1) {
					// assert(u == uu)
					parent[uu] = v;
					forest[uu] = forest[vv];
					level [uu] = level [vv] + 1;
					parent[matched[uu]] = uu;
					forest[matched[uu]] = forest[vv];
					level [matched[uu]] = level [vv] + 2;
					q.push(matched[uu]);
				} else if (level[uu]&1) {
					// odd level
				} else if (forest[uu] != forest[vv]){
					// found path. both are even level
					foundPath = true;
					augmentPath(v,u);
					break;
				} else {
					// blossom formed
					mergeBlossom(vv, uu, v, u);
				}
			}
		}

		return foundPath;
	}
};

