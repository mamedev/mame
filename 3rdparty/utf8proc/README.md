# utf8proc
[![CI](https://github.com/NanoComp/meep/actions/workflows/build-ci.yml/badge.svg)](https://github.com/JuliaStrings/utf8proc/actions/workflows/build-ci.yml)
[![AppVeyor status](https://ci.appveyor.com/api/projects/status/ivaa0v6ikxrmm5r6?svg=true)](https://ci.appveyor.com/project/StevenGJohnson/utf8proc)

[utf8proc](http://juliastrings.github.io/utf8proc/) is a small, clean C
library that provides Unicode normalization, case-folding, and other
operations for data in the [UTF-8
encoding](http://en.wikipedia.org/wiki/UTF-8).  It was [initially
developed](http://www.public-software-group.org/utf8proc) by Jan
Behrens and the rest of the [Public Software
Group](http://www.public-software-group.org/), who deserve *nearly all
of the credit* for this package.  With the blessing of the Public
Software Group, the [Julia developers](http://julialang.org/) have
taken over development of utf8proc, since the original developers have
moved to other projects.

utf8proc is used for basic Unicode
support in the [Julia language](http://julialang.org/), and the Julia
developers became involved because they wanted to add Unicode 7 support and other features; it is now regularly updated to keep up with recent Unicode releases.

There are also utf8proc wrappers for [Ruby](https://www.ruby-toolbox.com/projects/utf8_proc) and [Rust](https://docs.rs/utf8proc/latest/utf8proc/) ([github](https://github.com/Techcable/utf8proc.rs)).  (The original utf8proc package also included PostgreSQL bindings.)

The utf8proc package is licensed under the
free/open-source [MIT "expat"
license](http://opensource.org/licenses/MIT) (plus certain Unicode
data governed by the similarly permissive [Unicode data
license](http://www.unicode.org/copyright.html#Exhibit1)); please see
the included `LICENSE.md` file for more detailed information.

## Quick Start

Typical users should download a [utf8proc release](http://juliastrings.github.io/utf8proc/releases/) rather than cloning directly from github.

For compilation of the C library, run `make`.  You can also install the library and header file with `make install` (by default into `/usr/local/lib` and `/usr/local/bin`, but this can be changed by `make prefix=/some/dir`).  `make check` runs some tests, and `make clean` deletes all of the generated files.

Alternatively, you can compile with `cmake`, e.g. by
```sh
mkdir build
cmake -S . -B build
cmake --build build
```

### Using other compilers
The included `Makefile` supports GNU/Linux flavors and MacOS with `gcc`-like compilers; Windows users will typically use `cmake`.

For other Unix-like systems and other compilers, you may need to pass modified settings to `make` in order to use the correct compilation flags for building shared libraries on your system.

For HP-UX with HP's `aCC` compiler and GNU Make (installed as `gmake`), you can compile with
```
gmake CC=/opt/aCC/bin/aCC CFLAGS="+O2" PICFLAG="+z" C99FLAG="-Ae" WCFLAGS="+w" LDFLAG_SHARED="-b" SOFLAG="-Wl,+h"
```
To run `gmake install` you will need GNU coreutils for the `install` command, and you may want to pass `prefix=/opt libdir=/opt/lib/hpux32` or similar to change the installation location.

### Using with CMake

A CMake Config-file package is provided. To use utf8proc in a CMake project:

```cmake
add_executable (app app.c)
find_package (utf8proc 2.9.0 REQUIRED)
target_link_libraries (app PRIVATE utf8proc::utf8proc)
```

## General Information

The C library is found in this directory after successful compilation
and is named `libutf8proc.a` (for the static library) and
`libutf8proc.so` (for the dynamic library).

The Unicode version supported is 17.0.0.

For Unicode normalizations, the following options are used:

* Normalization Form C:  `STABLE`, `COMPOSE`
* Normalization Form D:  `STABLE`, `DECOMPOSE`
* Normalization Form KC: `STABLE`, `COMPOSE`, `COMPAT`
* Normalization Form KD: `STABLE`, `DECOMPOSE`, `COMPAT`

## C Library

The documentation for the C library is found in the `utf8proc.h` header file.
`utf8proc_map` is function you will most likely be using for mapping UTF-8
strings, unless you want to allocate memory yourself.

## To Do

See the Github [issues list](https://github.com/JuliaLang/utf8proc/issues).

## Contact

Bug reports, feature requests, and other queries can be filed at
the [utf8proc issues page on Github](https://github.com/JuliaLang/utf8proc/issues).

## See also

An independent Lua translation of this library, [lua-mojibake](https://github.com/differentprogramming/lua-mojibake), is also available.

## Examples

### Convert codepoint to string
```c
// Convert codepoint `a` to utf8 string `str`
utf8proc_int32_t a = 223;
utf8proc_uint8_t str[16] = { 0 };
utf8proc_encode_char(a, str);
printf("%s\n", str);
// ß
```

### Convert string to codepoint
```c
// Convert string `str` to pointer to codepoint `a`
utf8proc_uint8_t str[] = "ß";
utf8proc_int32_t a;
utf8proc_iterate(str, -1, &a);
printf("%d\n", a);
// 223
```

### Casefold

```c
// Convert "ß"  (U+00DF) to its casefold variant "ss"
utf8proc_uint8_t str[] = "ß";
utf8proc_uint8_t *fold_str;
utf8proc_map(str, 0, &fold_str, UTF8PROC_NULLTERM | UTF8PROC_CASEFOLD);
printf("%s\n", fold_str);
// ss
free(fold_str);
```

### Normalization Form C/D (NFC/NFD)
```c
// Decompose "\u00e4\u00f6\u00fc" = "äöü" into "a\u0308o\u0308u\u0308" (= "äöü" via combining char U+0308)
utf8proc_uint8_t input[] = {0xc3, 0xa4, 0xc3, 0xb6, 0xc3, 0xbc}; // "\u00e4\u00f6\u00fc" = "äöü" in UTF-8
utf8proc_uint8_t *nfd= utf8proc_NFD(input); // = {0x61, 0xcc, 0x88, 0x6f, 0xcc, 0x88, 0x75, 0xcc, 0x88}

// Compose "a\u0308o\u0308u\u0308" into "\u00e4\u00f6\u00fc" (= "äöü" via precomposed characters)
utf8proc_uint8_t *nfc= utf8proc_NFC(nfd);

free(nfd);
free(nfc);
```
