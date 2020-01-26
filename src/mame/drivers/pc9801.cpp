// license:BSD-3-Clause
// copyright-holders:Angelo Salese,Carl
/***************************************************************************************************

    PC-9801 (c) 1981 NEC

    driver by Angelo Salese

    TODO:
    - move sound bios ROMs into pc9801_26 / pc9801_86 devices
    - proper 8251 uart hook-up on keyboard
    - SASI/SCSI support;
    - Write a PC80S31K device (also used on PC-8801 and PC-88VA, it's the FDC + Z80 sub-system);
    - Finish DIP-Switches support
    - text scrolling
    - GRCG+
    - rewrite using slot devices
    - some later SWs put "Invalid command byte 05" (Absolutely Mahjong on Epson logo)
    - investigate on POR bit
    - test 2dd more
    - clean-ups/split into devices.

    TODO (PC-9801RS):
    - extra features;
    - keyboard shift doesn't seem to disable properly;
    - clean-up duplicate code;

    TODO (PC-9821):
    - fix CPU for some clones;
    - "cache error"
    - undumped IDE ROM, kludged to work
    - Compatibility is untested;

    TODO (PC-9821AS)
    - IPL ROM banking contradicts greatly from the other machines;

    TODO: (PC-486MU)
    - Tries to read port C of i8255_sys (-> 0x35) at boot without setting up the control
      port. This causes a jump to invalid program area;
    - Dies on ARTIC check;
    - Presumably one ROM is undumped?

    TODO: (PC-9821AP)
    - No way to exit the initial loop. Code looks broken/bad dump?

    floppy issues TODO (* denotes actually fixed, to be moved into specific sheet)
    - 46okunen (DOS not booting / disk swap);
    * ckrynn
    - aishogi: (asserts upon loading, 3'5 image?)
    - akitsuka: (works in PC-9801RS only)
    * alice
    * genghis
    * arcshu
    * arcus2
    * artjigs1 / artjigs2 / artjigs3
    * Atlantia (disk swap?)
    - azusa108 (disk i/o error)
    * bacta2
    - btech (disk swap?)
    - baycity
    - beast (keeps reading command sense)
    * beast2
    * bellsave (disk swap? select B on config menu)
    * biblems2 (at new game loading)
    * birdywld

    * Bokosuka Wars
    * jangou2: floppy fails to load after the title screen;
    - runners (size assert)
    - Sorcerian (2dd image)
    - Twilight Zone 3 (2dd image)

    List of per-game TODO:
    - 4dboxing: inputs are unresponsive;
    - 4dboxing: crashes after user disk creation (regression);
    - agumixsl: non-interlace mode doesn't resize graphics, has rectangle selection bugs (note: needs GDC = 5 MHz to boot);
    - agenesis: fails loading, attempting to read IDE RAM switch port;
    - alice: doesn't set bitmap interlace properly, can't do disk swaps via the File Manager;
    - applecl1: can't pass hands apparently;
    - arctic, fsmoon: Doesn't detect sound board (tied to 0x00ec ports);
    - arcus2: has intro glitches;
    - artjigs*: some text doesn't appear? Namely under the puzzles and when you clear one;
    - atragon: HDD install disk swap doesn't work?
    - asokokof: black screen with BGM, executes invalid opcode (previous note "waits at 0x225f6");
    - arquelph: beeps out at initial sound check,  no voice samples, extra sound board tested;
    - akitsuka: could not setup "initial data" (regression);
    - bandkun: can't install to HDD, has unemulated sound boards in settings (Roland MT-32 & D-10/D-110, Kawai MSB-98, Korg M1, MIDI);
    - biblems2: initial GLODIA logo uses raster effects?
    - bishohzx: Soft House logo uses pseudo-ROZ effect (?), no title screen graphics?
    - bishotsu: beeps out before game (missing sound board?), doesn't draw some text?

    - deflektr: no sound, moans about a DIP-SW setting during loading, has timing issues (keyboard being too fast on PC-9801RS);
    - edge: has gfx glitch when intro scrolls to top-left;
    - edge: user disk creation screen is offset?
    - idolsaga: Moans with a "(program) ended. remove the floppy disk and turn off the power."
    - karateka: no sound;
    - lovelyho: Doesn't show kanjis in PC-9801F version (tries to read them thru the 0xa9 port);
    - madoum1, madoum2, madoum3: doesn't display bitmap gfxs during gameplay;
    - quarth: sound cuts off at title screen, doesn't work on 9801rs (bogus "corrupt .exe" detected);
    - prinmak2, tim: cursor stays stuck when using mouse (works with keyboard);
    - puyopuyo: beeps out when it's supposed to play samples, Not supposed to use ADPCM, is it a PIT issue?
    - runners: wrong double height on the title screen;
    - rusty: black stripes when scrolling;
    - rusty: voice pitches are too slow (tested with -26 and -86);
    - win211: EGC drawing issue (byte wide writes?)
    - win31: doesn't boot at

    per-game TODO (Dounjishi SW):
    - Absolutely Mahjong: Transitions are too fast.

    per-game TODO (PC-9821):
    - Battle Skin Panic: gfx bugs at the Gainax logo, it crashes after it;
    - Policenauts: CD-ROM drive not found;

    Notes:
    - annivers: GRPH (ALT) key cycles through different color schemes (normal, b&w, legacy);
    - Animahjong V3 makes advantage of the possibility of installing 2 sound boards, where SFX and BGMs are played on separate chips.
    - Apple Club 1/2 needs data disks to load properly;
    - Beast Lord: needs a titan.fnt, in MS-DOS
    - fhtag2: product key is 001J0283TA 100001
    - To deprotect BASIC modules set 0xcd7 in ram to 0

========================================================================================

    This series features a huge number of models released between 1982 and 1997. They
    were not IBM PC-compatible, but they had similar hardware (and software: in the
    1990s, they run MS Windows as OS)

    Models:

                      |  CPU                          |   RAM    |            Drives                                     | CBus| Release |
    PC-9801           |  8086 @ 5                     |  128 KB  | -                                                     |  6  | 1982/10 |
    PC-9801F1         |  8086-2 @ 5/8                 |  128 KB  | 5"2DDx1                                               |  4  | 1983/10 |
    PC-9801F2         |  8086-2 @ 5/8                 |  128 KB  | 5"2DDx2                                               |  4  | 1983/10 |
    PC-9801E          |  8086-2 @ 5/8                 |  128 KB  | -                                                     |  6  | 1983/11 |
    PC-9801F3         |  8086-2 @ 5/8                 |  256 KB  | 5"2DDx1, 10M SASI HDD                                 |  2  | 1984/10 |
    PC-9801M2         |  8086-2 @ 5/8                 |  256 KB  | 5"2HDx2                                               |  4  | 1984/11 |
    PC-9801M3         |  8086-2 @ 5/8                 |  256 KB  | 5"2HDx1, 20M SASI HDD                                 |  3  | 1985/02 |
    PC-9801U2         |  V30 @ 8                      |  128 KB  | 3.5"2HDx2                                             |  2  | 1985/05 |
    PC-98XA1          |  80286 @ 8                    |  512 KB  | -                                                     |  6  | 1985/05 |
    PC-98XA2          |  80286 @ 8                    |  512 KB  | 5"2DD/2HDx2                                           |  6  | 1985/05 |
    PC-98XA3          |  80286 @ 8                    |  512 KB  | 5"2DD/2HDx1, 20M SASI HDD                             |  6  | 1985/05 |
    PC-9801VF2        |  V30 @ 8                      |  384 KB  | 5"2DDx2                                               |  4  | 1985/07 |
    PC-9801VM0        |  V30 @ 8/10                   |  384 KB  | -                                                     |  4  | 1985/07 |
    PC-9801VM2        |  V30 @ 8/10                   |  384 KB  | 5"2DD/2HDx2                                           |  4  | 1985/07 |
    PC-9801VM4        |  V30 @ 8/10                   |  384 KB  | 5"2DD/2HDx2, 20M SASI HDD                             |  4  | 1985/10 |
    PC-98XA11         |  80286 @ 8                    |  512 KB  | -                                                     |  6  | 1986/05 |
    PC-98XA21         |  80286 @ 8                    |  512 KB  | 5"2DD/2HDx2                                           |  6  | 1986/05 |
    PC-98XA31         |  80286 @ 8                    |  512 KB  | 5"2DD/2HDx1, 20M SASI HDD                             |  6  | 1986/05 |
    PC-9801UV2        |  V30 @ 8/10                   |  384 KB  | 3.5"2DD/2HDx2                                         |  2  | 1986/05 |
    PC-98LT1          |  V50 @ 8                      |  384 KB  | 3.5"2DD/2HDx1                                         |  0  | 1986/11 |
    PC-98LT2          |  V50 @ 8                      |  384 KB  | 3.5"2DD/2HDx1                                         |  0  | 1986/11 |
    PC-9801VM21       |  V30 @ 8/10                   |  640 KB  | 5"2DD/2HDx2                                           |  4  | 1986/11 |
    PC-9801VX0        |  80286 @ 8 & V30 @ 8/10       |  640 KB  | -                                                     |  4  | 1986/11 |
    PC-9801VX2        |  80286 @ 8 & V30 @ 8/10       |  640 KB  | 5"2DD/2HDx2                                           |  4  | 1986/11 |
    PC-9801VX4        |  80286 @ 8 & V30 @ 8/10       |  640 KB  | 5"2DD/2HDx2, 20M SASI HDD                             |  4  | 1986/11 |
    PC-9801VX4/WN     |  80286 @ 8 & V30 @ 8/10       |  640 KB  | 5"2DD/2HDx2, 20M SASI HDD                             |  4  | 1986/11 |
    PC-98XL1          |  80286 @ 8 & V30 @ 8/10       | 1152 KB  | -                                                     |  4  | 1986/12 |
    PC-98XL2          |  80286 @ 8 & V30 @ 8/10       | 1152 KB  | 5"2DD/2HDx2                                           |  4  | 1986/12 |
    PC-98XL4          |  80286 @ 8 & V30 @ 8/10       | 1152 KB  | 5"2DD/2HDx1, 20M SASI HDD                             |  4  | 1986/12 |
    PC-9801VX01       |  80286-10 @ 8/10 & V30 @ 8/10 |  640 KB  | -                                                     |  4  | 1987/06 |
    PC-9801VX21       |  80286-10 @ 8/10 & V30 @ 8/10 |  640 KB  | 5"2DD/2HDx2                                           |  4  | 1987/06 |
    PC-9801VX41       |  80286-10 @ 8/10 & V30 @ 8/10 |  640 KB  | 5"2DD/2HDx2, 20M SASI HDD                             |  4  | 1987/06 |
    PC-9801UV21       |  V30 @ 8/10                   |  640 KB  | 3.5"2DD/2HDx2                                         |  2  | 1987/06 |
    PC-98XL^2         |  i386DX-16 @ 16 & V30 @ 8     |  1.6 MB  | 5"2DD/2HDx2, 40M SASI HDD                             |  4  | 1987/10 |
    PC-98LT11         |  V50 @ 8                      |  640 KB  | 3.5"2DD/2HDx1                                         |  0  | 1987/10 |
    PC-98LT21         |  V50 @ 8                      |  640 KB  | 3.5"2DD/2HDx1                                         |  0  | 1987/10 |
    PC-9801UX21       |  80286-10 @ 10 & V30 @ 8      |  640 KB  | 3.5"2DD/2HDx2                                         |  3  | 1987/10 |
    PC-9801UX41       |  80286-10 @ 10 & V30 @ 8      |  640 KB  | 3.5"2DD/2HDx2, 20M SASI HDD                           |  3  | 1987/10 |
    PC-9801LV21       |  V30 @ 8/10                   |  640 KB  | 3.5"2DD/2HDx2                                         |  0  | 1988/03 |
    PC-9801CV21       |  V30 @ 8/10                   |  640 KB  | 3.5"2DD/2HDx2                                         |  2  | 1988/03 |
    PC-9801UV11       |  V30 @ 8/10                   |  640 KB  | 3.5"2DD/2HDx2                                         |  2  | 1988/03 |
    PC-9801RA2        |  i386DX-16 @ 16 & V30 @ 8     |  1.6 MB  | 5"2DD/2HDx2                                           |  4  | 1988/07 |
    PC-9801RA5        |  i386DX-16 @ 16 & V30 @ 8     |  1.6 MB  | 5"2DD/2HDx2, 40M SASI HDD                             |  4  | 1988/07 |
    PC-9801RX2        |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 5"2DD/2HDx2                                           |  4  | 1988/07 |
    PC-9801RX4        |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 5"2DD/2HDx2, 20M SASI HDD                             |  4  | 1988/07 |
    PC-98LT22         |  V50 @ 8                      |  640 KB  | 3.5"2DD/2HDx1                                         |  0  | 1988/11 |
    PC-98LS2          |  i386SX-16 @ 16 & V30 @ 8     |  1.6 MB  | 5"2DD/2HDx2                                           |  0  | 1988/11 |
    PC-98LS5          |  i386SX-16 @ 16 & V30 @ 8     |  1.6 MB  | 5"2DD/2HDx2, 40M SASI HDD                             |  0  | 1988/11 |
    PC-9801VM11       |  V30 @ 8/10                   |  640 KB  | 5"2DD/2HDx2                                           |  4  | 1988/11 |
    PC-9801LV22       |  V30 @ 8/10                   |  640 KB  | 3.5"2DD/2HDx2                                         |  0  | 1989/01 |
    PC-98RL2          |  i386DX-20 @ 16/20 & V30 @ 8  |  1.6 MB  | 5"2DD/2HDx2                                           |  4  | 1989/02 |
    PC-98RL5          |  i386DX-20 @ 16/20 & V30 @ 8  |  1.6 MB  | 5"2DD/2HDx2, 40M SASI HDD                             |  4  | 1989/02 |
    PC-9801EX2        |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 3.5"2DD/2HDx2                                         |  3  | 1989/04 |
    PC-9801EX4        |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 3.5"2DD/2HDx2, 20M SASI HDD                           |  3  | 1989/04 |
    PC-9801ES2        |  i386SX-16 @ 16 & V30 @ 8     |  1.6 MB  | 3.5"2DD/2HDx2                                         |  3  | 1989/04 |
    PC-9801ES5        |  i386SX-16 @ 16 & V30 @ 8     |  1.6 MB  | 3.5"2DD/2HDx2, 40M SASI HDD                           |  3  | 1989/04 |
    PC-9801LX2        |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 3.5"2DD/2HDx2                                         |  0  | 1989/04 |
    PC-9801LX4        |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 3.5"2DD/2HDx2, 20M SASI HDD                           |  0  | 1989/04 |
    PC-9801LX5        |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 3.5"2DD/2HDx2, 40M SASI HDD                           |  0  | 1989/06 |
    PC-98DO           |  V30 @ 8/10                   |  640 KB  | 5"2DD/2HDx2                                           |  1  | 1989/06 |
    PC-9801LX5C       |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 3.5"2DD/2HDx2, 40M SASI HDD                           |  0  | 1989/06 |
    PC-9801RX21       |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 5"2DD/2HDx2                                           |  4  | 1989/10 |
    PC-9801RX51       |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 5"2DD/2HDx2, 40M SASI HDD                             |  4  | 1989/10 |
    PC-9801RA21       |  i386DX-20 @ 16/20 & V30 @ 8  |  1.6 MB  | 5"2DD/2HDx2                                           |  4  | 1989/11 |
    PC-9801RA51       |  i386DX-20 @ 16/20 & V30 @ 8  |  1.6 MB  | 5"2DD/2HDx2, 40M SASI HDD                             |  4  | 1989/11 |
    PC-9801RS21       |  i386SX-16 @ 16 & V30 @ 8     |  640 KB  | 5"2DD/2HDx2                                           |  4  | 1989/11 |
    PC-9801RS51       |  i386SX-16 @ 16 & V30 @ 8     |  640 KB  | 5"2DD/2HDx2, 40M SASI HDD                             |  4  | 1989/11 |
    PC-9801N          |  V30 @ 10                     |  640 KB  | 3.5"2DD/2HDx1                                         |  0  | 1989/11 |
    PC-9801TW2        |  i386SX-20 @ 20 & V30 @ 8     |  640 KB  | 3.5"2DD/2HDx2                                         |  2  | 1990/02 |
    PC-9801TW5        |  i386SX-20 @ 20 & V30 @ 8     |  1.6 MB  | 3.5"2DD/2HDx2, 40M SASI HDD                           |  2  | 1990/02 |
    PC-9801TS5        |  i386SX-20 @ 20 & V30 @ 8     |  1.6 MB  | 3.5"2DD/2HDx2, 40M SASI HDD                           |  2  | 1990/06 |
    PC-9801NS         |  i386SX-12 @ 12               |  1.6 MB  | 3.5"2DD/2HDx1                                         |  0  | 1990/06 |
    PC-9801TF5        |  i386SX-20 @ 20 & V30 @ 8     |  1.6 MB  | 3.5"2DD/2HDx2, 40M SASI HDD                           |  2  | 1990/07 |
    PC-9801NS-20      |  i386SX-12 @ 12               |  1.6 MB  | 3.5"2DD/2HDx1, 20M SASI HDD                           |  0  | 1990/09 |
    PC-98RL21         |  i386DX-20 @ 20 & V30 @ 8     |  1.6 MB  | 5"2DD/2HDx2                                           |  4  | 1990/09 |
    PC-98RL51         |  i386DX-20 @ 20 & V30 @ 8     |  1.6 MB  | 5"2DD/2HDx1, 40M SASI HDD                             |  4  | 1990/09 |
    PC-98DO+          |  V33A @ 8/16                  |  640 KB  | 5"2DD/2HDx2                                           |  1  | 1990/10 |
    PC-9801DX2        |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 5"2DD/2HDx2                                           |  4  | 1990/11 |
    PC-9801DX/U2      |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 3.5"2DD/2HDx2                                         |  4  | 1990/11 |
    PC-9801DX5        |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 5"2DD/2HDx2, 40M SASI HDD                             |  4  | 1990/11 |
    PC-9801DX/U5      |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 3.5"2DD/2HDx2, 40M SASI HDD                           |  4  | 1990/11 |
    PC-9801NV         |  V30HL @ 8/16                 |  1.6 MB  | 3.5"2DD/2HDx1                                         |  0  | 1990/11 |
    PC-9801DS2        |  i386SX-16 @ 16 & V30 @ 8     |  640 KB  | 5"2DD/2HDx2                                           |  4  | 1991/01 |
    PC-9801DS/U2      |  i386SX-16 @ 16 & V30 @ 8     |  640 KB  | 3.5"2DD/2HDx2                                         |  4  | 1991/01 |
    PC-9801DS5        |  i386SX-16 @ 16 & V30 @ 8     |  640 KB  | 5"2DD/2HDx2, 40M SASI HDD                             |  4  | 1991/01 |
    PC-9801DS/U5      |  i386SX-16 @ 16 & V30 @ 8     |  640 KB  | 3.5"2DD/2HDx2, 40M SASI HDD                           |  4  | 1991/01 |
    PC-9801DA2        |  i386DX-20 @ 16/20 & V30 @ 8  |  1.6 MB  | 5"2DD/2HDx2                                           |  4  | 1991/01 |
    PC-9801DA/U2      |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 3.5"2DD/2HDx2                                         |  4  | 1991/01 |
    PC-9801DA5        |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 5"2DD/2HDx2, 40M SASI HDD                             |  4  | 1991/01 |
    PC-9801DA/U5      |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 3.5"2DD/2HDx2, 40M SASI HDD                           |  4  | 1991/01 |
    PC-9801DA7        |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 5"2DD/2HDx2, 100M SCSI HDD                            |  4  | 1991/02 |
    PC-9801DA/U7      |  80286-12 @ 10/12 & V30 @ 8   |  640 KB  | 3.5"2DD/2HDx2, 100M SCSI HDD                          |  4  | 1991/02 |
    PC-9801UF         |  V30 @ 8/16                   |  640 KB  | 3.5"2DD/2HDx2                                         |  2  | 1991/02 |
    PC-9801UR         |  V30 @ 8/16                   |  640 KB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk                        |  2  | 1991/02 |
    PC-9801UR/20      |  V30 @ 8/16                   |  640 KB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk, 20M SASI HDD          |  2  | 1991/02 |
    PC-9801NS/E       |  i386SX-16 @ 16               |  1.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk                        |  0  | 1991/06 |
    PC-9801NS/E20     |  i386SX-16 @ 16               |  1.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk, 20M SASI HDD          |  0  | 1991/06 |
    PC-9801NS/E40     |  i386SX-16 @ 16               |  1.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk, 40M SASI HDD          |  0  | 1991/06 |
    PC-9801TW7        |  i386SX-20 @ 20 & V30 @ 8     |  1.6 MB  | 3.5"2DD/2HDx2, 100M SCSI HDD                          |  2  | 1991/07 |
    PC-9801TF51       |  i386SX-20 @ 20 & V30 @ 8     |  1.6 MB  | 3.5"2DD/2HDx2, 40M SASI HDD                           |  2  | 1991/07 |
    PC-9801TF71       |  i386SX-20 @ 20 & V30 @ 8     |  1.6 MB  | 3.5"2DD/2HDx2, 100M SCSI HDD                          |  2  | 1991/07 |
    PC-9801NC         |  i386SX-20 @ 20               |  2.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk                        |  0  | 1991/10 |
    PC-9801NC40       |  i386SX-20 @ 20               |  2.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk, 40M SASI HDD          |  0  | 1991/10 |
    PC-9801CS2        |  i386SX-16 @ 16               |  640 KB  | 3.5"2DD/2HDx2                                         |  2  | 1991/10 |
    PC-9801CS5        |  i386SX-16 @ 16               |  640 KB  | 3.5"2DD/2HDx2, 40M SASI HDD                           |  2  | 1991/10 |
    PC-9801CS5/W      |  i386SX-16 @ 16               |  3.6 MB  | 3.5"2DD/2HDx2, 40M SASI HDD                           |  2  | 1991/11 |
    PC-98GS1          |  i386SX-20 @ 20 & V30 @ 8     |  2.6 MB  | 3.5"2DD/2HDx2, 40M SASI HDD                           |  3  | 1991/11 |
    PC-98GS2          |  i386SX-20 @ 20 & V30 @ 8     |  2.6 MB  | 3.5"2DD/2HDx2, 40M SASI HDD, 1xCD-ROM                 |  3  | 1991/11 |
    PC-9801FA2        |  i486SX-16 @ 16               |  1.6 MB  | 5"2DD/2HDx2                                           |  4  | 1992/01 |
    PC-9801FA/U2      |  i486SX-16 @ 16               |  1.6 MB  | 3.5"2DD/2HDx2                                         |  4  | 1992/01 |
    PC-9801FA5        |  i486SX-16 @ 16               |  1.6 MB  | 5"2DD/2HDx2, 40M SCSI HDD                             |  4  | 1992/01 |
    PC-9801FA/U5      |  i486SX-16 @ 16               |  1.6 MB  | 3.5"2DD/2HDx2, 40M SCSI HDD                           |  4  | 1992/01 |
    PC-9801FA7        |  i486SX-16 @ 16               |  1.6 MB  | 5"2DD/2HDx2, 100M SCSI HDD                            |  4  | 1992/01 |
    PC-9801FA/U7      |  i486SX-16 @ 16               |  1.6 MB  | 3.5"2DD/2HDx2, 100M SCSI HDD                          |  4  | 1992/01 |
    PC-9801FS2        |  i386SX-20 @ 16/20            |  1.6 MB  | 5"2DD/2HDx2                                           |  4  | 1992/05 |
    PC-9801FS/U2      |  i386SX-20 @ 16/20            |  1.6 MB  | 3.5"2DD/2HDx2                                         |  4  | 1992/05 |
    PC-9801FS5        |  i386SX-20 @ 16/20            |  1.6 MB  | 5"2DD/2HDx2, 40M SCSI HDD                             |  4  | 1992/05 |
    PC-9801FS/U5      |  i386SX-20 @ 16/20            |  1.6 MB  | 3.5"2DD/2HDx2, 40M SCSI HDD                           |  4  | 1992/05 |
    PC-9801FS7        |  i386SX-20 @ 16/20            |  1.6 MB  | 5"2DD/2HDx2, 100M SCSI HDD                            |  4  | 1992/01 |
    PC-9801FS/U7      |  i386SX-20 @ 16/20            |  1.6 MB  | 3.5"2DD/2HDx2, 100M SCSI HDD                          |  4  | 1992/01 |
    PC-9801NS/T       |  i386SL(98) @ 20              |  1.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk                        |  0  | 1992/01 |
    PC-9801NS/T40     |  i386SL(98) @ 20              |  1.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk, 40M SASI HDD          |  0  | 1992/01 |
    PC-9801NS/T80     |  i386SL(98) @ 20              |  1.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk, 80M SASI HDD          |  0  | 1992/01 |
    PC-9801NL         |  V30H @ 8/16                  |  640 KB  | 1.25 MB RAM Disk                                      |  0  | 1992/01 |
    PC-9801FX2        |  i386SX-12 @ 10/12            |  1.6 MB  | 5"2DD/2HDx2                                           |  4  | 1992/05 |
    PC-9801FX/U2      |  i386SX-12 @ 10/12            |  1.6 MB  | 3.5"2DD/2HDx2                                         |  4  | 1992/05 |
    PC-9801FX5        |  i386SX-12 @ 10/12            |  1.6 MB  | 5"2DD/2HDx2, 40M SCSI HDD                             |  4  | 1992/05 |
    PC-9801FX/U5      |  i386SX-12 @ 10/12            |  1.6 MB  | 3.5"2DD/2HDx2, 40M SCSI HDD                           |  4  | 1992/05 |
    PC-9801US         |  i386SX-16 @ 16               |  1.6 MB  | 3.5"2DD/2HDx2                                         |  2  | 1992/07 |
    PC-9801US40       |  i386SX-16 @ 16               |  1.6 MB  | 3.5"2DD/2HDx2, 40M SASI HDD                           |  2  | 1992/07 |
    PC-9801US80       |  i386SX-16 @ 16               |  1.6 MB  | 3.5"2DD/2HDx2, 80M SASI HDD                           |  2  | 1992/07 |
    PC-9801NS/L       |  i386SX-20 @ 10/20            |  1.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk                        |  0  | 1992/07 |
    PC-9801NS/L40     |  i386SX-20 @ 10/20            |  1.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk, 40M SASI HDD          |  0  | 1992/07 |
    PC-9801NA         |  i486SX-20 @ 20               |  2.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk                        |  0  | 1992/11 |
    PC-9801NA40       |  i486SX-20 @ 20               |  2.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk, 40M SASI HDD          |  0  | 1992/11 |
    PC-9801NA120      |  i486SX-20 @ 20               |  2.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk, 120M SASI HDD         |  0  | 1992/11 |
    PC-9801NA/C       |  i486SX-20 @ 20               |  2.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk                        |  0  | 1992/11 |
    PC-9801NA40/C     |  i486SX-20 @ 20               |  2.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk, 40M SASI HDD          |  0  | 1992/11 |
    PC-9801NA120/C    |  i486SX-20 @ 20               |  2.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk, 120M SASI HDD         |  0  | 1992/11 |
    PC-9801NS/R       |  i486SX(J) @ 20               |  1.6 MB  | 3.5"2DD/2HDx1 (3mode), 1.25MB RAM Disk                |  0  | 1993/01 |
    PC-9801NS/R40     |  i486SX(J) @ 20               |  1.6 MB  | 3.5"2DD/2HDx1 (3mode), 1.25MB RAM Disk, 40M SASI HDD  |  0  | 1993/01 |
    PC-9801NS/R120    |  i486SX(J) @ 20               |  1.6 MB  | 3.5"2DD/2HDx1 (3mode), 1.25MB RAM Disk, 120M SASI HDD |  0  | 1993/01 |
    PC-9801BA/U2      |  i486DX2-40 @ 40              |  1.6 MB  | 3.5"2DD/2HDx2                                         |  3  | 1993/01 |
    PC-9801BA/U6      |  i486DX2-40 @ 40              |  3.6 MB  | 3.5"2DD/2HDx1, 40M SASI HDD                           |  3  | 1993/01 |
    PC-9801BA/M2      |  i486DX2-40 @ 40              |  1.6 MB  | 5"2DD/2HDx2                                           |  3  | 1993/01 |
    PC-9801BX/U2      |  i486SX-20 @ 20               |  1.6 MB  | 3.5"2DD/2HDx2                                         |  3  | 1993/01 |
    PC-9801BX/U6      |  i486SX-20 @ 20               |  3.6 MB  | 3.5"2DD/2HDx1, 40M SASI HDD                           |  3  | 1993/01 |
    PC-9801BX/M2      |  i486SX-20 @ 20               |  1.6 MB  | 5"2DD/2HDx2                                           |  3  | 1993/01 |
    PC-9801NX/C       |  i486SX(J) @ 20               |  1.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk                        |  0  | 1993/07 |
    PC-9801NX/C120    |  i486SX(J) @ 20               |  3.6 MB  | 3.5"2DD/2HDx1, 1.25MB RAM Disk                        |  0  | 1993/07 |
    PC-9801P40/D      |  i486SX(J) @ 20               |  5.6 MB  | 40MB IDE HDD                                          |  0  | 1993/07 |
    PC-9801P80/W      |  i486SX(J) @ 20               |  7.6 MB  | 80MB IDE HDD                                          |  0  | 1993/07 |
    PC-9801P80/P      |  i486SX(J) @ 20               |  7.6 MB  | 80MB IDE HDD                                          |  0  | 1993/07 |
    PC-9801BA2/U2     |  i486DX2-66 @ 66              |  3.6 MB  | 3.5"2DD/2HDx2                                         |  3  | 1993/11 |
    PC-9801BA2/U7     |  i486DX2-66 @ 66              |  3.6 MB  | 3.5"2DD/2HDx1, 120MB IDE HDD                          |  3  | 1993/11 |
    PC-9801BA2/M2     |  i486DX2-66 @ 66              |  3.6 MB  | 5"2DD/2HDx2                                           |  3  | 1993/11 |
    PC-9801BS2/U2     |  i486SX-33 @ 33               |  3.6 MB  | 3.5"2DD/2HDx2                                         |  3  | 1993/11 |
    PC-9801BS2/U7     |  i486SX-33 @ 33               |  3.6 MB  | 3.5"2DD/2HDx1, 120MB IDE HDD                          |  3  | 1993/11 |
    PC-9801BS2/M2     |  i486SX-33 @ 33               |  3.6 MB  | 5"2DD/2HDx2                                           |  3  | 1993/11 |
    PC-9801BX2/U2     |  i486SX-25 @ 25               |  1.8 MB  | 3.5"2DD/2HDx2                                         |  3  | 1993/11 |
    PC-9801BX2/U7     |  i486SX-25 @ 25               |  3.6 MB  | 3.5"2DD/2HDx1, 120MB IDE HDD                          |  3  | 1993/11 |
    PC-9801BX2/M2     |  i486SX-25 @ 25               |  1.8 MB  | 5"2DD/2HDx2                                           |  3  | 1993/11 |
    PC-9801BA3/U2     |  i486DX-66 @ 66               |  3.6 MB  | 3.5"2DD/2HDx2                                         |  3  | 1995/01 |
    PC-9801BA3/U2/W   |  i486DX-66 @ 66               |  7.6 MB  | 3.5"2DD/2HDx2, 210MB IDE HDD                          |  3  | 1995/01 |
    PC-9801BX3/U2     |  i486SX-33 @ 33               |  1.6 MB  | 3.5"2DD/2HDx2                                         |  3  | 1995/01 |
    PC-9801BX3/U2/W   |  i486SX-33 @ 33               |  5.6 MB  | 3.5"2DD/2HDx2, 210MB IDE HDD                          |  3  | 1995/01 |
    PC-9801BX4/U2     |  AMD/i 486DX2-66 @ 66         |  2 MB    | 3.5"2DD/2HDx2                                         |  3  | 1995/07 |
    PC-9801BX4/U2/C   |  AMD/i 486DX2-66 @ 66         |  2 MB    | 3.5"2DD/2HDx2, 2xCD-ROM                               |  3  | 1995/07 |
    PC-9801BX4/U2-P   |  Pentium ODP @ 66             |  2 MB    | 3.5"2DD/2HDx2                                         |  3  | 1995/09 |
    PC-9801BX4/U2/C-P |  Pentium ODP @ 66             |  2 MB    | 3.5"2DD/2HDx2, 2xCD-ROM                               |  3  | 1995/09 |

    For more info (e.g. optional hardware), see http://www.geocities.jp/retro_zzz/machines/nec/9801/mdl98cpu.html


    PC-9821 Series

    PC-9821 (1992) - aka 98MULTi, desktop computer, 386 based
    PC-9821A series (1993->1994) - aka 98MATE A, desktop computers, 486 based
    PC-9821B series (1993) - aka 98MATE B, desktop computers, 486 based
    PC-9821C series (1993->1996) - aka 98MULTi CanBe, desktop & tower computers, various CPU
    PC-9821Es (1994) - aka 98FINE, desktop computer with integrated LCD, successor of the PC-98T
    PC-9821X series (1994->1995) - aka 98MATE X, desktop computers, Pentium based
    PC-9821V series (1995) - aka 98MATE Valuestar, desktop computers, Pentium based
    PC-9821S series (1995->2996) - aka 98Pro, tower computers, PentiumPro based
    PC-9821R series (1996->2000) - aka 98MATE R, desktop & tower & server computers, various CPU
    PC-9821C200 (1997) - aka CEREB, desktop computer, Pentium MMX based
    PC-9821 Ne/Ns/Np/Nm (1993->1995) - aka 98NOTE, laptops, 486 based
    PC-9821 Na/Nb/Nw (1995->1997) - aka 98NOTE Lavie, laptops, Pentium based
    PC-9821 Lt/Ld (1995) - aka 98NOTE Light, laptops, 486 based
    PC-9821 La/Ls (1995->1997) - aka 98NOTE Aile, laptops, Pentium based

====

Documentation notes (for unemulated stuff, courtesy of T. Kodaka and T. Kono):

IDE:
(r/w)
0x430: IDE drive switch
0x432: IDE drive switch
0x435: <unknown>

                                                    (ISA correlated i/o)
----------------------------------------------------------
0x0640      |WORD|R/W|Data Register                |01F0h
0x0642      |BYTE| R |Error Register               |01F1h
0x0642      |BYTE| W |Write Precomp Register       |01F1h
0x0644      |BYTE|R/W|Sector Count                 |01F2h
0x0646      |BYTE|R/W|Sector Number                |01F3h
0x0648      |BYTE|R/W|Cylinder Low                 |01F4h
0x064A      |BYTE|R/W|Cylinder High                |01F5h
0x064C      |BYTE|R/W|SDH Register                 |01F6h
0x064E      |BYTE| R |Status Register              |01F7h
0x064E      |BYTE| W |Command Register             |01F7h
0x074C      |BYTE| R |Alternate Status Register    |03F6h
0x074C      |BYTE| W |Digital Output Register      |03F6h
0x074E      |BYTE| R |Digital Input Register       |03F7h

Video F/F (i/o 0x68):
KAC mode (video ff = 5) is basically how the kanji ROM could be accessed, 1=thru the CG window ports, 0=thru the kanji
window RAM at 0xa4***.
My guess is that the system locks up or doesn't have any data if the wrong port is being accessed.

Ext Video F/F (i/o 0x6a):
0000 011x enables EGC
0000 111x enables PC-98GS
0010 000x enables multicolor (a.k.a. 256 colors mode)
0010 001x enables 65'536 colors
0010 010x 64k color palette related (?)
0010 011x full screen reverse (?)
0010 100x text and gfxs synthesis (?)
0010 101x 256 color palette registers fast write (?)
0010 110x 256 color overscan (?)
0100 000x (0) CRT (1) Plasma/LCD
0100 001x text and gfxs right shifted one dot (undocumented behaviour)
0100 010x hi-res mode in PC-9821
0110 000x EEGC mode
0110 001x VRAM config (0) plain (1) packed
0110 011x AGDC mode
0110 100x 480 lines
0110 110x VRAM bitmap orientation (0) MSB left-to-right LSB (1) LSB left-to-right MSB
1000 001x CHR GDC clock (0) 2,5 MHz (1) 5 MHz
1000 010x BMP GDC clock
1000 111x related to GFX accelerator cards (like Vision864)
1100 010x chart GDC operating mode (?)
(everything else is undocumented / unknown)

Keyboard TX commands:
0xfa ACK
0xfc NACK
0x95
---- --xx extension key settings (00 normal 11 Win and App Keys enabled)
0x96 identification codes
0x9c
-xx- ---- key delay (11 = 1000 ms, 10 = 500 ms, 01 = 500 ms, 00 = 250 ms)
---x xxxx repeat rate (slow 11111 -> 00001 fast)
0x9d keyboard LED settings
0x9f keyboard ID

****************************************************************************************************/

