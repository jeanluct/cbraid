// Runs braidlab's conjtest path (WordToBraid, MakeLCF, AreConjugate) on
// known cases, plus the cbraid issue #3 centralizer reproducer as a
// control.  Build with -D_GLIBCXX_DEBUG -fsanitize=address,undefined.
#include <cstdio>
#include <cstring>
#include <list>
#include <vector>
#include "braiding.h"
using namespace CBraid;
using namespace Braiding;
typedef std::vector<int> W;
static ArtinBraid lcf(const W& w, int n) {
  std::list<sint16> l(w.begin(), w.end());
  ArtinBraid B = WordToBraid(l, n); B.MakeLCF(); return B;
}
static W inv(W w) { W r(w.rbegin(), w.rend()); for (auto& x : r) x = -x; return r; }
static W cat(W a, const W& b) { a.insert(a.end(), b.begin(), b.end()); return a; }
static W halftwist(int n) { W w; for (int k = n-1; k >= 1; --k) for (int i = 1; i <= k; ++i) w.push_back(i); return w; }
int main(int argc, char** argv) {
  int bad = 0;
  auto run = [&](const char* name, const W& a, const W& b, int n, bool expect) {
    ArtinBraid C(n);
    bool got = AreConjugate(lcf(a,n), lcf(b,n), C);
    printf("%-34s got=%d expected=%d %s\n", name, got, expect, got==expect ? "OK" : "WRONG");
    if (got != expect) ++bad; fflush(stdout);
  };
  if (argc > 1 && !strcmp(argv[1], "centralizer")) {   // control: issue #3
    list<ArtinBraid> z = Centralizer(lcf({3,3,4,3,3,2,1,4,3,2}, 5));
    printf("centralizer reproducer: %zu generators\n", z.size()); return 0;
  }
  W w = {3,3,4,3,3,2,1,4,3,2}, g = {1,-2,3,2,-1,4};
  run("reproducer vs itself", w, w, 5, true);
  run("reproducer vs a conjugate", w, cat(cat(g,w),inv(g)), 5, true);
  // Hang regression: FirstFactorInLCF as merged in PR #9 returned Delta
  // for inf>0 braids with factors, out of step with Cycling(); this case
  // then never returns.  Smallest found (none on 3 or 4 strings).
  run("s2 s4 s3^-1 Delta^2 vs itself", cat(W{2,4,-3}, cat(halftwist(5),halftwist(5))),
      cat(W{2,4,-3}, cat(halftwist(5),halftwist(5))), 5, true);
  for (int n = 3; n <= 6; ++n) {
    W D = halftwist(n), D2 = cat(D,D), h; for (int i = 1; i < n; ++i) h.push_back(i);
    char s[64];
    snprintf(s, 64, "halftwist(%d) vs itself", n);    run(s, D, D, n, true);
    snprintf(s, 64, "fulltwist(%d) vs itself", n);    run(s, D2, D2, n, true);
    snprintf(s, 64, "halftwist(%d) vs conjugate", n); run(s, D, cat(cat(h,D),inv(h)), n, true);
    snprintf(s, 64, "fulltwist(%d) vs halftwist", n); run(s, D2, D, n, false);
  }
  printf("done, %d wrong\n", bad); return bad != 0;
}
