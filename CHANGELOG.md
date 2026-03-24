# Changelog for CBraid and Braiding

## Notes

After `r12` the code was moved from Google Code/SVN to GitHub/Git, so there are no SVN-style revision numbers after that point.

The entries below are a lightly edited copy of historical SVN logs.

---

## Revisions in Google Code / SVN period

### r11 (2014-07-04)

- Removed some unused variables in new code.

### r10 (2014-07-04)

- New code contributed by Maria Cumplido (student of Juan Gonzalez-Meneses).
- Juan's description: the old code computed some sets called Ultra Summit Sets, while the new one computes Sets of Sliding Circuits (which are in general the same as above, but not always).

### r9 (2013-12-13)

- Updated `CHANGELOG`.

### r8 (2013-12-13)

- Updated `CHANGELOG`.

### r7 (2013-09-24)

- The code would not compile on Mac OS X clang XCode 5 without adding `std::` namespace specifications to stdlib functions.

### r6 (2013-08-03)

- Remove unused variables `cur`, `current`.

### r5 (2011-10-18)

- Update `svn:ignore`.

### r4 (2011-10-07)

- Remove `-fPIC` compile flag.

### r3 (2011-09-19)

- Updated GPL (renamed `LICENSE`).
- Improve Makefile for programs.
- Moved remaining `.h` files to `include`.
- Updated `CHANGELOG` to reflect move to Google Code.
- Tiny `INSTALL` file.

### r2 (2011-09-19)

- Imported sources from jeanluc's private braid project (`braid:trunk/cbraid r447`).

---

## Revisions from Jean-Luc's private braid project

### r432 (2011-09-17)

- Move all executables (including `braiding_main.cpp`) to `programs` folder.
- Removed sample and braiding folders.
- Makefile also builds `braiding` program.

### r431 (2011-09-17)

- Removed unused vars and trailing spaces from braiding.

### r428 (2011-09-16)

- Merged braiding into `libcbraid`. Updated Makefile.
- The `cbraid` and `braiding` `.h` files are now in an `include` folder.
- Moved `braidlcf` to the braidlcs Multi_Use_Code, added Makefile that should be able to build MEX files on different systems.

### r424 (2011-09-14)

- Better comment for update; use flags needed for Matlab MEX-file compilation.

### r418 (2011-09-13)

- Braiding: wrap in `Braiding` namespace.
- Braiding: reformat.

### r417 (2011-09-13)

- Include `std` global namespaces in braiding, since they are no longer included in CBraid.
- For CBraid, do not include namespace `CBraid` in `cbraid.cpp`, but include it in the namespace itself.

### r416 (2011-09-13)

- Added program `braiding` by Juan Gonzalez-Meneses.

### r415 (2011-09-13)

- Updated for GCC 4.4.5:
  - No longer allowed to use `typedef A<T> A`. Use `typedef Factor<P> CanonicalFactor`.
  - Various other places where stricter template parsing made the compiler unhappy.
  - Changed `time.h` to `ctime`.

### r414 (2011-09-13)

- Changes to CBraid for gcc-3.4:
  - Redefined `sint16` and `uint16` to be `int` and `unsigned int` rather than `short` and `unsigned short`.
  - Added `typename` after typedef and in external class definition.
  - Added `std::` in many places.
  - Changed `optarg` to `OptArg` to avoid name clash.
  - Bugfix in `cbraid_interface`: function `Complement` used `~a` (which does not compile) rather than the inverse of `*this`.
- Note: these changes were not sufficient for GCC 4.4.5, but were kept as a separate historical commit.

### r413 (2011-09-13)

- Added source code CBraid version 2001/12/07 by Jae Choon Cha.