#include "emu.h"
#include "includes/pc9801.h"
#include "machine/input_merger.h"



WRITE8_MEMBER(pc9801_state::rtc_w)
{
	m_rtc->c0_w((data & 0x01) >> 0);
	m_rtc->c1_w((data & 0x02) >> 1);
	m_rtc->c2_w((data & 0x04) >> 2);
	m_rtc->stb_w((data & 0x08) >> 3);
	m_rtc->clk_w((data & 0x10) >> 4);
	m_rtc->data_in_w(((data & 0x20) >> 5));
	if(data & 0xc0)
		logerror("RTC write to undefined bits %02x\n",data & 0xc0);
}

WRITE8_MEMBER(pc9801_state::dmapg4_w)
{
	if(offset < 4)
		m_dma_offset[(offset+1) & 3] = data & 0x0f;
}

WRITE8_MEMBER(pc9801_state::dmapg8_w)
{
	if(offset == 4)
		m_dma_autoinc[data & 3] = (data >> 2) & 3;
	else if(offset < 4)
		m_dma_offset[(offset+1) & 3] = data;
}

WRITE8_MEMBER(pc9801_state::nmi_ctrl_w)
{
	m_nmi_ff = offset;
}

WRITE8_MEMBER(pc9801_state::vrtc_clear_w)
{
	m_pic1->ir2_w(0);
}

READ8_MEMBER(pc9801_state::fdc_2hd_ctrl_r)
{
	return 0x44; //unknown port meaning 2hd flag?
}

WRITE8_MEMBER(pc9801_state::fdc_2hd_ctrl_w)
{
	//logerror("%02x ctrl\n",data);
	if(((m_fdc_2hd_ctrl & 0x80) == 0) && (data & 0x80))
		m_fdc_2hd->soft_reset();

	m_fdc_2hd_ctrl = data;

	if(data & 0x40)
	{
		m_fdc_2hd->set_ready_line_connected(0);
		m_fdc_2hd->ready_w(0);
	}
	else
		m_fdc_2hd->set_ready_line_connected(1);

	if(!m_sys_type) // required for 9801f 2hd adapter bios
	{
		m_fdc_2hd->subdevice<floppy_connector>("0")->get_device()->mon_w(data & 8 ? ASSERT_LINE : CLEAR_LINE);
		m_fdc_2hd->subdevice<floppy_connector>("1")->get_device()->mon_w(data & 8 ? ASSERT_LINE : CLEAR_LINE);
	}
	else if(!(m_fdc_ctrl & 4)) // required for 9821
	{
		m_fdc_2hd->subdevice<floppy_connector>("0")->get_device()->mon_w(data & 8 ? CLEAR_LINE : ASSERT_LINE);
		m_fdc_2hd->subdevice<floppy_connector>("1")->get_device()->mon_w(data & 8 ? CLEAR_LINE : ASSERT_LINE);
	}
}


