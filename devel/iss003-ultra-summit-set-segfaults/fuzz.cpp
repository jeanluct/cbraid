// Random AreConjugate calls, as braidlab's conjtest makes them.  Pairs:
// (b, g b g^-1) must be conjugate; also b times Delta^k variants.
#include <cstdio>
#include <cstdlib>
#include <list>
#include <vector>
#include "braiding.h"
using namespace CBraid; using namespace Braiding;
typedef std::vector<int> W;
static ArtinBraid lcf(const W& w, int n) { std::list<sint16> l(w.begin(), w.end()); ArtinBraid B = WordToBraid(l, n); B.MakeLCF(); return B; }
static W inv(W w) { W r(w.rbegin(), w.rend()); for (auto& x : r) x = -x; return r; }
static W cat(W a, const W& b) { a.insert(a.end(), b.begin(), b.end()); return a; }
static W delta(int n) { W w; for (int k = n-1; k >= 1; --k) for (int i = 1; i <= k; ++i) w.push_back(i); return w; }
static W rnd(int n, int len) { W w; for (int i = 0; i < len; ++i) { int g = 1 + rand() % (n-1); w.push_back(rand()%2 ? g : -g); } return w; }
int main(int argc, char** argv) {
  int N = atoi(argv[1]); srand(atoi(argv[2])); int bad = 0;
  for (int t = 0; t < N; ++t) {
    int n = 3 + rand() % 4;
    W b = rnd(n, rand() % 8), g = rnd(n, 1 + rand() % 6), D = delta(n);
    int k = rand() % 4;                        // b * Delta^k: Delta-power-heavy
    for (int i = 0; i < k; ++i) b = cat(b, D);
    ArtinBraid C(n);
    bool c1 = AreConjugate(lcf(b,n), lcf(cat(cat(g,b),inv(g)),n), C);
    if (!c1) { ++bad; printf("WRONG: n=%d case %d not conjugate to its conjugate\n", n, t); }
  }
  printf("fuzz done: %d cases, %d wrong\n", N, bad); return bad != 0;
}
