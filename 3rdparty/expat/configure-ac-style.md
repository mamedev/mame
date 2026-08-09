# Style guidelines for `configure.ac`

Version `2019.08.09.21.54`


## Purpose

Define a small set of rules for style used in Expat's `configure.ac`
so that we have a common ground and something documented to refer to
in pull requests, when style is off.


## 1. Quoting
Quote "everything":
```
AC_DEFINE([HAVE_FOO], [1], [Define to 1 if you have the `foo' function.])
```

## 2. Parameter indentation

Parameters to functions either go
- (a) on the the same line or
- (b) align vertically or
- (c) go to the next line, with the first character indented 2 spaces more
  than the first *non-`[`*(!) parent level character,
  i.e. 2 or [3 columns further right](https://www.gnu.org/software/autoconf/manual/autoconf-2.69/html_node/Autoconf-Language.html):

```
CALL([parameter], [parameter], [parameter])

CALL([parameter],
     [parameter],
     [parameter])

CALL([parameter], [parameter],
  [CALL(
     [CALL()])])

  ^  ^
  |  2 + 3(!) spaces
  2 spaces
```

## 3. Consecutive call / multi-line indentation

Consecutive calls to macros (= on the the same nesting level) are aligned vertically:

```
CALL([parameter],
  [CALL([])
   CALL([])
   CALL([])])
```

## 4. Closing bracket placement
Closing braces accumulate on the same line in general...

```
CALL(
  [CALL([CALL([])],
        [CALL([])])
   CALL([])])
```

...but can go a new line (e.g. with `AC_LANG_SOURCE`) to match parameter indentation rule (2), i.e. either

```
CALL([CALL([
    one
    two
  ])],
  [CALL()])
```

.. or ..
```
CALL([CALL([
        one
        two
     ])],
     [CALL()])
```


EOF