READ8_MEMBER(pc9801_state::fdc_2dd_ctrl_r)
{
		int ret = (!m_fdc_2dd->subdevice<floppy_connector>("0")->get_device()->ready_r()) ? 0x10 : 0;
		ret |= (m_fdc_2dd->subdevice<floppy_connector>("1")->get_device()->ready_r()) ? 0x10 : 0;
		return ret | 0x40; //unknown port meaning, might be 0x70
}

WRITE8_MEMBER(pc9801_state::fdc_2dd_ctrl_w)
{
	logerror("%02x ctrl\n",data);
	if(((m_fdc_2dd_ctrl & 0x80) == 0) && (data & 0x80))
		m_fdc_2dd->soft_reset();

	m_fdc_2dd_ctrl = data;
	m_fdc_2dd->subdevice<floppy_connector>("0")->get_device()->mon_w(data & 8 ? CLEAR_LINE : ASSERT_LINE);
	m_fdc_2dd->subdevice<floppy_connector>("1")->get_device()->mon_w(data & 8 ? CLEAR_LINE : ASSERT_LINE);
}

READ8_MEMBER(pc9801_state::ide_ctrl_r)
{
	address_space &ram = m_maincpu->space(AS_PROGRAM);
	// this makes the ide driver not do 512 to 256 byte sector translation, the 9821 looks for bit 6 of offset 0xac403 of the kanji ram to set this, the rs unknown
	ram.write_byte(0x457, ram.read_byte(0x457) | 0xc0);
	return m_ide_sel;
}

WRITE8_MEMBER(pc9801_state::ide_ctrl_w)
{
	if(!(data & 0x80))
		m_ide_sel = data & 1;
}

READ16_MEMBER(pc9801_state::ide_cs0_r)
{
	return m_ide[m_ide_sel]->read_cs0(offset, mem_mask);
}

WRITE16_MEMBER(pc9801_state::ide_cs0_w)
{
	m_ide[m_ide_sel]->write_cs0(offset, data, mem_mask);
}

READ16_MEMBER(pc9801_state::ide_cs1_r)
{
	return m_ide[m_ide_sel]->read_cs1(offset, mem_mask);
}

WRITE16_MEMBER(pc9801_state::ide_cs1_w)
{
	m_ide[m_ide_sel]->write_cs1(offset, data, mem_mask);
}

READ8_MEMBER( pc9801_state::sasi_data_r )
{
	uint8_t data = m_sasi_data_in->read();

	if(m_sasi_ctrl_in->read() & 0x80)
		m_sasibus->write_ack(1);
	return data;
}

WRITE8_MEMBER( pc9801_state::sasi_data_w )
{
	m_sasi_data = data;

	if (m_sasi_data_enable)
	{
		m_sasi_data_out->write(m_sasi_data);
		if(m_sasi_ctrl_in->read() & 0x80)
			m_sasibus->write_ack(1);
	}
}

WRITE_LINE_MEMBER( pc9801_state::write_sasi_io )
{
	m_sasi_ctrl_in->write_bit2(state);

	m_sasi_data_enable = !state;

	if (m_sasi_data_enable)
	{
		m_sasi_data_out->write(m_sasi_data);
	}
	else
	{
		m_sasi_data_out->write(0);
	}
	if((m_sasi_ctrl_in->read() & 0x9C) == 0x8C)
		m_pic2->ir1_w(m_sasi_ctrl & 1);
	else
		m_pic2->ir1_w(0);
}

WRITE_LINE_MEMBER( pc9801_state::write_sasi_req )
{
	m_sasi_ctrl_in->write_bit7(state);

	if (!state)
		m_sasibus->write_ack(0);

	if((m_sasi_ctrl_in->read() & 0x9C) == 0x8C)
		m_pic2->ir1_w(m_sasi_ctrl & 1);
	else
		m_pic2->ir1_w(0);

	m_dmac->dreq0_w(!(state && !(m_sasi_ctrl_in->read() & 8) && (m_sasi_ctrl & 2)));
}


READ8_MEMBER( pc9801_state::sasi_status_r )
{
	uint8_t res = 0;

	if(m_sasi_ctrl & 0x40) // read status
	{
	/*
	    x--- ---- REQ
	    -x-- ---- ACK
	    --x- ---- BSY
	    ---x ---- MSG
	    ---- x--- CD
	    ---- -x-- IO
	    ---- ---x INT?
	*/
		res |= m_sasi_ctrl_in->read();
	}
	else // read drive info
	{
/*
        xx-- ---- unknown but tested
        --xx x--- SASI-1 media type
        ---- -xxx SASI-2 media type
*/
		//res |= 7 << 3; // read mediatype SASI-1
		//res |= 7;   // read mediatype SASI-2
	}
	return res;
}

WRITE8_MEMBER( pc9801_state::sasi_ctrl_w )
{
	/*
	    x--- ---- channel enable
	    -x-- ---- read switch
	    --x- ---- sel
	    ---- x--- reset line
	    ---- --x- dma enable
	    ---- ---x irq enable
	*/

	m_sasibus->write_sel(BIT(data, 5));

	if(m_sasi_ctrl & 8 && ((data & 8) == 0)) // 1 -> 0 transition
	{
		m_sasibus->write_rst(1);
//      m_timer_rst->adjust(attotime::from_nsec(100));
	}
	else
		m_sasibus->write_rst(0); // TODO

	m_sasi_ctrl = data;

//  m_sasibus->write_sel(BIT(data, 0));
}

READ8_MEMBER(pc9801_state::f0_r)
{
	if(offset == 0)
	{
		// iterate thru all devices to check if an AMD98 is present
		for (pc9801_amd98_device &amd98 : device_type_iterator<pc9801_amd98_device>(machine().root_device()))
		{
			logerror("Read AMD98 ID %s\n",amd98.tag());
			return 0x18; // return the right ID
		}

		logerror("Read port 0 from 0xf0 (AMD98 check?)\n");
		return 0; // card not present
	}

	return 0xff;
}

void pc9801_state::pc9801_map(address_map &map)
{
	map(0xa0000, 0xa3fff).rw(FUNC(pc9801_state::tvram_r), FUNC(pc9801_state::tvram_w)); //TVRAM
	map(0xa8000, 0xbffff).rw(FUNC(pc9801_state::gvram_r), FUNC(pc9801_state::gvram_w)); //bitmap VRAM
//  map(0xcc000, 0xcffff).rom().region("sound_bios", 0); //sound BIOS
	map(0xd6000, 0xd6fff).rom().region("fdc_bios_2dd", 0); //floppy BIOS 2dd
	map(0xd7000, 0xd7fff).rom().region("fdc_bios_2hd", 0); //floppy BIOS 2hd
	map(0xe8000, 0xfffff).rom().region("ipl", 0);
}

/* first device is even offsets, second one is odd offsets */
void pc9801_state::pc9801_common_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x001f).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask16(0xff00);
	map(0x0000, 0x001f).rw(FUNC(pc9801_state::pic_r), FUNC(pc9801_state::pic_w)).umask16(0x00ff); // i8259 PIC (bit 3 ON slave / master) / i8237 DMA
	map(0x0020, 0x002f).w(FUNC(pc9801_state::rtc_w)).umask16(0x00ff);
	map(0x0030, 0x0037).rw(m_ppi_sys, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0xff00); //i8251 RS232c / i8255 system port
	map(0x0040, 0x0047).rw(m_ppi_prn, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x0040, 0x0047).rw(m_keyb, FUNC(pc9801_kbd_device::rx_r), FUNC(pc9801_kbd_device::tx_w)).umask16(0xff00); //i8255 printer port / i8251 keyboard
	map(0x0050, 0x0057).rw("ppi8255_fdd", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0xff00);
	map(0x0050, 0x0057).w(FUNC(pc9801_state::nmi_ctrl_w)).umask16(0x00ff); // NMI FF / i8255 floppy port (2d?)
	map(0x0060, 0x0063).rw(m_hgdc1, FUNC(upd7220_device::read), FUNC(upd7220_device::write)).umask16(0x00ff); //upd7220 character ports / <undefined>
	map(0x0064, 0x0064).w(FUNC(pc9801_state::vrtc_clear_w));
//  map(0x006c, 0x006f) border color / <undefined>
	map(0x0070, 0x007f).rw(m_pit8253, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0xff00);
	map(0x0070, 0x007f).rw(FUNC(pc9801_state::txt_scrl_r), FUNC(pc9801_state::txt_scrl_w)).umask16(0x00ff); //display registers / i8253 pit
	map(0x0090, 0x0090).r(m_fdc_2hd, FUNC(upd765a_device::msr_r));
	map(0x0092, 0x0092).rw(m_fdc_2hd, FUNC(upd765a_device::fifo_r), FUNC(upd765a_device::fifo_w));
	map(0x0094, 0x0094).rw(FUNC(pc9801_state::fdc_2hd_ctrl_r), FUNC(pc9801_state::fdc_2hd_ctrl_w));
	map(0x0090, 0x0093).rw(m_sio, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0xff00);
	map(0x7fd8, 0x7fdf).rw("ppi8255_mouse", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0xff00);
}

void pc9801_state::pc9801_io(address_map &map)
{
	pc9801_common_io(map);
	map(0x0020, 0x002f).w(FUNC(pc9801_state::dmapg4_w)).umask16(0xff00);
	map(0x0068, 0x0068).w(FUNC(pc9801_state::pc9801_video_ff_w)); //mode FF / <undefined>
	map(0x0080, 0x0080).rw(FUNC(pc9801_state::sasi_data_r), FUNC(pc9801_state::sasi_data_w));
	map(0x0082, 0x0082).rw(FUNC(pc9801_state::sasi_status_r), FUNC(pc9801_state::sasi_ctrl_w));
	map(0x00a0, 0x00af).rw(FUNC(pc9801_state::pc9801_a0_r), FUNC(pc9801_state::pc9801_a0_w)); //upd7220 bitmap ports / display registers
	map(0x00c8, 0x00cb).m(m_fdc_2dd, FUNC(upd765a_device::map)).umask16(0x00ff);
	map(0x00cc, 0x00cc).rw(FUNC(pc9801_state::fdc_2dd_ctrl_r), FUNC(pc9801_state::fdc_2dd_ctrl_w)); //upd765a 2dd / <undefined>
	map(0x00f0, 0x00ff).r(FUNC(pc9801_state::f0_r)).umask16(0x00ff);
}

/*************************************
 *
 * PC-9801RS specific handlers (IA-32)
 *
 ************************************/

/* TODO: it's possible that the offset calculation is actually linear. */
/* TODO: having this non-linear makes the system to boot in BASIC for PC-9821. Perhaps it stores settings? How to change these? */
READ8_MEMBER(pc9801_state::pc9801rs_knjram_r)
{
	uint32_t pcg_offset;

	pcg_offset = (m_font_addr & 0x7fff) << 5;
	pcg_offset|= offset & 0x1e;
	pcg_offset|= m_font_lr;

	if(!(m_font_addr & 0xff))
	{
		int char_size = m_video_ff[FONTSEL_REG];
		return m_char_rom[(m_font_addr >> 8) * (8 << char_size) + (char_size * 0x800) + ((offset >> 1) & 0xf)];
	}

	/* TODO: investigate on this difference */
	if((m_font_addr & 0xff00) == 0x5600 || (m_font_addr & 0xff00) == 0x5700)
		return m_kanji_rom[pcg_offset];

	pcg_offset = (m_font_addr & 0x7fff) << 5;
	pcg_offset|= offset & 0x1f;
//  pcg_offset|= m_font_lr;

	return m_kanji_rom[pcg_offset];
}

WRITE8_MEMBER(pc9801_state::pc9801rs_knjram_w)
{
	uint32_t pcg_offset;

	pcg_offset = (m_font_addr & 0x7fff) << 5;
	pcg_offset|= offset & 0x1e;
	pcg_offset|= m_font_lr;

	if((m_font_addr & 0xff00) == 0x5600 || (m_font_addr & 0xff00) == 0x5700)
	{
		m_kanji_rom[pcg_offset] = data;
		m_gfxdecode->gfx(2)->mark_dirty(pcg_offset >> 5);
	}
}

/* FF-based */
WRITE8_MEMBER(pc9801_state::pc9801rs_bank_w)
{
	if(offset == 1)
	{
		if((data & 0xf0) == 0x00 || (data & 0xf0) == 0x10)
		{
			if((data & 0xed) == 0x00)
			{
				m_ipl->set_bank((data & 2) >> 1);
				return;
			}
		}

		logerror("Unknown EMS ROM setting %02x\n",data);
	}
	if(offset == 3)
	{
		if((data & 0xf0) == 0x20)
			m_vram_bank = (data & 2) >> 1;
		else
		{
			logerror("Unknown EMS RAM setting %02x\n",data);
		}
	}
}

READ8_MEMBER(pc9801_state::a20_ctrl_r)
{
	if(offset == 0x01)
		return (m_gate_a20 ^ 1) | 0xfe;
	else if(offset == 0x03)
		return (m_gate_a20 ^ 1) | (m_nmi_ff << 1);

	return f0_r(space,offset);
}

WRITE8_MEMBER(pc9801_state::a20_ctrl_w)
{
	if(offset == 0x00)
	{
		uint8_t por;
		/* reset POR bit */
		// TODO: is there any other way that doesn't involve direct r/w of ppi address?
		por = m_ppi_sys->read(2) & ~0x20;
		m_ppi_sys->write(2, por);
		m_maincpu->set_input_line(INPUT_LINE_A20, CLEAR_LINE);
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		m_gate_a20 = 0;
	}

	if(offset == 0x01)
		m_gate_a20 = 1;

	if(offset == 0x03)
	{
		if(data == 0x02)
			m_gate_a20 = 1;
		else if(data == 0x03)
			m_gate_a20 = 0;
	}
	m_maincpu->set_input_line(INPUT_LINE_A20, m_gate_a20);
}

READ8_MEMBER(pc9801_state::grcg_r)
{
	if(offset == 6)
	{
		logerror("GRCG mode R\n");
		return 0xff;
	}
	else if(offset == 7)
	{
		logerror("GRCG tile R\n");
		return 0xff;
	}
	return txt_scrl_r(space,offset);
}

WRITE8_MEMBER(pc9801_state::grcg_w)
{
	if(offset == 6)
	{
//      logerror("%02x GRCG MODE\n",data);
		m_grcg.mode = data;
		m_grcg.tile_index = 0;
		return;
	}
	else if(offset == 7)
	{
//      logerror("%02x GRCG TILE %02x\n",data,m_grcg.tile_index);
		m_grcg.tile[m_grcg.tile_index] = bitswap<8>(data,0,1,2,3,4,5,6,7);
		m_grcg.tile_index ++;
		m_grcg.tile_index &= 3;
		return;
	}

	txt_scrl_w(space,offset,data);
}

WRITE16_MEMBER(pc9801_state::egc_w)
{
	if(!m_ex_video_ff[2])
		return;

	if(!(m_egc.regs[1] & 0x6000) || (offset != 4)) // why?
		COMBINE_DATA(&m_egc.regs[offset]);
	switch(offset)
	{
		case 1:
		case 3:
		case 5:
		{
			uint8_t color = 0;
			switch((m_egc.regs[1] >> 13) & 3)
			{
				case 1:
					//back color
					color = m_egc.regs[5];
					break;
				case 2:
					//fore color
					color = m_egc.regs[3];
					break;
				default:
					return;
			}
			m_egc.pat[0] = (color & 1) ? 0xffff : 0;
			m_egc.pat[1] = (color & 2) ? 0xffff : 0;
			m_egc.pat[2] = (color & 4) ? 0xffff : 0;
			m_egc.pat[3] = (color & 8) ? 0xffff : 0;
			break;
		}
		case 6:
		case 7:
			m_egc.count = (m_egc.regs[7] & 0xfff) + 1;
			m_egc.first = true;
			m_egc.init = false;
			break;
	}
}

READ8_MEMBER(pc9801_state::fdc_mode_ctrl_r)
{
	return (m_fdc_ctrl & 3) | 0xf0 | 8 | 4;
}

WRITE8_MEMBER(pc9801_state::fdc_mode_ctrl_w)
{
	/*
	---- x--- ready line?
	---- --x- select type (1) 2hd (0) 2dd
	---- ---x select irq
	*/

	m_fdc_2hd->subdevice<floppy_connector>("0")->get_device()->set_rpm(data & 0x02 ? 360 : 300);
	m_fdc_2hd->subdevice<floppy_connector>("1")->get_device()->set_rpm(data & 0x02 ? 360 : 300);

	m_fdc_2hd->set_rate(data & 0x02 ? 500000 : 250000);

	m_fdc_ctrl = data;
	//if(data & 0xfc)
	//  logerror("FDC ctrl called with %02x\n",data);
}

#if 0
READ8_MEMBER(pc9801_state::pc9801rs_2dd_r)
{
//  if(m_fdc_ctrl & 1)
//      return 0xff;

	if((offset & 1) == 0)
	{
		switch(offset & 6)
		{
			case 0: return m_fdc_2hd->msr_r(space, 0, 0xff);
			case 2: return m_fdc_2hd->fifo_r(space, 0, 0xff);
			case 4: return 0x44; //2dd flag
		}
	}

	logerror("Read to undefined port [%02x]\n",offset+0x90);

	return 0xff;
}

WRITE8_MEMBER(pc9801_state::pc9801rs_2dd_w)
{
//  if(m_fdc_ctrl & 1)
//      return;

	if((offset & 1) == 0)
	{
		switch(offset & 6)
		{
			case 2: m_fdc_2hd->fifo_w(space, 0, data, 0xff); return;
			case 4: logerror("%02x 2DD FDC ctrl\n",data); return;
		}
	}

	logerror("Write to undefined port [%02x] %02x\n",offset+0x90,data);
}
#endif

WRITE8_MEMBER(pc9801_state::pc9801rs_video_ff_w)
{
	if(offset == 1)
	{
		if((data & 0xf0) == 0) /* disable any PC-9821 specific HW regs */
			m_ex_video_ff[(data & 0xfe) >> 1] = data & 1;

		if(0)
		{
			static const char *const ex_video_ff_regnames[] =
			{
				"16 colors mode",   // 0
				"<unknown>",        // 1
				"EGC related",      // 2
				"<unknown>"         // 3
			};

			logerror("Write to extended video FF register %s -> %02x\n",ex_video_ff_regnames[(data & 0x06) >> 1],data & 1);
		}
		//else
		//  logerror("Write to extended video FF register %02x\n",data);

		return;
	}

	pc9801_video_ff_w(space,offset,data);
}

WRITE8_MEMBER(pc9801_state::pc9801rs_a0_w)
{
	if((offset & 1) == 0 && offset & 8 && m_ex_video_ff[ANALOG_16_MODE])
	{
		switch(offset)
		{
			case 0x08: m_analog16.pal_entry = data & 0xf; break;
			case 0x0a: m_analog16.g[m_analog16.pal_entry] = data & 0xf; break;
			case 0x0c: m_analog16.r[m_analog16.pal_entry] = data & 0xf; break;
			case 0x0e: m_analog16.b[m_analog16.pal_entry] = data & 0xf; break;
		}

		m_palette->set_pen_color((m_analog16.pal_entry)+0x10,
												pal4bit(m_analog16.r[m_analog16.pal_entry]),
												pal4bit(m_analog16.g[m_analog16.pal_entry]),
												pal4bit(m_analog16.b[m_analog16.pal_entry]));
		return;
	}

	pc9801_a0_w(space,offset,data);
}

READ8_MEMBER( pc9801_state::access_ctrl_r )
{
	if(offset == 1)
		return m_access_ctrl;

	return 0xff;
}

WRITE8_MEMBER( pc9801_state::access_ctrl_w )
{
	if(offset == 1)
		m_access_ctrl = data;
}

