# Primal Rage protection investigation — status notes

**DO NOT COMMIT** — working notes for the `primal-rage-blood` branch.
Task: understand/fix MAME blood & gameplay defects caused by unemulated protection.
Issue: https://github.com/asturur/mame/issues/3

## TL;DR so far

Primal Rage's protection (Atari XGA FPGA **136094-0004A**, undumped, sits overlaid
on color RAM at 0xD80000-0xDFFFFF) is a **decryption oracle**: the game keeps
per-character data tables in ROM as ciphertext and asks the chip to decrypt
words at runtime. MAME's current simulation (`primrage_protection_r/w` in
`src/mame/atari/atarigt.cpp`) returns a value that makes every query answer
**decode to 0**, so all protection-derived data is zero. Symptoms (confirmed
by community lore about bootlegs with chip absent — floating dinosaurs when
punched, blood kicking in/out or turning brown, fighters falling): wrong
character position after hits, wrong/yellow blood, roast-fatality palette
going black.

## Setup / artifacts

- Build: `make SUBTARGET=atarigt SOURCES=src/mame/atari/atarigt.cpp REGENIE=1 -j$(sysctl -n hw.ncpu)`
  → binary `./atarigt` (repo root).
- ROMs: `~/.mame/roms-local/` (`primrage.7z` merged + unzipped `primrage/` folder;
  extracted copy with old-set subfolders at `/tmp/primrage_rom/`).
- Run+record: `./atarigt primrage -rompath ~/.mame/roms-local -record bloodNN.inp -log -window`
  - inp files land in `inp/`; `error.log` in cwd (overwritten each run — copy it aside!).
  - blood01.inp = first exploratory session; blood02.inp = eventful session
    (wrong dino position after hit, blood, roast-to-black).
- Logging added in `atarigt.cpp` (this branch, uncommitted): canonical lines
  `PROT R/W f=<frame> v=<scanline> pc=<PC> @<addr> -> <val> (raw <colorram>) mode=<n> [tag]`.
  Masks: `LOG_PROTECTION` (interesting) + `LOG_PROTECTION_ALL` (all colorram traffic),
  both enabled via `VERBOSE`.
- Program ROM images (68020, byte-interleaved, 2MB):
  - `progdump.bin` (repo root) = primrage Jan 1995 (v2.3j) — verified == interleave of
    `rage_136102-2044a_pgmuu.29l`(+0) `2043a_pgmum`(+1) `2042a_pgmlm`(+2) `2041a_pgmll`(+3).
  - `/tmp/prog20.bin` = primrage20 (Aug 1994), `/tmp/progo.bin` = primrageo (Dec 1994).
- Disassemblies (repo root, from MAME debugger `dasm`): `prot_28000.asm`,
  `prot_module.asm` (0x2810C-0x2890x), `prot_3e3c0.asm`, `prot_44200.asm`,
  `prot_44280.asm`, `prot_3e454.asm`, `site_10940.asm`, `site_15840.asm`,
  `site_179c0.asm`, `site_19290.asm`.
  Regenerate: `printf 'dasm out.asm,ADDR,LEN,1\nquit\n' > /tmp/ds.txt && ./atarigt primrage -rompath ~/.mame/roms-local -debug -debugscript /tmp/ds.txt -video none -sound none -nothrottle`
- Extracted data: `/tmp/keytable.bin` (0x700 key bytes), `/tmp/ciphertables.json`
  (5 tables), `/tmp/rleheaders.json` (RLE xoffs/yoffs pairs).

## Reverse-engineered architecture (primrage v2.3 Jan 1995 addresses)

All protection accesses go through colorram handlers; the sim's hardcoded PCs
(0x20f90/0x27592/0x3d8dc/0x437fa) are for the **Dec 1994 set**; Jan build has
same code relocated (0x28xxx/0x3Exxx/0x44xxx). Address-sequence mode detection
still fires. All log lines show `[UNKNOWN-PC]` on this set.

### Game-side protection module

