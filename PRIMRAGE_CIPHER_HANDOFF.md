# Primal Rage 136094-0004A cipher: from the PSX table to the algorithm

**DO NOT COMMIT this file** (it embeds data lifted from the PlayStation port).
Companion notes: PRIMRAGE_NOTES.md. MAME implementation:
`src/mame/atari/atarixga.cpp` (`atari_136094_0004a_device`), wired into
`src/mame/atari/atarigt.cpp` on branch `primal-rage-blood`.

## Status (2026-09-12): SOLVED for everything the shipped game does

The chip is the same kind of device as the two solved XGA relatives: a
16-bit Fibonacci LFSR that is loaded with the ciphertext and clocked a
key-dependent number of times. What is new compared with 136094-0072 /
136095-0072 is that the feedback mask is selected by a per-character word
the game writes before each query. Every earlier software attack assumed the
chip's function depends only on (key byte, ciphertext); that assumption was
false, which is why linear/affine/orbit tests over the mixed five-character
corpus all came out random.

Result on the 4177 validated `(i, c) -> r` pairs:

| outcome | pairs |
|---|---|
| exact | 4168 |
| one clock short, explained by the 0x0001 orbit rule below | 6 |
| PSX entry differs from the model by XOR 0x0700 (dx off by one) | 3 |

The three XOR-0x0700 entries are not on any special orbit and their
neighbours are smooth animation data; the arcade ciphertext encodes dx = -37
where the PSX table stores -36. Treat them as PSX data, not chip behaviour.
Nothing in the tables below was edited; the tables are simply no longer used
by MAME, which now computes every answer. Where the computed answer differs
from the PSX table:

| table (characters) | index | ciphertext | PSX delta word (dx, dy) | computed delta word (dx, dy) | difference |
|---|---|---|---|---|---|
| 1 (Blizzard, Talon) | 0x247 | 0xBE07 | 0xDC4A (-36, +74) | 0xDB4A (-37, +74) | dx -1 |
| 1 (Blizzard, Talon) | 0x248 | 0x0DDE | 0xDC4A (-36, +74) | 0xDB4A (-37, +74) | dx -1 |
| 2 (Chaos) | 0x0AF | 0x194F | 0xDC43 (-36, +67) | 0xDB43 (-37, +67) | dx -1 |

The game shifts the delta left by 6 into 26.6 fixed point, so each is one
pixel of horizontal body offset on one animation frame. The PSX tables are
kept at the end of this file for history and for re-verification.

## The algorithm

Notation: `i` = query index (the game writes the ciphertext to
`0xDC7800 + 2*i`), `c` = ciphertext word, `r` = chip response,
`d = r XOR ((i*0x6915 + 0x6915) & 0xFFFF)` = the two signed delta bytes the
game uses.

1. **Key RAM.** 2K x 8 bit. A word written to `0xDC7800 + 2*s` while in key
   mode stores its low byte at `SRAM[s]`. Three routines upload the same
   0x700 bytes: the boot init (`0x217E0`, the one MAME sees at power-up) and
   the checksum routine `0x1F83A` both write `ROM[0xEA406 + s]` to slot `s`;
   the diagnostic routine `0x44228` (only when its argument has both low
   bits set) writes `ROM[0xEAB05 - s]`, i.e. the same bytes reversed. The
   layout that satisfies the data is `SRAM[s] = ROM[0xEA406 + s]`
   (`key[0x6FF - s]` in the table below, which lists the bytes as `0x44228`
   reads them). Earlier notes called the boot upload "reversed" because they
   only knew `0x44228`; the boot init at `0x217E0` is the normal path.
2. **Key selection.** `k = SRAM[key_offset(i)]` with an address bit
   permutation plus XOR, the same shape as the 0072's `key_offset`:

   | SRAM bit | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
   |---|---|---|---|---|---|---|---|---|---|---|---|
   | from index bit | 3 | 5 | 2 | 4 | 0 | 1 | 7 | 6 | 8 | 9 | 10 |
   | XOR | 0 | 1 | 1 | 0 | 1 | 0 | 0 | 1 | 0 | 0 | 0 |

   This is the unique solution among all 11! x 2^11 permutation/mask pairs
   for either upload layout.
3. **Clock count.** `n = kmap[k]`, a fixed 8-bit -> integer table with values
   16..125. The 110 key bytes the game uses map onto exactly the 110 values
   16..125, one each (so `kmap` is a bijection on the used keys; the other 146
   entries are unobservable from the game). `n` is even iff bit 7 of `k` is
   set, and bit 7 of the selected key byte equals index bit 0 (the
   `key[i] & 0x80 == (i & 0x10) << 3` property seen in the upload). The table
   is in `atarixga.cpp`; no closed form was found (not GF(2)-affine, not
   modular-affine in `k`).
4. **Feedback taps** come from the character word `W` (ROM `0xEB0C0[charID]`)
   that the game writes to `0xDC8700` (player 1, routine `0x442F2`, followed
   by the constant `0x8016`) or to `0xDE4000` and `0xDEC000` (player 2,
   routine `0x3E3D4`) before the query:

   | characters | W | taps |
   |---|---|---|
   | Sauron, Diablo | 2694 | BCC8 |
   | Blizzard, Talon | 6EE0 | AED5 |
   | Chaos | 34F7 | 9D79 |
   | Vertigo | 32B9 | FD10 |
   | Armadon | 4D5A | 82A3 |

   Each taps word is the unique mask (of 65536, Fibonacci form) under which
   all within-table ciphertext collisions of that character sit within 69
   clocks of each other. How the chip turns `W` into the mask is unknown
   (see open points); MAME uses this lookup.
5. **LFSR.** `A(x) = ((x << 1) & 0xFFFF) | parity(x & taps)`; `r = A^n(c)`.
6. **Early-out rule.** If the state equals `0x0001` after any of the `n`
   clocks, the answer is one clock short (`A^(n-1)(c)`), or `0` if it
   happened on the last clock. This is the 136095-0072 rule with its
   constant `0xC000 = A^-2(0x0001)` rewritten in terms of the state; the six
   short-by-one pairs in the corpus all lie on the orbit through
   `0x4000, 0x8000, 0x0001` of their table's LFSR, and no non-deviating pair
   does within the required bound.
7. **Zero ciphertext** never occurs in the queried ranges. MAME mirrors
   136095-0072 (`A^(n-1)(1)`); untested.

Bus protocol as implemented (window offsets relative to 0xD80000; the chip
overlays colour RAM and only drives the bus for status and result):

| access | effect |
|---|---|
| read 0x44010 (DC4010) | enter key-upload mode |
| read 0x4C7C0 / 0x4C7C4 (DCC7C0 / DCC7C4) | leave key-upload mode |
| write 0x47800+2s in key mode | SRAM[s] = data & 0xFF |
| write 0x48700 / 0x64000 / 0x6C000 | character word -> taps |
| read 0x44022 (DC4022) | enter query mode |
| write 0x47800+2i in query mode | compute reply for (i, data) |
| read 0x44700 (DC4700) | status, always 0x8000 |
| read 0x4C7C2 (DCC7C2) | reply; leaves query mode |

The mode-entry reads the game performs before those (DCC7C4 x2, DCC7CA x2,
DCC7C6, DCC7CC, DC4008) are ignored; the real decode may differ, but this
reproduces the game's traffic.

### Reference implementation and verifier (Python, self-contained)

```python
import re
txt = open('PRIMRAGE_CIPHER_HANDOFF.md').read()
blk = lambda n: re.search(r'### '+n+r'[^\n]*\n```\n(.*?)```', txt, re.S).group(1)
key = bytes.fromhex(''.join(blk('key').split()))
cipher = [[int(w,16) for w in blk('cipher_t%d'%t).split()] for t in range(5)]
delta  = [[int(w,16) for w in blk('psxdelta_t%d'%t).split()] for t in range(5)]
maxt = [0x3BE,0x413,0x310,0x2AE,0x2BD]
TAPS = [0xBCC8,0xAED5,0x9D79,0xFD10,0x82A3]
KMAP = {}  # filled from atarixga.cpp, or derived: see "How we got here" step 7
sram = [key[0x6FF - s] for s in range(0x700)] + [0]*0x100

def key_offset(i):
    src = [3,5,2,4,0,1,7,6,8,9,10]; xor = 0x096
    return sum(((i >> src[b]) & 1) << b for b in range(11)) ^ xor

def decipher(taps, i, c):
    par = lambda x: bin(x).count('1') & 1
    A = lambda x: ((x << 1) & 0xFFFF) | par(x & taps)
    n = KMAP[sram[key_offset(i)]]
    x, early = c, False
    for _ in range(n):
        x = A(x)
        if x == 1: early = True
    if not early: return x
    if x == 1: return 0
    x = c
    for _ in range(n - 1): x = A(x)
    return x

# derive KMAP from the corpus (each key byte must give one n)
def dlog_table(taps):
    par = lambda x: bin(x).count('1') & 1
    d, x = {}, 1
    for n in range(65535):
        d[x] = n; x = ((x << 1) & 0xFFFF) | par(x & taps)
    return d
for t in range(5):
    dl = dlog_table(TAPS[t])
    for i in range(maxt[t] + 1):
        c = cipher[t][i]
        if c == 0: continue
        r = delta[t][i] ^ ((i*0x6915 + 0x6915) & 0xFFFF)
        n = (dl[r] - dl[c]) % 65535
        if n <= 125: KMAP.setdefault(sram[key_offset(i)], n)
assert len(KMAP) == 110 and sorted(KMAP.values()) == list(range(16, 126))

ok = bad = 0
for t in range(5):
    for i in range(maxt[t] + 1):
        c = cipher[t][i]
        if c == 0: continue
        r = delta[t][i] ^ ((i*0x6915 + 0x6915) & 0xFFFF)
        if decipher(TAPS[t], i, c) == r: ok += 1
        else: bad += 1; print('mismatch t%d i=%03x c=%04x r=%04x' % (t, i, c, r))
print(ok, 'match,', bad, 'mismatch')   # expect 4174 match, 3 mismatch (the XOR 0x0700 entries)
```

## How the three XGA chips compare