WRITE8_MEMBER( pc9801_state::pc9801rs_mouse_freq_w )
{
	/* TODO: bit 3 used */
	if(offset == 3)
	{
		m_mouse.freq_reg = data & 3;
		m_mouse.freq_index = 0;
	}
}

READ8_MEMBER( pc9801_state::midi_r )
{
	/* unconnect, needed by Amaranth KH to boot */
	return 0xff;
}

READ8_MEMBER(pc9801_state::pic_r)
{
	return ((offset >= 4) ? m_pic2 : m_pic1)->read(offset & 3);
}

WRITE8_MEMBER(pc9801_state::pic_w)
{
	((offset >= 4) ? m_pic2 : m_pic1)->write(offset & 3, data);
}

READ16_MEMBER(pc9801_state::grcg_gvram_r)
{
	uint16_t ret = upd7220_grcg_r(space, (offset + 0x4000) | (m_vram_bank << 16), mem_mask);
	return bitswap<16>(ret,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
}

WRITE16_MEMBER(pc9801_state::grcg_gvram_w)
{
	data = bitswap<16>(data,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
	upd7220_grcg_w(space, (offset + 0x4000) | (m_vram_bank << 16), data, mem_mask);
}

READ16_MEMBER(pc9801_state::grcg_gvram0_r)
{
	uint16_t ret = upd7220_grcg_r(space, offset | (m_vram_bank << 16), mem_mask);
	return bitswap<16>(ret,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
}

WRITE16_MEMBER(pc9801_state::grcg_gvram0_w)
{
	data = bitswap<16>(data,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
	upd7220_grcg_w(space, offset | (m_vram_bank << 16), data, mem_mask);
}

void pc9801_state::ipl_bank(address_map &map)
{
	map(0x00000, 0x2ffff).rom().region("ipl", 0);
}

void pc9801_state::pc9801ux_map(address_map &map)
{
	map(0x0a0000, 0x0a3fff).rw(FUNC(pc9801_state::tvram_r), FUNC(pc9801_state::tvram_w));
	map(0x0a4000, 0x0a4fff).rw(FUNC(pc9801_state::pc9801rs_knjram_r), FUNC(pc9801_state::pc9801rs_knjram_w));
	map(0x0a8000, 0x0bffff).rw(FUNC(pc9801_state::grcg_gvram_r), FUNC(pc9801_state::grcg_gvram_w));
	map(0x0e0000, 0x0e7fff).rw(FUNC(pc9801_state::grcg_gvram0_r), FUNC(pc9801_state::grcg_gvram0_w));
	map(0x0e8000, 0x0fffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
}

void pc9801_state::pc9801ux_io(address_map &map)
{
	map.unmap_value_high();
	pc9801_common_io(map);
	map(0x0020, 0x002f).w(FUNC(pc9801_state::dmapg8_w)).umask16(0xff00);
	map(0x0050, 0x0057).noprw(); // 2dd ppi?
	map(0x005c, 0x005f).r(FUNC(pc9801_state::timestamp_r)).nopw(); // artic
	map(0x0068, 0x006b).w(FUNC(pc9801_state::pc9801rs_video_ff_w)).umask16(0x00ff); //mode FF / <undefined>
	map(0x0070, 0x007f).rw(FUNC(pc9801_state::grcg_r), FUNC(pc9801_state::grcg_w)).umask16(0x00ff); //display registers "GRCG" / i8253 pit
	map(0x00a0, 0x00af).rw(FUNC(pc9801_state::pc9801_a0_r), FUNC(pc9801_state::pc9801rs_a0_w)); //upd7220 bitmap ports / display registers
	map(0x00bc, 0x00bf).rw(FUNC(pc9801_state::fdc_mode_ctrl_r), FUNC(pc9801_state::fdc_mode_ctrl_w));
	map(0x00c8, 0x00cb).m(m_fdc_2hd, FUNC(upd765a_device::map)).umask16(0x00ff);
	map(0x00cc, 0x00cc).rw(FUNC(pc9801_state::fdc_2hd_ctrl_r), FUNC(pc9801_state::fdc_2hd_ctrl_w));
	map(0x00f0, 0x00ff).rw(FUNC(pc9801_state::a20_ctrl_r), FUNC(pc9801_state::a20_ctrl_w)).umask16(0x00ff);
	map(0x0438, 0x043b).rw(FUNC(pc9801_state::access_ctrl_r), FUNC(pc9801_state::access_ctrl_w));
	map(0x043c, 0x043f).w(FUNC(pc9801_state::pc9801rs_bank_w)); //ROM/RAM bank
	map(0x04a0, 0x04af).w(FUNC(pc9801_state::egc_w));
	map(0x3fd8, 0x3fdf).rw(m_pit8253, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0xff00);
}

void pc9801_state::pc9801rs_map(address_map &map)
{
	pc9801ux_map(map);
//  map(0x0d8000, 0x0d9fff).rom().region("ide",0);
	map(0x0da000, 0x0dbfff).ram(); // ide ram
	map(0xee8000, 0xefffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
	map(0xfe8000, 0xffffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
}

void pc9801_state::pc9801rs_io(address_map &map)
{
	map.unmap_value_high();
	pc9801ux_io(map);
	map(0x0430, 0x0433).rw(FUNC(pc9801_state::ide_ctrl_r), FUNC(pc9801_state::ide_ctrl_w)).umask16(0x00ff);
	map(0x0640, 0x064f).rw(FUNC(pc9801_state::ide_cs0_r), FUNC(pc9801_state::ide_cs0_w));
	map(0x0740, 0x074f).rw(FUNC(pc9801_state::ide_cs1_r), FUNC(pc9801_state::ide_cs1_w));
	map(0x1e8c, 0x1e8f).noprw(); // temp
	map(0xbfd8, 0xbfdf).w(FUNC(pc9801_state::pc9801rs_mouse_freq_w));
	map(0xe0d0, 0xe0d3).r(FUNC(pc9801_state::midi_r));
}

/*************************************
 *
 * PC-9821 specific handlers
 *
 ************************************/

WRITE8_MEMBER(pc9801_state::pc9821_video_ff_w)
{
	if(offset == 1)
	{
		if(((data & 0xfe) == 4) && !m_ex_video_ff[3]) // TODO: many other settings are protected
			return;
		m_ex_video_ff[(data & 0xfe) >> 1] = data & 1;

		//if((data & 0xfe) == 0x20)
		//  logerror("%02x\n",data & 1);
	}

	/* Intentional fall-through */
	pc9801rs_video_ff_w(space,offset,data);
}

READ8_MEMBER(pc9801_state::pc9821_a0_r)
{
	if((offset & 1) == 0 && offset & 8)
	{
		if(m_ex_video_ff[ANALOG_256_MODE])
		{
			logerror("256 color mode [%02x] R\n",offset);
			return 0;
		}
		else if(m_ex_video_ff[ANALOG_16_MODE]) //16 color mode, readback possible there
		{
			uint8_t res = 0;

			switch(offset)
			{
				case 0x08: res = m_analog16.pal_entry & 0xf; break;
				case 0x0a: res = m_analog16.g[m_analog16.pal_entry] & 0xf; break;
				case 0x0c: res = m_analog16.r[m_analog16.pal_entry] & 0xf; break;
				case 0x0e: res = m_analog16.b[m_analog16.pal_entry] & 0xf; break;
			}

			return res;
		}
	}

	return pc9801_a0_r(space,offset);
}

WRITE8_MEMBER(pc9801_state::pc9821_a0_w)
{
	if((offset & 1) == 0 && offset & 8 && m_ex_video_ff[ANALOG_256_MODE])
	{
		switch(offset)
		{
			case 0x08: m_analog256.pal_entry = data & 0xff; break;
			case 0x0a: m_analog256.g[m_analog256.pal_entry] = data & 0xff; break;
			case 0x0c: m_analog256.r[m_analog256.pal_entry] = data & 0xff; break;
			case 0x0e: m_analog256.b[m_analog256.pal_entry] = data & 0xff; break;
		}

		m_palette->set_pen_color((m_analog256.pal_entry)+0x20,
												m_analog256.r[m_analog256.pal_entry],
												m_analog256.g[m_analog256.pal_entry],
												m_analog256.b[m_analog256.pal_entry]);
		return;
	}

	pc9801rs_a0_w(space,offset,data);
}

READ8_MEMBER(pc9801_state::window_bank_r)
{
	if(offset == 1)
		return m_pc9821_window_bank & 0xfe;

	return 0xff;
}

WRITE8_MEMBER(pc9801_state::window_bank_w)
{
	if(offset == 1)
		m_pc9821_window_bank = data & 0xfe;
	else
		logerror("PC-9821 $f0000 window bank %02x\n",data);
}

uint8_t pc9801_state::m_sdip_read(uint16_t port, uint8_t sdip_offset)
{
	if(port == 2)
		return m_sdip[sdip_offset];

	logerror("Warning: read from unknown SDIP area %02x %04x\n",port,0x841c + port + (sdip_offset % 12)*0x100);
	return 0xff;
}

void pc9801_state::m_sdip_write(uint16_t port, uint8_t sdip_offset,uint8_t data)
{
	if(port == 2)
	{
		m_sdip[sdip_offset] = data;
		return;
	}

	logerror("Warning: write from unknown SDIP area %02x %04x %02x\n",port,0x841c + port + (sdip_offset % 12)*0x100,data);
}

READ8_MEMBER(pc9801_state::sdip_0_r) { return m_sdip_read(offset, 0+m_sdip_bank*12); }
READ8_MEMBER(pc9801_state::sdip_1_r) { return m_sdip_read(offset, 1+m_sdip_bank*12); }
READ8_MEMBER(pc9801_state::sdip_2_r) { return m_sdip_read(offset, 2+m_sdip_bank*12); }
READ8_MEMBER(pc9801_state::sdip_3_r) { return m_sdip_read(offset, 3+m_sdip_bank*12); }
READ8_MEMBER(pc9801_state::sdip_4_r) { return m_sdip_read(offset, 4+m_sdip_bank*12); }
READ8_MEMBER(pc9801_state::sdip_5_r) { return m_sdip_read(offset, 5+m_sdip_bank*12); }
READ8_MEMBER(pc9801_state::sdip_6_r) { return m_sdip_read(offset, 6+m_sdip_bank*12); }
READ8_MEMBER(pc9801_state::sdip_7_r) { return m_sdip_read(offset, 7+m_sdip_bank*12); }
READ8_MEMBER(pc9801_state::sdip_8_r) { return m_sdip_read(offset, 8+m_sdip_bank*12); }
READ8_MEMBER(pc9801_state::sdip_9_r) { return m_sdip_read(offset, 9+m_sdip_bank*12); }
READ8_MEMBER(pc9801_state::sdip_a_r) { return m_sdip_read(offset, 10+m_sdip_bank*12); }
READ8_MEMBER(pc9801_state::sdip_b_r) { return m_sdip_read(offset, 11+m_sdip_bank*12); }

WRITE8_MEMBER(pc9801_state::sdip_0_w) { m_sdip_write(offset,0+m_sdip_bank*12,data); }
WRITE8_MEMBER(pc9801_state::sdip_1_w) { m_sdip_write(offset,1+m_sdip_bank*12,data); }
WRITE8_MEMBER(pc9801_state::sdip_2_w) { m_sdip_write(offset,2+m_sdip_bank*12,data); }
WRITE8_MEMBER(pc9801_state::sdip_3_w) { m_sdip_write(offset,3+m_sdip_bank*12,data); }
WRITE8_MEMBER(pc9801_state::sdip_4_w) { m_sdip_write(offset,4+m_sdip_bank*12,data); }
WRITE8_MEMBER(pc9801_state::sdip_5_w) { m_sdip_write(offset,5+m_sdip_bank*12,data); }
WRITE8_MEMBER(pc9801_state::sdip_6_w) { m_sdip_write(offset,6+m_sdip_bank*12,data); }
WRITE8_MEMBER(pc9801_state::sdip_7_w) { m_sdip_write(offset,7+m_sdip_bank*12,data); }
WRITE8_MEMBER(pc9801_state::sdip_8_w) { m_sdip_write(offset,8+m_sdip_bank*12,data); }
WRITE8_MEMBER(pc9801_state::sdip_9_w) { m_sdip_write(offset,9+m_sdip_bank*12,data); }
WRITE8_MEMBER(pc9801_state::sdip_a_w) { m_sdip_write(offset,10+m_sdip_bank*12,data); }
WRITE8_MEMBER(pc9801_state::sdip_b_w)
{
	if(offset == 3)
		m_sdip_bank = (data & 0x40) >> 6;

	if(offset == 2)
		m_sdip_write(offset,11+m_sdip_bank*12,data);

	if((offset & 2) == 0)
		logerror("SDIP area B write %02x %02x\n",offset,data);
}

READ16_MEMBER(pc9801_state::timestamp_r)
{
	return (m_maincpu->total_cycles() >> (16 * offset));
}

/* basically a read-back of various registers */
// bit 1: GDC clock select (port 0x6a, selects with 0x84 & bit 0)
// bit 0: current setting
READ8_MEMBER(pc9801_state::ext2_video_ff_r)
{
	uint8_t res;

	res = 0;

	switch(m_ext2_ff)
	{
//      case 0x00: ?
//      case 0x01: 200 line color / b&w mode (i/o 0x68 -> 0x02)
//      case 0x02: Odd-numbered raster mask  (i/o 0x68 -> 0x08)
		case 0x03: res = m_video_ff[DISPLAY_REG]; break; // display reg
//      case 0x04: palette mode (i/o 0x6a -> 0x00)
//      case 0x05: GDC sync mode (i/o 0x6a -> 0x40)
//      case 0x06: unknown (i/o 0x6a -> 0x44)
//      case 0x07: EGC compatibility mode (i/o 0x6a -> 0x04)
//      case 0x08: Protected mode f/f (i/o 0x6a -> 0x06)
//      case 0x09: GDC clock #0 (i/o 0x6a -> 0x82)
		case 0x0a: res = m_ex_video_ff[ANALOG_256_MODE]; break; // 256 color mode
//      case 0x0b: VRAM access mode (i/o 0x6a -> 0x62)
//      case 0x0c: unknown
//      case 0x0d: VRAM boundary mode (i/o 0x6a -> 0x68)
//      case 0x0e: 65536 color GFX mode (i/o 0x6a -> 0x22)
//      case 0x0f: 65,536 color palette mode (i/o 0x6a -> 0x24)
//      case 0x10: unknown (i/o 0x6a -> 0x6a)
//      case 0x11: Reverse mode related (i/o 0x6a -> 0x26)
//      case 0x12: 256 color overscan color (i/o 0x6a -> 0x2c)
//      case 0x13: Reverse mode related (i/o 0x6a -> 0x28)
//      case 0x14: AGDC Drawing processor selection (i/o 0x6a -> 0x66)
//      case 0x15: unknown (i/o 0x6a -> 0x60)
//      case 0x16: unknown (i/o 0x6a -> 0xc2)
//      case 0x17: bitmap config direction (i/o 0x6a -> 0x6c)
//      case 0x18: High speed palette write (i/o 0x6a -> 0x2a)
//      case 0x19: unknown (i/o 0x6a -> 0x48)
//      case 0x1a: unknown (i/o 0x6a -> 0xc8)
//      case 0x1b: unknown (i/o 0x6a -> 0x2e)
//      case 0x1c: unknown (i/o 0x6a -> 0x6e)
//      case 0x1d: unknown (i/o 0x6a -> 0xc0)
//      case 0x1e: unknown (i/o 0x6a -> 0x80 or 0x46?)
//      case 0x1f: unknown (i/o 0x6a -> 0x08)
		default:
			if(m_ext2_ff < 0x20)
				popmessage("PC-9821: read ext2 f/f with value %02x",m_ext2_ff);
			break;
	}

	res|= (m_ex_video_ff[GDC_IS_5MHz] << 1);

	return res;
}

WRITE8_MEMBER(pc9801_state::ext2_video_ff_w)
{
	m_ext2_ff = data;
}

/*READ8_MEMBER(pc9801_state::winram_r)
{
    offset = (offset & 0x1ffff) | (m_pc9821_window_bank & 0xfe) * 0x10000;
    return
}


WRITE8_MEMBER(pc9801_state::winram_w)
{
    offset = (offset & 0x1ffff) | (m_pc9821_window_bank & 0xfe) * 0x10000;
}*/

// TODO: analog 256 mode needs HW tests
READ16_MEMBER(pc9801_state::pc9821_grcg_gvram_r)
{
	if(m_ex_video_ff[ANALOG_256_MODE])
	{
		u16 *ext_gvram = (u16 *)m_ext_gvram.target();
		int bank = offset >> 14;
		if(bank <= 1)
			return ext_gvram[((m_analog256.bank[bank])*0x4000) + (offset & 0x3fff)];
		return 0xffff;
	}

	return grcg_gvram_r(space,offset,mem_mask);
}

WRITE16_MEMBER(pc9801_state::pc9821_grcg_gvram_w)
{
	if(m_ex_video_ff[ANALOG_256_MODE])
	{
		u16 *ext_gvram = (u16 *)m_ext_gvram.target();
		int bank = offset >> 14;
		if(bank <= 1)
			COMBINE_DATA(&ext_gvram[((m_analog256.bank[bank])*0x4000) + (offset & 0x3fff)]);
		return;
	}

	grcg_gvram_w(space,offset,data,mem_mask);
}

READ16_MEMBER(pc9801_state::pc9821_grcg_gvram0_r)
{
	if(m_ex_video_ff[ANALOG_256_MODE])
	{
		switch(offset*2)
		{
			case 4: return m_analog256.bank[0];
			case 6: return m_analog256.bank[1];
		}

		//return 0;
	}

	return grcg_gvram0_r(space,offset,mem_mask);
}

WRITE16_MEMBER(pc9801_state::pc9821_grcg_gvram0_w)
{
	if(m_ex_video_ff[ANALOG_256_MODE])
	{
		//printf("%08x %08x\n",offset*2,data);
		if(mem_mask & 0xff)
		{
			switch(offset*2)
			{
				case 4: m_analog256.bank[0] = data & 0xf; break;
				case 6: m_analog256.bank[1] = data & 0xf; break;
			}
		}
		return;
	}

	grcg_gvram0_w(space,offset,data,mem_mask);
}


void pc9801_state::pc9821_map(address_map &map)
{
	//map(0x00080000, 0x0009ffff).rw(FUNC(pc9801_state::winram_r), FUNC(pc9801_state::winram_w))
	map(0x000a0000, 0x000a3fff).rw(FUNC(pc9801_state::tvram_r), FUNC(pc9801_state::tvram_w));
	map(0x000a4000, 0x000a4fff).rw(FUNC(pc9801_state::pc9801rs_knjram_r), FUNC(pc9801_state::pc9801rs_knjram_w));
	map(0x000a8000, 0x000bffff).rw(FUNC(pc9801_state::pc9821_grcg_gvram_r), FUNC(pc9801_state::pc9821_grcg_gvram_w));
//  map(0x000cc000, 0x000cffff).rom().region("sound_bios", 0); //sound BIOS
//  map(0x000d8000, 0x000d9fff).rom().region("ide",0)
	map(0x000da000, 0x000dbfff).ram(); // ide ram
	map(0x000e0000, 0x000e7fff).rw(FUNC(pc9801_state::pc9821_grcg_gvram0_r), FUNC(pc9801_state::pc9821_grcg_gvram0_w));
	map(0x000e8000, 0x000fffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
	map(0x00f00000, 0x00f9ffff).ram().share("ext_gvram");
	map(0xffee8000, 0xffefffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
	map(0xfff00000, 0xfff9ffff).ram().share("ext_gvram");
	map(0xfffe8000, 0xffffffff).m(m_ipl, FUNC(address_map_bank_device::amap16));
}

void pc9801_state::pc9821_io(address_map &map)
{
//  map.unmap_value_high(); // TODO: a read to somewhere makes this to fail at POST
	map(0x0000, 0x001f).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask32(0xff00ff00);
	map(0x0000, 0x001f).lr8(NAME([this] (address_space &s, offs_t o, u8 mm) { return BIT(o, 1) ? 0xff : pic_r(s, o, mm); })).umask32(0x00ff00ff);
	map(0x0000, 0x001f).w(FUNC(pc9801_state::pic_w)).umask32(0x00ff00ff);  // i8259 PIC (bit 3 ON slave / master) / i8237 DMA
	map(0x0020, 0x002f).w(FUNC(pc9801_state::rtc_w)).umask32(0x000000ff);
	map(0x0020, 0x002f).w(FUNC(pc9801_state::dmapg8_w)).umask32(0xff00ff00);
	map(0x0030, 0x0037).rw(m_ppi_sys, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask32(0xff00ff00); //i8251 RS232c / i8255 system port
	map(0x0040, 0x0047).rw(m_ppi_prn, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask32(0x00ff00ff);
	map(0x0040, 0x0047).rw(m_keyb, FUNC(pc9801_kbd_device::rx_r), FUNC(pc9801_kbd_device::tx_w)).umask32(0xff00ff00); //i8255 printer port / i8251 keyboard
	map(0x0050, 0x0053).w(FUNC(pc9801_state::nmi_ctrl_w)).umask32(0x00ff00ff);
	map(0x005c, 0x005f).r(FUNC(pc9801_state::timestamp_r)).nopw(); // artic
	map(0x0060, 0x0063).rw(m_hgdc1, FUNC(upd7220_device::read), FUNC(upd7220_device::write)).umask32(0x00ff00ff); //upd7220 character ports / <undefined>
	map(0x0060, 0x0063).r(FUNC(pc9801_state::unk_r)).umask32(0xff00ff00); // mouse related (unmapped checking for AT keyb controller\PS/2 mouse?)
	map(0x0064, 0x0064).w(FUNC(pc9801_state::vrtc_clear_w));
	map(0x0068, 0x006b).w(FUNC(pc9801_state::pc9821_video_ff_w)).umask32(0x00ff00ff); //mode FF / <undefined>
	map(0x0070, 0x007f).rw(m_pit8253, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask32(0xff00ff00);
	map(0x0070, 0x007f).rw(FUNC(pc9801_state::grcg_r), FUNC(pc9801_state::grcg_w)).umask32(0x00ff00ff); //display registers "GRCG" / i8253 pit
	map(0x0090, 0x0093).m(m_fdc_2hd, FUNC(upd765a_device::map)).umask32(0x00ff00ff);
	map(0x0094, 0x0094).rw(FUNC(pc9801_state::fdc_2hd_ctrl_r), FUNC(pc9801_state::fdc_2hd_ctrl_w));
	map(0x00a0, 0x00af).rw(FUNC(pc9801_state::pc9821_a0_r), FUNC(pc9801_state::pc9821_a0_w)); //upd7220 bitmap ports / display registers
//  map(0x00b0, 0x00b3) PC9861k (serial port?)
//  map(0x00b9, 0x00b9) PC9861k
//  map(0x00bb, 0x00bb) PC9861k
	map(0x00bc, 0x00bf).rw(FUNC(pc9801_state::fdc_mode_ctrl_r), FUNC(pc9801_state::fdc_mode_ctrl_w));
	map(0x00c8, 0x00cb).m(m_fdc_2hd, FUNC(upd765a_device::map)).umask32(0x00ff00ff);
	map(0x00cc, 0x00cc).rw(FUNC(pc9801_state::fdc_2hd_ctrl_r), FUNC(pc9801_state::fdc_2hd_ctrl_w));
	//  map(0x00d8, 0x00df) AMD98 (sound?) board
	map(0x00f0, 0x00ff).rw(FUNC(pc9801_state::a20_ctrl_r), FUNC(pc9801_state::a20_ctrl_w)).umask32(0x00ff00ff);
//  map(0x0188, 0x018f).rw(FUNC(pc9801_state::pc9801_opn_r), FUNC(pc9801_state::pc9801_opn_w)); //ym2203 opn / <undefined>
//  map(0x018c, 0x018f) YM2203 OPN extended ports / <undefined>
	map(0x0430, 0x0433).rw(FUNC(pc9801_state::ide_ctrl_r), FUNC(pc9801_state::ide_ctrl_w)).umask32(0x00ff00ff);
	map(0x0438, 0x043b).rw(FUNC(pc9801_state::access_ctrl_r), FUNC(pc9801_state::access_ctrl_w));
//  map(0x043d, 0x043d) ROM/RAM bank (NEC)
	map(0x043c, 0x043f).w(FUNC(pc9801_state::pc9801rs_bank_w)); //ROM/RAM bank (EPSON)
	map(0x0460, 0x0463).rw(FUNC(pc9801_state::window_bank_r), FUNC(pc9801_state::window_bank_w));
	map(0x04a0, 0x04af).w(FUNC(pc9801_state::egc_w));
//  map(0x04be, 0x04be) FDC "RPM" register
	map(0x0640, 0x064f).rw(FUNC(pc9801_state::ide_cs0_r), FUNC(pc9801_state::ide_cs0_w));
	map(0x0740, 0x074f).rw(FUNC(pc9801_state::ide_cs1_r), FUNC(pc9801_state::ide_cs1_w));
//  map(0x08e0, 0x08ea) <undefined> / EMM SIO registers
	map(0x09a0, 0x09a0).rw(FUNC(pc9801_state::ext2_video_ff_r), FUNC(pc9801_state::ext2_video_ff_w)); // GDC extended register r/w
//  map(0x09a8, 0x09a8) GDC 31KHz register r/w
//  map(0x0c07, 0x0c07) EPSON register w
//  map(0x0c03, 0x0c03) EPSON register 0 r
//  map(0x0c13, 0x0c14) EPSON register 1 r
//  map(0x0c24, 0x0c24) cs4231 PCM board register control
//  map(0x0c2b, 0x0c2b) cs4231 PCM board low byte control
//  map(0x0c2d, 0x0c2d) cs4231 PCM board hi byte control
//  map(0x0cc0, 0x0cc7) SCSI interface / <undefined>
//  map(0x0cfc, 0x0cff) PCI bus
	map(0x1e8c, 0x1e8f).noprw(); // IDE RAM switch
	map(0x2ed0, 0x2edf).lr8(NAME([] (address_space &s, offs_t o, u8 mm) { return 0xff; })).umask32(0xffffffff); // unknown sound related
	map(0x3fd8, 0x3fdf).rw(m_pit8253, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask32(0xff00ff00); // <undefined> / pit mirror ports
	map(0x7fd8, 0x7fdf).rw("ppi8255_mouse", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask32(0xff00ff00);
	map(0x841c, 0x841f).rw(FUNC(pc9801_state::sdip_0_r), FUNC(pc9801_state::sdip_0_w));
	map(0x851c, 0x851f).rw(FUNC(pc9801_state::sdip_1_r), FUNC(pc9801_state::sdip_1_w));
	map(0x861c, 0x861f).rw(FUNC(pc9801_state::sdip_2_r), FUNC(pc9801_state::sdip_2_w));
	map(0x871c, 0x871f).rw(FUNC(pc9801_state::sdip_3_r), FUNC(pc9801_state::sdip_3_w));
	map(0x881c, 0x881f).rw(FUNC(pc9801_state::sdip_4_r), FUNC(pc9801_state::sdip_4_w));
	map(0x891c, 0x891f).rw(FUNC(pc9801_state::sdip_5_r), FUNC(pc9801_state::sdip_5_w));
	map(0x8a1c, 0x8a1f).rw(FUNC(pc9801_state::sdip_6_r), FUNC(pc9801_state::sdip_6_w));
	map(0x8b1c, 0x8b1f).rw(FUNC(pc9801_state::sdip_7_r), FUNC(pc9801_state::sdip_7_w));
	map(0x8c1c, 0x8c1f).rw(FUNC(pc9801_state::sdip_8_r), FUNC(pc9801_state::sdip_8_w));
	map(0x8d1c, 0x8d1f).rw(FUNC(pc9801_state::sdip_9_r), FUNC(pc9801_state::sdip_9_w));
	map(0x8e1c, 0x8e1f).rw(FUNC(pc9801_state::sdip_a_r), FUNC(pc9801_state::sdip_a_w));
	map(0x8f1c, 0x8f1f).rw(FUNC(pc9801_state::sdip_b_r), FUNC(pc9801_state::sdip_b_w));
//  map(0xa460, 0xa46f) cs4231 PCM extended port / <undefined>
//  map(0xbfdb, 0xbfdb) mouse timing port
//  map(0xc0d0, 0xc0d3) MIDI port, option 0 / <undefined>
//  map(0xc4d0, 0xc4d3) MIDI port, option 1 / <undefined>
//  map(0xc8d0, 0xc8d3) MIDI port, option 2 / <undefined>
//  map(0xccd0, 0xccd3) MIDI port, option 3 / <undefined>
//  map(0xd0d0, 0xd0d3) MIDI port, option 4 / <undefined>
//  map(0xd4d0, 0xd4d3) MIDI port, option 5 / <undefined>
//  map(0xd8d0, 0xd8d3) MIDI port, option 6 / <undefined>
//  map(0xdcd0, 0xdcd3) MIDI port, option 7 / <undefined>
	map(0xe0d0, 0xe0d3).r(FUNC(pc9801_state::midi_r)); // MIDI port, option 8 / <undefined>
//  map(0xe4d0, 0xe4d3) MIDI port, option 9 / <undefined>
//  map(0xe8d0, 0xe8d3) MIDI port, option A / <undefined>
//  map(0xecd0, 0xecd3) MIDI port, option B / <undefined>
//  map(0xf0d0, 0xf0d3) MIDI port, option C / <undefined>
//  map(0xf4d0, 0xf4d3) MIDI port, option D / <undefined>
//  map(0xf8d0, 0xf8d3) MIDI port, option E / <undefined>
//  map(0xfcd0, 0xfcd3) MIDI port, option F / <undefined>
}

// TODO: identify this, might be an alt way to access SDIP?
READ8_MEMBER(pc9801_state::as_unkdev_data_r)
{
	if (offset == 0)
		return m_unkdev0468[m_unkdev0468_addr];

	return 0xff;
}

WRITE8_MEMBER(pc9801_state::as_unkdev_data_w)
{
	if (offset == 0)
		m_unkdev0468[m_unkdev0468_addr] = data;

	// offset == 0: access bit?
}

WRITE8_MEMBER(pc9801_state::as_unkdev_addr_w)
{
	if (offset == 0)
		m_unkdev0468_addr = data;
}

void pc9801_state::pc9821as_io(address_map &map)
{
	pc9821_io(map);
	map(0x0468, 0x046b).rw(FUNC(pc9801_state::as_unkdev_data_r), FUNC(pc9801_state::as_unkdev_data_w)).umask32(0x00ff00ff);
	map(0x046c, 0x046f).w(FUNC(pc9801_state::as_unkdev_addr_w)).umask32(0x00ff00ff);
}

void pc9801_state::upd7220_1_map(address_map &map)
{
	map(0x00000, 0x03fff).ram().share("video_ram_1");
}

void pc9801_state::upd7220_2_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram().share("video_ram_2");
}

void pc9801_state::upd7220_grcg_2_map(address_map &map)
{
	map(0x00000, 0x3ffff).rw(FUNC(pc9801_state::upd7220_grcg_r), FUNC(pc9801_state::upd7220_grcg_w)).share("video_ram_2");
}

CUSTOM_INPUT_MEMBER(pc9801_state::system_type_r)
{
//  System Type (0x00 stock PC-9801, 0xc0 PC-9801U / PC-98LT, PC-98HA, 0x80 others)
	return m_sys_type;
}

static INPUT_PORTS_START( pc9801 )
	PORT_START("DSW1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH,IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("upd1990a", upd1990a_device, data_out_r)
	PORT_DIPNAME( 0x0002, 0x0000, "DSW1" ) // error beep if OFF
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Display Type" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0000, "Normal Display" )
	PORT_DIPSETTING(      0x0008, "Hi-Res Display" )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, "DSW5" ) // goes into basic with this off, PC-9801VF / PC-9801U setting
	PORT_DIPSETTING(      0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) ) // V30 / V33
	PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) // printer busy
	PORT_DIPSETTING(      0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) // 8 / 4096
	PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) // LCD display
	PORT_DIPSETTING(      0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) ) //system clock = 5 MHz (0) / 8 MHz (1)
	PORT_DIPSETTING(      0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(pc9801_state, system_type_r)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "System Specification" ) PORT_DIPLOCATION("SW1:1") //jumps to daa00 if off, presumably some card booting
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Terminal Mode" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Text width" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "40 chars/line" )
	PORT_DIPSETTING(    0x00, "80 chars/line" )
	PORT_DIPNAME( 0x08, 0x00, "Text height" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "20 lines/screen" )
	PORT_DIPSETTING(    0x00, "25 lines/screen" )
	PORT_DIPNAME( 0x10, 0x00, "Memory Switch Init" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) ) //Fix memory switch condition
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) ) //Initialize Memory Switch with the system default
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "DSW4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("MOUSE_X")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_RESET PORT_SENSITIVITY(30) PORT_KEYDELTA(30)

	PORT_START("MOUSE_Y")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_RESET PORT_SENSITIVITY(30) PORT_KEYDELTA(30)

	PORT_START("MOUSE_B")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Mouse Right Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_CODE(MOUSECODE_BUTTON3) PORT_NAME("Mouse Middle Button")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Mouse Left Button")

	PORT_START("ROM_LOAD")
	PORT_CONFNAME( 0x01, 0x01, "Load floppy 2hd BIOS" )
	PORT_CONFSETTING(    0x00, DEF_STR( Yes ) )
	PORT_CONFSETTING(    0x01, DEF_STR( No ) )
	PORT_CONFNAME( 0x02, 0x02, "Load floppy 2dd BIOS" )
	PORT_CONFSETTING(    0x00, DEF_STR( Yes ) )
	PORT_CONFSETTING(    0x02, DEF_STR( No ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pc9801rs )
	PORT_INCLUDE( pc9801 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x80, 0x80, "GDC clock" ) PORT_DIPLOCATION("SW1:8") // DSW 2-8
	PORT_DIPSETTING(    0x80, "2.5 MHz" )
	PORT_DIPSETTING(    0x00, "5 MHz" )

	PORT_MODIFY("DSW4")
	PORT_DIPNAME( 0x04, 0x00, "CPU Type" ) PORT_DIPLOCATION("SW4:8") // DSW 3-8
	PORT_DIPSETTING(    0x04, "V30" )
	PORT_DIPSETTING(    0x00, "I386" )

	PORT_MODIFY("DSW5")
	PORT_DIPNAME( 0x08, 0x00, "Graphic Function" ) // DSW 1-8
	PORT_DIPSETTING(      0x08, "Basic (8 Colors)" )
	PORT_DIPSETTING(      0x00, "Expanded (16/4096 Colors)"  )
	PORT_DIPNAME( 0x10, 0x10, "Display Type" ) // LCD display, 98DO Demo explicitly wants it to be non-Plasma
	PORT_DIPSETTING(      0x10, "RGB" )
	PORT_DIPSETTING(      0x00, "Plasma" )

	PORT_MODIFY("ROM_LOAD")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME( 0x04, 0x04, "Load IDE BIOS" )
	PORT_CONFSETTING(    0x00, DEF_STR( Yes ) )
	PORT_CONFSETTING(    0x04, DEF_STR( No ) )

//  PORT_START("SOUND_CONFIG")
//  PORT_CONFNAME( 0x01, 0x00, "Sound Type" )
//  PORT_CONFSETTING(    0x00, "YM2203 (OPN)" )
//  PORT_CONFSETTING(    0x01, "YM2608 (OPNA)" )
INPUT_PORTS_END

static INPUT_PORTS_START( pc9821 )
	PORT_INCLUDE( pc9801rs )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "S-Dip SW Init" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x40, 0x40, "Conventional RAM size" ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, "640 KB" )
	PORT_DIPSETTING(    0x00, "512 KB" )
INPUT_PORTS_END

static const gfx_layout charset_8x8 =
{
	8,8,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout charset_8x16 =
{
	8,16,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};

static const gfx_layout charset_16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

static GFXDECODE_START( gfx_pc9801 )
	GFXDECODE_ENTRY( "chargen", 0x00000, charset_8x8,     0x000, 0x01 )
	GFXDECODE_ENTRY( "chargen", 0x00800, charset_8x16,    0x000, 0x01 )
	GFXDECODE_ENTRY( "kanji",   0x00000, charset_16x16,   0x000, 0x01 )
	GFXDECODE_ENTRY( "raw_kanji",   0x00000, charset_16x16,   0x000, 0x01 )
	GFXDECODE_ENTRY( "new_chargen",0, charset_16x16,   0x000, 0x01 )
GFXDECODE_END

/****************************************
*
* I8259 PIC interface
*
****************************************/

/*
irq assignment (PC-9801F):

8259 master:
ir0 PIT
ir1 keyboard
ir2 vblank
ir3
ir4 rs-232c
ir5
ir6
ir7 slave irq

8259 slave:
ir0 printer
ir1 IDE?
ir2 2dd floppy irq
ir3 2hd floppy irq
ir4 opn
ir5 mouse
ir6
ir7
*/


READ8_MEMBER(pc9801_state::get_slave_ack)
{
	if (offset==7) { // IRQ = 7
		return m_pic2->acknowledge();
	}
	return 0x00;
}

/****************************************
*
* I8253 PIT interface
*
****************************************/

/* These rates do NOT appear to represent actual XTALs. They are likely obtained in
   different ways on different PC-98 models as divisions of extant XTAL frequencies
   such as 14.7456 MHz, 15.9744 MHz, 19.6608 MHz and 23.9616 MHz.
   PC-9801RS needs X1 for the pit, otherwise Uchiyama Aki no Chou Bangai has sound pitch bugs
   PC-9821 definitely needs X2, otherwise there's a timer error at POST. Unless it needs a different clock anyway ...
   */
#define MAIN_CLOCK_X1 1'996'800
#define MAIN_CLOCK_X2 2'457'600

/****************************************
*
* I8237 DMA interface
*
****************************************/

WRITE_LINE_MEMBER(pc9801_state::dma_hrq_changed)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	m_dmac->hack_w(state);

//  logerror("%02x HLDA\n",state);
}

WRITE_LINE_MEMBER(pc9801_state::tc_w )
{
	/* floppy terminal count */
	m_fdc_2hd->tc_w(state);
	if(m_fdc_2dd)
		m_fdc_2dd->tc_w(state);

//  logerror("TC %02x\n",state);
}

READ8_MEMBER(pc9801_state::dma_read_byte)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_offset[m_dack] << 16) | offset;
	if(offset == 0xffff)
	{
		switch(m_dma_autoinc[m_dack])
		{
			case 1:
			{
				uint8_t page = m_dma_offset[m_dack];
				m_dma_offset[m_dack] = ((page + 1) & 0xf) | (page & 0xf0);
				break;
			}
			case 3:
				m_dma_offset[m_dack]++;
				break;
		}
	}

//  logerror("%08x\n",addr);

	return program.read_byte(addr);
}


WRITE8_MEMBER(pc9801_state::dma_write_byte)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = (m_dma_offset[m_dack] << 16) | offset;
	if(offset == 0xffff)
	{
		switch(m_dma_autoinc[m_dack])
		{
			case 1:
			{
				uint8_t page = m_dma_offset[m_dack];
				m_dma_offset[m_dack] = ((page + 1) & 0xf) | (page & 0xf0);
				break;
			}
			case 3:
				m_dma_offset[m_dack]++;
				break;
		}
	}
//  logerror("%08x %02x\n",addr,data);

	program.write_byte(addr, data);
}

void pc9801_state::set_dma_channel(int channel, int state)
{
	if (!state) m_dack = channel;
}

WRITE_LINE_MEMBER(pc9801_state::dack0_w){ /*logerror("%02x 0\n",state);*/ set_dma_channel(0, state); }
WRITE_LINE_MEMBER(pc9801_state::dack1_w){ /*logerror("%02x 1\n",state);*/ set_dma_channel(1, state); }
WRITE_LINE_MEMBER(pc9801_state::dack2_w){ /*logerror("%02x 2\n",state);*/ set_dma_channel(2, state); }
WRITE_LINE_MEMBER(pc9801_state::dack3_w){ /*logerror("%02x 3\n",state);*/ set_dma_channel(3, state); }

/*
ch1 cs-4231a
ch2 FDC
ch3 SCSI
*/

/****************************************
*
* PPI interfaces
*
****************************************/

WRITE8_MEMBER(pc9801_state::ppi_sys_portc_w)
{
	m_beeper->set_state(!(data & 0x08));
}

READ8_MEMBER(pc9801_state::ppi_mouse_porta_r)
{
	uint8_t res;
	uint8_t isporthi;
	const char *const mousenames[] = { "MOUSE_X", "MOUSE_Y" };

	res = ioport("MOUSE_B")->read() & 0xf0;
	isporthi = ((m_mouse.control & 0x20) >> 5)*4;

	if((m_mouse.control & 0x80) == 0)
		res |= ioport(mousenames[(m_mouse.control & 0x40) >> 6])->read() >> (isporthi) & 0xf;
	else
	{
		if(m_mouse.control & 0x40)
			res |= (m_mouse.ly >> isporthi) & 0xf;
		else
			res |= (m_mouse.lx >> isporthi) & 0xf;
	}

//  logerror("A\n");
	return res;
}

WRITE8_MEMBER(pc9801_state::ppi_mouse_porta_w)
{
//  logerror("A %02x\n",data);
}

WRITE8_MEMBER(pc9801_state::ppi_mouse_portb_w)
{
//  logerror("B %02x\n",data);
}

WRITE8_MEMBER(pc9801_state::ppi_mouse_portc_w)
{
	if((m_mouse.control & 0x80) == 0 && data & 0x80)
	{
		m_mouse.lx = ioport("MOUSE_X")->read();
		m_mouse.ly = ioport("MOUSE_Y")->read();
	}

	m_mouse.control = data;
}

READ8_MEMBER(pc9801_state::unk_r)
{
	return 0xff;
}

/****************************************
*
* UPD765 interface
*
****************************************/

static void pc9801_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35hd", FLOPPY_35_HD);
}

static void pc9801_cbus_devices(device_slot_interface &device)
{
//  PC-9801-14
	device.option_add("pc9801_26", PC9801_26);
	device.option_add("pc9801_86", PC9801_86);
//  PC-9801-86
//  PC-9801-26 + PC-9801-86 (?)
//  PC-9801-86 + Chibi-Oto
	device.option_add("pc9801_118", PC9801_118);
	device.option_add("pc9801_spb", PC9801_SPEAKBOARD);
//  Spark Board
//  AMD-98 (AmuseMent boarD)
	device.option_add("pc9801_amd98", PC9801_AMD98);
	device.option_add("mpu_pc98", MPU_PC98);
}

//  Jast Sound, could be put independently

WRITE_LINE_MEMBER( pc9801_state::fdc_2dd_irq )
{
	logerror("IRQ 2DD %d\n",state);

	if(m_fdc_2dd_ctrl & 8)
	{
		m_pic2->ir2_w(state);
	}
}

WRITE_LINE_MEMBER( pc9801_state::pc9801rs_fdc_irq )
{
	/* 0xffaf8 */

	//logerror("%02x %d\n",m_fdc_ctrl,state);

	if(m_fdc_ctrl & 1)
		m_pic2->ir3_w(state);
	else
		m_pic2->ir2_w(state);
}

WRITE_LINE_MEMBER( pc9801_state::pc9801rs_fdc_drq )
{
	if(m_fdc_ctrl & 1)
		m_dmac->dreq2_w(state ^ 1);
	else
		m_dmac->dreq3_w(state ^ 1);
}

uint32_t pc9801_state::a20_286(bool state)
{
	return (state ? 0xffffff : 0x0fffff);
}

/****************************************
*
* Init emulation status
*
****************************************/

//
void pc9801_state::pc9801_palette(palette_device &palette) const
{
	for(int i = 0; i < 8; i++)
		palette.set_pen_color(i, pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));

	for(int i = 8; i < palette.entries(); i++)
		palette.set_pen_color(i, rgb_t::black());
}