- **0x28310 = public API `prot_update(player)`** — called from game code all over
  (0x10950, 0x10a3a, 0x10a70, 0x10ac8, 0x15852, 0x15892, 0x179ce, 0x179d8,
  0x192a8, and wrappers below). Per-player struct at `0xffffe7bc + p*0x94`;
  character ID byte at struct+0x7a (0..6, seven dinosaurs).
  - If object flag bit19 (`($40,obj) & 0x80000`): **bypass** — copies obj($18/1c)
    → struct($2c/$30) without chip. (MAME's zero-answers ≈ permanent bypass.)
  - Else calls 0x28222, and if the computed index changed: calls 0x442F2
    (player 0 only) then 0x2810C (the chip query); result deltas stored at
    `0xffff8844 + p*8` (two longs).
  - struct($2c) = obj($18) + delta1; struct($30) = obj($1c) + delta2 —
    "protected coordinates", copied to globals 0xffff871c/8720 (P1) and
    0xffff8724/8728 (P2) by caller 0x179c0. With flag 0xffffdcb1 set they're
    written back into another object (0xffffc6bc array) via 0x2286e.
  - Debug display block gated on RAM flag `0xffffe3cc` prints the values
    (dev leftover; prints zeros in MAME).
- **0x28222 = compute index**: per-char base picture number (see table below)
  subtracted from current RLE sprite code: `idx16 = word(0xFFD78000 + obj($52)*16) & 0x7fff`
  (RLE object RAM word 0 = picture number); clamped to [0, per-char max from
  ROM word table 0xE78E]. If player!=0 also **sends** (value, charID) to chip
  via mode-3 routine 0x3E3D4.
- **0x2810C = query wrapper**: selects per-char cipher table (see below), calls
  0x28038, splits result into two sign-extended bytes → `<<6` (26.6 fixed) →
  per-player delta longs at 0xffff8844+p*8. delta1 negated if 0x17AA4(p)==0
  (facing direction!). Anti-tamper: verifies protection-access counter
  0xffff86f4 advanced, else calls 0x19ab6.
- **0x28038 = chip exchange primitive** `f(index, table_base, mode_flag)`:
  - reads DCC7CA, DCC7CA, DCC7C6, DC4022 → sim enters "mode 2"
  - writes `cipher = word(table_base + 2*index)` to `0xDC7800 + 2*index`
  - polls DC4700 until bit15 (sim returns 0x8000)
  - reads result at DCC7C2
  - returns `result XOR (index*0x6915 + 0x6915)` ← **whitening**
  - **MAME sim returns exactly the whitening → function returns 0 always. This
    is the core bug.** The 0x6915 formula in the sim is NOT the cipher — it's
    the game's own mask, reconstructed by Aaron from this very code.
- **0x3E3D4 = mode-3 send** (~every frame): writes charID byte → DC80F2, saves
  raw word at DC7AF2, writes ROM word `0xEB0C0[charID]` → DE4000 and DEC000,
  clr → DA8700, restores saved word → DC4700. Sends data TO chip; reads nothing.
- **0x442F2 = aux send**: reads DCC7CC twice (sums into 0xffff86f0 checksum
  accumulator — **real chip presumably returns nonzero here; sim gives raw 0**),
  writes `0xEB0C0[arg]` then constant 0x8016 → DC8700, reads DCC7CC twice more.
- **0x44228 = key upload** (boot, and when arg&3==3): triggers mode 1
  (DCC7C4, DCC7C4, DC4010 reads), then writes 0x700 key words to
  DC7800+2*i for i=0..0x6FF. **Key bytes read BACKWARDS from ROM 0xEAB05**
  (i.e. key[i] = rom[0xEAB05 - i]). = FPGA SETKEY analog (cf. atarixga.cpp).
- **0x102B2**: per-frame write 0 → DB0000 (strobe/decoy).

### Data locations (v2.3 Jan 1995 = progdump.bin)

- Whitening constant 0x6915: `mulu #$6915` at 0x280F4 (v2.0: 0x25676; v2.3o: 0x2764e).
- Key table: 0x700 bytes ending at ROM 0xEAB05 (reversed). Same bytes in all
  3 sets (v2.0 src ptr 0xECCF3, v2.3o 0xEE0C7). First bytes (key[0..7]):
  79 47 30 0b 33 4a 21 47. Values span 5..252, 110 distinct.
- Key block `0xEB0C0`: 7 words `2694 6EE0 34F7 32B9 4D5A 2694 6EE0` indexed by
  charID for the send channels; sits right before build timestamp string.
  Identical in all 3 sets (v2.0 @0xED290, v2.3o @0xEE680).
- Cipher tables: 5 × 0x898 bytes at **0xF0000, 0xF0898, 0xF1130, 0xF1838,
  0xF1F40** (0x44C words each). **Identical bytes in all three sets.**
  Char→table: {0,5}→0xF0000, {1,6}→0xF0898, 2→0xF1130, 3→0xF1838, 4→0xF1F40.
- Char picture bases (ROM words): c0/c5 @0x57E64 = 0x0EF0; c1/c6 @0x5BEDA
  = 0x12AE; c2 @0x5AB8C = 0x0BE0; c3 @0x5D8E4 = 0x16C1; c4 @0x59856 = 0x1FAA.
- Per-char max index table @0xE78E: 3BE 413 310 2AE 2BD 3BE 413 (2267).
  Ranges tile perfectly: pics 0xBE0..0x2267 = all character sprites.
- RLE picture directory (region "rle", 16BE; moh=even byte, mol=odd): 4 words
  per picture [xoffs s16, yoffs s16, flags, offlo]. First ROM pair
  (136102-2101a moh0 @byte0, 2100a mol0 @byte1) contains the full directory.

## Cipher status (open problem)

Model: chip result = w(i) ⊕ delta(i) where w(i) = (i*0x6915+0x6915)&0xffff and
delta = 2 signed bytes (X/Y fixups per animation frame). The chip decrypts the
game-supplied ciphertext word (tables above) using the uploaded key table
(key byte per index, selection permutation σ unknown) — same *protocol* as
Moto Frenzy 136094-0072 (atarixga.cpp).

Tested & failed:
1. Moto Frenzy decipher as-is, σ ∈ {identity, reversed, byte-addr}, with/without
   whitening: match rate ≈ random baseline (~6% at ±32 bound). **Not the same
   LFSR/tables.**
2. Per-index consistent key search (all 5 tables must decrypt small under one k):
   1/1100 hits = chance. **Cipher family (polys/kmap) differs from 0072.**
3. Known-plaintext hypothesis "delta = RLE header xoffs/yoffs": exact GF(2)
   linear solve+verify per key group — 0 verified of 79 groups. (Makes sense:
   RLE hw already applies those offsets at draw; the protected deltas are
   *logical* per-frame data, e.g. body-part/emitter offsets from the animation
   tool — not recoverable from ROM art.)
4. Weak positive: RANSAC on sign-extension parity (bit7==bit6, bit15==bit14 of
   whitened plaintext), σ=identity: overall inlier 0.78 vs ~0.55-0.65 overfit
   baseline; 25/107 groups >85%. Suggests small-delta + linearity + σ≈identity
   is roughly right but not proven.

Not yet tried:
- Space Lords 136095-0072 cipher variant (different lfsr taps + poly_lsb).
- Brute-force of the 0072-*family* parameter space (lfsr taps, L-orbit start,
  kmap-as-free-permutation makes naive brute force infeasible; need to absorb
  kmap by testing "∃k" consistency instead).
- Extract better statistical constraints from consumer semantics (e.g. deltas
  near 0 for idle frames; smoothness across animation frames).
- primrage20/primrageo protection modules verified same key/tables; their
  *code* differs only in addresses.

## Where the blood specifically might come from (open)

- The mode-2 deltas feed "protected coordinates" globals (0xffff871c..8728) and
  can be written back into a second object (0xffffc6bc array, flag 0xffffdcb1)
  — candidate: blood/effect emitter placement.
- The 0x442F2 checksum stream (0xffff86f0 accumulator, DCC7CC reads = 0 in
  MAME) and the anti-tamper callback 0x19ab6 are unexplored — could gate
  blood enable/palette (yellow blood / black roast).
- TODO: disassemble consumers of 0xffff8844/871c/8720/8724/8728 and 0xffff86f0;
  find blood object spawn code; correlate with blood02.inp events.

## Symptom log (user observations)

- blood01 session: yellow blood on hit, paused at frame ~7587. Mode-2 heartbeat
  runs 1-2×/frame with incrementing index params (2 streams: fast ~every frame,
  slow every ~4 frames); mode-3 send 2×/frame; 0x442F2 every ~4 frames;
  DC80F2 charID switched 4→6 at frame 7113 (character change).
- blood02 session (recorded): wrong dino position after a hit; blood visible;
  roast effect rendered player black.

## FIX ATTEMPT #1 (blood) — palette poisoning by protection traffic

`atarigt_v.cpp` blender uses palette as four banks of 32-entry RGB channel LUTs:
`mram = pens[(color_latch & 0xc0) << 7]`, indexes 0..0x1f per channel → only
pens 0x0000-1F / 0x2000-1F / 0x4000-1F / 0x6000-1F are ever displayed.
Colorram mapping (colorram_w): word addr 0x20000-0x27FFF = red+green of pen
(addr&0x7fff); 0x30000-0x37FFF = blue. CPU addr = 0xD80000 + 2*wordaddr.

Protection traffic falls through into those LUTs in MAME:
- DE4000 (mode-3, every frame) → BLUE of pen 0x2000 = bank-1 LUT entry 0 (black level)
- DEC000 (mode-3, every frame) → BLUE of pen 0x6000 = bank-3 entry 0
- DC7800+2i (key upload i≤0x6FF, mode-2 queries i≤charmax; chars 1/6 max 0x413)
  → RED+GREEN of pens 0x3C00+i — crosses bank-2 LUT (0x4000-0x401F) with key
  bytes and ciphertext!
- DC8700 (0x442F2) → pen 0x4380 r/g (not displayed), DA8700 → TRAM (unused).

Hypothesis: yellow blood / black roast = LUT poisoning. On real HW the chip
intercepts its window; writes never reach color RAM.

Change (uncommitted, atarigt.cpp): `primrage_chip_write(addr)` — suppresses
colorram_w for 0xDC4000-47FF, 0xDC7800-8FFF, 0xDCC780-C7FF, 0xDE4000-3,
0xDEC000-3, 0xDA8700-3, 0xDB0000-3 (primrage only, protection handler still
sees everything). Rebuilt OK. **Verify by replaying blood02.inp** —
`./atarigt primrage -rompath ~/.mame/roms-local -playback blood02.inp -log -window`
(deterministic: same fights, same blood moments; only palette behavior differs).
If blood turns red / roast renders correctly → root cause confirmed; position
defects remain (delta channel still zero, separate fix).

## FIX ATTEMPT #1 RESULT: FALSIFIED

User replayed blood02.inp with chip-write suppression: **no change, blood still
yellow.** Palette/LUT poisoning is NOT the blood mechanism. (Suppression code
kept — it is hardware-correct and harmless — but it is not the fix. Can revert.)

## Detection-vs-data analysis (blood is DATA-driven)

Checked the "game detects bad chip → degrades" theory:
- 0x86f0 checksum accumulator: refs 0x1f842 & 0x1f87c (accumulate DCC7C0/DC4010
  + a SECOND key upload from ROM 0xEA406 forward, 0x700 bytes — note different
  key source than 0xEAB05!), 0x215c6 (init only), 0x28044/0x3e3f6/0x44234/
  0x442fa (accumulate). **Never compared** → not a pass/fail gate; it is data.
- Anti-tamper 0x19ab6 (called from 0x2810C when access counter 86f4 stalls):
  zeroes the two player object pointers 0xffffc688 → makes FIGHTERS vanish/fall
  (matches bootleg "fighters randomly fall"), NOT blood color.
Conclusion: blood color is selected from protection-decrypted DATA. With the
chip returning 0 for everything, the game picks the wrong (yellow) blood.
Therefore the ONLY real fixes are (a) break the cipher or (b) hardware I/O trace.
No shortcut via detection-suppression exists.

## Cipher-break feasibility (honest)

Same PROTOCOL as Moto Frenzy 136094-0072 / Space Lords 136095-0072 (SETKEY +
oracle), but a DIFFERENT cipher instance (their algorithm scores at random vs
our tables — see failed tests above). Recovering a keyed 16→16 permutation from
ciphertext with NO known plaintext is the hard case; the 0072/0072 breaks were
done by S. Neves (cryptographer) et al. and effectively needed chosen-input
access to real hardware. Not realistically crackable from ROM data alone.

## Second key table discovered

0x1f83a uploads a SECOND 0x700-byte key stream read FORWARD from ROM 0xEA406
(vs the main one read backward from 0xEAB05). Two key schedules — dump and
diff both; may indicate two chip contexts or a key-refresh. (extract TODO)

## FIX ATTEMPT #1 ADDENDUM — suppression was WRONG, reverted

A/B replay of blood03.inp proved the chip-write suppression itself CORRUPTED
graphics (magenta blocks, broken sky rows): **0xDB0000 is not a protection
strobe — it is the game's color-latch write** (colorram word 0x18000 = latch
read by atarigt_v.cpp screen_update; same word tmek uses for ignore_writes).
Writes DE4000/DEC000/DA8700 may be similar dual-purpose colorram traffic.
Suppression code reverted (function kept [[maybe_unused]]). Do NOT re-add
without per-address proof.