| aspect | 136094-0072 Moto Frenzy | 136095-0072 Space Lords | 136094-0004A Primal Rage |
|---|---|---|---|
| core | 16-bit LFSR, key-dependent clock count | same | same |
| taps | fixed 0x8016 | 0xC100 plus a low byte the game writes to a register | full 16-bit mask chosen by the character word the game writes |
| input basis | table L, ciphertext bit 5 skipped, powers of two special-cased | identity: pure clocking of `c` | identity: pure clocking of `c` |
| key RAM | 2K words, one write per word | 4K words | 2K bytes, one byte per word write |
| key selection | 10-bit permutation + XOR of address bits | 12-bit permutation + XOR | 11-bit permutation + XOR |
| key byte to clocks | 128-entry `kmap`, twins via `k ^ 0xA8` | same idea, different table | 256-entry table, 110 entries known, no twin rule needed by the game |
| early-out rule | `c` on the inverse orbit of 0x8010 within k+3 steps: one clock short, or 0 | same with 0xC000 and k+13 steps | state passes through 0x0001 while clocking: one clock short, or 0 (0xC000 is A^-2(0x0001) in Space Lords' LFSR) |
| mode select | reads at window offsets 0x10 (key), 0x20 (query), 0xFC0 (reset) | reads at 0x20, 0x42, 0xC00, 0xFC0 | reads at DC4010 (key) and DC4022 (query): offsets 0x10 and 0x20+2 from DC4000, as on Moto Frenzy |
| reply | on a read of the address that was written | computed on the write, latched, read back | computed on the write, latched, read back at DCC7C2, as on Space Lords |
| how it was recovered | hardware oracle (Kirkegaard, Neves, Wilhelmsen) | hardware oracle | static: arcade ciphertext plus PSX plaintext, no hardware |

Moto Frenzy is the quirky first version, Space Lords cleaned up the datapath
and made the polynomial partly programmable, Primal Rage made the whole
polynomial programmable and tied it to the character. That per-character
taps register is the one real novelty, and it is what defeated every attack
that pooled the five characters.

## How we got from the table to the algorithm

The PSX port supplied plaintext for every ciphertext the arcade game can
send, but no chosen inputs, so the break had to come from structure rather
than from oracle queries. The chain, in the order it was actually found:

1. **Re-read the protocol from the disassembly instead of the notes.** The
   player-1 query path (`0x442F2`) writes the character word from ROM
   `0xEB0C0` and then the constant `0x8016` to `DC8700`; the player-2 path
   (`0x3E3D4`) writes the same character word to `DE4000` and `DEC000`.
   `0x8016` is the 136094-0072's `lfsr1` feedback mask in `atarixga.cpp`, so
   the chip is a programmable-polynomial member of the same family, and the
   character word groups exactly like the five cipher tables. The handoff's
   claim that "the character is not sent to the chip" was wrong, and every
   earlier test had pooled the five characters into one function.
2. **Simplify the relatives.** Space Lords' `decipher()` is, for a
   non-power-of-two input outside the special orbit, exactly
   `p = lfsr1^(kmap[k]+14)(c)`: the powers-of-two table is the identity basis,
   so the chip just loads `c` into the LFSR and clocks it. That gives the
   family prior `r = A^n(c)` with `A` an LFSR step.
3. **Orbit test, per table, over all feedback masks.** Two queries in the same
   table with the same ciphertext must have responses on the same LFSR orbit,
   a bounded number of clocks apart, regardless of key mapping. Under the
   0x8016 mask the distances were uniformly random (so the taps are not
   0x8016), but sweeping all 65536 masks per table found exactly one mask per
   table under which every within-table collision pair (9, 9, 7, 3, 3 pairs)
   is within 69 clocks. For table 1 the chance of that is about 1e-24.
4. **The map is pure clocking.** With each table's mask,
   `dlog(r) - dlog(c)` over the orbit of 0x0001 falls in the window 16..125
   for all but 9 of 4177 pairs, with 110 distinct values, matching the 110
   distinct key bytes. The same index gives the same count in every table
   (950 of 959 multi-table indices; the rest are the 9 special pairs), so
   `n` depends on the address only and the character word affects only the
   taps.
5. **Locate the key.** Neither the forward nor the reversed upload layout
   makes the key byte at the query index determine `n`, so the key index is
   scrambled. Index bit 0 is constant within every class of equal `n`, and
   the upload has `bit7(key[s]) == bit4(s)`, so index bit 0 must feed SRAM bit
   4. A brute force over all 11-bit bit-permutations and XOR masks, requiring
   only that equal key bytes give equal `n` (twins are allowed, as in the
   relatives), found exactly one solution and only for the reversed layout.
   An earlier run that also demanded distinct key bytes for distinct `n`, and
   one that assumed the forward layout, found nothing; both assumptions were
   wrong, not the search.
6. **Read off `kmap`** as the induced key byte -> `n` function; it is
   consistent over all 1044 addresses including the 20 above index 0x400.
7. **Special cases.** The six short-by-one pairs all lie on one orbit per
   table, and those orbits pass through `0x4000, 0x8000, 0x0001`; requiring
   every non-deviating pair to be off that orbit within its bound leaves the
   "state hits 0x0001 during clocking" rule, which is also what Space Lords'
   `0xC000` check means once its constant is translated.

Tools used: the parsing/orbit/brute-force scripts were run from this file's
data alone (a few hundred lines of Python and C: an all-taps orbit scan, a
threaded permutation search, and the verifier above).

## What is still missing, and what was tried for it

### 1. Character word -> feedback taps (MAME uses a 5-entry lookup)

Only five `(W, taps)` samples exist, all from ROM 0xEB0C0, so any family with
more than about 80 free bits fits trivially and cannot be identified. Tests:

| family | result |
|---|---|
| `taps = W`, `rev(W)`, byte swap, `~W`, all rotations, all shifts | no |
| `taps = f(W) XOR / + / - constant` for every f above | no |
| `taps = W * m mod 2^16` (all odd m; even m impossible, taps 1 is odd) | no |
| carry-less `taps = W (x) m`, all 65536 m | no |
| `taps = W * K` in GF(2^16), every irreducible polynomial (4080), Fibonacci and Galois bases, also on `W ^ 0x8016` and `rev(taps)` | no |
| `taps = LFSR^k(W)` with 0x8016 taps, either form, common k | no (each W reaches its taps at unrelated k) |
| `taps = LFSR_W^k(seed)` for seeds 0x8016, 0x6801, 1, 0x8000, 0xFFFF, taps W or rev(W), both forms | no |
| same polynomial in another convention (reciprocal `(taps<<1)|1`, `rev(taps)`) | no |
| **bit permutation of W plus XOR mask**, i.e. the same shape as `key_offset` | **yes**: 6 masks, 48 (permutation, mask) pairs fit; 0 of 100000 random five-word tap sets admit any (baseline below) |

Test used (a mask `m` admits a permutation iff the multiset of 5-bit column
signatures of `taps ^ m` equals that of `W`):

```python
T = [0xBCC8,0xAED5,0x9D79,0xFD10,0x82A3]; W = [0x2694,0x6EE0,0x34F7,0x32B9,0x4D5A]
from collections import Counter
sig = lambda vals: Counter(tuple((v >> b) & 1 for v in vals) for b in range(16))
masks = [m for m in range(65536) if sig([t ^ m for t in T]) == sig(W)]
print(len(masks), ['%04x' % m for m in masks])   # 6: d0c4 d4c6 d8c6 f0e4 f4e6 f8e6
# baseline: 100000 random five-word tap sets admit 0 masks (C version, 2 min)
```

The permutation family is therefore the best lead. Every candidate agrees on
these wires (`~` = inverted):

| taps bit | 0 | 2 | 3 | 4 | 6 | 7 | 8 | 9 | 12 | 14 | 15 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| = W bit | 6 | ~4 | 2 | 5 | ~3 | ~(0 or 12) | (0 or 12) | (11 or 14) | ~(11 or 14) | ~10 | ~15 |

Taps bits 1, 10, 11 take W bits 8, 7, 13 in some order (bit 8 inverted when it
lands on 10 or 11, bits 7/13 inverted when landing on 1); taps bits 5 and 13
take W bits 1 and 9 in either order (inverted when swapped). W bits 0 and 12,
11 and 14, 7 and 13 are indistinguishable because they are equal in all five
words. Since every W has bit 15 clear, all candidates make taps bit 15 = 1,
i.e. an always-invertible LFSR, which is a sensible design and supports the
family. One extra `(W, taps)` sample from hardware would cut the 48 to at most
a handful; three well-chosen ones would settle it. Without it MAME keeps the
lookup: all candidates agree on the five words the game uses, so nothing in the
shipped game depends on the choice.

### 2. Key byte -> clock count (`kmap`, 110 of 256 known)

Tested for structure, all negative: GF(2)-affine in `k` (for `n` and `n-16`),
modular affine `n = a*k + b mod M` for M in 110..256, `n>>1` as a function of
`k & 0x7F` (25 conflicts), the 0072 twin rule `k ^ 0xA8` (22 twin pairs
present, all with different `n`). The only structure found: `n` is even iff
bit 7 of `k` is set. The relatives' tables also have no closed form. The 146
unused entries are unobservable from the game; hardware with a custom key
upload (one distinct byte per slot) would read them all in one pass.

### 3. Corner cases inherited by analogy

- `c = 0`: MAME returns `A^(n-1)(1)` like 136095-0072; the game never sends it.
- state hitting 0x0001 on the last clock returns 0 (136095-0072 rule); no
  corpus entry does this.
- `c = 0x0001` (m = 0) is treated as not early; matches 136095-0072's power-of-two path.

### 4. Bus decode

Modelled from the game's access sequences; the roles of the `0x8016` write after
the character word, of the `DCC7C4/C6/CA/CC` and `DC4008` reads, and of the
checksum readbacks (DCC7CC, DCC7C0/DC4010 sums) are unknown. `DC8700` is data
window word 0x780, so the player-1 character write may be an ordinary data
write the chip repurposes. None of this affects the game: the checksum
accumulator is never compared.

### 4b. The checksum accumulator 0xFFFF86F0 is inert (verified 2026-09-12)

The HLE returned 0 for the chip reads that the game sums into RAM word
`0xFFFF86F0` (DCC7C4 x3 after the key upload, DCC7CC x4 in `0x442F2`, DCC7C0
and DC4010 in `0x1F83A`); the device now returns colour RAM contents there.
Whether anything consumes the sum was checked both ways:

Static: the program ROM contains exactly six references to the address
(`0x1F844`, `0x215C8`, `0x28046`, `0x3E3F8`, `0x44236`, `0x442FC`); each is
either an `add.w` into it, a plain overwrite (`0x3E3F2` stores DCC7C0), or the
query primitive `0x28038` using the word as scratch: it clears it, spins on it
while polling DC4700, stores the DCC7C2 result into it and reads that back.
Every query therefore destroys whatever was accumulated. The only
register-relative user is the boot init (`0x217E0`, A4), which only adds.
The routine `0x1F83A` that re-uploads the key and sums DCC7C0/DC4010 is item
17, "DUMP KEYS", of the hidden developer menu at `0x1F5C0` (jump table of 18
items: Reset Game, Exit Menu, Set Time Unit, Drone Options, Damage Toggle,
Debug flags, CENTER OF MASS EDITOR, PLAYFIELD EDITOR, MOVE/SEQN/TUNE EDITOR
TOGGLE, Dizzy Toggle, HUSH, Tribe edit, SHOW AI LEVEL, START AUTOPLAY, TRIBE
TOGGLE, DUMP KEYS); it is not reachable in normal play. ("CENTER OF MASS
EDITOR" is presumably the tool that authored the protected per-frame deltas.)

Dynamic: a 90 s headless attract run with a read watchpoint on the word
(1974 queries, 961 player-1 aux sends) logged readers only at `0x280CC` /
`0x280FE` (the primitive reading its own scratch), the read halves of the
`add.w` instructions, and `0xC78` / `0xCA8`, which is the boot RAM self-test
walking every work RAM word. No compare, branch or display depends on the
sum. By contrast the access counter `0xFFFF86F4` next to it is read and
compared at `0x28214` (the anti-tamper check), so the game does consume such
values when it means to. A replay of a full fight (armadon01.inp, 2334 frames, 49486 queries, 24697 aux sends) with the same watchpoints produced exactly the same reader set. Conclusion: the sum is dead data; returning colour RAM (the device) or 0 (the old HLE) there makes no difference to the game.

### 4c. The hidden developer menu (where "DUMP KEYS" lives)

Routine `0x1F5BA` is a modal developer menu (item names at ROM `0x6C36C`,
pointers to text at `0xDF714`..): Reset Game, Exit Menu, Set Time Unit, Drone
Options, Damage Toggle, Debug flags, CENTER OF MASS EDITOR, PLAYFIELD EDITOR,
MOVE EDITOR TOGGLE, SEQN EDITOR TOGGLE, TUNE EDITOR TOGGLE, Dizzy Toggle,
HUSH, Tribe edit, SHOW AI LEVEL, START AUTOPLAY, TRIBE TOGGLE, DUMP KEYS
(= `0x1F83A`, the key re-upload plus checksum sums). Its only caller is the
main loop at `0x10554`, gated at `0x10542` on `(RAM 0xFFFFC5F8 & 0x30) ==
0x30`. `0xFFFFC5F8` is refreshed every frame by `0x21EF6` from the OS input
latch `0xFFFF8484`, which the interrupt at `0x860A` fills with the inverted
byte read at `0xFFE80003`. So the menu opens while the two switch lines on
bits 4 and 5 of that byte are both held low: in the driver these are bits
0x10 and 0x20 of the "P1_P2" port, currently `IPT_UNUSED` (Primal Rage only
assigns 0x02 and 0x08 of that byte, the player 4th buttons). Inside the menu
the cursor moves with bits 0x80 / 0x40 and selects with bit 0x01 of the
per-player input word `0xFFFFE180` (filled by `0x1EBEE`; joystick up/down and
a button, to be confirmed in play).

The lines are only sampled when the operator option **Debug Switches** is
**Enabled**: `0xFFFFC5F8` is filled through the OS input service `0x86B0`,
which returns 0 unless bit 2 of the OS flags word `0xFFFF8498` is set; that
word is loaded at boot from EEPROM setting 0x2A (`0x5B36`), and the operator
menu page at `0x446A` edits it with the generic bit-field editor `0x4630`
whose option block at `0xBA30` encodes each line's field in its prefix byte
(`position = byte >> 3`, `width = byte & 7`): "Debug Switches" 0x11 = bit 2,
"Video Slave Asserted" 0x19 = bit 3 (default High (REV 1) = 1, the live value
0x08), "!Development" 0x21 = bit 4. Verified in MAME: with bit 2 forced from
Lua and the two lines held, the menu routine is entered; with bit 2 clear,
holding the lines does nothing.

The "Debug Switches" page is itself hidden: it is the `?DEBUG OPTIONS` entry
of the SELECT TEST menu table at `0xBA98` (with `?SCOPE LOOPS`), and the
menu renderer skips `?`-prefixed lines unless bit 4 ("Development") of
`0xFFFF8498` is set. That bit is set by the test-menu entry code at `0x5BCE`:
when the SELECT TEST menu comes up it reads the joystick latch (`0x8678`)
and, if P1 Up + P1 Button 1 + P2 Up (port bits 0x82008000) are all held, prints
"OK, I HEARD YOU" and waits up to 180 frames for them to be released, then
sets bit 4. Bit 4 is not saved (setting 0x2A is written with it masked off);
bit 2, the Debug Switches option, is. Test mode only opens at power-up in this
game (the service switch turned on during attract did nothing in 60 s).

Verified in MAME with Lua-driven inputs (service switch on from frame 1,
combo held, released 30 frames after the message): "OK, I HEARD YOU" at frame
89, flag word 0x08 -> 0x18, and the `?DEBUG OPTIONS` line rendered.

MAME has the two switch lines mapped as one input, "Developer Menu" (key M
by default, `PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_OTHER )` in the primrage
ports). Full procedure:

1. Turn the service switch on (F2) and reset (F3) while holding P1 Up,
   P1 Button 1 and P2 Up (default keys: Up arrow, Left Ctrl, R). Wait for
   "OK, I HEARD YOU", then release the three inputs within 3 seconds.
2. In SELECT TEST choose DEBUG OPTIONS, set Debug Switches to Enabled, save
   (it goes to EEPROM setting 0x2A, so this step is needed once per NVRAM).
3. Service switch off, reset into the game, tap M. Exit Menu returns to play.

Without the input mapping the debugger alternative for step 3 is
`bpset 10550,1,{d0=0x30;bpclear;g}`, and for the EEPROM bit
`w@ffff8498=w@ffff8498|4` before pressing (RAM only, not persistent).

### 5. Three PSX entries

Table 1 index 0x247, 0x248 and table 2 index 0x0AF differ from the model by
one unit of dx; MAME produces the model's value. Only hardware (or another
plaintext source) can say which is the arcade's.

### What would settle everything

With a board and a patched program ROM (or a ROM-socket injector): upload a
key of 256 distinct bytes spread over the slots, write a few dozen chosen
character words, and for each query `c = 1` plus a handful of random words at
one index. The LFSR step count is then read off directly (`dlog` under the
recovered taps), giving the full `kmap`, the character-word wiring, the
`c = 0` and last-clock behaviour, and the answer for the three disputed
entries. This is a few hundred bus transactions, not a gameplay capture.

## Data

All hex. Tables indexed by i (word index). Pair validity: cipher word != 0
and i <= max_t. Char->table: {0,5}->t0 {1,6}->t1 2->t2 3->t3 4->t4.
max_t: t0=0x3BE t1=0x413 t2=0x310 t3=0x2AE t4=0x2BD.

### key (0x700 bytes; SRAM[i]=byte i; ROM byte i = maincpu[0xEAB05-i])
```
7947300b334a21473d331a5317657455939dcde0c1a99dd48ae0f3b5c6c6d5d3
775833654a3c193d177b6358056506268e90c18ef0a4f5ab99a6d4ebf6bdb5f5
390b4b7b054d4a4d581a1928734d194793d9b89dfad4c1e18e9dba9d99f9a4b6
74764d742d05154d1c5a583e47631c58abe4d3ab85ca8a87b8e3cefa98a5eb85
63585a174d2d497e735652547b19497becd9bdf7f9a0fcf7fcd2e18a99e483bd
1c737f4f284b154f742855542d2d264ca0fc8989b698a5f3f99298a9d3b2ebf5
73540b224725742220284041403b7e4d999299f3bb9283a9d393d5d498bbf0d4
281c3b49740a743047630e22630e4b47e9b5d4ecf7a7cee1fcd483bacdd4f3c6
0b417e17521c1d1c393e4a77257b3b738ecafafa92bd9295d49cf0b6abfae0ae
6f28201c320b262d1a06550565415326b5f9a99dc6f79c8ed5d38798b8bba79c
637b5626203e300b5a174d777e0e4b33ce9ed4cde4a0d9a7d9d9f585bba0bbc1
4b283e3c1d1a7e53254f173e63325a65a0f3f7eb9c9c85fae4cdfcec98a5a7b6
47174b1c22470a7f79734f6363053376d4baf7e3f385b6bb87d2d4cee489bafa
0e4c7f2d53061a15220a3d0539210522f7fa90e9a9a693f6a4e3998eaed5f5cd
404a7655651941261774394a0532400a9299c1e0b5e4e085e0b6f398ca9da0d4
393b7b4f3e7940300b2d391c284f7f76a6e3ebcde985bb99bbf9abe4fc9387d9
7b5e791d63281c174c40555874074d52d993e0bbca87b2cdf6b5b5ae8ae1cd98
476f473d286f0e77334a333020337625fcfc89f3e3e4b6c6c1fcd38ed9d4f687
063e563030302805774b26794d521954f395b2cd8387bdcd99ecd395a5a5b5c1
655a4f52533c5a774b774a7b49055806bbcabda0abe4ae90d2f9baf7f985e9a9
7458153d401d2d3b6f790e2d4b537b74f7859ed58399ae89f799a9d5f7aea4b8
55173c4b28217758530b5e74254d054a95a087cae099cae1b29c8387e08ae4d2
201d7622401954304d15400649174f3ef9d5a5c6e4ce98cece98a0a4d98393d5
5e330a654974053c7e3e250b47250a73f5a785bde4abf589f6abb6e1bbbaa4a4
284c3d39743c774f1a39255a05773221e9859cbda6d5a792c185d3b893b6f9f3
5a47304a326f05747919554b58055e4c93ebf3a5e3fcd5d3b69583f6b6e989bb
2174654f4a5440190e79524f153b7e5298e1fca59ea4bba4f783fcd9e4f5e3d9
1722797976541979324b7f07657e331aec92e4cea0b5a7989893b5ecd9b893ba
0a7e4c7753403d7f535e5a4f4f33472598e3e49ce9f7bdf3e489d492d9f9d49d
795a200756302d193e0a0b0705523321f7b6b2d4f5a6ab99c1a598e189ae93f3
057641191d06771d4b4f744d58175539d9aed583d9c1bbeba6bde3ece9a59399
22281c1c39526526653b520b212d3e5ae999e1a6d9e9d9d2f9ce909da6f995a7
7b6352171c152d586379775473410e76ce92e0a9e0879da993f6a587fcd4a5ab
7725285e584177541d071c05562d4f63cea0d998939cc1f989a5b89cd4ab95b6
5677300e3b491a201953157b22302656bdf685a589d3e0fa8792e0abebbb8abb
39197f284a1522734b5a0a4b060e3005ecd29c83f9d9fabdb8bae1cd9cb5c69c
4a474f473e400e3b55174f0573222d4aa7a7fce0e0d3f6e4a5b8d9d3a9a099a4
4a39203c280b5a4f55735e3d7b5a4f1ad9a592f0d2998a98a0f5f6e3a599e4cd
7e7f410e5207580a4055403e4976225685d99d9ccdcec6ebf9a0c1f5e4c1b6fa
0a4f0a4f224c05735a567e54582d7425d3e4e3e0d9faa0fcd2e1e9f0abebfc85
174c63564c5830651a586577631d6326a693cdd2a0ba87cdf0a5b8baa7b8fa89
412554334032492519562d3341213353a6bafc959de0b2998599d99d83a499b8
406373764d331a25403d4d3c307e7f33a7cdb8bbfaf799f7ebd5abd2d4a9f595
3d4d0e7b4c1539175e26394a7e655263a4f9d9e392cdfaf6f78efce1e3f7f3ec
0a7e5449551c774b470b587b391c7322e0b8a4b28e93facdb29ce4f3879ec1ca
3254303e4c5828073b4053561921791db5a48abd92abd3caaeae89aeb6aba4fc
0b7e284d790b747e05560b324d175452e9cefcb5cef7aba0cab5caf3e185e1e3
543b391d47474f6515254b1715637e5ecad3939eecc1d3f0a6d9e18ae08ea699
223c4d3e1c223d7f54263b3063304779bdd9f5c6a79cd4bdbbcdaefcf7b8fae9
4a062d3b1522763e47794c281a39176fa593f0999dd3d287f5ab9898bdcd9df9
2d2619204d5e1a4a304920533c4c210bbdabd5989598c1fa9998f7fc9d93cdf9
653c300a07587f25202547280763544d8993ae99f090f6a5fcd48e8eae8e99ca
536f330b3d734b7e497f1a764d30323eb6d4f692c6a69ea5bb8e83d4cab5a4ab
205a1d0e1c5a3d3e3d255e3c330b5347f5f5caf3d5a5e1aed4d3a6cda4baebd9
3d790e4949204f5e4a74226340587b47b5f6f0b5cad3d5f9e1d2cde1cd9ef998
1a543b25732d741a5e170b155373330bf6baa49998cd9ea9bb99e1ca9ed595fa
```
### cipher_t0 (0x44C words BE; ROM 0xF0000+0x0; valid i<=0x3BE)
```
038b 2927 dcdd efec 5641 e985 3b61 4235 ef46 8c0f bde0 d13d 1e28 10a3 cdbf d493
b620 13b8 a46b 6490 9b83 e779 935a 2e35 de94 da6e f50a 1055 6041 dfc9 739f 78c5
6eef cba9 0245 2a5b 9b75 1ea4 0697 2025 c2e5 518f 2878 674d 1bd7 5130 d551 c96d
6359 7901 deac aabc fe3d 4c65 275b af02 5acf b0c2 fc1d 1ae3 58db dae3 1d70 db2d
f330 78d5 d278 fb36 617e 8ac9 d3fe 4aa7 5534 846e 1571 4ed3 cd34 bdd3 d68a 0836
836e 5108 279e 5886 1094 f5af 92bb 5105 a61b 7307 2bf8 7266 9b6b 35c7 0197 edac
9dda 5011 ef8a 5abb 1ce4 259a 3f10 5760 221c df73 b698 a62c e42c bbfb 11b4 1d2e
b6ba 0044 eeb3 2467 f1cb cea9 57bd 1a04 ef7c 8c36 b960 f436 33e0 987d ee57 5cb5
89c6 7684 e08c 3f10 3ad4 9ce7 da1a b13a e88c 051a 3a23 b5b4 31c5 77dc 0d01 d52d
cbda cfb3 13c7 bd94 c31a 1454 7814 c3ab e82b 5871 f706 0a2f c34e c0e4 f249 75db
6bd8 f7d4 2d9f 2985 7288 51c3 e171 38e9 fc58 6fb5 91a2 ee8a 0139 9b08 2cf4 3a08
ca26 d433 3551 b394 a797 f3e0 190c aecf 646f 6dc9 8adf e88a ed59 0ea6 9c04 2624
2574 7a4a 3f7a c9f0 e9a3 1040 8ea9 003f addb be92 4380 1af2 49ff 79cf ac26 bb16
759a 2cac 18c3 86bd 6f91 5814 b806 a3d7 84cd 12da 3c86 deef 3174 ec8a 8b18 5895
b833 9ffe b1b8 5ad2 8e24 0edf ed0c 4517 de78 6496 f06f d08c b19f 2b33 a9ea cfb5
ab85 0bac 14c3 8a5e a9a4 7603 e2be bf17 c724 2564 1cb3 73bd afd7 9bdb a717 c98b
d6a7 1894 b790 4bf8 e715 5cad 767d 04e1 788e 3804 a272 d189 4344 19c7 74dd 9ff3
9f77 cd6a 90e9 5808 255a 027d a1e2 a609 9f5c cb1f 93f5 99ea 71ae 57c4 46c2 644f
31d2 95e4 aa36 9f0d c149 da43 929a 06ad 7bb0 183f 5587 a81e ced9 6f83 35aa 508d
bebc db98 7971 c64c 0eca 3284 d81c 2c44 24d2 8cd3 331b a877 dcd6 135d 7b4d e4b4
96ea 040d 1070 af69 06a5 3304 4a8f 19bd caae 87f1 7ecd da37 87c2 1ffc 0d1b a69d
b031 847f 7ee5 5fa0 dd78 1183 a5b7 f147 2d4d dc98 d9e4 8514 91c0 15cf 0999 cbb1
1c12 cfb5 2282 19c4 4760 5b68 db9d df74 76da b00e 7084 9636 feb6 840c e1ad 3dab
942e a3fe b4a4 bbc1 7442 3f82 c721 823a 639c 2cbe 2d1c ea79 df46 7711 ece2 e3db
9af3 6394 e000 4df6 eb5f 8d5e 6b62 e975 c673 ca66 a3bc 57b4 60b7 9b75 4ddd 8866
5245 8c36 e171 ba20 0655 3947 f90b a7e2 7cc7 ffd2 571d 38b2 764e 6036 326e b54f
6ab2 3b87 2909 fc21 354d 4fbf ce62 0e05 e78c 85d1 62fb 1451 47b4 9071 9a74 8e2c
d032 7bb4 902b 5883 cd7a d552 ed91 1953 096f 82a4 f715 02bc fc93 bd48 b177 7302
1223 0cbb 40e3 caee 36cf ce21 06cb 0713 472c f097 9714 c83a 3520 1c8f c7ee 3dee
fa18 500c 091f 557b 3815 2d2c f699 471c 999e 1d03 5f18 a609 c2ae 389d 91f1 fd36
78c9 3442 2999 4b10 8756 d498 55c8 6600 bf1f 7663 a8ef b6ac 2df7 028c 3940 9211
cc91 5047 a64e cf6d d181 3149 8b11 2f9b 3201 2fe4 ac97 85bc d45f 3ed6 35d6 3858
669f 2fcc fd3a b0ae 18eb 49d3 e94f 57c9 6398 8adc 948c eb3a a0f4 3689 c2e0 bbf5
e32f 487c 099c ba36 af22 e49e 7924 7e68 dec2 d48a fe05 79e0 3020 65a6 25f2 3b93
7892 daec a9b0 a6e6 15b8 53d5 d865 0f04 1faf d1b1 fa63 f068 96ff 5a24 3697 1a9f
f9c1 2621 fc31 1dd8 9b64 1009 01a6 e065 496d 6481 647f 8285 30cf 3d39 ec31 cd45
c125 8ad7 b3eb 9961 c420 4cb7 9059 7f9a 88a1 d258 1858 7b39 bd57 750e c563 bae0
5f50 cf1b 3ff9 3bfd 367e f3fe 0641 dbc9 f669 743a d5b4 b8de ddd8 2c08 0bd8 6fd9
de91 a2ab c96a d10b 8b91 c92d a198 2810 af70 7b97 a709 8155 455e 5615 d211 32ac
1d26 7b3d 2a94 9337 e7ce ceb8 a134 584e e757 7949 821b 9bed b57c 3bfc 7a9e c17e
17ba 5ee8 b0b6 9ee5 a7ba bbab 0f41 a66f cc12 86e6 8eee 59b3 c6f9 ee1a 1ee6 6e8e
e377 4c2a 7e6d 5a33 1467 6b79 47be bf61 b5ac 8a70 cd28 516f 223d 3d5e 9b66 e989
0531 7ab5 22bc 9be5 2033 cbd1 c3dd a8a4 f1b3 37dd 7f6e a0aa c3f1 1299 82fc 8771
6f3e 4395 f703 3039 c51c 99b5 f8b7 4290 6a5c 5cab 5f7a cec8 536d 2a5b eaaa d8c6
6267 8d7f cf87 a930 509d 1ec5 5589 2bc7 d135 d3dc 198b 7546 7fc5 accd f2c9 a7ef
b88d 648d 7ca3 db66 5fd3 2197 1881 3f39 165e b90a f30f 52a7 17b3 95bc 99c0 ded9
b54c 3ba6 44d9 a53e 81dd 6c94 3313 ccbd 1fc8 0f7e 0d6e db8d a2fd 3ead bed0 5d36
a334 4d52 f8a3 2440 c061 c75c 079a b7e7 c2ba 2c81 26f5 df92 f2fc 1f63 a2db 7a09
2799 d1e7 64b4 d08f c62e 3073 93b6 4375 d382 6923 4bef 4667 514d 39fb 3d01 9067
0c2a 50d8 a15f fe14 fbeb 92f8 f4e6 76a3 55f3 2741 1f99 8429 4d57 9fd9 fcbe 963f
30c2 fa70 3b59 37df 3508 3592 4d91 447f a0cc 1750 bcc8 309a 575c 624b 59ad a5fd
8ce1 dbff 1029 ca47 07bc ce45 ef74 f49a dc29 3bac fc37 8618 fc10 6164 777a 74e7
58d9 7367 0c82 e547 e4dc 34ec f919 5d24 30e8 7afb d463 0079 4ba6 8049 d98b 4f3b
0f60 95bd 11e0 adee 5d15 e68c 80b7 1365 7fab ea6d aa4b e096 1861 32c4 e273 3815
ea76 261d 7723 1535 880d e85a 2243 7953 cbb5 6853 8037 23a9 9cde 1ef4 3394 f90f
09ba 8e39 9948 2eac c6f9 e3e4 32b6 76b5 9510 204c 41a1 8265 ac6a 0473 a8fa f75b
4fad 64c1 574f e051 fecb e71c 515c 41a7 5e54 5970 45d6 7c36 6a52 f2d4 f72e 47aa
5d9f 28be 21b9 2a42 42e8 68e3 47bd 42db 7edb c8a4 4088 6434 dedd 8731 2134 e7b8
abb4 769a 993e fa9e f93e 36d4 9241 14c7 a415 2ac3 f45c 7650 0478 baac 2e5b fabf
ae10 173d 7f34 b190 57e7 b9ea 5ea9 de48 0515 6454 f182 7ada ecf5 fc6b e929 4bb0
a37e eba6 addc 6292 bfe1 b511 ff27 2159 8008 68eb 69c8 2fa4 e01d 7d4e 136f 8ab3
472c 3797 de6c 14fe de69 ab30 3b80 a8df c58a 9f9e 0bc2 0198 41b9 ecf4 923e 6cc5
add8 3931 fe5f 114c b48c 3c41 f508 5117 5ba2 fe9b e175 92cb 75ff d699 5bb6 3d5e
6c6f 75cc 4f2d 1a9b 64f0 b615 2b84 d2ff c476 1c4b 7ff1 70cb 505b c308 9e0f b65b
3ce8 580b b903 2b33 b639 6111 4973 e0d6 655b 9d5a a705 ff92 dbc8 0f0f 3a33 fc08
89fa a33e 542c b091 c9bd 2bd0 d9d0 e3f6 a10c 88ff 9414 ce55 b1b6 7e88 fd36 42ef
a41a ae95 ef47 2037 45ff 1981 f6c1 a14b 5ab5 55b2 e23a 5864 811b c2b2 ed6b 603f
b40d df81 234e deb9 4ce9 f0a4 8399 d767 1f34 1cb0 e1ce d38f b4ae 54c8 8569 dc0b
d0b8 d6da 1add 19a3 87df 6a1e b34b 842f caa8 0132 a371 2e92
```
### cipher_t1 (0x44C words BE; ROM 0xF0000+0x898; valid i<=0x413)
```
8636 41cd de2a 2c8f 084e 8d2c b3d5 3dff ec06 5c79 5f20 cd3d 9937 d47e 09b8 39d9
252e 196b a0ac c082 293e ca52 8d9f e141 36ce 4cf0 33fd 0b46 b57c bb18 aa35 a627
b94c c17f 3842 18eb 0413 f8dc a209 0dcc bddd 5b0e 4e79 947c 6472 e174 20b2 4ad4
e1b9 3604 231d 26e8 9435 cf0e 7f6c 4483 4c0b dd92 ea79 b77a 9d50 880c cab5 e1a6
cfe2 21b2 fe83 e7c7 af80 6e8e 1866 949f 421b 78ea 7703 2a2c b2e0 71f3 23c5 2e8b
e12a 3ff5 808a e88e 9aea 5dae 8cbb 1eaf ef86 60f2 920c 6d13 7eb3 a768 2078 a534
a43d 3bd8 f728 7fe3 4d46 5630 d1d3 5a20 d4dc 40a9 0dd0 93dd 20b7 911f 9a5a b0d3
5c25 c107 6950 d00e 5888 4859 3244 af37 20ae 3eea 9796 4049 f957 8f8a 3ef5 7758
9bf0 75f6 b134 4041 6918 e324 3833 0307 5b08 94ae 07da 545b b593 e789 d5f4 216b
8703 ed1e 862d 6cd8 886b 63ac eae1 48b0 8694 09b5 4ef1 e84d 32e5 f217 c6de 90ac
da7f d305 33fa 0fa2 1a93 f32f cfd8 cb11 e994 81ce 4ce1 8ca0 88da 75c7 efff 4275
c8c5 2abe c52e fd58 fb25 aace 0cf7 c201 b697 2f0d 65f6 f106 ee0a e752 0e51 8dce
9f84 e962 4eb2 b54e 2625 8eb3 7a92 b490 e579 3c2a b158 56d0 06a3 7e6a 304f cd56
1efc ad58 27c9 768c a751 eff9 c3f1 4d5e 6a71 2b19 b522 7ef4 584e 7763 960b df50
0b0e b49d 0b85 3eee 591f 2656 6a2f a844 bb30 35c9 2d19 9bf4 0f71 0e75 20ee 8ae7
e3b3 6a4c dbf9 8985 0695 9484 0904 f60f f2a6 b216 2128 ef21 2949 d0d3 b234 34df
508e 8aa2 84c9 47d2 1913 d4d0 1baa 0dd8 c758 bfd3 7d3b 3eee 9f5c 69bf d78e 1ef1
d7b6 75ff a88b 2ba3 b1e0 6394 ec44 5795 37e3 ba5a ba89 308c 3689 2bd7 da51 8144
165c f949 87f1 c446 a53b 8bf8 0713 97e8 ba4a 0f3d 7738 baab 989f 5fbc ff56 5c3d
1cee f9b0 45d1 5799 1144 8967 c90f 539c 934f 4c8a 1225 5151 f588 4fdd 4dde 526b
ea3c 4c32 c883 dcb4 7433 2721 71f7 6ec8 fbc0 0ef1 1b18 7973 c084 cc53 e754 c6ce
e5ae 1853 cbcc 2bb5 d81f 6095 9774 9e4a f174 dbb9 6862 95bd 210b f2ad cf7f aebd
96bb fc41 0b68 c315 bff0 fdc5 2a81 34cf fb6e 5d1b 4710 5ebe 2ace 9c94 55e1 cf4a
0e04 54c7 8ff7 857b fde6 169e 8707 c13f f0d7 be88 9293 d37d c86f 5ee0 9eaa ee12
bad8 1b00 0c13 d84a a54b 90d7 2868 64de 57ab 3020 bcb4 cc91 9808 4206 735f 2d5b
a176 46e7 b950 49e3 01a1 6bbd a648 2af6 c581 139e 0abc 8196 8e9d c087 20b3 77d1
c690 4ce8 d725 ee7d 07a6 5cce 1e83 5154 8795 15ca 9a6f 9ac4 4caa c6cb fcfe 1ca1
d0dc 324e 28e9 4323 c7ff 25a6 88ff 43f0 d189 196e 7c37 a567 e1a8 9951 8464 d103
b6fe 91bc 77d4 3a9a b47f dc33 30a0 ee85 1e4c f3e6 b6cb 1f53 3729 d229 ea34 f1fc
b1ea b619 6a4c c8f7 457f cba9 858b 8e5f d125 d43b 0689 ca39 406c ccd4 04a9 b2c9
24c9 9844 715e 0911 a966 a1af 7285 10b9 8f8f 9fe5 1c15 7a3f 725f 4c85 f7c7 dd2c
29f5 8fc5 8fed a9df a262 459c c731 0a4d 67d9 eb50 89af 3523 d7f1 724e d708 f2ca
0544 3da3 cd6c 6e00 3ae8 0a65 2fb9 f8a8 7e89 afda 5a7f 3692 fbba 56ab 1f86 587b
f551 7b02 1613 2bd1 6c2f 971d f878 7dd9 d408 1620 7c0a a3e7 0f55 57da 07a5 1510
77d0 6fc8 1b04 94fc c017 faeb 1af1 1614 6e25 025f cdc4 0119 91e0 3721 68c2 ee02
605b 2653 d536 af41 7e2b b751 36b5 64f8 09b1 2f7d a6f8 bfda 1834 e0de f415 8edc
e9c4 29b2 6c0b fdf1 620c cdfe f311 be07 0dde 2938 b486 5ebc fcd4 bcb6 25c3 e4e0
ec02 2f15 38a9 b4ce 3fca 8d00 cdb7 0f00 8d38 2421 3f4a ceb6 cbc5 82f0 70a2 ace0
dda4 8c1e cd63 9997 1e7b efbb b2af e75f 499f 4ea9 6f49 afb0 1a99 6de7 6174 fd48
edd0 7cb8 0f90 3112 e881 8bac 78de d198 e67f 7132 6744 9d52 73b0 2f34 6c62 3e12
3a14 e5ea d4fc 7448 05bd f05b 9019 b3ab 807f 13f6 96f2 3c09 dd13 3bbf f3e8 9a23
aebe d473 a888 5c5d bc39 d1d6 d61f bdf3 eb77 bc0f 8b7e e262 8934 9636 0205 3121
8d4b 16e5 0ff2 a620 04ed 0abf 659b fac8 cddb a56a d5c2 7996 f22e 4afd b78e e1f0
e43e 14a7 84b5 fbc2 52ae c3f7 0ca3 8159 21a6 311f 17cf 2cba 124a 09ba 9a51 4950
8ee9 65c3 2044 648d 51e7 de36 84b3 d645 06bc 4bcd 12ac ed29 98c1 8ef9 4bde f85b
9895 e6c5 4df1 77ec 5971 dfdf 71d9 ad35 03d3 efab 2017 3afc 1302 d4f0 4cb3 1121
d626 c566 a4b5 01fa 2178 bb29 d24c 2277 d2a9 008f b6a6 d7ba 5f6b 7bac 4416 1606
113d 8be3 3342 8fc1 dd82 70a4 8989 05c4 4dce e454 21c0 9b55 4c1e bb4b 52ae 2b15
34aa edb4 cb93 b342 d394 832d ec6f 0abf 5943 f27f c8e5 771a a9b7 6124 a78a a57f
3089 063c 21db 97c8 160a aa91 946c 1b74 9338 44e5 4d5e fcc1 9fd3 4605 2f84 e79c
3ae0 e2bf 1ac0 fd55 d6b2 7c55 88e7 d829 4814 bae1 9fc2 829e c07b b8b8 bc76 7284
40a6 f855 d887 5c49 4c19 6774 80be d703 f53e d4dc f665 5439 725a 9ca2 00fd 0230
e445 f7dc 8897 692a da32 efde 3d32 333d 2564 62d6 8254 9d6c 4692 80c0 d090 8c52
b9b0 32af 8f84 30e5 e614 048d eeae 1d50 4f49 4b3a e2ca 00e1 9367 77ff 6ad6 baa3
cfb3 82b5 7b90 db1d 48c1 3926 95f4 141f f84e 4369 8829 f210 e634 7054 612c 39bf
30a1 e26e a49f 50e6 613f f742 8f0e 2f88 9fdc 3fda 789d ba20 672d 9562 73f8 72ae
80e2 02c2 6a8d 1cb3 4b1a 6a10 8272 c90f 2887 277c 4cbb 995e 7834 4c93 0cb3 596c
1ee3 634e 0ce3 af81 6538 731c 2ba1 f857 cec4 49c2 f42b c152 f0f0 bf1b d544 18a5
8eed 0164 97e9 6001 780f 3047 a35b 2dd7 c378 c84f 04b6 d853 b532 31e1 7784 9d41
0747 e0e6 38e0 9379 502c 80b2 b361 fa97 8991 0893 0449 92e3 0711 c0d0 e80b 1f80
74ab 3e75 7402 1218 f34a de84 6a7c ee07 a03b f236 c0ab 1174 9ea6 a253 3a0b c2a3
b88a f456 3875 39f2 461a 9cec 797d ef2b cb71 3dd6 d461 24b6 a80d 37be 4191 9512
3331 500a ff58 b0cb e1f6 4c88 aa3e fb99 b7c0 938f 3f36 0dc6 cfde f3aa e8d3 0d0d
2926 8221 d538 8b18 8551 0ff1 de43 fa9b 57ac 24fa e2e5 747e 0806 3a48 88c3 f633
7892 9c94 db59 0c9f 9737 1458 d048 01a4 7901 cbb2 bfd1 67be eb08 ba40 8012 8f23
22c0 4bc5 b500 ee7d 1692 7ed4 00c9 3485 4835 fccd 505a 40e5 01ea ea52 6400 841a
7e5b 1cc4 ce8a f2ea eaf9 cf03 ec0d ab33 8e86 f7da c82b d577 3123 c947 a086 f1e8
7411 f06d e670 4f0e cf0d 2c9f 4277 052f e0cc b8c1 9824 f662 c17d 8b82 b376 d1ef
0537 e116 c089 ff74 2310 79ec cd52 514f 4135 6d38 999c 895d
```
### cipher_t2 (0x44C words BE; ROM 0xF0000+0x1130; valid i<=0x310)
```
8064 410a 5b27 4e08 926d ad06 dad8 5a9f 8f75 b05f 780b 5715 d71b 9083 1b43 1784
9e6d 8d79 b9f3 90ae 2ed2 b8c5 2397 eb1e 30c7 7bfe 3a00 b415 3df2 21a3 3d6f 7969
c6d9 0577 c481 0908 7cdf d615 f78c 3cba 1a4c 9cdd d7af d453 74c1 6505 3d5d 9285
35b7 e144 1801 b8a9 1556 a6f6 dae5 0d7a 4da9 bdc8 1487 a0bd 4b5d 6186 1cca 0af5
f70f 9096 503c 80e6 b059 5f61 aa14 cd19 ac4d c12d c286 51e0 a956 c816 1ab0 3334
1c40 a638 6407 311f 7a23 35f2 96f0 20cb 5f94 8adf 83f6 c2be 1d5b 9fee c978 4bcb
983f f137 49a5 a54d 8512 bb56 183a d233 8217 967c c3f3 8918 dc84 fd1c ab66 c4fa
f0bd 718d 04ae 6ddb 8c46 0b4f 187c f776 025f 2d46 70c4 c9bc a9cf a927 2f1c 6bdd
7893 34d1 94b7 d7bd ec81 cb52 1f7e 31af 2202 057d 4acb f632 4293 12e9 f767 ff67
067c 0545 1724 3d05 bffa c9cc 646e 0eb4 8faf f50a 685e f325 2576 e3bd a5da 8419
df66 76ef 3ea2 a640 ccbb 489b 651f a99f 7302 f0cc 031e 88db 57ef 99b8 cc05 194f
0b2f 5494 af7f 41ee d573 8d2d a608 1b40 1601 7638 da67 12ee 8c06 b061 b49b 4d67
8624 d8e1 ca1d b055 110b 9711 e4f7 282e 9578 4400 bfd9 e02e efc3 f955 01d6 62eb
b800 3a89 74df 7365 7060 a4d7 d483 2f83 7eb4 e646 2a90 01ce 8a20 732b 2e02 7d62
d6c9 f377 a4e5 25c0 120b aa9d 553c 4175 0dbb ed05 4db6 5705 e016 cf76 fa22 2d37
bcb3 a31d 9ebd 85ac 26e6 4977 2e69 3260 19da 0794 6b61 48da d39b 06fe ff1a 976a
0f5a fe85 d7e4 410e df38 1596 6478 b09b e856 6f3b 127a 71aa d5ee 66f2 8cb7 decb
c21b c65f 148f 32e4 e317 a37b 3eea d989 b1d5 6458 0b0b 3d40 9a6b c935 ddb9 a0cb
81b4 3932 4765 961a d9fc 3664 3393 c1e8 83f0 5fc4 f90e 5d43 a449 d052 e559 f661
5182 c8b7 d75e fd07 673a 3c30 1dfa 1237 c90e 896d 16ac e350 ce8b 64a2 9c4e 9384
a4d7 f90a 5902 7564 6dad ffe9 9aa7 d268 e52a 5760 d81c c610 be96 45a4 5772 65b0
7288 5cb7 8379 1616 75aa a571 269e bc15 c5ad fa0c 38f9 eb6d ed0d 8c38 632d 549b
dd12 2422 c6b1 38e6 4809 de3f 082f e26f fd21 9f94 87b7 9d66 4131 de7b 228b 9d8f
1428 f171 ab1c e843 7130 674a 68c3 33ad 29e0 741d be94 fdad ce0e c4b0 622e 49c8
4a4e 8d2e ff1a 3593 d48d 07cd e4f6 74fe c32a fe9a 6842 9550 050d c7c7 d0ce 1dfb
32b8 b4a9 9451 c986 efd0 7aae 4141 2641 650c 7866 5c85 25ed c1d7 ab42 ac51 d1d7
1b16 1877 a5db 8087 9d76 2b9b 3ff8 8c17 0a12 10ec 6e6f a202 d3dd d422 24d0 abc2
b762 b1f4 3417 09f8 852d 6f05 52d6 034f 3985 d210 5e62 d97d 564b b8fb dc5b 612a
821b 4f8c b35d c732 8f17 bb3b 772d 1003 d058 8614 f8ec 4baf 6fac e5e1 cd53 6fcf
2c24 d289 0fc6 9ecf 732b 4966 b890 47e8 ab32 7613 7575 e8be 0c79 637e babf cac2
3dd7 cb64 a406 c4d6 0ca6 0026 0a1c 3183 2ae3 d784 e86f 71c8 4950 a5b7 d889 357a
c156 fe6c afba 809a 740a 3336 a675 c994 e81e 9784 003c ac09 a59e db05 269f 7f57
b971 68ba 74cd 312d afae 593d bc98 0ce2 b2a0 1b85 626b fd15 7013 1f46 d358 cde1
eedf ab87 c039 e5fc a3a3 977d 6b39 e34a 85ea 2490 1bc5 8b44 db9c 6ad8 b86a cd91
fd46 dc14 4275 7896 aba4 ec1f 9027 a608 bb45 48bc a085 ced1 8f5f ee5d 0fc6 bac6
980e f6b0 0de9 7f1a ab6b 3b6f fdbb d4ab 57bf 14b7 edd8 05d8 af92 44b6 8497 961b
7f73 52fc e426 6946 2354 c823 100f f7b2 ccd1 3d76 ad4b 72b7 d7bf 3d97 b317 2c9e
22b1 f1c7 1ab5 4c3b 420c ce6c 82c1 bb53 f9ed 894d de13 3bf0 4477 835f 5a65 b3af
d742 0894 73d1 edf6 3720 e9ee 4b62 7a62 b45a 6198 5b7a baf4 0776 cc25 0455 0ed7
6177 f8ae 9901 f89f 58fa 43f9 7e80 06b3 4709 5e38 08ab 2660 d20a 584a 50ef 8c32
3a0a 1d80 edcc bfd3 b1e2 2037 34d2 0883 b329 f2cd 7f93 4816 aba5 195d 03b5 e6d3
8a24 da9a edcb 3ddb 2ad9 6b5e c256 0686 47b1 5a5d 0f90 71a1 90fb 7269 0c6b c1cb
005c b348 a16e 1088 bea7 2918 86c7 ad63 d29c 1e39 369e 9703 7fe2 6db6 3708 3910
9fc1 f210 ce4f 2559 506c bbc0 487e 0973 5fa4 02ca 25df 6d08 1ace 7100 7426 2dd1
fc3b 5cad 2d2f e96f 31be 210c 158f 5a84 8b05 2089 35cd 5393 1db3 cb3f fb1a 1eab
7f96 e92f ec0d e396 368a 37ff 16c5 c25c 38d2 0985 f932 e934 123f be5e 07d8 2c59
bd61 ef3e 5a89 6286 8e79 8507 158f fea6 0114 9146 4d43 eaef db08 7c17 1eea 1fcc
ac7d 3c4d b847 f2d2 71a1 4a66 9650 7fe5 051f 23ee 0fad 3b83 7abc 0e88 2722 abb5
42e8 53ed 4650 1e3f f217 03fe 9a4a d604 988c 5fb7 6da2 ebd9 b0db 2e7d 2b77 e5d4
7c0b d763 51ff fbd0 3be5 b82e a9ce 0ccd 3e6d 53a6 5a74 cc61 ea0a 5ee5 8bf7 5011
0c13 0336 0c06 f50a 1f34 b7c3 55c6 7741 3b67 b8d3 5dde 09ef c6e4 8bdf 5bca 6cb8
ee2c ca6b eab6 a3cc f619 4bbf bb67 3f58 964d e435 7c35 8094 b6f1 e513 6a1a afb8
1a1c 0183 d85c 5d94 fbe4 dfee fe7b 0cfd 889b 9a2f 8821 769e 8bb1 d397 0258 d948
0825 3ce6 5e4f 3fde 9057 f919 b6d6 6669 dee1 7348 938a 4a84 7aae b24a 7bc0 c99f
322b 5721 dd5b 33fb caa5 837c 86dd aa14 57c8 188c 7ef5 74d9 0160 3aa0 7ec1 1df8
74ed 14b6 f24c c36c bcc7 9e69 60e3 9e6e 805b 943a ab39 fc03 2b17 5c67 3869 5e99
3253 1606 3207 4067 d7b4 ae22 0b34 7178 faa3 4848 4f14 2ae3 d755 496e d36f 07a6
eb3a dec9 92bf ee1e d9dd b7d9 3a47 666d 5b2c 9570 c8b3 e18b bde7 77d4 a39d eafd
234a 1c04 5843 b2e3 6688 93b2 ee8d 3f34 c454 85df dc64 fd1a 898a 1329 4a3d d97f
c5f3 b428 b5b9 a930 d3fd 4c4d 0592 20d7 5af3 2e33 dbe2 5101 01d3 2ce6 8030 f86b
adc0 c85d 705e b034 b3be 50e7 cb88 f7b1 0935 64df 4e03 b199 1202 5b4e 4670 bb5a
9287 16b7 cc1d c735 80e1 0032 9b21 43de 36f6 5d24 87aa 21a4 9a21 e91d 4c62 c3b7
821c b0f1 a2c5 558f 17a1 8082 6da5 ecd9 d29c 391b 8cb6 8104 947b b7e6 08f7 7c89
859e 5f40 ea92 0464 58c9 68c0 e01c 3bff c8ce 7511 4397 29e8 1ddf 9648 9e6a 7f03
ad26 ca4b aae5 1fa8 9d9d c759 d2b1 476c 83e2 9aac d5a4 a874 4eea 8846 0960 03b9
b608 07a4 20cb a6eb ad50 5e0c d217 1061 53b1 14f9 24f0 4d47 0224 08a8 9411 c1ac
352a e1fb 6357 2066 390e 0571 9c21 81b7 086f f167 823c 65b8 2bb1 e0a4 8dac afb2
fec4 0a29 acbb 6ae1 67ae 51cd 4728 6575 0ab1 8689 65e7 e0f5 6506 cb30 8ece f3ca
1b47 c5e1 9a00 c186 bce0 f5e1 0fab 3bf1 0214 f5ee 8db8 aa84
```
### cipher_t3 (0x44C words BE; ROM 0xF0000+0x1838; valid i<=0x2AE)
```
d7b4 ae22 0b34 7178 faa3 4848 4f14 2ae3 d755 496e d36f 07a6 eb3a dec9 92bf ee1e
d9dd b7d9 3a47 666d 5b2c 9570 c8b3 e18b bde7 77d4 a39d eafd 234a 1c04 5843 b2e3
6688 93b2 ee8d 3f34 c454 85df dc64 fd1a 898a 1329 4a3d d97f c5f3 b428 b5b9 a930
d3fd 4c4d 0592 20d7 5af3 2e33 dbe2 5101 01d3 2ce6 8030 f86b adc0 c85d 705e b034
b3be 50e7 cb88 f7b1 0935 64df 4e03 b199 1202 5b4e 4670 bb5a 9287 16b7 cc1d c735
80e1 0032 9b21 43de 36f6 5d24 87aa 21a4 9a21 e91d 4c62 c3b7 821c b0f1 a2c5 558f
17a1 8082 6da5 ecd9 d29c 391b 8cb6 8104 947b b7e6 08f7 7c89 859e 5f40 ea92 0464
58c9 68c0 e01c 3bff c8ce 7511 4397 29e8 1ddf 9648 9e6a 7f03 ad26 ca4b aae5 1fa8
9d9d c759 d2b1 476c 83e2 9aac d5a4 a874 4eea 8846 0960 03b9 b608 07a4 20cb a6eb
ad50 5e0c d217 1061 53b1 14f9 24f0 4d47 0224 08a8 9411 c1ac 352a e1fb 6357 2066
390e 0571 9c21 81b7 086f f167 823c 65b8 2bb1 e0a4 8dac afb2 fec4 0a29 acbb 6ae1
67ae 51cd 4728 6575 0ab1 8689 65e7 e0f5 6506 cb30 8ece f3ca 1b47 c5e1 9a00 c186
bce0 f5e1 0fab 3bf1 0214 f5ee 8db8 aa84 d30a 8891 1baa ecf8 36ee d5df 2eec e3cb
6024 4810 98f9 c220 b20a 29fa 6b5d 1b8b fea6 1f07 fab2 ed7e ba19 5014 6a33 796a
2f79 4261 44e6 40cf 4e35 acf0 592c 9a94 33ee 7214 1112 7800 586d 5548 e587 cf0a
7b22 bd5f 27bf cc39 9e89 6ee1 a6fd f4a0 5460 c555 d9bd baa7 1bf9 946d da00 58d8
532e bb44 0e8c 7122 f8ec 6af1 5d4b 064c e3b1 b3b6 9035 7a37 01f1 d879 17a6 dfe2
c894 beca b023 cac0 7cce 723b 95e6 1e61 4934 11a8 7757 982f a6b4 c3b8 8fe4 22ef
b3c8 50b2 de31 fc5f 9741 52a0 ab7c a1b5 f82d 6d4c dec5 fbb5 3b8f 8060 9023 c516
9ed8 bc81 ef30 e475 bff3 c55c b904 8844 9630 1fd2 5315 fb72 1ec3 3f70 a58b e3b0
93e9 a7b2 06d3 b2ee 8f3f dc34 bb20 863f f4a9 296a 6233 db60 ac90 c388 9020 18ac
bf37 b061 f415 4187 d841 432d 55b4 ed1a 4592 dd92 c5f9 528e b14a a3d2 da99 2ae7
d3c6 e888 9e46 6bc3 719c f38a bd3d bc60 ac2e 7f00 54e7 f994 3a76 867e d7e0 4113
1444 6d8d 9754 52fe 4767 f1b2 209d d5f4 afbd 7e9f 965c 87aa 6881 bc10 d0c6 8738
0760 929a 96c7 4334 ffa8 7337 46e5 805c 9c80 8c4f 3d17 e462 a822 537d f974 6bbb
e74c 9825 7580 d033 b86b 0b19 8e2e c7f7 58c6 8b30 686b 7be6 67e2 175a b5f6 6182
f5c7 1c35 53ff 98c5 28e4 a4f0 001c 82de f2c2 6584 bb27 94c7 086c d1d4 01eb b23d
10be da2f 581b d94a 8ea0 5fd5 661c ceaa c223 3638 9238 a4c0 9602 70d6 75a2 d3c9
f08e 5335 1130 e937 f729 bc71 4a6a b14a 2fb5 f6f1 f785 2523 9fa4 a4e8 13fc f3dd
e6ee 1b20 438a 4d11 2453 3f7b b063 5ddc 90c4 2315 fbb3 d1d1 ecf7 3927 d878 0a8f
11ec 41ac e387 9ef6 f0d6 76fe 1ba5 c329 4847 b9d1 bed7 cfc0 65e1 6fc2 56dd 5c30
8626 a305 fe42 0bf0 59d4 7c5b d88f 470f 6a94 0b8a 0178 152a 0445 f038 ff23 916c
41cc 0045 cd16 1f3d fada 6c29 c46c 13ba e42a 36b2 376f dbfa bbaf e801 e460 4c79
8204 331c 867e 7592 b87e 97b5 90dd 4b80 ba7d 8536 cc77 0f13 e333 5729 502e d1dc
7de7 83f4 ea54 2adc bab9 2458 1485 71c3 71aa dd93 a664 9806 afe8 6110 af69 f653
6ba7 3b8d 7a9d eae9 5ffe d283 129c 4dae 5142 4bde e19f b4b2 2fed 14fb e35a b1b2
25b6 b929 1ea1 dbc1 bd78 0685 e867 7d2e 94df 56ca a14b 812d b554 1195 2b96 f89c
dde2 39ff 0465 85c8 750a 63f9 f8ae de02 bde6 4c63 a502 cfdf b970 0964 1968 6296
2a5f bfbb 5024 20f8 bd85 ebd5 0cb7 b373 19f6 d119 9003 9e3b 8e15 1bb1 5302 6680
2bbd 0799 e453 0f37 79a1 985f 3ddb 825a f2b6 120c 63e0 6960 f7a1 2382 cadf 0d86
30a6 f198 d5c1 a007 0074 96b7 39cf 6ba9 59c1 f382 97b6 db69 b835 228f b80a e05b
82e5 d3b7 2036 397a f00c 6013 0359 afcd 473d fbfc 4080 eef1 5633 12f0 c4f0 5ca0
dbc6 585b d0ba e0eb ae50 1f04 4f5f 5dde 6a69 68ab 4236 433f 3089 0cb0 2804 69d5
d0d1 81ed 9762 d4b8 4471 e4c9 4d0b da74 5ff4 79f6 c578 6e76 aab4 1689 1ace 782b
c81e 8a2a 87e6 63fe cea4 6f09 4ac7 54ad ffb1 1106 aa50 47c5 fb41 40e2 6fa2 f58f
ee30 fd87 f80f 21da 0708 6dda 0a50 ad8a c2bf b328 4b42 573b 5729 d4b6 baec 0c11
128b 06bb 2739 ef39 2bbb fa2b 4876 361e 8d20 0701 af05 5e0f a842 0984 8caa f373
92df d110 9f77 0963 2a32 dad7 8023 524e e888 296c 15c9 ae89 d494 db2b b52f ce78
501e 72ba d20a 72f2 e60d 388a 10c4 a9ea 823b 2361 b212 bd08 146b d46e 6eb6 2ca5
9cc1 3068 63b1 63b8 d3a3 e099 b1f4 3087 5d93 f942 8f95 3c34 f225 2c12 a8a3 b20a
a72c ca4e d84e b5b9 4b50 c1a7 c614 5834 2bbb e5b9 9e6c fe94 73d1 4c9a d389 6ec9
1bf2 1a45 dd98 c7cc dd09 eaf8 085d 8203 c9d8 6aa3 8593 87b1 ff11 768f 4abb d190
32bf b22b c82e b658 55af e41b ba64 0277 8c4c 6db5 15cc 121f e2f4 8c8b 12a4 e626
d53f e04b 4ba7 392f d3a9 bf0d 2a70 eed9 c0fc 48c1 7fd8 9bc4 e76e 1681 60f5 525d
ab81 ddc2 b9b5 845c d262 e14d b61c 4aa2 cc0d a908 2a11 c9e9 49e9 1da4 d8c7 42db
f6cc 3894 9543 fff3 8732 864e 19bf c184 f335 4c75 2f1d 9bd3 7044 d00f bc4b f0c7
0281 5f36 e3ac 09ec f16a 7c5f ab3d 67bd ae95 97f4 d3f3 9beb 110f 4deb b8ba f369
36ba 036e 4770 4d96 0268 0ba3 912e 8ae7 87ce 5c7f 3fac 8b96 d81d 1905 f16c f4aa
9c1f 30ad d9a3 3801 4693 3df7 4afd 9f00 8c54 c02d 3e6e 931a fa89 6d86 fc97 0333
f730 7765 d25f 7ef6 40a2 15a9 932d 03d8 d188 25a3 197f 329d a041 ae79 6046 c445
0550 5847 1747 4c9f abfd 6628 4070 d8c0 e641 8d0c 591b a8b5 f8bd 57fc f056 28a1
cfe1 eaa4 e976 8dbb e9ae 9e94 14b5 d8eb 33ce 2f57 5b15 f10f e41b 463c d693 48c9
ba21 d9d8 c012 8a9c b8f7 1328 285a a8fa 1b17 f8e6 7941 d553 5117 cac6 2f8a b563
fa80 9e19 fb33 b554 6589 ae21 7060 cc7c 33df 3456 14c8 feb3 13a7 0ac8 3cb8 e71d
6482 b886 6614 9378 91cf 0a41 220e af09 8e7d 656c f18e 48be 9042 9c4d ed82 3412
1572 0f30 a8a5 779d 5c7a becc 4cbf 3d2a 7463 88ad b205 b9a6 98de 8533 f507 4a1f
5c01 8f38 328f 854b 2a37 c35f 63a7 f355 d777 066b 089d 655c de27 c13a 3394 7298
dd7c 6d96 8e63 9642 b40e d9e2 7636 e23d 634d 90ed 9199 bb6e 54b3 65d5 592c 4a01
48b2 4416 c6f8 4a31 b9a7 9d3b 29d4 b85c 3cd3 65f1 1e38 7945
```
### cipher_t4 (0x44C words BE; ROM 0xF0000+0x1F40; valid i<=0x2BD)
```
f16a 7c5f ab3d 67bd ae95 97f4 d3f3 9beb 110f 4deb b8ba f369 36ba 036e 4770 4d96
0268 0ba3 912e 8ae7 87ce 5c7f 3fac 8b96 d81d 1905 f16c f4aa 9c1f 30ad d9a3 3801
4693 3df7 4afd 9f00 8c54 c02d 3e6e 931a fa89 6d86 fc97 0333 f730 7765 d25f 7ef6
40a2 15a9 932d 03d8 d188 25a3 197f 329d a041 ae79 6046 c445 0550 5847 1747 4c9f
abfd 6628 4070 d8c0 e641 8d0c 591b a8b5 f8bd 57fc f056 28a1 cfe1 eaa4 e976 8dbb
e9ae 9e94 14b5 d8eb 33ce 2f57 5b15 f10f e41b 463c d693 48c9 ba21 d9d8 c012 8a9c
b8f7 1328 285a a8fa 1b17 f8e6 7941 d553 5117 cac6 2f8a b563 fa80 9e19 fb33 b554
6589 ae21 7060 cc7c 33df 3456 14c8 feb3 13a7 0ac8 3cb8 e71d 6482 b886 6614 9378
91cf 0a41 220e af09 8e7d 656c f18e 48be 9042 9c4d ed82 3412 1572 0f30 a8a5 779d
5c7a becc 4cbf 3d2a 7463 88ad b205 b9a6 98de 8533 f507 4a1f 5c01 8f38 328f 854b
2a37 c35f 63a7 f355 d777 066b 089d 655c de27 c13a 3394 7298 dd7c 6d96 8e63 9642
b40e d9e2 7636 e23d 634d 90ed 9199 bb6e 54b3 65d5 592c 4a01 48b2 4416 c6f8 4a31
b9a7 9d3b 29d4 b85c 3cd3 65f1 1e38 7945 2baa c64a b3a0 9ffc ee78 5aea 4a18 690e
ed06 9935 5e95 d992 30b7 b87e d003 44f8 6039 2ac6 f0a2 6986 ed04 54cc 7aaa 4e44
6431 5c79 d082 4442 629e 029a b07f 914d 3b86 1388 b7ec 7676 e978 55a2 223f 57d5
dfa8 bd56 c750 298b 065d e85d 8f4a ae98 ae8a 3060 4dd2 f839 a7f1 35f0 0cf9 4924
654d 4e02 12c4 b8cc e56a 6e29 ad96 7aa0 9312 c1cd a2c6 1352 dc71 60ab 9e35 38fd
3f30 9276 e65e 6a93 6f0d 8b17 eeba b384 66d2 0e08 995d 32a0 6bdf 9574 bdd9 05d3
f172 29cd 4995 af2d 96cd 944b 1a54 0b88 c95d 54aa db35 bbdd 4a90 f4b5 96a8 40f2
5ecb 6152 246c 3c0e 2aae 4770 ce87 167f 1afd bcaf 9388 f376 8df1 c9f2 3d98 2d70
fe18 4af1 b257 7bbe 5ed4 db97 933e be93 d236 9f3a 6c45 d6fe c5e3 233b 4065 b7aa
ebfa e4d8 f628 3826 03c8 6f40 6a6f f01f 64c8 ba58 c103 03b7 b0f4 801f 73d5 e13e
e582 6389 3dca 3d67 9f40 86d1 0de1 025b d9e5 73c7 dd8d 9bc8 0291 2e97 4a52 9a87
098b 978c 83c5 4bf4 698b f052 9513 6327 b546 541f 8a37 6f50 87a2 bf3a 8fa3 9acb
5eb7 6e5a 6897 4be0 9d4b 89d4 4454 f7e5 1636 c28f aeca 4c18 0c95 5105 ea8a d121
61db c7eb d444 1cc7 1687 5e67 2a7a 12d5 7933 4165 2f1b 554a b42e 570e c873 e9e2
898a 185c 60ad d944 6ba8 f688 69dd c5ab 69a1 0f5b dc44 248b c958 f484 c249 816e
4b10 b469 7ef1 83d4 8c32 988e a5b2 aa3f 017d e4fe 170c b79d 3beb 937e c18c 5e3f
587d 466e f612 74ee e74c 5015 e6f9 878d 1f4a 7f66 f975 492e da95 4b33 9c61 8116
4c88 888c 1b23 bfa3 0ba9 6611 3d2d a692 23d1 5b36 fd41 57b4 0b83 e1fb bd9c 757b
53a1 3de7 bf68 3326 ca1c cb5c bd5c 3c89 a950 6e73 dd5f 2083 5995 64d1 4537 fc91
6879 b148 f918 8e59 3f9f 8613 ca68 d536 e635 cdd4 8310 1ab6 4dbf d2ab becd e07c
960c 3e94 b475 ffd5 7ffa 0c18 19d5 8bc1 b724 05e8 1ef3 179f a8dc 3dbb b6b1 1da4
9d66 50f5 73e3 7dbc 5c57 3f86 e226 7f15 7706 ee92 a66a c10b 66f2 f74c 4e8a 9065
ddf5 c02c a51a 1c38 540c ef85 5fac 41ba 4fd9 e2f2 5f44 9bc9 e789 810d 15ed a550
18c6 888c 1856 bb99 57d3 bc2d 6e20 e412 91f8 184e bbaa 6e2b bbe9 b488 6092 f565
b874 cfc4 b60b 9376 61ab 2a60 7053 ead4 9749 6610 f713 d0e8 a291 a72f 0958 1f50
27e9 0fa8 4336 0ba2 27cf 4624 6ae4 d868 1a96 9556 598e d5f2 8d29 2d5b d4fb c33f
1c4c 7d18 7c9c 2020 1267 f9be ecb1 0768 11c0 3b24 ba4c d0d0 5314 81d3 3ec3 643a
0f61 0495 0195 6580 fe72 01bf 3a86 7981 4c51 2966 8c31 f8e6 8b84 f8d2 abb5 c0d1
6f0c 7f0f 7260 73df 9bea 2383 f804 d3ab 35e4 8a03 740f f475 c6e3 1907 4355 687d
6fd8 8f63 fa32 f220 308f edb3 6eec 70c1 1de2 65e3 4820 552e b580 f5a9 5eeb 2af7
fd78 b8b5 6ecb 1998 4a44 af56 480f c308 ad79 617f 30d1 31dc d3ff 544b e6d3 466d
1012 ec0f cf0c 4392 0536 be54 2c39 a01e 8a23 a540 5e26 2829 10dd c415 a708 6dfd
3043 658f ede0 3104 e4e1 01fe 14be 1d69 cd61 5a01 0ce6 7f34 1e44 b4fd 8b4c 8d18
dd3b 314e c0ff 2f60 7ff4 b225 c957 4a72 00d1 076d f7ff 2dce b97e 1144 5c9e d7ea
0e3e 6087 0f64 5f3a b749 e5b4 3e18 e253 12b5 14c1 ebbc a858 ff24 30a4 7bd3 13e4
d24e f033 9899 187f 3875 0116 70f2 a2f6 38f3 95df 9093 f9c6 ae81 a450 fceb b08c
1047 f331 a145 102f 57f4 3af1 f530 8d00 bfbd 4411 585d 0507 0ea5 61a4 fd02 cd59
1808 7331 c453 83f2 b315 18ae 9a4b d1f4 8f5c 3898 0178 7679 885f ddef e66f 732f
44ca fc24 c217 8c56 5df4 28a3 0d72 3b4d adfa f436 b26a 85ca b4b1 53fa 9085 8ba4
efd2 29ff 551c ee68 3e02 8e03 dd84 3e7c 012a d4e4 51b4 759d a014 22d5 5639 d6c8
f0bd ab43 3449 81c9 fdfa bcdc 0a25 c3fc 91ec d71a de1a e547 141a bf04 5fb9 2796
97d8 2e7a 134e 5bed 8611 4fcf acac 9415 0a66 c33b ff24 9e30 2d7f 0cda ff81 1f43
c369 4f1f e6fe d196 cd51 d7ea 77c5 5af6 7ca5 f32c 7255 ed13 1884 4310 dee2 169f
b0ff f692 1368 e38c 0a15 d9f3 9c25 b934 d753 945a ff5e e910 5956 c731 19f2 5c7a
6c90 4576 796f b55f 000f 2668 000f 2c6a 000f 326c 000f 386e 000f 3e70 000f 4472
000f 4a74 000f 5076 0003 ffff eea0 0000 2492 0000 1b00 ffff d7c0 0000 0f1b 0000
2e00 ffff edb0 0000 0d35 0000 2f00 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
```
### psxdelta_t0 (0x44C words: hi=dx lo=dy, signed bytes; from PSX ENGLISH.EXE)
```
fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34
fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34
fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34
fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34
fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34 fb34
fc2f fe2a fe27 ff26 ff26 ff26 ff26 ff26 ff26 ff26 ff26 ff26 ff26 ff26 ff26 ff26
ff26 ff26 ff26 ff26 ff26 ff26 ff26 0034 fb34 f734 f134 0525 0025 fc25 f225 0333
0333 0333 0333 0333 0333 0333 0333 0333 0333 0333 0333 0333 0333 0333 0333 0333
fe3a f53c ea3e ea3e ea3e ea3e ea3e ea3e ea3e ea3e ea3e ea3e ea3e ea3e ea3e ea3e
ea3e ea3e ea3e ea3e ea3e ea3e ea3e ea3e ea3e ea3e 0932 0d32 1534 1534 1534 1534
1534 1534 1534 1534 1534 1534 1534 1534 1534 1534 1534 1534 1534 1534 1534 1534
1534 1534 1534 1534 0208 0208 0208 0208 0208 0208 0208 0208 fc30 fc30 002a fe2f
fd33 fa37 fa3e f94c fd39 0036 f135 f135 f135 f135 f836 0038 0038 0038 0038 f638
c43c c43c c43c c43c c43c e43e f53a 0036 0036 be3e be3e b53b a738 a738 a036 9037
9037 0038 0038 0038 0038 0038 0038 0038 0038 0038 0038 0038 0038 0038 0038 0038
0038 f739 f739 e03e e03e 0c3b 0c3b 0c3b 0c3b 0c3b 0c3b 0c3b 0638 0137 0137 0137
fa37 f137 f137 f137 f137 f137 0237 0d36 0d36 0d36 0d36 0d36 0d36 0237 ee3a ee3a
ee3a ee3a f937 f937 f937 0829 0129 df2b df2b f12b fb28 fb28 0c29 0c29 0c29 0130
d547 d547 d547 d547 d547 d547 d53d dc36 e732 f72d 002a 0429 0429 0429 0429 0429
f82e d23c d23c d23c d23c eb31 fb2a 0027 0027 0027 0027 0027 0027 0027 de2a de2a
de2a de2a de2a de2a de2a de2a f92a f92a 0427 0427 0805 0805 0805 0805 0805 0805
0805 0805 0805 0805 0805 0805 0805 0805 0805 0805 0805 0805 f801 f801 f801 0509
0509 0509 0509 0509 0509 0509 0509 0509 0509 0509 0509 0509 0509 0509 0509 0509
0509 0509 0509 0509 0509 0509 0509 0509 0509 0509 0509 0509 0509 0430 0430 0430
0430 0430 0430 0430 0430 0430 0430 0529 0529 0529 0529 0529 0529 0529 0529 0529
0529 0736 0736 0736 0736 0736 0736 0736 0736 0736 0736 0736 0736 0736 0736 1737
1737 1737 0637 0637 0637 0637 0637 0637 0637 0637 0637 06fe 06fe 06fe 06fe 06fe
06fe 00fd 1005 04f2 04f2 0f25 0f25 0f25 0f25 0f25 0f25 0f25 0f25 0f25 0f25 0f25
0f25 0f25 230c 230c 230c 230c 28fd 28fd 022e 022e 022e 052c 0a26 0a26 1a17 1a12
2212 2712 2712 2f12 2f12 2f12 f834 f834 f834 f834 f834 f834 f834 f834 f834 f834
f834 f834 f834 f834 f834 f834 f834 f834 002f 002f 002f 002f 002f 002f 002f 002f
002f 002f 002f 002f 002f 002f 002f f333 f333 dc3f dc3f dc3f dc3f f938 f938 f938
f938 0625 0625 0625 0625 0625 0625 0625 0625 0625 0625 0625 0625 0625 0625 0625
0625 0625 0625 0625 0625 0625 0625 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 00e7 fe3b fe3b fe3b fe3b fe3b fe3b
fe3b fe3b fe3b fe3b fe3b fe3b fe3b fe3b fe3b fe3b fe3b fe3b 083b 083b 083b 083b
083b 244f 244f 244f 0603 0603 0603 0603 0603 0603 0603 0603 3802 0530 0530 0530
0530 0530 0530 0530 0530 0530 0530 0530 e732 e732 e732 e732 ef32 f831 f831 ff31
ff31 ff31 ff31 ff31 ff31 ff31 ff31 ff31 ff31 010d 010d 010d 010d 010d 010d 0256
0231 0231 0231 0231 0231 0231 0231 0231 0231 0231 0231 0231 0231 0231 0231 0231
0231 0231 0231 0231 0231 0231 0231 0231 0231 0231 0231 0231 0231 0231 0231 0231
0231 0231 fb31 f431 f431 f431 f431 f431 f431 f431 f431 f431 f431 f431 f431 f431
f431 f431 f431 f431 f431 f431 f431 f431 f431 fe35 fe35 fe35 0507 0507 0507 0507
0507 0507 0507 0507 0507 0507 0507 0507 0507 0507 0507 0507 0507 0507 0507 0507
0507 092e 092e 092e 002f fa2c fa2c fa2c fa2c 0031 0031 0433 0433 0433 0433 0d24
0d24 0229 0229 e93b 0307 0307 050d 050d 050d 050d 050d 050d 0700 0700 0700 1b4f
0406 0406 0406 0406 0406 0730 0730 0730 0730 0730 0730 0730 0730 0730 0730 0730
0730 0730 0730 0730 0730 f92a f92a f92a f92a f92a f92a f92a 1e35 1816 150d 150d
150d 150d 150d 150d 150d 150d 150d 150d 150d fd07 fd07 fd07 f708 f505 f505 f505
f5fe f5fe f518 f210 f210 f210 d91e e127 e127 e127 ec07 f107 f107 f107 f107 f107
f107 f107 ec09 ec09 2e3d 2e2a 2c20 2c1a 2c1a 2622 2324 1b1f 1b1f 1b1f 1b1f 1b1f
1d18 2018 3012 2812 2812 2812 2812 2812 2311 1c12 1916 0f1e 0224 fb26 fa28 fa28
f62e fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36
fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36 fe36
fe36 fe36 fe36 fe36 fe36 fe36 0531 0531 0531 0531 0531 0531 0531 0531 0531 0531
0531 0531 0531 0531 0531 ff2d fb2d fb2d fb2d fb2d fb2d fb2d fb2d fb2d fb2d fb2d
fb2d fb2d fb2d fb2d 032f 032f 032f 032f 032f 032f 032f 032f 032f 032f 032f 032f
032f 032f 032f 032f 032f 032f 032f 032f 1023 101b 1013 1013 140f 190c 190c 190c
190c 190c 190c 190c 190c 190c 190c 190c 190c 140b fb36 fb36 fb36 fb36 fb36 fb36
0432 0432 0432 0432 0432 0432 0432 0432 0432 0432 0432 0432 0432 0432 1506 1506
1506 1506 1506 1506 1506 1506 1506 1506 1506 1506 0117 0117 0117 0117 013f 013f
0134 0134 fc34 fa37 fa37 fa37 fa37 fa37 fa37 fa37 fa37 fa37 fa37 fa37 fa37 fa37
fa37 fa37 fa37 fa37 0036 0036 f836 e33d e33d dd3d dd3d dd3d d93d d23a c939 c239
c238 fd37 fd37 f937 f737 f437 f437 f437 f937 fd37 fd37 fd37 fd37 fd37 fd37 0436
0435 0435 0435 fe30 f727 f727 f727 f727 f729 fb2b fe2d fe2e fe30 fe22 fd23 f926
f928 fd23 fd37 fd37 fd37 0436 0435 0435 0435 fe30 f727 f727 f727 f727 f729 fb2b
fe2d fe2e fe30 fe22 fd23 f926 f928 fd23 fb7f 7eed 1851 5676 fff2 fffb 6023 818e
7eef ffa2 4100 0892 d8bf ee3d 4c14 88bb efed bf17 be80 50c7 7ebf fb10 f518 11fd
bfdf 77b5 fd00 31c3 96ff f7f9 02c4 0055 a7ff ff7d 0920 20c8
```
### psxdelta_t1 (0x44C words: hi=dx lo=dy, signed bytes; from PSX ENGLISH.EXE)
```
ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff3c
ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff3c ff36 ff36
ff36 ff36 ff36 ff36 ff36 ff36 ff36 ff36 ff36 ff36 ff36 ff36 ff36 ff36 ff36 ff36
ff36 ff36 ff36 ff36 ff36 ff36 ff36 ff36 ff36 ff36 ff36 ff36 ff36 ff30 ff22 ff22
ff22 ff22 ff22 ff22 ff22 ff22 ff22 ff22 ff22 ff22 ff22 ff22 ff22 ff22 ff22 ff22
ff22 ff22 ff22 ff3f ff3f ff3f ff3f ff3f ff3f ff3f ff3f ff3f ff3f ff3f ff3f ff3f
ff3f ff3f ff3f ff3f ff3f fb3f fb3f fb3f fb3f ff26 fd28 fa29 fa29 fc3e f83c f83c
f83c f83c f83c f83c f83c fb39 fb39 fb39 fb39 fb39 fc3c f83c f83c ed41 ed41 ed41
ed41 ed41 ed41 ed41 ed41 ed41 ed41 ed41 ed41 ed41 ed41 ed41 ed41 ed41 ed41 ed41
ed41 ed41 ed41 073f 0c3e 1240 153f 153f 153f 153f 153f 153f 153f 153f 153f 153f
153f 153f 153f 153f 153f 153f 153f 153f 153f 153f 153f 153f fd3b 0235 002d 0112
0112 0112 0112 0112 f712 f314 f314 fd4a fd4a fd3d fd38 fd36 f741 f741 f741 f741
f741 f741 f741 f741 f741 f741 f741 f741 f741 f741 f741 f741 f741 0042 fe45 f444
ea47 e746 de45 de45 de45 da45 cd47 ca47 c247 c240 fc3e fc3e fa3e f73f f541 fe40
fd41 f441 ee41 eb40 ed3f f13f f13f f73f f73f fa3c fe3e 063e 043e fa42 f946 f948
f948 f948 f948 f943 f943 fc3e ff3e f93f f63f f540 0140 0244 0244 0045 fe43 f93d
f030 ed2f eb2f ec32 f136 f638 fa3a fa3a fc3d f72c f62e f62f f62f f62f fd2a f92a
f42c f033 f135 f042 f348 f348 f348 f53f fa3b fa3b fa37 fa33 fa2e fe2b fd28 fd28
fb28 f728 f328 f328 f928 fe26 fe26 fe26 fe26 fe26 fe26 fe26 fe26 fe26 fe26 fe26
fe26 fe26 fe26 fe26 fe26 fe26 fe26 fe26 f92e f430 f930 f62e ef23 e322 d722 d423
d724 da2a e32b e62b f22c ed30 ea2e e62b 0027 f212 eb11 e60f e40e e40e e40e e40e
ea12 ea12 f515 f515 f717 f717 f717 f717 f717 f20f f00e ee11 ea16 ea16 ea16 ea16
f314 f114 fa14 0617 0617 fa0e fa0e fa0e fa0e fa0e fa0e f808 f708 f517 ff1a 091a
091a 0a10 0a05 1203 1203 1203 1203 0905 0905 0607 0207 0207 fe3d fe3d fe3d fe3d
fe3d fe3d fe3d fe3d fe3d fe3d fe3d fe3d fe3d fe3d fe3d fe3d fe3d fe3d fe3d fe3d
fe3d ff2a ff2a ff2a ff2a ff2a ff2a ff2a ff2a ff2a ff2a 003f 0140 0140 0140 0140
0140 0140 0140 0140 0140 0140 0140 0140 0140 0140 0a2e 1527 1527 1527 1527 1527
1527 1527 0a26 0827 0632 023a ff33 0932 0932 0932 0932 0932 0932 022c 002d 002d
002d 002d 0019 0e15 0e15 0e15 0e15 1611 1f12 1f12 1f12 281a 281a 281a ff37 ff37
042b 042b 042b 022d 0032 fe38 fe38 fe3b 0554 176b 1e70 2073 226f 2871 2e69 2c62
2d4b 2d32 2c1d 2c1d 2c1d 2c1d 2c1d 2c1d 2c1d 2c1d f111 fa3f fa3f fd48 fc53 f75f
f654 f748 f729 f718 f718 f718 ee18 ee15 ee15 ee15 ee15 ee15 ee15 ee15 f516 f718
f718 f718 f718 f718 f718 f718 f718 f718 fa23 f72a f12e ea35 e237 de3a e33c fe3d
fb42 fb42 f53f f63e f63e f63e f63e fb42 0015 0015 0015 0015 0015 0015 f6f8 0d26
0d26 0d26 0d26 0d26 0d26 0d26 0d26 0d26 0d26 1a2f 1a2f 1a2f 1a2f 1a2f 2135 013c
013c fb3e f83e f53f f540 f640 f440 f240 ef40 ef40 ef40 ef40 ef40 f740 fb40 fe40
fe40 fe40 fe40 f93c f53c e740 dd43 dc4a dc4a e343 e343 eb3d f13f f13f fd3f fd3f
f20a f60a f60a f60a f60a ef04 ea00 f004 f004 f004 f004 f004 f004 f004 f004 f004
f004 0039 1336 1a2e 0536 e143 0002 0002 ff02 ff02 2243 1643 0742 0742 b947 a950
ad58 02fd 02fd 02fe f600 f600 f600 f600 f600 ff00 ff00 031f 0b17 1216 111e 043a
0051 fc59 fc50 fb4e fd4a f843 f843 fb43 ff41 ff41 043d 0a37 1437 2a33 312c 312c
3d31 3d31 4133 4133 4133 4133 4133 4133 fb3f fb3f f93f f83f f83f f83f f83f f83f
f83f f83f f83f f83f ec3c ec3c ec3c ec3c ec3c ec3c ec3c fb3c 003a 043d 043d 043d
093b 093b 093b f43c f43c fa3c 003e 003e 053f 1547 1547 1547 1547 1547 1547 1547
1547 1547 f442 0945 0945 fd45 fd45 fd45 fd45 fd45 fd45 fd45 fd45 0340 0a42 fd45
fd45 fd45 fd45 fb3f f13a f13a f13a f13a f13a f13a fa3e fc40 0141 0141 0141 fc3e
f239 f239 f239 f239 f239 f43b f93c fa3e fd3d fd3d fd3d fd3d fd3d fd3d fd3d fd3d
fd3d 0044 0044 0044 0044 0044 0044 0044 0044 0044 fe40 fe40 0039 0039 0039 0039
0039 0039 0039 0039 ed40 df51 ef08 ef08 ed09 e90d e90d 003b 0330 071e 0b11 0c0a
0c0a 0c0a 0c0a 0c0a 0c0a 0044 0044 053d 093d 093d 093d 093d 093d 093d e902 dffa
dfef f5f3 f5e3 231a 231a 231a 231a 231a 231a 231a 131a 091a 051a 051a fe1a fb1a
f51a f31a f31a f31a f31a f012 2918 2918 2918 2918 2c0b 2c0b 0608 0608 010e 010e
010e 010e 010e 010e 010e ea0e fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40
fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40
fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40 fe40 f93b ef37 ef37 ef37
ef37 ef37 ef37 f73a f73a f73a fe40 ff27 ff27 ff27 ff27 022c 022c 022c 022c 022c
022c 022c 022c 022c 022c 022c 022c 022c 022c 022c 022c 022c 022c 022c fd2b fd2a
fd2a fd2a fd2a fd2a fd2a fd2a fd2a fd2a fd2a fd2a fd2a fd2a fd2a 003d ff3f ff3f
ff3f ff3f ff3f ff3f ff3f ff3f ff3f ff3f ff3f ff3f ff3f ff3f ff3f ff3f ff3f ff3f
ff3f fa40 e942 e942 e942 e942 e942 e942 e942 e942 e942 e942 e942 e942 e942 ff3f
ff46 ff46 0346 0346 0346 0346 0346 0346 0346 0040 0040 0030 0429 0422 0422 0422
0422 0422 0422 0422 0422 0422 0422 0422 0429 0430 0430 0430 0430 0430 0430 0430
0430 0430 0430 0430 0c22 0c22 0c22 0c22 081d 0518 0518 0518 0518 0518 0518 0518
0518 0518 0518 0518 0518 0518 0518 0518 0518 0518 0518 0518 0518 0518 0518 0518
ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d
ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d f821 f821 f821
f821 f821 f821 f821 f821 f821 f821 f821 f821 f821 f821 f821 f821 f821 f821 f821
f821 f821 f821 fa2b fa2b fa2b fa2b fa2b fa2b fa2b fa2b fa2b fa2b fa2b fa2b fa2b
fa2b fa2b fa2b fa2b fa2b 022e ff31 f931 f531 f523 f523 f523
```
### psxdelta_t2 (0x44C words: hi=dx lo=dy, signed bytes; from PSX ENGLISH.EXE)
```
ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d
ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d ff2d f821
f821 f821 f821 f821 f821 f821 f821 f821 f821 f821 f821 f821 f821 f821 f821 f821
f821 f821 f821 f821 f821 fa2b fa2b fa2b fa2b fa2b fa2b fa2b fa2b fa2b fa2b fa2b
fa2b fa2b fa2b fa2b fa2b fa2b fa2b 022e ff31 f931 f531 f523 f523 f523 f523 0731
0731 fb33 fb33 fb33 fb33 fb33 fb33 fb33 fb33 fb33 fb33 fb33 fb33 fb33 fb33 fb33
fb33 0530 0c2d 162d 162d 162d 162d 162d 162d 162d 162d 162d 162d 162d 162d 162d
162d 162d 162d ff2a ff23 fe1f f546 f835 fc29 fd27 0029 fb2e 04eb 04eb 04eb 04eb
04eb 04eb 04eb 04eb 04eb 04eb 04eb f22f ea2d e42f e42f eb2f f132 ee32 e930 e331
e331 eb34 f234 ef34 ee36 ee36 ee36 ee36 ee36 f338 fa36 f438 f438 f438 f438 f438
f438 ec33 de31 d62f d52f d52f d52f e130 f22c 012d 012d f630 f532 ed39 e53c dc43
ed48 ee48 f444 f33c fb33 fb33 0e2f 132c ff2c ec1f e91e e41d e31e ea1e ed20 ed20
e31f e11f e11f ea1f ef1f fe21 111f 1b11 1e0e 1315 1314 1314 0c19 071f 0027 f926
f621 fc2f fa2c ee26 ee20 ee17 ef10 eb10 eb10 eb10 ed0b ed0b ed0b f322 f322 f322
e82a cb48 c64e c64e c64e cf40 cf40 df34 e72d ee28 ee25 f321 f321 e625 b73a b441
b441 b23b ae30 ae30 b82e bf2b cd29 d726 e523 190b 190b 190b 190b 190b 190b 190b
190b 190b f40f f40f f40f f40f fa0a fa0a fa0a fa0a fa0a fa0a fa0a fa0a fa0a f711
fa01 0203 0708 0708 0708 0708 0708 0708 0708 0708 0708 f80b f309 f309 f309 f309
fa06 0401 0401 0401 fc04 f50e f50e f50e f50e f50e fd2f fd2f fd2f fd2f fd2f fd2f
fd2f fd2f fd2f fd2f fd2f f020 f020 f020 f020 f020 f020 f020 f020 f020 f020 022b
092b 092b 092b 092b 092b 092b 092b 032c 002c fc2e fc2e 002d 0e25 0e25 0e25 0e25
0e25 0e25 0e25 0628 0628 022b ff2e f61f ff23 fd26 fd26 fd26 fd26 f728 f628 f226
f226 ef22 ef22 fa09 fb10 fb2f fe33 060b 1009 1304 1304 1304 0a1d 0d14 1013 1407
100a 1306 050d 050d 050d 050d f533 ec32 ee36 f33b f83d fa34 0024 040f 040b 040b
040b 040b fb2d ec27 e21f e21f ee25 f625 f625 f625 f72a f72a f52d f52d f52d f52d
f52d f52d f52d f52d f52d f52d f52d f52d f52d f52d f52d f52d f52d f52d f52d f52d
f52d 0332 1330 1b2f 1b32 1833 1833 1332 082e 022b 012a 002b fe2c 002d 1106 1106
1106 1106 1106 0a06 0a06 0a06 0a06 0a06 0a06 0a06 0a06 0a06 0a06 0a06 0a06 0a06
0a06 0a06 0a06 0107 fe07 fc07 f704 f6fb f7ec fbe9 fbe9 ea23 e924 d826 c72a c72a
c72a c72a c72a c72a c72a c42a b335 a539 a539 9f37 9f37 9947 8e51 8f67 8f67 8f67
8f67 8f67 8f67 8f67 8f67 8f67 956d 9d75 a578 a77b ae78 b372 b372 b667 bc5b bb45
bc32 c430 eb26 f130 e12c b234 b346 b846 a846 9745 9649 9649 9649 9649 9649 9654
9654 8d4c 974c a649 ca4b e04c e946 eb54 fc07 fe32 fe32 f934 d43c d43c e839 ee39
ee39 ee39 f535 ff35 ff35 ff35 ff35 ff35 ff35 ff35 ff35 ff35 ff35 ff35 102a 0039
f446 f251 e451 e451 de52 de52 d74d d74d e24b e24b e445 ea47 ea4c e84d e24e e24e
e552 e552 e552 e552 fc32 fc32 fc32 fc32 fc32 fc32 fc32 fc32 fc32 f636 e742 f306
f306 f306 002c 0019 fd05 fd05 fd05 fd05 fd2b fb28 fc22 fc22 fc22 fc22 fc22 fd27
fb2b fb2d fb2d fb2d fb2d fb2d fb2d fb2d fb2d fb2d 1103 0fff 13f9 16ed 16dd 12d1
0d08 0d08 0d08 0d08 0d08 0d08 0507 0507 0507 0507 000a fe0b fe0b fb0e fb0e fa10
fa12 fa14 fa14 fa14 fa14 fd1e fb26 fa2c f92f f92f 0a0b 0a0b 0a0b 0a0b 0a0b 0720
0aee 16f4 1b03 1409 0f0d 1b39 0139 f62c f62d f62d f92f f92f f92f f52f f02f ed2f
ed2f ed2f ed2f ed2f f633 f633 fb33 fb33 fb33 fb33 fb33 f534 ef34 ef34 e834 e834
e834 e834 e834 e834 e834 e02f e02f ee30 ee30 ee2b ee2b ee2b ea29 ea29 ee2d f331
f331 f331 f331 f331 f331 f331 f331 f331 f331 f331 f331 f331 f331 0035 0336 1437
1737 1c37 2136 2436 2730 2430 2430 1f30 222b 1f2b 1b2b 1b2b 0f2b 0a2f 0a2f 0a2f
0a2f 1030 1030 142c 142c 1d22 221f 1f17 1b12 180a 180a 180a 180a 1a12 1e17 2219
261c 2c27 3823 481e 5115 540b 5807 4e04 4e04 4e04 4e04 4e04 4e04 4e04 4e04 4e04
4e04 4e04 4e04 4e04 4e04 4e04 4e04 4e04 4e04 002e 002e 062e 062e 062e 062e 062e
062e 062e 062e 062e 062e 062e 062e 062e 062e 062e 062e 062e 062e 062e 062e 062e
570a f009 f009 f009 f009 3e04 ff0f ff0f ff0f ff0f 500a fa0b fa0b fa0b fa0b fa0b
4605 fd0b fd0b fd0b fd0b fd0b ff0b ff0b ff0b ff0b ff0b ff0b ff0b ff0b 4007 0e2c
0e2c 1429 1429 1f1a 1f1a 1f1a 1a13 170f 170f 170f 170f 170f 1912 1d15 231d 2a24
351f 401f 4815 490a 490a 490a 450a 000c 000c 000c 000c 000c f50e f10e f10a f10a
f10a f10a 4006 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 bdb5 4d0a c169
6afe dfa4 7614 44ec fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30
fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30
fc30 fc30 fc24 fc1d fb1b f918 f918 f918 f918 f918 f918 f918 f918 f918 f918 f918
f918 f918 f918 f918 f918 f918 f918 f918 f918 032f 032f 032f 032f 032f 032f 032f
032f 032f 032f 032f 032f 032f 032f 032f 032f 032f fd2f fd2f fd2f fa2f fa19 fa19
fa19 f719 f932 f532 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232
f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232
f232 f232 f232 0732 0f32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32
1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32
1e32 1e32 1e32 1e32 f532 f532 f532 f92f f92f f930 fd37 0043 fd53 ff00 ff00 ff00
ff00 ff00 06fa 06fa 06fa 0031 0031 f833 0031 0031 0031 0031 0031 0031 e737 ee35
f731 f731 fa31 fa31 fa31 fa31 fa31 fa31 fa31 fa31 fa31 fa31 fa31 fa31 0e31 1531
2231 2331 2531 322f 322f 322f 322f 2634 2234 1934 0f34 0b34
```
### psxdelta_t3 (0x44C words: hi=dx lo=dy, signed bytes; from PSX ENGLISH.EXE)
```
fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30
fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc30 fc24 fc1d
fb1b f918 f918 f918 f918 f918 f918 f918 f918 f918 f918 f918 f918 f918 f918 f918
f918 f918 f918 f918 f918 032f 032f 032f 032f 032f 032f 032f 032f 032f 032f 032f
032f 032f 032f 032f 032f 032f fd2f fd2f fd2f fa2f fa19 fa19 fa19 f719 f932 f532
f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232
f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 f232 0732
0f32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32
1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32 1e32
f532 f532 f532 f92f f92f f930 fd37 0043 fd53 ff00 ff00 ff00 ff00 ff00 06fa 06fa
06fa 0031 0031 f833 0031 0031 0031 0031 0031 0031 e737 ee35 f731 f731 fa31 fa31
fa31 fa31 fa31 fa31 fa31 fa31 fa31 fa31 fa31 fa31 0e31 1531 2231 2331 2531 322f
322f 322f 322f 2634 2234 1934 0f34 0b34 0734 fc18 fc18 fc18 fc18 fc18 fc18 fc18
e424 e424 e424 f324 f91f f91f 001a 001a fb17 fb17 fb17 fb17 fb17 fb17 fb17 fb17
fb17 fb17 fb17 fb17 1f1e 1f1e 2b21 3928 3930 3930 3930 1830 1830 1830 1830 1830
1030 0630 fc05 fc05 fc05 fc05 fc05 16f8 16f8 16f8 16f8 16f8 16f8 16f8 16f8 1602
1602 1602 1602 1602 1602 1602 1602 1602 16f7 ff2e ff2e ff2e ff2e ff2e ff2e ff2e
ff2e ff1a ff1a ff1a ff1a ff1a ff1a ff1a ff1a ff31 ff31 ff31 ff31 ff31 ff31 ff31
ff31 ff31 ff31 ff31 ff31 ff31 1d31 3631 3631 3631 3631 3631 3631 2a31 2a31 1732
1732 0033 f933 f91a f91a f91a f91a f91a f91a f91a f91a f91a f91a f91a f91a f91a
ff32 ff32 1043 0208 0208 0208 0005 0234 06f7 06f7 06f7 0d00 0d00 0601 0601 2425
2425 2414 2408 2903 0330 0330 0330 0330 0330 0330 0330 0330 0330 0330 0330 0330
0330 0330 0330 0330 0330 0330 0330 0330 0330 0330 0330 0330 0330 0330 0025 fc1c
fc1c fc1c fc1c fc1c fc1c 0030 0030 0030 0030 0030 0030 0030 0030 0030 0030 0030
0030 0030 0030 f430 ea30 e230 e230 e230 e230 e230 e230 e230 e230 e230 e230 e230
e230 e230 e230 e230 e230 e230 e230 e230 e230 e230 e230 ee2f ee2f f82f f82f f82f
0032 0032 0032 0032 0032 0032 0032 0032 0032 0032 0032 0032 0032 0032 0032 0032
0032 0032 0032 0032 ff2e ff2e ff2e ff2e ff2e ff2e ff2e ff2e ff2e ff2e ff2e ff2e
0100 f507 f409 ed0b e808 e305 de05 dd06 0a2c 0a2c 1c16 2e07 3a00 3afa 3afa 3afa
3afa 002f 002f 002f 002f 002f 002f 002f 002f 002f 002f d633 d633 d133 c833 c833
c833 c833 c833 c833 c833 dc04 dd00 e8f8 eef5 f7f4 fcf3 fcf3 fcf3 fcf3 3709 3709
3709 3709 2f09 2c05 2c05 2c03 2904 03ff 03ff 03ff 03ff 02fb 02f8 05f7 05f7 1f15
1615 130f 0d0b 330a 3300 3300 32ff 2703 2703 2008 160d 160d 160d 160d 160d 160d
1614 1622 0b26 0b28 042d 042d 042d 042d 042d 042d 042d 042d 042d 042d 042d 042d
042d 042d 042d 042d 042d 042d 042d 042d 042d 042d 042d 042d fe33 f738 f43b ec3b
e83b e53b e53b e53b e53b e53b e53b e53b e53b e53b e53b e53b e53b e53b e53b e53b
fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34
fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34
fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34 fd34
fd34 fd34 fd34 fd34 fd34 fd34 0734 0720 0717 030c 030c 0106 0106 0106 0106 0106
0106 0106 0106 0106 0106 0106 0106 0106 0106 0106 0106 0106 0106 0106 0106 0106
0106 0106 0106 0106 0106 0106 0106 0430 0430 0430 0430 0430 0430 0430 0430 0430
0430 0430 0430 0430 0430 0430 0430 0430 0430 0430 0430 0430 0430 0430 0430 0430
0430 0430 0430 0425 041a 041a 041a 0413 0413 0413 0413 0413 0413 0413 0413 0413
0413 0413 0413 0413 0413 0413 fd12 fd12 fd12 fd12 fd12 fd12 fd12 fd12 0220 0220
0220 0220 0220 0220 0220 0220 0220 0220 0220 0220 0220 0220 0220 0220 0220 0220
0333 0333 0333 0333 0333 0333 0333 0333 0333 0333 0333 0333 0033 fd2f fd2f fd2f
fc1f ff1b fe18 f910 f910 f910 f30c f50c f50c f50c f50c f50c f50c f50c f50c f50c
f50c f50c f50c 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 7493 38d2
fe95 9ee9 4c21 4262 8bff bb60 6cda 6c6e 5477 bb19 2a08 7ade cf67 f8eb 7429 008d
d731 fdc7 d95f ed5a e77f feb3 e7d2 247b 3b6f f70e 8110 cc5e 1ab7 ccc4 1368 8116
1bf7 bfe5 6f11 3aa1 74f1 7f13 2a03 9934 0efb fdd0 4190 c1f9 bffe 3f08 4620 87b6
dad3 5a24 ca26 d7f3 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335
0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335
0335 0335 0335 fe2f fc29 f929 f929 f929 f929 f929 f929 f929 f929 f929 f929 f929
f929 f929 f929 f929 f929 f929 f929 f929 f929 0733 0733 0733 0733 0733 0733 0733
0733 0733 0733 0733 0733 0733 0733 0733 0733 0733 0334 0334 0334 0334 0334 0334
0334 0334 0334 0036 f636 e936 012d fe2e f631 ea31 0138 fa3c f83c f63d f63d f63d
f63d f63d f63d f63d f63d f63d f63d f63d f63d f63d f63d f63d f63d f63d f63d f63d
f63d f63d 0a35 0f34 1634 1634 1634 1634 1634 1634 1634 1634 1634 1634 1634 1634
1634 1634 1634 1634 1634 1634 1634 1634 1634 0234 022d 0229 0020 0020 0020 fa11
fa11 fa11 fa51 fe41 0133 032c 032c 0934 f936 f336 f336 f336 fc37 fc37 fc37 fc37
fc37 fc37 fc37 f639 f53c f53c f53c f53c fd39 fd39 fd39 fd39 0734 0734 0734 0034
de34 c834 b83a b03f b03f b03f b03f ba36 c836 d732 e532 ec32 f633 fb35 0032 0032
0032 0032 0032 0032 0032 ff59 ff59 ff59 ff59 ff59 0849 0545
```
### psxdelta_t4 (0x44C words: hi=dx lo=dy, signed bytes; from PSX ENGLISH.EXE)
```
0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335
0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 fe2f
fc29 f929 f929 f929 f929 f929 f929 f929 f929 f929 f929 f929 f929 f929 f929 f929
f929 f929 f929 f929 f929 0733 0733 0733 0733 0733 0733 0733 0733 0733 0733 0733
0733 0733 0733 0733 0733 0733 0334 0334 0334 0334 0334 0334 0334 0334 0334 0036
f636 e936 012d fe2e f631 ea31 0138 fa3c f83c f63d f63d f63d f63d f63d f63d f63d
f63d f63d f63d f63d f63d f63d f63d f63d f63d f63d f63d f63d f63d f63d 0a35 0f34
1634 1634 1634 1634 1634 1634 1634 1634 1634 1634 1634 1634 1634 1634 1634 1634
1634 1634 1634 1634 1634 0234 022d 0229 0020 0020 0020 fa11 fa11 fa11 fa51 fe41
0133 032c 032c 0934 f936 f336 f336 f336 fc37 fc37 fc37 fc37 fc37 fc37 fc37 f639
f53c f53c f53c f53c fd39 fd39 fd39 fd39 0734 0734 0734 0034 de34 c834 b83a b03f
b03f b03f b03f ba36 c836 d732 e532 ec32 f633 fb35 0032 0032 0032 0032 0032 0032
0032 ff59 ff59 ff59 ff59 ff59 0849 0545 0545 0545 0545 0838 fc33 fc33 fc33 0235
0235 fb28 f528 f528 f72d fc32 f732 fc29 ee2e f038 f038 f73b fb3c fb3c 023c fd39
fd39 0132 0132 0132 0132 0132 0132 0132 0132 0132 0132 0132 0132 0132 0132 0132
0132 0132 0132 0132 fc2a fc2a fc2a fc2a fc2a fc2a fc2a fc2a fc2a fc2a fc2a f32a
ef2a ef2a ef2a ef2a f82c f82c f10b f10b f10b f10b f10b f10b f10b f10b f10b f10b
f10b f10b f10b f10b f10b f10b f10b f10b f10b f10b f10b f10b f10b f10b f10b f10b
f10b f10b f10b f10b f10b f10b f10b e234 d73a d73a d73a e33a eb3a f33a f934 f934
0636 0636 0636 0636 0636 0636 0636 0636 0636 0636 fa26 fa26 fa26 fa26 fa26 fa26
fa26 fa26 fa26 fa26 0c33 0c33 0c33 0c33 0c33 0c33 0c33 0c33 0c33 0c33 0c33 0c33
0c33 0c33 0c33 0c33 0c33 0c33 0c33 0c33 0c33 0c33 0c33 0c33 0c33 fd2a fd2a fd2a
fd2a fd2a fd2a fd2a fd2a fd2a fd2a fd2a fd2a fa11 fa11 074a 0b25 0b25 1a12 1a12
0710 0f1f 1a1c 1e10 1e06 1e06 fa47 e043 e54a e54a e54a e54a dd45 dd34 d31e dc18
e311 e311 e311 e311 e311 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335
0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 0335 fc30 fc30 fc30 f830
f830 f830 f830 f830 f830 f830 f830 f830 f830 f830 f830 f830 f830 0132 fe19 fe19
fe19 fe00 fe00 fe00 f9f1 f9f1 f9f1 f9f1 f9f1 f9f1 043a 043a 043a 043a 043a 043a
043a 043a 1632 e42e fa2a fa2a 072a 072a 072a fb2a ff2f ff2f f12f ee2f e62a e62a
e62a e62a e62a e62a f132 fa33 0034 0034 0034 0034 0034 0034 0034 0034 0034 0034
0034 f82c f328 eb24 e71b e71b e71b e71b 0433 0433 0433 0433 0433 0433 0433 0433
0433 0433 0433 0433 0433 0433 0433 0433 0433 0433 0433 f90b ed0e e809 e809 fc39
ee2a e019 e019 e019 e019 e019 0331 0331 0331 0331 0331 0331 0331 0331 0331 0331
20fe 20fe 20fe 20fe 20fe 20fe 20fe e20c e20c e20c e20c e20c e20c e20c e20c e20c
da34 24fb 24fb 1cf3 0dda 0024 ef21 f41d e81d e81d e81d e81d e81d e81d e81d e81d
e017 e017 e017 e017 e017 e017 f717 f717 f717 f717 f221 f221 ed28 fb28 f62a f62a
f62a f62a f62a 0332 0332 1339 0b39 0339 f739 ec39 e539 dc39 da39 da39 da39 da39
da39 da39 da39 da39 da39 da39 da39 da39 da39 da39 da39 da39 da39 da39 da39 da39
da39 da39 da39 da39 da39 da39 da39 da39 da39 da39 da39 da39 da39 da39 da39 da39
da19 da19 da19 da19 da19 da19 da19 da19 da19 da19 da19 da19 0633 0633 0633 0633
0633 0633 0633 0633 0633 0633 0633 0633 0633 0633 0633 0633 0633 0633 0633 0633
0330 0330 ff28 fa1f f816 f612 f60e f60e f60e f60e f60e f60e f60e f60e f60e f60e
f60e f60e f60e f60e f60e f60e f60e f60e f60e ff35 ff35 ff35 ff35 ff35 ff35 ff35
ff35 ff35 ff35 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 fdfe 7f7d 3010 8799
7efb efd2 0c20 9af1 0610 0973 010b fb0c 0201 1e00 0a14 0873 020b 000d 0301 1e00
0710 0973 010b fb04 0301 1e00 0a14 0873 020b 0006 0402 1e00 0712 0973 010b fb0c
0201 1e00 0d15 0873 020b 000c 0301 1e00 0812 0973 010b fb04 0301 1e00 0a15 0873
020b 0006 0402 1e00 0610 0973 010b fb0c 0101 1e00 0a14 0873 020b 000c 0301 1e00
0710 0973 010b fb0c 0201 1e00 0914 0873 030b fb04 0202 1e00 0710 0973 010b 030c
0101 2100 0a14 0873 020b 030d 0301 2100 0710 0973 010b 0300 0101 2100 0a14 0873
020b 0304 0301 2100 101d 0773 0315 000d 0401 1e00 0f19 0773 0314 0006 0402 1e00
101d 0664 0313 000d 0401 1e00 101d 0773 0315 0006 0402 1e00 101d 0773 0313 000d
0401 1e00 0f14 0773 030b 0005 0102 1e00 101d 0773 0315 030d 0401 2100 0f1d 0773
0315 0305 0402 2100 1618 0a64 030e 0000 0000 1e00 1618 0a64 030e 0000 0000 1e00
1618 0a64 030e 0000 0000 1e00 1618 0a64 030e 0000 0000 1e00 1618 0a64 030e 0000
0000 1e00 1618 0a64 030e 0000 0000 1e00 1618 0a64 030e 0000 0000 1e00 1618 0a64
030e 0000 0000 1e00 0423 0a6e 0214 0500 0201 1e00 0324 0a64
```

## Historical: earlier structural facts and negative attack logs (superseded)

Kept verbatim for the record. Everything below assumed the chip's map does not depend on the character; item 12's "probability below 5e-11" argument is correct for that model and simply shows the model was wrong.

## Structural facts about the uploaded SRAM

- Size is exactly 0x700 = 7*0x100 meaningful bytes. Each byte is transferred
  in a 16-bit bus write; the ciphertext tables themselves are not uploaded.
- There are only 110 distinct byte values in the complete upload. The most
  frequent values occur 20-29 times.
- For all 1792 entries, `key[i] & 0x80` equals `(i & 0x10) << 3`. Equivalently,
  every 32-entry block contains 16 values below 0x80 followed by 16 values at
  or above 0x80. This is exact, not a statistical correlation.
- Removing that address-determined high bit leaves only 84 distinct 7-bit
  values. Treat `key[i] & 0x7f` as a candidate effective key symbol and the
  high bit as a possible address/key cancellation input, not as eight fully
  independent key bits.
- The exact `0xA8` twin normalization used by both solved 0072 chips does not
  expose a hidden duplicate table here. It leaves 88 distinct values and only
  6 of the 896 aligned low-half/high-half pairs become equal. Exhausting every
  fixed high-key normalization mask `0x80..0xff` finds at most 15/896 aligned
  pairs (mask `0xb7`), consistent with coincidence rather than a twin layout;
  the mask that minimizes the total alphabet (`0x81`) still leaves 82 values.
  The forced high bit remains a clue, but not evidence that Primal uses the
  solved chips' exact twin rule.
- The seven 0x100-byte pages contain 101, 101, 99, 96, 94, 103, and 95 distinct
  values respectively. They are therefore not ordinary bijective 8-bit S-boxes.
- No entries separated by 16 bytes are identical (0/1776), so the exact bit-7
  pattern does not come from simply duplicating low-half values with the high
  bit toggled.
- This structure makes "seven banks/rounds" worth testing, but does not prove
  it. It could instead be a deliberately constrained collection of per-address
  keys. The second upload reverses the same bytes and must be kept as a live
  state/layout possibility.

## Pair reconstruction
```python
w = lambda i: (i*0x6915 + 0x6915) & 0xffff
# for t in range(5), i in range(max_t[t]+1):
#   c = cipher_t[i];  skip if c == 0
#   d = psxdelta_t[i]              # final signed-byte coordinate delta
#   r = w(i) ^ d                   # actual 16-bit chip response
#   known_pair = (i, c) -> r
```

Applying the stated validity rule produces **4177**, not 5329, known pairs:
959/1044/785/687/702 from tables 0..4 respectively. The older 5329 count
included entries beyond the per-character maximum. Those tails must not be
used as cryptographic evidence unless code tracing proves the arcade queries
them.

## Validation of the corpus itself

These deltas are not hypothetical: wired into MAME's protection HLE they fix
Primal Rage's knockback/positions in-game (user-verified). The PSX port
(Probe, 1995) embeds the arcade protection module with the chip replaced by
direct reads of these tables (MIPS fn 0x8001A660 in GAME/ENGLISH.EXE).

An offline reimplementation of the current MAME lookup checked all 4177 valid
`(i,c)` pairs: **4177 matches, 0 misses, 0 ambiguous matches**. The dynamic
"757/757" number elsewhere in the notes is the number of mode-2 transactions
seen during one Armadon replay, not the total static corpus size. Thus exact
emulation of the known game queries is solved even though the general FPGA
function is not.

## Earlier attack setup (completed)

- PROVEN: sigma=identity and sigma=reversed both yield same-(keybyte,c) query
  pairs with CONTRADICTING `r` (3 resp. 1 cases) -> key-index mapping is
  scrambled (bit-permutation like 136094-0072's key_offset, or worse).
- 123 ciphertext-collision sets across the corpus; no `r` collision anywhere
  (expected agreements if key equality were random: ~1 -> uninformative).
- ORBIT ATTACK (family assumption M_k = A^g(k) M_0, A = 16-bit LFSR companion
  matrix): two pairs with equal c satisfy r_j = A^x r_i for some |x|<=~128,
  REGARDLESS of sigma and kmap. Brute-forcing all 65536 tap words of A over
  all collision pairs; correct taps must satisfy >~90%, random taps ~0.4%.
  No such A was found in either Fibonacci or Galois form; results follow.

## Attack log — session 2026-09-11 (Claude Opus 5), all NEGATIVE

5. Fibonacci-LFSR conjugation family (M_k = A^g(k) M_0, A = 16-bit left-shift
   companion): sigma/kmap-independent orbit test over the 123 ciphertext-
   collision pairs, all 65535 tap words, both directions, 130 steps:
   best score 5/123 on the chip-response `r` side AND on the semantic-delta
   `d` side
   (random baseline 1-3/123). FAMILY RULED OUT for both output conventions.
   Galois-form conjugation was subsequently tested in session 2026-09-12.
6. Exact 136094-0072 key_offset bit wiring (10-bit): produces same-(key,c)
   contradictions. Ruled out.
7. All 3584 rotational sigmas (i -> +/-i + K mod 0x700): affine-with-constant
   consistency on the top-3 key-value groups: ZERO candidates.
8. Per-index keystream models (chip ignores key, decrypt = c op ks(i)):
   XOR / SUB / XOR-after-whitening: 0/959 multi-table indices agree. Ruled out.

## Attack log — session 2026-09-12, all NEGATIVE except HLE validation

9. Galois LFSR orbit search, sigma/kmap-independent, all 65536 tap masks,
   both shift directions, both output conventions, 130 steps:
   - actual chip response `r`: best 6/123 collision pairs in either direction;
   - semantic delta `d`: left/invertible best 6/123; right/invertible best
     5/123. A misleading 13/123 right-shift score came only from tiny,
     non-invertible tap masks collapsing small signed deltas into short cycles.
   A true shared-orbit family should satisfy nearly all 123 pairs. Galois and
   Fibonacci forms are therefore both ruled out.
10. Both solved relatives' address-bit permutations were tested under every
    11-bit XOR mask (with natural 10/11/12-bit truncation/extensions). Every
    candidate had many inconsistent per-key affine groups.
11. Exhaustive signed address-bit permutation search: all 11! permutations
    and all XOR masks that keep every observed query inside the uploaded
    0x000-0x6ff SRAM region were tested in optimized C++. That is
    **5,078,384,640 mappings per output convention**, for both `r` and `d`.
    Result: **zero affine survivors**. A separate no-XOR pass over all
    39,916,800 permutations also found zero survivors.
12. Mapping-independent affine RANSAC removed sigma from the assumptions.
    Each index with all five tables contributes five `(c,r)` points; four
    indices provide 20 equations, enough to determine or reject an affine
    16->16 map. 100,000,000 random four-index groups were tested for `r`, and
    another 100,000,000 for `d`: **zero consistent quadruples**. There are
    only 110 distinct uploaded byte values. If one byte selected a shared
    affine map, even the most evenly/adversarially distributed grouping of
    the 687 full-five-sample indices predicts at least 23.8 sampled same-key
    quadruples; observing zero has probability below 5e-11 under that model.
    This rules out a one-byte-selected exact affine map independently of
    identity/reversal/bit permutation/general sigma. It does NOT rule out
    multi-byte key use, a nonlinear map, or state/address dependence.
13. Broad algebraic-normal-form fits also looked random:
    - affine in `c` with coefficients quadratic in the 11 address bits:
      1139 features, rank 972, 3205 contradictory equations;
    - quadratic in `c` with coefficients affine in address:
      1644 features, rank 1527, 2650 contradictions;
    - a common ciphertext substitution `F(c)` through Boolean degree four
      plus a completely free per-address XOR mask: 3560 independent features,
      still 617 contradictions;
    - holding out one character table produced essentially zero exact 16-bit
      predictions for all lower-degree models. This is a useful guard against
      accepting an interpolator that only memorizes the corpus.
14. No global modular-affine form `T(r) = a*T(c) + b_i (mod 65536)` survived,
    testing identity, byte swap, bit reversal, nibble swap, and every bit
    rotation for `T`. There are also no power-of-two ciphertexts in the 4177
    valid pairs, so the solved relatives' power-of-two special case cannot
    explain these failures.
15. Exact HLE validation succeeded: matching `(i,c)` against the five arcade
    source tables and returning the corresponding PSX delta reproduces all
    4177 valid relationships with no collision ambiguity.
16. A source-level comparison of the two solved devices found a meaningful
    family parameter that the earlier exact-Moto test had not covered. Both
    build a result from 16 of 17 LFSR-derived power words, but Moto Frenzy omits
    power 5 while Space Lords omits power 15; Space Lords also has an explicitly
    programmable 8-bit polynomial and a latched-result protocol resembling
    Primal Rage's transaction flow.
17. The full-five-sample indices were therefore tested against a deliberately
    over-permissive Space Lords family. Independently for each of the 687
    indices, the search allowed every `poly_lsb` (0..255), every internal state
    `k` (0..0x70), and every possible omitted power (0..16): 491,776 candidate
    cores per index, 337,850,112 over the corpus. The power-of-two and reverse-
    orbit exceptions from the real device were included. Four interpretations
    were tested: `c -> r`, `r -> c`, `c -> d`, and `d -> c`. Result: **zero
    five-sample fits, and no candidate even matched three of the five words at
    any index**. Allowing an arbitrary 16-bit XOR mask or modular-add offset
    after the core separately at every address also produced zero five-sample
    fits. Thus neither direction of the exact Space Lords core, nor the natural
    "unknown omitted power" generalization, explains Primal Rage—even when the
    polynomial, internal key, and post-processing constant are allowed to vary
    impossibly freely by address. This does not rule out nonlinear pre/post
    scrambling, several SRAM bytes, a different core, or stateful processing.

## Where this leaves the problem

The map `(key_sram, i, c) -> r` is none of the following tested families:
either solved 0072 cipher; the Space Lords skeleton with arbitrary
polynomial/internal state/omitted power and a per-address XOR or addition; an
address-selected one-byte affine cipher under any tested or arbitrary
grouping; a Fibonacci/Galois shared-LFSR orbit family; a simple per-index
keystream; a low-degree global Boolean map of address+ciphertext; or a simple
modular ARX form. A reversed upload or more complicated address scrambling is
still possible, but it cannot rescue the one-byte affine family because the
mapping-independent RANSAC test did not assume an address mapping at all.

The viable families are now qualitatively broader: several SRAM bytes may be
combined; the seven 0x100-byte regions may feed rounds or lookup stages; address
bits may enter the datapath directly; the cipher may be a nonlinear Feistel/SPN
or use addition/carries; or the device may be stateful. With only up to five
known ciphertexts at a given address, infinitely many such 16->16 functions
fit the corpus but disagree on a sixth input. A polynomial, decision tree, or
perfect hash can always be made to fit; that is a corpus encoding, not evidence
that the hardware was recovered.

### Is hardware access necessary?

**Not for accurate emulation of the released game's known mode-2 traffic.**
The PSX-derived HLE is exact on all 4177 validated pairs and already covers the
757-query replay.

**Probably yes for a silicon-faithful arbitrary-input/key implementation, given
only the present corpus—but not logically unavoidable.** Hardware can be
avoided if an FPGA configuration/netlist, Atari engineering documentation, a
prototype with an independent encryption pass, or a strongly constrained
architecture is found. Without one of those, the static data are
information-theoretically underdetermined: no address has more than five known
inputs out of 65536, and the hoped-for sharing across equal one-byte keys is
now strongly falsified.

The highest-value hardware experiment is not another gameplay capture. Use a
patched program ROM plus a logic analyzer/bus capture to turn one board into a
chosen-input oracle:

1. Hold `i` fixed and query `c = 0`, all 16 powers of two, then random words.
   This immediately distinguishes affine, Feistel, arithmetic, and special-case
   behavior. If feasible, sweep all 65536 `c` values at one address.
2. Repeat after controlled SRAM uploads: all zero, all one repeated byte,
   one changed SRAM location, one-hot bits, and changed 0x100-byte banks. This
   identifies which SRAM locations influence the fixed query and whether the
   seven-bank structure is meaningful.
3. Repeat selected inputs at multiple addresses and in different query orders
   to detect address mixing and statefulness.
4. Capture address, data, read/write, and timing for both writes and reads;
   retain raw traces. Normal game output alone loses the chosen-input leverage.

Hardware logistics are realistic rather than speculative. Start from the
[Atari Primal Rage operator's manual and PCB schematics](https://wwyss.ch/Arcades_Manuals/Manuals/Arcades/PRIMAL_RAGE.pdf).
There is also a useful precedent from the same preservation community: a
[ROM-socket FPGA board was used to inject code and monitor bus traffic](https://mamedev.emulab.it/haze/2017/11/06/doing-things-properly/)
while extracting protected data from PGM2 hardware. The electrical details are
different, but the acquisition strategy is directly relevant.

If hardware cannot be obtained, the best software-only route is constrained
circuit synthesis with honest cross-validation: choose a small Feistel/SPN/ARX
or banked-lookup architecture, fit it on four tables, and demand exact prediction
of the fifth. Do not count an exact fit using one free value or coefficient per
known pair as a cipher break.