MACHINE_START_MEMBER(pc9801_state,pc9801_common)
{
	m_rtc->cs_w(1);
	m_rtc->oe_w(1);

	int ram_size = m_ram->size() - (640*1024);

	address_space& space = m_maincpu->space(AS_PROGRAM);
	space.install_ram(0, (ram_size < 0) ? m_ram->size() - 1 : (640*1024) - 1, m_ram->pointer());
	if(ram_size > 0)
		space.install_ram(1024*1024, (1024*1024) + ram_size - 1, &m_ram->pointer()[(640*1024)]);

	save_item(NAME(m_sasi_data));
	save_item(NAME(m_sasi_data_enable));
	save_item(NAME(m_sasi_ctrl));
	save_pointer(NAME(m_egc.regs), 8);
}

MACHINE_START_MEMBER(pc9801_state,pc9801f)
{
	MACHINE_START_CALL_MEMBER(pc9801_common);

	m_fdc_2hd->set_rate(500000);
	m_fdc_2dd->set_rate(250000);
	m_sys_type = 0x00 >> 6;
}

MACHINE_START_MEMBER(pc9801_state,pc9801rs)
{
	MACHINE_START_CALL_MEMBER(pc9801_common);

	m_sys_type = 0x80 >> 6;
}

MACHINE_START_MEMBER(pc9801_state,pc9801bx2)
{
	MACHINE_START_CALL_MEMBER(pc9801rs);

	save_pointer(NAME(m_sdip), 24);
}