## Blood pipeline fully traced (blood03.inp, hit at ~3630-3800ms)

- Each fighter = 2 RLE objects (e.g. Sauron e8/e9 col 80/90, Blizzard e12/e13
  col 84/90). The "Blizzard flies into the sky" bug = his own body objects
  launched up (y 223→87 in ~7 frames) after Sauron's bite-grab → knockback
  trajectory broken by protection (bootleg float symptom).
- Blood in MAME = only 4 small drip objects (e10/e11 col C0 left, e14/e15 col
  C4 right), pics 0x7AF-0x887, palette rows = orange-brown ramp (bootleg
  "brown blood" symptom). Real hardware: blood sprays all over + paints floor.
  User confirmed operator setting already FULL GORE → not a dipswitch issue.
- Object engine: effect pool at 0xffff89f8 (0x60-byte structs, anim-script ptr
  at +8, current pic +0xA, flags +0x28; interpreter 0x22620, list build writes
  RLE RAM from 0x2261A/0x22618). Spawner writers during hit: 0x1940E, 0x1A68C.
- 0x1935A per-player attached-effect updater: rel = (current RLE code from
  RLE RAM entry word0) - charbase; if 0<=rel<=0x4B0 reads effect anim id from
  per-char ROM table at playerstruct($24) indexed by rel; else anim 0x1E1.
  So WHICH effect anim plays per frame is plain ROM data.
