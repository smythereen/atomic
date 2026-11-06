# atomic

Header-only x86 atomic primitives used across the unit tree. There is no
`libatomic.a`: installing the unit copies three headers into `~/cmn/`.

**Requires the [base](https://github.com/smythereen/base) repository** to be
installed first (project root with `BASE`, `rules.mak`, `targets.mak`, and the
`repo` unit). Clone base, then fetch this unit with `repo add atomic` or via
the project starter list.

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

- gcc with x86 inline asm (`lock` / `xaddl` / `cmpxchg*`).
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

## License

Most files: `Apache-2.0 OR GPL-2.0-or-later` (see `LICENSE`, `LICENSES/`).

`atomic_32.h` is **`GPL-2.0-only`** (Linux kernel–derived). Choosing Apache-2.0
for the rest of a project does not relicense that file.