MACHINE_START_MEMBER(pc9801_state,pc9821)
{
	MACHINE_START_CALL_MEMBER(pc9801rs);

	save_pointer(NAME(m_sdip), 24);
}

MACHINE_START_MEMBER(pc9801_state,pc9821ap2)
{
	MACHINE_START_CALL_MEMBER(pc9821);

	// ...
}

MACHINE_RESET_MEMBER(pc9801_state,pc9801_common)
{
	memset(m_tvram.get(), 0, sizeof(uint16_t) * 0x2000);

	m_beeper->set_state(0);

	m_nmi_ff = 0;
	m_mouse.control = 0xff;
	m_mouse.freq_reg = 0;
	m_mouse.freq_index = 0;
	m_dma_autoinc[0] = m_dma_autoinc[1] = m_dma_autoinc[2] = m_dma_autoinc[3] = 0;
	memset(&m_egc, 0, sizeof(m_egc));
}

MACHINE_RESET_MEMBER(pc9801_state,pc9801f)
{
	MACHINE_RESET_CALL_MEMBER(pc9801_common);

	uint8_t op_mode;
	uint8_t *ROM;
	uint8_t *PRG = memregion("fdc_data")->base();
	int i;

	ROM = memregion("fdc_bios_2dd")->base();
	op_mode = (ioport("ROM_LOAD")->read() & 2) >> 1;

	for(i=0;i<0x1000;i++)
		ROM[i] = PRG[i+op_mode*0x8000];

	ROM = memregion("fdc_bios_2hd")->base();
	op_mode = ioport("ROM_LOAD")->read() & 1;

	for(i=0;i<0x1000;i++)
		ROM[i] = PRG[i+op_mode*0x8000+0x10000];
}

MACHINE_RESET_MEMBER(pc9801_state,pc9801rs)
{
	MACHINE_RESET_CALL_MEMBER(pc9801_common);

	m_gate_a20 = 0;
	m_fdc_ctrl = 3;
	m_access_ctrl = 0;
	m_ide_sel = 0;
	m_maincpu->set_input_line(INPUT_LINE_A20, m_gate_a20);

	if(memregion("ide"))
	{
		if(!(ioport("ROM_LOAD")->read() & 4))
			m_maincpu->space(AS_PROGRAM).install_rom(0xd8000, 0xd9fff, memregion("ide")->base());
		else
			m_maincpu->space(AS_PROGRAM).install_rom(0xd8000, 0xd9fff, memregion("ide")->base() + 0x2000);
	}
}

MACHINE_RESET_MEMBER(pc9801_state,pc9821)
{
	MACHINE_RESET_CALL_MEMBER(pc9801rs);

	m_pc9821_window_bank = 0x08;
}

WRITE_LINE_MEMBER(pc9801_state::vrtc_irq)
{
	if(state)
		m_pic1->ir2_w(1);
}


FLOPPY_FORMATS_MEMBER( pc9801_state::floppy_formats )
	FLOPPY_PC98_FORMAT,
	FLOPPY_PC98FDI_FORMAT,
	FLOPPY_FDD_FORMAT,
	FLOPPY_DCP_FORMAT,
	FLOPPY_DIP_FORMAT,
	FLOPPY_NFD_FORMAT
FLOPPY_FORMATS_END

TIMER_DEVICE_CALLBACK_MEMBER( pc9801_state::mouse_irq_cb )
{
	if((m_mouse.control & 0x10) == 0)
	{
		m_mouse.freq_index ++;

//      logerror("%02x\n",m_mouse.freq_index);
		if(m_mouse.freq_index > m_mouse.freq_reg)
		{
//          logerror("irq %02x\n",m_mouse.freq_reg);
			m_mouse.freq_index = 0;
			m_pic2->ir5_w(0);
			m_pic2->ir5_w(1);
		}
	}
}

void pc9801_atapi_devices(device_slot_interface &device)
{
	device.option_add("pc9801_cd", PC9801_CD);
}

void pc9801_state::pc9801_keyboard(machine_config &config)
{
	PC9801_KBD(config, m_keyb, 53);
	m_keyb->irq_wr_callback().set(m_pic1, FUNC(pic8259_device::ir1_w));
}

void pc9801_state::pc9801_mouse(machine_config &config)
{
	i8255_device &ppi_mouse(I8255(config, "ppi8255_mouse"));
	ppi_mouse.in_pa_callback().set(FUNC(pc9801_state::ppi_mouse_porta_r));
	ppi_mouse.out_pa_callback().set(FUNC(pc9801_state::ppi_mouse_porta_w));
	ppi_mouse.in_pb_callback().set_ioport("DSW3");
	ppi_mouse.out_pb_callback().set(FUNC(pc9801_state::ppi_mouse_portb_w));
	ppi_mouse.in_pc_callback().set_ioport("DSW4");
	ppi_mouse.out_pc_callback().set(FUNC(pc9801_state::ppi_mouse_portc_w));

	TIMER(config, "mouse_timer").configure_periodic(FUNC(pc9801_state::mouse_irq_cb), attotime::from_hz(120));
}

void pc9801_state::pc9801_cbus(machine_config &config)
{
	pc9801_slot_device &cbus0(PC9801CBUS_SLOT(config, "cbus0", pc9801_cbus_devices, "pc9801_26"));
	cbus0.set_memspace(m_maincpu, AS_PROGRAM);
	cbus0.set_iospace(m_maincpu, AS_IO);
	cbus0.int_cb<0>().set("ir3", FUNC(input_merger_device::in_w<0>));
	cbus0.int_cb<1>().set("ir5", FUNC(input_merger_device::in_w<0>));
	cbus0.int_cb<2>().set("ir6", FUNC(input_merger_device::in_w<0>));
	cbus0.int_cb<3>().set("ir9", FUNC(input_merger_device::in_w<0>));
	cbus0.int_cb<4>().set("pic8259_slave", FUNC(pic8259_device::ir2_w));
	cbus0.int_cb<5>().set("ir12", FUNC(input_merger_device::in_w<0>));
	cbus0.int_cb<6>().set("ir13", FUNC(input_merger_device::in_w<0>));

	pc9801_slot_device &cbus1(PC9801CBUS_SLOT(config, "cbus1", pc9801_cbus_devices, nullptr));
	cbus1.set_memspace(m_maincpu, AS_PROGRAM);
	cbus1.set_iospace(m_maincpu, AS_IO);
	cbus1.int_cb<0>().set("ir3", FUNC(input_merger_device::in_w<1>));
	cbus1.int_cb<1>().set("ir5", FUNC(input_merger_device::in_w<1>));
	cbus1.int_cb<2>().set("ir6", FUNC(input_merger_device::in_w<1>));
	cbus1.int_cb<3>().set("ir9", FUNC(input_merger_device::in_w<1>));
	cbus1.int_cb<4>().set("pic8259_slave", FUNC(pic8259_device::ir3_w));
	cbus1.int_cb<5>().set("ir12", FUNC(input_merger_device::in_w<1>));
	cbus1.int_cb<6>().set("ir13", FUNC(input_merger_device::in_w<1>));
//  TODO: six max slots

	INPUT_MERGER_ANY_HIGH(config, "ir3").output_handler().set("pic8259_master", FUNC(pic8259_device::ir3_w));
	INPUT_MERGER_ANY_HIGH(config, "ir5").output_handler().set("pic8259_master", FUNC(pic8259_device::ir5_w));
	INPUT_MERGER_ANY_HIGH(config, "ir6").output_handler().set("pic8259_master", FUNC(pic8259_device::ir6_w));
	INPUT_MERGER_ANY_HIGH(config, "ir9").output_handler().set("pic8259_slave", FUNC(pic8259_device::ir1_w));
	INPUT_MERGER_ANY_HIGH(config, "ir12").output_handler().set("pic8259_slave", FUNC(pic8259_device::ir4_w));
	INPUT_MERGER_ANY_HIGH(config, "ir13").output_handler().set("pic8259_slave", FUNC(pic8259_device::ir5_w));
}

void pc9801_state::pc9801_sasi(machine_config &config)
{
	SCSI_PORT(config, m_sasibus, 0);
	m_sasibus->set_data_input_buffer("sasi_data_in");
	m_sasibus->io_handler().set(FUNC(pc9801_state::write_sasi_io)); // bit2
	m_sasibus->cd_handler().set("sasi_ctrl_in", FUNC(input_buffer_device::write_bit3));
	m_sasibus->msg_handler().set("sasi_ctrl_in", FUNC(input_buffer_device::write_bit4));
	m_sasibus->bsy_handler().set("sasi_ctrl_in", FUNC(input_buffer_device::write_bit5));
	m_sasibus->ack_handler().set("sasi_ctrl_in", FUNC(input_buffer_device::write_bit6));
	m_sasibus->req_handler().set(FUNC(pc9801_state::write_sasi_req));
	m_sasibus->set_slot_device(1, "harddisk", PC9801_SASI, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_0));

	output_latch_device &sasi_out(OUTPUT_LATCH(config, "sasi_data_out"));
	m_sasibus->set_output_latch(sasi_out);
	INPUT_BUFFER(config, "sasi_data_in");
	INPUT_BUFFER(config, "sasi_ctrl_in");

	m_dmac->in_ior_callback<0>().set(FUNC(pc9801_state::sasi_data_r));
	m_dmac->out_iow_callback<0>().set(FUNC(pc9801_state::sasi_data_w));
}

void pc9801_state::cdrom_headphones(device_t *device)
{
	cdda_device *cdda = device->subdevice<cdda_device>("cdda");
	cdda->add_route(0, "^^lheadphone", 1.0);
	cdda->add_route(1, "^^rheadphone", 1.0);
}

void pc9801_state::pc9801_ide(machine_config &config)
{
	SPEAKER(config, "lheadphone").front_left();
	SPEAKER(config, "rheadphone").front_right();
	ATA_INTERFACE(config, m_ide[0]).options(ata_devices, "hdd", nullptr, false);
	m_ide[0]->irq_handler().set("ideirq", FUNC(input_merger_device::in_w<0>));
	ATA_INTERFACE(config, m_ide[1]).options(pc9801_atapi_devices, "pc9801_cd", nullptr, false);
	m_ide[1]->irq_handler().set("ideirq", FUNC(input_merger_device::in_w<1>));
	m_ide[1]->slot(0).set_option_machine_config("pc9801_cd", cdrom_headphones);

	INPUT_MERGER_ANY_HIGH(config, "ideirq").output_handler().set("pic8259_slave", FUNC(pic8259_device::ir1_w));

	SOFTWARE_LIST(config, "cd_list").set_original("pc98_cd");
}

void pc9801_state::pc9801_common(machine_config &config)
{
	PIT8253(config, m_pit8253, 0);
	m_pit8253->set_clk<0>(MAIN_CLOCK_X1); // heartbeat IRQ
	m_pit8253->out_handler<0>().set(m_pic1, FUNC(pic8259_device::ir0_w));
	m_pit8253->set_clk<1>(MAIN_CLOCK_X1); // Memory Refresh
	m_pit8253->set_clk<2>(MAIN_CLOCK_X1); // RS-232C
	m_pit8253->out_handler<2>().set(m_sio, FUNC(i8251_device::write_txc));
	m_pit8253->out_handler<2>().append(m_sio, FUNC(i8251_device::write_rxc));

	AM9517A(config, m_dmac, 5000000); // unknown clock, TODO: check channels 0 - 1
	m_dmac->out_hreq_callback().set(FUNC(pc9801_state::dma_hrq_changed));
	m_dmac->out_eop_callback().set(FUNC(pc9801_state::tc_w));
	m_dmac->in_memr_callback().set(FUNC(pc9801_state::dma_read_byte));
	m_dmac->out_memw_callback().set(FUNC(pc9801_state::dma_write_byte));
	m_dmac->in_ior_callback<2>().set(m_fdc_2hd, FUNC(upd765a_device::dma_r));
	m_dmac->out_iow_callback<2>().set(m_fdc_2hd, FUNC(upd765a_device::dma_w));
	m_dmac->out_dack_callback<0>().set(FUNC(pc9801_state::dack0_w));
	m_dmac->out_dack_callback<1>().set(FUNC(pc9801_state::dack1_w));
	m_dmac->out_dack_callback<2>().set(FUNC(pc9801_state::dack2_w));
	m_dmac->out_dack_callback<3>().set(FUNC(pc9801_state::dack3_w));

	PIC8259(config, m_pic1, 0);
	m_pic1->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic1->in_sp_callback().set_constant(1);
	m_pic1->read_slave_ack_callback().set(FUNC(pc9801_state::get_slave_ack));

	PIC8259(config, m_pic2, 0);
	m_pic2->out_int_callback().set(m_pic1, FUNC(pic8259_device::ir7_w)); // TODO: Check ir7_w
	m_pic2->in_sp_callback().set_constant(0);

	I8255(config, m_ppi_sys, 0);
	m_ppi_sys->in_pa_callback().set_ioport("DSW2");
	m_ppi_sys->in_pb_callback().set_ioport("DSW1");
	m_ppi_sys->in_pc_callback().set_constant(0xa0); // 0x80 cpu triple fault reset flag?
	m_ppi_sys->out_pc_callback().set(FUNC(pc9801_state::ppi_sys_portc_w));

	I8255(config, m_ppi_prn, 0);
	// TODO: check this one
	m_ppi_prn->in_pb_callback().set_ioport("DSW5");

	pc9801_keyboard(config);
	pc9801_mouse(config);
	pc9801_cbus(config);

	I8251(config, m_sio, 0);

	PC9801_MEMSW(config, m_memsw, 0);

	UPD765A(config, m_fdc_2hd, 8'000'000, true, true);
	m_fdc_2hd->intrq_wr_callback().set(m_pic2, FUNC(pic8259_device::ir3_w));
	m_fdc_2hd->drq_wr_callback().set(m_dmac, FUNC(am9517a_device::dreq2_w)).invert();
	FLOPPY_CONNECTOR(config, "upd765_2hd:0", pc9801_floppies, "525hd", pc9801_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765_2hd:1", pc9801_floppies, "525hd", pc9801_state::floppy_formats);

	i8255_device &ppi_fdd(I8255(config, "ppi8255_fdd"));
	ppi_fdd.in_pa_callback().set_constant(0xff);
	ppi_fdd.in_pb_callback().set_constant(0xff);
	ppi_fdd.in_pc_callback().set_constant(0xff);
	//ppi_fdd.out_pc_callback().set(FUNC(pc9801_state::ppi_fdd_portc_w));

	SOFTWARE_LIST(config, "disk_list").set_original("pc98");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(21.0526_MHz_XTAL, 848, 0, 640, 440, 0, 400);
	m_screen->set_screen_update(FUNC(pc9801_state::screen_update));
	m_screen->screen_vblank().set(FUNC(pc9801_state::vrtc_irq));

	UPD7220(config, m_hgdc1, 21.0526_MHz_XTAL / 8);
	m_hgdc1->set_addrmap(0, &pc9801_state::upd7220_1_map);
	m_hgdc1->set_draw_text(FUNC(pc9801_state::hgdc_draw_text));
	m_hgdc1->vsync_wr_callback().set(m_hgdc2, FUNC(upd7220_device::ext_sync_w));

	UPD7220(config, m_hgdc2, 21.0526_MHz_XTAL / 8);
	m_hgdc2->set_addrmap(0, &pc9801_state::upd7220_2_map);
	m_hgdc2->set_display_pixels(FUNC(pc9801_state::hgdc_display_pixels));

	SPEAKER(config, "mono").front_center();

	BEEP(config, m_beeper, 2400).add_route(ALL_OUTPUTS, "mono", 0.15);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pc9801);
}