- The protection deltas (mode-2 plaintext, two signed bytes <<6) are added to
  obj($18/$1c) to form "protected coords" (0xffff871c-8728), which feed
  (a) P1-P2 distance calc 0x103DC (hit/collision), (b) write-back into a
  second object (0x2286e) — i.e. **per-animation-frame body-part/emitter
  offsets**. With all deltas 0 (MAME sim): single degenerate spray from wrong
  point, no spread/floor painting, broken knockback, airborne float.

**CONCLUSION: every observed symptom (yellow-ish sparse blood, no floor
painting, sky-launch knockback) is the SAME root cause — the mode-2 plaintext
deltas are zeroed. There is no separate blood switch. Fix = real decrypted
values.**

## Console-port known-plaintext hunt: PSX — NEGATIVE

Tested user's PSX "Primal Rage (USA)" data track (194MB user data extracted
from MODE2/2352; ISO9660 with per-character dirs ARMD/BLIZ/CBRA/CHOS/DIAB/
SAUR/TALN + GAME/LUMPS + SLUS_001.26). Method: since the known XGA ciphers are
GF(2)-linear in the ciphertext for a fixed key byte, XOR-dependencies among
same-key arcade ciphertexts force the corresponding plaintexts to XOR to a
computable constant (XOR of whitenings, affine part cancels on even-sized
sets). Scanned the ENTIRE ISO at every byte alignment for delta tables
satisfying those constraints, in 3 layouts: 16-bit interleaved (both byte
orders), separate byte arrays (stride 1), 32-bit entries (stride 4).
Tables 0/1/3 (19-31 dependency sets each): **zero survivors** (one early
21-hit cluster at ~148MB was 2/19 sets = structural false positive, inside
DIAB_VIC-area data). Caveats: test assumes (a) cipher linear/affine per key
(like 136094-0072 family), (b) PSX table index == arcade anim index, (c)
unscaled byte values. Port is by Probe Entertainment (different engine) —
mismatch on (b)/(c) plausible. Avenue considered CLOSED unless someone finds
the original Atari animation tool data.

