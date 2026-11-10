# atomic

Header-only x86 atomic primitives used across the unit tree. There is no
`libatomic.a`: installing the unit copies three headers into `~/cmn/`.

**Requires the [base](https://github.com/smythereen/base) repository** to be
installed first (project root with `BASE`, `rules.mak`, `targets.mak`, and the
`repo` unit). The base unit make system handles C, C++, and Rust, including
large mixed-language projects. Clone base, then fetch this unit with
`repo add atomic` or via the project starter list.

## What it provides

### `atomic_32.h` (`GPL-2.0-only`)

Derived from the Linux kernel x86 `atomic.h`, adapted for this tree.
Defines `atom_32` and non-blocking locked 32-bit ops: read/set, add/sub,
inc/dec, add-and-test / sub-and-test, and related helpers (`s32_*`).

### `cmpxchg.h`

8/16/32-bit compare-and-exchange via `CompareAndExchange(mem, old, new)`
(returns the previous value in memory).

### `atomic_bump.h`

- `Atomic_Bump` — locked `xaddl` +1, returns the new value.
- `Atomic_Bump_And_Mask` — bump then clamp with a mask.
- `Atomic_Bump_With_Max` — bump with a maximum bound.
- `Atomic_Exchange` — locked exchange of a `u32`.
- Includes `cmpxchg.h` so both headers can be used in either order.

All three headers `#include <common.h>` for the shared integer typedefs.

## Makefile / locations

```
atomic/makefile       # LIBRARIES empty; exports *.h to ~/cmn
atomic/UNIT.deps      # base 1   (no require lines)
```

```sh
cd ~/atomic && make header
# installs ~/cmn/atomic_32.h ~/cmn/atomic_bump.h ~/cmn/cmpxchg.h
```

## System prerequisites

- gcc (C) or g++ (compatible C++ header/bridge usage below) with x86 inline asm (`lock` / `xaddl` / `cmpxchg*`).
- [sys](https://github.com/smythereen/sys) headers (`common.h`) present in
  `~/cmn` before any translation unit includes these headers.

## Direct public-unit dependencies

| Unit | Kind | Evidence |
|------|------|----------|
| *(none for linking)* | — | Header-only; `LIBRARIES` is empty; a program using only these headers links with libc alone |
| [sys](https://github.com/smythereen/sys) (compile-time types) | `#include <common.h>` | Every atomic header includes `common.h`. `UNIT.deps` does **not** `require sys` (avoids a fetch cycle with `sys` → `require atomic`). Install both before compile. |

**Not a dependency:** [sem](https://github.com/smythereen/sem). Passing `-lsem` when
`libsem.a` is absent fails with `cannot find -lsem`; the same program links
and runs with no `-lsem`. Semaphores may *use* these headers; this unit does
not link them.

## Indirect dependencies

None for the library product (there is none). Consumers that also call into
`libsys.a` pick up sys’s own deps separately.

```
atomic (headers) ──include──► common.h (sys)
sys/sysprt.c     ──include──► atomic_32.h
```

## Usage example (from private `a_test.c`, verified)

```c
#include <atomic_bump.h>
#include <stdio.h>
#include <stdlib.h>

int main(int ac, char *av[]) {
	u32 i, l = 0xFFFFFFFFu;
	u32 max = ac > 1 ? strtoul(av[1], 0, 10) : 5;
	(void)max;
	for (i = 0xFFFFFFFFu; Atomic_Bump(&i) < 8; )
		printf("%u\n", Atomic_Bump(&l));
	return 0;
}
```

```sh
gcc -o a_test a_test.c -I$HOME/cmn
./a_test
```

No `-lsys` / `-lsem` required for this program.

## Version

Release tag: `v0.2.0` (documentation update; version number unchanged).

## Using this unit from C++

This unit can be used from **C and C++**. Keep the unit's `.c` implementation
files compiled as C with `gcc`/`CC`; compile your C++ client with `g++`/`CXX`
and use `g++` for the final mixed-language link. There is no need to rebuild
the C library as C++.

Base's shared rules recognize C++ product sources named `.cc` and compile
them with `CXX` and `CXXFLAGS` (for example `make CXX=g++`). `CPP` names
the preprocessor, not the C++ compiler. The commands below assume the
headers and libraries have already been installed into `$HOME/cmn` and
`$HOME/lib`. GNU C++17 on Linux x86-64 is the example target; GNU extensions
in the shared types/headers are intentional.

`atomic_32.h` can be included directly in C++: its operations are macros
and `static inline` functions, so there are no external library symbols to
mangle and no `-latomic` link flag for this unit. `common.h` provides its own
C-linkage guard. The same x86 instruction/alignment constraints apply in both
languages.

**Current header caveat:** `cmpxchg.h` and `atomic_bump.h` use `new` as a
parameter and assembly-operand name. That is a C++ keyword.
`atomic_bump.h` also uses the removed C++17 `register` storage specifier,
which Clang diagnoses. An `extern "C"` block does **not** make
that syntax valid C++. Use the directly compatible `atomic_32.h` operations
where suitable, or put calls to those other two headers behind a small
C-compiled bridge, as shown below. This uses the existing unit unchanged;
it is not a claim that those headers parse directly as C++.

Save as `use_atomic.cc`:

```cpp
#include <atomic_32.h>
int main() {
    atom_32 count = ATOMIC_INIT(0);
    s32_inc(&count);
    return S32_Read(&count) != 1;
}
```

```sh
g++ -std=gnu++17 -I"$HOME/cmn" use_atomic.cc -L"$HOME/lib"  -o use_atomic
./use_atomic
```

For `Atomic_Bump` / `CompareAndExchange`, create these project-local files.
Do not include the affected atomic headers in the C++ translation unit.

`atomic_bridge.h` (the **application's** C/C++ boundary):

```c
#ifndef APP_ATOMIC_BRIDGE_H
#define APP_ATOMIC_BRIDGE_H
#include <common.h>
#ifdef __cplusplus
extern "C" {
#endif
u32 app_bump(u32 *value);
u32 app_compare_exchange(u32 *value, u32 expected, u32 replacement);
#ifdef __cplusplus
}
#endif
#endif
```

`atomic_bridge.c` (compile as C, never with `g++`):

```c
#include "atomic_bridge.h"
#include <atomic_bump.h>
u32 app_bump(u32 *value) { return Atomic_Bump(value); }
u32 app_compare_exchange(u32 *value, u32 expected, u32 replacement) {
    return CompareAndExchange(value, expected, replacement);
}
```

`bridge_demo.cc`:

```cpp
#include "atomic_bridge.h"
int main() {
    u32 value = 0;
    if (app_bump(&value) != 1) return 1;
    return app_compare_exchange(&value, 1, 2) == 1 && value == 2 ? 0 : 1;
}
```

```sh
gcc -std=gnu11 -I"$HOME/cmn" -c atomic_bridge.c -o atomic_bridge.o
g++ -std=gnu++17 -I"$HOME/cmn" bridge_demo.cc atomic_bridge.o -o bridge_demo
./bridge_demo
```

The C++ example above was compiled, linked and run on Linux x86-64 using
Clang 22.1.8 via `zig c++ -std=gnu++17 -nostdlib++`, with GCC 14.2-built C
objects where needed. It uses no C++ standard-library runtime. The `g++`
command is the GNU-toolchain form; that driver was not available for this
smoke check. This is not an exhaustive API or concurrency test.

## License

Most files: `Apache-2.0 OR GPL-2.0-or-later` (see `LICENSE`, `LICENSES/`).

`atomic_32.h` is **`GPL-2.0-only`** (Linux kernel–derived). Choosing Apache-2.0
for the rest of a project does not relicense that file.
