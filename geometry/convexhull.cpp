#include <vector>
#include <algorithm>

using namespace std;

typedef pair<int, int> point;

long long ccw(point a, point b, point c) {
	b.first -= a.first; b.second -= a.second;
	c.first -= a.first; c.second -= a.second;
	return b.first * (long long)c.second - c.first * (long long)b.second;
}

vector<point> convexHull(vector<point> dat) {
	vector<point> upper, lower;
	sort(dat.begin(), dat.end());
	dat.erase(unique(dat.begin(), dat.end()), dat.end());
	if (dat.size() <= 2)
		return dat;
	for (int i = 0; i < dat.size(); i++) {
		while (upper.size() >= 2 && ccw(*++upper.rbegin(), *upper.rbegin(), dat[i]) >= 0) upper.pop_back();
		while (lower.size() >= 2 && ccw(*++lower.rbegin(), *lower.rbegin(), dat[i]) <= 0) lower.pop_back();
		upper.emplace_back(dat[i]);
		lower.emplace_back(dat[i]);
	}
	upper.insert(upper.end(), ++lower.rbegin(), --lower.rend());
	return upper;
}

int main() {
	int n;
	vector<point> dat;
	scanf("%d",&n);
	for(int i = 0; i < n; i++) {
		int x,y;
		scanf("%d%d",&x,&y);
		dat.emplace_back(x,y);
	}
	auto result = convexHull(dat);
	for(auto p : result) {
		printf("(%d,%d)\n", p.first, p.second);
	}
	return 0;
}