## *** SOLUTION FOUND: PSX port contains the plaintext (2026-09-11) ***

The console-port hunt DID pay off — not in the data files but in the ENGINE:
`GAME/ENGLISH.EXE` (PSX disc, sector 75353, 569344 bytes; SLUS_001.26 is just
a loader) is a source port of the arcade game (debug strings `GOT_HIT()
ATK_NULL`, `REX_MSL()`, stray `*/` etc.). It contains the arcade protection
module translated to MIPS with THE CHIP REPLACED BY PLAINTEXT TABLE READS:

- 0x8001A914 = arcade 0x28310 prot_update (same 0x94 struct pitch, same
  bit19 bypass, same delta array + <<6 fixed-point conversion)
- 0x8001A7E0 = arcade 0x28222 compute-index (same per-char base switch; the
  per-char max table `3BE 413 310 2AE 2BD 3BE 413` sits verbatim at file
  0x5eca8 = addr 0x8006E4A8)
- 0x8001A660 = arcade 0x2810C query, but instead of the chip it does
  `lb table[2*idx]` / `lb table[2*idx+1]` = the TWO SIGNED DELTA BYTES.
  Per-char plaintext tables (file offsets in ENGLISH.EXE):
    chars 0,5 → 0x8007A7D0 (file 0x6AFD0)
    chars 1,6 → 0x8007B068 (file 0x6B868)
    char 2    → 0x8007B900 (file 0x6C100)
    char 3    → 0x8007C008 (file 0x6C808)
    char 4    → 0x8007C710 (file 0x6CF10)
  Char→table mapping identical to arcade. Jump table at file 0x8f4.