void pc9801_state::pc9801(machine_config &config)
{
	I8086(config, m_maincpu, 5000000); //unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9801_state::pc9801_map);
	m_maincpu->set_addrmap(AS_IO, &pc9801_state::pc9801_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	pc9801_common(config);

	MCFG_MACHINE_START_OVERRIDE(pc9801_state, pc9801f)
	MCFG_MACHINE_RESET_OVERRIDE(pc9801_state, pc9801f)

	// TODO: maybe force dips to avoid beep error
	RAM(config, m_ram).set_default_size("640K").set_extra_options("128K,256K,384K,512K");

	UPD765A(config, m_fdc_2dd, 8'000'000, false, true);
	m_fdc_2dd->intrq_wr_callback().set(FUNC(pc9801_state::fdc_2dd_irq));
	m_fdc_2dd->drq_wr_callback().set(m_dmac, FUNC(am9517a_device::dreq3_w)).invert();
	FLOPPY_CONNECTOR(config, "upd765_2dd:0", pc9801_floppies, "525dd", pc9801_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765_2dd:1", pc9801_floppies, "525dd", pc9801_state::floppy_formats);

	pc9801_sasi(config);
	UPD1990A(config, m_rtc);

	m_dmac->in_ior_callback<3>().set(m_fdc_2dd, FUNC(upd765a_device::dma_r));
	m_dmac->out_iow_callback<3>().set(m_fdc_2dd, FUNC(upd765a_device::dma_w));

	PALETTE(config, m_palette, FUNC(pc9801_state::pc9801_palette), 16);
}


void pc9801_state::pc9801rs(machine_config &config)
{
	I386SX(config, m_maincpu, MAIN_CLOCK_X1*8); // unknown clock.
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9801_state::pc9801rs_map);
	m_maincpu->set_addrmap(AS_IO, &pc9801_state::pc9801rs_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	pc9801_common(config);

	ADDRESS_MAP_BANK(config, "ipl_bank").set_map(&pc9801_state::ipl_bank).set_options(ENDIANNESS_LITTLE, 16, 18, 0x18000);

	MCFG_MACHINE_START_OVERRIDE(pc9801_state, pc9801rs)
	MCFG_MACHINE_RESET_OVERRIDE(pc9801_state, pc9801rs)

	m_dmac->set_clock(MAIN_CLOCK_X1*8); // unknown clock

	pc9801_ide(config);
	UPD4990A(config, m_rtc);

	RAM(config, m_ram).set_default_size("1664K").set_extra_options("640K,3712K,7808K,14M");

	m_hgdc2->set_addrmap(0, &pc9801_state::upd7220_grcg_2_map);

	PALETTE(config, m_palette, FUNC(pc9801_state::pc9801_palette), 16 + 16);
}

void pc9801_state::pc9801vm(machine_config &config)
{
	pc9801rs(config);
	V30(config.replace(), m_maincpu, 10000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9801_state::pc9801ux_map);
	m_maincpu->set_addrmap(AS_IO, &pc9801_state::pc9801ux_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	m_ram->set_default_size("640K").set_extra_options("640K"); // ???

	MCFG_MACHINE_START_OVERRIDE(pc9801_state, pc9801rs)
	MCFG_MACHINE_RESET_OVERRIDE(pc9801_state, pc9801_common)
}

void pc9801_state::pc9801ux(machine_config &config)
{
	pc9801rs(config);
	i80286_cpu_device &maincpu(I80286(config.replace(), m_maincpu, 10000000));
	maincpu.set_addrmap(AS_PROGRAM, &pc9801_state::pc9801ux_map);
	maincpu.set_addrmap(AS_IO, &pc9801_state::pc9801ux_io);
	maincpu.set_a20_callback(i80286_cpu_device::a20_cb(&pc9801_state::a20_286, this));
	maincpu.set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));
//  AM9157A(config, "i8237", 10000000); // unknown clock
}

void pc9801_state::pc9801bx2(machine_config &config)
{
	pc9801rs(config);
	I486(config.replace(), m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9801_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9801_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	MCFG_MACHINE_START_OVERRIDE(pc9801_state, pc9801bx2)
}

void pc9801_state::pc9821(machine_config &config)
{
	pc9801rs(config);
	I486(config.replace(), m_maincpu, 16000000); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9801_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9801_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	m_pit8253->set_clk<0>(MAIN_CLOCK_X2);
	m_pit8253->set_clk<1>(MAIN_CLOCK_X2);
	m_pit8253->set_clk<2>(MAIN_CLOCK_X2);

	MCFG_MACHINE_START_OVERRIDE(pc9801_state, pc9821)
	MCFG_MACHINE_RESET_OVERRIDE(pc9801_state, pc9821)

	m_dmac->set_clock(16000000); // unknown clock

	PALETTE(config.replace(), m_palette, FUNC(pc9801_state::pc9801_palette), 16 + 16 + 256);
}

void pc9801_state::pc9821as(machine_config &config)
{
	pc9821(config);
	I486(config.replace(), m_maincpu, 33000000); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9801_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9801_state::pc9821as_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));
}

void pc9801_state::pc9821ap2(machine_config &config)
{
	pc9821(config);
	I486(config.replace(), m_maincpu, 66666667); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9801_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9801_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	MCFG_MACHINE_START_OVERRIDE(pc9801_state, pc9821ap2)
}

void pc9801_state::pc9821v20(machine_config &config)
{
	pc9821(config);
	PENTIUM(config.replace(), m_maincpu, 32000000); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &pc9801_state::pc9821_map);
	m_maincpu->set_addrmap(AS_IO, &pc9801_state::pc9821_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));
}

/* took from "raw" memory dump, uncomment ROM_FILL if you want to play with it */
#define LOAD_IDE_ROM \
	ROM_REGION( 0x4000, "ide", ROMREGION_ERASEVAL(0xcb) ) \
	ROM_LOAD( "d8000.rom", 0x0000, 0x2000, BAD_DUMP CRC(5dda57cc) SHA1(d0dead41c5b763008a4d777aedddce651eb6dcbb) ) \
	ROM_IGNORE( 0x2000 ) \
	ROM_IGNORE( 0x2000 ) \
	ROM_IGNORE( 0x2000 )

// all of these are half size :/
#define LOAD_KANJI_ROMS \
	ROM_REGION( 0x80000, "raw_kanji", ROMREGION_ERASEFF ) \
	ROM_LOAD16_BYTE( "24256c-x01.bin", 0x00000, 0x4000, BAD_DUMP CRC(28ec1375) SHA1(9d8e98e703ce0f483df17c79f7e841c5c5cd1692) ) \
	ROM_CONTINUE(                      0x20000, 0x4000  ) \
	ROM_LOAD16_BYTE( "24256c-x02.bin", 0x00001, 0x4000, BAD_DUMP CRC(90985158) SHA1(78fb106131a3f4eb054e87e00fe4f41193416d65) ) \
	ROM_CONTINUE(                      0x20001, 0x4000  ) \
	ROM_LOAD16_BYTE( "24256c-x03.bin", 0x40000, 0x4000, BAD_DUMP CRC(d4893543) SHA1(eb8c1bee0f694e1e0c145a24152222d4e444e86f) ) \
	ROM_CONTINUE(                      0x60000, 0x4000  ) \
	ROM_LOAD16_BYTE( "24256c-x04.bin", 0x40001, 0x4000, BAD_DUMP CRC(5dec0fc2) SHA1(41000da14d0805ed0801b31eb60623552e50e41c) ) \
	ROM_CONTINUE(                      0x60001, 0x4000  ) \
	ROM_REGION( 0x100000, "kanji", ROMREGION_ERASEFF ) \
	ROM_REGION( 0x80000, "new_chargen", ROMREGION_ERASEFF )
/*
F - 8086 5
*/

ROM_START( pc9801f )
	ROM_REGION16_LE( 0x18000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "urm01-02.bin", 0x00000, 0x4000, CRC(cde04615) SHA1(8f6fb587c0522af7a8131b45d13f8ae8fc60e8cd) )
	ROM_LOAD16_BYTE( "urm02-02.bin", 0x00001, 0x4000, CRC(9e39b8d1) SHA1(df1f3467050a41537cb9d071e4034f0506f07eda) )
	ROM_LOAD16_BYTE( "urm03-02.bin", 0x08000, 0x4000, CRC(95e79064) SHA1(c27d96949fad82aeb26e316200c15a4891e1063f) )
	ROM_LOAD16_BYTE( "urm04-02.bin", 0x08001, 0x4000, CRC(e4855a53) SHA1(223f66482c77409706cfc64c214cec7237c364e9) )
	ROM_LOAD16_BYTE( "urm05-02.bin", 0x10000, 0x4000, CRC(ffefec65) SHA1(106e0d920e857e59da12225a489ca2756ca405c1) )
	ROM_LOAD16_BYTE( "urm06-02.bin", 0x10001, 0x4000, CRC(1147760b) SHA1(4e0299091dfd53ac7988d40c5a6775a10389faac) )

	ROM_REGION16_LE( 0x1000, "fdc_bios_2dd", ROMREGION_ERASEFF )

	ROM_REGION16_LE( 0x1000, "fdc_bios_2hd", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "fdc_data", ROMREGION_ERASEFF ) // 2dd fdc bios, presumably bad size (should be 0x800 for each rom)
	ROM_LOAD16_BYTE( "urf01-01.bin", 0x00000, 0x4000, BAD_DUMP CRC(2f5ae147) SHA1(69eb264d520a8fc826310b4fce3c8323867520ee) )
	ROM_LOAD16_BYTE( "urf02-01.bin", 0x00001, 0x4000, BAD_DUMP CRC(62a86928) SHA1(4160a6db096dbeff18e50cbee98f5d5c1a29e2d1) )
	ROM_LOAD( "2hdif.rom", 0x10000, 0x1000, BAD_DUMP CRC(9652011b) SHA1(b607707d74b5a7d3ba211825de31a8f32aec8146) ) // needs dumping from a board

	ROM_REGION( 0x800, "kbd_mcu", ROMREGION_ERASEFF)
	ROM_LOAD( "mcu.bin", 0x0000, 0x0800, NO_DUMP ) //connected through a i8251 UART, needs decapping

	/* note: ROM names of following two might be swapped */
	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "d23128c-17.bin", 0x00000, 0x00800, BAD_DUMP CRC(eea57180) SHA1(4aa037c684b72ad4521212928137d3369174eb1e) ) //original is a bad dump, this is taken from i386 model
	ROM_LOAD("hn613128pac8.bin",0x00800, 0x01000, BAD_DUMP CRC(b5a15b5c) SHA1(e5f071edb72a5e9a8b8b1c23cf94a74d24cb648e) ) //bad dump, 8x16 charset? (it's on the kanji board)

	LOAD_KANJI_ROMS
ROM_END

/*
UX - 80286 10 + V30 8
*/

