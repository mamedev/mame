# TC0480SCP row zoom: the x offset compensation, from first principles

Notes for the fix in `tc0480scp.cpp` `bg23_draw`:

```
- x_index -= (m_x_offset - 0x1f + layer * 4) * ((row_zoom & 0xff) << 8);
+ x_index -= (m_x_offset - 15 - layer * 4) * ((row_zoom & 0xff) << 8);
```

The old coefficient was hand-tuned (the code carried a `flawed calc ??`
comment). The new one is not a tuning: it is the only value consistent with
the rest of the same function. This document derives it.

## 1. How bg23_draw draws a row

For BG2/BG3 the code renders each screen row as a 1-D affine resample of one
source row, walking a 16.16 fixed-point source pointer:

```
u(i) = (x_index + i * x_step) >> 16        i = screen column 0..511
```

- `x_step` is the zoom factor in source texels per screen pixel (16.16).
  `0x10000` means 1:1; smaller means magnification (the chip only expands
  on X). Per-row zoom reduces it further: `x_step = zoomx - (r << 8)` with
  `r` the row's zoom byte.
- `x_index` is where the walk starts: the source coordinate under screen
  column 0.

The fix is entirely about `x_index`; `x_step` was never wrong.

## 2. What x_index is built out of

Ignoring the sub-pixel terms (the `(255 - ctrl[0x10+layer]) << 8` fine
scroll and the rowscroll low bytes are fractional phase only), define

```
A = 15 + 4*layer        (BG0:15, BG1:19, BG2:23, BG3:27)
```

The function computes, per frame:

```
sx = (scrollx + A) << 16  +  (m_x_offset - A) * zoomx
```

and per row, after subtracting rowscroll and reducing the step:

```
x_step  = zoomx - (r << 8)
x_index = sx - rowscroll * 65536 - (m_x_offset - A) * (r << 8)
```

Substituting `sx`, the two `(m_x_offset - A)` terms merge over the row's
actual step:

```
x_index = (scrollx' + A) * 65536  +  (m_x_offset - A) * x_step
```

so the sampled texel at screen column `i` is (with `s = x_step / 65536`):

```
u(i) = scrollx' + A + (i + m_x_offset - A) * s
```

This form is the whole story: the transformation is a zoom anchored at a
fixed screen column

```
i0 = A - m_x_offset
```

At `i = i0` the `s` term vanishes: `u(i0) = scrollx' + A` regardless of
zoom. Every zoom level pivots around column `i0`.

Sanity check with zoom off (`s = 1`): `u(i) = scrollx' + m_x_offset + i` -
the `+A` and `-A` cancel, all four layers coincide, matching the non-zoomed
tilemap path. That is why `A` is invisible at zoom 1 and why the bug could
hide for so long.

## 3. What A physically is

`A` is not a magic number - it already appears in the same function, as the
`+ 15 + layer * 4` on the scroll base in `sx`. It is the chip's per-layer
pixel pipeline delay: the TC0480SCP fetches the four bg layers staggered 4
dot-clocks apart so the mixer can composite them serially. At each scanline
start the scroll value is latched into the layer's zoom accumulator a fixed
number of dot-clocks before that layer's first output pixel, and each
dot-clock adds `x_step`. A start offset of `d` clocks at step `s` displaces
the sampled origin by `d * s`, which is why the origin term must scale with
the step. `m_x_offset` is the driver's calibration of the total delay for
the machine's video timing (0x1f for taito_z).

## 4. Why per-row zoom forces the same coefficient

Row zoom does not get its own accumulator setup: the hardware reloads each
line with the same latched origin and only the increment changes per row. A
zoomed row and an unzoomed row must therefore share the pivot column `i0`.
In code terms, `x_index` must equal `base + (m_x_offset - A) * x_step` for
the row's actual step - so reducing the step by `r << 8` requires reducing
the origin term by `(m_x_offset - A) * (r << 8)`. That is exactly the
changed line. The compensation coefficient is not a free parameter; it is
pinned to the coefficient already used in `sx`.

## 5. What the old code did wrong, and why only BG3 showed it

Old coefficient: `m_x_offset - 0x1f + 4*layer`. Correct:
`m_x_offset - 15 - 4*layer`. With `m_x_offset = 0x1f = 31`:

| layer | old = 4*layer | correct = 16 - 4*layer |
|-------|---------------|------------------------|
| BG2   | 8             | 8                      |
| BG3   | 12            | 4                      |

At `m_x_offset = 0x1f` the two expressions intersect at layer 2: for BG2
the wrong formula happens to give the right number, so games validating the
tuning against BG2 looked fine. Only BG3 was off, by `delta = 8` per unit
step.

A rowzoomed BG3 row was drawn with its pivot in the wrong place, i.e.
laterally shifted by

```
error(r) = delta * (r << 8) / 65536 = 8r / 256 = r / 32 pixels
```

Racing Beat's road puts `r ~ 0x20` near the horizon growing to `r ~ 0xa0+`
at the bottom: a shift of ~1px at the horizon growing to ~5-8px at the
bottom. BG2 (the other road half) used the accidentally-correct value, so
the two road halves and the sprite kerbs diverged progressively down the
screen - the visible "road break".

## 6. Summary

Screen column `i` samples texel `scroll + A + (i + x_offset - A) * step`.
Zoom pivots at the column where the step coefficient vanishes; that pivot
must not move when row zoom changes the step, so the row compensation must
use the same `(x_offset - A)` with `A = 15 + 4*layer`. The old
`- 0x1f + 4*layer` flipped the sign of the layer stagger and only
coincided with the correct value at layer 2.

Possible cleanup: hoist `const int pixel_delay = 15 + layer * 4;` at the
top of `bg23_draw` and use it in both places, making the pairing explicit.

Caveat: this derivation proves the two coefficients must be equal; it does
not independently prove `15 + 4*layer` is the true hardware delay. That
constant is inherited from the existing `sx` line, which the frame-level
zoom path already validates in other TC0480SCP games (footchmp, metalb,
undrfire); regression passes on those cover that half.