FIX IMPLEMENTED (uncommitted, atarigt.cpp): mode-2 handler answers
`m_protresult = (idx*0x6915+0x6915) ^ primrage_prot_delta[t][idx]`, where t is
found by matching the game's written ciphertext word against the arcade cipher
tables in ROM (space.read_word(0xf0000 + t*0x898 + idx*2)). The 5 delta tables
(5×0x44C words, from the PSX exe) are embedded as `primrage_prot_delta`.
This is EXACT HLE: complete coverage of every mode-2 query the game can make.

VERIFIED: blood03.inp replay, same timestamps — Blizzard sky-launch GONE,
fighters grounded and normal. (Replay diverges after first protection-
influenced event, as expected; live play testing by user pending.)

Remaining unemulated (still zeros): DCC7CC reads (0x442F2 checksum channel),
mode-1 upload readback (DCC7C4), DCC7C0/DC4010 checksum reads (0x1f83a).
Watch for residual oddities; those channels are the suspects.

Also: linearity test of the cipher with true (c,p) pairs FAILED for σ=identity
per-key grouping → chip key selection likely address-scrambled (like Moto
Frenzy's key_offset bit permutation) and/or cipher non-linear. With 5329 known
plaintext pairs now available, an offline cipher break (for a "real" emulation
instead of HLE, and for primrage20 differences if any) is a tractable future
project. Pairs: c = word at maincpu 0xF0000+t*0x898+2i, p = w(i) ^
psxdelta_t(i), w(i) = (i*0x6915+0x6915)&0xffff.

Files: /tmp/psxdelta_{A7D0,B068,B900,C008,C710}.bin (raw 0x898 each),
/tmp/deltatables.inc (generated C), /tmp/ENGLISH.EXE, /tmp/psx_data.bin.
PSX disc at "Primal Rage (USA)/" in repo root — DO NOT COMMIT (nor the .inc
provenance comment removal — keep attribution honest in any upstream PR).

## Fix VALIDATED + module fully mapped (end of 2026-09-11 session)

- User confirmed in live play: Diablo's on-ground offset is now correct — he
  falls to the ground properly (was floating/mispositioned before the fix).
  diablo01.inp recorded on the fixed build demonstrates it (in inp/).
- Full arcade↔PSX protection-module map (all MIPS addrs in ENGLISH.EXE):
  28038+2810C→8001A660 (query→plaintext read), 28222→8001A7E0 (index),
  28310→8001A914 (update), 28582→8001AAD8 (both players),
  28598/2862C→8001AB00/8001ABE8 (get prot X/Y), 286C0/286D4→8001ACD0/8001AD00,
  286E8→8001AD30, 2873C→8001AE0C, 28794→8001AEE8, 287D2→8001AF44.
- The PSX port has NO counterpart of the checksum channels (mode-1 DCC7C4
  readback, DCC7CC reads, DCC7C0/DC4010 sums): Probe deleted them → they are
  chip-integrity machinery, not game data. Mode-2 deltas were the ONLY
  game-data channel; the implemented fix covers the protection completely.
- Therefore the remaining "normal hit blood is yellow" issue is NOT
  protection-related. Next: user records a short inp showing it on the fixed
  build; freeze frame, find blood object color field, decode its palette row,
  trace who writes that CRAM row and from which ROM data. (Note: verify what
  real v2.3 hardware shows for normal-hit blood — could yellow even be
  correct for some characters? Get reference footage of the same matchup.)
- Other console ports (Saturn/Jaguar CD): NOT worth bulk analysis now; only
  as targeted cross-check if a specific table value looks wrong in play.
- Video comparisons: only produce when user asks; deliver mp4.

## *** FIX #2: YELLOW BLOOD SOLVED — RLE palette misalignment (2026-09-11) ***

Chain of evidence (blood05.inp, K-triggered scene dump seq1, yellow blood
f029-f050 at (224,140), game timer 57, abs screen frame ~1546-1568):
- Blood fountain = RLE pics 0x517-0x51B, 5bpp (flags 0x163, bppsel 1), color
  field 0x99; impact starburst = pic 0x9E+, 6bpp, col 0xE8 (authored yellow,
  correct). NO dedicated blood palette upload: CLUT rows come from ROM block
  0x62256 (uploaded whole at round start by 0x29FFC): row 0x99 = red blood
  ramp, row 0x9A = tan ramp. 5bpp sprite w/ col 0x99 in MAME read pens
  0x990-0x9AF => art's upper pixels landed on tan row 0x9A → YELLOW.
- Color 0x99 traced to ROM effect definition record (A4=0x6DEDC, word +6 =
  0x0190, | 0x800 flag by interpreter at 0x223A8/0x223CA → w1 0x990). It is a
  ROM CONSTANT — not protection, not victim-dependent.
- Lua experiment (write-tap rewrite col 0x99→0x98 at spawn): fountain turned
  red — user-confirmed correct look.
- ROOT CAUSE: atarirle.cpp computed pen base = palettebase+color UNALIGNED.
  Real hardware concatenates color and pixel bits (they cannot overlap), so a
  5bpp object's base is aligned to 32 pens: col 0x99 ≡ 0x980 → art reads RED
  row 0x99. FIX (uncommitted, src/mame/atari/atarirle.cpp draw_rle):
    palettebase = (m_palettebase + color) & ~u32((1 << info.bpp) - 1);
  Verified on replay with no hacks: fountain red (snap/fix_f1552/1564.png).
  Only objects with depth-misaligned color fields are affected (in the whole
  captured scene, only the blood fountain: col 0x99 odd). NOTE: touches all
  atarirle games (hydra→primrage) — regression-check tmek/guardians etc
  before upstreaming; may also fix known atarigt sprite color quirks.

Debug tooling built along the way (keep): scene_dump.lua (K-triggered
120-frame state+PNG dumper; user asked for 160 — bump), Lua write-tap
color/hide hacks in /tmp, EEPROM snapshot protocol: /tmp/eeprom_snapshot must
be cp'd to nvram/primrage/eeprom before EVERY record & playback (inp does not
carry NVRAM; without it replays desync — bit us twice).