ROM_START( pc9801ux )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf_ux.rom",  0x10000, 0x08000, CRC(c7942563) SHA1(61bb210d64c7264be939b11df1e9cd14ffeee3c9) )
	ROM_LOAD( "bios_ux.rom", 0x18000, 0x18000, BAD_DUMP CRC(97375ca2) SHA1(bfe458f671d90692104d0640730972ca8dc0a100) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_ux.rom",     0x000000, 0x046800, BAD_DUMP CRC(19a76eeb) SHA1(96a006e8515157a624599c2b53a581ae0dd560fd) )

	LOAD_KANJI_ROMS
//  LOAD_IDE_ROM
ROM_END

/*
RX - 80286 12 (no V30?)

The bios is from a 386 model not an RX
*/

ROM_START( pc9801rx )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf_rs.rom",  0x10000, 0x08000, BAD_DUMP CRC(c1815325) SHA1(a2fb11c000ed7c976520622cfb7940ed6ddc904e) )
	ROM_LOAD( "bios_rx.rom", 0x18000, 0x18000, BAD_DUMP CRC(0a682b93) SHA1(76a7360502fa0296ea93b4c537174610a834d367) )
	// fix csum
	ROM_FILL(0x2fffe, 1, 0x0d)

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_rx.rom",     0x000000, 0x046800, BAD_DUMP CRC(456d9fc7) SHA1(78ba9960f135372825ab7244b5e4e73a810002ff) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
RS - 386SX 16

(note: might be a different model!)
*/

ROM_START( pc9801rs )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf_rs.rom",  0x10000, 0x08000, CRC(c1815325) SHA1(a2fb11c000ed7c976520622cfb7940ed6ddc904e) )
	ROM_LOAD( "bios_rs.rom", 0x18000, 0x18000, BAD_DUMP CRC(315d2703) SHA1(4f208d1dbb68373080d23bff5636bb6b71eb7565) )

	/* following is an emulator memory dump, should be checked and nuked */
	ROM_REGION( 0x100000, "memory", 0 )
	ROM_LOAD( "00000.rom", 0x00000, 0x8000, CRC(6e299128) SHA1(d0e7d016c005cdce53ea5ecac01c6f883b752b80) )
	ROM_LOAD( "c0000.rom", 0xc0000, 0x8000, CRC(1b43eabd) SHA1(ca711c69165e1fa5be72993b9a7870ef6d485249) )  // 0xff everywhere
	ROM_LOAD( "c8000.rom", 0xc8000, 0x8000, CRC(f2a262b0) SHA1(fe97d2068d18bbb7425d9774e2e56982df2aa1fb) )
	ROM_LOAD( "d0000.rom", 0xd0000, 0x8000, CRC(1b43eabd) SHA1(ca711c69165e1fa5be72993b9a7870ef6d485249) )  // 0xff everywhere
	ROM_LOAD( "e8000.rom", 0xe8000, 0x8000, CRC(4e32081e) SHA1(e23571273b7cad01aa116cb7414c5115a1093f85) )  // contains n-88 basic (86) v2.0
	ROM_LOAD( "f0000.rom", 0xf0000, 0x8000, CRC(4da85a6c) SHA1(18dccfaf6329387c0c64cc4c91b32c25cde8bd5a) )
	ROM_LOAD( "f8000.rom", 0xf8000, 0x8000, CRC(2b1e45b1) SHA1(1fec35f17d96b2e2359e3c71670575ad9ff5007e) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_rs.rom", 0x00000, 0x46800, BAD_DUMP CRC(da370e7a) SHA1(584d0c7fde8c7eac1f76dc5e242102261a878c5e) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
BX2/U2 - 486SX - (should be 33, but "dumper" note says it's 25 MHz)

Yet another franken-dump done with a lame program, shrug

*/

ROM_START( pc9801bx2 )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "pc98bank0.bin",  0x00000, 0x08000, BAD_DUMP CRC(bfd100cc) SHA1(cf8e6a5679cca7761481abef0ba4b35ead39efdb) )
	ROM_LOAD( "pc98bank1.bin",  0x00000, 0x08000, BAD_DUMP CRC(d0562af8) SHA1(2c4fd27eb598f4b8a00f3e86941ba27007d58e47) )
	ROM_LOAD( "pc98bank2.bin",  0x00000, 0x08000, BAD_DUMP CRC(12818a14) SHA1(9c31e8ac85d78fa779d6bbc2095557065294ec09) )
	ROM_LOAD( "pc98bank3.bin",  0x00000, 0x08000, BAD_DUMP CRC(d0bda44e) SHA1(c1022a3b2be4d2a1e43914df9e4605254e5f99d5) )
	ROM_LOAD( "pc98bank4.bin",  0x10000, 0x08000, BAD_DUMP CRC(be8092f4) SHA1(12c8a166b8c6ebbef85568b67e1f098562883365) )
	ROM_LOAD( "pc98bank5.bin",  0x18000, 0x08000, BAD_DUMP CRC(4e32081e) SHA1(e23571273b7cad01aa116cb7414c5115a1093f85) )
	ROM_LOAD( "pc98bank6.bin",  0x20000, 0x08000, BAD_DUMP CRC(f878c160) SHA1(cad47f09075ffe4f7b51bb937c9f716c709d4596) )
	ROM_LOAD( "pc98bank7.bin",  0x28000, 0x08000, BAD_DUMP CRC(1bd6537b) SHA1(ff9ee1c976a12b87851635ce8991ac4ad607675b) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_rs.rom", 0x00000, 0x46800, BAD_DUMP CRC(da370e7a) SHA1(584d0c7fde8c7eac1f76dc5e242102261a878c5e) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
VM - V30 8/10

TODO: missing itf roms, if they exist
*/

ROM_START( pc9801vm )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
//  ROM_LOAD( "itf_ux.rom",  0x10000, 0x08000, BAD_DUMP CRC(c7942563) SHA1(61bb210d64c7264be939b11df1e9cd14ffeee3c9) )
//  ROM_LOAD( "bios_vm.rom", 0x18000, 0x18000, CRC(2e2d7cee) SHA1(159549f845dc70bf61955f9469d2281a0131b47f) )
	// bios
	ROM_LOAD16_BYTE( "cpu_board_1a_23128e.bin",   0x10001, 0x4000, CRC(9965c914) SHA1(1ed318b774340bd532ef02ac02f39a012354dbf8) )
	ROM_LOAD16_BYTE( "cpu_board_4a_d23128ec.bin", 0x10000, 0x4000, CRC(e7c24a70) SHA1(cc9584b8e56b391f103e9d559d397d0bc6d00b35) )
	ROM_LOAD16_BYTE( "cpu_board_2a_d23c256ec.bin", 0x08001, 0x4000, CRC(3874970d) SHA1(e50ec5ae38f00dbfd156288dd42c7f2a2bf8bc35) )
	ROM_CONTINUE( 0x00001, 0x4000 )
	ROM_LOAD16_BYTE( "cpu_board_3a_23c256e.bin",   0x08000, 0x4000, CRC(4128276e) SHA1(32acb7eee779a31838a17ce51b05a9a987af4099) )
	ROM_CONTINUE( 0x00000, 0x4000 )

	ROM_REGION( 0x80000, "chargen", 0 )
//  ROM_LOAD( "font_vm.rom",     0x000000, 0x046800, BAD_DUMP CRC(456d9fc7) SHA1(78ba9960f135372825ab7244b5e4e73a810002ff) )
	// TODO: contains 8x8 "graphics" characters but we don't use them
	ROM_LOAD( "main_board_12f_d2364ec.bin", 0x000000, 0x002000, CRC(11197271) SHA1(8dbd2f25daeed545ea2c74d849f0a209ceaf4dd7) )

	ROM_REGION( 0x80000, "raw_kanji", ROMREGION_ERASEFF )
	// on main board, uPD23100 type roms
	// kanji and most other 16x16 characters
	ROM_LOAD16_BYTE( "main_board_12h_231000.bin", 0x00000, 0x20000, CRC(ecc2c062) SHA1(36c935c0f26c02a2b1ea46f5b6cd03fc11c7b003) )
	ROM_LOAD16_BYTE( "main_board_10h_231000.bin", 0x00001, 0x20000, CRC(91d78281) SHA1(85a18ad40e281e68071f91800201e43d78fb4f1c) )
	// 8x16 characters and the remaining 16x16 characters, with inverted bit order like 12f
	ROM_LOAD16_BYTE( "main_board_8h_d23256ac.bin", 0x40000, 0x04000, CRC(62a32ba6) SHA1(cdab480ae0dad9d128e52afb15e6c0b2b122cc3f) )
	ROM_CONTINUE( 0x40001, 0x04000 )

	ROM_REGION( 0x100000, "kanji", ROMREGION_ERASEFF )
	ROM_REGION( 0x80000, "new_chargen", ROMREGION_ERASEFF )

//  LOAD_KANJI_ROMS
//  LOAD_IDE_ROM
ROM_END

/*
VM11 - V30 8/10

TODO: this ISN'T a real VM11 model!
*/

ROM_START( pc9801vm11 )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf_ux.rom",  0x10000, 0x08000, BAD_DUMP CRC(c7942563) SHA1(61bb210d64c7264be939b11df1e9cd14ffeee3c9) )
	ROM_LOAD( "bios_vm.rom", 0x18000, 0x18000, CRC(2e2d7cee) SHA1(159549f845dc70bf61955f9469d2281a0131b47f) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_vm.rom",     0x000000, 0x046800, BAD_DUMP CRC(456d9fc7) SHA1(78ba9960f135372825ab7244b5e4e73a810002ff) )

	LOAD_KANJI_ROMS
//  LOAD_IDE_ROM
ROM_END



/*
98MATE A - 80486SX 25

(note: might be a different model!)
*/

ROM_START( pc9821 )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf.rom",  0x10000, 0x08000, CRC(dd4c7bb8) SHA1(cf3aa193df2722899066246bccbed03f2e79a74a) )
	ROM_LOAD( "bios.rom", 0x18000, 0x18000, BAD_DUMP CRC(34a19a59) SHA1(2e92346727b0355bc1ec9a7ded1b444a4917f2b9) )
	ROM_FILL(0x24c40, 4, 0) // hide the _32_ marker until we have a 32-bit clean IDE bios otherwise windows tries to
							// make a 32-bit call into 16-bit code
	ROM_FILL(0x27ffe, 1, 0x92)
	ROM_FILL(0x27fff, 1, 0xd7)

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x00000, 0x46800, BAD_DUMP CRC(a61c0649) SHA1(554b87377d176830d21bd03964dc71f8e98676b1) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
As - 80486DX 33
*/

ROM_START( pc9821as )
	ROM_REGION( 0x80000, "biosrom", ROMREGION_ERASEFF )
//  ROM_LOAD( "itf.rom",     0x10000, 0x08000, BAD_DUMP CRC(dd4c7bb8) SHA1(cf3aa193df2722899066246bccbed03f2e79a74a) )
//  ROM_LOAD( "bios_as.rom", 0x18000, 0x18000, BAD_DUMP CRC(0a682b93) SHA1(76a7360502fa0296ea93b4c537174610a834d367) )
	ROM_LOAD( "mvs0100-1.bin", 0x00000, 0x80000, CRC(ca37b631) SHA1(8c481dd0608d6c27235bc88bd77e345628dc28a1) )

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	// TODO: not quite right yet, needs actual itf rom loaded
	ROM_COPY( "biosrom", 0x20000, 0x10000, 0x08000 )
	ROM_COPY( "biosrom", 0x30000, 0x18000, 0x18000 )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_as.rom",     0x000000, 0x046800, BAD_DUMP CRC(456d9fc7) SHA1(78ba9960f135372825ab7244b5e4e73a810002ff) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
PC-9821AP2/U8W
80486DX2 66MHz
DOS 5.0, Windows 3.1
5.6MB RAM, up to 73.6MB
340MB HD
Expansion slot C-BUS4 (4)
Graphics controller S3 86C928
*/

ROM_START( pc9821ap2 )
	ROM_REGION( 0x80000, "biosrom", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("phd0104")
	ROM_SYSTEM_BIOS(0, "phd0104",  "PHD0104")
	ROMX_LOAD( "phd0104.rom",     0x000000, 0x80000, CRC(da73b372) SHA1(2c15b63a0869b81ef7f04972dbb0975f4e77d384), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "phd0102",  "PHD0102")
	ROMX_LOAD( "phd0102.rom",     0x000000, 0x80000, CRC(3036774c) SHA1(59856a348f156adf5eca06326f967aca54ff871c), ROM_BIOS(1) )

	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF ) // TODO: identify ROM banks
	ROM_COPY( "biosrom", 0x20000, 0x10000, 0x08000 )
	ROM_COPY( "biosrom", 0x30000, 0x18000, 0x18000 )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x00000, 0x46800, BAD_DUMP CRC(a61c0649) SHA1(554b87377d176830d21bd03964dc71f8e98676b1) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END


/*
98NOTE - i486SX 33
*/

ROM_START( pc9821ne )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf.rom",     0x10000, 0x08000, CRC(dd4c7bb8) SHA1(cf3aa193df2722899066246bccbed03f2e79a74a) )
	ROM_LOAD( "bios_ne.rom", 0x18000, 0x18000, BAD_DUMP CRC(2ae070c4) SHA1(d7963942042bfd84ed5fc9b7ba8f1c327c094172) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_ne.rom", 0x00000, 0x46800, BAD_DUMP CRC(fb213757) SHA1(61525826d62fb6e99377b23812faefa291d78c2e) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
Epson PC-486MU - 486 based, unknown clock
*/

ROM_START( pc486mu )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "bios_486mu.rom", 0x00000, 0x18000, BAD_DUMP CRC(57b5d701) SHA1(15029800842e93e07615b0fd91fb9f2bfe3e3c24))
	ROM_RELOAD(                 0x18000, 0x18000 ) // missing rom?

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_486mu.rom", 0x0000, 0x46800, CRC(456d9fc7) SHA1(78ba9960f135372825ab7244b5e4e73a810002ff))

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
98MULTi Ce2 - 80486SX 25
*/

ROM_START( pc9821ce2 )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf_ce2.rom",  0x10000, 0x08000, CRC(273e9e88) SHA1(9bca7d5116788776ed0f297bccb4dfc485379b41) )
	ROM_LOAD( "bios_ce2.rom", 0x18000, 0x018000, BAD_DUMP CRC(76affd90) SHA1(910fae6763c0cd59b3957b6cde479c72e21f33c1) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_ce2.rom",     0x000000, 0x046800, CRC(d1c2702a) SHA1(e7781e9d35b6511d12631641d029ad2ba3f7daef) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
98MATE X - 486/Pentium based
*/

ROM_START( pc9821xs )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf.rom",  0x10000, 0x08000, BAD_DUMP CRC(dd4c7bb8) SHA1(cf3aa193df2722899066246bccbed03f2e79a74a) )
	ROM_LOAD( "bios_xs.rom",     0x18000, 0x018000, BAD_DUMP CRC(0a682b93) SHA1(76a7360502fa0296ea93b4c537174610a834d367) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_xs.rom",     0x000000, 0x046800, BAD_DUMP CRC(c9a77d8f) SHA1(deb8563712eb2a634a157289838b95098ba0c7f2) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END


/*
98MATE VALUESTAR - Pentium based
*/

ROM_START( pc9821v13 )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf.rom",      0x10000, 0x08000, CRC(dd4c7bb8) SHA1(cf3aa193df2722899066246bccbed03f2e79a74a) )
	ROM_LOAD( "bios_v13.rom", 0x18000, 0x18000, BAD_DUMP CRC(0a682b93) SHA1(76a7360502fa0296ea93b4c537174610a834d367) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_a.rom", 0x00000, 0x46800, BAD_DUMP CRC(c9a77d8f) SHA1(deb8563712eb2a634a157289838b95098ba0c7f2) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END

/*
98MATE VALUESTAR - Pentium based
*/

ROM_START( pc9821v20 )
	ROM_REGION16_LE( 0x30000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "itf_v20.rom",  0x10000, 0x08000, CRC(10e52302) SHA1(f95b8648e3f5a23e507a9fbda8ab2e317d8e5151) )
	ROM_LOAD( "bios_v20.rom", 0x18000, 0x18000, BAD_DUMP CRC(d5d1f13b) SHA1(bf44b5f4e138e036f1b848d6616fbd41b5549764) )

	ROM_REGION( 0x80000, "chargen", 0 )
	ROM_LOAD( "font_v20.rom",     0x000000, 0x046800, BAD_DUMP CRC(6244c4c0) SHA1(9513cac321e89b4edb067b30e9ecb1adae7e7be7) )

	LOAD_KANJI_ROMS
	LOAD_IDE_ROM
ROM_END



void pc9801_state::init_pc9801_kanji()
{
	#define copy_kanji_strip(_dst,_src,_fill_type) \
	for (uint32_t i = _dst, k = _src; i < _dst + 0x20; i++, k++) \
	{ \
		for (uint32_t j = 0; j < 0x20; j++) \
			kanji[j+(i << 5)] = _fill_type ? new_chargen[j+(k << 5)] : 0; \
	}
	uint32_t pcg_tile;
	uint8_t *kanji = memregion("kanji")->base();
	uint8_t *raw_kanji = memregion("raw_kanji")->base();
	uint8_t *new_chargen = memregion("new_chargen")->base();
	uint8_t *chargen = memregion("chargen")->base();

	/* Convert the ROM bitswap here from the original structure */
	/* TODO: kanji bitswap should be completely wrong, will check it out once that a dump is remade. */
	for (uint32_t i = 0; i < 0x80000 / 0x20; i++)
	{
		for (uint32_t j = 0; j < 0x20; j++)
		{
			pcg_tile = bitswap<16>(i,15,14,13,12,11,7,6,5,10,9,8,4,3,2,1,0) << 5;
			kanji[j+(i << 5)] = raw_kanji[j+pcg_tile];
		}
	}

	/* convert charset into even/odd structure */
	for (uint32_t i = 0; i < 0x80000 / 0x20; i++)
	{
		for (uint32_t j = 0; j < 0x10; j++)
		{
			new_chargen[j*2 + (i << 5)] = chargen[j + (i << 5)];
			new_chargen[j*2 + (i << 5) + 1] = chargen[j + (i << 5) + 0x10];
		}
	}

	/* now copy the data from the fake roms into our kanji struct */
	copy_kanji_strip(0x0800,   -1,0); copy_kanji_strip(0x0820,   -1,0); copy_kanji_strip(0x0840,   -1,0); copy_kanji_strip(0x0860,   -1,0);
	copy_kanji_strip(0x0900,   -1,0); copy_kanji_strip(0x0920,0x3c0,1); copy_kanji_strip(0x0940,0x3e0,1); copy_kanji_strip(0x0960,0x400,1);
	copy_kanji_strip(0x0a00,   -1,0); copy_kanji_strip(0x0a20,0x420,1); copy_kanji_strip(0x0a40,0x440,1); copy_kanji_strip(0x0a60,0x460,1);
	copy_kanji_strip(0x0b00,   -1,0); copy_kanji_strip(0x0b20,0x480,1); copy_kanji_strip(0x0b40,0x4a0,1); copy_kanji_strip(0x0b60,0x4c0,1);
	copy_kanji_strip(0x0c00,   -1,0); copy_kanji_strip(0x0c20,0x4e0,1); copy_kanji_strip(0x0c40,0x500,1); copy_kanji_strip(0x0c60,0x520,1);
	copy_kanji_strip(0x0d00,   -1,0); copy_kanji_strip(0x0d20,0x540,1); copy_kanji_strip(0x0d40,0x560,1); copy_kanji_strip(0x0d60,0x580,1);
	copy_kanji_strip(0x0e00,   -1,0); copy_kanji_strip(0x0e20,   -1,0); copy_kanji_strip(0x0e40,   -1,0); copy_kanji_strip(0x0e60,   -1,0);
	copy_kanji_strip(0x0f00,   -1,0); copy_kanji_strip(0x0f20,   -1,0); copy_kanji_strip(0x0f40,   -1,0); copy_kanji_strip(0x0f60,   -1,0);
	{
		int src_1,dst_1;

		for(src_1=0x1000,dst_1=0x660;src_1<0x8000;src_1+=0x100,dst_1+=0x60)
		{
			copy_kanji_strip(src_1,             -1,0);
			copy_kanji_strip(src_1+0x20,dst_1+0x00,1);
			copy_kanji_strip(src_1+0x40,dst_1+0x20,1);
			copy_kanji_strip(src_1+0x60,dst_1+0x40,1);
		}
	}
	#undef copy_kanji_strip
}

void pc9801_state::init_pc9801vm_kanji()
{
	uint32_t raw_tile;
	uint8_t *chargen = memregion("chargen")->base();
	uint8_t *raw_kanji = memregion("raw_kanji")->base();
	uint8_t *kanji = memregion("kanji")->base();

	/* swap bits for 8x8 characters, discard 8x8 "graphics" characters */
	/* TODO: should we keep and use the "graphics" characters? */
	for( uint32_t i = 0; i < 0x100; i++ )
	{
		for( uint32_t j = 0; j < 8; j++ )
		{
			chargen[i*8+j] = bitswap<8>(chargen[i*0x10+j],0,1,2,3,4,5,6,7);
		}
	}
	/* swap bits for 8x16 characters */
	for( uint32_t i = 0; i < 0x100; i++ )
	{
		for( uint32_t j = 0; j < 0x10; j++ )
		{
			chargen[0x100*8+i*0x10+j] = bitswap<8>(chargen[0x100*0x10+i*0x10+j],0,1,2,3,4,5,6,7);
		}
	}
	/* 16x16 0x0020-0x077f */
	for( uint32_t hibyte = 0x00; hibyte <= 0x07; hibyte++ )
	{
		for( uint32_t lobyte = 0x20; lobyte <= 0x7f; lobyte++ )
		{
			raw_tile = bitswap<16>(hibyte*0x100+lobyte,15,14,13,12,11,7,6,5,10,9,8,4,3,2,1,0) * 0x20;
			for( uint32_t line = 0; line < 0x20; line++ )
			{
				kanji[(hibyte*0x100+lobyte)*0x20+line] = raw_kanji[raw_tile+line];
			}
		}
	}
	/* 16x16 0x0820-0x0f7f (swapped bits) */
	for( uint32_t hibyte = 0x08; hibyte <= 0x0f; hibyte++ )
	{
		for( uint32_t lobyte = 0x20; lobyte <= 0x7f; lobyte++ )
		{
			raw_tile = bitswap<16>((hibyte-0x08)*0x100+lobyte,15,14,13,12,11,7,6,5,10,9,8,4,3,2,1,0) * 0x20 + 0x2000 * 0x20;
			for( uint32_t line = 0; line < 0x20; line++ )
			{
				kanji[(hibyte*0x100+lobyte)*0x20+line] = bitswap<8>(raw_kanji[raw_tile+line],0,1,2,3,4,5,6,7);
			}
		}
	}
	/* 16x16 0x1020-0x4f7f */
	for( uint32_t hibyte = 0x10; hibyte <= 0x4f; hibyte++ )
	{
		for( uint32_t lobyte = 0x20; lobyte <= 0x7f; lobyte++ )
		{
			raw_tile = bitswap<16>((hibyte-0x10)*0x100+lobyte,15,14,7,13,6,5,12,11,10,9,8,4,3,2,1,0) * 0x20;
			for( uint32_t line = 0; line < 0x20; line++ )
			{
				kanji[(hibyte*0x100+lobyte)*0x20+line] = raw_kanji[raw_tile+line];
			}
		}
	}
	/* 16x16 0x5020-0x537f */
	for( uint32_t hibyte = 0x50; hibyte <= 0x53; hibyte++ )
	{
		for( uint32_t lobyte = 0x20; lobyte <= 0x7f; lobyte++ )
		{
			raw_tile = bitswap<16>((hibyte-0x50)*0x100+lobyte,15,14,13,12,11,7,6,5,10,9,8,4,3,2,1,0) * 0x20 + 0x1000 * 0x20;
			for( uint32_t line = 0; line < 0x20; line++ )
			{
				kanji[(hibyte*0x100+lobyte)*0x20+line] = raw_kanji[raw_tile+line];
			}
		}
	}
}

/* Genuine dumps */
COMP( 1983, pc9801f,    0,        0, pc9801,    pc9801,   pc9801_state, init_pc9801_kanji,   "NEC",   "PC-9801F",                      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
COMP( 1985, pc9801vm,   pc9801ux, 0, pc9801vm,  pc9801rs, pc9801_state, init_pc9801vm_kanji, "NEC",   "PC-9801VM",                     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

/* TODO: ANYTHING below there needs REDUMPING! */
COMP( 1989, pc9801rs,   0,        0, pc9801rs,  pc9801rs, pc9801_state, init_pc9801_kanji,   "NEC",   "PC-9801RS",                     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) //TODO: not sure about the exact model
COMP( 1985, pc9801vm11, pc9801ux, 0, pc9801vm,  pc9801rs, pc9801_state, init_pc9801_kanji,   "NEC",   "PC-9801VM11",                   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
COMP( 1987, pc9801ux,   0,        0, pc9801ux,  pc9801rs, pc9801_state, init_pc9801_kanji,   "NEC",   "PC-9801UX",                     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
COMP( 1988, pc9801rx,   pc9801rs, 0, pc9801rs,  pc9801rs, pc9801_state, init_pc9801_kanji,   "NEC",   "PC-9801RX",                     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
COMP( 1993, pc9801bx2,  pc9801rs, 0, pc9801bx2, pc9801rs, pc9801_state, init_pc9801_kanji,   "NEC",   "PC-9801BX2/U2",                 MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
COMP( 1994, pc9821,     0,        0, pc9821,    pc9821,   pc9801_state, init_pc9801_kanji,   "NEC",   "PC-9821 (98MATE)",              MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) //TODO: not sure about the exact model
COMP( 1993, pc9821as,   pc9821,   0, pc9821as,  pc9821,   pc9801_state, init_pc9801_kanji,   "NEC",   "PC-9821 (98MATE A)",            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
COMP( 1993, pc9821ap2,  pc9821,   0, pc9821ap2, pc9821,   pc9801_state, init_pc9801_kanji,   "NEC",   "PC-9821AP2/U8W (98MATE A)",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
COMP( 1994, pc9821xs,   pc9821,   0, pc9821,    pc9821,   pc9801_state, init_pc9801_kanji,   "NEC",   "PC-9821 (98MATE Xs)",           MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
COMP( 1994, pc9821ce2,  pc9821,   0, pc9821,    pc9821,   pc9801_state, init_pc9801_kanji,   "NEC",   "PC-9821 (98MULTi Ce2)",         MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
COMP( 1994, pc9821ne,   pc9821,   0, pc9821,    pc9821,   pc9801_state, init_pc9801_kanji,   "NEC",   "PC-9821 (98NOTE)",              MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
COMP( 1994, pc486mu,    pc9821,   0, pc9821,    pc9821,   pc9801_state, init_pc9801_kanji,   "Epson", "PC-486MU",                      MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1998, pc9821v13,  pc9821,   0, pc9821,    pc9821,   pc9801_state, init_pc9801_kanji,   "NEC",   "PC-9821 (98MATE VALUESTAR 13)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
COMP( 1998, pc9821v20,  pc9821,   0, pc9821v20, pc9821,   pc9801_state, init_pc9801_kanji,   "NEC",   "PC-9821 (98MATE VALUESTAR 20)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
