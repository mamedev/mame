# Primal Rage II in MAME — driver situation

**DO NOT COMMIT** — companion note to PRIMRAGE_NOTES.md (which covers Primal
Rage 1 / atarigt). Written 2026-09-12 from the current tree.

## The game

Primal Rage II, Atari Games, canceled after location test (~1996/97). Only a
**test version 0.36a** survives; the game was never released. Two physical
cabinets are known to exist (one famously at Galloping Ghost Arcade). MAME set
name: `primrag2`.

## Completely different hardware from Primal Rage 1

PR1 is Atari GT (68EC020 + custom Atari video + CAGE sound + XGA protection
FPGA — everything we worked on in `atari/atarigt.cpp`). PR2 shares NOTHING
with it: it runs on **"Time Warner ZN-1"** — a Sony ZN-1 arcade motherboard,
i.e. arcade PlayStation hardware (MIPS R3000 CXD8530CQ + CXD8561Q GPU +
CXD2922 SPU), with a Time Warner BIOS. Driver: **`src/mame/sony/zn.cpp`**
(shared with dozens of Capcom/Taito/etc. ZN games).

- `primrag2_state` class: zn.cpp:1208
- machine config `coh1000w`: zn.cpp:1219 (ZN-1 with 2MB VRAM, 8MB RAM)
- GAME lines: zn.cpp:6039 (`coh1000w` Time Warner ZN-1 BIOS root) and
  zn.cpp:6040 (`primrag2`, clone of coh1000w)

## Board specifics (from the driver's PCB notes, zn.cpp:1100-1205)

Game board "PSXTRA A055056-" on top of the standard ZN-1 main board.
Silkscreen easter egg: "IM FEELING A LITTLE ANXIOUS, IF YOU KNOW WHAT I MEAN".
- 4× 27C040 EPROMs (PR2_036.U14-U17) = 2MB of MIPS boot/program code mapped
  at 0x1F000000 (zn.cpp:1254)
- **Quantum Fireball 1080AT 1GB IDE hard drive** with the game data, driven
  by a VIA **VT83C461** IDE controller (custom DMA + IRQ glue in the state
  class — this driver is basically "ZN-1 + IDE")
- Dallas DS1232 watchdog (600ms, hooked at zn.cpp:1230)
- 2× **CAT702** protection chips (standard ZN security): 'TW01' on the main
  board (BIOS pair) and 'TW02' on the game board. Both are DUMPED and
  emulated (8-byte keys as ROM regions: tw01.ic652 in the BIOS macro
  zn.cpp:5801, tw02 in the game set zn.cpp:5828). CAT702 is a solved,
  properly emulated device (`src/devices/machine/cat702.cpp`).
- NEC uPD78081 MCU on the ZN mainboard: internal ROM `NO_DUMP` (standard for
  all ZN sets; I/O behavior is emulated at a higher level).
- Gun connectors (JGUN1/JGUN2) and three RJ45 network jacks (SERIN/SEROUT/
  SLONET) — networking not emulated (typical Atari SLONET linking).

## ROM set contents (zn.cpp:5816-5833)

- Time Warner BIOS msm27c402zb.ic353 (dumped, english)
- 4 program EPROMs (dumped, good)
- AT28C16 EEPROM (dumped)
- CAT702 TW02 key (dumped)
- CHD hard disk image `primrag2` SHA1 bc615068...

## Emulation status: WORKING

Flags: `MACHINE_SUPPORTS_SAVE` only — **no NOT_WORKING, no protection flag,
no imperfect flags**. The game boots and plays. Historical note: MAME Testers
bug 5614 ("primrag2: Game no longer boots to attract mode") was a regression
later fixed by smf; the set has been promoted for years.

Practical notes for running it:
- It needs the CHD (`primrag2` in a `roms/primrag2/` folder or dir named by
  the DISK_IMAGE), the BIOS set `coh1000w`, and the EPROMs.
- As a location-test build it is rough BY DESIGN: debug text, missing
  animations/balance, test-menu quirks are the game itself, not emulation
  bugs. Only 2 of the planned characters' movesets were near-complete at
  cancellation.
- Because it is PlayStation-architecture, fidelity follows MAME's PSX core
  (same renderer quirks as every zn.cpp game: no texture correction etc. as
  per real PSX).

## Relationship to our Primal Rage 1 work

None technically — different hardware, different driver, no XGA chip, no
encrypted tables; PR2's protection (CAT702) is fully dumped and emulated.
Two loose connections worth knowing:
1. The PR2 *game code* is a PSX-family codebase like the PR1 console port we
   mined for the delta tables; if anyone ever wants to cross-check PR1
   animation data conventions, the PR2 hard disk is another (later) sample of
   Atari's PSX-era data layout.
2. Rumor/lore says PR2's roster reuses PR1 characters as "human avatars";
   the PR1 digitized assets do not appear in PR2's arcade data in any form
   relevant to us.

## If we ever wanted to improve primrag2

Realistic items (none currently planned by us):
- Verify the CHD against a second drive dump (single-source 1GB IDE dumps
  from the 90s are always suspect; no known issues though).
- SLONET/serial linking is unemulated (as on all Atari ZN/linked games).
- uPD78081 internal ROM remains NO_DUMP tree-wide for ZN.
- Anything gameplay-odd should first be compared against the Galloping Ghost
  cabinet footage before being called an emulation bug — it's a 0.36a test
  build.