## FIX #1 BUGFIX: Armadon below-ground fall (2026-09-11 evening)

User captured armadon01.inp + K scene dump (Sauron vs Armadon, cave stage):
Armadon (char 4, table 4) hit->exaggerated jump->fell below ground. Scene dump
showed his anim indices valid but P1 deltas stuck at 0 during the whole fall;
log verify showed table-4 (and would-be table-3) queries answered with delta 0.
CAUSE: driver matched ciphertext at `0xf0000 + t*0x898`, but the ROM tables
are NOT uniform stride: real bases 0xF0000/0xF0898/0xF1130/0xF1838/0xF1F40
(gaps 0x898,0x898,0x708,0x708 — tables 3,4 are shorter). t=3,4 reads were
garbage -> never matched -> chars 3 (Vertigo) and 4 (Armadon) got zero deltas.
(Diablo worked because he's char 5 sharing table 0 with Sauron.)
FIX: explicit table_base[5] array in primrage_protection_w. Verified on
armadon01.inp replay: **757/757 mode-2 queries match PSX ground truth across
all tables.** Also update PRIMRAGE_CIPHER_HANDOFF.md mental model: cipher
table stride is not 0x898 for t>=3 (extraction there used correct bases).

## Next steps (proposed order)

1. Trace blood-spawn code path from blood02.inp replay under debugger
   (`./atarigt primrage -rompath ~/.mame/roms-local -debug -playback blood02.inp`),
   breakpoints at 0x2810C/0x28310 consumers; identify which value gates blood
   color/palette.
2. Search game code for readers of 0xffff86f0/86f4 (checksum) and 0x19ab6
   (anti-tamper) to rule in/out the "protection failure detected → degrade
   blood" theory — bootlegs showed *random* blood misbehavior, consistent with
   game actively detecting bad chip answers.
3. Cryptanalysis continued: port Space Lords variant; then parametric search
   of the 0072 family with kmap absorbed.
4. Long-shot: locate a Primal Rage PCB owner for a chip-oracle trace
   (the ultimate fix — full I/O capture like Kirkegaard/Neves/Wilhelmsen did).

## Housekeeping

- `atarigt.cpp` logging changes are uncommitted on branch `primal-rage-blood`.
- This file must not be committed.
- Generated files in repo root: atarigt (binary), progdump.bin, prot_*.asm,
  site_*.asm, error.log, inp/blood*.inp — all untracked, keep until solved.

## *** CIPHER SOLVED (2026-09-12) — real 136094-0004A emulation replaces the HLE ***

The chip is a 16-bit Fibonacci LFSR loaded with the ciphertext and clocked
n = kmap[key byte] times (16..125), feedback taps selected by the per-character
word (ROM 0xEB0C0) the game writes before each query (DC8700 / DE4000 /
DEC000); key byte = SRAM[key_offset(i)] with an 11-bit permutation+XOR, live
SRAM layout = the checksum routine's forward upload from 0xEA406. Early-out
rule: state passing through 0x0001 shortens by one clock (Space Lords' 0xC000
rule). Model reproduces 4174/4177 corpus pairs; the 3 misses are PSX entries
off by one in dx. Full derivation, algorithm spec, reference verifier and the
preserved PSX tables: PRIMRAGE_CIPHER_HANDOFF.md. Implementation:
`atari_136094_0004a_device` in src/mame/atari/atarixga.cpp, hooked from
primrage_protection_r/w in atarigt.cpp (PSX delta tables removed from the
driver). Unknown: how the chip derives taps from the character word (5-entry
lookup used), kmap for the 146 unused key bytes.
