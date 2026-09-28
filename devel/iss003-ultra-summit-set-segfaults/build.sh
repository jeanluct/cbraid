#!/usr/bin/env bash
# Build the issue-#3 test programs against two versions of cbraid's lib/:
#
#   old: commit 6f06e68 (before the issue-#3 fix; what braidlab's
#        extern/cbraid currently contains)
#   new: the working tree (the fix, plus any edits being tested)
#
# Each program is built with GCC's checked standard library
# (-D_GLIBCXX_DEBUG, which aborts on dereferencing a past-the-end
# iterator) and AddressSanitizer/UBSan.
#
# Usage (from anywhere):  devel/iss003-ultra-summit-set-segfaults/build.sh
# Outputs: build/{harness,fuzz}-{old,new} in this folder.
set -euo pipefail

here="$(cd "$(dirname "$0")" && pwd)"
repo="$(git -C "${here}" rev-parse --show-toplevel)"
out="${here}/build"
mkdir -p "${out}/old-src"

# Old sources, extracted from git so the working tree is untouched.
git -C "${repo}" archive 6f06e68 lib include | tar -x -C "${out}/old-src"

flags=(-std=c++11 -g -O1 -D_GLIBCXX_DEBUG
       -fsanitize=address,undefined -fno-omit-frame-pointer)

for version in old new; do
  if [ "${version}" = old ]; then src="${out}/old-src"; else src="${repo}"; fi
  for prog in harness fuzz; do
    g++ "${flags[@]}" -I"${src}/include" "${here}/${prog}.cpp" \
        "${src}/lib/braiding.cpp" "${src}/lib/cbraid.cpp" \
        -o "${out}/${prog}-${version}"
    echo "built ${out}/${prog}-${version}"
  done
done
