// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*******************************************************************************

                            -= Seta Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU    :    68000
Custom :    X1-001A, X1-002A (SDIP64)   Sprites
            X1-001
            X1-002
            X1-004           (SDIP52)   Inputs
            X1-005 or X1-009 (DIP48)    NVRAM/simple protection
            X1-006           (SDIP64)   Palette
            X1-007           (SDIP42)   Video blanking (feeds RGB DACs)
            X1-010           (QFP80)    Sound: 16 Bit PCM
            X1-011           (QFP80)    Graphics mixing
            X1-012           (QFP100)   Tilemaps
            X1-014                      Sprites?

-------------------------------------------------------------------------------
Ordered by Board        Year + Game                             Licensed To
-------------------------------------------------------------------------------
P0-047A                 89 The Roulette                         Visco
P0-053-1                89 Castle of Dragon/Dragon Unit         Taito / RomStar / Athena
P0-053A                 91 Strike Gunner S.T.G                  Athena / Tecmo
P0-053A                 92 Quiz Kokology                        Tecmo
P0-055B                 89 Wit's                                Athena
P0-055D                 90 Thunder & Lightning                  Romstar / Visco
Promat PCB              94 Wiggie Waggie(5)                     --
Promat PCB              94 Super Bar(5)                         --
P0-058A                 90 Jockey Club                          Visco
P0-058C                 98 International Toote (6)              Coinmaster (bootleg)
P0-063A                 91 Rezon                                Allumer
P0-068B  (M6100723A)    92 Block Carnival                       Visco
P0-072-2 (prototype)    92 Blandia (prototype)                  Allumer
P0-077A  (BP922)        92 Ultraman Club                        Banpresto
P0-078A                 92 Blandia (7)                          Allumer
P0-079A                 92 Zing Zing Zip                        Allumer / Tecmo
P0-079A                 94 Eight Forces                         Tecmo
P0-080A  (BP923)        92 SD Gundam Neo Battling (3)           Banpresto
?                       93 Athena no Hatena?                    Athena
?        (93111A)       93 J.J.Squawkers                        Athena / Able
?        (93111A)       93 War Of Aero                          Yang Cheng
P0-081A  (BP933KA)      93 Mobile Suit Gundam                   Banpresto
P0-083A  (BP931)        93 Ultra Toukon Densetsu                Banpresto / Tsuburaya Prod.
P0-092A                 93 Daioh                                Athena
P0-072-2 (prototype)    93 Daioh(prototype)                     Athena
?        (93111A)       93 Daioh(conversion)                    Athena
P0-096A  (BP934KA)      93 Kamen Rider                          Banpresto
P0-097A                 93 Oishii Puzzle ..                     Sunsoft + Atlus
bootleg                 9? Triple Fun (4)                       bootleg (Comad?)
P0-100A                 93 Quiz Kokology 2                      Tecmo
P0-102A                 93 Mad Shark                            Allumer
P0-107A  (prototype?)   94 Orbs (prototype?)                    American Sammy
P0-107A                 93 Kero Kero Keroppi no Isshoni Asobou  Sammy Industries  [added Chack'n, Hau]
P0-111A                 94 Magical Speed                        Allumer
P0-114A  (SKB-001)      94 Krazy Bowl                           American Sammy
P0-117A  (DH-01)        95 Extreme Downhill                     Sammy Japan
P0-117A?                95 Sokonuke Taisen Game                 Sammy Industries
P0-120A  (BP954KA)      95 Gundhara                             Banpresto
P0-122A  (SZR-001)      95 Zombie Raid                          American Sammy
?                       96 Crazy Fight                          Subsino
-------------------------------------------------------------------------------
(3) Same board as "Ultraman Club" despite the different PCB number
(4) this is a bootleg of Oishii Puzzle, in english, is there an official
    version?  the sound system has been replaced with an OKI M6295
    hardware is definitely bootleg. standard simple layout board with no
    custom chips and no manufacturer on the pcb.
(5) The game code is based on Thunder and Lightning but the PCB is custom
    there are a few gfx emulation bugs (flipping of some border tiles and
    sprites not leaving the screen correctly) its possible the custom hw
    doesn't behave *exactly* the same as the original seta hw
(6) To enter test mode press O (open door), then F2 (turn function key), then E (bet 3-4).
(7) Bad tilemaps colors in demo mode are real game bug. Fade-in and fade-out "bad" colors are also right.
    Bad sprites priorities are real game bugs. The bad-looking colors in Jurane stage are right.

*******************************************************************************

Notes:
- jjsquawk is modified from jjsquawko so nuts don't fall from the trees shaken by white animal.

DIP Locations verified from manuals for:
- Zing Zing Zip
- Extreme Downhill
- Kero Kero Keroppi's Let's Play Together

TODO:
- I think the best way to correctly align tilemaps and sprites and account for
  both flipping and different visible areas is to have a table with per game
  vertical and horizontal offsets for sprites, tilemaps and possibly the "floating
  tilemaps" (made of sprites) for both the flipped and normal screen cases.
  Current issues: metafox test grid not aligned when screen flipped, madshark & utoukond
  ("floating tilemaps" sprites when flipped)
  krzybowl not aligned vertically when screen flipped
  zombraid not aligned when flipped vertically or horizontally

- bad sound in sokonuke?
- in msgundam1, colors for the score display screw up after the second animation
  in attract mode. The end of the animation also has garbled sprites.
  Note that the animation is not present in msgundam.
- Some games: battery backed portion of RAM (e.g. zombraid)
- the zombraid crosshair hack can go if the nvram regions are figured out.
- Some games: programmable timer that generates IRQ. See e.g. gundhara:
  lev 4 is triggered by writes at d00000-6 and drives the sound.
  See also msgundam.

- drgnunit sprite/bg unaligned when screen flipped (check I/O test in service mode)
- extdwnhl has some wrong colored tiles in one of the attract mode images and in
  later tracks.
- oisipuzl doesn't support screen flip? tilemap flipping is also kludged in the video driver.
- eightfrc has alignment problems both flipped and not
- flip screen and mirror support not working correctly in zombraid
- gundhara visible area might be smaller (zombraid uses the same MachineDriver, and
  the current area is right for it)
- crazyfgt: emulate protection & tickets, fix graphics glitches, find correct clocks.
- jjsquawk: Player's shot sound is missing (not requested to X1-010?).
  Many sounds are wrong since MAME 0.62.
  i.e.
  all scene: when you beat enemies or yellow walking eggs
  stage 1: weasels throw eggs, white animals (shaking trees) are damaged, rabbit jump
  stage 2: when BOX-MEN gets angry
- games using 6bpp gfx switch tilemaps color mode. Only blandia uses both, while the other ones use only mode 1, thus mode 0 is untested for them

*******************************************************************************

Note:   if MAME_DEBUG is defined, pressing Z with:

        Q           shows layer 0
        W           shows layer 1
        A           shows the sprites

        Keys can be used together!


                        [ 0, 1 Or 2 Scrolling Layers ]

    Each layer consists of 2 tilemaps: only one can be displayed at a
    given time (the games usually flip continuously between the two).
    The two tilemaps share the same scrolling registers.

        Layer Size:             1024 x 512
        Tiles:                  16x16x4 (16x16x6 in some games)
        Tile Format:

            Offset + 0x0000:
                            f--- ---- ---- ----     Flip X
                            -e-- ---- ---- ----     Flip Y
                            --dc ba98 7654 3210     Code

            Offset + 0x1000:

                            fedc ba98 765- ----     -
                            ---- ---- ---4 3210     Color

            The other tilemap for this layer (always?) starts at
            Offset + 0x2000.


                            [ 1024 Sprites ]

    Sprites are 16x16x4. They are just like those in "The NewZealand Story",
    "Revenge of DOH" etc (tnzs.cpp). Obviously they're hooked to a 16 bit
    CPU here, so they're mapped a bit differently in memory. Additionally,
    there are two banks of sprites. The game can flip between the two to
    do double buffering, writing to a bit of a control register(see below)


        Spriteram16_2 + 0x000.w

                        f--- ---- ---- ----     Flip X
                        -e-- ---- ---- ----     Flip Y
                        --dc ba-- ---- ----     -
                        ---- --98 7654 3210     Code (Lower bits)

        Spriteram16_2 + 0x400.w

                        fedc b--- ---- ----     Color
                        ---- -a9- ---- ----     Code (Upper Bits)
                        ---- ---8 7654 3210     X

        Spriteram16   + 0x000.w

                        fedc ba98 ---- ----     -
                        ---- ---- 7654 3210     Y



                            [ Floating Tilemap ]

    There's a floating tilemap made of vertical columns composed of 2x16
    "sprites". Each 32 consecutive "sprites" define a column.

    For column I:

        Spriteram16_2 + 0x800 + 0x40 * I:

                        f--- ---- ---- ----     Flip X
                        -e-- ---- ---- ----     Flip Y
                        --dc b--- ---- ----     -
                        ---- --98 7654 3210     Code (Lower bits)

        Spriteram16_2 + 0xc00 + 0x40 * I:

                        fedc b--- ---- ----     Color
                        ---- -a9- ---- ----     Code (Upper Bits)
                        ---- ---8 7654 3210     -

    Each column has a variable horizontal position and a vertical scrolling
    value (see also the Sprite Control Registers). For column I:


        Spriteram16   + 0x400 + 0x20 * I:

                        fedc ba98 ---- ----     -
                        ---- ---- 7654 3210     Y

        Spriteram16   + 0x408 + 0x20 * I:

                        fedc ba98 ---- ----     -
                        ---- ---- 7654 3210     Low Bits Of X



                        [ Sprites Control Registers ]


        Spriteram16   + 0x601.b

                        7--- ----       0
                        -6-- ----       Flip Screen
                        --5- ----       0
                        ---4 ----       1 (Sprite Enable?)
                        ---- 3210       ???

        Spriteram16   + 0x603.b

                        7--- ----       0
                        -6-- ----       Sprite Bank
                        --5- ----       0 = Sprite Buffering (blandia,msgundam,qzkklogy)
                        ---4 ----       0
                        ---- 3210       Columns To Draw (1 is the special value for 16)

        Spriteram16   + 0x605.b

                        7654 3210       High Bit Of X For Columns 7-0

        Spriteram16   + 0x607.b

                        7654 3210       High Bit Of X For Columns f-8

*******************************************************************************

Dragon Unit [Prototype of "Castle Of Dragon"]

PCB:    P0-053-1
CPU:    68000-8
Sound:  X1-010
OSC:    16.0000MHz

Chips:  X1-001A, X1-002A, X1-004, X1-006, X1-007, X1-010, X1-011, X1-012

*******************************************************************************

Wit's

(c)1989 Athena (distributed by Visco)
P0-055B (board is made by Seta)

CPU  : TMP68000N-8
Sound: X1-010
OSC  : 16.000MHz

ROMs:
UN001001.U1 - Main program (27256)
UN001002.U4 - Main program (27256)

UN001003.10A - Samples (28pin mask)
UN001004.12A /

UN001005.2L - Graphics (28pin mask)
UN001006.4L |
UN001007.5L |
UN001008.7L /

Custom chips:   X1-001A     X1-002A
                X1-004 (x2)
                X1-006
                X1-007
                X1-010

*******************************************************************************

Thunder & Lightning

Location      Device      File ID      Checksum
-----------------------------------------------
U1  1A        27C256        M4           C18C   [ MAIN PROG ] [ EVEN ]
U4  3A        27C256        M5           12E1   [ MAIN PROG ] [ ODD  ]
U29 10A      23C4001        R27          37F2   [   HIGH    ]
U39 12A      23C4001        R28          0070   [   LOW     ]
U6  2K       23C1000        T14          1F7D   [   C40     ]
U9  4K       23C1000        T15          7A15   [   C30     ]
U14 5K       23C1000        T16          BFFD   [   C20     ]
U20 7K       23C1000        T17          7AE7   [   C10     ]

PCB: PO055D

CPU: 68000 8MHz

Custom: X1-001A     X1-002A
        X1-004
        X1-006
        X1-007
        X1-010

*******************************************************************************

Athena no Hatena?

CPU  : 68000-16
Sound: X1-010
OSC  : 16.0000MHz

ROMs:
fs001001.evn - Main programs (27c4001)
fs001002.odd /

fs001004.pcm - Samples (8M mask - read as 27c800)
fs001003.gfx - Graphics (16M mask - read as 27c160)

Chips:  X1-001A X1-002A
        X1-004
        X1-006
        X1-007
        X1-010

*******************************************************************************

Blandia by Allumer

This set is coming from an original Blandia PCB ref : P0-078A

As usually, it use a lot of customs allumer chips !

*******************************************************************************

Blandia (prototype)

PCB:    P0-072-2
CPU:    68000-16
Sound:  X1-010
OSC:    16.0000MHz

Chips:  X1-001A     X1-002A
        X1-004
        X1-007
        X1-010
        X1-011 x2   X1-012 x2

*******************************************************************************

Block Carnival / Thunder & Lightning 2

P0-068B, M6100723A

CPU  : MC68000B8
Sound: X1-010
OSC  : 16.000MHz

ROMs:
u1.a1 - Main programs (27c010)
u4.a3 /

bl-chr-0.j3 - Graphics (4M mask)
bl-chr-1.l3 /

bl-snd-0.a13 - Sound (8M mask)

Custom chips:   X1-001A X1-002A
                X1-004
                X1-006
                X1-007
                X1-009
                X1-010

Other:
Lithium battery x1

*******************************************************************************

Daioh

DAIOH
Allumer 1993, Sammy license
P0-092A


FG-001-003
FG-001-004  X1-002A X1-001A             FG-001-001
                                        FG-001-002
FG-001-005   X1-11 X1-12
FG-001-006   X1-11 X1-12
                                       68000-16
FG-001-007

   X1-10                           16MHz

                            X1-007  X1-004

*******************************************************************************

Eight Forces

P0-079A (Same board as ZingZingZip)

CPU  : MC68000B16
Sound: X1-010
OSC  : 16.000MHz

ROMs:
uy2-u4.u3 - Main program (even)(27c2001)
uy2-u3.u4 - Main program (odd) (27c2001)

u63.bin - Sprites (HN62434, read as 27c4200)
u64.bin /

u69.bin - Samples (HN62318, read as 27c8001)
u70.bin /

u66.bin - Layer 1 (HN62418, read as 27c800)
u68.bin - Layer 2 (HN62418, read as 27c800)

PALs (not dumped):
uy-012.206 (PAL16L8A)
uy-013.14  (PAL16L8A)
uy-014.35  (PAL16L8A)
uy-015.36  (PALCE16V8)
uy-016.76  (PAL16L8A)
uy-017.116 (PAL16L8A)

Custom:     X1-001A X1-002A
            X1-004
            X1-007
            X1-010
            X1-011 (x2)     X1-012 (x2)

*******************************************************************************

Extreme Downhill

(c)1995 Sammy
DH-01
P0-117A (board is made by Seta/Allumer)

CPU  : MC68HC000B16
Sound: X1-010
OSC: 16.0000MHz (X1), 14.3180MHz (X2)

ROMs:
fw001002.201 - Main program (even) (Macronics 27c4000)
fw001001.200 - Main program (odd)  (Macronics 27c4000)

fw001005.205 - (32pin mask, read as 27c8001)
fw001007.026 /

fw001003.202 - (42pin mask, read as 27c160)
fw001004.206 |
fw001006.152 /

PALs (16L8ACN, not dumped):
FW-001
FW-002
FW-003
FW-005

Custom chips:   X1-001A     X1-002A
                X1-004
                X1-007
                X1-010
                X1-011 (x2) X1-012 (x2)

*******************************************************************************

GundHara

(C) 1995 Banpresto
Seta/Allumer Hardware

PCB: BP954KA
PCB: P0-120A
CPU: TMP68HC000N16 (68000, 64 pin DIP)
SND: ?
OSC: 16.000MHz
RAM: 6264 x 8, 62256 x 4
DIPS: 2 x 8 position
Other Chips:    PALs x 6 (not dumped)
                NEC 71054C
                X1-004
                X1-007
                X1-010
                X1-011 x2   X1-012 x2
                X1-001A     X1-002A

On PCB near JAMMA connector is a small push button to reset the PCB.

ROMS:
BPGH-001.102    27C040
BPGH-002.103    27C4000
BPGH-003.U3     27C4000
BPGH-004.U4     23C4000
BPGH-005.200    23C16000
BPGH-006.201    23C16000
BPGH-007.U63    23C16000
BPGH-008.U64    23C16000
BPGH-009.U65    27C4000
BPGH-010.U66    TC538200
BPGH-011.U67    TC538000
BPGH-012.U68    TC5316200
BPGH-013.U70    TC538000

*******************************************************************************

Zombie Raid
Sammy, 1996
Hardware info by Guru

This is a gun shooting game using Seta/Allumer hardware.

PCB Layout
----------

SZR-001
P0-122A
------------------------------------------------------------------
        FY001012.12L*       FY001009.U67      FY001007.U65
   FY001011.13L*    FY001010.U68     FY001008.U66    FY001006.U200
   X1-010    6264

    CONN1          X1-011(x2)     X1-002A
       ADC0834     X1-012(x2)     X1-001A

                                            3V_BATT
                                                    4464

X1-007   6264(x2)   6264(x2)    6264(x2)    6264(x2)

       16.000MHz                                 HM9253101(x2)

X1-004 DSW2        D71054C
       DSW1    TMP68HC000N-16  FY001004.U4     FY001001.U102
                                      FY001003.U3    FY001002.U103
------------------------------------------------------------------

Notes:
*     = These ROMs located on a small daughterboard. Main PCB locations used as filename extension.
CONN1 = 8 pin header for gun connection

*******************************************************************************

J.J. Squawkers

68HC000N -16N

2)   Allumer  X1-012
2)   Allumer  X1-011
2)   Allumer  X1-014

X1-010
X1-007
X1-004
16.000MHz

NEC 71054C  ----???

*******************************************************************************

Kamen Rider

Kamen Riderclub Battleracer
Banpresto, 1993
Hardware info by Guru

Runs on Seta/Allumer hardware

PCB No: BP934KA   P0-096A
CPU   : MC68HC000B16
OSC   : 16.000MHz
RAM   : LH5160D-10L (x9), CXK58257AP-10L (x2)
DIPSW : 8 position (x2)
CUSTOM: X1-010
        X1-007
        X1-004
        X1-011 (x2)
        X1-012 (x2)
        X1-002A
        X1-001A
OTHER : NEC71054C, some PALs

ROMs  :
        FJ001007.152    27c4096     near X1-011 & X1-010 (sound program?)
        FJ001008.26     8M Mask     connected to X1-010, near FJ001007
        FJ001003.25     27c4096     main program for 68k
        FJ001006.22     16M Mask    gfx
        FJ001005.21     16M Mask    gfx

*******************************************************************************

Krazy Bowl

PCB:    SKB-001
        P0-114A

FV   FV                           2465
001  001                          2465           X1-005
004  003      X1-002A  X1-001A
                                       58257     FV
                                                 001
                                                 002 (even)
                                       58257
                  14.318MHz                      FV
                                                 001
FV 001 005                                       001 (odd)
FV 001 006
  2465                                      68HC000B16
                 NEC4701  NEC4701

X1-010           X1-006
                 X1-007      X1-004

*******************************************************************************

Mad Shark

Allumer, 1993
This game is a vertical shoot'em-up and runs on fairly standard Allumer hardware.
Hardware info by Guru

PCB Layout
----------

P0-102A
----------------------------------------------------
|     X1-010   FQ001007 FQ001006 FQ001005 FQ001004 |
|           LH5160                                 |
|                    X1-011  X1-011       X1-002A  |
|                                                  |
|J                   X1-012  X1-012       X1-001A  |
|A X1-007                                          |
|M   LH5160           LH5160 LH5160 LH5160         |
|M                                                 |
|A   LH5160           LH5160 LH5160 LH5160         |
|                                         FQ001002 |
|*           MC68HC000B16                          |
|  X1-004                                          |
|                                         FQ001001 |
|                        LH52250                   |
| DSW2(8) DSW1(8) 16MHz  LH52250    D71054         |
----------------------------------------------------

Notes:
      *: 4 jumper pads for region selection (hardwired)

*******************************************************************************

Magical Speed

(c)1994 Allumer

PCB P0-111A:
+--------------------------------------------------+
| VR1 X1-010   FU001007 FU001006 FU001005 FU001004 |
|           W2465K                                 |
|  CN3               X1-011  X1-011       X1-002A  |
|  CN1                                             |
|J CN2               X1-012  X1-012       X1-001A  |
|A X1-007                                          |
|M   W2465K U54 U50   W2465K W2465K W2465K U53     |
|M                                                 |
|A   W2465K     U51   W2465K W2465K W2465K         |
|                                         FU001002 |
|*           MC68HC000B16                          |
|  X1-004                                          |
|                                         FU001001 |
|                          CXK58257                |
| SW1 RST1 DSW2 DSW1 16MHz CXK58257 D71054 U52     |
+--------------------------------------------------+

CPU   : MC68HC000B16
OSC   : 16.000MHz
RAM   : WinBond W2465K-70LL (x9), SONY CXK58257AP-10L (x2)
DIPSW : 8 position (x2)
CUSTOM: X1-010       Sound
        X1-004       Input
        X1-007       Video DAC
        X1-011 (x2)  Tilemap
        X1-012 (x2)  Tilemap
        X1-002A      Sprites
        X1-001A      Sprites

OTHER : NEC71054C

VR1   : Sound adjust pot
SW1   : Service switch
RST1  : Reset

CN1   : 7-Pin header to drive lights underneath buttons to show what cards are available to play
CN2   : 8-Pin header to drive lights underneath buttons to show what cards are available to play
CN3   : 5-Pin header connected to auxiliary PCB to drive lights about the cabinet

PAL   :FU-011 @ U50
       FU-012 @ U51
       FU-013 @ U52
       FU-014 @ u53
       FU-015 @ U54

*******************************************************************************

Mobile Suit Gundam

Banpresto 1993
P0-081A
                               SW2  SW1

FA-001-008                          FA-001-001
FA-001-007    X1-002A X1-001A       FA-002-002
                              5160
                              5160
                                        71054
FA-001-006                    5160     62256
FA-001-005    X1-011  X1-012  5160     62256

FA-001-004    X1-011  X1-012  5160
5160                          5160

                                68000-16

                                         16MHz
  X1-010
                    X1-007   X1-004     X1-005

*******************************************************************************

Oishii Puzzle Ha Irimasenka

PCB  : P0-097A
CPU  : 68000
Sound: X1-010
OSC  : 14.31818MHz

All ROMs are 23c4000

Custom chips:   X1-001A X1-002A
                X1-004
                X1-007
                X1-010
                X1-011 (x2) X1-012 (x2)

*******************************************************************************

Triple Fun

Triple Fun
??, 19??
Hardware info by Guru


CPU   : TMP68HC000P-16 (68000)
SOUND : OKI M6295
DIPSW : 8 position (x2)
XTAL  : 16.000 MHz (8MHz written on PCB, located near OKI chip)
        14.31818MHz (near 68000)
RAM   : 62256 (x2), 6264 (x8), 2018 (x14)
PROMs : None
PALs  : PALCE16V8H (x13)
OTHER : TPC1020AFN-084C (84 pin PLCC)

ROMs  :

04.bin + 05.bin    Main Program
01.bin             Sound Program
02.bin + 03.bin    OKI Samples
06.bin to 11.bin   GFX

*******************************************************************************

Quiz Kokology

(c)1992 Tecmo

P0-053A

CPU  : MC68000B8
Sound: X1-010
OSC  : 16.000MHz

Custom chips:   X1-001A X1-002A
                X1-004
                X1-006  X1-007
                X1-010
                X1-011  X1-012

*******************************************************************************

Quiz Koko-logy 2

(c)1992 Tecmo

P0-100A

CPU  : MC68HC000B16
Sound: X1-010
OSC  : 16.000MHz

FN001001.106 - Main program (27C4096)
FN001003.107 / (40pin 2M mask)

FN001004.100 - OBJ chr. (42pin mask)
FN001005.104 - BG chr. (42pin mask)
FN001006.105 - Samples (32pin mask)

Custom chips:   X1-001A     X1-002A
                X1-004
                X1-006
                X1-007
                X1-010
                X1-011      X1-012

*******************************************************************************

Rezon (Japan)

PCB     : P0-063A
CPU     : TOSHIBA TMP68HC000N-16
Sound   : X1-010
OSC     : 16.000MHz
Other   : Allumer
            X1-001A         X1-002A
            X1-004
            X1-007
            X1-011 x 2      X1-012 x 2

*******************************************************************************

SD Gundam Neo Battling

Banpresto, 1992
This game runs on Seta/Allumer hardware
Hardware info by Guru

PCB Layout
----------

P0-080A
BP923
|----------------------------------------------
|DSW1  DSW2                   LH5168  62256   |
|                             LH5168  62256   |
|LH5168                                       |
|                                             |
|         BP923004                   BP923001 |
|BP923005 BP923003 X1-002A X1-001A   BP923002 |
|                                  16MHz      |
|                              TMP68HC000N-16 |
|X1-010                           PAL         |
|                                 PAL  X1-006 |
|                                             |
|                                             |
|                              X1-004  X1-007 |
|                                             |
|                                             |
|                                             |
|                                             |
|                 J A M M A                   |
-----------------------------------------------

Notes:
      68k clock: 16.000MHz
      VSync: 58Hz
      HSync: 15.22kHz

*******************************************************************************

Sokonuke Taisen Game (Japan)

(c)1995 Sammy

CPU:    68HC000
Sound:  All PCM ?
OSC:    16MHz

*******************************************************************************

Strike Gunner

(c)1991 Athena (distributed by Tecmo)

P0-053A

CPU  : TMP68000N-8
Sound: X1-010
OSC  : 16.000MHz

Custom chips:   X1-001A X1-002A
                X1-004
                X1-006  X1-007
                X1-010
                X1-011  X1-012

*******************************************************************************

Ultraman Club

Banpresto, 1992
Board looks similar to Castle of Dragon PCB.
Hardware info by Guru

PCB No: P0-077A (Seta Number)
        BP922   (Banpresto Number)

CPU: MC68HC000B16
OSC: 16.000MHz
DIP SW x 2 (8 position)

RAM: Sharp LH5160D-10L x 3, Hitachi S256KLP-12 x 2
PALs (2 x PAL16L8, not dumped)
SETA Chips: X1-010
            X1-004
            X1-007
            X1-006
            X1-002A
            X1-001A

Controls are 8 way Joystick and 2 buttons.

ROMs:

UW001006.U48      27C010                                               \  Main Program
UW001007.U49      27C010                                               /

BP-U-001.U1       4M mask (40 pin, 512k x 8), read as MX27C4100        \  GFX
BP-U-002.U2       4M mask (40 pin, 512k x 8), read as MX27C4100        /

BP-U-003.U13      8M mask (32 pin, 1M x 8),   read as MX27C8000           Sound

*******************************************************************************

Ultra Toukon Densetsu
Banpresto, 1993
Hardware info by Guru

This game runs on fairly standard Allumer hardware.

PCB Layout
----------

P0-083A
BP931
----------------------------------------------------
|     X1-010  93UTA08  93UTA06 93UTA04  93UTA02    |
|                93UTA07 93UTA05  93UTA03  93UTA01 |
|  YM3438   LH5116                                 |
|  LH5116            X1-011  X1-011       X1-002A  |
|  Z80 93UTA009                                    |
|J                   X1-012  X1-012       X1-001A  |
|A X1-007                                          |
|M   LH5116           LH5160 LH5160 LH5160         |
|M                                                 |
|A   LH5116           LH5160 LH5160 LH5160         |
|                                                  |
|*      16MHz                                      |
|  X1-004                                   62256  |
|                                           62256  |
| DSW1(8)               93UTA011  93UTA010         |
| DSW2(8)   68HC000N-16                            |
----------------------------------------------------

Notes:
      *: 4 jumper pads for region selection (hardwired)
      Z80 clock = 4.000MHz
      VSync: 60Hz
      HSync: 15.21kHz


1.048.576 93uta03.63
1.048.576 93uta04.64
1.048.576 93uta05.66
1.048.576 93uta06.67
1.048.576 93uta07.68
1.048.576 93uta08.69

*******************************************************************************

War of Aero
Project M E I O U

93111A  YANG CHENG

CPU   : TOSHIBA TMP68HC000N-16
Sound : Allumer X1-010
OSC   : 16.000000MHz
Other : Allumer
            X1-001A  X1-002A
            X1-004
            X1-007
            X1-011 x 2
            X1-012 x 2
        NEC
            C324C
            D71054C

*******************************************************************************

Zing Zing Zip

P0-079A

UY-001-005   X1-002A   X1-001A   5168-10      256k-12
UY-001-006                       5168-10      UY-001-001
UY-001-007                                    UY-001-002
UY-001-008   X1-011 X1-012                    58257-12
                                 5168-10
UY-001-010   X1-011 X1-012       5168-10
UY-001-017
UY-001-018
                                 5168-10
X1-010                           5168-10       68000-16


                           8464-80
                           8464-80       16MHz


                             X1-007    X1-004

*******************************************************************************

Pairs Love
Allumer, 199x
Hardware info by Guru

PCB Layout
----------

P0-068B
|-----------------------------------------|
|             X1-007  X1-006   UT2-001-005|
|                                         |
|     4050                     UT2-001-004|
|                                         |
|                                         |
|                             X1-002A     |
|                                         |
|J                                        |
|A   X1-004                               |
|M                            X1-001A     |
|M           DSW1                         |
|A                                        |
|            DSW2                  6264   |
|                                  6264   |
|    X1-009                               |
|                      62256              |
|                                         |
|                    68000                |
|    UT2-001-003       62256  UT2-001-002 |
|                   6264                  |
|    X1-010  16MHz            UT2-001-001 |
|-----------------------------------------|
Notes:
      68000 clock: 8.000MHz
      VSync: 60Hz

*******************************************************************************

Rezon (Taito License)
Allumer / Taito, 1992
Hardware info by Guru

This game runs on fairly standard Allumer hardware.

PCB Layout
----------

P0-063A (Allumer code printed on the PCB)
M6100627A REZON (Taito sticker)
|-----------------------------------------------------------|
|  VOL   3404   6264 US001009     US001007      US001005    |
| MB3730                    US001008      US001006          |
|                                                           |
|                                                           |
|                                                           |
|  PAL4           X1-010       X1-011       X1-011          |
|                                                           |
|                                                           |
|J                                                          |
|A                             X1-012       X1-012          |
|M                                                          |
|M  X1-007                                                  |
|A                                                          |
|        6116  6116                           PAL2   PAL3   |
|                                                           |
|                         62256       62256                 |
|  16MHZ                  62256       62256     6264  6264  |
|         DSW2(8)                                           |
|  X1-004                 PAL1                              |
|         DSW1(8)  |------------|US001004      REZON_1_P    |
|                  |   68000    |   US001003      REZON_0_P |
|  RESET_SW        |------------|       62256          62256|
|-----------------------------------------------------------|
Notes:
      68000 clock  - 16.000MHz
      X1-010 clocks - pin1 16.000MHz, pin2 8.000MHz, pin79 4.000MHz, pin80 2.000MHz
      VSync - 57.5Hz
      PAL1  - PAL16L8 labelled 'US-010'
      PAL2  - PAL16L8 labelled 'US-011'
      PAL3  - PAL16L8 labelled 'US-012'
      PAL4  - PAL16L8 labelled 'US-013'
      62256 - 32K x8 SRAM
      6264  - 8K x8 SRAM
      6116  - 2K x8 SRAM

      Custom IC's -
                    X1-001A (SDIP64)    \ Sprite Generators
                    X1-002A (SDIP64)    /
                    X1-004  (SDIP52)      Input Related Functions (connected to joystick/input controls)
                    X1-007  (SDIP42)      Video DAC? (connected to RGB output)
                    X1-010  (QFP80)       Sound Chip, 16Bit PCM
                    X1-011  (x2, QFP100)\ Tilemap Generators
                    X1-012  (x2, QFP100)/

      ROMs -
            Filename         Type               Use
            ---------------------------------------------------
            REZON_0_P.U3     27C1000 (DIP32)    \
            REZON_1_P.U4     27C1000 (DIP32)    | 68000 Program
            US001003.U102    27C1000 (DIP32)    |
            US001004.U103    27C1000 (DIP32)    /

            US001005.U63     4M MaskROM (DIP42) \ Sprites
            US001006.U64     4M MaskROM (DIP42) /

            US001007.U66     4M MaskROM (DIP42) \ Tiles
            US001008.U68     4M MaskROM (DIP42) /

            US001009.U70     4M MaskROM (DIP32)   PCM Samples

*******************************************************************************

Crazy Fight
Subsino 1996
Hardware info by Guru
This game runs on Allumer-based hardware.
It is a whack-a-mole type game using 6 buttons.

PCB Layout
----------

186P010
|--------------------------------------------------------------------|
|    VOLUME               68000              ROM_U68                 |
|TDA1519  LM358 LM324                                                |
|              YM3014     ROM_U3             ROM_U67                 |
|           YM3812                                                   |
|4.43361875MHz            62256             X1-011  X1-012           |
|                                 PLSI1032                           |
|        M6295            ROM_U4                                     |
|                                            ROM_U66                 |
|                         62256                                      |
|   ROM_U85                                  ROM_U65                 |
|J                                16MHz                              |
|A                                          X1-011  X1-012           |
|M                                                                   |
|M  X1-007        PAL                                                |
|A                                           6164       X1-001A      |
|    6164                                                            |
|                                                                    |
|    6164                                    6164       X1-002A      |
|                DSW2(8)  DSW1(8)                                    |
| TD62003                            PAL   ROM_U228     ROM_U226     |
|    J2    DIP42                     PAL         ROM_U227    ROM_U225|
|--------------------------------------------------------------------|
Notes:
      68000 clock  - 16.00MHz (DIP64)
      YM3812 clock - 4.000MHz [16/4] (DIP24)
      M6295 clock  - 1.108404688 [4.43361875/4]. Pin 7 HIGH (QFP44)
      PLSI1032     - Lattice pLSI1032-80LJ Programmable Logic Device (PLCC84)
      6164         - 8kx8 SRAM (SDIP28)
      62256        - 32kx8 SRAM (SDIP28)
      TD62003      - Toshiba TD62003 7-Channel Darlington Sink Driver (DIP16)
      DIP42        - Oki ULA. Note several pins have no connection.
                     Pins 6-26 tied to inputs on JAMMA connector.
                     Some other pins tied to logic.
      J2           - 4 pin connector. Pin 2 tied to DIP42 IC pin 27
                     and pin 3 tied to TD62003 pin 16
      Custom Chips - X1-007
                     X1-001A
                     X1-002A
                     X1-011 (2)
                     X1-012 (2)
      HSync        - 15.1433kHz
      VSync        - 59.1851Hz

*******************************************************************************

International Toote

Main PCB (P0-058C):

    TOSHIBA TMP68HC000N-16
    X1-010
    X1-001A
    X1-002A
    X1-004        (x2)
    X1-007
    X1-011
    X1-012
    HD63B50P      (ACIA)

Horse Race I/O Expansion (PCB-HRE-000):

    MAX238CNG     (RS-232)
    DIP24         (glue on markings?)
    MC68B50CP     (x2, ACIA)
    EF68B21P      (x2, PIA)
    ULN2803A      (x2, Darlington Transistor Array)
    4116R-001-151 (x2, Resistor Network)
    2.45760 MHz Osc.

Note: on screen copyright is (c)1998 Coinmaster.
      The I/O board has      (c)1993 Coinmaster.

*******************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/adc083x.h"
#include "machine/ds2430a.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/upd4701.h"
#include "machine/upd4992.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"
#include "sound/x1_010.h"
#include "sound/ymopm.h"
#include "sound/ymopn.h"
#include "sound/ymopl.h"
#include "video/x1_001.h"

#include "x1_012.h"

#include "diserial.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "inttoote.lh"
#include "jockeyc.lh"
#include "setaroul.lh"

#include <algorithm>


namespace {

class seta_state : public driver_device
{
public:
	seta_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_watchdog(*this, "watchdog"),
		m_screen(*this, "screen"),
		m_spritegen(*this, "spritegen"),
		m_layers(*this, "layer%u", 1U),
		m_x1snd(*this, "x1snd"),
		m_soundlatch(*this, "soundlatch"),
		m_oki(*this, "oki"),
		m_dsw(*this, "DSW"),
		m_paletteram(*this, "paletteram%u", 1U),
		m_x1_bank(*this, "x1_bank"),
		m_oki_bank(*this, "oki_bank"),
		m_palette(*this, "palette")
	{ }

	void madshark(machine_config &config);
	void madsharkbl(machine_config &config);
	void jjsquawb(machine_config &config);
	void oisipuzl(machine_config &config);
	void zingzipbl(machine_config &config);
	void eightfrc(machine_config &config);
	void gundhara(machine_config &config);
	void triplfun(machine_config &config);
	void blandiap(machine_config &config);
	void wits(machine_config &config);
	void msgundam(machine_config &config);
	void msgundamb(machine_config &config);
	void extdwnhl(machine_config &config);
	void zingzip(machine_config &config);
	void wiggie(machine_config &config);
	void umanclub(machine_config &config);
	void daioh(machine_config &config);
	void atehate(machine_config &config);
	void blockcarb(machine_config &config);
	void wrofaero(machine_config &config);
	void blockcar(machine_config &config);
	void drgnunit(machine_config &config);
	void stg(machine_config &config);
	void qzkklogy(machine_config &config);
	void orbs(machine_config &config);
	void daiohp(machine_config &config);
	void krzybowl(machine_config &config);
	void qzkklgy2(machine_config &config);
	void kamenrid(machine_config &config);
	void superbar(machine_config &config);
	void jjsquawk(machine_config &config);
	void blandia(machine_config &config);
	void utoukond(machine_config &config);
	void rezon(machine_config &config);

	void init_wiggie();
	void init_bankx1();
	void init_madsharkbl();

	void palette_init_RRRRRGGGGGBBBBB_proms(palette_device &palette) const;

	X1_001_SPRITE_GFXBANK_CB_MEMBER(setac_gfxbank_callback);

	u32 screen_update_seta_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	void set_tilemaps_flip(int val) { m_tilemaps_flip = val; }

	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<watchdog_timer_device> m_watchdog;
	required_device<screen_device> m_screen;
	required_device<x1_001_device> m_spritegen;
	optional_device_array<x1_012_device, 2> m_layers;
	optional_device<x1_010_device> m_x1snd;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<okim6295_device> m_oki;

	optional_ioport m_dsw;

	optional_shared_ptr_array<u16, 2> m_paletteram;

	optional_memory_bank m_x1_bank;
	optional_memory_bank m_oki_bank;

	required_device<palette_device> m_palette;

	u8 m_vregs = 0;

	int m_tilemaps_flip = 0;
	int m_samples_bank = 0;

	void seta_coin_counter_w(u8 data);
	void seta_coin_lockout_w(u8 data);
	void seta_vregs_w(u8 data);
	u16 seta_dsw_r(offs_t offset);

	void blockcar_interrupt_w(u8 data);
	u16 extdwnhl_watchdog_r();
	void utoukond_sound_control_w(u8 data);

	void blandia_palette(palette_device &palette) const;
	void zingzip_palette(palette_device &palette) const;
	void gundhara_palette(palette_device &palette) const;
	void jjsquawk_palette(palette_device &palette) const;
	u32 screen_update_seta_no_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_seta(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void screen_vblank_seta_buffer_sprites(int state);
	u16 ipl0_ack_r();
	void ipl0_ack_w(u16 data = 0);
	u16 ipl1_ack_r();
	void ipl1_ack_w(u16 data = 0);
	void ipl2_ack_w(u16 data = 0);
	TIMER_DEVICE_CALLBACK_MEMBER(seta_interrupt_1_and_2);
	TIMER_DEVICE_CALLBACK_MEMBER(seta_interrupt_2_and_4);

	void set_pens();
	void seta_layers_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_bank_size);
	void pit_out0(int state);

	void atehate_map(address_map &map) ATTR_COLD;
	void blandia_map(address_map &map) ATTR_COLD;
	void blandia_x1_map(address_map &map) ATTR_COLD;
	void blandiap_map(address_map &map) ATTR_COLD;
	void blockcar_map(address_map &map) ATTR_COLD;
	void blockcarb_map(address_map &map) ATTR_COLD;
	void blockcarb_sound_map(address_map &map) ATTR_COLD;
	void daioh_map(address_map &map) ATTR_COLD;
	void daiohp_map(address_map &map) ATTR_COLD;
	void drgnunit_map(address_map &map) ATTR_COLD;
	void extdwnhl_map(address_map &map) ATTR_COLD;
	void jjsquawb_map(address_map &map) ATTR_COLD;
	void kamenrid_map(address_map &map) ATTR_COLD;
	void krzybowl_map(address_map &map) ATTR_COLD;
	void madshark_map(address_map &map) ATTR_COLD;
	void madsharkbl_map(address_map &map) ATTR_COLD;
	void madsharkbl_oki_map(address_map &map) ATTR_COLD;
	void msgundam_map(address_map &map) ATTR_COLD;
	void msgundamb_map(address_map &map) ATTR_COLD;
	void oisipuzl_map(address_map &map) ATTR_COLD;
	void orbs_map(address_map &map) ATTR_COLD;
	void rezon_map(address_map &map) ATTR_COLD;
	void triplfun_map(address_map &map) ATTR_COLD;
	void umanclub_map(address_map &map) ATTR_COLD;
	void utoukond_map(address_map &map) ATTR_COLD;
	void utoukond_sound_io_map(address_map &map) ATTR_COLD;
	void utoukond_sound_map(address_map &map) ATTR_COLD;
	void wiggie_map(address_map &map) ATTR_COLD;
	void wiggie_sound_map(address_map &map) ATTR_COLD;
	void wits_map(address_map &map) ATTR_COLD;
	void wrofaero_map(address_map &map) ATTR_COLD;
	void zingzip_map(address_map &map) ATTR_COLD;
	void zingzipbl_map(address_map &map) ATTR_COLD;
};

class thunderl_state : public seta_state
{
public:
	thunderl_state(const machine_config &mconfig, device_type type, const char *tag) :
		seta_state(mconfig, type, tag)
	{ }

	void thunderl(machine_config &config);
	void thunderlbl(machine_config &config);

protected:
	u16 thunderl_protection_r();
	void thunderl_protection_w(offs_t offset, u16 data);

	virtual void machine_start() override ATTR_COLD;

	void thunderl_map(address_map &map) ATTR_COLD;
	void thunderlbl_map(address_map &map) ATTR_COLD;
	void thunderlbl_sound_map(address_map &map) ATTR_COLD;
	void thunderlbl_sound_portmap(address_map &map) ATTR_COLD;

private:
	u8 m_thunderl_protection_reg = 0;
};

class magspeed_state : public seta_state
{
public:
	magspeed_state(const machine_config &mconfig, device_type type, const char *tag) :
		seta_state(mconfig, type, tag),
		m_leds(*this, "led%u", 0U)
	{ }

	void magspeed(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void lights_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void magspeed_map(address_map &map) ATTR_COLD;

	output_finder<48> m_leds;

	u16 m_lights[3] = { };
};

class keroppi_state : public seta_state
{
public:
	keroppi_state(const machine_config &mconfig, device_type type, const char *tag) :
		seta_state(mconfig, type, tag),
		m_coins(*this, "COINS")
	{ }

	void keroppi(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u16 protection_r();
	u16 protection_init_r();
	u16 coin_r();
	void prize_w(u16 data);
	TIMER_CALLBACK_MEMBER(prize_hop_callback);

	void keroppi_map(address_map &map) ATTR_COLD;

	required_ioport m_coins;

	emu_timer *m_prize_hop_timer = nullptr;

	int m_prize_hop = 0;
	int m_protection_count = 0;
};

class zombraid_state : public seta_state
{
public:
	zombraid_state(const machine_config &mconfig, device_type type, const char *tag) :
		seta_state(mconfig, type, tag),
		m_adc(*this, "adc"),
		m_gun_inputs(*this, {"GUNX1", "GUNY1", "GUNX2", "GUNY2"}),
		m_gun_recoil(*this, "Player%u_Gun_Recoil", 1U)
	{ }

	void zombraid(machine_config &config);
	void init_zombraid();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	double adc_cb(u8 input);
	u16 gun_r();
	void gun_w(u16 data);

	void zombraid_map(address_map &map) ATTR_COLD;
	void zombraid_x1_map(address_map &map) ATTR_COLD;

	required_device<adc083x_device> m_adc;
	required_ioport_array<4> m_gun_inputs;
	output_finder<2> m_gun_recoil;
};

class setaroul_state : public seta_state
{
public:
	setaroul_state(const machine_config &mconfig, device_type type, const char *tag) :
		seta_state(mconfig, type, tag),
		m_rtc(*this, "rtc"),
		m_hopper(*this, "hopper"),
		m_bet(*this, "BET.%02X", 0),
		m_leds(*this, "led%u", 0U)
	{ }

	void setaroul(machine_config &config);
	void setaroulm(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_drop_start);
	ioport_value coin_sensors_r();
	ioport_value hopper_sensors_r();

	void screen_vblank(int state);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void rtc_w(u16 data);
	u16 rtc_r(offs_t offset);

	u16 inputs_r();
	void mux_w(u16 data);

	void pay_w(u8 data);
	void led_w(u8 data);

	u16 spritecode_r(offs_t offset);
	void spritecode_w(offs_t offset, u16 data);

	void spriteylow_w(offs_t offset, u16 data);

	void spritectrl_w(offs_t offset, u16 data);

	void setaroul_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	template <uint8_t Irq1, uint8_t Irq2> TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void setaroul_map(address_map &map) ATTR_COLD;

	required_device<upd4992_device> m_rtc;  // ! Actually D4911C !
	required_device<ticket_dispenser_device> m_hopper;
	required_ioport_array<26> m_bet;

	output_finder<2> m_leds;

	u8 m_mux = 0;
	u8 m_pay = 0;
	u8 m_led = 0;
	uint64_t m_coin_start_cycles = 0;

	void show_outputs();
};

class pairlove_state : public seta_state
{
public:
	pairlove_state(const machine_config &mconfig, device_type type, const char *tag) :
		seta_state(mconfig, type, tag)
	{ }

	void pairlove(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

protected:
	u16 prot_r(offs_t offset);
	void prot_w(offs_t offset, u16 data);

	void pairlove_map(address_map &map) ATTR_COLD;

	std::unique_ptr<u16 []> m_protram;
	std::unique_ptr<u16 []> m_protram_old;
};

class crazyfgt_state : public seta_state
{
public:
	crazyfgt_state(const machine_config &mconfig, device_type type, const char *tag) :
		seta_state(mconfig, type, tag),
		m_eeprom(*this, "eeprom")
	{ }

	void crazyfgt(machine_config &config);

private:
	void coin_counter_w(u8 data);
	void outputs_w(u8 data);

	void crazyfgt_map(address_map &map) ATTR_COLD;

	required_device<ds2430a_device> m_eeprom;
};

class jockeyc_state : public seta_state
{
public:
	jockeyc_state(const machine_config &mconfig, device_type type, const char *tag) :
		seta_state(mconfig, type, tag),
		m_rtc(*this, "rtc"),
		m_hopper1(*this, "hopper1"), m_hopper2(*this, "hopper2"),
		m_inttoote_700000(*this, "inttoote_700000"),
		m_key1(*this, "KEY1.%u", 0), m_key2(*this, "KEY2.%u", 0),
		m_dsw1(*this, "DSW1"),
		m_dsw2_3(*this, "DSW2_3"),
		m_cabinet(*this, "CABINET"),
		m_p1x(*this, "P1X"),
		m_p1y(*this, "P1Y"),
		m_out_cancel(*this, "cancel%u", 1U),
		m_out_payout(*this, "payout%u", 1U),
		m_out_start(*this, "start%u", 1U),
		m_out_help(*this, "help"),
		m_out_itstart(*this, "start")
	{ }

	void inttoote(machine_config &config);
	void jockeyc(machine_config &config);

	void init_inttoote();

private:
	void rtc_w(u16 data);
	u16 rtc_r(offs_t offset);

	u16 dsw_r(offs_t offset);
	u16 comm_r();

	u16 mux_r();
	void jockeyc_mux_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void jockeyc_out_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u16 trackball_r(offs_t offset);

	DECLARE_MACHINE_START(jockeyc);
	DECLARE_MACHINE_START(inttoote);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void inttoote_mux_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void inttoote_out_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 inttoote_700000_r(offs_t offset);

	void inttoote_map(address_map &map) ATTR_COLD;
	void jockeyc_map(address_map &map) ATTR_COLD;

	required_device<upd4992_device> m_rtc;  // ! Actually D4911C !
	required_device<ticket_dispenser_device> m_hopper1, m_hopper2; // the 2nd hopper is optional

	optional_shared_ptr<u16> m_inttoote_700000;
	required_ioport_array<5> m_key1, m_key2;
	required_ioport m_dsw1, m_dsw2_3;
	optional_ioport m_cabinet;
	optional_ioport m_p1x;
	optional_ioport m_p1y;

	output_finder<2> m_out_cancel;
	output_finder<2> m_out_payout;
	output_finder<2> m_out_start;
	output_finder<> m_out_help;
	output_finder<> m_out_itstart;

	u16 m_mux = 0;
	u16 m_out = 0;

	void update_hoppers();
	void show_outputs();
};


/***************************************************************************

                        Callbacks for the TileMap code

                              [ Tiles Format ]

Offset + 0x0000:
                    f--- ---- ---- ----     Flip X
                    -e-- ---- ---- ----     Flip Y
                    --dc ba98 7654 3210     Code

Offset + 0x1000:

                    fedc ba98 765- ----     -
                    ---- ---- ---4 3210     Color


                      [ TileMaps Control Registers]

Offset + 0x0:                               Scroll X
Offset + 0x2:                               Scroll Y
Offset + 0x4:
                    fedc ba98 765- ----     -
                    ---- ---- ---4 ----     Tilemap color mode switch (used in blandia and the other games using 6bpp graphics)
                    ---- ---- ---- 3---     Tilemap Select (There Are 2 Tilemaps Per Layer)
                    ---- ---- ---- -21-     0 (1 only in eightfrc, when flip is on!)
                    ---- ---- ---- ---0     ?

***************************************************************************/

X1_001_SPRITE_GFXBANK_CB_MEMBER(seta_state::setac_gfxbank_callback)
{
	const int bank = (color & 0x06) >> 1;
	code = (code & 0x3fff) + (bank * 0x4000);

	return code;
}

void seta_state::video_start()
{
	m_samples_bank = -1;    // set the samples bank to an out of range value at start-up
	if (m_x1_bank != nullptr)
		m_x1_bank->set_entry(0); // TODO : Unknown init

	m_vregs = 0;
	save_item(NAME(m_vregs));
}


/***************************************************************************

                            Palette Init Functions

***************************************************************************/

/* 2 layers, 6 bit deep.

   The game can select to repeat every 16 colors to fill the 64 colors for the 6bpp gfx
   or to use the first 64 colors of the palette regardless of the color code!
*/
void seta_state::blandia_palette(palette_device &palette) const
{
	for (int color = 0; color < 0x20; color++)
	{
		for (int pen = 0; pen < 0x40; pen++)
		{
			// layer 2-3
			palette.set_pen_indirect(0x0200 + ((color << 6) | pen), 0x200 + ((color << 4) | (pen & 0x0f)));
			palette.set_pen_indirect(0x1200 + ((color << 6) | pen), 0x200 + pen);

			// layer 0-1
			palette.set_pen_indirect(0x0a00 + ((color << 6) | pen), 0x400 + ((color << 4) | (pen & 0x0f)));
			palette.set_pen_indirect(0x1a00 + ((color << 6) | pen), 0x400 + pen);
		}
	}

	// setup the colortable for the effect palette.
	// what are used for palette from 0x800 to 0xBFF?
	for (int i = 0; i < 0x2200; i++)
		palette.set_pen_indirect(0x2200 + i, 0x600 + (i & 0x1ff));
}


/* layers have 6 bits per pixel, but the color code has a 16 colors granularity,
   even if the low 2 bits are ignored (so there are only 4 different palettes) */
void seta_state::gundhara_palette(palette_device &palette) const
{
	for (int color = 0; color < 0x20; color++)
	{
		for (int pen = 0; pen < 0x40; pen++)
		{
			palette.set_pen_indirect(0x0200 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff)); // used?
			palette.set_pen_indirect(0x1200 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff));

			palette.set_pen_indirect(0x0a00 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff)); // used?
			palette.set_pen_indirect(0x1a00 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff));
		}
	}
}


// layers have 6 bits per pixel, but the color code has a 16 colors granularity
void seta_state::jjsquawk_palette(palette_device &palette) const
{
	for (int color = 0; color < 0x20; color++)
	{
		for (int pen = 0; pen < 0x40; pen++)
		{
			palette.set_pen_indirect(0x0200 + ((color << 6) | pen), 0x400 + (((color << 4) + pen) & 0x1ff)); // used by madshark
			palette.set_pen_indirect(0x1200 + ((color << 6) | pen), 0x400 + (((color << 4) + pen) & 0x1ff));

			palette.set_pen_indirect(0x0a00 + ((color << 6) | pen), 0x200 + (((color << 4) + pen) & 0x1ff)); // used by madshark
			palette.set_pen_indirect(0x1a00 + ((color << 6) | pen), 0x200 + (((color << 4) + pen) & 0x1ff));
		}
	}
}


// layer 0 is 6 bit per pixel, but the color code has a 16 colors granularity
void seta_state::zingzip_palette(palette_device &palette) const
{
	for (int color = 0; color < 0x20; color++)
	{
		for (int pen = 0; pen < 0x40; pen++)
		{
			palette.set_pen_indirect(0x400 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff)); // used?
			palette.set_pen_indirect(0xc00 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff));
		}
	}
}

// color prom
void seta_state::palette_init_RRRRRGGGGGBBBBB_proms(palette_device &palette) const
{
	const u8 *const color_prom = memregion("proms")->base();
	for (int x = 0; x < 0x200 ; x++)
	{
		const int data = (color_prom[x*2] << 8) | color_prom[x*2 + 1];
		palette.set_pen_color(x, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
	}
}

void setaroul_state::setaroul_palette(palette_device &palette) const
{
	m_spritegen->gfx(0)->set_granularity(16);
	m_layers[0]->gfx(0)->set_granularity(16);

	palette_init_RRRRRGGGGGBBBBB_proms(palette);
}


void seta_state::set_pens()
{
	for (int i = 0; i < m_paletteram[0].bytes() / 2; i++)
	{
		const u16 data = m_paletteram[0][i];

		rgb_t color = rgb_t(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

		if (m_palette->indirect_entries() != 0)
			m_palette->set_indirect_color(i, color);
		else
			m_palette->set_pen_color(i, color);
	}

	if (m_paletteram[1] != nullptr)
	{
		for (int i = 0; i < m_paletteram[1].bytes() / 2; i++)
		{
			const u16 data = m_paletteram[1][i];

			rgb_t color = rgb_t(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

			if (m_palette->indirect_entries() != 0)
				m_palette->set_indirect_color(i + m_paletteram[0].bytes() / 2, color);
			else
				m_palette->set_pen_color(i + m_paletteram[0].bytes() / 2, color);
		}
	}
}


/***************************************************************************

                                Screen Drawing

***************************************************************************/

// For games without tilemaps
u32 seta_state::screen_update_seta_no_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	set_pens();
	bitmap.fill(0x1f0, cliprect);

	m_spritegen->draw_sprites(screen, bitmap,cliprect,0x1000);
	return 0;
}


// For games with 1 or 2 tilemaps
void seta_state::seta_layers_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_bank_size)
{
	const rectangle &visarea = screen.visible_area();
	const int vis_dimy = visarea.max_y - visarea.min_y + 1;

	const int flip = m_spritegen->is_flipped() ^ m_tilemaps_flip;
	for (int layer = 0; layer < 2; layer++)
	{
		if (m_layers[layer].found())
		{
			m_layers[layer]->set_flip(flip ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);

			// the hardware wants different scroll values when flipped

			/*  bg x scroll      flip
			    metafox     0000 025d = 0, $400-$1a3 = $400 - $190 - $13
			    eightfrc    ffe8 0272
			                fff0 0260 = -$10, $400-$190 -$10
			                ffe8 0272 = -$18, $400-$190 -$18 + $1a      */

			m_layers[layer]->update_scroll(vis_dimy, flip);
		}
	}

	unsigned layers_ctrl = ~0U;
#ifdef MAME_DEBUG
	if (screen.machine().input().code_pressed(KEYCODE_Z))
	{   int msk = 0;
		if (screen.machine().input().code_pressed(KEYCODE_Q))   msk |= 1;
		if (screen.machine().input().code_pressed(KEYCODE_W))   msk |= 2;
		if (screen.machine().input().code_pressed(KEYCODE_A))   msk |= 8;
		if (msk != 0) layers_ctrl &= msk;

		if (m_layers[1].found())
			popmessage("VR:%02X L0:%04X L1:%04X",
				m_vregs, m_layers[0]->vctrl(2), m_layers[1]->vctrl(2));
		else if (m_layers[0].found())
			popmessage("L0:%04X", m_layers[0]->vctrl(2));
	}
#endif

	bitmap.fill(0, cliprect);

	const int order = m_layers[1].found() ? m_vregs : 0;
	if (order & 1)  // swap the layers?
	{
		if (m_layers[1].found())
		{
			if (layers_ctrl & 2) m_layers[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		}

		if (order & 2)  // layer-sprite priority?
		{
			if (layers_ctrl & 8) m_spritegen->draw_sprites(screen, bitmap,cliprect,sprite_bank_size);

			if (order & 4)
			{
				popmessage("Missing palette effect. Contact MAMETesters.");
			}

			if (layers_ctrl & 1) m_layers[0]->draw(screen, bitmap, cliprect, 0, 0);
		}
		else
		{
			if (order & 4)
			{
				popmessage("Missing palette effect. Contact MAMETesters.");
			}

			if (layers_ctrl & 1) m_layers[0]->draw(screen, bitmap, cliprect, 0, 0);

			if (layers_ctrl & 8) m_spritegen->draw_sprites(screen, bitmap,cliprect,sprite_bank_size);
		}
	}
	else
	{
		if (layers_ctrl & 1) m_layers[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

		if (order & 2)  // layer-sprite priority?
		{
			if (layers_ctrl & 8) m_spritegen->draw_sprites(screen, bitmap,cliprect,sprite_bank_size);

			if ((order & 4) && m_paletteram[1] != nullptr)
			{
				m_layers[1]->draw_tilemap_palette_effect(bitmap, cliprect, flip);
			}
			else
			{
				if (order & 4)
				{
					popmessage("Missing palette effect. Contact MAMETesters.");
				}

				if (m_layers[1].found())
				{
					if (layers_ctrl & 2) m_layers[1]->draw(screen, bitmap, cliprect, 0, 0);
				}
			}
		}
		else
		{
			if ((order & 4) && m_paletteram[1] != nullptr)
			{
				m_layers[1]->draw_tilemap_palette_effect(bitmap, cliprect, flip);
			}
			else
			{
				if (order & 4)
				{
					popmessage("Missing palette effect. Contact MAMETesters.");
				}

				if (m_layers[1].found())
				{
					if (layers_ctrl & 2) m_layers[1]->draw(screen, bitmap, cliprect, 0, 0);
				}
			}

			if (layers_ctrl & 8) m_spritegen->draw_sprites(screen,bitmap,cliprect,sprite_bank_size);
		}
	}

}

u32 seta_state::screen_update_seta_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	seta_layers_update(screen, bitmap, cliprect, 0x1000);
	return 0;
}


u32 setaroul_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x0, cliprect);

	if (m_led & 0x80)
		seta_layers_update(screen, bitmap, cliprect, 0x800);

	return 0;
}

void setaroul_state::screen_vblank(int state)
{
	// rising edge
	if (state)
		m_spritegen->tnzs_eof();
}


u32 seta_state::screen_update_seta(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	set_pens();
	return screen_update_seta_layers(screen, bitmap, cliprect);
}


/***************************************************************************

                                Common Routines

***************************************************************************/

/*

 uPD71054C Timer

*/

void seta_state::pit_out0(int state)
{
	if (state)
		m_maincpu->set_input_line(4, ASSERT_LINE);
}


// DSW reading for 16 bit CPUs
u16 seta_state::seta_dsw_r(offs_t offset)
{
	const u16 dsw = m_dsw->read();
	if (offset == 0)    return (dsw >> 8) & 0xff;
	else                return (dsw >> 0) & 0xff;
}


/*

 Sprites Buffering

*/

void seta_state::screen_vblank_seta_buffer_sprites(int state)
{
	// rising edge
	if (state)
	{
		m_spritegen->setac_eof();
	}
}


/***************************************************************************

                                    Main CPU

(for debugging it is useful to be able to peek at some memory regions that
 the game writes to but never reads from. I marked this regions with an empty
 comment to distinguish them, since there's always the possibility that some
 games actually read from this kind of regions, expecting some hardware
 register's value there, instead of the data they wrote)

***************************************************************************/

u16 seta_state::ipl0_ack_r()
{
	if (!machine().side_effects_disabled())
		ipl0_ack_w();
	return 0;
}

void seta_state::ipl0_ack_w(u16 data)
{
	m_maincpu->set_input_line(1, CLEAR_LINE);
}

u16 seta_state::ipl1_ack_r()
{
	if (!machine().side_effects_disabled())
		ipl1_ack_w();
	return 0;
}

void seta_state::ipl1_ack_w(u16 data)
{
	m_maincpu->set_input_line(2, CLEAR_LINE);
}

void seta_state::ipl2_ack_w(u16 data)
{
	m_maincpu->set_input_line(4, CLEAR_LINE);
}


/*      76-- ----
        --5- ----     Sound Enable
        ---4 ----     toggled in IRQ1 by many games, irq acknowledge?
                      [original comment for the above: ?? 1 in oisipuzl, sokonuke (layers related)]
        ---- 3---     Coin #1 Lock Out
        ---- -2--     Coin #0 Lock Out
        ---- --1-     Coin #1 Counter
        ---- ---0     Coin #0 Counter     */

// some games haven't the coin lockout device (blandia, eightfrc, extdwnhl, gundhara, kamenrid, magspeed, sokonuke, zingzip, zombraid)
void seta_state::seta_coin_counter_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));

	if (m_x1snd.found())
		m_x1snd->enable_w(BIT(data, 6));
}

void seta_state::seta_coin_lockout_w(u8 data)
{
	seta_coin_counter_w(data);

	machine().bookkeeping().coin_lockout_w(0, !BIT(data, 2));
	machine().bookkeeping().coin_lockout_w(1, !BIT(data, 3));
}

void seta_state::seta_vregs_w(u8 data)
{
	m_vregs = data;

	/* Partly handled in vh_screenrefresh:

	        76-- ----
	        --54 3---     Samples Bank (in blandia, eightfrc, zombraid)
	        ---- -2--
	        ---- --1-     Sprites Above Frontmost Layer
	        ---- ---0     Layer 0 Above Layer 1
	*/

	const int new_bank = (data >> 3) & 0x7;

	if (new_bank != m_samples_bank)
	{
		m_samples_bank = new_bank;
		if (m_x1_bank != nullptr)
			m_x1_bank->set_entry(m_samples_bank);
	}
}


/***************************************************************************
                                Athena no Hatena?
***************************************************************************/

void seta_state::atehate_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                             // ROM
	map(0x900000, 0x9fffff).ram();                             // RAM
	map(0x100000, 0x103fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0x200000, 0x200001).nopw();                        // ? watchdog ?
	map(0x300000, 0x300001).nopw();                        // ? 0 (irq ack lev 2?)
	map(0x500000, 0x500001).nopw();                        // ? (end of lev 1: bit 4 goes 1,0,1)
	map(0x600000, 0x600003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x700000, 0x7003ff).ram().share("paletteram1");  // Palette
	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xb00000, 0xb00001).portr("P1");                 // P1
	map(0xb00002, 0xb00003).portr("P2");                 // P2
	map(0xb00004, 0xb00005).portr("COINS");              // Coins
	map(0xc00000, 0xc00001).ram();                             // ? 0x4000
	map(0xe00000, 0xe03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
}


/***************************************************************************
                        Blandia
***************************************************************************/

void seta_state::blandia_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();                             // ROM (up to 2MB)
	map(0x200000, 0x20ffff).ram();                             // RAM (main ram for zingzip, wrofaero writes to 20f000-20ffff)
	map(0x210000, 0x21ffff).ram();                             // RAM (gundhara)
	map(0x300000, 0x30ffff).ram();                             // RAM (wrofaero and blandia only?)
	map(0x400000, 0x400001).portr("P1");                 // P1
	map(0x400002, 0x400003).portr("P2");                 // P2
	map(0x400004, 0x400005).portr("COINS");              // Coins
	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_counter_w));       // Coin Counter (no lockout)
	map(0x500003, 0x500003).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x500004, 0x500005).nopw();
	map(0x600000, 0x600003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x700000, 0x7003ff).ram();                             // (rezon,jjsquawk)
	map(0x700400, 0x700fff).ram().share("paletteram1");  // Palette
	map(0x703c00, 0x7047ff).ram().share("paletteram2"); // 2nd Palette for the paletteoffseteffect
	map(0x800000, 0x8005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0x800600, 0x800607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0x880000, 0x880001).ram();                             // ? 0xc000
	map(0x900000, 0x903fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xa00000, 0xa00005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 0&1 Ctrl
	map(0xa80000, 0xa80005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 2&3 Ctrl
	map(0xb00000, 0xb03fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0xb04000, 0xb0ffff).ram();                             // (jjsquawk)
	map(0xb80000, 0xb83fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2&3
	map(0xb84000, 0xb8ffff).ram();                             // (jjsquawk)
	map(0xc00000, 0xc03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0xd00000, 0xd00007).nopw();                        // ?
	map(0xe00000, 0xe00001).w(FUNC(seta_state::ipl1_ack_w));              // ? VBlank IRQ Ack
	map(0xf00000, 0xf00001).w(FUNC(seta_state::ipl2_ack_w));              // ? Sound  IRQ Ack
}

void seta_state::blandia_x1_map(address_map &map)
{
	map(0x00000, 0xbffff).rom();
	map(0xc0000, 0xfffff).bankr("x1_bank");
}


/***************************************************************************
    Blandia (proto), Gundhara, J.J.Squawkers, Rezon, War of Aero, Zing Zing Zip
                        (with slight variations)
***************************************************************************/

void seta_state::blandiap_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();                             // ROM (up to 2MB)
	map(0x200000, 0x20ffff).ram();                             // RAM (main ram for zingzip, wrofaero writes to 20f000-20ffff)
	map(0x210000, 0x21ffff).ram();                             // RAM (gundhara)
	map(0x300000, 0x30ffff).ram();                             // RAM (wrofaero only?)
	map(0x400000, 0x400001).portr("P1");                 // P1
	map(0x400002, 0x400003).portr("P2");                 // P2
	map(0x400004, 0x400005).portr("COINS");              // Coins
	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_counter_w));       // Coin Counter (no lockout)
	map(0x500003, 0x500003).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x500004, 0x500005).nopw();
	map(0x600000, 0x600003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x700000, 0x7003ff).ram();                             // (rezon,jjsquawk)
	map(0x700400, 0x700fff).ram().share("paletteram1");  // Palette
	map(0x703c00, 0x7047ff).ram().share("paletteram2"); // 2nd Palette for the paletteoffseteffect
	map(0x800000, 0x803fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0x804000, 0x80ffff).ram();                             // (jjsquawk)
	map(0x880000, 0x883fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2&3
	map(0x884000, 0x88ffff).ram();                             // (jjsquawk)
	map(0x900000, 0x900005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 0&1 Ctrl
	map(0x980000, 0x980005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 2&3 Ctrl
	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xa80000, 0xa80001).ram();                             // ? 0x4000
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xc00000, 0xc03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0xd00000, 0xd00007).nopw();                        // ?
	map(0xe00000, 0xe00001).w(FUNC(seta_state::ipl1_ack_w));              // ? VBlank IRQ Ack
	map(0xf00000, 0xf00001).w(FUNC(seta_state::ipl2_ack_w));              // ? Sound  IRQ Ack
}


/***************************************************************************
    Blandia, Gundhara, J.J.Squawkers, Rezon, War of Aero, Zing Zing Zip
                    and Zombie Raid (with slight variations)
***************************************************************************/

double zombraid_state::adc_cb(u8 input)
{
	if (input == ADC083X_AGND)
		return 0.0;
	else if (input == ADC083X_VREF)
		return 1.0;
	else
		return m_gun_inputs[input - ADC083X_CH0]->read() / 255.0;
}

u16 zombraid_state::gun_r()// Serial interface
{
	return m_adc->do_read();
}

// Bit 0 is clock, 1 is data, 2 is reset
void zombraid_state::gun_w(u16 data)
{
	m_adc->cs_write(BIT(data, 2));
	m_adc->di_write(BIT(data, 1));
	m_adc->clk_write(BIT(data, 0));

	// Gun Recoils
	// Note:  In debug menu recoil solenoids strobe when held down.  Is this correct??
	m_gun_recoil[0] = BIT(data, 4);
	m_gun_recoil[1] = BIT(data, 3);
}

void seta_state::rezon_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();                             // ROM (up to 2MB)
	map(0x200000, 0x20ffff).ram();                             // RAM
	map(0x400000, 0x400001).portr("P1");                 // P1
	map(0x400002, 0x400003).portr("P2");                 // P2
	map(0x400004, 0x400005).portr("COINS");              // Coins

	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_counter_w));       // Coin Counter (no lockout)
	map(0x500003, 0x500003).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x500004, 0x500005).w(FUNC(seta_state::ipl1_ack_w));
	map(0x500006, 0x500007).nopr();

	map(0x600000, 0x600003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x700000, 0x7003ff).ram();                             // (rezon,jjsquawk)
	map(0x700400, 0x700fff).ram().share("paletteram1");  // Palette
	map(0x701000, 0x70ffff).ram();                             //
	map(0x800000, 0x803fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0x804000, 0x80ffff).ram();                             // (jjsquawk)
	map(0x880000, 0x883fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2&3
	map(0x884000, 0x88ffff).ram();                             // (jjsquawk)
	map(0x900000, 0x900005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 0&1 Ctrl
	map(0x980000, 0x980005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 2&3 Ctrl
	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xa80000, 0xa80001).ram();                             // ? 0x4000
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xc00000, 0xc03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
}

void seta_state::zingzip_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();                             // ROM (up to 2MB)
	map(0x200000, 0x20ffff).ram();                             // RAM
	map(0x210000, 0x21ffff).ram();                             // RAM (gundhara)
	map(0x300000, 0x30ffff).ram().share("nvram");              // actually 8K x8 SRAM in zombraid
	map(0x400000, 0x400001).portr("P1");                 // P1
	map(0x400002, 0x400003).portr("P2");                 // P2
	map(0x400004, 0x400005).portr("COINS");              // Coins

	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_counter_w));       // Coin Counter (no lockout)
	map(0x500003, 0x500003).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x500004, 0x500005).noprw();

	map(0x600000, 0x600003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x700000, 0x7003ff).ram();                             // (rezon,jjsquawk)
	map(0x700400, 0x700fff).ram().share("paletteram1");  // Palette
	map(0x701000, 0x70ffff).ram();                             //
	map(0x800000, 0x803fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0x804000, 0x80ffff).ram();                             // (jjsquawk)
	map(0x880000, 0x883fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2&3
	map(0x884000, 0x88ffff).ram();                             // (jjsquawk)
	map(0x900000, 0x900005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 0&1 Ctrl
	map(0x980000, 0x980005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 2&3 Ctrl
	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xa80000, 0xa80001).ram();                             // ? 0x4000
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xc00000, 0xc03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0xe00000, 0xe00001).w(m_watchdog, FUNC(watchdog_timer_device::reset16_w));
}

void seta_state::wrofaero_map(address_map &map)
{
	zingzip_map(map);
	map(0xd00000, 0xd00007).w("pit", FUNC(pit8254_device::write)).umask16(0x00ff);
	map(0xf00000, 0xf00001).w(FUNC(seta_state::ipl2_ack_w));
}

void zombraid_state::zombraid_map(address_map &map)
{
	zingzip_map(map);
	map(0x400000, 0x400001).nopw();
	map(0xf00000, 0xf00001).w(FUNC(zombraid_state::gun_w));
	map(0xf00002, 0xf00003).r(FUNC(zombraid_state::gun_r));
}

void zombraid_state::zombraid_x1_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom();
	map(0x80000, 0xfffff).bankr("x1_bank");
}

void seta_state::zingzipbl_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom(); // ok
	map(0x200000, 0x20ffff).ram(); // ok
	// TODO: coins (possibly 0x400000). For now only free-play 'works'.
	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_lockout_w)); // ok
	map(0x500003, 0x500003).w(FUNC(seta_state::seta_vregs_w)); // maybe?
	map(0x700400, 0x700fff).ram().share("paletteram1"); // ok
	map(0x800000, 0x803fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // ok
	map(0x880000, 0x883fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // ok
	map(0x902001, 0x902001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // should be ok, but bad ROM
	// the following appear to be video registers, but laid out differently than in the original. Trampoline galore for now. TODO: verify
	map(0x902004, 0x902005).lw16(NAME([this] (offs_t offset, uint16_t data, uint16_t mem_mask) { m_layers[1]->vctrl_w(1, data, mem_mask); }));
	map(0x902006, 0x902007).lw16(NAME([this] (offs_t offset, uint16_t data, uint16_t mem_mask) { m_layers[1]->vctrl_w(0, data, mem_mask); }));
	map(0x902008, 0x902009).lw16(NAME([this] (offs_t offset, uint16_t data, uint16_t mem_mask) { m_layers[0]->vctrl_w(1, data, mem_mask); }));
	map(0x90200a, 0x90200b).lw16(NAME([this] (offs_t offset, uint16_t data, uint16_t mem_mask) { m_layers[0]->vctrl_w(0, data, mem_mask); }));
	map(0x900004, 0x900005).lw16(NAME([this] (offs_t offset, uint16_t data, uint16_t mem_mask) { m_layers[0]->vctrl_w(2, data, mem_mask); }));
	map(0x980004, 0x980005).lw16(NAME([this] (offs_t offset, uint16_t data, uint16_t mem_mask) { m_layers[1]->vctrl_w(2, data, mem_mask); }));
	map(0x902010, 0x902013).r(FUNC(seta_state::seta_dsw_r)); // ok
	map(0x902014, 0x902015).portr("P1"); // ok
	map(0x902016, 0x902017).portr("P2"); // ok

	// TODO: sprites also seem to have different registers, need correct implementation
	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xa00608, 0xa00fff).ram(); // zeroed on start up
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr

	map(0xc00000, 0xc000ff).ram(); // zeroed on startup, doesn't seem to be used later
	map(0xe00000, 0xe00001).w(m_watchdog, FUNC(watchdog_timer_device::reset16_w));
}

void seta_state::jjsquawb_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();                             // ROM (up to 2MB)
	map(0x200000, 0x20ffff).ram().share("workram");     // RAM (pointer for zombraid crosshair hack)
	map(0x210000, 0x21ffff).ram();                             // RAM (gundhara)
	map(0x300000, 0x30ffff).ram();                             // RAM (wrofaero only?)
	map(0x400000, 0x400001).portr("P1");                 // P1
	map(0x400002, 0x400003).portr("P2");                 // P2
	map(0x400004, 0x400005).portr("COINS");              // Coins
	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_lockout_w));       // Coin Lockout
	map(0x500003, 0x500003).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x500004, 0x500005).nopw();
	map(0x600000, 0x600003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x700000, 0x70b3ff).ram();                             // RZ: (rezon,jjsquawk)
	map(0x70b400, 0x70bfff).ram().share("paletteram1");  // Palette
	map(0x70c000, 0x70ffff).ram();                             //
	map(0x800000, 0x803fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0
	map(0x804000, 0x807fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2
	map(0x884000, 0x88ffff).ram();                             // (jjsquawk)
	map(0x908000, 0x908005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 0&1 Ctrl
	map(0x909000, 0x909005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 2&3 Ctrl
	map(0xa0a000, 0xa0a5ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // RZ: Sprites Y
	map(0xa0a600, 0xa0a607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
//  map(0xa80000, 0xa80001).ram()                              // ? 0x4000
	map(0xb0c000, 0xb0ffff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // RZ: Sprites Code + X + Attr
	map(0xc00000, 0xc03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0xe00000, 0xe00001).nopw();                        // ? VBlank IRQ Ack
	map(0xf00000, 0xf00001).nopw();                        // ? Sound  IRQ Ack
}


/***************************************************************************
        Orbs
***************************************************************************/

void seta_state::orbs_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                             // ROM
	map(0xf00000, 0xf0ffff).ram();                             // RAM
	map(0x100000, 0x100001).nopr();                         // ?
	map(0x200000, 0x200001).nopr();                         // ?
	map(0x300000, 0x300003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x400000, 0x400001).nopw();                        // ?
	map(0x500000, 0x500001).portr("P1");                 // P1
	map(0x500002, 0x500003).portr("P2");                 // P2
	map(0x500004, 0x500005).portr("COINS");              // Coins
	//map(0x600000, 0x60000f).r(FUNC(seta_state::krzybowl_input_r);   // P1
	map(0x8000f0, 0x8000f1).ram();                             // NVRAM
	map(0x800100, 0x8001ff).ram();                             // NVRAM
	map(0xa00000, 0xa03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0xb00000, 0xb003ff).ram().share("paletteram1");  // Palette
	map(0xc00000, 0xc03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xd00000, 0xd00001).ram();                             // ? 0x4000
	map(0xe00000, 0xe005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xe00600, 0xe00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
}


/***************************************************************************
                  Kero Kero Keroppi no Isshoni Asobou
***************************************************************************/

static const u16 keroppi_protection_word[] = {
	0x0000,
	0x0000, 0x0000, 0x0000,
	0x2000, 0x2000, 0x2000,
	0x2000, 0x2000, 0x2000,
	0x0400, 0x0400, 0x0400,
	0x0000, 0x0000, 0x0000
};


u16 keroppi_state::protection_r()
{
	const u16 result = keroppi_protection_word[m_protection_count];

	if (!machine().side_effects_disabled())
	{
		m_protection_count++;
		if (m_protection_count > 15)
			m_protection_count = 15;
	}
	return result;
}

u16 keroppi_state::protection_init_r()
{
	if (!machine().side_effects_disabled())
		m_protection_count = 0;

	return 0x00;
}

u16 keroppi_state::coin_r()
{
	u16 result = m_coins->read();

	if (m_prize_hop == 2)
	{
		result &= ~0x0002;      // prize hopper
		if (!machine().side_effects_disabled())
			m_prize_hop = 0;
	}

	return result;
}

TIMER_CALLBACK_MEMBER(keroppi_state::prize_hop_callback)
{
	m_prize_hop = 2;
}

void keroppi_state::prize_w(u16 data)
{
	if ((data & 0x0010) && !m_prize_hop)
	{
		m_prize_hop = 1;
		m_prize_hop_timer->adjust(attotime::from_seconds(3), 0x20);
	}
}

void keroppi_state::keroppi_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                      // ROM
	map(0xf00000, 0xf0ffff).ram();                                      // RAM
	map(0x100000, 0x100001).r(FUNC(keroppi_state::protection_r));       //
	map(0x200000, 0x200001).r(FUNC(keroppi_state::protection_init_r));  //
	map(0x300000, 0x300003).r(FUNC(keroppi_state::seta_dsw_r));         // DSW
	map(0x400000, 0x400001).nopw();                                     // ?
	map(0x500000, 0x500001).portr("P1");                                // P1
	map(0x500002, 0x500003).portr("P2");                                // P2
	map(0x500004, 0x500005).r(FUNC(keroppi_state::coin_r));             // Coins
	map(0x8000f0, 0x8000f1).ram();                                      // NVRAM
	map(0x800100, 0x8001ff).ram();                                      // NVRAM
	map(0x900000, 0x900001).nopw();                                     // ?
	map(0x900002, 0x900003).w(FUNC(keroppi_state::prize_w));            //
	map(0xa00000, 0xa03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0xb00000, 0xb003ff).ram().share("paletteram1");                 // Palette
	map(0xc00000, 0xc03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xd00000, 0xd00001).ram();                                      // ? 0x4000
	map(0xe00000, 0xe005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xe00600, 0xe00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
}

void keroppi_state::machine_start()
{
	seta_state::machine_start();

	m_prize_hop_timer = timer_alloc(FUNC(keroppi_state::prize_hop_callback), this);

	m_prize_hop = 0;
	m_protection_count = 0;

	save_item(NAME(m_prize_hop));
	save_item(NAME(m_protection_count));
}


/***************************************************************************
                                Block Carnival
***************************************************************************/

void seta_state::blockcar_interrupt_w(u8 data)
{
	// ? 0/1 (IRQ acknowledge?)
	if (!BIT(data, 0))
		m_maincpu->set_input_line(3, CLEAR_LINE);
}

// similar to krzybowl
void seta_state::blockcar_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                             // ROM
	map(0xf00000, 0xf03fff).ram();                             // RAM
	map(0xf04000, 0xf041ff).ram();                             // Backup RAM?
	map(0xf05000, 0xf050ff).ram();                             // Backup RAM?
	map(0x100000, 0x100001).nopw();                        // ? 1 (start of interrupts, main loop: watchdog?)
	map(0x200001, 0x200001).w(FUNC(seta_state::blockcar_interrupt_w));
	map(0x300000, 0x300003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x400001, 0x400001).w(FUNC(seta_state::seta_coin_lockout_w));       // Coin Lockout + Sound Enable (bit 4?)
	map(0x500000, 0x500001).portr("P1");                 // P1
	map(0x500002, 0x500003).portr("P2");                 // P2
	map(0x500004, 0x500005).portr("COINS");              // Coins
	map(0xa00000, 0xa03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0xb00000, 0xb003ff).ram().share("paletteram1");  // Palette
	map(0xc00000, 0xc03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16)); // Sprites Code + X + Attr
	map(0xd00000, 0xd00001).ram(); // ? 0x4000
	map(0xe00000, 0xe005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16)); // Sprites Y
	map(0xe00600, 0xe00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
}

void seta_state::blockcarb_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                             // ROM
	map(0xf00000, 0xf03fff).ram();                             // RAM
	map(0xf04000, 0xf041ff).ram();                             // Backup RAM?
	map(0xf05000, 0xf050ff).ram();                             // Backup RAM?
	map(0x100000, 0x100001).nopw();                        // ? 1 (start of interrupts, main loop: watchdog?)
	map(0x200001, 0x200001).w(FUNC(seta_state::blockcar_interrupt_w));
	map(0x300000, 0x300003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x400001, 0x400001).w(FUNC(seta_state::seta_coin_lockout_w));       // Coin Lockout + Sound Enable (bit 4?)
	map(0x500000, 0x500001).portr("P1");                 // P1
	map(0x500002, 0x500003).portr("P2");                 // P2
	map(0x500004, 0x500005).portr("COINS");              // Coins
	map(0x500009, 0x500009).w("oki", FUNC(okim6295_device::write));
	map(0x50000d, 0x50000d).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xa00000, 0xa03fff).noprw();   // Sound - not on this bootleg
	map(0xb00000, 0xb003ff).ram().share("paletteram1");  // Palette
	map(0xc00000, 0xc03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16)); // Sprites Code + X + Attr
	map(0xd00000, 0xd00001).ram(); // ? 0x4000
	map(0xe00000, 0xe005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16)); // Sprites Y
	map(0xe00600, 0xe00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
}


/***************************************************************************
                                Daioh
***************************************************************************/

void seta_state::daioh_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                             // ROM
	map(0x100000, 0x10ffff).ram();                             // RAM
	map(0x400000, 0x400001).portr("P1");                 // P1
	map(0x400002, 0x400003).portr("P2");                 // P2
	map(0x400004, 0x400005).portr("COINS");              // Coins
	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_lockout_w));       // Coin Lockout
	map(0x500003, 0x500003).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x500004, 0x500005).nopw();
	map(0x500006, 0x500007).portr("EXTRA");              // Buttons 4,5,6
	map(0x600000, 0x600003).r(FUNC(seta_state::seta_dsw_r));   // DSW
	map(0x700000, 0x7003ff).ram();
	map(0x700400, 0x700fff).ram().share("paletteram1");  // Palette
	map(0x701000, 0x70ffff).ram();                             //
	map(0x800000, 0x803fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0x804000, 0x80ffff).ram();                             //
	map(0x880000, 0x883fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2&3
	map(0x884000, 0x88ffff).ram();                             //
	map(0x900000, 0x900005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 0&1 Ctrl
	map(0x980000, 0x980005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 2&3 Ctrl
	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16)); // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));

	map(0xa80000, 0xa80001).ram(); // ? 0x4000
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16)); // Sprites Code + X + Attr
	map(0xb04000, 0xb13fff).ram();
	map(0xc00000, 0xc03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0xe00000, 0xe00001).nopw();    //
}


/***************************************************************************
                       Daioh (location test version)
***************************************************************************/

void seta_state::daiohp_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().mirror(0x080000);         // ROM
	map(0x100000, 0x17ffff).rom().mirror(0x080000);         // ROM
	map(0x200000, 0x20ffff).ram();                             // RAM
	map(0x400000, 0x400001).portr("P1");                 // P1
	map(0x400002, 0x400003).portr("P2");                 // P2
	map(0x400004, 0x400005).portr("COINS");              // Coins
	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_lockout_w));       // Coin Lockout
	map(0x500003, 0x500003).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x500004, 0x500005).nopw();
	map(0x500006, 0x500007).portr("EXTRA");              // Buttons 4,5,6
	map(0x600000, 0x600003).r(FUNC(seta_state::seta_dsw_r));   // DSW
	map(0x700000, 0x7003ff).ram();
	map(0x700400, 0x700fff).ram().share("paletteram1");  // Palette
	map(0x701000, 0x70ffff).ram();                             //
	map(0x800000, 0x803fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0x804000, 0x80ffff).ram();                             //
	map(0x880000, 0x883fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2&3
	map(0x884000, 0x88ffff).ram();                             //
	map(0x900000, 0x900005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 0&1 Ctrl
	map(0x980000, 0x980005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 2&3 Ctrl
	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16)); // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));

	map(0xa80000, 0xa80001).ram(); // ? 0x4000
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16)); // Sprites Code + X + Attr
	map(0xb04000, 0xb13fff).ram();
	map(0xc00000, 0xc03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0xd00000, 0xd00007).nopw();                        // ?
	map(0xe00000, 0xe00001).nopw();                        // ? VBlank IRQ Ack
	map(0xf00000, 0xf00001).nopw();                        // ? Sound  IRQ Ack
}


/***************************************************************************
        Dragon Unit, Quiz Kokology, Quiz Kokology 2, Strike Gunner
***************************************************************************/

void seta_state::drgnunit_map(address_map &map)
{
	map(0x000000, 0x0bffff).rom();                             // ROM
	map(0xf00000, 0xf0ffff).ram();                             // RAM (qzkklogy)
	map(0xffc000, 0xffffff).ram();                             // RAM (drgnunit,stg)
	map(0x100000, 0x103fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0x200000, 0x200001).nopw();                        // Watchdog
	map(0x300000, 0x300001).nopw();                        // ? IRQ Ack
	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_lockout_w));       // Coin Lockout
	map(0x500003, 0x500003).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x600000, 0x600003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x700000, 0x7003ff).ram().share("paletteram1");  // Palette
	map(0x800000, 0x800005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM Ctrl
	map(0x900000, 0x903fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM
	map(0x904000, 0x90ffff).nopw();                        // unused (qzkklogy)
	map(0xb00000, 0xb00001).portr("P1");                 // P1
	map(0xb00002, 0xb00003).portr("P2");                 // P2
	map(0xb00004, 0xb00005).portr("COINS");              // Coins
	map(0xb00006, 0xb00007).nopr();                         // unused (qzkklogy)
	map(0xc00000, 0xc00001).ram();                             // ? $4000
	map(0xd00000, 0xd005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xd00600, 0xd00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xe00000, 0xe03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
}


/***************************************************************************
                                The Roulette
***************************************************************************/

void setaroul_state::machine_start()
{
	seta_state::machine_start();

	m_leds.resolve();
}

// Coin drop
void setaroul_state::machine_reset()
{
	seta_state::machine_reset();

	m_coin_start_cycles = 0;
}

INPUT_CHANGED_MEMBER( setaroul_state::coin_drop_start )
{
	if (newval && !m_coin_start_cycles)
		m_coin_start_cycles = m_maincpu->total_cycles();
}

ioport_value setaroul_state::coin_sensors_r()
{
	u8 data = 0x03;

	// simulates the passage of coins through multiple sensors
	if (m_coin_start_cycles)
	{
		attotime diff = m_maincpu->cycles_to_attotime(m_maincpu->total_cycles() - m_coin_start_cycles);

		if (diff <= attotime::from_msec(16*10))
			data &= ~0x02;

		if (diff > attotime::from_msec(16*5) && diff < attotime::from_msec(16*15))
			data &= ~0x01;

		if (diff > attotime::from_msec(16*15))
			m_coin_start_cycles = 0;
	}

	return data;
}

// the spritey low bits are mapped to 1 in every 4 bytes here as if it were a 32-bit bus..which is weird
// other ram is similar..

void setaroul_state::spritecode_w(offs_t offset, u16 data)
{
	if ((offset & 1) == 1) m_spritegen->spritecodelow_w8(offset >> 1, (data & 0xff00) >> 8);
	if ((offset & 1) == 0) m_spritegen->spritecodehigh_w8(offset >> 1, (data & 0xff00) >> 8);
}

u16 setaroul_state::spritecode_r(offs_t offset)
{
	u16 ret;
	if ((offset & 1) == 1)
		ret = m_spritegen->spritecodelow_r8(offset >> 1);
	else
		ret = m_spritegen->spritecodehigh_r8(offset >> 1);
	return ret << 8;
}

void setaroul_state::spriteylow_w(offs_t offset, u16 data)
{
	if ((offset & 1) == 0) m_spritegen->spriteylow_w8(offset >> 1, (data & 0xff00) >> 8);
}

void setaroul_state::spritectrl_w(offs_t offset, u16 data)
{
	if ((offset & 1) == 0) m_spritegen->spritectrl_w8(offset >> 1, (data & 0xff00) >> 8);
}

// RTC (To do: write a D4911C device)
u16 setaroul_state::rtc_r(offs_t offset)
{
	if (offset >= 7)
		++offset;
	if (offset / 2 >= 7)
		return 0;
	return (m_rtc->read(offset / 2) >> ((offset & 1) * 4)) & 0xf;
}

void setaroul_state::rtc_w(u16 data)
{
}

// Inputs
u16 setaroul_state::inputs_r()
{
	if (m_mux < 0x1a)
		return m_bet[m_mux]->read();
	return 0xff;
}
void setaroul_state::mux_w(u16 data)
{
	m_mux = data;
}

// Outputs
void setaroul_state::show_outputs()
{
#ifdef MAME_DEBUG
	popmessage("Pay: %02X Led: %02X", m_pay, m_led);
#endif
}

void setaroul_state::pay_w(u8 data)
{
	m_pay = data;

	machine().bookkeeping().coin_counter_w(6,   data  & 0x01);  // coin in         (meter 6 in input test, touch '7')
	machine().bookkeeping().coin_counter_w(5,   data  & 0x02);  // coupon in       (meter 5 in input test, touch '6')
	machine().bookkeeping().coin_counter_w(4,   data  & 0x04);  // coin drop       (meter 4 in input test, touch '5')
	machine().bookkeeping().coin_counter_w(3,   data  & 0x08);  // unused?         (meter 3 in input test, touch '4')
	machine().bookkeeping().coin_counter_w(2,   data  & 0x10);  // medal out       (meter 2 in input test, touch '3')
	machine().bookkeeping().coin_counter_w(1,   data  & 0x20);  // note in         (meter 1 in input test, touch '2')
	//                                          data  & 0x40    // hopper lock-out (lock.o  in input test, touch '8')
	//                                          data  & 0x80    // hopper motor    (hop.h   in input test, touch '0')
	m_hopper->motor_w((!(data & 0x40) && (data & 0x80)) ? 1 : 0);

	show_outputs();
}

void setaroul_state::led_w(u8 data)
{
	m_led = data;

	m_leds[0] = BIT(data, 0);  // pay out        (hop.c in input test, touch '1')
	m_leds[1] = BIT(data, 1);  // call attendant (cal.o in input test, touch '9')
	//
	//                          data  & 0x10    // hopper divider (divider in input test, touch '10')
	//                          data  & 0x80    // video enable?

	show_outputs();
}

void setaroul_state::setaroul_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();

	map(0x800000, 0x800003).noprw(); // RS232C Auto Time Set: r/w

	map(0xc00000, 0xc03fff).ram().share("nvram");

	map(0xc40000, 0xc40001).noprw(); // lev. 2/5 irq ack
	map(0xc80000, 0xc80001).noprw(); // lev. 4   irq ack

	map(0xcc0000, 0xcc001f).rw(FUNC(setaroul_state::rtc_r), FUNC(setaroul_state::rtc_w));

	map(0xd00000, 0xd00001).w(m_watchdog, FUNC(watchdog_timer_device::reset16_w));

	map(0xd40000, 0xd40001).portr("DSW1-A");
	map(0xd40001, 0xd40001).w(FUNC(setaroul_state::pay_w));
	map(0xd40002, 0xd40003).portr("DSW1-B");

	map(0xd40004, 0xd40005).portr("DSW2-A");
	map(0xd40006, 0xd40007).portr("DSW2-B");

	map(0xd40008, 0xd40009).portr("COIN");
	map(0xd40009, 0xd40009).w(FUNC(setaroul_state::led_w));
	map(0xd4000a, 0xd4000b).portr("DOOR");

	map(0xd40010, 0xd40011).rw(FUNC(setaroul_state::inputs_r), FUNC(setaroul_state::mux_w));

	map(0xd40018, 0xd40019).portr("DSW3");

	map(0xdc0000, 0xdc3fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound

	map(0xe00000, 0xe03fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1");
	map(0xe40000, 0xe40005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM Ctrl
	map(0xf00000, 0xf03fff).rw(FUNC(setaroul_state::spritecode_r), FUNC(setaroul_state::spritecode_w));
	map(0xf40000, 0xf40bff).w(FUNC(setaroul_state::spriteylow_w));
	map(0xf40c00, 0xf40c11).w(FUNC(setaroul_state::spritectrl_w));

//  map(0xf80000, 0xf80001).w(FUNC(setaroul_state::xxx)); // $40 at boot
}


/***************************************************************************
                        Extreme Downhill / Sokonuke
***************************************************************************/

u16 seta_state::extdwnhl_watchdog_r()
{
	// TODO: open bus for unmapped I/O areas?
	// extdwnhl wants to read non-zero at POST for watchdog (?) otherwise will mangle RAM boundaries
	// and fails booting.
	// It also perform a blantantly invalid access during ending, expecting anything that isn't a 0xffff
	// to skip it. (A0=0x20434f56, https://mametesters.org/view.php?id=8614)
	m_watchdog->reset16_w();
	return 0xffff;
}

void seta_state::extdwnhl_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                             // ROM
	map(0x200000, 0x20ffff).ram();                             // RAM
	map(0x210000, 0x21ffff).ram();                             // RAM
	map(0x220000, 0x23ffff).ram();                             // RAM (sokonuke)
	map(0x400000, 0x400001).portr("P1");                 // P1
	map(0x400002, 0x400003).portr("P2");                 // P2
	map(0x400004, 0x400005).portr("COINS");              // Coins
	map(0x400008, 0x40000b).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x40000c, 0x40000d).r(FUNC(seta_state::extdwnhl_watchdog_r)).w(m_watchdog, FUNC(watchdog_timer_device::reset16_w));    // Watchdog (extdwnhl (R) & sokonuke (W) MUST RETURN $FFFF)
	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_counter_w));       // Coin Counter (no lockout)
	map(0x500003, 0x500003).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x500004, 0x500007).noprw();                             // IRQ Ack  (extdwnhl (R) & sokonuke (W))
	map(0x600400, 0x600fff).ram().share("paletteram1");  // Palette
	map(0x601000, 0x610bff).ram();                             //
	map(0x800000, 0x803fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0x804000, 0x80ffff).ram();                             //
	map(0x880000, 0x883fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2&3
	map(0x884000, 0x88ffff).ram();                             //
	map(0x900000, 0x900005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 0&1 Ctrl
	map(0x980000, 0x980005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 2&3 Ctrl
	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xa80000, 0xa80001).ram();                             // ? 0x4000
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xb04000, 0xb13fff).ram();                             //
	map(0xe00000, 0xe03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
}


/***************************************************************************
        (Kamen) Masked Riders Club Battle Race / Mad Shark
***************************************************************************/

void seta_state::kamenrid_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                             // ROM
	map(0x200000, 0x20ffff).ram();                             // RAM
	map(0x500000, 0x500001).portr("P1");                 // P1
	map(0x500002, 0x500003).portr("P2");                 // P2
	map(0x500004, 0x500007).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x500008, 0x500009).portr("COINS");              // Coins
	map(0x50000c, 0x50000d).rw(m_watchdog, FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));    // xx Watchdog? (sokonuke)
	map(0x600001, 0x600001).w(FUNC(seta_state::seta_coin_counter_w));       // Coin Counter (no lockout)
	map(0x600003, 0x600003).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x600004, 0x600005).w(FUNC(seta_state::ipl1_ack_w));
	map(0x600006, 0x600007).w(FUNC(seta_state::ipl2_ack_w));
	map(0x700000, 0x7003ff).ram();                             // Palette RAM (tested)
	map(0x700400, 0x700fff).ram().share("paletteram1");  // Palette
	map(0x701000, 0x703fff).ram();                             // Palette
	map(0x800000, 0x803fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0x804000, 0x807fff).ram(); // tested
	map(0x880000, 0x883fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2&3
	map(0x884000, 0x887fff).ram(); // tested
	map(0x900000, 0x900005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 0&1 Ctrl
	map(0x980000, 0x980005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 2&3 Ctrl
	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xa80000, 0xa80001).ram();                             // ? $4000
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xb04000, 0xb07fff).ram();                             // tested
	map(0xc00000, 0xc00007).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0x00ff);
	map(0xd00000, 0xd03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
}

// almost identical to kamenrid
void seta_state::madshark_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                             // ROM
	map(0x200000, 0x20ffff).ram();                             // RAM
	map(0x500000, 0x500001).portr("P1");                 // P1
	map(0x500002, 0x500003).portr("P2");                 // P2
	map(0x500004, 0x500005).portr("COINS");              // Coins
	map(0x500008, 0x50000b).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x50000c, 0x50000d).w(m_watchdog, FUNC(watchdog_timer_device::reset16_w));
	map(0x600001, 0x600001).w(FUNC(seta_state::seta_coin_lockout_w));       // Coin Lockout
	map(0x600003, 0x600003).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x600004, 0x600005).w(FUNC(seta_state::ipl1_ack_w));
	map(0x600006, 0x600007).w(FUNC(seta_state::ipl2_ack_w));
	map(0x700400, 0x700fff).ram().share("paletteram1");  // Palette
	map(0x800000, 0x803fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0x880000, 0x883fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2&3
	map(0x900000, 0x900005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 0&1 Ctrl
	map(0x980000, 0x980005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 2&3 Ctrl

	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xa80000, 0xa80001).ram();                             // ? $4000
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xc00000, 0xc00007).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0x00ff);
	map(0xd00000, 0xd03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
}

void seta_state::madsharkbl_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                             // ROM
	map(0x100001, 0x100001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x200000, 0x20ffff).ram();                             // RAM
	map(0x500000, 0x500001).portr("P1");                 // P1
	map(0x500002, 0x500003).portr("P2");                 // P2
	map(0x500004, 0x500005).portr("COINS");              // Coins
	map(0x500008, 0x50000b).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x50000d, 0x50000d).lw8(NAME([this] (u8 data) { m_oki_bank->set_entry(bitswap<2>(data, 3, 2)); })); // watchdog has been patched out, Oki bank selection instead. TODO: doesn't work
	map(0x600001, 0x600001).w(FUNC(seta_state::seta_coin_lockout_w));       // Coin Lockout
	map(0x600003, 0x600003).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x600004, 0x600005).w(FUNC(seta_state::ipl1_ack_w));
	map(0x600006, 0x600007).w(FUNC(seta_state::ipl2_ack_w));
	map(0x700400, 0x700fff).ram().share("paletteram1");  // Palette
	map(0x800000, 0x803fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0x880000, 0x883fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2&3
	map(0x900000, 0x900005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 0&1 Ctrl
	map(0x980000, 0x980005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 2&3 Ctrl

	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xa80000, 0xa80001).ram();                             // ? $4000
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xc00000, 0xc00007).noprw(); // leftover from the PIC of the original
	map(0xd00000, 0xd03fff).noprw(); // leftover from the X1-010 of the original
}

void seta_state::madsharkbl_oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr(m_oki_bank);
}

void magspeed_state::lights_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_lights[offset]);

	for (int i = 0; i < 16; i++)
		m_leds[offset * 16 + i] = BIT(m_lights[offset], i);

//  popmessage("%04X %04X %04X", m_lights[0], m_lights[1], m_lights[2]);
}

// almost identical to kamenrid
void magspeed_state::magspeed_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                              // ROM
	map(0x1f8000, 0x1f8fff).noprw();                                            // NVRAM?
	map(0x200000, 0x20ffff).ram();                                              // RAM
	map(0x500000, 0x500001).portr("P1");                                        // P1
	map(0x500002, 0x500003).portr("P2");                                        // P2
	map(0x500004, 0x500005).portr("COINS");                                     // Coins
	map(0x500008, 0x50000b).r(FUNC(magspeed_state::seta_dsw_r));                // DSW
	map(0x50000c, 0x50000d).w(m_watchdog, FUNC(watchdog_timer_device::reset16_w));
	map(0x500011, 0x500011).w(FUNC(magspeed_state::seta_coin_counter_w));       // Coin Counter (no lockout)
	map(0x500015, 0x500015).w(FUNC(magspeed_state::seta_vregs_w));              // Video Registers
	map(0x500018, 0x500019).w(FUNC(magspeed_state::ipl1_ack_w));                // lev 2 irq ack?
	map(0x50001c, 0x50001d).w(FUNC(magspeed_state::ipl2_ack_w));                // lev 4 irq ack?
	map(0x600000, 0x600005).w(FUNC(magspeed_state::lights_w));                  // Lights
	map(0x600006, 0x600007).nopw();                                             // ?
	map(0x700000, 0x7003ff).ram();                                              // Palette RAM (tested)
	map(0x700400, 0x700fff).ram().share("paletteram1");                         // Palette
	map(0x701000, 0x703fff).ram();                                              // Palette RAM (tested)
	map(0x800000, 0x803fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0x804000, 0x807fff).ram();                                              // tested
	map(0x880000, 0x883fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2&3
	map(0x884000, 0x887fff).ram();                                              // tested
	map(0x900000, 0x900005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 0&1 Ctrl
	map(0x980000, 0x980005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 2&3 Ctrl
	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xa80000, 0xa80001).ram();                                              // ? $4000
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xb04000, 0xb07fff).ram();                                              // tested
	map(0xc00000, 0xc00007).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0x00ff);
	map(0xd00000, 0xd03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
}


/***************************************************************************
                                Krazy Bowl
***************************************************************************/

void seta_state::krzybowl_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                             // ROM
	map(0xf00000, 0xf0ffff).ram();                             // RAM
	map(0x100000, 0x100001).nopr();                         // ?
	map(0x200000, 0x200001).nopr();                         // ?
	map(0x300000, 0x300003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x400000, 0x400001).nopw();                        // ?
	map(0x500000, 0x500001).portr("P1");                 // P1
	map(0x500002, 0x500003).portr("P2");                 // P2
	map(0x500004, 0x500005).portr("COINS");              // Coins
	map(0x600000, 0x600007).r("upd1", FUNC(upd4701_device::read_xy)).umask16(0x00ff); // P1 trackball
	map(0x600008, 0x60000f).r("upd2", FUNC(upd4701_device::read_xy)).umask16(0x00ff); // P2 trackball
	map(0x8000f0, 0x8000f1).ram();                             // NVRAM
	map(0x800100, 0x8001ff).ram();                             // NVRAM
	map(0xa00000, 0xa03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0xb00000, 0xb003ff).ram().share("paletteram1");  // Palette
	map(0xc00000, 0xc03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xd00000, 0xd00001).ram();                             // ? 0x4000
	map(0xe00000, 0xe005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xe00600, 0xe00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
}


/***************************************************************************
                            Mobile Suit Gundam
***************************************************************************/

// Mirror RAM is necessary or startup, to clear Work RAM after the test

void seta_state::msgundam_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                             // ROM
	map(0x100000, 0x1fffff).rom();                             // ROM
	map(0x200000, 0x20ffff).ram().mirror(0x70000);          // RAM
	map(0x400000, 0x400001).portr("P1");                 // P1
	map(0x400002, 0x400003).portr("P2");                 // P2
	map(0x400004, 0x400005).portr("COINS");              // Coins
	map(0x400000, 0x400001).w(FUNC(seta_state::ipl1_ack_w));               // Lev 2 IRQ Ack
	map(0x400004, 0x400005).w(FUNC(seta_state::ipl2_ack_w));               // Lev 4 IRQ Ack
	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_lockout_w));       // Coin Lockout
	map(0x500002, 0x500003).nopw();                                               // ?
	map(0x500005, 0x500005).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x600000, 0x600003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x700400, 0x700fff).ram().share("paletteram1");  // Palette
	map(0x800000, 0x8005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0x800600, 0x800607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0x880000, 0x880001).ram();                             // ? 0x4000
	map(0x900000, 0x903fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xa00000, 0xa03fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0xa80000, 0xa83fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2&3
	map(0xb00000, 0xb00005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 0&1 Ctrl
	map(0xb80000, 0xb80005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 2&3 Ctrl
	map(0xc00000, 0xc03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0xd00000, 0xd00007).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0x00ff);
}

void seta_state::msgundamb_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                             // ROM
	map(0x100000, 0x1fffff).rom();                             // ROM
	map(0x200000, 0x20ffff).ram();          // RAM
	map(0x400000, 0x400001).portr("P1");                 // P1
	map(0x400002, 0x400003).portr("P2");                 // P2
	map(0x400004, 0x400005).portr("COINS");              // Coins
	map(0x400000, 0x400001).w(FUNC(seta_state::ipl1_ack_w));               // Lev 2 IRQ Ack
	map(0x400004, 0x400005).w(FUNC(seta_state::ipl2_ack_w));               // Lev 4 IRQ Ack
	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_counter_w));
	map(0x500003, 0x500003).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x500004, 0x500005).nopw();
	map(0x600000, 0x600003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x700400, 0x700fff).ram().share("paletteram1");  // Palette
	map(0x800000, 0x803fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0x880000, 0x883fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2&3
	map(0x900000, 0x900005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 0&1 Ctrl
	map(0x980000, 0x980005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 2&3 Ctrl
	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xa80000, 0xa80001).ram();                             // ? 0x4000
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xc00000, 0xc03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0xd00000, 0xd00007).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0x00ff);
}

/***************************************************************************
                                Oishii Puzzle
***************************************************************************/

// similar to wrofaero
void seta_state::oisipuzl_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                             // ROM
	map(0x100000, 0x17ffff).rom();                             // ROM
	map(0x200000, 0x20ffff).ram();                             // RAM
	map(0x300000, 0x300003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x400000, 0x400001).portr("P1");                 // P1
	map(0x400002, 0x400003).portr("P2");                 // P2
	map(0x400004, 0x400005).portr("COINS");              // Coins
	map(0x400000, 0x400001).nopw();                        // ? IRQ Ack
	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_lockout_w));       // Coin Lockout
	map(0x500003, 0x500003).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x500004, 0x500005).nopw();                        // ? IRQ Ack
	map(0x700000, 0x703fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0x800000, 0x803fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0x880000, 0x883fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2&3
	map(0x900000, 0x900005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 0&1 Ctrl
	map(0x980000, 0x980005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 2&3 Ctrl
	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xa80000, 0xa80001).ram();                             // ? 0x4000
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xc00400, 0xc00fff).ram().share("paletteram1");  // Palette
}


/***************************************************************************
                                Triple Fun
***************************************************************************/

// Same as oisipuzl but with the sound system replaced

void seta_state::triplfun_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                             // ROM
	map(0x100000, 0x17ffff).rom();                             // ROM
	map(0x200000, 0x20ffff).ram();                             // RAM
	map(0x300000, 0x300003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x400000, 0x400001).portr("P1");                 // P1
	map(0x400002, 0x400003).portr("P2");                 // P2
	map(0x400004, 0x400005).portr("COINS");              // Coins
	map(0x400000, 0x400001).nopw();                        // ? IRQ Ack
	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_lockout_w));       // Coin Lockout
	map(0x500003, 0x500003).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x500004, 0x500005).nopw();                        // ? IRQ Ack
	map(0x500007, 0x500007).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // tfun sound
	map(0x700000, 0x703fff).noprw();
	map(0x800000, 0x803fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0x880000, 0x883fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2&3
	map(0x900000, 0x900005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 0&1 Ctrl
	map(0x980000, 0x980005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w));     // VRAM 2&3 Ctrl
	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xa80000, 0xa80001).ram();                             // ? 0x4000
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xc00400, 0xc00fff).ram().share("paletteram1");  // Palette
}


/***************************************************************************
                        Thunder & Lightning
***************************************************************************/

/* Protection only present in thunderl set.
   Implemented using a registered PALCE16V8H: TL-9. Main CPU performs
   several writtings to the address space mapped to the PAL to save a value
   into PAL registers. Eventually, CPU reads back that value and performs
   some checkings.
   If the value is not the proper one, a soft reset is done.
   Address during writting operation is mapped in the following way:

   A2  -> I2
   A3  -> I3
   A6  -> I4
   A8  -> I5
   A11 -> I6
   A13 -> I7
   A15 -> I8
   A16 -> I9

   Data sent by the CPU during these writting operations is not used to
   calculate the protection register value, only some address lines are used
   to compute the value, as shown above.
   Every write operation done to the PAL discards the previous stored value
   in the registers and stores a new computed value, following the logic
   equations programmed in the PAL.

   (I1 = CLK : pulses when accessing to writting handler and acts as clock
   for internal latches in PAL)
   (I11 = /OE : asserted when accessing to reading handler. Sub-address
   used for reading here is no used to compute the result value

   I19  -> D0
   I18  -> D1
   I17  -> D2
   I16  -> D3
   I15  -> D4
   I14  -> D5
   I13  -> D6
   I12  -> D7
*/
u16 thunderl_state::thunderl_protection_r()
{
//  logerror("PC %06X - Protection Read\n", m_maincpu->pc());

	return m_thunderl_protection_reg;
}
void thunderl_state::thunderl_protection_w(offs_t offset, u16 data)
{
	// data byte written here is not used to save the value into protection registers
	const u32 addr = offset * 2;

	m_thunderl_protection_reg =
		(BIT(addr, 2) << 0)
		| ((BIT(addr, 2) & BIT(~addr, 3)) << 1)
		| ((BIT(addr, 2) | BIT(~addr, 6)) << 2)
		| ((BIT(addr, 2) | BIT(~addr, 6) | BIT(~addr, 8)) << 3)
		| ((BIT(addr, 3) & BIT(~addr, 11) & BIT(addr, 15)) << 4)
		| ((BIT(addr, 6) & BIT(addr, 13)) << 5)
		| (((BIT(addr, 6) & BIT(addr, 13)) | BIT(~addr, 16)) << 6)
		| ((((BIT(addr, 6) & BIT(addr, 13)) | BIT(~addr, 16)) & (BIT(addr, 2) | BIT(~addr, 6) | BIT(~addr, 8))) << 7);

	//  logerror("PC %06X - Protection Written: %04X <- %04X\n", m_maincpu->pc(), addr, data);
}

// Similar to downtown etc.

void thunderl_state::thunderl_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();                       // ROM
	map(0xffc000, 0xffffff).ram();                       // RAM
	map(0x100000, 0x103fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0x200000, 0x200001).rw(FUNC(thunderl_state::ipl1_ack_r), FUNC(thunderl_state::ipl1_ack_w));
	map(0x300000, 0x300001).nopw();                      // ?
	map(0x400000, 0x41ffff).w(FUNC(thunderl_state::thunderl_protection_w)); // Protection
	map(0x500001, 0x500001).w(FUNC(thunderl_state::seta_coin_lockout_w));   // Coin Lockout
	map(0x600000, 0x600003).r(FUNC(thunderl_state::seta_dsw_r));            // DSW
	map(0x700000, 0x7003ff).ram().share("paletteram1");  // Palette
	map(0xb00000, 0xb00001).portr("P1");                 // P1
	map(0xb00002, 0xb00003).portr("P2");                 // P2
	map(0xb00004, 0xb00005).portr("COINS");              // Coins
	map(0xb0000c, 0xb0000d).r(FUNC(thunderl_state::thunderl_protection_r)); // Protection
	map(0xc00000, 0xc00001).ram();                       // ? 0x4000
	map(0xd00000, 0xd005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xd00600, 0xd00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xe00000, 0xe03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
}


void thunderl_state::thunderlbl_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();                       // ROM
	map(0xffc000, 0xffffff).ram();                       // RAM
//  map(0x100000, 0x103fff).rw("x1snd", FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));  // Sound
	map(0x200000, 0x200001).rw(FUNC(thunderl_state::ipl1_ack_r), FUNC(thunderl_state::ipl1_ack_w));
	map(0x300000, 0x300001).nopw();                      // ?
	map(0x500001, 0x500001).w(FUNC(thunderl_state::seta_coin_lockout_w));       // Coin Lockout
	map(0x600000, 0x600003).r(FUNC(thunderl_state::seta_dsw_r));                // DSW
	map(0x700000, 0x7003ff).ram().share("paletteram1");  // Palette
	map(0xb00000, 0xb00001).portr("P1");                 // P1
	map(0xb00002, 0xb00003).portr("P2");                 // P2
	map(0xb00004, 0xb00005).portr("COINS");              // Coins
	map(0xb0000c, 0xb0000d).w(m_spritegen, FUNC(x1_001_device::spritectrl_w8)).umask16(0xff00); // the bootleg is modified to write the first byte of spritectrl here, rather than the usual address
	map(0xb00008, 0xb00008).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xc00000, 0xc00001).ram();                       // ? 0x4000
	map(0xd00000, 0xd005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xd00600, 0xd00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xe00000, 0xe03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
}


/***************************************************************************
                    Wit's
***************************************************************************/
// Similar to thunderl but without protection

void seta_state::wits_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();                       // ROM
	map(0xffc000, 0xffffff).ram();                       // RAM
	map(0x100000, 0x103fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0x200000, 0x200001).rw(FUNC(seta_state::ipl1_ack_r), FUNC(seta_state::ipl1_ack_w));
	map(0x300000, 0x300001).nopw();                      // ?
	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_lockout_w));       // Coin Lockout
	map(0x600000, 0x600003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x700000, 0x7003ff).ram().share("paletteram1");  // Palette
	map(0xb00000, 0xb00001).portr("P1");                 // P1
	map(0xb00002, 0xb00003).portr("P2");                 // P2
	map(0xb00004, 0xb00005).portr("COINS");              // Coins
	map(0xb00008, 0xb00009).portr("P3");                 // P3 (wits)
	map(0xb0000a, 0xb0000b).portr("P4");                 // P4 (wits)
	map(0xc00000, 0xc00001).ram();                       // ? 0x4000
	map(0xd00000, 0xd005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xd00600, 0xd00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xe00000, 0xe03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xe04000, 0xe07fff).ram();
}


/***************************************************************************
                    Wiggie Waggie
***************************************************************************/

void seta_state::wiggie_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();                       // ROM
	map(0xffc000, 0xffffff).ram();                       // RAM
	map(0x100000, 0x103fff).noprw();                     // X1_010 is not used
	map(0x200000, 0x200001).rw(FUNC(seta_state::ipl1_ack_r), FUNC(seta_state::ipl1_ack_w));
	map(0x300000, 0x300001).nopw();                      // ?
	map(0x400000, 0x41ffff).nopw();                      // Protection (but not used, taken from thunderl code)
	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_lockout_w));       // Coin Lockout
	map(0x600000, 0x600003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x700000, 0x7003ff).ram().share("paletteram1");  // Palette
	map(0xb00000, 0xb00001).portr("P1");                 // P1
	map(0xb00002, 0xb00003).portr("P2");                 // P2
	map(0xb00004, 0xb00005).portr("COINS");              // Coins
	map(0xb0000c, 0xb0000d).nopw();                      // Protection (but not used, taken from thunderl code)
	map(0xb00008, 0xb00008).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xc00000, 0xc00001).ram();                       // ? 0x4000
	map(0xd00000, 0xd005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xd00600, 0xd00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xe00000, 0xe03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
}

void seta_state::wiggie_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9800, 0x9800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}


/***************************************************************************
                    Ultraman Club / SD Gundam Neo Battling
***************************************************************************/

void seta_state::umanclub_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                             // ROM
	map(0x200000, 0x20ffff).ram();                             // RAM
	map(0x300000, 0x3003ff).ram().share("paletteram1");  // Palette
	map(0x300400, 0x300fff).ram();                             //
	map(0x400000, 0x400001).portr("P1");                 // P1
	map(0x400002, 0x400003).portr("P2");                 // P2
	map(0x400004, 0x400005).portr("COINS");              // Coins
	map(0x400000, 0x400001).nopw();                        // ? (end of lev 2)
	map(0x400004, 0x400005).nopw();                        // ? (end of lev 2)
	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_lockout_w));       // Coin Lockout
	map(0x600000, 0x600003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xa80000, 0xa80001).ram();                             // ? 0x4000
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xc00000, 0xc03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
}


/***************************************************************************
                            Ultra Toukond Densetsu
***************************************************************************/

void seta_state::utoukond_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                             // ROM
	map(0x200000, 0x20ffff).ram();                             // RAM
	map(0x400000, 0x400001).portr("P1");                 // P1
	map(0x400002, 0x400003).portr("P2");                 // P2
	map(0x400004, 0x400005).portr("COINS");              // Coins
	map(0x500001, 0x500001).w(FUNC(seta_state::seta_coin_lockout_w));       // Coin Lockout
	map(0x500003, 0x500003).w(FUNC(seta_state::seta_vregs_w));              // Video Registers
	map(0x600000, 0x600003).r(FUNC(seta_state::seta_dsw_r));                // DSW
	map(0x700400, 0x700fff).ram().share("paletteram1");  // Palette
	map(0x800000, 0x803fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0x880000, 0x883fff).ram().w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2&3
	map(0x900000, 0x900005).w(m_layers[0], FUNC(x1_012_device::vctrl_w));// VRAM 0&1 Ctrl
	map(0x980000, 0x980005).w(m_layers[1], FUNC(x1_012_device::vctrl_w));// VRAM 2&3 Ctrl
	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xc00001, 0xc00001).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xe00000, 0xe00001).nopw();                        // ? ack
}


/***************************************************************************
                                Pairs Love
***************************************************************************/

u16 pairlove_state::prot_r(offs_t offset)
{
	const u16 retdata = m_protram[offset];
	//osd_printf_debug("pairs love protection? read %06x %04x %04x\n", m_maincpu->pc(), offset, retdata);
	if (!machine().side_effects_disabled())
		m_protram[offset] = m_protram_old[offset];
	return retdata;
}

void pairlove_state::prot_w(offs_t offset, u16 data)
{
	//osd_printf_debug("pairs love protection? write %06x %04x %04x\n", m_maincpu->pc(), offset, data);
	m_protram_old[offset] = m_protram[offset];
	m_protram[offset] = data;
}

void pairlove_state::pairlove_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                                          // ROM
	map(0x100000, 0x100001).nopw();                                         // ? 1 (start of interrupts, main loop: watchdog?)
	map(0x200000, 0x200001).nopw();                                         // ? 0/1 (IRQ acknowledge?)
	map(0x300000, 0x300003).r(FUNC(pairlove_state::seta_dsw_r));            // DSW
	map(0x400001, 0x400001).w(FUNC(pairlove_state::seta_coin_lockout_w));   // Coin Lockout + Sound Enable (bit 4?)
	map(0x500000, 0x500001).portr("P1");                                    // P1
	map(0x500002, 0x500003).portr("P2");                                    // P2
	map(0x500004, 0x500005).portr("COINS");                                 // Coins
	map(0x900000, 0x9001ff).rw(FUNC(pairlove_state::prot_r), FUNC(pairlove_state::prot_w));
	map(0xa00000, 0xa03fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0xb00000, 0xb00fff).ram().share("paletteram1");                     // Palette
	map(0xc00000, 0xc03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     // Sprites Code + X + Attr
	map(0xd00000, 0xd00001).ram();                                          // ? 0x4000
	map(0xe00000, 0xe005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     // Sprites Y
	map(0xe00600, 0xe00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xf00000, 0xf0ffff).ram();                                          // RAM
}

void pairlove_state::machine_start()
{
	seta_state::machine_start();

	m_protram = make_unique_clear<u16 []>(0x200/2);
	m_protram_old = make_unique_clear<u16 []>(0x200/2);

	save_pointer(NAME(m_protram), 0x200/2);
	save_pointer(NAME(m_protram_old), 0x200/2);
}


/***************************************************************************
                            Crazy Fight
***************************************************************************/

void crazyfgt_state::coin_counter_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
}

void crazyfgt_state::outputs_w(u8 data)
{
	// TODO: lower bits also used (for palette banking effect???)
	m_eeprom->data_w(!BIT(data, 5));
}

void crazyfgt_state::crazyfgt_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x400000, 0x40ffff).ram();
	map(0x610000, 0x610001).portr("COINS");
	map(0x610002, 0x610003).portr("UNK");
	map(0x610004, 0x610005).portr("INPUT");
	map(0x610006, 0x610007).nopw();
	map(0x620001, 0x620001).w(FUNC(crazyfgt_state::coin_counter_w));
	map(0x620003, 0x620003).w(FUNC(crazyfgt_state::outputs_w));
	map(0x630000, 0x630003).r(FUNC(crazyfgt_state::seta_dsw_r));
	map(0x640400, 0x640fff).writeonly().share("paletteram1");    // Palette
	map(0x650000, 0x650003).w("ymsnd", FUNC(ym3812_device::write)).umask16(0x00ff);
	map(0x658001, 0x658001).w("oki", FUNC(okim6295_device::write));
	map(0x670000, 0x670001).r(FUNC(crazyfgt_state::ipl0_ack_r));
	map(0x800000, 0x803fff).w(m_layers[1], FUNC(x1_012_device::vram_w)).share("layer2"); // VRAM 2
	map(0x880000, 0x883fff).w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0
	map(0x900000, 0x900005).rw(m_layers[1], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w)); // VRAM 2&3 Ctrl
	map(0x980000, 0x980005).rw(m_layers[0], FUNC(x1_012_device::vctrl_r), FUNC(x1_012_device::vctrl_w)); // VRAM 0&1 Ctrl
	map(0xa00000, 0xa005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16)); // Sprites Y
	map(0xa00600, 0xa00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0xa80000, 0xa80000).w(m_spritegen, FUNC(x1_001_device::spritebgflag_w8));    // ? 0x4000
	map(0xb00000, 0xb03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16)); // Sprites Code + X + Attr
}


/***************************************************************************
                                 Jockey Club
***************************************************************************/

// RTC (To do: write a D4911C device)
u16 jockeyc_state::rtc_r(offs_t offset)
{
	if (offset >= 7)
		++offset;
	if (offset / 2 >= 7)
		return 0;
	return (m_rtc->read(offset / 2) >> ((offset & 1) * 4)) & 0xf;
}

void jockeyc_state::rtc_w(u16 data)
{
}

// Outputs
void jockeyc_state::show_outputs()
{
#ifdef MAME_DEBUG
	popmessage("Mux: %04X Out: %04X", m_mux & (~0xf8), m_out);
#endif
}

u16 jockeyc_state::mux_r()
{
	switch (m_mux & 0xf8)
	{
		case 0x08:  return (m_key2[0]->read() << 8) | m_key1[0]->read();
		case 0x10:  return (m_key2[1]->read() << 8) | m_key1[1]->read();
		case 0x20:  return (m_key2[2]->read() << 8) | m_key1[2]->read();
		case 0x40:  return (m_key2[3]->read() << 8) | m_key1[3]->read();
		case 0x80:  return (m_key2[4]->read() << 8) | m_key1[4]->read();
	}
	logerror("%06X: unknown key read, mux = %04x\n", m_maincpu->pc(), m_mux);
	return 0xffff;
}

void jockeyc_state::jockeyc_mux_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_mux);

	// 0x8000 lamp 5  (p1 cancel)
	// 0x4000 lamp 4  (p2 payout)
	// 0x2000 lamp 3  (p1 payout)
	// 0x1000 lamp 2
	// 0x0800 lamp 1
	// 0x0400 p2 divider
	// 0x0200 hopper 1 motor
	// 0x0100 hopper 2 motor / switch hopper output to p2 (single hopper mode)
	// 0x00f8 key mux
	// 0x0004 p1 divider
	// 0x0002 hopper 2 motor / switch hopper output to p1 (single hopper mode)
	// 0x0001 hopper 1 motor

	m_out_cancel[0] = BIT(data, 15);
	m_out_payout[1] = BIT(data, 14);
	m_out_payout[0] = BIT(data, 13);

	update_hoppers();
	show_outputs();
}

void jockeyc_state::jockeyc_out_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_out);

	// 0x8000 lamp 8  (p2 start)
	// 0x4000 lamp 7  (p1 start)
	// 0x2000 meter 6 (coin 2/4)
	// 0x1000 meter 5 (p1 hopper coin out)
	// 0x0800 meter 4
	// 0x0400 meter 3
	// 0x0200 meter 2 (coin 1/3)
	// 0x0100 meter 1
	// 0x0080 ? always set, save for "backup memory is wrong" screen and ram test
	// 0x0040
	// 0x0020 lamp 6  (p2 cancel)
	// 0x0010 call attendant
	// 0x0008 p2 hopper lockout
	// 0x0004 p1 hopper lockout
	// 0x0002
	// 0x0001

	m_out_start[1] = BIT(data, 15);
	m_out_start[0] = BIT(data, 14);
	m_out_cancel[1] = BIT(data, 5);

	machine().bookkeeping().coin_counter_w(6, data  & 0x2000); // coin 2/4
	machine().bookkeeping().coin_counter_w(5, data  & 0x1000); // p1 hopper coin out
	machine().bookkeeping().coin_counter_w(2, data  & 0x0200); // coin 1/3

	update_hoppers();
	show_outputs();
}

void jockeyc_state::update_hoppers()
{
	if (!m_cabinet)
		return;

	if (m_cabinet->read() & 1)
	{
		// double hoppers
		m_hopper1->motor_w((m_mux & 0x0201) && !(m_out & 0x0004));
		m_hopper2->motor_w((m_mux & 0x0102) && !(m_out & 0x0008));
	}
	else
	{
		// single hopper (jockeyc: in test mode, use key 5/6 to select pay1/pay2)
		m_hopper1->motor_w((m_mux & 0x0201) && (!(m_out & 0x0004) || !(m_out & 0x0008)));
	}
}

u16 jockeyc_state::dsw_r(offs_t offset)
{
	const int shift = offset * 4;
	return  ((((m_dsw1->read()   >> shift)     & 0xf)) << 0) |
			((((m_dsw2_3->read() >> shift)     & 0xf)) << 4) |
			((((m_dsw2_3->read() >> (shift+8)) & 0xf)) << 8) ;
}

u16 jockeyc_state::comm_r()
{
	return 0xffff;//machine().rand();
}

/*
  There is a hidden editor activated by writing 7 in $ffd268.b (i.e. the main loop routine).
  This would be triggered at AA58 when pressing "Special Test". But the latter routine is not called in the released code.
  The editor is comprised of 7 screens operated with a trackball and two buttons.

  Another note... on Christmas day the attract loop includes a "Merry Xmas" screen ($ffd268.b = 3)
*/
#define JOCKEYC_HIDDEN_EDITOR 0

u16 jockeyc_state::trackball_r(offs_t offset)
{
	switch (offset)
	{
		case 0/2:   return (m_p1x->read() >> 0) & 0xff;
		case 2/2:   return (m_p1x->read() >> 8) & 0xff;
		case 4/2:   return (m_p1y->read() >> 0) & 0xff;
		case 6/2:   return (m_p1y->read() >> 8) & 0xff;
	}
	return 0;
}

void jockeyc_state::jockeyc_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom(); // ROM (up to 2MB)

	map(0x200000, 0x200001).rw(FUNC(jockeyc_state::mux_r), FUNC(jockeyc_state::jockeyc_mux_w));
	map(0x200002, 0x200003).portr("COIN");
	map(0x200010, 0x200011).portr("SERVICE").w(FUNC(jockeyc_state::jockeyc_out_w));

	map(0x300000, 0x300001).w(m_watchdog, FUNC(watchdog_timer_device::reset16_w));
	map(0x300002, 0x300003).noprw(); // clr.l $300000 (watchdog)

	map(0x300010, 0x300011).nopw();    // lev1 ack
	map(0x300020, 0x300021).nopw();    // lev2 ack
	map(0x300040, 0x300041).nopw();    // lev4 ack
	map(0x300060, 0x300061).nopw();    // lev6 ack

	map(0x400000, 0x400007).r(FUNC(jockeyc_state::trackball_r));
#if !JOCKEYC_HIDDEN_EDITOR
	map(0x400000, 0x400007).nopr();
#endif

	map(0x500000, 0x500003).r(FUNC(jockeyc_state::dsw_r)); // DSW x 3
	map(0x600000, 0x600001).r(FUNC(jockeyc_state::comm_r)); // comm data
	map(0x600002, 0x600003).r(FUNC(jockeyc_state::comm_r)); // comm status (bits 0,4,5,6)

	map(0x800000, 0x80001f).rw(FUNC(jockeyc_state::rtc_r), FUNC(jockeyc_state::rtc_w));

	map(0x900000, 0x903fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));  // Sound

	map(0xa00000, 0xa00005).w(m_layers[0], FUNC(x1_012_device::vctrl_w));   // VRAM 0&1 Ctrl
	map(0xb00000, 0xb03fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1
	map(0xb04000, 0xb0ffff).nopw(); // likely left-over

	map(0xc00000, 0xc00001).ram();     // ? 0x4000

	map(0xd00000, 0xd005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16)); // Sprites Y
	map(0xd00600, 0xd00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));

	map(0xe00000, 0xe03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16)); // Sprites Code + X + Attr

	map(0xffc000, 0xffffff).ram().share("nvram"); // RAM (battery backed)
}


/***************************************************************************
                             International Toote
***************************************************************************/

// Same as Jockey Club but with additional protection

void jockeyc_state::inttoote_mux_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_mux);

	// 0x8000 lamp?
	// 0x1000 lamp (help button)
	// 0x0800 lamp (start button)

	m_out_help = BIT(data, 12);
	m_out_itstart = BIT(data, 11);

	update_hoppers();
	show_outputs();
}

void jockeyc_state::inttoote_out_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_out);

	// 0x2000 meter (key in)
	// 0x1000 meter (coin out)
	// 0x0800 meter (coin in)
	// 0x0100 meter (key out)
	// 0x0080 ? set when there are credits

	machine().bookkeeping().coin_counter_w(0, data  & 0x2000); // key in
	machine().bookkeeping().coin_counter_w(1, data  & 0x1000); // coin out
	machine().bookkeeping().coin_counter_w(2, data  & 0x0800); // coin in
	machine().bookkeeping().coin_counter_w(3, data  & 0x0100); // key out

	update_hoppers();
	show_outputs();
}

u16 jockeyc_state::inttoote_700000_r(offs_t offset)
{
	return m_inttoote_700000[offset] & 0x3f;
}

void jockeyc_state::inttoote_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom(); // ROM (up to 2MB)

	map(0x200000, 0x200001).rw(FUNC(jockeyc_state::mux_r), FUNC(jockeyc_state::inttoote_mux_w));
	map(0x200002, 0x200003).portr("COIN");
	map(0x200010, 0x200011).portr("SERVICE").w(FUNC(jockeyc_state::inttoote_out_w));

	map(0x300000, 0x300001).w(m_watchdog, FUNC(watchdog_timer_device::reset16_w));

	map(0x300010, 0x300011).nopw();    // lev1 ack
	map(0x300020, 0x300021).nopw();    // lev2 ack
	map(0x300040, 0x300041).nopw();    // lev4 ack
	map(0x300060, 0x300061).nopw();    // lev6 ack

	map(0x500000, 0x500003).r(FUNC(jockeyc_state::dsw_r)); // DSW x 3

	map(0x700000, 0x700101).ram().r(FUNC(jockeyc_state::inttoote_700000_r)).share("inttoote_700000");

	map(0x800000, 0x80001f).rw(FUNC(jockeyc_state::rtc_r), FUNC(jockeyc_state::rtc_w));

	map(0x900000, 0x903fff).rw(m_x1snd, FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound

	map(0xa00000, 0xa00005).w(m_layers[0], FUNC(x1_012_device::vctrl_w));   // VRAM 0&1 Ctrl
	map(0xb00000, 0xb03fff).ram().w(m_layers[0], FUNC(x1_012_device::vram_w)).share("layer1"); // VRAM 0&1

	map(0xc00000, 0xc00001).ram();     // ? 0x4000

	map(0xd00000, 0xd005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16)); // Sprites Y
	map(0xd00600, 0xd00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));

	map(0xe00000, 0xe03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16)); // Sprites Code + X + Attr

	map(0xffc000, 0xffffff).ram().share("nvram"); // RAM (battery backed)
}


/***************************************************************************
                            Ultra Toukon Densetsu
***************************************************************************/

void seta_state::utoukond_sound_control_w(u8 data)
{
	if (!BIT(data, 6))
		m_soundlatch->acknowledge_w();

	// other bits used for banking? (low nibble seems to always be 2)
}

void seta_state::utoukond_sound_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xefff).ram();
	map(0xf000, 0xffff).rw(m_x1snd, FUNC(x1_010_device::read), FUNC(x1_010_device::write));
}

void seta_state::utoukond_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ymsnd", FUNC(ym3438_device::read), FUNC(ym3438_device::write));
	map(0x80, 0x80).w(FUNC(seta_state::utoukond_sound_control_w));
	map(0xc0, 0xc0).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}


/***************************************************************************

                                Input Ports

***************************************************************************/

#define JOY_TYPE1_1BUTTON(_n_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN                        ) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN                        ) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START##_n_                 )

#define JOY_TYPE1_2BUTTONS(_n_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN                        ) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START##_n_                 )

#define JOY_TYPE1_3BUTTONS(_n_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START##_n_                 )


#define JOY_TYPE2_1BUTTON(_n_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN                        ) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN                        ) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START##_n_                 )

#define JOY_TYPE2_2BUTTONS(_n_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN                        ) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START##_n_                 )

#define JOY_TYPE2_3BUTTONS(_n_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START##_n_                 )


#define JOY_ROTATION(_n_, _left_, _right_ ) \
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_PLAYER(_n_) PORT_SENSITIVITY(15) PORT_KEYDELTA(15) PORT_CODE_DEC(KEYCODE_##_left_) PORT_CODE_INC(KEYCODE_##_right_)



/***************************************************************************
                                Athena no Hatena?
***************************************************************************/
/*
        Athena no Hatena is a quiz game that uses only four buttons for inputs.
        However, the hidden "Test Program" menu makes use of the standard
        stick/3-button input layout. With the default input mapping, the menus
        are unusable as the three SHOT buttons are unmapped. So we have two
        input configurations to allow the debug menu to be usable.

        More information about the Test Program menu:
        http://sudden-desu.net/entry/athena-no-hatena-debug-menu-and-functions
*/

static INPUT_PORTS_START( atehate )
	PORT_START("INPUT_TYPE")
	PORT_CONFNAME(0x01,0x00,"Input Type")
	PORT_CONFSETTING(0x00, "Default Control Panel")
	PORT_CONFSETTING(0x01, "Joystick/3 Button Control Panel (for Debug)")

	PORT_START("P1")    // Player 1
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x01)

	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1  )

	PORT_START("P2")    // Player 2
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x01)

	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("INPUT_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2  )

	PORT_START("COINS") // Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // 4 Bits Called "Cut DSW"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSW")   // 2 DSWs - $e00001 & 3.b
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW2:8" ) // Listed as "Unused"

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0200, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x1000, "4" )
	PORT_DIPSETTING(      0x2000, "5" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0xc000, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0000, "20K Only" )
	PORT_DIPSETTING(      0x8000, "20K, Every 30K" )
	PORT_DIPSETTING(      0x4000, "30K, Every 40K" )
INPUT_PORTS_END


/***************************************************************************
                                Blandia
***************************************************************************/

static INPUT_PORTS_START( blandia )
	PORT_START("P1")    //Player 1 - $400000.w
	JOY_TYPE1_3BUTTONS(1)

	PORT_START("P2")    //Player 2 - $400002.w
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS") //Coins - $400004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW")   //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Coinage Type" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0002, "Coin Mode 1" )
	PORT_DIPSETTING(      0x0000, "Coin Mode 2" )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(      0x0080, DEF_STR( 5C_1C ) )        PORT_CONDITION("DSW",0x0002,EQUALS,0x0002)
	PORT_DIPSETTING(      0x0040, DEF_STR( 4C_1C ) )        PORT_CONDITION("DSW",0x0002,EQUALS,0x0002)
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ) )        PORT_CONDITION("DSW",0x0002,EQUALS,0x0002)
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW",0x0002,EQUALS,0x0002)
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x0002,EQUALS,0x0002)
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW",0x0002,EQUALS,0x0002)
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )        PORT_CONDITION("DSW",0x0002,EQUALS,0x0002)
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW",0x0002,NOTEQUALS,0x0002)
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_4C ) )        PORT_CONDITION("DSW",0x0002,NOTEQUALS,0x0002)
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW",0x0002,NOTEQUALS,0x0002)
	PORT_DIPSETTING(      0x0080, "3 Coins/7 Credits" )     PORT_CONDITION("DSW",0x0002,NOTEQUALS,0x0002) // Manuals states "2 Coin 7 Credit"
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_5C ) )        PORT_CONDITION("DSW",0x0002,NOTEQUALS,0x0002)
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_6C ) )        PORT_CONDITION("DSW",0x0002,NOTEQUALS,0x0002)
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )        PORT_CONDITION("DSW",0x0002,NOTEQUALS,0x0002)
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )


	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0200, "1, 1 Round" ) // Test mode shows 1 in both blandia and blandiap
	PORT_DIPSETTING(      0x0300, "1, 2 Rounds" ) // Test mode shows 0 in blandia, 2 in blandiap (neither match actual behaviour)
	PORT_DIPSETTING(      0x0100, "2" ) // Test mode shows 2 in blandia, 3 in blandiap (blandiap test mode is wrong)
	PORT_DIPSETTING(      0x0000, "3" ) // Test mode shows 3 in blandia, 4 in blandiap (blandiap test mode is wrong)
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, "2 Player Game" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, "2 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Credit"  )
	PORT_DIPNAME( 0x2000, 0x2000, "Continue" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x2000, "1 Credit" )
	PORT_DIPSETTING(      0x0000, "1 Coin"   )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END



/***************************************************************************
                                Block Carnival
***************************************************************************/

static INPUT_PORTS_START( blockcar )
	PORT_START("P1")    //Player 1 - $500001.b
	JOY_TYPE1_2BUTTONS(1)   // button2 = speed up

	PORT_START("P2")    //Player 2 - $500003.b
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins + DSW - $500005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_DIPNAME( 0x0010, 0x0000, "Title" )     // This is a jumper pad
	PORT_DIPSETTING(      0x0010, "Thunder & Lightning 2" )
	PORT_DIPSETTING(      0x0000, "Block Carnival" )

	PORT_START("DSW")   //2 DSWs - $300003 & 1.b
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x000c, "20K, Every 50K" )
	PORT_DIPSETTING(      0x0004, "20K, Every 70K" )
	PORT_DIPSETTING(      0x0008, "30K, Every 60K" )
	PORT_DIPSETTING(      0x0000, "30K, Every 90K" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0030, "2" )
	PORT_DIPSETTING(      0x0020, "3" )
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) // Listed as "Unused"
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:8") // Listed as "Unused"
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW1:1" ) // Listed as "Unused"
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Unknown 1-3" ) PORT_DIPLOCATION("SW1:4") // service mode, according to a file in the archive
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_2C ) )
INPUT_PORTS_END



/***************************************************************************
                                Daioh
***************************************************************************/

static INPUT_PORTS_START( daioh )
	PORT_START("P1")
	JOY_TYPE1_3BUTTONS(1)

	PORT_START("P2")
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	// These are NOT Dip Switches but jumpers
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )   // JP9
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )   // JP8
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )   // JP7
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Country" )           // JP6
	PORT_DIPSETTING(      0x0080, "USA (6 buttons)" )
	PORT_DIPSETTING(      0x0000, "Japan (2 buttons)" )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Auto Shot" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0200, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x1000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x2000, "5" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x8000, "300k and every 800k" )
	PORT_DIPSETTING(      0xc000, "500k and every 1000k" )
	PORT_DIPSETTING(      0x4000, "800k and 2000k only" )
	PORT_DIPSETTING(      0x0000, "1000k Only" )

	PORT_START("EXTRA")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                       Daioh (prototype)
***************************************************************************/

static INPUT_PORTS_START( daiohp )
	PORT_START("P1")
	JOY_TYPE1_3BUTTONS(1)

	PORT_START("P2")
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Auto Shot" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0200, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x1000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x2000, "5" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x8000, "100k and every 600k" )
	PORT_DIPSETTING(      0xc000, "200k and every 800k" )
	PORT_DIPSETTING(      0x4000, "300k and 1000k only" )
	PORT_DIPSETTING(      0x0000, "500k Only" )

	PORT_START("COINS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_DIPNAME( 0x00F0, 0x0000, "Country" ) PORT_DIPLOCATION("SW3:1,2,3,4")
	PORT_DIPSETTING(      0x0080, "USA (6 buttons)" ) // any setting other than 0 is USA
	PORT_DIPSETTING(      0x0000, "Japan (2 buttons)" )

	PORT_START("EXTRA")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                       Daioh (prototype, earlier)
***************************************************************************/

static INPUT_PORTS_START( daiohp2 )
	PORT_INCLUDE(daiohp)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x8000, "300k and every 800k" )
	PORT_DIPSETTING(      0xc000, "500k and every 1000k" )
	PORT_DIPSETTING(      0x4000, "800k and 2000k only" )
	PORT_DIPSETTING(      0x0000, "1000k Only" )
INPUT_PORTS_END

/***************************************************************************
                                Dragon Unit
***************************************************************************/

static INPUT_PORTS_START( drgnunit )
	PORT_START("P1")    //Player 1
	JOY_TYPE1_3BUTTONS(1)

	PORT_START("P2")    //Player 2
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_DIPNAME( 0x0010, 0x0010, "Coinage Type" )
	PORT_DIPSETTING(      0x0010, "Coin Mode 1" )
	PORT_DIPSETTING(      0x0000, "Coin Mode 2" )
	PORT_DIPNAME( 0x0020, 0x0020, "Title" )
	PORT_DIPSETTING(      0x0020, "Dragon Unit" )
	PORT_DIPSETTING(      0x0000, "Castle of Dragon" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "(C) / License" )
	PORT_DIPSETTING(      0x00c0, "Athena (Japan)" )
	PORT_DIPSETTING(      0x0080, "Athena / Taito (Japan)" )
	PORT_DIPSETTING(      0x0040, "Seta USA / Taito America" )
	PORT_DIPSETTING(      0x0000, "Seta USA / Romstar" )

	PORT_START("DSW")   //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0003, 0x0002, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0003, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, "150K, Every 300K" )
	PORT_DIPSETTING(      0x000c, "200K, Every 400K" )
	PORT_DIPSETTING(      0x0004, "300K, Every 500K" )
	PORT_DIPSETTING(      0x0000, "400K Only" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0020, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7" )    // Labeled "Don't Touch" in manual
	PORT_DIPNAME( 0x0080, 0x0080, "Stage Time" ) PORT_DIPLOCATION("SW1:8")    // Labeled "Don't Touch" in manual but it seems to work fine
	PORT_DIPSETTING(      0x0080, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Extra 20s" )

	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW2:1" )    // Labeled "Don't Touch" in manual
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "1 of 4 Scenes" )
	PORT_DIPSETTING(      0x0000, "1 of 8 Scenes" )
	PORT_SERVICE_DIPLOC(  0x0800, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )            PORT_CONDITION("COINS",0x0010,EQUALS,0x0010)
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )            PORT_CONDITION("COINS",0x0010,EQUALS,0x0010)
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )            PORT_CONDITION("COINS",0x0010,EQUALS,0x0010)
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )            PORT_CONDITION("COINS",0x0010,EQUALS,0x0010)
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )            PORT_CONDITION("COINS",0x0010,NOTEQUALS,0x0010)
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )            PORT_CONDITION("COINS",0x0010,NOTEQUALS,0x0010)
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )            PORT_CONDITION("COINS",0x0010,NOTEQUALS,0x0010)
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )            PORT_CONDITION("COINS",0x0010,NOTEQUALS,0x0010)
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )            PORT_CONDITION("COINS",0x0010,EQUALS,0x0010)
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_1C ) )            PORT_CONDITION("COINS",0x0010,EQUALS,0x0010)
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )            PORT_CONDITION("COINS",0x0010,EQUALS,0x0010)
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_2C ) )            PORT_CONDITION("COINS",0x0010,EQUALS,0x0010)
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_2C ) )            PORT_CONDITION("COINS",0x0010,NOTEQUALS,0x0010)
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )            PORT_CONDITION("COINS",0x0010,NOTEQUALS,0x0010)
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_4C ) )            PORT_CONDITION("COINS",0x0010,NOTEQUALS,0x0010)
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )            PORT_CONDITION("COINS",0x0010,NOTEQUALS,0x0010)

INPUT_PORTS_END


/***************************************************************************
                                The Roulette
***************************************************************************/

static INPUT_PORTS_START( setaroul )
	PORT_START("DSW1-A") // d40001.b
	PORT_DIPNAME( 0x01, 0x01, "Accept Coins" )          PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Accept Note/Coupon" )    PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPNAME( 0x08, 0x00, "Menu 3:Log 4:RS232" )    PORT_DIPLOCATION("SW1:5") // enable menus 3 & 4 in stats screen
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )

	PORT_START("DSW1-B") // d40003.b
	PORT_DIPNAME( 0x01, 0x00, "Play Jingle?" )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPNAME( 0x02, 0x02, "Use Hopper" )            PORT_DIPLOCATION("SW1:3") // needed for payout
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x04, 0x04, "SW1:2?" )                PORT_DIPLOCATION("SW1:2") // unused? not shown in input test
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "SW1:1?" )                PORT_DIPLOCATION("SW1:1") // unused? not shown in input test
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2-A") // d40005.b
	PORT_DIPNAME( 0x01, 0x00, "Check Door 1?" )         PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPNAME( 0x02, 0x00, "Check Door 2?" )         PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPNAME( 0x04, 0x00, "Check Door 3?" )         PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPNAME( 0x08, 0x08, "SW2:2" )                 PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2-B") // d40007.b
	PORT_DIPNAME( 0x01, 0x01, "SW2:4" )                 PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Menu 5:RS323? 6:Sound 7:Gfx 8:Clock" ) PORT_DIPLOCATION("SW2:3") // enable 4 *hidden* menus and debug key
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPNAME( 0x04, 0x04, "SW2:5" )                 PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x08, 0x08 )                          PORT_DIPLOCATION("SW2:1") // service mode

	PORT_START("COIN") // d40009.b
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r)) // medal (causes hopper over run / empty if the dsw is on)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset")         // rst     (button SW5? Press twice quickly to enter the keyboard test)
	PORT_DIPNAME( 0x04, 0x04, "Credit Meter" )    PORT_DIPLOCATION("SW6:1")  // crt.mtr (switch SW6? Shows stats screen. With added menus, if their dsw is on)
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Attendant Pay") // att.pay (clears error)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3    ) PORT_NAME("Note")          // note    (same as 100 coins)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2    ) PORT_NAME("Coupon")        // cupon   (same as  10 coins)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(setaroul_state::coin_sensors_r))

	PORT_START("COIN1") // start the coin drop sequence (see coin_sensors_r)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(setaroul_state::coin_drop_start), 0)

	PORT_START("DOOR") // d4000b.b
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("Drop") // drop    ("coin drop jam or time out" error when stuck low)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER   )                   // hop.ovf (hopper overflow, ignored?)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("Door 1") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("Door 2") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("Door 3") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT    )

#define PORT_BET(_TAG) \
	PORT_START(_TAG) \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME(_TAG " Row 0") \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME(_TAG " Row 1") \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME(_TAG " Row 2") \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME(_TAG " Row 3") \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME(_TAG " Row 4") \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME(_TAG " Row 5") \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME(_TAG " Row 6") \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME(_TAG " Row 7")

	// d40011.b (26 columns, 1 bit per row)
	PORT_BET("BET.00")
	PORT_BET("BET.01")
	PORT_BET("BET.02")
	PORT_BET("BET.03")
	PORT_BET("BET.04")
	PORT_BET("BET.05")
	PORT_BET("BET.06")
	PORT_BET("BET.07")
	PORT_BET("BET.08")
	PORT_BET("BET.09")
	PORT_BET("BET.0A")
	PORT_BET("BET.0B")
	PORT_BET("BET.0C")
	PORT_BET("BET.0D")
	PORT_BET("BET.0E")
	PORT_BET("BET.0F")
	PORT_BET("BET.10")
	PORT_BET("BET.11")
	PORT_BET("BET.12")
	PORT_BET("BET.13")
	PORT_BET("BET.14")
	PORT_BET("BET.15")
	PORT_BET("BET.16")
	PORT_BET("BET.17")
	PORT_BET("BET.18")
	PORT_BET("BET.19")

	PORT_START("DSW3") // d40019.b
	PORT_DIPNAME( 0x03, 0x03, "Payout %" )             PORT_DIPLOCATION("SW3:7,8")
	PORT_DIPSETTING(    0x00, "65"   )
	PORT_DIPSETTING(    0x01, "75"   )
	PORT_DIPSETTING(    0x02, "85"   )
	PORT_DIPSETTING(    0x03, "97.3" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )     PORT_DIPLOCATION("SW3:5,6")
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x10, 0x10, "Menu 1:Time 2:Payout" ) PORT_DIPLOCATION("SW3:4") // dsw3 4 (enable menus 1 & 2 in stats screen)
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x20, 0x20, "Payout Key" )           PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x40, 0x40, "Hopper Divider" )       PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Hopper Sensor" )        PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x80, "Active Low (Error)"  ) // "Hopper Over Run" error
	PORT_DIPSETTING(    0x00, "Active High" )
INPUT_PORTS_END

static INPUT_PORTS_START( setaroulm )
	PORT_INCLUDE( setaroul )

	PORT_MODIFY("DSW2-B")
	PORT_DIPNAME( 0x01, 0x01, "Show Reels" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************
                                Eight Force
***************************************************************************/

static INPUT_PORTS_START( eightfrc )
	PORT_START("P1")    //Player 1
	JOY_TYPE1_2BUTTONS(1)

	PORT_START("P2")    //Player 2
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")   //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Shared Credits" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Credits To Start" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, "1" )
	PORT_DIPSETTING(      0x0000, "2" )

	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x6000, 0x6000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x4000, "2" )
	PORT_DIPSETTING(      0x6000, "3" )
	PORT_DIPSETTING(      0x2000, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Japanese ) )
INPUT_PORTS_END



/***************************************************************************
                                Extreme Downhill
***************************************************************************/

static INPUT_PORTS_START( extdwnhl )
	PORT_START("P1") //Player 1
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2") //Player 2
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2  )

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE )  // "test"
	// These are NOT Dip Switches but jumpers
	PORT_DIPNAME( 0x0030, 0x0030, "Country" )
	PORT_DIPSETTING(      0x0030, DEF_STR( World ) )
//  PORT_DIPSETTING(      0x0020, DEF_STR( World ) )    // duplicated settings
	PORT_DIPSETTING(      0x0010, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japan ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW") //2 DSWs - $400009 & b.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Continue Coin" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, "Normal: Start 1C / Continue 1C" )
	PORT_DIPSETTING(      0x0000, "Half Continue: Start 2C / Continue 1C" )
	PORT_DIPNAME( 0x8000, 0x8000, "Game Mode" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x8000, "Finals Only" )
	PORT_DIPSETTING(      0x0000, "Semi-Finals & Finals" )
INPUT_PORTS_END



/***************************************************************************
                                Gundhara
***************************************************************************/

static INPUT_PORTS_START( gundhara )
	PORT_START("P1") //Player 1
	JOY_TYPE1_3BUTTONS(1)

	PORT_START("P2") //Player 2
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:7,8") // Yes, the manual shows it takes both switches
	PORT_DIPSETTING(      0x00c0, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0c00, "3" )
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x3000, "200K" )
	PORT_DIPSETTING(      0x2000, "200K, Every 200K" )
	PORT_DIPSETTING(      0x1000, "400K" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

/***************************************************************************
                                Zombie Raid
***************************************************************************/

static INPUT_PORTS_START( zombraid )
	PORT_START("P1") //Player 1
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)  PORT_NAME("P1 Trigger")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)  PORT_NAME("P1 Reload")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2") //Player 2
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)  PORT_NAME("P2 Trigger")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)  PORT_NAME("P2 Reload")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Vertical Screen Flip" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, "Horizontal Screen Flip" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) // Listed as "Unused"
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "2 Coins to Start, 1 to Continue" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW1:8" ) // Listed as "Unused"

	/* The gun calibration defaults to: left=0xc0, right=0x40, top=0x48, bottom=0xa8
	   The user calibrated values are lost each time MAME starts, so the gun always needs to be re-calibrated.
	   Either NVRAM or battery backed up RAM is not emulated.
	   For now it is best to just use a Save State after calibration to remember the setting. */
	PORT_START("GUNX1")   // Player 1 Gun X       ($f00003)
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, -1, 0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)
	PORT_START("GUNY1")   // Player 1 Gun Y       ($f00003)
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y,  1, 0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("GUNX2")   // Player 2 Gun X       ($f00003)
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, -1, 0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(2)
	PORT_START("GUNY2")   // Player 2 Gun Y       ($f00003)
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y,  1, 0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END


/***************************************************************************
                                J.J.Squawkers
***************************************************************************/

static INPUT_PORTS_START( jjsquawk )
	PORT_START("P1") //Player 1 - $400000.w
	JOY_TYPE1_3BUTTONS(1)

	PORT_START("P2") //Player 2 - $400002.w
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS") //Coins - $400004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Unknown 2-8" ) PORT_DIPLOCATION("SW2:8") // ?? screen related
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0200, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x3000, 0x2000, "Energy" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x2000, "2" ) // factory default
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x1000, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x8000, "20K, Every 100K" )   //                          TYPO on manual "20000 200000"
	PORT_DIPSETTING(      0xc000, "50K, Every 200K" )   // manufacturer setting //  TYPO on manual "50000 100000"
	PORT_DIPSETTING(      0x4000, "70K, 200K Only" )
	PORT_DIPSETTING(      0x0000, "100K Only" )
INPUT_PORTS_END

/***************************************************************************
                (Kamen) Masked Riders Club Battle Race
***************************************************************************/

static INPUT_PORTS_START( kamenrid )
	PORT_START("P1") //Player 1
	JOY_TYPE1_2BUTTONS(1)   // BUTTON3 in "test mode" only

	PORT_START("P2") //Player 2
	JOY_TYPE1_2BUTTONS(2)   // BUTTON3 in "test mode" only

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT )
	// These are NOT Dip Switches but jumpers
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Country" )
	PORT_DIPSETTING(      0x0080, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japan ) )

	PORT_START("DSW")   // IN3 - 2 DSWs - $500005 & 7.b
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW , "SW2:8" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:6")   // Manual states "Unused", but masked at 0x001682
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:4")   // Manual states "Unused", but masked at 0x001682
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )                       // (displays debug infos)
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:3")   // Manual states "Unused", but masked at 0x001682
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )                       // (unknown effect at 0x00606a, 0x0060de, 0x00650a)
	PORT_DIPNAME( 0x0040, 0x0040, "Intro Music" )       PORT_DIPLOCATION("SW2:2")   // check code at 0x001792
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(      0x0500, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************
                                Krazy Bowl
***************************************************************************/

#define KRZYBOWL_TRACKBALL(_dir_, _n_ ) \
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_##_dir_ ) PORT_PLAYER(_n_) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_REVERSE

static INPUT_PORTS_START( krzybowl )
	PORT_START("P1") //Player 1
	JOY_TYPE1_3BUTTONS(1)

	PORT_START("P2") //Player 2
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Frames" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, "10" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Trackball ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Joystick ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Force Coinage" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Unknown 2-8" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("TRACK1_X") //Rotation X Player 1
	KRZYBOWL_TRACKBALL(X,1)

	PORT_START("TRACK1_Y") //Rotation Y Player 1
	KRZYBOWL_TRACKBALL(Y,1)

	PORT_START("TRACK2_X") //Rotation X Player 2
	KRZYBOWL_TRACKBALL(X,2) PORT_REVERSE

	PORT_START("TRACK2_Y") //Rotation Y Player 2
	KRZYBOWL_TRACKBALL(Y,2)
INPUT_PORTS_END


/***************************************************************************
                                Mad Shark
***************************************************************************/

static INPUT_PORTS_START( madshark )
	PORT_START("P1") //Player 1
	JOY_TYPE1_2BUTTONS(1)   // BUTTON3 in "test mode" only

	PORT_START("P2") //Player 2
	JOY_TYPE1_2BUTTONS(2)   // BUTTON3 in "test mode" only

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT )
	// Soldered Jumpers on board
	PORT_CONFNAME( 0x0010, 0x0010, DEF_STR( Unused ) )
	PORT_CONFSETTING(      0x0010, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( On ) )
	PORT_CONFNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_CONFSETTING(      0x0020, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( On ) )
	PORT_CONFNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_CONFSETTING(      0x0040, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( On ) )
	// TODO: pinpoint for what market is the Chinese title for
	// (Is our current romset even suited for this setting to properly work? It still says Mad Shark during attract)
	PORT_CONFNAME( 0x0080, 0x0000, "Title Language" )       // Changes title graphics only
	PORT_CONFSETTING(      0x0000, "English" )              // Mad Shark - title used in most of the world, including Japan
	PORT_CONFSETTING(      0x0080, "Traditional Chinese" )  // 最強鮫 - presumably for Taiwan (Zuìqiáng Jiāo) or Hong Kong (Zeoi Koeng Gaau)

	PORT_START("DSW") //2 DSWs
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x0018, "1000k" )
	PORT_DIPSETTING(      0x0008, "1000k 2000k" )
	PORT_DIPSETTING(      0x0010, "1500k 3000k" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ) )
INPUT_PORTS_END


/***************************************************************************
                                Magical Speed
***************************************************************************/

static INPUT_PORTS_START( magspeed )
	PORT_START("P1") // Player 1
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Card 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Card 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Card 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Card 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1  )

	PORT_START("P2") // Player 2
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Card 1") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Card 2") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Card 3") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Card 4") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2  )

	PORT_START("COINS") // Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW")   // 2 DSWs - $500009 & B.b
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0002, "SW2:7" ) // Listed as Unused in the manual
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:6" ) // Listed as Unused in the manual
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:5" ) // Listed as Unused in the manual
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:4" ) // Listed as Unused in the manual
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:3" ) // Listed as Unused in the manual
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(      0x0500, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc000, 0xc000, "Number of Rounds" ) PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(      0x4000, "1" )
	PORT_DIPSETTING(      0xc000, "2" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Unused ) ) // Undefined in the manual
	PORT_DIPSETTING(      0x0000, DEF_STR( Unused ) ) // Undefined in the manual
INPUT_PORTS_END



/***************************************************************************
                            Mobile Suit Gundam
***************************************************************************/


static INPUT_PORTS_START( msgundam )
	PORT_START("P1") //Player 1 - $400000.w
	JOY_TYPE1_2BUTTONS(1)

	PORT_START("P2") //Player 2 - $400002.w
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins - $400004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Language ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japanese ) )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown 2-7" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0600, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:2,3")   // unverified, from the manual
	PORT_DIPSETTING(      0x0400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Unknown 1-4" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Memory Check" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Unknown 1-7" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( msgundam1 )
	PORT_INCLUDE(msgundam)

	PORT_MODIFY("COINS") // IN2 - Coins - $400004.w
	// this set seems to be a Japan set, English mode doesn't work correctly
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Language ) )
//  PORT_DIPSETTING(      0x0080, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japanese ) )
INPUT_PORTS_END


/***************************************************************************
                            Oishii Puzzle
***************************************************************************/

static INPUT_PORTS_START( oisipuzl )
	PORT_START("P1") //Player 1
	JOY_TYPE1_2BUTTONS(1)

	PORT_START("P2") //Player 2
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" ) // Manual States dips 4-7 are unused
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) // Listed as "Unused"
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW1:7" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW1:8" ) // Listed as "Unused"
INPUT_PORTS_END



/***************************************************************************
                                Quiz Kokology
***************************************************************************/

static INPUT_PORTS_START( qzkklogy )
	PORT_START("P1") //Player 1 - $b00001.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 )  PORT_PLAYER(1) PORT_NAME("P1 Pause (Cheat)")// pause (cheat)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1  )

	PORT_START("P2") //Player 2 - $b00003.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 )  PORT_PLAYER(2) PORT_NAME("P2 Pause (Cheat)")// pause (cheat)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2  )

	PORT_START("COINS") //Coins - $b00005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0003, 0x0003, "DS2-1&2*" )      PORT_DIPLOCATION("SW2:1,2") // Manual States dips 1-5 are unused
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPNAME( 0x0004, 0x0004, "Highlight Right Answer (Cheat)") PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DSW2-4" )            PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "DSW2-5" )            PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0020, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:4") // Manual States this dip is unused
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x8000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x4000, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
INPUT_PORTS_END



/***************************************************************************
                                Quiz Kokology 2
***************************************************************************/

static INPUT_PORTS_START( qzkklgy2 )
	PORT_INCLUDE(qzkklogy)

	PORT_MODIFY("P1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0008, 0x0008, "Skip Real DAT Rom Check?" )  PORT_DIPLOCATION("SW2:4") // 'ON' it will pass DAT rom even if it isn't mapped(!)
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

INPUT_PORTS_END

/***************************************************************************
                                    Rezon
***************************************************************************/

static INPUT_PORTS_START( rezon )
	PORT_START("P1") //Player 1
	JOY_TYPE1_3BUTTONS(1)   // 1 used??

	PORT_START("P2") //Player 2
	JOY_TYPE1_3BUTTONS(2)   // 1 used ??

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT1 )
	PORT_CONFNAME( 0x0010, 0x0010, "Licensee" )
	PORT_CONFSETTING(      0x0010, "Allumer" )
	PORT_CONFSETTING(      0x0000, "Taito" )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" ) // Listed as "Unused"
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x0000, "Upright 1 Controller" )
	PORT_DIPSETTING(      0x0018, "Upright 2 Controllers" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Cocktail ) )
//  PORT_DIPSETTING(      0x0010, "10" )                // Unused / Not Defined
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) // Listed as "Unused"
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END

/***************************************************************************
                            SD Gundam Neo Battling
***************************************************************************/

/*
    When the "Stage Select" dip switch is on and button1 is pressed during boot,
    pressing P1's button3 freezes the game (pressing P2's button3 resumes it).
*/
static INPUT_PORTS_START( neobattl )
	PORT_START("P1") // Player 1 - $400000.w
	JOY_TYPE1_1BUTTON(1)    // bump to 3 buttons for freezing to work

	PORT_START("P2") //Player 2 - $400002.w
	JOY_TYPE1_1BUTTON(2)    // bump to 3 buttons for freezing to work

	PORT_START("COINS") //Coins - $400004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:3")  // used but not documented
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Stage Select (Cheat)") PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:5")  // unused?
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:6")  // unused?
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:7")  // unused?
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, 0x0080, "DSW2:8" )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0300, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DSW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy )    ) // "Easy"
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal )  ) // "Normal"
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard )    ) // "Difficult"
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) ) // "Very Difficult"
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW1:5,6,7,8")
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END



/***************************************************************************
                                Sokonuke
***************************************************************************/

static INPUT_PORTS_START( sokonuke )
	PORT_START("P1") //Player 1
	JOY_TYPE1_1BUTTON(1)

	PORT_START("P2") //Player 2
	JOY_TYPE1_1BUTTON(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") //2 DSWs - $400009 & b.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DSW2 Switch 6*" )    PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Cheap Continue" )    PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")   // unused?
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END



/***************************************************************************
                                Strike Gunner
***************************************************************************/

static INPUT_PORTS_START( stg )
	PORT_START("P1") //Player 1 - $b00001.b
	JOY_TYPE1_3BUTTONS(1)

	PORT_START("P2") //Player 2 - $b00003.b
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS") //Coins - $b00005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )
//  PORT_DIPNAME( 0x00f0, 0x00f0, "Title" )
	/* This is the index in a table with pointers to the
	   title logo, but the table is filled with just 1 value */

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy )    ) // 0
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  ) // 4
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard )    ) // 8
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) ) // b
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0010, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0020, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW2:8" )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0400, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW1:4" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW1:8" )
INPUT_PORTS_END



/***************************************************************************
                            Thunder & Lightning
***************************************************************************/

static INPUT_PORTS_START( thunderl )
	PORT_START("P1") //Player 1
	JOY_TYPE1_2BUTTONS(1)   // button2 = speed up

	PORT_START("P2") //Player 2
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins + DSW
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_DIPNAME( 0x0010, 0x0000, "Force 1 Life" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, "Copyright" )
	PORT_DIPSETTING(      0x0080, "Romstar" )
	PORT_DIPSETTING(      0x00c0, "Seta (Romstar License)" )
	PORT_DIPSETTING(      0x00e0, "Seta (Visco License)" )
	PORT_DIPSETTING(      0x00a0, "Visco" )
	PORT_DIPSETTING(      0x0060, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )

	PORT_START("DSW") //2 DSWs - $600003 & 1.b
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x000c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(      0x00c0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x00d0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0090, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(      0x00b0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_4C ) )

	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )      // WEIRD!
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x2000, "3" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( thunderlbl )

	PORT_INCLUDE( thunderl )

	PORT_MODIFY("COINS")
	PORT_DIPNAME( 0x00e0, 0x00e0, "Copyright" )
	PORT_DIPSETTING(      0x0080, DEF_STR( None ) )
	PORT_DIPSETTING(      0x00c0, "Hyogo (Hyogo License)" )
	PORT_DIPSETTING(      0x00e0, "(Hyogo License)" )
	PORT_DIPSETTING(      0x00a0, "Hyogo" )
	PORT_DIPSETTING(      0x0060, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
INPUT_PORTS_END



/***************************************************************************
                                Ultraman Club
***************************************************************************/

static INPUT_PORTS_START( umanclub )
	PORT_START("P1") //Player 1
	JOY_TYPE1_2BUTTONS(1)

	PORT_START("P2") //Player 2
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW") //2 DSWs
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Unknown DSW2 - 3*" )     PORT_DIPLOCATION("SW2:3")   // tested...Manual states "Unused"
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Stage Select (Cheat)")       PORT_DIPLOCATION("SW2:4")   // Manual states "Unused"
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW2:5" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW2:6" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW2:7" ) // Listed as "Unused"
	PORT_SERVICE_DIPLOC(   0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0300, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END


/***************************************************************************
                            Ultra Toukon Densetsu
***************************************************************************/

static INPUT_PORTS_START( utoukond )
	PORT_START("P1") //Player 1
	JOY_TYPE1_3BUTTONS(1)

	PORT_START("P2") //Player 2
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	// These are NOT Dip Switches but jumpers
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW") //2 DSWs
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x00d0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00b0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0090, DEF_STR( 1C_7C ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, "100k" )
	PORT_DIPSETTING(      0x0000, "150k" )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END



/***************************************************************************
                                War of Aero
***************************************************************************/

static INPUT_PORTS_START( wrofaero )
	PORT_START("P1") //Player 1 - $400000.w
	JOY_TYPE1_3BUTTONS(1)   // 3rd button selects the weapon
							// when the dsw for cheating is on

	PORT_START("P2") //Player 2 - $400002.w
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS") //Coins - $400004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Unknown 1-3*" ) PORT_DIPLOCATION("SW1:3")    // tested...Manual states "Unused"
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Stage & Weapon Select (Cheat)") PORT_DIPLOCATION("SW1:4") // P2 Start Is Freeze Screen...Manual states "Unused"
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7" ) // Listed as "Unused"
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END



/***************************************************************************
                                    Wit's
***************************************************************************/

static INPUT_PORTS_START( wits )
	PORT_START("P1") //Player 1
	JOY_TYPE1_2BUTTONS(1)

	PORT_START("P2") //Player 2
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins + DSW
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_DIPNAME( 0x0010, 0x0010, "Unknown 3-4*" )  // Jumpers, I guess
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Unknown 3-5*" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x0040, "License" )
	PORT_DIPSETTING(      0x00c0, "Romstar" )
	PORT_DIPSETTING(      0x0080, "Seta U.S.A" )
	PORT_DIPSETTING(      0x0040, "Visco (Japan Only)" )
	PORT_DIPSETTING(      0x0000, "Athena (Japan Only)" )

	PORT_START("DSW") //2 DSWs - $600003 & 1.b
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, "150k, 350k" )
	PORT_DIPSETTING(      0x000c, "200k, 500k" )
	PORT_DIPSETTING(      0x0004, "300k, 600k" )
	PORT_DIPSETTING(      0x0000, "400k" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0020, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, "Play Mode" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "2 Players" )
	PORT_DIPSETTING(      0x0000, "4 Players" )
	PORT_DIPNAME( 0x0080, 0x0080, "CPU Player During Multi-Player Game" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Yes ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0400, "Every 3rd Loop" )
	PORT_DIPSETTING(      0x0000, "Every 7th Loop" )
	PORT_SERVICE_DIPLOC(  0x0800, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )

	PORT_START("P3") //Player 3
	JOY_TYPE1_2BUTTONS(3)

	PORT_START("P4") //Player 4
	JOY_TYPE1_2BUTTONS(4)
INPUT_PORTS_END


/***************************************************************************
                                Zing Zing Zip
***************************************************************************/

static INPUT_PORTS_START( zingzip )
	PORT_START("P1") //Player 1 - $400000.w
	JOY_TYPE1_2BUTTONS(1)

	PORT_START("P2") //Player 2 - $400002.w
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins - $400004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  ) // no coin 2
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_CONFNAME( 0x0010, 0x0010, "Unknown (bit 4)" )
	PORT_CONFSETTING(      0x0010, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( On ) )
	PORT_CONFNAME( 0x0020, 0x0020, "Unknown (bit 5)" )
	PORT_CONFSETTING(      0x0020, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( On ) )
	PORT_CONFNAME( 0x0040, 0x0040, "Unknown (bit 6)" )
	PORT_CONFSETTING(      0x0040, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( On ) )
	PORT_CONFNAME( 0x0080, 0x0000, "Title Language" )       // Not listed in manual
	PORT_CONFSETTING(      0x0080, "English and Chinese" )
	PORT_CONFSETTING(      0x0000, "English only" )         // Without 真真急炮/Zhēn zhēn jí pào

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" ) // Manual States dips 3-7 are unused
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) // Listed as "Unused"
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END

/*************************************
        Pairs Love
*************************************/

static INPUT_PORTS_START( pairlove )
	PORT_START("P1") //Player 1 - $500001.b
	JOY_TYPE1_2BUTTONS(1)   // button2 = speed up

	PORT_START("P2") //Player 2 - $500003.b
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins + DSW - $500005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )

	PORT_START("DSW")    // 2 DIP switches
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) // Listed as "Unused"
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW1:1" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0200, 0x0200, "SW1:2" ) // Listed as "Unused"
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END

/***************************************************************************
                                Orbs
***************************************************************************/

static INPUT_PORTS_START( orbs )
	PORT_START("P1") //Player 1
	JOY_TYPE1_1BUTTON(1)

	PORT_START("P2") //Player 2 ??
	JOY_TYPE1_1BUTTON(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Stock" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Level_Select ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Timer speed" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Fast" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Force Coinage (Half Coin)" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Unknown 1-8" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************
                  Kero Kero Keroppi no Isshoni Asobou
***************************************************************************/


static INPUT_PORTS_START( keroppi )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Prize Out") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Prize Hopper") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0003, 0x0003, "Obstacle Course Payout Setting" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, "Highest (0, 3, 5, 7, 15 Tickets)" )
	PORT_DIPSETTING(      0x0001, "High (0, 3, 4, 5, 10 Tickets)" )
	PORT_DIPSETTING(      0x0002, "Low (0, 1, 2, 3, 6 Tickets)" )
	PORT_DIPSETTING(      0x0003, "Medium (0, 2, 3, 4, 8 Tickets)" )
	PORT_DIPNAME( 0x000c, 0x000c, "Treasure Hunt Payout Setting" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0000, "Highest (5, 10, 15 Tickets)" )
	PORT_DIPSETTING(      0x0004, "High (5, 7, 10 Tickets)" )
	PORT_DIPSETTING(      0x0008, "Low (1, 3, 5 Tickets)" )
	PORT_DIPSETTING(      0x000c, "Medium (3, 5, 7 Tickets)" )
	PORT_DIPNAME( 0x0030, 0x0030, "New Jersey Payout Setting" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, "Always Payout 5 Tickets" )
	PORT_DIPSETTING(      0x0010, "Always Payout 3 Tickets" )
	PORT_DIPSETTING(      0x0020, "Always Payout 1 Ticket" )
	PORT_DIPSETTING(      0x0030, DEF_STR( Off ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Mercy Ticket Payout Setting (Treasure Hunt Only" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x0000, "No Payout" )
	PORT_DIPSETTING(      0x0040, "3 Tickets" )
	PORT_DIPSETTING(      0x0080, "2 Tickets" )
	PORT_DIPSETTING(      0x00c0, "1 Ticket" )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Vending Style" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, "Ticket" )
	PORT_DIPSETTING(      0x0000, "No Vending" )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x6000, 0x6000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Payout Setting" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x8000, "Normal Payout" )
	PORT_DIPSETTING(      0x0000, "No Payout" )

INPUT_PORTS_END

static INPUT_PORTS_START( keroppij )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Prize Out") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Prize Hopper") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0003, "1" )
	PORT_DIPSETTING(      0x0005, "2" )
	PORT_DIPSETTING(      0x0001, "3" )
	PORT_DIPSETTING(      0x0007, "4" )
	PORT_DIPSETTING(      0x0006, "5" )
	PORT_DIPSETTING(      0x0002, "6" )
	PORT_DIPSETTING(      0x0004, "7" )
	PORT_DIPSETTING(      0x0000, "8" )
	PORT_DIPNAME( 0x0038, 0x0038, "Game Select" ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0038, "No. 1,2,3" )
	PORT_DIPSETTING(      0x0030, "No. 1" )
	PORT_DIPSETTING(      0x0028, "No. 2,3" )
	PORT_DIPSETTING(      0x0020, "No. 3" )
	PORT_DIPSETTING(      0x0018, "No. 1,2" )
	PORT_DIPSETTING(      0x0010, "No. 2" )
	PORT_DIPSETTING(      0x0008, "No. 1,3" )
	PORT_DIPSETTING(      0x0000, "No. 1,2,3" )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown 2-7" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Unknown 2-8" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Unknown 1-4" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Unknown 1-5" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

/***************************************************************************
                                Crazy Fight
***************************************************************************/

static INPUT_PORTS_START( crazyfgt )
	PORT_START("COINS") //Coins - $610000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(ds2430a_device::data_r))

	PORT_START("UNK") //? - $610002.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INPUT") //Player - $610004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Button 2 (top center)")    PORT_CODE(KEYCODE_5_PAD) // JAMMA parts side pin 20
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Button 5 (bottom center)") PORT_CODE(KEYCODE_2_PAD) // JAMMA parts side pin 21
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Button 1 (top left)")      PORT_CODE(KEYCODE_4_PAD) // JAMMA parts side pin 18
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Button 4 (bottom left)")   PORT_CODE(KEYCODE_1_PAD) // JAMMA parts side pin 19
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Button 3 (top right)")     PORT_CODE(KEYCODE_6_PAD) // JAMMA parts side pin 22
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Button 6 (bottom right)")  PORT_CODE(KEYCODE_3_PAD) // JAMMA parts side pin 23
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE1 ) // ticket

	PORT_START("DSW") //2 DSWs - $630001 & 3.b
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x00c0, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x00c0, "5" )
	PORT_DIPSETTING(      0x0080, "10" )
	PORT_DIPSETTING(      0x0040, "15" )
	PORT_DIPSETTING(      0x0000, "20" )

	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Difficulty?" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0c00, "0" )
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0400, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x3000, 0x3000, "Energy" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x1000, "24" )
	PORT_DIPSETTING(      0x2000, "32" )
	PORT_DIPSETTING(      0x3000, "48" )
	PORT_DIPSETTING(      0x0000, "100" )
	PORT_DIPNAME( 0xc000, 0xc000, "Bonus?" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0xc000, "0" )
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0x4000, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
INPUT_PORTS_END

/***************************************************************************
                                 Jockey Club
***************************************************************************/

/*
    Betting Panel         (keys)

    1 1-2 2-3 3-4 4-5 5-6 (1QWERT)
    2 1-3 2-4 3-5 4-6     (2ASDF)
    3 1-4 2-5 3-6         (3ZXC)
    4 1-5 2-6             (4YU)
    5 1-6                 (5H)
    6                     (6)
*/

static INPUT_PORTS_START( jockeyc_keyboards )
	PORT_START("KEY1.0")  // 200000.w (0x08)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY2.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1.1")  // 200000.w (0x10)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Payout")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3  ) PORT_NAME("P1 Credit") // shown in test mode, but seems unused
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Cancel")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY2.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Payout")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START4  ) PORT_NAME("P2 Credit") // shown in test mode, but seems unused
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Cancel")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1.2")  // 200000.w (0x20)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 1-2") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 1-3") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 1-4") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 1-5") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 1-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY2.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 1-2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 1-3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 1-4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 1-5")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 1-6")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1.3")  // 200000.w (0x40)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 2-3") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 2-4") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 2-5") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 2-6") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 3-4") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY2.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 2-3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 2-4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 2-5")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 2-6")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 3-4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1.4")  // 200000.w (0x80)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 3-5") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 3-6") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 4-5") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 4-6") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Bet 5-6") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY2.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 3-5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 3-6")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 4-5")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 4-6")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Bet 5-6")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( jockeyc )
	PORT_INCLUDE( jockeyc_keyboards )

	PORT_START("COIN") // 200002.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_CUSTOM ) // Coin Drop - 1P
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_CUSTOM ) // Hopper Overflow - 1P
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper1", FUNC(ticket_dispenser_device::line_r)) // Hopper Coin Out - 1P
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER   ) // Attendant Pay - 1P
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Coin B - 1P")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Coin A - 1P")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_CUSTOM ) // Coin Sense 2 - 1P
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) // Coin Sense 1 - 1P
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_CUSTOM ) // Coin Drop - 2P
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_CUSTOM ) // Sel Sense (single hopper mode) / Hopper Overflow - 2P (double hopper mode)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper2", FUNC(ticket_dispenser_device::line_r)) // Hopper Coin Out - 2P (double hopper mode)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER   ) // Attendant Pay - 2P
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("Coin B - 2P")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Coin A - 2P")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_CUSTOM ) // Coin Sense 2 - 2P
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_CUSTOM ) // Coin Sense 1 - 2P

	PORT_START("SERVICE") // 200010.w
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR ) PORT_TOGGLE
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER    ) PORT_NAME("Special Test")  PORT_CODE(KEYCODE_F1) // enter Special Screen in test mode
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Call Attendant") // Flips an output bit (lamp?)
	// Electronic key switches, fitted beneath the front panel:
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Last Game Key") PORT_TOGGLE // Test Mode at boot, Last Game during play
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Meter Key")     PORT_TOGGLE
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset Key")     PORT_TOGGLE // reset error condition, e.g. hopper empty error
	PORT_CONFNAME( 0x8000, 0x0000, "Backup Battery"  )
	PORT_CONFSETTING(      0x0000, "OK" )
	PORT_CONFSETTING(      0x8000, "NG" )

	PORT_START("DSW1") // SW1
	PORT_DIPNAME( 0x01, 0x01, "Coin Type" )             PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Coin" )
	PORT_DIPSETTING(    0x00, "Medal" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )                 PORT_DIPLOCATION("SW1:3")
	PORT_DIPNAME( 0x08, 0x00, "Max Jackpot" )           PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "500 coins" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x10, 0x10, "Music During Race" )     PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Coin Divider?" )         PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Hopper" )                PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "Single" )
	PORT_DIPSETTING(    0x00, "Double" )
	PORT_DIPNAME( 0x80, 0x80, "Coin Sensor" )           PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Single" )
	PORT_DIPSETTING(    0x80, "Double" )

	PORT_START("DSW2_3") // SW2 & SW3
	PORT_DIPNAME( 0x0007, 0x0007, "Payout Rate" )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0007, "100%" )
	PORT_DIPSETTING(      0x0006, "95%" )
	PORT_DIPSETTING(      0x0005, "90%" )
	PORT_DIPSETTING(      0x0004, "85%" )
	PORT_DIPSETTING(      0x0003, "80%" )
	PORT_DIPSETTING(      0x0002, "75%" )
	PORT_DIPSETTING(      0x0001, "70%" )
	PORT_DIPSETTING(      0x0000, "65%" )
	PORT_DIPUNKNOWN_DIPLOC(0x0008, 0x0008, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x0010, 0x0010, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x0020, 0x0020, "SW2:6")
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, "1 Coin/10 Credits" )

	PORT_DIPUNKNOWN_DIPLOC(0x0100, 0x0100, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0200, 0x0200, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0400, 0x0400, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0800, 0x0800, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x1000, 0x1000, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x2000, 0x2000, "SW3:6")
	PORT_DIPNAME( 0x4000, 0x4000, "Auto Bet" )          PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Skip Race" )         PORT_DIPLOCATION("SW3:8") // debug? corrupt background
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("CABINET")
	PORT_CONFNAME( 0x01, 0x01, "Fitted Hoppers" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )

#if JOCKEYC_HIDDEN_EDITOR
	PORT_START("P1X") // 400001/3.b (low/high)
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(8)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // dec
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // inc
	PORT_START("P1Y") // 400005/7.b (low/high)
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(8)
#endif
INPUT_PORTS_END


/***************************************************************************
                             International Toote
***************************************************************************/

static INPUT_PORTS_START( inttoote )
	PORT_INCLUDE( jockeyc_keyboards )
	PORT_MODIFY("KEY1.1") // 200000.w (0x10)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no 1p credit
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no separate start keys?
	PORT_MODIFY("KEY2.1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	// inttoote: press cancel before betting to repeat the last bet

	PORT_START("COIN") // 200002.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM ) // P1 coin out
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_CUSTOM )  // P2 coin out
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SERVICE") // 200010.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) PORT_TOGGLE // open the door when in function menu to access the test mode
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Help") // press together with one of 1-2, 1-3, 1-4, 1-5, 1-6, 2-3, 2-4 to set clock
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	// Electronic key switch, fitted beneath the front panel:
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Function Key") PORT_TOGGLE // Function menu
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_CONFNAME( 0x8000, 0x0000, "Backup Battery" )
	PORT_CONFSETTING(      0x0000, "OK" )
	PORT_CONFSETTING(      0x8000, "NG" )

	PORT_START("DSW1") // SW1
	PORT_DIPNAME( 0x03, 0x03, "Max Bet (Per Horse)" )     PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "10" )
	PORT_DIPSETTING(    0x02, "20" )
	PORT_DIPSETTING(    0x01, "99" )
	PORT_DIPSETTING(    0x00, "99 (alt)" )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coinage ) )        PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/50 Credits" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown SW1:6" )           PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown SW1:7" )           PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown SW1:8" )           PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2_3") // SW2 & SW3
	PORT_DIPNAME( 0x0001, 0x0001, "Unknown SW2:1" )       PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Unknown SW2:2" )       PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Unknown SW2:3" )       PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0018, "Betting Clock Speed" ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x0018, "Slowest" )
	PORT_DIPSETTING(      0x0010, "Slower" )
	PORT_DIPSETTING(      0x0008, "Faster" )
	PORT_DIPSETTING(      0x0000, "Fastest" )
	PORT_DIPNAME( 0x01e0, 0x01e0, "Payout Rate" )         PORT_DIPLOCATION("SW2:6,7,8,SW3:1")
	PORT_DIPSETTING(      0x01e0, "80%" )
	PORT_DIPSETTING(      0x01c0, "81%" )
	PORT_DIPSETTING(      0x01a0, "82%" )
	PORT_DIPSETTING(      0x0180, "83%" )
	PORT_DIPSETTING(      0x0160, "84%" )
	PORT_DIPSETTING(      0x0140, "85%" )
	PORT_DIPSETTING(      0x0120, "86%" )
	PORT_DIPSETTING(      0x0100, "87%" )
	PORT_DIPSETTING(      0x00e0, "88%" )
	PORT_DIPSETTING(      0x00c0, "89%" )
	PORT_DIPSETTING(      0x00a0, "90%" )
	PORT_DIPSETTING(      0x0080, "91%" )
	PORT_DIPSETTING(      0x0060, "92%" )
	PORT_DIPSETTING(      0x0040, "93%" )
	PORT_DIPSETTING(      0x0020, "94%" )
	PORT_DIPSETTING(      0x0000, "95%" )
	PORT_DIPNAME( 0x0200, 0x0200, "Unknown SW3:2" )       PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Hopper Payout" )       PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Horses" )              PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(      0x0800, "Random (6 Out Of 100)" ) // 6 horses randomly chosen from a stable of 100
	PORT_DIPSETTING(      0x0000, "Cyclic (8 Set Races)"  ) // 8 set races continually cycled (player has a mental history of the preceding races)
	PORT_DIPNAME( 0x1000, 0x1000, "Odds" )                PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x1000, "Lower" )
	PORT_DIPSETTING(      0x0000, "Higher" )
	PORT_DIPNAME( 0x2000, 0x2000, "Unknown SW3:6" )       PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Unknown SW3:7" )       PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Unknown SW3:8" )       PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************

                                Graphics Layouts

Sprites and layers use 16x16 tile, made of four 8x8 tiles. They can be 4
or 6 planes deep and are stored in a wealth of formats.

***************************************************************************/

						// First the 4 bit tiles


// The tilemap bitplanes are packed togheter
static const gfx_layout layout_tilemap =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,4) },
	{ STEP4(4*4*8*3,1), STEP4(4*4*8*2,1), STEP4(4*4*8,1), STEP4(0,1) },
	{ STEP8(0,4*4), STEP8(4*4*8*4,4*4) },
	16*16*4
};


// The sprite bitplanes are separated (but there are 2 per ROM)
static const gfx_layout layout_sprites =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2) + 8, RGN_FRAC(1,2) + 0, 8, 0 },
	{ STEP8(0,1), STEP8(8*2*8,1) },
	{ STEP8(0,8*2), STEP8(8*2*8*2,8*2) },
	16*16*2
};


static const gfx_layout layout_tilemap_8bpp =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,4) },
	{ STEP4(8*4*8*3,1), STEP4(8*4*8*2,1), STEP4(8*4*8,1), STEP4(0,1) },
	{ STEP8(0,8*4), STEP8(8*4*8*4,8*4) },
	16*16*8
};


						// Then the 6 bit tiles


// The tilemap bitplanes are packed together
static const gfx_layout layout_tilemap_6bpp =
{
	16,16,
	RGN_FRAC(1,1),
	6,
	{ STEP4(0,4), STEP2(4*4,4) },
	{ STEP4(6*4*8*3,1), STEP4(6*4*8*2,1), STEP4(6*4*8,1), STEP4(0,1) },
	{ STEP8(0,6*4), STEP8(6*4*8*4,6*4) },
	16*16*6
};


/***************************************************************************
                                Blandia
***************************************************************************/

static GFXDECODE_START( gfx_blandia_layer1 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_tilemap_6bpp, 16*32+64*32*1, 32 ) // [0] Layer 1
	GFXDECODE_ENTRY( "gfx2", 0, layout_tilemap_6bpp, 16*32+64*32*3, 32 ) // [1] Layer 1
GFXDECODE_END

static GFXDECODE_START( gfx_blandia_layer2 )
	GFXDECODE_ENTRY( "gfx3", 0, layout_tilemap_6bpp, 16*32+64*32*0, 32 ) // [0] Layer 2
	GFXDECODE_ENTRY( "gfx3", 0, layout_tilemap_6bpp, 16*32+64*32*2, 32 ) // [1] Layer 2
GFXDECODE_END

static GFXDECODE_START( gfx_sprites )
	GFXDECODE_ENTRY( "gfx1", 0, layout_sprites, 0, 32 ) // Sprites
GFXDECODE_END

/***************************************************************************
                                Dragon Unit
***************************************************************************/

static GFXDECODE_START( gfx_drgnunit )
	GFXDECODE_ENTRY( "gfx2", 0, layout_tilemap, 0, 32 ) // [0] Layer 1
GFXDECODE_END

/***************************************************************************
                                The Roulette
***************************************************************************/

static GFXDECODE_START( gfx_setaroul )
	GFXDECODE_ENTRY( "gfx2", 0, layout_tilemap_8bpp, 0, 32 ) // [0] Layer 1
GFXDECODE_END

static GFXDECODE_START( gfx_setaroul_sprites )
	GFXDECODE_ENTRY( "gfx1", 0, layout_sprites,  0x100, 16 ) // Sprites
GFXDECODE_END

/***************************************************************************
                                J.J.Squawkers
***************************************************************************/

static GFXDECODE_START( gfx_jjsquawk_layer1 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_tilemap_6bpp, 16*32+64*32*0, 32 ) // [0] Layer 1
	GFXDECODE_ENTRY( "gfx2", 0, layout_tilemap_6bpp, 16*32+64*32*2, 32 ) // [1] Layer 1
GFXDECODE_END

static GFXDECODE_START( gfx_jjsquawk_layer2 )
	GFXDECODE_ENTRY( "gfx3", 0, layout_tilemap_6bpp, 16*32+64*32*1, 32 ) // [0] Layer 2
	GFXDECODE_ENTRY( "gfx3", 0, layout_tilemap_6bpp, 16*32+64*32*3, 32 ) // [1] Layer 2
GFXDECODE_END

/* The bitplanes are packed togheter: 4 bits in one rom, 2 bits in another.
   Since there isn't simmetry between the two roms, we load the latter with
   ROM_LOAD16_BYTE. This way we can think of it as a 4 planes rom, with the
   upper 2 planes unused.    */

static const gfx_layout layout_tilemap_6bpp_jjsquawkb =
{
	16,16,
	RGN_FRAC(1,2),
	6,
	{RGN_FRAC(1,2) + 0*4, RGN_FRAC(1,2) + 1*4, 2*4,3*4,0*4,1*4},
	{256+128,256+129,256+130,256+131, 256+0,256+1,256+2,256+3,
		128,129,130,131, 0,1,2,3},
	{0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,
		32*16,33*16,34*16,35*16,36*16,37*16,38*16,39*16},
	16*16*4
};

static GFXDECODE_START( gfx_jjsquawkb_layer1 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_tilemap_6bpp_jjsquawkb, 16*32+64*32*0, 32 ) // [0] Layer 1
	GFXDECODE_ENTRY( "gfx2", 0, layout_tilemap_6bpp_jjsquawkb, 16*32+64*32*2, 32 ) // [1] Layer 1
GFXDECODE_END

static GFXDECODE_START( gfx_jjsquawkb_layer2 )
	GFXDECODE_ENTRY( "gfx3", 0, layout_tilemap_6bpp_jjsquawkb, 16*32+64*32*1, 32 ) // [0] Layer 2
	GFXDECODE_ENTRY( "gfx3", 0, layout_tilemap_6bpp_jjsquawkb, 16*32+64*32*3, 32 ) // [1] Layer 2
GFXDECODE_END

/***************************************************************************
                            Mobile Suit Gundam
***************************************************************************/

static GFXDECODE_START( gfx_msgundam_layer1 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_tilemap, 0x400, 32 ) // [0] Layer 1
GFXDECODE_END

static GFXDECODE_START( gfx_msgundam_layer2 )
	GFXDECODE_ENTRY( "gfx3", 0, layout_tilemap, 0x200, 32 ) // [0] Layer 2
GFXDECODE_END

/***************************************************************************
                                Pairs Love
***************************************************************************/

// TODO: pairlove sets up two identical palette banks at 0-1ff and 0x200-0x3ff in-game, 0x200-0x3ff only in service mode.
//       Maybe there's a coloroffsetregister to somewhere?
static GFXDECODE_START( gfx_pairlove )
	GFXDECODE_ENTRY( "gfx1", 0, layout_sprites, 0x200, 32 ) // Sprites
GFXDECODE_END



/***************************************************************************
                                Wiggie Waggle / Super Bar
****************************************************************************/

/* these seem to have some silly address swapping, different on each game
  we handle it here, but we could also handle it in the init instead */
static const gfx_layout wiggie_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4),RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4) },
	{ 0,1,2,3,4,5,6,7,
		64,65,66,67,68,69,70,71 },
	{ 0*8, 16*8, 4*8, 20*8,
		2*8, 18*8, 6*8, 22*8,
		1*8, 17*8, 5*8, 21*8,
		3*8, 19*8, 7*8, 23*8 },
	16*16
};


static GFXDECODE_START( gfx_wiggie )
	GFXDECODE_ENTRY( "gfx1", 0, wiggie_layout, 0x0, 32 ) // bg tiles
GFXDECODE_END

static const gfx_layout superbar_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4),RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4) },
	{ 0,1,2,3,4,5,6,7,
		64,65,66,67,68,69,70,71 },
	{ 0*8, 2*8,   16*8, 18*8,
		1*8, 3*8,     17*8, 19*8,
		4*8, 6*8,     20*8, 22*8,
		5*8, 7*8,     21*8, 23*8 },
	16*16
};


static GFXDECODE_START( gfx_superbar )
	GFXDECODE_ENTRY( "gfx1", 0, superbar_layout, 0x0, 32 ) // bg tiles
GFXDECODE_END

/***************************************************************************
                                U.S. Classic
***************************************************************************/

static GFXDECODE_START( gfx_usclssic )
	GFXDECODE_ENTRY( "gfx2", 0, layout_tilemap_6bpp, 512+64*32*0, 32 ) // [0] Layer 1
	GFXDECODE_ENTRY( "gfx2", 0, layout_tilemap_6bpp, 512+64*32*1, 32 ) // [1] Layer 1
GFXDECODE_END


/***************************************************************************
                                Zing Zing Zip
***************************************************************************/

static GFXDECODE_START( gfx_zingzip_layer1 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_tilemap_6bpp,         16*32*2, 32 ) // [0] Layer 1
	GFXDECODE_ENTRY( "gfx2", 0, layout_tilemap_6bpp, 16*32*2+64*32*1, 32 ) // [1] Layer 1
GFXDECODE_END

static GFXDECODE_START( gfx_zingzip_layer2 )
	GFXDECODE_ENTRY( "gfx3", 0, layout_tilemap,              16*32*1, 32 ) // [0] Layer 2
GFXDECODE_END

static const gfx_layout layout_zzbl =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2) + 8, RGN_FRAC(1,2) + 0, 8, 0 },
	{ STEP8(0,1), STEP8(8*2*16,1) },
	{ STEP16(0,8*2) },
	16*16*2
};

static const gfx_layout layout_zzbl_6bpp =
{
	16,16,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(2,3) + 8, RGN_FRAC(2,3) + 0, RGN_FRAC(1,3) + 8, RGN_FRAC(1,3) + 0, 8, 0 },
	{ STEP8(8*2*16,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	16*16*2
};


static GFXDECODE_START( gfx_zingzipbl_sprites )
	GFXDECODE_ENTRY( "gfx1", 0, layout_zzbl,              16*32*0, 32 ) // [0] Sprites
GFXDECODE_END

static GFXDECODE_START( gfx_zingzipbl_layer1 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_zzbl_6bpp,         16*32*2, 32 ) // [0] Layer 1
	GFXDECODE_ENTRY( "gfx2", 0, layout_zzbl_6bpp, 16*32*2+64*32*1, 32 ) // [1] Layer 1
GFXDECODE_END

static GFXDECODE_START( gfx_zingzipbl_layer2 )
	GFXDECODE_ENTRY( "gfx3", 0, layout_zzbl,              16*32*1, 32 ) // [0] Layer 2
GFXDECODE_END


/***************************************************************************

                                Machine drivers

***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(seta_state::seta_interrupt_1_and_2)
{
	int scanline = param;

	if (scanline == 240)
		m_maincpu->set_input_line(1, HOLD_LINE);

	if (scanline == 112)
		m_maincpu->set_input_line(2, HOLD_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(seta_state::seta_interrupt_2_and_4)
{
	int scanline = param;

	if (scanline == 240)
		m_maincpu->set_input_line(2, ASSERT_LINE);

	if (scanline == 112)
		m_maincpu->set_input_line(4, ASSERT_LINE);
}


/***************************************************************************
                                Athena no Hatena?
***************************************************************************/

void seta_state::atehate(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::atehate_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(seta_state::seta_interrupt_1_and_2), "screen", 0, 1);

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // correct (test grid)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta_no_layers));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(512);    // sprites only

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16000000);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                                Blandia
***************************************************************************/

/*
    Similar to wrofaero, but the layers are 6 planes deep (and
    the pens are strangely mapped to palette entries) + the
    samples are bankswitched
*/

void seta_state::blandia(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::blandia_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(seta_state::seta_interrupt_2_and_4), "screen", 0, 1);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(8, 0); // correct (test grid, startup bg)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.screen_vblank().set(FUNC(seta_state::screen_vblank_seta_buffer_sprites));
	screen.set_palette(m_palette);

	X1_012(config, m_layers[0], m_palette, gfx_blandia_layer1).set_xoffsets(6, -2);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_blandia_layer2).set_xoffsets(6, -2);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette, FUNC(seta_state::blandia_palette), (16*32 + 64*32*4)*2, 0x600*2);  // sprites, layer1, layer2, palette effect - layers 1&2 are 6 planes deep

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16_MHz_XTAL);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
	m_x1snd->set_addrmap(0, &seta_state::blandia_x1_map);
}

void seta_state::blandiap(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::blandiap_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(seta_state::seta_interrupt_2_and_4), "screen", 0, 1);

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(8, 0); // correct (test grid, startup bg)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.screen_vblank().set(FUNC(seta_state::screen_vblank_seta_buffer_sprites));
	screen.set_palette(m_palette);

	X1_012(config, m_layers[0], m_palette, gfx_blandia_layer1).set_xoffsets(6, -2);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_blandia_layer2).set_xoffsets(6, -2);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette, FUNC(seta_state::blandia_palette), (16*32 + 64*32*4)*2, 0x600*2);  // sprites, layer1, layer2, palette effect - layers 1&2 are 6 planes deep

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16000000);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
	m_x1snd->set_addrmap(0, &seta_state::blandia_x1_map);
}


/***************************************************************************
                                Block Carnival
***************************************************************************/

void seta_state::blockcar(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL / 2); // 8 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::blockcar_map);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // unknown
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta_no_layers));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 3, ASSERT_LINE);

	PALETTE(config, m_palette).set_entries(512);    // sprites only

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16_MHz_XTAL);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


void seta_state::blockcarb_sound_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom();
	map(0xd000, 0xd7ff).ram();
	map(0xf000, 0xf001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xf002, 0xf002).noprw(); // ?
	map(0xf004, 0xf004).nopw(); // ?
	map(0xf006, 0xf006).nopw(); // ?
	map(0xf008, 0xf008).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xf00a, 0xf00a).nopr(); // ?
}

void seta_state::blockcarb(machine_config &config)
{
	blockcar(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::blockcarb_map);

	Z80(config, m_audiocpu, 4000000); // unk freq
	m_audiocpu->set_addrmap(AS_PROGRAM, &seta_state::blockcarb_sound_map);

	// the sound hardware / program is ripped from Mercs (CPS1)
	config.device_remove("x1snd");

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 4000000)); // unk freq
	ymsnd.irq_handler().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, "oki", 1000000, okim6295_device::PIN7_HIGH) // clock frequency & pin 7 not verified
		.add_route(ALL_OUTPUTS, "mono", 0.5);
}



/***************************************************************************
                                Daioh
***************************************************************************/

void seta_state::daioh(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);   // 16 MHz, MC68000-16, Verified from PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::daioh_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(seta_state::seta_interrupt_1_and_2), "screen", 0, 1);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // correct (test grid, planet)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57.42);   // verified on PCB
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.set_palette(m_palette);

	X1_012(config, m_layers[0], m_palette, gfx_msgundam_layer1).set_xoffsets(-2, -2);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_msgundam_layer2).set_xoffsets(-2, -2);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette).set_entries(512 * 3);    // sprites, layer1, layer2

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16_MHz_XTAL);   // 16 MHz, Verified from PCB audio
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                       Daioh (prototype)
***************************************************************************/

void seta_state::daiohp(machine_config &config)
{
	daioh(config);
	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::daiohp_map);
}


/***************************************************************************
                Dragon Unit, Quiz Kokology, Strike Gunner
***************************************************************************/

/*
    drgnunit,qzkklogy,stg:
    lev 1 == lev 3 (writes to $500000, bit 4 -> 1 then 0)
    lev 2 drives the game
*/

void seta_state::drgnunit(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL / 2); // 8 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::drgnunit_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(seta_state::seta_interrupt_1_and_2), "screen", 0, 1);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(2, 2); // correct (test grid and I/O test)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.screen_vblank().set(FUNC(seta_state::screen_vblank_seta_buffer_sprites));
	screen.set_palette(m_palette);

	X1_012(config, m_layers[0], m_palette, gfx_drgnunit).set_xoffsets(-2, -2);
	m_layers[0]->set_screen(m_screen);
	PALETTE(config, m_palette).set_entries(512);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16_MHz_XTAL);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void seta_state::stg(machine_config &config)
{
	drgnunit(config);
	m_spritegen->set_fg_xoffsets(0, 0); // sprites correct? (panel), tilemap correct (test grid)
}

void seta_state::qzkklogy(machine_config &config)
{
	drgnunit(config);
	m_spritegen->set_fg_xoffsets(1, 1); // correct (timer, test grid)
	m_layers[0]->set_xoffsets(-1, -1);
}

//  Same as qzkklogy, but with a 16MHz CPU

void seta_state::qzkklgy2(machine_config &config)
{
	drgnunit(config);
	// basic machine hardware
	m_maincpu->set_clock(16000000);   // 16 MHz

	m_spritegen->set_fg_xoffsets(0, 0); // sprites unknown, tilemaps correct (test grid)
	m_layers[0]->set_xoffsets(-3, -1);
}


/***************************************************************************
                                The Roulette
***************************************************************************/

template <uint8_t Irq1, uint8_t Irq2>
TIMER_DEVICE_CALLBACK_MEMBER(setaroul_state::interrupt)
{
	int scanline = param;

	if ((scanline % 32) == 0) // every 2ms?
		m_maincpu->set_input_line(Irq1, HOLD_LINE); // read 1 board column (out of 26) every other call

	if (scanline == 248)
		m_maincpu->set_input_line(Irq2, HOLD_LINE); // vblank

	// lev 6: RS232
}

void setaroul_state::setaroul(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL / 2); // 8 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &setaroul_state::setaroul_map);
	TIMER(config, "scantimer").configure_scanline(*this, NAME((&setaroul_state::interrupt<2, 4>)), "screen", 0, 1);

	WATCHDOG_TIMER(config, m_watchdog);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_setaroul_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 7); // unknown (flipped offsets are unused: game handles flipping manually without setting the flip bit)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0, -0x1);
	m_spritegen->set_bg_xoffsets(0, 0x2);

	NVRAM(config, "nvram", nvram_device::DEFAULT_RANDOM);

	// devices
	UPD4992(config, m_rtc, 32'768); // ! Actually D4911C !
	ACIA6850(config, "acia0", 0);
	TICKET_DISPENSER(config, "hopper", attotime::from_msec(150));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(setaroul_state::screen_update));
	screen.screen_vblank().set(FUNC(setaroul_state::screen_vblank));
	screen.set_palette(m_palette);

	X1_012(config, m_layers[0], m_palette, gfx_setaroul).set_xoffsets(0, 5);
	m_layers[0]->set_screen(m_screen);
	PALETTE(config, m_palette, FUNC(setaroul_state::setaroul_palette), 512);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	X1_010(config, m_x1snd, 16_MHz_XTAL);   // 16 MHz
	m_x1snd->add_route(0, "speaker", 1.0, 0);
	m_x1snd->add_route(1, "speaker", 1.0, 1);

	// layout
	config.set_default_layout(layout_setaroul);
}

void setaroul_state::setaroulm(machine_config &config)
{
	setaroul(config);

	TIMER(config.replace(), "scantimer").configure_scanline(*this, NAME((&setaroul_state::interrupt<5, 4>)), "screen", 0, 1);
}


/***************************************************************************
                                Eight Force
***************************************************************************/

void seta_state::eightfrc(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::zingzip_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(seta_state::seta_interrupt_1_and_2), "screen", 0, 1);
	WATCHDOG_TIMER(config, m_watchdog);

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(4, 3); // correct (test mode)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.set_palette(m_palette);

	X1_012(config, m_layers[0], m_palette, gfx_msgundam_layer1);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_msgundam_layer2);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette).set_entries(512 * 3);    // sprites, layer1, layer2

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16000000);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
	m_x1snd->set_addrmap(0, &seta_state::blandia_x1_map);
}


/***************************************************************************
                        Extreme Downhill / Sokonuke
***************************************************************************/

/*
    extdwnhl:
    lev 1 == lev 3 (writes to $500000, bit 4 -> 1 then 0)
    lev 2 drives the game
*/
void seta_state::extdwnhl(machine_config &config)
{
	// TODO: verify clocks (16 MHz and 14.318 MHz XTALs are both on board)

	// basic machine hardware
	M68000(config, m_maincpu, 16000000);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::extdwnhl_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(seta_state::seta_interrupt_1_and_2), "screen", 0, 1);
	WATCHDOG_TIMER(config, m_watchdog);

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // correct (test grid, background images)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.set_palette(m_palette);

	X1_012(config, m_layers[0], m_palette, gfx_zingzip_layer1).set_xoffsets(-2, -2);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_zingzip_layer2).set_xoffsets(-2, -2);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette, FUNC(seta_state::zingzip_palette), 16*32 + 16*32 + 64*32*2, 0x600);    // sprites, layer2, layer1 - layer 1 gfx is 6 planes deep

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	X1_010(config, m_x1snd, 16000000);   // 16 MHz
	m_x1snd->add_route(0, "speaker", 1.0, 0);
	m_x1snd->add_route(1, "speaker", 1.0, 1);
}


/***************************************************************************
                                Gundhara
***************************************************************************/

/*
    lev 1: sample end? (needed in zombraid otherwise music stops)
           gundhara's debug code calls it "BUT_IPL" and does nothing
    lev 2: VBlank
    lev 4: Sound (generated by a timer mapped at $d00000-6 ?)
*/
void seta_state::gundhara(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::wrofaero_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(seta_state::seta_interrupt_1_and_2), "screen", 0, 1);

	pit8254_device &pit(PIT8254(config, "pit", 0)); // uPD71054C
	pit.set_clk<0>(16000000/2/8);
	pit.out_handler<0>().set(FUNC(seta_state::pit_out0));

	WATCHDOG_TIMER(config, m_watchdog);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // correct (test mode)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.set_palette(m_palette);

	X1_012(config, m_layers[0], m_palette, gfx_jjsquawk_layer1);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_jjsquawk_layer2);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette, FUNC(seta_state::gundhara_palette), 16*32 + 64*32*4, 0x600);  // sprites, layer2, layer1 - layers are 6 planes deep (seta_state,but have only 4 palettes)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16_MHz_XTAL);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                                Zombie Raid
***************************************************************************/

void zombraid_state::machine_start()
{
	seta_state::machine_start();

	m_gun_recoil.resolve();
}

void zombraid_state::zombraid(machine_config &config)
{
	gundhara(config);

	config.device_remove("pit"); // present on PCB but not used?

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &zombraid_state::zombraid_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	ADC0834(config, m_adc);
	m_adc->set_input_callback(FUNC(zombraid_state::adc_cb));

	m_layers[0]->set_xoffsets(-2, -2); // correct for normal, flip screen not working yet
	m_layers[1]->set_xoffsets(-2, -2);

	m_x1snd->set_addrmap(0, &zombraid_state::zombraid_x1_map);
}

/***************************************************************************
                                J.J.Squawkers
***************************************************************************/

/*
    lev 1 == lev 3 (writes to $500000, bit 4 -> 1 then 0)
    lev 2 drives the game
*/
void seta_state::jjsquawk(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::zingzip_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(seta_state::seta_interrupt_1_and_2), "screen", 0, 1);
	WATCHDOG_TIMER(config, m_watchdog);

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(1, 1); // correct (test mode)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.set_palette(m_palette);

	X1_012(config, m_layers[0], m_palette, gfx_jjsquawk_layer1).set_xoffsets(-1, -1);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_jjsquawk_layer2).set_xoffsets(-1, -1);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette, FUNC(seta_state::jjsquawk_palette), 16*32 + 64*32*4, 0x600);  // sprites, layer2, layer1 - layers are 6 planes deep

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16000000);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void seta_state::jjsquawb(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::jjsquawb_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(seta_state::seta_interrupt_1_and_2), "screen", 0, 1);

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(1, 1); // correct (test mode)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.set_palette(m_palette);

	X1_012(config, m_layers[0], m_palette, gfx_jjsquawkb_layer1).set_xoffsets(-1, -1);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_jjsquawkb_layer2).set_xoffsets(-1, -1);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette, FUNC(seta_state::jjsquawk_palette), 16*32 + 64*32*4, 0x600);  // sprites, layer2, layer1 - layers are 6 planes deep

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16000000);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                (Kamen) Masked Riders Club Battle Race
***************************************************************************/

//  kamenrid: lev 2 by vblank, lev 4 by timer
void seta_state::kamenrid(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::kamenrid_map);

	WATCHDOG_TIMER(config, m_watchdog);

	pit8254_device &pit(PIT8254(config, "pit", 0)); // uPD71054C
	pit.set_clk<0>(16000000/2/8);
	pit.out_handler<0>().set(FUNC(seta_state::pit_out0));

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // correct (map, banpresto logo)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 2, ASSERT_LINE);

	X1_012(config, m_layers[0], m_palette, gfx_msgundam_layer1).set_xoffsets(-2, -2);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_msgundam_layer2).set_xoffsets(-2, -2);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette).set_entries(512 * 3);    // sprites, layer1, layer2

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16000000);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                                Orbs
***************************************************************************/

// The CPU clock has been verified/measured, PCB only has one OSC and it's 14.318180 MHz

void seta_state::orbs(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 14.318181_MHz_XTAL / 2); // 7.143 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::orbs_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(seta_state::seta_interrupt_1_and_2), "screen", 0, 1);

	X1_001(config, m_spritegen, 14.318181_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // unknown
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(1*8, 39*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta_no_layers));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(512);    // sprites only

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	X1_010(config, m_x1snd, 14.318181_MHz_XTAL);   // 14.318180 MHz
	m_x1snd->add_route(0, "speaker", 1.0, 0);
	m_x1snd->add_route(1, "speaker", 1.0, 1);
}


/***************************************************************************
                  Kero Kero Keroppi no Isshoni Asobou
***************************************************************************/

void keroppi_state::keroppi(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 14318180/2); // 7.143 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &keroppi_state::keroppi_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(keroppi_state::seta_interrupt_1_and_2), "screen", 0, 1);

	X1_001(config, m_spritegen, 14318180, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(keroppi_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // unknown
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(keroppi_state::screen_update_seta_no_layers));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(512);    // sprites only

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	X1_010(config, m_x1snd, 14318180);   // 14.318180 MHz
	m_x1snd->add_route(0, "speaker", 1.0, 0);
	m_x1snd->add_route(1, "speaker", 1.0, 1);
}


/***************************************************************************
                                Krazy Bowl
***************************************************************************/

void seta_state::krzybowl(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 14.318181_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::krzybowl_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(seta_state::seta_interrupt_1_and_2), "screen", 0, 1);

	upd4701_device &upd1(UPD4701A(config, "upd1"));
	upd1.set_portx_tag("TRACK1_X");
	upd1.set_porty_tag("TRACK1_Y");

	upd4701_device &upd2(UPD4701A(config, "upd2"));
	upd2.set_portx_tag("TRACK2_X");
	upd2.set_porty_tag("TRACK2_Y");

	X1_001(config, m_spritegen, 14.318181_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // correct (test grid)
	m_spritegen->set_fg_yoffsets(-0x06, 0x0e);
	m_spritegen->set_bg_yoffsets(-0x3, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(1*8, 39*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta_no_layers));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(512);    // sprites only

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 14.318181_MHz_XTAL);
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                                Mad Shark
***************************************************************************/

//  madshark: lev 2 by vblank, lev 4 by timer
void seta_state::madshark(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::madshark_map);

	pit8254_device &pit(PIT8254(config, "pit", 0)); // uPD71054C
	pit.set_clk<0>(16000000/2/8);
	pit.out_handler<0>().set(FUNC(seta_state::pit_out0));

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // unknown (wrong when flipped, but along y)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	WATCHDOG_TIMER(config, m_watchdog);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 2, ASSERT_LINE);

	X1_012(config, m_layers[0], m_palette, gfx_jjsquawk_layer1);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_jjsquawk_layer2);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette, FUNC(seta_state::jjsquawk_palette), 16*32 + 64*32*4, 0x600);  // sprites, layer2, layer1 - layers are 6 planes deep

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16000000);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void seta_state::madsharkbl(machine_config &config) // bootleg doesn't actually use the Seta customs
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::madsharkbl_map);

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // unknown (wrong when flipped, but along y)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 2, ASSERT_LINE);

	X1_012(config, m_layers[0], m_palette, gfx_jjsquawk_layer1);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_jjsquawk_layer2);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette, FUNC(seta_state::jjsquawk_palette), 16*32 + 64*32*4, 0x600);  // sprites, layer2, layer1 - layers are 6 planes deep

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 1'000'000, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->set_addrmap(0, &seta_state::madsharkbl_oki_map);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

/***************************************************************************
                                Magical Speed
***************************************************************************/

void magspeed_state::machine_start()
{
	seta_state::machine_start();

	m_leds.resolve();

	save_item(NAME(m_lights));
}

//  magspeed: lev 2 by vblank, lev 4 by timer
void magspeed_state::magspeed(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &magspeed_state::magspeed_map);

	WATCHDOG_TIMER(config, m_watchdog);

	pit8254_device &pit(PIT8254(config, "pit", 0)); // uPD71054C
	pit.set_clk<0>(16000000/2/8);
	pit.out_handler<0>().set(FUNC(magspeed_state::pit_out0));

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(magspeed_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // floating tilemap maybe 1px off in test grid
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(magspeed_state::screen_update_seta));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 2, ASSERT_LINE);

	X1_012(config, m_layers[0], m_palette, gfx_msgundam_layer1).set_xoffsets(0, -2);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_msgundam_layer2).set_xoffsets(0, -2);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette).set_entries(512 * 3);    // sprites, layer1, layer2

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16000000);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                            Mobile Suit Gundam
***************************************************************************/

// msgundam lev 2 == lev 6 !

void seta_state::msgundam(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::msgundam_map);

	pit8254_device &pit(PIT8254(config, "pit", 0)); // uPD71054C
	pit.set_clk<0>(16000000/2/8);
	pit.out_handler<0>().set(FUNC(seta_state::pit_out0));

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // correct (test grid, banpresto logo)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(56.66); // between 56 and 57 to match a real PCB's game speed
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.screen_vblank().set(FUNC(seta_state::screen_vblank_seta_buffer_sprites));
	screen.screen_vblank().append_inputline(m_maincpu, 2, ASSERT_LINE);
	screen.set_palette(m_palette);

	X1_012(config, m_layers[0], m_palette, gfx_msgundam_layer1).set_xoffsets(-2, -2);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_msgundam_layer2).set_xoffsets(-2, -2);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette).set_entries(512 * 3);    // sprites, layer1, layer2

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16000000);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void seta_state::msgundamb(machine_config &config)
{
	msgundam(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::msgundamb_map);
}

/***************************************************************************
                            Oishii Puzzle
***************************************************************************/

void seta_state::oisipuzl(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::oisipuzl_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(seta_state::seta_interrupt_1_and_2), "screen", 0, 1);

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(1, 1); // correct (test mode) flip screen not supported?
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.set_palette(m_palette);

	X1_012(config, m_layers[0], m_palette, gfx_msgundam_layer1).set_xoffsets(-1, -1);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_msgundam_layer2).set_xoffsets(-1, -1);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette).set_entries(512 * 3);    // sprites, layer1, layer2

	set_tilemaps_flip(1); // flip is inverted for the tilemaps

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	X1_010(config, m_x1snd, 16000000);   // 16 MHz
	m_x1snd->add_route(0, "speaker", 1.0, 0);
	m_x1snd->add_route(1, "speaker", 1.0, 1);
}


/***************************************************************************
                            Triple Fun
***************************************************************************/

// same as oisipuzl but with different interrupts and sound

void seta_state::triplfun(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::triplfun_map);

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(1, 1); // correct (test mode) flip screen not supported?
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 3, HOLD_LINE);

	X1_012(config, m_layers[0], m_palette, gfx_msgundam_layer1).set_xoffsets(-1, -1);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_msgundam_layer2).set_xoffsets(-1, -1);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette).set_entries(512 * 3);    // sprites, layer1, layer2

	set_tilemaps_flip(1); // flip is inverted for the tilemaps

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	okim6295_device &oki(OKIM6295(config, "oki", 792000, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	oki.add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
}


/***************************************************************************
                                    Rezon
***************************************************************************/

void seta_state::rezon(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL); // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::rezon_map);

	WATCHDOG_TIMER(config, m_watchdog);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // correct (test grid)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57.42); // approximation from PCB video
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 2, ASSERT_LINE);

	X1_012(config, m_layers[0], m_palette, gfx_msgundam_layer1).set_xoffsets(-2, -2);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_msgundam_layer2).set_xoffsets(-2, -2);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette).set_entries(512 * 3); // sprites, layer1, layer2

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16_MHz_XTAL); // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                        Thunder & Lightning / Wit's
***************************************************************************/

void thunderl_state::machine_start()
{
	seta_state::machine_start();

	save_item(NAME(m_thunderl_protection_reg));
}

// thunderl lev 2 = lev 3 - other levels lead to an error

void thunderl_state::thunderl(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL / 2); // 8 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &thunderl_state::thunderl_map);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(thunderl_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // unknown
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(thunderl_state::screen_update_seta_no_layers));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 2, ASSERT_LINE);

	PALETTE(config, m_palette).set_entries(512);    // sprites only

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16_MHz_XTAL);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


void thunderl_state::thunderlbl_sound_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xdfff).rom();
	map(0xf800, 0xffff).ram();
}

void thunderl_state::thunderlbl_sound_portmap(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x01).mirror(0x3e).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xc0, 0xc0).mirror(0x3f).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void thunderl_state::thunderlbl(machine_config &config)
{
	thunderl(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &thunderl_state::thunderlbl_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4); // XTAL verified, divider unknown, but Z8400A PS, so likely
	m_audiocpu->set_addrmap(AS_PROGRAM, &thunderl_state::thunderlbl_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &thunderl_state::thunderlbl_sound_portmap);

	// the sound hardware / program is ripped from Tetris (S16B)
	config.device_remove("x1snd");

	YM2151(config, "ymsnd", 16_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 1.0); // XTAL verified, divider unknown

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);
}


void seta_state::wiggie(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL / 2); // 8 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::wiggie_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4);   // 4 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &seta_state::wiggie_sound_map);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_wiggie);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // some problems but they seem y co-ordinate related?
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta_no_layers));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 2, ASSERT_LINE);

	PALETTE(config, m_palette).set_entries(512);    // sprites only

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 16_MHz_XTAL / 16, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);
}

void seta_state::superbar(machine_config &config)
{
	wiggie(config);
	m_spritegen->set_info(gfx_superbar);
}

void seta_state::wits(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL / 2); // 8 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::wits_map);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // unknown
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta_no_layers));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 2, ASSERT_LINE);

	PALETTE(config, m_palette).set_entries(512);    // sprites only

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16_MHz_XTAL);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                    Ultraman Club / SD Gundam Neo Battling
***************************************************************************/

void seta_state::umanclub(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::umanclub_map);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // correct (test grid)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta_no_layers));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 3, HOLD_LINE);

	PALETTE(config, m_palette).set_entries(512);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16_MHz_XTAL);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                            Ultra Toukond Densetsu
***************************************************************************/

void seta_state::utoukond(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::utoukond_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(seta_state::seta_interrupt_1_and_2), "screen", 0, 1);

	Z80(config, m_audiocpu, 16000000/4);   // 4 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &seta_state::utoukond_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &seta_state::utoukond_sound_io_map);

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // unknown (wrong when flipped, but along y)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.set_palette(m_palette);

	X1_012(config, m_layers[0], m_palette, gfx_msgundam_layer1).set_xoffsets(0, -2);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_msgundam_layer2).set_xoffsets(0, -2);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette).set_entries(512 * 3);    // sprites, layer1, layer2

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);
	m_soundlatch->set_separate_acknowledge(true);

	X1_010(config, m_x1snd, 16000000);
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.50);

	ym3438_device &ymsnd(YM3438(config, "ymsnd", 16000000/2)); // 8 MHz
	ymsnd.irq_handler().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.30);
}


/***************************************************************************
                                War of Aero
***************************************************************************/

void seta_state::wrofaero(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::wrofaero_map);

	pit8254_device &pit(PIT8254(config, "pit", 0)); // uPD71054C
	pit.set_clk<0>(16000000/2/8);
	pit.out_handler<0>().set(FUNC(seta_state::pit_out0));

	WATCHDOG_TIMER(config, m_watchdog);

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // correct (test mode)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 2, HOLD_LINE);

	X1_012(config, m_layers[0], m_palette, gfx_msgundam_layer1);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_msgundam_layer2);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette).set_entries(512 * 3);    // sprites, layer1, layer2

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16000000);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                                Zing Zing Zip
***************************************************************************/

/* zingzip lev 3 = lev 2 + lev 1 !
   SR = 2100 -> lev1 is ignored so we must supply int 3, since the routine
   at int 1 is necessary: it plays the background music.
*/

void seta_state::zingzip(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::zingzip_map);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // sprites unknown, tilemaps correct (test grid)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	WATCHDOG_TIMER(config, m_watchdog);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57.42); // taken from other games but seems to better match PCB videos
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 3, HOLD_LINE);

	X1_012(config, m_layers[0], m_palette, gfx_zingzip_layer1).set_xoffsets(-2, -1);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_zingzip_layer2).set_xoffsets(-2, -1);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette, FUNC(seta_state::zingzip_palette), 16*32 + 16*32 + 64*32*2, 0x600);    // sprites, layer2, layer1 - layer 1 gfx is 6 planes deep

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16_MHz_XTAL);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


void seta_state::zingzipbl(machine_config &config)
{
	zingzip(config);
	m_spritegen->set_info(gfx_zingzipbl_sprites);
	m_layers[0]->set_info(gfx_zingzipbl_layer1);
	m_layers[1]->set_info(gfx_zingzipbl_layer2);
	// TODO: layers x and y offsets' adjustments

	M68000(config.replace(), m_maincpu, 16000000);   // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &seta_state::zingzipbl_map);

	subdevice<screen_device>("screen")->screen_vblank().set_inputline(m_maincpu, 6, HOLD_LINE); // TODO: there's probably more than this

	config.device_remove("x1snd");

	OKIM6295(config, "oki", 1000000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/***************************************************************************
                                Pairs Love
***************************************************************************/

void pairlove_state::pairlove(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL / 2); // 8 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &pairlove_state::pairlove_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(pairlove_state::seta_interrupt_1_and_2), "screen", 0, 1);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_pairlove);
	m_spritegen->set_gfxbank_callback(FUNC(pairlove_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // unknown
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(pairlove_state::screen_update_seta_no_layers));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(2048);   // sprites only

	// sound hardware
	SPEAKER(config, "mono").front_center();

	X1_010(config, m_x1snd, 16000000);   // 16 MHz
	m_x1snd->add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                                Crazy Fight
***************************************************************************/

void crazyfgt_state::crazyfgt(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &crazyfgt_state::crazyfgt_map);

	DS2430A(config, m_eeprom);

	X1_001(config, m_spritegen, 16_MHz_XTAL, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(crazyfgt_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // wrong (empty background column in title screen, but aligned sprites in screen select)
	m_spritegen->set_fg_yoffsets(-0x12, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.1851);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(crazyfgt_state::screen_update_seta));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 1, ASSERT_LINE);

	X1_012(config, m_layers[0], m_palette, gfx_blandia_layer1).set_xoffsets(0, -2);
	m_layers[0]->set_screen(m_screen);
	X1_012(config, m_layers[1], m_palette, gfx_blandia_layer2).set_xoffsets(0, -2);
	m_layers[1]->set_screen(m_screen);
	PALETTE(config, m_palette, FUNC(crazyfgt_state::gundhara_palette), 16*32 + 64*32*4, 0x600);  // sprites, layer2, layer1 - layers are 6 planes deep (seta_state,but have only 4 palettes)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 16_MHz_XTAL / 4));
	ymsnd.irq_handler().set_inputline(m_maincpu, 2);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.5);

	okim6295_device &oki(OKIM6295(config, "oki", 4.433619_MHz_XTAL / 4, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                                 Jockey Club
***************************************************************************/

// Test mode shows a 16ms and 2ms counters, then there's vblank and presumably ACIA irqs ...
TIMER_DEVICE_CALLBACK_MEMBER(jockeyc_state::interrupt)
{
	int scanline = param;

	// ACIA IRQ
	if (scanline == 15)
		m_maincpu->set_input_line(4, HOLD_LINE);

	if (scanline == 38)
		m_maincpu->set_input_line(1, HOLD_LINE);

	if (scanline == 61)
		m_maincpu->set_input_line(2, HOLD_LINE);

	if (scanline >= 85 && (scanline % 23) == 0)
		m_maincpu->set_input_line(6, HOLD_LINE);
}

MACHINE_START_MEMBER(jockeyc_state, jockeyc)
{
	m_out_cancel.resolve();
	m_out_payout.resolve();
	m_out_start.resolve();
}


void jockeyc_state::jockeyc(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(16'000'000)/2); // TMP68000N-8
	m_maincpu->set_addrmap(AS_PROGRAM, &jockeyc_state::jockeyc_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(jockeyc_state::interrupt), "screen", 0, 1);

	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_seconds(2.0)); // jockeyc: watchdog test error if over 2.5s

	X1_001(config, m_spritegen, 16000000, m_palette, gfx_sprites);
	m_spritegen->set_gfxbank_callback(FUNC(seta_state::setac_gfxbank_callback));
	// position kludges
	m_spritegen->set_fg_xoffsets(0, 0); // sprites correct? (bets), tilemap correct (test grid)
	m_spritegen->set_fg_yoffsets(-0x12+8, 0x0e);
	m_spritegen->set_bg_yoffsets(0x1, -0x1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_RANDOM);

	MCFG_MACHINE_START_OVERRIDE(jockeyc_state, jockeyc)
	// devices
	UPD4992(config, m_rtc, 32'768); // ! Actually D4911C !
	ACIA6850(config, "acia0", 0);
	TICKET_DISPENSER(config, "hopper1", attotime::from_msec(150));
	TICKET_DISPENSER(config, "hopper2", attotime::from_msec(150));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(seta_state::screen_update_seta_layers));
	screen.set_palette(m_palette);

	X1_012(config, m_layers[0], m_palette, gfx_drgnunit).set_xoffsets(126, -2);
	m_layers[0]->set_screen(m_screen);
	PALETTE(config, m_palette, FUNC(seta_state::palette_init_RRRRRGGGGGBBBBB_proms), 512 * 1);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	X1_010(config, m_x1snd, 16000000);
	m_x1snd->add_route(0, "speaker", 1.0, 0);
	m_x1snd->add_route(1, "speaker", 1.0, 1);

	// layout
	config.set_default_layout(layout_jockeyc);
}


/***************************************************************************
                             International Toote
***************************************************************************/

MACHINE_START_MEMBER(jockeyc_state, inttoote)
{
	m_out_help.resolve();
	m_out_itstart.resolve();
}

void jockeyc_state::inttoote(machine_config &config)
{
	jockeyc(config);

	M68000(config.replace(), m_maincpu, XTAL(16'000'000)); // TMP68HC000N-16
	m_maincpu->set_addrmap(AS_PROGRAM, &jockeyc_state::inttoote_map);

	MCFG_MACHINE_START_OVERRIDE(jockeyc_state, inttoote)

	m_layers[0]->set_xoffsets(0, -2);

	// I/O board (not hooked up yet)
	PIA6821(config, "pia0");
	PIA6821(config, "pia1");

	ACIA6850(config, "acia1", 0);
	ACIA6850(config, "acia2", 0);

	// layout
	config.set_default_layout(layout_inttoote);
}



/***************************************************************************

                                ROMs Loading

***************************************************************************/

// used for 6bpp gfxs
#define ROM_LOAD24_BYTE(name, offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_SKIP(2))
#define ROM_LOAD24_WORD(name, offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(1))
#define ROM_LOAD24_WORD_SWAP(name, offset,length,hash)   ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(1))

ROM_START( drgnunit )
	ROM_REGION( 0x0c0000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "prg-e.bin", 0x000000, 0x020000, CRC(728447df) SHA1(8bdc52a4cc5f36794a47f963545bdaa26c9acd6b) )
	ROM_LOAD16_BYTE( "prg-o.bin", 0x000001, 0x020000, CRC(b2f58ecf) SHA1(5198e75b22bab630b458797988f2e443c601351f) )

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "obj-2.bin", 0x000000, 0x020000, CRC(d7f6ab5a) SHA1(a32f1705e833c339bd0c426a395cc706da96dad7) )
	ROM_LOAD16_BYTE( "obj-1.bin", 0x000001, 0x020000, CRC(53a95b13) SHA1(b7c7994441aafcea49662dc0fbebd6db836723f5) )
	ROM_LOAD16_BYTE( "obj-6.bin", 0x040000, 0x020000, CRC(80b801f7) SHA1(5b5635903137e50bc982d05b73c2648bbf182e71) )
	ROM_LOAD16_BYTE( "obj-5.bin", 0x040001, 0x020000, CRC(6b87bc20) SHA1(9a0e3e18339d6c12e63960fb940a56c16dcb87cf) )
	ROM_LOAD16_BYTE( "obj-4.bin", 0x080000, 0x020000, CRC(60d17771) SHA1(0874c10a2527293715db95bd7c83886d94f810cf) )
	ROM_LOAD16_BYTE( "obj-3.bin", 0x080001, 0x020000, CRC(0bccd4d5) SHA1(7139ef793efe7c6477f78b50207227b1be223755) )
	ROM_LOAD16_BYTE( "obj-8.bin", 0x0c0000, 0x020000, CRC(826c1543) SHA1(f669f255b4596da5648592b5993b02671e404102) )
	ROM_LOAD16_BYTE( "obj-7.bin", 0x0c0001, 0x020000, CRC(cbaa7f6a) SHA1(060f0651b8ca07d239ef1b7c63943cdd433e1ae9) )

	ROM_REGION( 0x100000, "gfx2", 0 )   // Layer 1
	ROM_LOAD16_BYTE( "scr-1o.bin",  0x000000, 0x020000, CRC(671525db) SHA1(e230e99754c9f40af7da2054bd5ea09823e0b1b7) )
	ROM_LOAD16_BYTE( "scr-2o.bin",  0x040000, 0x020000, CRC(2a3f2ed8) SHA1(9d188100437a10eb3d3097f28e28e3cb2dc1b21d) )
	ROM_LOAD16_BYTE( "scr-3o.bin",  0x080000, 0x020000, CRC(4d33a92d) SHA1(8b09768abb460446405224565eb6652d2dc1c571) )
	ROM_LOAD16_BYTE( "scr-4o.bin",  0x0c0000, 0x020000, CRC(79a0aa61) SHA1(9905d90afb759b1d983856d7bef17c139d4f0e4f) )
	ROM_LOAD16_BYTE( "scr-1e.bin",  0x000001, 0x020000, CRC(dc9cd8c9) SHA1(04450a5cfde5d6b69fdd745cd930309863e1aadd) )
	ROM_LOAD16_BYTE( "scr-2e.bin",  0x040001, 0x020000, CRC(b6126b41) SHA1(13275f05868d93af95ebb162d229b69ddd660438) )
	ROM_LOAD16_BYTE( "scr-3e.bin",  0x080001, 0x020000, CRC(1592b8c2) SHA1(d337de280c5ea3704dec9baa04c45e1c837924a9) )
	ROM_LOAD16_BYTE( "scr-4e.bin",  0x0c0001, 0x020000, CRC(8201681c) SHA1(7784a68828d728107b0228bb3568129c543cbf40) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "snd-1.bin", 0x000000, 0x020000, CRC(8f47bd0d) SHA1(c55e22ac4294931cfb72ac88a2128891d9f8ee93) )
	ROM_LOAD( "snd-2.bin", 0x020000, 0x020000, CRC(65c40ef5) SHA1(726b46144e2216d17b0828abad2f5e7c2305c174) )
	ROM_LOAD( "snd-3.bin", 0x040000, 0x020000, CRC(71fbd54e) SHA1(bdaf7ecf1c79c6c8fc82d959186ca2f3304729c8) )
	ROM_LOAD( "snd-4.bin", 0x060000, 0x020000, CRC(ac50133f) SHA1(d56a9569bd72c7bc13d09dcea9789cdc7252ffb4) )
	ROM_LOAD( "snd-5.bin", 0x080000, 0x020000, CRC(70652f2c) SHA1(04ff016a087a230efe4644eb76f68886aae26978) )
	ROM_LOAD( "snd-6.bin", 0x0a0000, 0x020000, CRC(10a1039d) SHA1(a1160fe600d39ae6fdbf247f634c2e094b3a675f) )
	ROM_LOAD( "snd-7.bin", 0x0c0000, 0x020000, CRC(decbc8b0) SHA1(9d315d1119fbc2bf889efdb174ebc5e26ecad859) )
	ROM_LOAD( "snd-8.bin", 0x0e0000, 0x020000, CRC(3ac51bee) SHA1(cb2ee501895b848d434991152dea293685f8ed22) )
ROM_END

ROM_START( wits )
	ROM_REGION( 0x010000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "un001001.u1", 0x000000, 0x008000, CRC(416c567e) SHA1(f0898ce4457efc272e0fec3447c9d4598684219e) )
	ROM_LOAD16_BYTE( "un001002.u4", 0x000001, 0x008000, CRC(497a3fa6) SHA1(cf035efddc2a90013e83dcb81687ba1896ba6055) )

	ROM_REGION( 0x080000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "un001008.7l", 0x000000, 0x020000, CRC(1d5d0b2b) SHA1(12cf1be316012e8ee910edfd1b892b7ce1383535) )
	ROM_LOAD16_BYTE( "un001007.5l", 0x000001, 0x020000, CRC(9e1e6d51) SHA1(9a87f0f18ac0b3d267fe8655d01750d693745c1f) )
	ROM_LOAD16_BYTE( "un001006.4l", 0x040000, 0x020000, CRC(98a980d4) SHA1(ab2c1ed83bccffabfacc8a185d1fbc3e8aaf210d) )
	ROM_LOAD16_BYTE( "un001005.2l", 0x040001, 0x020000, CRC(6f2ce3c0) SHA1(8086b44c7025bc0bffff75cc6c6c7846cc56e8d0) )

	ROM_REGION( 0x40000, "x1snd", 0 )   // Samples
	ROM_LOAD( "un001004.12a", 0x000000, 0x020000, CRC(a15ff938) SHA1(fdfdf73e85d89a39cfc5b3c3048a64178200f942) )
	ROM_LOAD( "un001003.10a", 0x020000, 0x020000, CRC(3f4b9e55) SHA1(3cecf89ae6a056622affcddec9e10be08761e56d) )
ROM_END

ROM_START( thunderl )
	ROM_REGION( 0x010000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "m4", 0x000000, 0x008000, CRC(1e6b9462) SHA1(f7f93479117e97d4e38632fef83c10345587f77f) )
	ROM_LOAD16_BYTE( "m5", 0x000001, 0x008000, CRC(7e82793e) SHA1(3e487f465d64af8c1c4852567b2fd9190363570c) )

	ROM_REGION( 0x080000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "t17", 0x000000, 0x020000, CRC(599a632a) SHA1(29da423dfe1f971cbb205767cf902d199d968d85) )
	ROM_LOAD16_BYTE( "t16", 0x000001, 0x020000, CRC(3aeef91c) SHA1(a5dc8c22a7bcc1199bdd09c7d0f1f8a378e757c5) )
	ROM_LOAD16_BYTE( "t15", 0x040000, 0x020000, CRC(b97a7b56) SHA1(c08d3586d489947af21f3493356e3a88d79746e8) )
	ROM_LOAD16_BYTE( "t14", 0x040001, 0x020000, CRC(79c707be) SHA1(f67fa40c8f6ab0fbce44997fdfbf699fea1f0df6) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "r28", 0x000000, 0x080000, CRC(a043615d) SHA1(e483fa9fd8e922578a9d7b6ced0750643089ca78) )
	ROM_LOAD( "r27", 0x080000, 0x080000, CRC(cb8425a3) SHA1(655afa295fbe99acc79c4004f03ed832560cff5b) )

	ROM_REGION(0x200, "plds", 0)        // Protection, bruteforced and recreated for GAL16V8
	ROM_LOAD("tl-9", 0x000000, 0x117, BAD_DUMP CRC(3b62882d) SHA1(a590648cb013f20d837f18ddb2e839a89bac5fcb))
ROM_END

ROM_START( thunderla )
	ROM_REGION( 0x010000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "tl-1-1.u1", 0x000000, 0x008000, CRC(3d4b1888) SHA1(9f26e777460e5ab8cf1f6cd97a8df7428f8068f7) )
	ROM_LOAD16_BYTE( "tl-1-2.u4", 0x000001, 0x008000, CRC(974dddda) SHA1(cb685904c7e3b48dee9bf274b1e81d87c9e8f573) )

	ROM_REGION( 0x080000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "t17", 0x000000, 0x020000, CRC(599a632a) SHA1(29da423dfe1f971cbb205767cf902d199d968d85) )
	ROM_LOAD16_BYTE( "t16", 0x000001, 0x020000, CRC(3aeef91c) SHA1(a5dc8c22a7bcc1199bdd09c7d0f1f8a378e757c5) )
	ROM_LOAD16_BYTE( "t15", 0x040000, 0x020000, CRC(b97a7b56) SHA1(c08d3586d489947af21f3493356e3a88d79746e8) )
	ROM_LOAD16_BYTE( "t14", 0x040001, 0x020000, CRC(79c707be) SHA1(f67fa40c8f6ab0fbce44997fdfbf699fea1f0df6) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "r28", 0x000000, 0x080000, CRC(a043615d) SHA1(e483fa9fd8e922578a9d7b6ced0750643089ca78) )
	ROM_LOAD( "r27", 0x080000, 0x080000, CRC(cb8425a3) SHA1(655afa295fbe99acc79c4004f03ed832560cff5b) )

	ROM_REGION(0x200, "plds", 0)        // Protection, bruteforced and recreated for GAL16V8
	ROM_LOAD("tl-9", 0x000000, 0x117, BAD_DUMP CRC(3b62882d) SHA1(a590648cb013f20d837f18ddb2e839a89bac5fcb))
ROM_END

ROM_START( thunderlbl )
	ROM_REGION( 0x010000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "20.g11", 0x000000, 0x008000, CRC(83500006) SHA1(f078e614078296df48bb8b953c3ba88f6f288255) )
	ROM_LOAD16_BYTE( "19.f11", 0x000001, 0x008000, CRC(1bb4cd03) SHA1(1a22bf02f2116b9f01ff01e18ef31497016df0d2) )

	// they ripped the sound CPU program from Tetris!
	ROM_REGION( 0x40000, "audiocpu", 0 ) // sound CPU code
	ROM_LOAD( "21.d5", 0x00000, 0x08000, CRC(bd9ba01b) SHA1(fafa7dc36cc057a50ae4cdf7a35f3594292336f4) )

	ROM_REGION( 0x080000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "25.a10", 0x000000, 0x020000, CRC(599a632a) SHA1(29da423dfe1f971cbb205767cf902d199d968d85) )
	ROM_LOAD16_BYTE( "24.a8",  0x000001, 0x020000, CRC(3aeef91c) SHA1(a5dc8c22a7bcc1199bdd09c7d0f1f8a378e757c5) )
	ROM_LOAD16_BYTE( "23.a5",  0x040000, 0x020000, CRC(b97a7b56) SHA1(c08d3586d489947af21f3493356e3a88d79746e8) )
	ROM_LOAD16_BYTE( "22.a3",  0x040001, 0x020000, CRC(79c707be) SHA1(f67fa40c8f6ab0fbce44997fdfbf699fea1f0df6) )
ROM_END

ROM_START( thunderlbl2 ) // 2 PCB stack, label JK274
	ROM_REGION( 0x010000, "maincpu", 0 )        // 68000 Code, both 27c256, on main PCB
	ROM_LOAD16_BYTE( "g11", 0x000000, 0x008000, CRC(e4842fbd) SHA1(6fc4cded6a7f2e7f331c22323c5b793a7bafdd06) )
	ROM_LOAD16_BYTE( "f11", 0x000001, 0x008000, CRC(b883ab13) SHA1(b835506b97359e3cd9e528d78c6195721be9e878) )

	// they ripped the sound CPU program from Tetris!
	ROM_REGION( 0x40000, "audiocpu", 0 ) // sound CPU code, on main PCB
	ROM_LOAD( "d", 0x00000, 0x08000, CRC(bd9ba01b) SHA1(fafa7dc36cc057a50ae4cdf7a35f3594292336f4) ) // 27c256

	ROM_REGION( 0x080000, "gfx1", 0 )   // Sprites, all 27c010a, on sub PCB
	ROM_LOAD16_BYTE( "a10", 0x000000, 0x020000, CRC(599a632a) SHA1(29da423dfe1f971cbb205767cf902d199d968d85) )
	ROM_LOAD16_BYTE( "a8",  0x000001, 0x020000, CRC(3aeef91c) SHA1(a5dc8c22a7bcc1199bdd09c7d0f1f8a378e757c5) )
	ROM_LOAD16_BYTE( "a5",  0x040000, 0x020000, CRC(b97a7b56) SHA1(c08d3586d489947af21f3493356e3a88d79746e8) )
	ROM_LOAD16_BYTE( "a3",  0x040001, 0x020000, CRC(79c707be) SHA1(f67fa40c8f6ab0fbce44997fdfbf699fea1f0df6) )

	ROM_REGION( 0xc00, "plds", 0 ) // all on sub PCB
	ROM_LOAD( "pal16l8acn.e13", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8acn.e14", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8acn.e15", 0x400, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8acn.f13", 0x600, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8acn.h9",  0x800, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8acn.i3",  0xa00, 0x104, NO_DUMP )
ROM_END

/*

Wiggie Waggie & Super Bar run on a bootleg SETA board with an OKI M6295 replacing the X1-010 sound chip.
Both games are based on Thunder & Lightning code.

PCB:

+--------------------------------------------------------+
| YM3012* YM2151*  2M-1                                  |
|                  2M-2*      6116             6116      |
| VOL  6116 M6295  2M-3*      6116             6116      |
|       A5   16MHz                             6116      |
+-+   Z80A               6264                  6116      |
  |                      6264                            |
+-+                                                      |
|                6116                                    |
|J               6116                                    |
|A                                                       |
|M                                    PAL                |
|M                                       +------+        |
|A   DSW2                                |Actel |        |
|              +-+                       |A1020A|        |
+-+  DSW1      |6|                       |PL84C |        |
  |            |8| 6264                  +------+        |
+-+            |0| 6264           6116     1M-4          |
|              |0|                6116     1M-5          |
|              |0| 512-1          6116     1M-6          |
|              +-+ 512-2          6116     1M-7          |
+--------------------------------------------------------+

  CPU: 68000P10 (8MHz)
       Z80A (4MHz)
Sound: OKI M6295
  OSC: 16MHz
Other: Actel A1020A PLC84C (used for graphics and graphic rom decode)

* Denotes unpopulated sockets

*/

ROM_START( wiggie )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "wiggie.e19", 0x00000, 0x10000, CRC(24b58f16) SHA1(96ef92ab79258da9322dd7e706bf05ac5143f7b7) )
	ROM_LOAD16_BYTE( "wiggie.e21", 0x00001, 0x10000, CRC(83ba6edb) SHA1(fa74fb39599ed877317db73d02d14df5b475fc35) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) // sound CPU code
	ROM_LOAD( "wiggie.a5", 0x00000, 0x10000, CRC(8078d77b) SHA1(4e6855d396a1bace2810b13b7dd08ccf5de89bd8) )

	ROM_REGION( 0x040000, "oki", 0 ) // Samples
	ROM_LOAD( "wiggie.d1", 0x00000, 0x40000, CRC(27fbe12a) SHA1(73f476a03b321ed1ae89104f5b32d77153fabb82) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "wiggie.j16", 0x00000, 0x20000, CRC(4fb40b8a) SHA1(120c9fd677071485a9f8accc2385117baf542b9c) ) // Drawn nude girls for the backgrounds
	ROM_LOAD( "wiggie.j18", 0x20000, 0x20000, CRC(ebc418e9) SHA1(a9af9bebce56608b0533d7d147191ebdceaca4e4) )
	ROM_LOAD( "wiggie.j20", 0x40000, 0x20000, CRC(c073501b) SHA1(4b4cd0fed5efe12bcd10f98a71becc212e7e753a) )
	ROM_LOAD( "wiggie.j21", 0x60000, 0x20000, CRC(22f6fa39) SHA1(d3e86e156434153335c5d2ce71417f35097f5ab7) )
ROM_END

ROM_START( superbar ) // All roms had a "PROMAT" label with no other information.  ROM size was silkscreened on the PCB
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "promat_512-1.e19", 0x00000, 0x10000, CRC(cc7f9e87) SHA1(6c63ee5ac1c145a151a972a2b6bcb29036dad02d) )
	ROM_LOAD16_BYTE( "promat_512-2.e21", 0x00001, 0x10000, CRC(5e8c7231) SHA1(16efbaa871335143490ca897e0573bbbcf16ff16) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) // sound CPU code
	ROM_LOAD( "promat.a5", 0x00000, 0x10000, CRC(8078d77b) SHA1(4e6855d396a1bace2810b13b7dd08ccf5de89bd8) ) // Same as Wiggie Waggie

	ROM_REGION( 0x040000, "oki", 0 ) // Samples
	ROM_LOAD( "promat_2m-1.d1", 0x00000, 0x40000, CRC(27fbe12a) SHA1(73f476a03b321ed1ae89104f5b32d77153fabb82) ) // Same as Wiggie Waggie
	// 2M-2 sample rom not populated
	// 2M-3 sample rom not populated

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "promat_1m-4.j16", 0x00000, 0x20000, CRC(43dbc99f) SHA1(36ac4df9286d8661c61e0dfc9788b936c5596c31) ) // Drawn clothed girls for the backgrounds
	ROM_LOAD( "promat_1m-5.j18", 0x20000, 0x20000, CRC(c09344b0) SHA1(4c54dbc602fa2ccddd232f145d3844a4d145611c) )
	ROM_LOAD( "promat_1m-6.j20", 0x40000, 0x20000, CRC(7d83f8ba) SHA1(55d026a3b98dd0e9a6263a0c913a1d9b6c30cfd1) )
	ROM_LOAD( "promat_1m-7.j21", 0x60000, 0x20000, CRC(734df92a) SHA1(0dfd58a3f47fa8dfa315df7adfad25ade97c2a3b) )
ROM_END

// note the ONLY byte that changes is the year, 1992 instead of 1991.
ROM_START( rezon )
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "us001001.u3",  0x000000, 0x020000, CRC(ab923052) SHA1(26761c228b63c300f635787e63e1276b6e3083f0) )
	ROM_LOAD16_BYTE( "rezon_1_p.u4", 0x000001, 0x020000, CRC(9ed32f8c) SHA1(68b926de4cb5f2632ab78b2cdf7409411fadbb1d) )
	// empty gap
	ROM_LOAD16_BYTE( "us001004.103", 0x100000, 0x020000, CRC(54871c7c) SHA1(2f807b15760b1e712fa69eee6f33cc8a36ee1c02) ) // 1xxxxxxxxxxxxxxxx = 0x00
	ROM_LOAD16_BYTE( "us001003.102", 0x100001, 0x020000, CRC(1ac3d272) SHA1(0f19bc9c19e355dad5b463b0fa33127523bf141b) ) // 1xxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "us001006.u64",  0x000000, 0x080000, CRC(a4916e96) SHA1(bfb63b72273e4fbf0843b3201bb4fddaf54909a7) )
	ROM_LOAD( "us001005.u63",  0x080000, 0x080000, CRC(e6251ebc) SHA1(f02a4c8373e33fc57e18e39f1b5ecff3f6d9ca9e) )

	ROM_REGION( 0x080000, "gfx2", 0 )   // Layer 1
	ROM_LOAD16_WORD_SWAP( "us001007.u66",  0x000000, 0x080000, CRC(3760b935) SHA1(f5fe69f7e93c90a5b6c1dff236402b962821e33f) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x080000, "gfx3", 0 )   // Layer 2
	ROM_LOAD16_WORD_SWAP( "us001008.u68",  0x000000, 0x080000, CRC(0ab73910) SHA1(78e2c0570c5c6f5e1cdb2fbeae73376923127024) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD16_WORD_SWAP( "us001009.u70",  0x000000, 0x100000, CRC(0d7d2e2b) SHA1(cfba19314ecb0a49ed9ff8df32cd6a3fe37ff526) )

	ROM_REGION( 0x800, "plds", 0 )
	ROM_LOAD( "us-010.u14", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "us-011.u35", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "us-012.u36", 0x400, 0x104, NO_DUMP )
	ROM_LOAD( "us-013.u76", 0x600, 0x104, NO_DUMP )
ROM_END

ROM_START( rezono )
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "us001001.u3",  0x000000, 0x020000, CRC(ab923052) SHA1(26761c228b63c300f635787e63e1276b6e3083f0) )
	ROM_LOAD16_BYTE( "us001002.u4",  0x000001, 0x020000, CRC(3dafa0d5) SHA1(80cdff86b99d364acbbf1322c73b2f26b1a93167) )
	// empty gap
	ROM_LOAD16_BYTE( "us001004.103", 0x100000, 0x020000, CRC(54871c7c) SHA1(2f807b15760b1e712fa69eee6f33cc8a36ee1c02) ) // 1xxxxxxxxxxxxxxxx = 0x00
	ROM_LOAD16_BYTE( "us001003.102", 0x100001, 0x020000, CRC(1ac3d272) SHA1(0f19bc9c19e355dad5b463b0fa33127523bf141b) ) // 1xxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "us001006.u64",  0x000000, 0x080000, CRC(a4916e96) SHA1(bfb63b72273e4fbf0843b3201bb4fddaf54909a7) )
	ROM_LOAD( "us001005.u63",  0x080000, 0x080000, CRC(e6251ebc) SHA1(f02a4c8373e33fc57e18e39f1b5ecff3f6d9ca9e) )

	ROM_REGION( 0x080000, "gfx2", 0 )   // Layer 1
	ROM_LOAD16_WORD_SWAP( "us001007.u66",  0x000000, 0x080000, CRC(3760b935) SHA1(f5fe69f7e93c90a5b6c1dff236402b962821e33f) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x080000, "gfx3", 0 )   // Layer 2
	ROM_LOAD16_WORD_SWAP( "us001008.u68",  0x000000, 0x080000, CRC(0ab73910) SHA1(78e2c0570c5c6f5e1cdb2fbeae73376923127024) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD16_WORD_SWAP( "us001009.u70",  0x000000, 0x100000, CRC(0d7d2e2b) SHA1(cfba19314ecb0a49ed9ff8df32cd6a3fe37ff526) )

	ROM_REGION( 0x800, "plds", 0 )
	ROM_LOAD( "us-010.u14", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "us-011.u35", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "us-012.u36", 0x400, 0x104, NO_DUMP )
	ROM_LOAD( "us-013.u76", 0x600, 0x104, NO_DUMP )
ROM_END

ROM_START( stg )
	ROM_REGION( 0x0c0000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "att01003.u27", 0x000000, 0x020000, CRC(7a640a93) SHA1(28c54eca9502d06ca55c2db91bfe7d149af006ed) )
	ROM_LOAD16_BYTE( "att01001.u9",  0x000001, 0x020000, CRC(4fa88ad3) SHA1(55e0e689758511cdf514a633ffe3d7729e281b52) )
	ROM_LOAD16_BYTE( "att01004.u33", 0x040000, 0x020000, CRC(bbd45ca1) SHA1(badb11faf5779e8444dd95eb08a94fbf9f73cc2c) ) // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "att01002.u17", 0x040001, 0x020000, CRC(2f8fd80c) SHA1(b8e16adc84b918b5eee05d032a7841e8d726eeeb) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "att01006.u32", 0x000000, 0x080000, CRC(6ad78ea2) SHA1(eb8fc9833fb1c7041f5e0a3b37c8de9156a034b6) )
	ROM_LOAD( "att01005.u26", 0x080000, 0x080000, CRC(a347ff00) SHA1(8455c5e7dfa25646b1782ab3bcf62fca91ca03ad) )

	ROM_REGION( 0x100000, "gfx2", 0 )   // Layer 1
	ROM_LOAD16_BYTE( "att01008.u39", 0x000000, 0x080000, CRC(20c47457) SHA1(53ddf8c076aa35fb87edc739bc9e9612a5a1526b) ) // FIRST AND SECOND HALF IDENTICAL
	ROM_LOAD16_BYTE( "att01007.u42", 0x000001, 0x080000, CRC(ac975544) SHA1(5cdd2c7aada7179d4bdaf8578134c0ef672a2704) ) // FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "att01009.u47", 0x000000, 0x080000, CRC(4276b58d) SHA1(a2e77dc3295791520c6cb25dea4e910b5a7bc137) )
	ROM_LOAD( "att01010.u55", 0x080000, 0x080000, CRC(fffb2f53) SHA1(0aacb24437e9a6874850313163922d834da27611) )
ROM_END

ROM_START( blandia )
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "ux001001.u3", 0x000000, 0x040000, CRC(2376a1f3) SHA1(705a3c5cc1137d14ffded6c949bf9aa650133eb7) )
	ROM_LOAD16_BYTE( "ux001002.u4", 0x000001, 0x040000, CRC(b915e172) SHA1(e43e50a664dc1286ece42a5ff8629b2da7fb49b4) )
	ROM_LOAD16_WORD_SWAP( "ux001003.u202",    0x100000, 0x100000, CRC(98052c63) SHA1(b523596de29038b3ec9f1b6e1f7374a6a8709d42) )

	ROM_REGION( 0x400000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "ux001005.u200", 0x300000, 0x100000, CRC(bea0c4a5) SHA1(a690c17fb7cbdab533c1dfad13abbad9359b9631) )
	ROM_LOAD( "ux001007.u201", 0x100000, 0x100000, CRC(4440fdd1) SHA1(7bfee90f81a2c867bd487abcf5905393ad400902) )
	ROM_LOAD( "ux001006.u63",  0x200000, 0x100000, CRC(abc01cf7) SHA1(c3f26e75eeb68073d2825be8df82cc6afcfbfb26) )
	ROM_LOAD( "ux001008.u64",  0x000000, 0x100000, CRC(413647b6) SHA1(594e010ca6f49ec82cc6d44fe23ac3427c4c3dbd) )

	ROM_REGION( 0x180000, "gfx2", 0 )   // Layer 1
	ROM_LOAD24_BYTE     ( "ux001009.u65", 0x000000, 0x080000, CRC(bc6f6aea) SHA1(673efa0c70587b5650ccf0a3c4bc316f53d52ba6) )
	ROM_LOAD24_WORD_SWAP( "ux001010.u66", 0x000001, 0x080000, CRC(bd7f7614) SHA1(dc865ff0f327f460956915b2018aaac815e8fce5) )

	ROM_REGION( 0x180000, "gfx3", 0 )   // Layer 2
	ROM_LOAD24_BYTE     ( "ux001011.u67",  0x000000, 0x080000, CRC(5efe0397) SHA1(a294a2dae9a10e93912543a8614a7f960a011f27) )
	ROM_LOAD24_WORD_SWAP( "ux001012.u068", 0x000001, 0x080000, CRC(f29959f6) SHA1(edccea3d0bf972a07edd6339e18792d089033bff) )

	// The c0000-fffff region is bankswitched
	ROM_REGION( 0x200000, "x1snd", 0 )  // Samples
	ROM_LOAD( "ux001013.u69", 0x000000, 0x100000, CRC(5cd273cd) SHA1(602e1f10454e2b1c941f2e6983872bb9ca77a542) )
	ROM_LOAD( "ux001014.u70", 0x100000, 0x080000, CRC(86b49b4e) SHA1(045b352950d848907af4c22b817d154b2cfff382) )

	ROM_REGION( 0xc00, "plds", 0 )
	ROM_LOAD( "ux-015.u206", 0x000, 0x117, CRC(08cddbdd) SHA1(c7330b96375f96406c63abe5d17d02e84828d884) )
	ROM_LOAD( "ux-016.u116", 0x200, 0x117, CRC(9734f1af) SHA1(e7299892a26e9e8ee607ece93cefdee19a13ffae) )
	ROM_LOAD( "ux-017.u14",  0x400, 0x117, CRC(9e95d8d5) SHA1(f7e7250e9aa6fc1c14874230a5b0019704e54c4c) )
	ROM_LOAD( "ux-018.u35",  0x600, 0x117, CRC(c9579473) SHA1(ff65a5ed840b60bd8416ae7e11805635bfcec9ad) )
	ROM_LOAD( "ux-019.u36",  0x800, 0x117, CRC(d85c359d) SHA1(251c1fc833c1be6b15e70240cbb6c997443baea3) )
	ROM_LOAD( "ux-020.u76",  0xa00, 0x117, CRC(116278bf) SHA1(e34bceab30dce17c2a9cb0bf51a5eda0a89da08c) )
ROM_END

ROM_START( blandiap )
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "prg-even.bin", 0x000000, 0x040000, CRC(7ecd30e8) SHA1(25e555a45bbb154170189b065206f7536a5dec1b) )
	ROM_LOAD16_BYTE( "prg-odd.bin",  0x000001, 0x040000, CRC(42b86c15) SHA1(9a4adcc16c35f84826a6effed5ebe439483ab856) )
	ROM_LOAD16_BYTE( "tbl0.bin",     0x100000, 0x080000, CRC(69b79eb8) SHA1(f7b33c99744d8b7f6e2991b4d2b35719eebd0b43) )
	ROM_LOAD16_BYTE( "tbl1.bin",     0x100001, 0x080000, CRC(cf2fd350) SHA1(4d0fb720af544f746eeaaad499be00e0d1c6f129) )

	ROM_REGION( 0x400000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "o-1.bin",  0x000000, 0x080000, CRC(4c67b7f0) SHA1(ad6bd4b880f0f63a803c097795a4b70f24c94848) )
	ROM_LOAD16_BYTE( "o-0.bin",  0x000001, 0x080000, CRC(5e7b8555) SHA1(040599db77041765f582aa99d6f616a7a2c4dd5c) )
	ROM_LOAD16_BYTE( "o-5.bin",  0x100000, 0x080000, CRC(40bee78b) SHA1(1ec0b1854c26ba300a3a54077332a9af55677dca) )
	ROM_LOAD16_BYTE( "o-4.bin",  0x100001, 0x080000, CRC(7c634784) SHA1(047287c630336001d2b1e21f7273ccc3d5278e3c) )
	ROM_LOAD16_BYTE( "o-3.bin",  0x200000, 0x080000, CRC(387fc7c4) SHA1(4e5bd3985f16aa7295110a9902adc8e1453c03ab) )
	ROM_LOAD16_BYTE( "o-2.bin",  0x200001, 0x080000, CRC(c669bb49) SHA1(db5051ea8b08672b6079004060e20fb250560d9f) )
	ROM_LOAD16_BYTE( "o-7.bin",  0x300000, 0x080000, CRC(fc77b04a) SHA1(b3c7b2cb9407cac261890e0355cbb87ac8e2e93c) )
	ROM_LOAD16_BYTE( "o-6.bin",  0x300001, 0x080000, CRC(92882943) SHA1(460f3ae37d6f88d3a6068e2fb8d0d330be7c786f) )

	ROM_REGION( 0x0c0000, "gfx2", 0 )   // Layer 1
	ROM_LOAD24_BYTE( "v1-2.bin",  0x000000, 0x020000, CRC(d524735e) SHA1(4d17e7896b6c6451effe8a19bf7a4919db0cc06d) )
	ROM_LOAD24_BYTE( "v1-1.bin",  0x000001, 0x020000, CRC(09bdf75f) SHA1(33bda046092d5bc3d8e8ffec25c745a2fda16a5c) )
	ROM_LOAD24_BYTE( "v1-0.bin",  0x000002, 0x020000, CRC(73617548) SHA1(9c04d0179cb93e9fb78cc8af1006ef3edfcde707) )
	ROM_LOAD24_BYTE( "v1-5.bin",  0x060000, 0x020000, CRC(eb440cdb) SHA1(180ed9d616c66b7fae1a3d1156028c8476e45bde) )
	ROM_LOAD24_BYTE( "v1-4.bin",  0x060001, 0x020000, CRC(803911e5) SHA1(a93cac42eda69698b393a1a49e3615ee60868838) )
	ROM_LOAD24_BYTE( "v1-3.bin",  0x060002, 0x020000, CRC(7f18e4fb) SHA1(0e51e3f88b90c07a1352bcd42e6438c947d4856e) )

	ROM_REGION( 0x0c0000, "gfx3", 0 )   // Layer 2
	ROM_LOAD24_BYTE( "v2-2.bin",  0x000000, 0x020000, CRC(c4f15638) SHA1(6be0f3e90ab23189caadfd99b3e4ded74749ebbc) )   // identical to v2-1
	ROM_LOAD24_BYTE( "v2-1.bin",  0x000001, 0x020000, CRC(c4f15638) SHA1(6be0f3e90ab23189caadfd99b3e4ded74749ebbc) )
	ROM_LOAD24_BYTE( "v2-0.bin",  0x000002, 0x020000, CRC(5b05eba9) SHA1(665001cdb3c9977f8f4c7ce551549f7fc640c6a9) )
	ROM_LOAD24_BYTE( "v2-5.bin",  0x060000, 0x020000, CRC(c2e57622) SHA1(994a4774d68f2d562d985951b06216d59f38afe9) )
	ROM_LOAD24_BYTE( "v2-4.bin",  0x060001, 0x020000, CRC(16ec2130) SHA1(187f548563577ca36cced9ae184d27e6fcdd7e6a) )
	ROM_LOAD24_BYTE( "v2-3.bin",  0x060002, 0x020000, CRC(80ad0c3b) SHA1(00fcbcf7805784d7298b92136e7f256d65029c44) )

	// The c0000-fffff region is bankswitched
	ROM_REGION( 0x200000, "x1snd", 0 )  // Samples
	ROM_LOAD( "s-0.bin",  0x000000, 0x020000, CRC(a5fde408) SHA1(89efcd37ef6c5b313169d74a962a7c074a09b12a) )
	ROM_CONTINUE(         0x100000, 0x020000  )
	ROM_LOAD( "s-1.bin",  0x020000, 0x020000, CRC(3083f9c4) SHA1(f5d2297c3d680eb1f128fa42a3a7f61badb9853a) )
	ROM_CONTINUE(         0x120000, 0x020000  )
	ROM_LOAD( "s-2.bin",  0x040000, 0x020000, CRC(a591c9ef) SHA1(83e665e342c42fd3582c83becfacc27a3a3e5a54) )
	ROM_CONTINUE(         0x140000, 0x020000  )
	ROM_LOAD( "s-3.bin",  0x060000, 0x020000, CRC(68826c9d) SHA1(a860b7b2140a5a506bf25110c08c6ea59db25743) )
	ROM_CONTINUE(         0x160000, 0x020000  )
	ROM_LOAD( "s-4.bin",  0x080000, 0x020000, CRC(1c7dc8c2) SHA1(006459a23de83fe48e11bdd6ebe23ef6a18a87e8) )
	ROM_CONTINUE(         0x180000, 0x020000  )
	ROM_LOAD( "s-5.bin",  0x0a0000, 0x020000, CRC(4bb0146a) SHA1(1e3c1739ea3c85296573426e55f25dce11f0ed2b) )
	ROM_CONTINUE(         0x1a0000, 0x020000  )
	ROM_LOAD( "s-6.bin",  0x0c0000, 0x020000, CRC(9f8f34ee) SHA1(60abb70ae87595ebae23df68d62f3b0ed4a2e768) )
	ROM_CONTINUE(         0x1c0000, 0x020000  ) // this half is 0
	ROM_LOAD( "s-7.bin",  0x0e0000, 0x020000, CRC(e077dd39) SHA1(a6f0881a026161710adc132bcf7cb95c4c8f2528) )
	ROM_CONTINUE(         0x1e0000, 0x020000  ) // this half is 0
ROM_END

ROM_START( blockcar )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "u1.a1",  0x000000, 0x020000, CRC(4313fb00) SHA1(f5b9e212436282284fa344e1c4200bc38ca3c50a) )
	ROM_LOAD16_BYTE( "u4.a3",  0x000001, 0x020000, CRC(2237196d) SHA1(5a9d972fac94e62f026c36bca0c2f5fe8e0e1a1d) )

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "bl-chr-0.u6.j3",  0x000000, 0x080000, CRC(a33300ca) SHA1(b0a7ccb77c3e8e33c12b83e254924f30209a4c2c) )
	ROM_LOAD( "bl-chr-1.u9.l3",  0x080000, 0x080000, CRC(563de808) SHA1(40b2f9f4a4cb1a019f6419572ee21d66dda7d4af) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "bl-snd-0.u39.a13",  0x000000, 0x100000, CRC(9c2130a2) SHA1(ef051528c3e37b61298f03a9d4a2649f0528dcfa) ) // 4 MBit silkscreen on PCB but it's actually double that
ROM_END

ROM_START( blockcarb )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "tl2.bin",  0x000000, 0x020000, CRC(049d0565) SHA1(4c4d2838336556aa486d8990d038f9ed9f021cfd) )
	ROM_LOAD16_BYTE( "tl1.bin",  0x000001, 0x020000, CRC(b0011882) SHA1(417d5d6d648cc121a1d26071806f14dc37930870) )

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "tl4.bin",  0x000000, 0x040000, CRC(6c4d53af) SHA1(69ed3b8374efd6f55c8ccf5f1db5c03a24cac9d6) )
	ROM_LOAD16_BYTE( "tl3.bin",  0x000001, 0x040000, CRC(bf4abe64) SHA1(78439f2f088b38c454cf3db7372175a5dc22b6a0) )
	ROM_LOAD16_BYTE( "tl6.bin",  0x080000, 0x040000, CRC(6d49fff2) SHA1(676de504be18ba0832000678846eb4527414a36d) )
	ROM_LOAD16_BYTE( "tl5.bin",  0x080001, 0x040000, CRC(9369e8dc) SHA1(645ae72a8b49ec43c26cdee5b6cb8cca5f46e542) )

	ROM_REGION( 0x100000, "oki", 0 )  // 6295 samples
	ROM_LOAD( "tl7.bin",  0x000000, 0x040000, CRC(41e899dc) SHA1(36c8161dcb68cdc312c7d1177dbcfb9b62b18f05) )    // == so2_09.12b  mercs      Mercs (World 900302)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "tl8.bin",  0x000000, 0x010000, CRC(d09d7c7a) SHA1(8e8532be08818c855d9c3ce45716eb07cfab5767) )    //cpu prg
ROM_END

ROM_START( qzkklogy )
	ROM_REGION( 0x0c0000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "3.u27", 0x000000, 0x020000, CRC(b8c27cde) SHA1(4c36076801b6c915888b925c1e37d772bab1bb02) )
	ROM_LOAD16_BYTE( "1.u9",  0x000001, 0x020000, CRC(ce01cd54) SHA1(ef91aecdf7b5586a6870ff237372d65f85cd4cd3) )
	ROM_LOAD16_BYTE( "4.u33", 0x040000, 0x020000, CRC(4f5c554c) SHA1(0a10cefdf2dd876e6cb78023c3c15af24ba3c39a) )
	ROM_LOAD16_BYTE( "2.u17", 0x040001, 0x020000, CRC(65fa1b8d) SHA1(81fd7785f138a189de978fd30dbfca36687cda17) )

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "t2709u32.u32", 0x000000, 0x080000, CRC(900f196c) SHA1(b60741c3242ce56cb61ea68093b571489db0c6fa) ) // FIRST AND SECOND HALF IDENTICAL
	ROM_LOAD( "t2709u26.u26", 0x080000, 0x080000, CRC(416ac849) SHA1(3bd5dd13a8f2693e8f160a4ecfff3b7610644f5f) ) // FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x100000, "gfx2", 0 )   // Layer 1
	ROM_LOAD16_BYTE( "t2709u42.u39", 0x000000, 0x080000, CRC(194d5704) SHA1(ab2833f7427d0608850c158b813bc49935ac7d6d) )
	ROM_LOAD16_BYTE( "t2709u39.u42", 0x000001, 0x080000, CRC(6f95a76d) SHA1(925f5880fb5153c1215d1f5ee1eff5b53a84abea) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "t2709u47.u47", 0x000000, 0x080000, CRC(0ebdad40) SHA1(6558eeaac76d98d91b0be6faa78f531f1e3b9f84) )
	ROM_LOAD( "t2709u55.u55", 0x080000, 0x080000, CRC(43960c68) SHA1(9a1901b65f989aa57ab8736ef0be3bac492c081c) )
ROM_END

ROM_START( umanclub )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "uw001006.u48", 0x000000, 0x020000, CRC(3dae1e9d) SHA1(91a738c299d134d198bad648383be87345f4f475) )
	ROM_LOAD16_BYTE( "uw001007.u49", 0x000001, 0x020000, CRC(5c21e702) SHA1(c69e9dd7dfac82f116885610f90878f865e629b3) )

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "bp-u-002.u2", 0x000000, 0x080000, CRC(936cbaaa) SHA1(f7932ee310eb792b2776ae8a9d29e1a492761b11) )
	ROM_LOAD( "bp-u-001.u1", 0x080000, 0x080000, CRC(87813c48) SHA1(7ec9b08fe0490d277c531e2b6394862df4d5678d) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "uw003.u13", 0x000000, 0x100000, CRC(e2f718eb) SHA1(fd085b68f76c8778816a1b7d47783b9dc20bff12) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "bp-u-004.u30", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "bp-u-005.u32", 0x200, 0x104, NO_DUMP )
ROM_END

ROM_START( zingzip )
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "uy001001.3",  0x000000, 0x040000, CRC(1a1687ec) SHA1(c840752dd87d8c1c30e6b31452173148e20538b1) )
	ROM_LOAD16_BYTE( "uy001002.4",  0x000001, 0x040000, CRC(62e3b0c4) SHA1(51a27fbf68a142dd132157bed1dc22acda3fa044) )

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "uy001006.64",  0x000000, 0x080000, CRC(46e4a7d8) SHA1(2c829e52d9aead351702335bf06aa0f337528306) )
	ROM_LOAD( "uy001005.63",  0x080000, 0x080000, CRC(4aac128e) SHA1(3ac64c84a40f86e29e33a218babcd21cae6dbfdb) )

	ROM_REGION( 0x200000, "gfx2", 0 )   // Layer 1
	ROM_LOAD24_WORD_SWAP( "uy001008.66", 0x000001, 0x100000, CRC(1dff7c4b) SHA1(94f581f4aae1ef417dce6e62a611a523205e8c27) ) // FIRST AND SECOND HALF IDENTICAL
	ROM_LOAD24_BYTE     ( "uy001007.65", 0x000000, 0x080000, CRC(ec5b3ab9) SHA1(e82fb050ae4e2486e43418fcb5fa726d92c5cd21) )

	ROM_REGION( 0x200000, "gfx3", 0 )   // Layer 2
	ROM_LOAD16_WORD_SWAP( "uy001010.68", 0x000000, 0x100000, CRC(bdbcdf03) SHA1(857f541697f76086ac6c761a3505678a3d3499df) ) // FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "uy001011.70",  0x000000, 0x100000, CRC(bd845f55) SHA1(345b79cfcd8c924d6ba365814286e518438f10bc) ) // uy001017 + uy001018

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "uy-012.u206", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "uy-013.u14",  0x200, 0x104, NO_DUMP )
	ROM_LOAD( "uy-014.u35",  0x400, 0x104, NO_DUMP )
	ROM_LOAD( "uy-015.u36",  0x600, 0x104, NO_DUMP )
	ROM_LOAD( "uy-016.u76",  0x800, 0x104, NO_DUMP )
ROM_END

ROM_START( zingzipbl )
	ROM_REGION( 0x80000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "prg9.bin",   0x000000, 0x040000, CRC(bf47a8cf) SHA1(87ef35c2dc4d25bbd90cd7528616d06362b20fc8) )
	ROM_LOAD16_BYTE( "prg10.bin",  0x000001, 0x040000, CRC(561501ba) SHA1(f9d488b6d6b313e543738905f11ebbc5f644eb09) )

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "17",  0x000000, 0x040000,  CRC(2d59ce97) SHA1(c24f863057721bd568caff6d9e3b4abd235f92fc) )
	ROM_LOAD16_BYTE( "18",  0x000001, 0x040000,  CRC(4e23144a) SHA1(1c4543687e693e7e9bf3a5790cb6e7458571964f) )
	ROM_LOAD16_BYTE( "19",  0x080000, 0x040000,  CRC(101beade) SHA1(2a7261583eb7326fbb50aa48fe5f0bc50e7a5180) )
	ROM_LOAD16_BYTE( "20",  0x080001, 0x040000,  CRC(ebff804d) SHA1(a0fc4ed6104cfc17c33697ff8ae75949c2e9945e) )

	ROM_REGION( 0x200000, "gfxtemp", 0 )    // Layer 1 + 2 combined (4bpp data)
	ROM_LOAD16_BYTE( "11",  0x000000, 0x080000,  CRC(2f3b292d) SHA1(931abc0b7570b32e41a11555c9d55a67cfdcd1df) )
	ROM_LOAD16_BYTE( "12",  0x000001, 0x080000,  CRC(b9d1cb25) SHA1(45cab6c2fb459f78ab9177f64e5c5039cbaa9e09) )
	ROM_LOAD16_BYTE( "13",  0x100000, 0x080000,  CRC(cabc66d9) SHA1(cf1777eb95822cd705edf9b7e4b2d4d6e75f33cf) )
	ROM_LOAD16_BYTE( "14",  0x100001, 0x080000,  CRC(fefad62f) SHA1(13aaf6cc6af4b42a1184f3fc6c07d9d966153dc1) )

	ROM_REGION( 0x180000, "gfx2", 0 )   // Layer 1
	ROM_COPY( "gfxtemp", 0x000000, 0x000000, 0x80000 )
	ROM_COPY( "gfxtemp", 0x100000, 0x080000, 0x80000 )
	// 2bpp of extra planes for this layer
	ROM_LOAD16_BYTE( "15",  0x100000, 0x040000, CRC(af7a786f) SHA1(de67960f529ebfff0f1d55c79912685f9eca9623) )
	ROM_LOAD16_BYTE( "16",  0x100001, 0x040000, CRC(06dee8f3) SHA1(8c5f489e53bc10e2bad9f98445328e2ec0eac7d2) )

	ROM_REGION( 0x100000, "gfx3", 0 )   // Layer 2
	ROM_COPY( "gfxtemp", 0x080000, 0x000000, 0x80000 )
	ROM_COPY( "gfxtemp", 0x180000, 0x080000, 0x80000 )

	ROM_REGION( 0x40000, "oki", 0 )    // OKI Samples - Not Seta
	ROM_LOAD( "8", 0x00000, 0x40000, BAD_DUMP CRC(7927a200) SHA1(fd6163d2867959ec14b418d6207ae024afd3b654) ) // BADADDR      xxxxxxxxxxxxxxx-xx
ROM_END

ROM_START( atehate )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "fs001001.evn", 0x000000, 0x080000, CRC(4af1f273) SHA1(79b28fe768aa634c31ee4e7687e62ebe78cf4014) )
	ROM_LOAD16_BYTE( "fs001002.odd", 0x000001, 0x080000, CRC(c7ca7a85) SHA1(1221f57d4aa3d2cb6662bc059978eafd65c1858f) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "fs001003.gfx", 0x000000, 0x200000, CRC(8b17e431) SHA1(643fc62d5bad9941630ab621ecb3c69ded9d4536) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "fs001004.pcm", 0x000000, 0x100000, CRC(f9344ce5) SHA1(cffbc235f3a8e9a5004e671d924affd321ec9eed) )
ROM_END

/*
The changes between the set daioh and daioha are very minimal, the main game effects are:

 - Fixes the crashing bug in the US version caused by pressing Shot1 and Shot2 in weird orders and timings.
 - 1UP, and 2UPs no longer spawn "randomly". (Only the fixed extend items exist, and the 1UPs from score)
 - After picking up a max powerup, a 1UP or a 2UP, daoiha sets the "item timer" to a random value.
   daioh always sets it to 0x7F.
 - The powerups spawned from picking up an additional max powerup are no longer random, but feeds from the
   original "spawn item" function (thus, it advances the "item timer")

So it's a bug fix version which also makes the game a little harder by limiting the spawning of 1ups
*/

ROM_START( daioh )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "fg001001.u3",  0x000000, 0x080000, CRC(e1ef3007) SHA1(864349efac3e3dc3ccdeb892fed285c73aea3997) )
	ROM_LOAD16_BYTE( "fg001002.u4",  0x000001, 0x080000, CRC(5e3481f9) SHA1(7585a7e56392fc2b13d466cf262383dd68d6d995) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "fg-001-004", 0x000000, 0x100000, CRC(9ab0533e) SHA1(b260ceb2b3e140971419329bee07a020171794f7) )
	ROM_LOAD( "fg-001-003", 0x100000, 0x100000, CRC(1c9d51e2) SHA1(1d6236ab28d11676386834fd6e405fd40198e924) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // Layer 1
	ROM_LOAD16_WORD_SWAP( "fg-001-005",  0x000000, 0x200000, CRC(c25159b9) SHA1(4c9da3233223508389c3c0f277a00aedfc860da4) )

	ROM_REGION( 0x200000, "gfx3", 0 ) // Layer 2
	ROM_LOAD16_WORD_SWAP( "fg-001-006",  0x000000, 0x200000, CRC(2052c39a) SHA1(83a444a76e68aa711b0e25a5aa963ca876a6357e) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "fg-001-007",  0x000000, 0x100000, CRC(4a2fe9e0) SHA1(e55b6f301f842ff5d3c7a0041856695ac1d8a78f) )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "fg-008.u206", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "fg-009.u14",  0x200, 0x104, NO_DUMP )
	ROM_LOAD( "fg-010.u35",  0x400, 0x104, NO_DUMP )
	ROM_LOAD( "fg-011.u36",  0x600, 0x104, NO_DUMP )
	ROM_LOAD( "fg-012.u76",  0x800, 0x104, NO_DUMP )
ROM_END

ROM_START( daioha )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "fg-001-001.u3",  0x000000, 0x080000, CRC(104ae74a) SHA1(928c467e3ff98285a4828a927d851fcdf296849b) )
	ROM_LOAD16_BYTE( "fg-001-002.u4",  0x000001, 0x080000, CRC(e39a4e67) SHA1(c3f47e9d407f32dbfaf209d29b4446e4de8829a2) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "fg-001-004", 0x000000, 0x100000, CRC(9ab0533e) SHA1(b260ceb2b3e140971419329bee07a020171794f7) )
	ROM_LOAD( "fg-001-003", 0x100000, 0x100000, CRC(1c9d51e2) SHA1(1d6236ab28d11676386834fd6e405fd40198e924) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // Layer 1
	ROM_LOAD16_WORD_SWAP( "fg-001-005",  0x000000, 0x200000, CRC(c25159b9) SHA1(4c9da3233223508389c3c0f277a00aedfc860da4) )

	ROM_REGION( 0x200000, "gfx3", 0 ) // Layer 2
	ROM_LOAD16_WORD_SWAP( "fg-001-006",  0x000000, 0x200000, CRC(2052c39a) SHA1(83a444a76e68aa711b0e25a5aa963ca876a6357e) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "fg-001-007",  0x000000, 0x100000, CRC(4a2fe9e0) SHA1(e55b6f301f842ff5d3c7a0041856695ac1d8a78f) )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "fg-008.u206", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "fg-009.u14",  0x200, 0x104, NO_DUMP )
	ROM_LOAD( "fg-010.u35",  0x400, 0x104, NO_DUMP )
	ROM_LOAD( "fg-011.u36",  0x600, 0x104, NO_DUMP )
	ROM_LOAD( "fg-012.u76",  0x800, 0x104, NO_DUMP )
ROM_END

ROM_START( daiohp ) // Found on the same P0-072-2 PCB as the Blandia prototype
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "prg_even.u3",    0x000000, 0x040000, CRC(3c97b976) SHA1(5850bf71b594a25f3e2de16f2933078c4a0dc518) )
	ROM_LOAD16_BYTE( "prg_odd.u4",     0x000001, 0x040000, CRC(aed2b87e) SHA1(d5b81614fbbda8a75418e69eb481e5adf38b4ebf) )
	ROM_LOAD16_BYTE( "data_even.u103", 0x100000, 0x040000, CRC(e07776ef) SHA1(5e75dd35fd8eae98182a9798a8b3eceb3e33b780) )
	ROM_LOAD16_BYTE( "data_odd.u102",  0x100001, 0x040000, CRC(b75b9a5c) SHA1(4c187105fe5253cc86862df1f3970fa45d4f7317) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "obj_1.u140",  0x000000, 0x040000, CRC(8ff6c5a9) SHA1(a2d188d44c8671282bf53f7927e099a212c0ed51) )
	ROM_LOAD16_BYTE( "obj_0.u142",  0x000001, 0x040000, CRC(78f45582) SHA1(021e635ba365558d9bf37a3b33b4c42b63119f0c) )
	ROM_LOAD16_BYTE( "obj_5.u141",  0x080000, 0x040000, CRC(6a671757) SHA1(aa6c2f916f1ca70514f1bb5754545171d8991456) )
	ROM_LOAD16_BYTE( "obj_4.u143",  0x080001, 0x040000, CRC(d387de72) SHA1(22f40a2daa98e52d6990aa52f9fde2cd66ad40d8) )
	ROM_LOAD16_BYTE( "obj_3.u144",  0x100000, 0x040000, CRC(d33ca640) SHA1(3d278cb46f2eabd03851ee470adfae5313988a27) )
	ROM_LOAD16_BYTE( "obj_2.u146",  0x100001, 0x040000, CRC(77560a03) SHA1(f766b56a88d49e4b41c9ed3c68e5478991033b5b) )
	ROM_LOAD16_BYTE( "obj_7.u145",  0x180000, 0x040000, CRC(e878ac92) SHA1(fc67cbefb050bfbc96f3350bb3d76bf0206e6553) )
	ROM_LOAD16_BYTE( "obj_6.u147",  0x180001, 0x040000, CRC(081f5fb1) SHA1(2fc6816704f7c42627ec47edd0e2ea88e7088101) )

	ROM_REGION( 0x200000, "gfx2", 0 )   // Layer 1
	ROM_LOAD16_BYTE( "bg1_0.u148",  0x000001, 0x080000, CRC(bec48d7a) SHA1(9fdcc8f461e48cb4244827bead980ad48acdfbd8) )
	ROM_LOAD16_BYTE( "bg1_1.u150",  0x000000, 0x080000, CRC(d5793a2f) SHA1(0623d51d405fde69622f1e15512fd8fc41209a59) )
	ROM_LOAD16_BYTE( "bg1_2.u149",  0x100001, 0x080000, CRC(5e674c30) SHA1(8f2e264df7d0b4f2a5a54d86dd0b3106d0ff7c15) )
	ROM_LOAD16_BYTE( "bg1_3.u151",  0x100000, 0x080000, CRC(6456fae1) SHA1(ce839e68342b62be61e29255ebdd8ddbd2b67a71) )

	ROM_REGION( 0x200000, "gfx3", 0 )   // Layer 2
	ROM_LOAD16_BYTE( "bg2_0.u164",  0x000001, 0x080000, CRC(7e46a10e) SHA1(a8576f7a140b065b88a0dab648f7b31c75fec006) )
	ROM_LOAD16_BYTE( "bg2_1.u166",  0x000000, 0x080000, CRC(9274123b) SHA1(b58e107a5bd222e454fd435d515e57cab52e6593) )
	ROM_LOAD16_BYTE( "bg2_2.u165",  0x100001, 0x080000, CRC(3119189b) SHA1(3a45ec8db30659d7fd47090cb137df05bbdc1c86) )
	ROM_LOAD16_BYTE( "bg2_3.u167",  0x100000, 0x080000, CRC(d3d68aa1) SHA1(14b0e4fd9bbdc2b6a99147dd6f6143d609d9110b) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "snd0.u156",  0x000000, 0x020000, CRC(4d253547) SHA1(87cda11dc86bc121cb8fb0e574006c3627158f51) )
	ROM_LOAD( "snd1.u157",  0x020000, 0x020000, CRC(79b56e22) SHA1(4b6c62e96dc1e8fb6dc0a76c505f9d805ef4684f) )
	ROM_LOAD( "snd2.u158",  0x040000, 0x020000, CRC(bc8de02a) SHA1(503c2c9f9ce029701e6a5b134d9407ab06e28913) )
	ROM_LOAD( "snd3.u159",  0x060000, 0x020000, CRC(939777fd) SHA1(3dd1b89a4f81f745c68037c568c885fe1403ed31) )
	ROM_LOAD( "snd4.u160",  0x080000, 0x020000, CRC(7b97716d) SHA1(6693e81dc008317c6a985558624f5d5cf00785e9) )
	ROM_LOAD( "snd5.u161",  0x0a0000, 0x020000, CRC(294e1cc9) SHA1(5faef5eb9f15c23686c2f66646c6f6724e7c611f) )
	ROM_LOAD( "snd6.u162",  0x0c0000, 0x020000, CRC(ecab073b) SHA1(f991fb9d9d4ffe24b67b233850ef0727dc6329b6) )
	ROM_LOAD( "snd7.u163",  0x0e0000, 0x020000, CRC(1b7ea768) SHA1(7dfa8cbcb839c76f3f9eefd6abbc2b424c3d970a) )

	ROM_REGION( 0xc00, "pals", 0 )
	ROM_LOAD( "con1x.u35",  0x000000, 0x104, CRC(ce8b57d9) SHA1(e433a8cee4f964123595f904170793e152290be1) )
	ROM_LOAD( "con2x.u36",  0x000200, 0x104, CRC(0b18db9e) SHA1(80e6aacb1455e15c6e665feaec8711070c14a901) )
	ROM_LOAD( "dec1x.u14",  0x000400, 0x104, CRC(d197abfe) SHA1(93f08d879c339ec00598383723912d7d0eab306c) )
	ROM_LOAD( "dec2x.u206", 0x000600, 0x104, CRC(35afbba8) SHA1(ce1cc0f75467a1ce6444250d741e70c2ed8d4c14) )
	ROM_LOAD( "pcon2.u110", 0x000800, 0x104, CRC(082882c2) SHA1(78385047ed8b1e2c11926c5ce8dea40450b0d0b0) )
	ROM_LOAD( "sc.u116",    0x000a00, 0x104, CRC(e57bfde9) SHA1(33632d007c8e48d756fc920985f82ae32dcd63e6) )
ROM_END

ROM_START( daiohp2 ) // Found on the same P0-072-2 PCB as the previous Daioh prototype
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )        // 68000 Code
	ROM_LOAD16_BYTE( "prg_even.u3",    0x000000, 0x020000, CRC(0079c08f) SHA1(6353c06ec24c9ed28c34c7023557b63471ca2514) )
	ROM_LOAD16_BYTE( "prg_odd.u4",     0x000001, 0x020000, CRC(d2a843ad) SHA1(1a867740227cbbbf7783cad9de1938508a21e8d3) )
	ROM_LOAD16_BYTE( "data_even.u103", 0x100000, 0x040000, CRC(a76139bb) SHA1(684c949ac4b652c645ab61be7acb2821fe1b6c8d) )
	ROM_LOAD16_BYTE( "data_odd.u102",  0x100001, 0x040000, CRC(075c4b30) SHA1(6acd4f21fe06bc4864e87f3174b64d73e33cdf22) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "obj_1.u140", 0x000000, 0x080000, CRC(01f12e59) SHA1(8741916643df8f723e1151a966a9e1436ba7b336) )
	ROM_LOAD16_BYTE( "obj_0.u142", 0x000001, 0x080000, CRC(361d47ae) SHA1(9d76e64087d5193f79036a934fc87387d2909212) )
	ROM_LOAD16_BYTE( "obj_3.u144", 0x100000, 0x080000, CRC(68b5be19) SHA1(e057773b83f721411782fc275e2cc1e586dfe090) )
	ROM_LOAD16_BYTE( "obj_2.u146", 0x100001, 0x080000, CRC(85f5a720) SHA1(c1eadb112192b9a5bd5b1efb67c756847b3dc191) )

	ROM_REGION( 0x200000, "gfx2", 0 )   // Layer 1
	ROM_LOAD16_BYTE( "bg1_0.u148", 0x000001, 0x080000, CRC(bec48d7a) SHA1(9fdcc8f461e48cb4244827bead980ad48acdfbd8) )
	ROM_LOAD16_BYTE( "bg1_1.u150", 0x000000, 0x080000, CRC(d5793a2f) SHA1(0623d51d405fde69622f1e15512fd8fc41209a59) )
	ROM_LOAD16_BYTE( "bg1_2.u149", 0x100001, 0x080000, CRC(85761988) SHA1(5602052f5fed5afcedc257b0ee07b4eca25f87cb) )
	ROM_LOAD16_BYTE( "bg1_3.u151", 0x100000, 0x080000, CRC(f6912766) SHA1(d919f679138bbc2dd7d79d814c220193024c769b) )

	ROM_REGION( 0x200000, "gfx3", 0 )   // Layer 2
	ROM_LOAD16_BYTE( "bg2_0.u164", 0x000001, 0x080000, CRC(7e46a10e) SHA1(a8576f7a140b065b88a0dab648f7b31c75fec006) )
	ROM_LOAD16_BYTE( "bg2_1.u166", 0x000000, 0x080000, CRC(9274123b) SHA1(b58e107a5bd222e454fd435d515e57cab52e6593) )
	ROM_LOAD16_BYTE( "bg2_2.u165", 0x100001, 0x080000, CRC(dc8ecfb7) SHA1(a202ff32c74601d5cd0aebdf84a481d36f540403) )
	ROM_LOAD16_BYTE( "bg2_3.u167", 0x100000, 0x080000, CRC(533ba782) SHA1(b5f62323be95b2def8d1383b400b4ef0d3b3d6cd) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "se_0.u69", 0x000000, 0x080000, CRC(21e4f093) SHA1(f0420d158dc5d182e41b6fb2ea3af6baf88bacb8) )
	ROM_LOAD( "se_1.u70", 0x080000, 0x080000, CRC(593c3c58) SHA1(475fb530a6d23269cb0aea6e294291c7463b57a2) )

	ROM_REGION( 0xc00, "pals", 0 )
	ROM_LOAD( "fa-023.u35",  0x000000, 0x117, CRC(f187ea2d) SHA1(d2f05b42c0bbc6dc711c525b2a63d4de3ac9de03) )
	ROM_LOAD( "fa-024.u36",  0x000200, 0x117, CRC(02c87697) SHA1(5ff985ba88f4de677cf13626c95eee0b59fbb96a) )
	ROM_LOAD( "fa-022.u14",  0x000400, 0x117, CRC(f780fd0e) SHA1(58513fdef8bff5bb32f7de04d2d5f1446c66d108) )
	ROM_LOAD( "fa-020.u206", 0x000600, 0x117, CRC(cd2cd02c) SHA1(150fdacfc44ea5a2f61c1cf626011d43b75ad618) )
	ROM_LOAD( "fa-025.u76",  0x000800, 0x117, CRC(875c0c81) SHA1(8c259b75f40bf8ad2971648e4bd3284ef5da30d5) )
	ROM_LOAD( "fa-021.u116", 0x000a00, 0x117, CRC(e335cf2e) SHA1(35f6fa2fb2da1dc5b1fad93f44947f76d6ef35aa) )
ROM_END

ROM_START( daiohp3 ) // P0-072-2 PCB
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )        // 68000 Code
	ROM_LOAD16_BYTE( "prg0even.u3",    0x000000, 0x020000, CRC(a69bceaa) SHA1(45863eccf32cf2374769ad4517678eb74eaca590) )
	ROM_LOAD16_BYTE( "prg0odd.u4",     0x000001, 0x020000, CRC(e3462ad8) SHA1(a5a1b0d79bc300a7bcc48fa39a750a0a060293ba) )
	ROM_LOAD16_BYTE( "prg1even.u103",  0x100001, 0x040000, NO_DUMP ) // unfortunately this set misses one interleaved ROM
	ROM_LOAD16_BYTE( "prg1odd.u102",   0x100001, 0x040000, CRC(1d6dbc45) SHA1(d626220cfbce5df0d83783c88443f3816a432434) )
	ROM_LOAD16_BYTE( "data_even.u103", 0x100000, 0x040000, BAD_DUMP CRC(e07776ef) SHA1(5e75dd35fd8eae98182a9798a8b3eceb3e33b780) )  // so to make it show something load the ones from daiohp until there's a good dump for this set
	ROM_LOAD16_BYTE( "data_odd.u102",  0x100001, 0x040000, BAD_DUMP CRC(b75b9a5c) SHA1(4c187105fe5253cc86862df1f3970fa45d4f7317) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "o_1.u140",  0x000000, 0x040000, CRC(8ff6c5a9) SHA1(a2d188d44c8671282bf53f7927e099a212c0ed51) )
	ROM_LOAD16_BYTE( "o_0.u142",  0x000001, 0x040000, CRC(78f45582) SHA1(021e635ba365558d9bf37a3b33b4c42b63119f0c) )
	ROM_LOAD16_BYTE( "o_5.u141",  0x080000, 0x040000, CRC(a00e2b63) SHA1(df5a6c4948a8a9f56d9d20b9e65755694f229718) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "o_4.u143",  0x080001, 0x040000, CRC(7f43b8b2) SHA1(665b8bebfa6bf6ed42986b2210dfb2c4cd06e3f1) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "o_3.u144",  0x100000, 0x040000, CRC(d33ca640) SHA1(3d278cb46f2eabd03851ee470adfae5313988a27) )
	ROM_LOAD16_BYTE( "o_2.u146",  0x100001, 0x040000, CRC(77560a03) SHA1(f766b56a88d49e4b41c9ed3c68e5478991033b5b) )
	ROM_LOAD16_BYTE( "o_7.u145",  0x180000, 0x040000, CRC(c16df1c5) SHA1(0babac313827168b3a78209e568b57ff45e34930) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "o_6.u147",  0x180001, 0x040000, CRC(06f1ccca) SHA1(eb79aea88ccefd461c2659094f6483d557237614) ) // 1xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "gfx2", 0 )   // Layer 1
	ROM_LOAD16_BYTE( "v1_0.u148", 0x000001, 0x080000, CRC(bec48d7a) SHA1(9fdcc8f461e48cb4244827bead980ad48acdfbd8) )
	ROM_LOAD16_BYTE( "v1_1.u150", 0x000000, 0x080000, CRC(d5793a2f) SHA1(0623d51d405fde69622f1e15512fd8fc41209a59) )
	ROM_LOAD16_BYTE( "v1_3.u149", 0x100001, 0x080000, CRC(0cef25da) SHA1(7f7447577093009efacff27659a0d8a95c29659a) )
	ROM_LOAD16_BYTE( "v1_4.u151", 0x100000, 0x080000, CRC(59cd26bc) SHA1(36e9fc43fba2af1d16b7c246b34354e3b93fa0fe) )

	ROM_REGION( 0x200000, "gfx3", 0 )   // Layer 2
	ROM_LOAD16_BYTE( "v2_0.u164", 0x000001, 0x080000, CRC(7e46a10e) SHA1(a8576f7a140b065b88a0dab648f7b31c75fec006) )
	ROM_LOAD16_BYTE( "v2_1.u166", 0x000000, 0x080000, CRC(9274123b) SHA1(b58e107a5bd222e454fd435d515e57cab52e6593) )
	ROM_LOAD16_BYTE( "v2_3.u165", 0x100001, 0x080000, CRC(71dfe0f4) SHA1(fd76966fa447bc2882b0c7dc447fac92ec9c136b) )
	ROM_LOAD16_BYTE( "v2_4.u167", 0x100000, 0x080000, CRC(49529f86) SHA1(b1953af8fbda87314fe95e893a43c064e0ad7121) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples, the last 3 ROMs were missing but since the first 5 match the dump above they may be the same, too
	ROM_LOAD( "s-0.u156",  0x000000, 0x020000, CRC(4d253547) SHA1(87cda11dc86bc121cb8fb0e574006c3627158f51) )
	ROM_LOAD( "s-1.u157",  0x020000, 0x020000, CRC(79b56e22) SHA1(4b6c62e96dc1e8fb6dc0a76c505f9d805ef4684f) )
	ROM_LOAD( "s-2.u158",  0x040000, 0x020000, CRC(bc8de02a) SHA1(503c2c9f9ce029701e6a5b134d9407ab06e28913) ) // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "s-3.u159",  0x060000, 0x020000, CRC(939777fd) SHA1(3dd1b89a4f81f745c68037c568c885fe1403ed31) )
	ROM_LOAD( "s-4.u160",  0x080000, 0x020000, CRC(7b97716d) SHA1(6693e81dc008317c6a985558624f5d5cf00785e9) )
	ROM_LOAD( "snd5.u161", 0x0a0000, 0x020000, BAD_DUMP CRC(294e1cc9) SHA1(5faef5eb9f15c23686c2f66646c6f6724e7c611f) )
	ROM_LOAD( "snd6.u162", 0x0c0000, 0x020000, BAD_DUMP CRC(ecab073b) SHA1(f991fb9d9d4ffe24b67b233850ef0727dc6329b6) )
	ROM_LOAD( "snd7.u163", 0x0e0000, 0x020000, BAD_DUMP CRC(1b7ea768) SHA1(7dfa8cbcb839c76f3f9eefd6abbc2b424c3d970a) )
ROM_END

ROM_START( daiohc ) // Found on a 93111A PCB - same PCB as War of Areo & J. J. Squawkers
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "15.u3", 0x000000, 0x040000, CRC(14616abb) SHA1(1ff5331b0de60230baa4ced58bec6a954cb599d5) )
	ROM_CONTINUE   (          0x100000, 0x040000  )
	ROM_LOAD16_BYTE( "14.u4", 0x000001, 0x040000, CRC(a029f991) SHA1(5d341fe5b3ac3bdda1d8e7cc8e6a260f04d00aa1) )
	ROM_CONTINUE   (          0x100001, 0x040000  )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD(  "9.u9",  0x000000, 0x080000, CRC(4444cbd4) SHA1(e039cd7e7093d399bc39aa4d355a03250e087fb3) ) // connects to U63 & U64 through a riser card
	ROM_LOAD( "10.u10", 0x080000, 0x080000, CRC(1d88d20b) SHA1(3cf95041d0876a4ef378651783e53cee1994ed3d) )
	ROM_LOAD( "11.u11", 0x100000, 0x080000, CRC(3e41de61) SHA1(7d3ddf3780bbe99b13937d75cbdbfb58449301a6) )
	ROM_LOAD( "12.u12", 0x180000, 0x080000, CRC(f35e3341) SHA1(9260460e1823d157201de02557c7136ef898cfb3) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // Layer 1
	ROM_LOAD16_WORD_SWAP( "5.u5", 0x000000, 0x080000, CRC(aaa5e41e) SHA1(fe362ec083cb13732ea07003a4a1a9c63d382f4b) ) // connects to U66 through a riser card
	ROM_LOAD16_WORD_SWAP( "6.u6", 0x080000, 0x080000, CRC(9ad8b4b4) SHA1(b6e4cff160ae0efe6f3fd0df9a8a618957c3ce61) )
	ROM_LOAD16_WORD_SWAP( "7.u7", 0x100000, 0x080000, CRC(babf194a) SHA1(ef838aab2d651c10553fb87552c67f289a8ac83d) )
	ROM_LOAD16_WORD_SWAP( "8.u8", 0x180000, 0x080000, CRC(2db65290) SHA1(4f4d65e984fad7bb1d886de67bc50645798282bb) )

	ROM_REGION( 0x200000, "gfx3", 0 ) // Layer 2
	ROM_LOAD16_WORD_SWAP( "1.u1", 0x000000, 0x080000, CRC(30f81f99) SHA1(9c164c798c7e869e92505d9d85f06f4a1c9a9528) ) // connects to U68 through a riser card
	ROM_LOAD16_WORD_SWAP( "2.u2", 0x080000, 0x080000, CRC(3b3e0f4e) SHA1(740afe4eefea480f941dd80a03392592d8d4b084) )
	ROM_LOAD16_WORD_SWAP( "3.u3", 0x100000, 0x080000, CRC(c5eef1c1) SHA1(d4b3188b39bad5c7a2c7b7dbc91a79c7ee80a3a1) )
	ROM_LOAD16_WORD_SWAP( "4.u4", 0x180000, 0x080000, CRC(851115b6) SHA1(b8e1e22231d131085c90afcf30ff35a2866edff5) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "data.u69", 0x000000, 0x080000, CRC(21e4f093) SHA1(f0420d158dc5d182e41b6fb2ea3af6baf88bacb8) )
	ROM_LOAD( "data.u70", 0x080000, 0x080000, CRC(593c3c58) SHA1(475fb530a6d23269cb0aea6e294291c7463b57a2) )

	ROM_REGION( 0x200, "gals", 0 )
	ROM_LOAD( "gal.u14",  0x000000, 0x117, CRC(b972b479) SHA1(50da73b4cc7b9c0ff8fb19b2c34d05a4dbc8f0cb) )
ROM_END

ROM_START( msgundam )
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_WORD_SWAP( "fa003002.u25",  0x000000, 0x080000, CRC(1cc72d4c) SHA1(5043d693b5a8116a077d5b6997b658cb287e2aa7) )
	ROM_LOAD16_WORD_SWAP( "fa001001.u20",  0x100000, 0x100000, CRC(fca139d0) SHA1(b56282c69f7ec64c697a48e42d59a2565401c032) )

	ROM_REGION( 0x400000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "fa001008.u21",  0x000000, 0x200000, CRC(e7accf48) SHA1(dca9d53bc9cf0ecb661358d5a3f388c4ce9388e7) )
	ROM_LOAD( "fa001007.u22",  0x200000, 0x200000, CRC(793198a6) SHA1(45f53870e74b14126680d18dd58dbbe01a6ef509) )

	ROM_REGION( 0x100000, "gfx2", 0 )   // Layer 1
	ROM_LOAD16_WORD_SWAP( "fa001006.u23",  0x000000, 0x100000, CRC(3b60365c) SHA1(bdf5a0b1b45eb75dbbb6725d1e5303716321aeb9) )

	ROM_REGION( 0x080000, "gfx3", 0 )   // Layer 2
	ROM_LOAD16_WORD_SWAP( "fa001005.u24",  0x000000, 0x080000, CRC(8cd7ff86) SHA1(ce7eb90776e21239f8f52e822c636143506c6f9b) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "fa001004.u26",  0x000000, 0x100000, CRC(b965f07c) SHA1(ff7827cc80655465ffbb732d55ba81f21f51a5ca) )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "fa-011.u50", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "fa-012.u51", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "fa-013.u52", 0x400, 0x104, NO_DUMP )
	ROM_LOAD( "fa-014.u53", 0x600, 0x104, NO_DUMP )
	ROM_LOAD( "fa-015.u54", 0x800, 0x104, NO_DUMP )
ROM_END

ROM_START( msgundam1 )
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_WORD_SWAP( "fa002002.u25",  0x000000, 0x080000, CRC(dee3b083) SHA1(e2ad626aa0109906846dd9e9053ffc83b7bf4d2e) )
	ROM_LOAD16_WORD_SWAP( "fa001001.u20",  0x100000, 0x100000, CRC(fca139d0) SHA1(b56282c69f7ec64c697a48e42d59a2565401c032) )

	ROM_REGION( 0x400000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "fa001008.u21",  0x000000, 0x200000, CRC(e7accf48) SHA1(dca9d53bc9cf0ecb661358d5a3f388c4ce9388e7) )
	ROM_LOAD( "fa001007.u22",  0x200000, 0x200000, CRC(793198a6) SHA1(45f53870e74b14126680d18dd58dbbe01a6ef509) )

	ROM_REGION( 0x100000, "gfx2", 0 )   // Layer 1
	ROM_LOAD16_WORD_SWAP( "fa001006.u23",  0x000000, 0x100000, CRC(3b60365c) SHA1(bdf5a0b1b45eb75dbbb6725d1e5303716321aeb9) )

	ROM_REGION( 0x080000, "gfx3", 0 )   // Layer 2
	ROM_LOAD16_WORD_SWAP( "fa001005.u24",  0x000000, 0x080000, CRC(8cd7ff86) SHA1(ce7eb90776e21239f8f52e822c636143506c6f9b) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "fa001004.u26",  0x000000, 0x100000, CRC(b965f07c) SHA1(ff7827cc80655465ffbb732d55ba81f21f51a5ca) )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "fa-011.u50", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "fa-012.u51", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "fa-013.u52", 0x400, 0x104, NO_DUMP )
	ROM_LOAD( "fa-014.u53", 0x600, 0x104, NO_DUMP )
	ROM_LOAD( "fa-015.u54", 0x800, 0x104, NO_DUMP )
ROM_END

ROM_START( msgundamb ) // 2 PCB stack, one has a 'Tecnoval - tecnologia valenciana del recreativo' sticker
	ROM_REGION( 0x300000, "maincpu", 0 )        // 68000 Code, on lower board
	ROM_LOAD16_BYTE( "d-4.bin", 0x000000, 0x040000, CRC(ca5bfa89) SHA1(108435cb65919b4b90be102e7ac2799501149fc8) ) // 27c020
	ROM_LOAD16_BYTE( "d-2.bin", 0x000001, 0x040000, CRC(b4b86d1b) SHA1(d9b625cfdabcabed9308fccc29c66adfe566a996) ) // 27c020
	ROM_LOAD16_BYTE( "27c8001-7.bin",   0x100000, 0x100000, CRC(803f279c) SHA1(1095ac434ce553ed56d106556e7d23ccac1f0cd4) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD16_BYTE( "27c8001-8.bin",   0x100001, 0x100000, CRC(a310fa93) SHA1(2ee1616699c95ed2b8c46d43de4cffece1b033ea) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x400000, "gfx1", 0 )   // Sprites, on top board, almost identical to the original but split
	ROM_LOAD16_BYTE( "27c8001-3.bin", 0x000000, 0x100000, CRC(9f36d867) SHA1(01a15dcdcb1077d7c8678762f58ad705dc29e8c9) )
	ROM_LOAD16_BYTE( "27c8001-2.bin", 0x000001, 0x100000, CRC(70d333d9) SHA1(9ef3b0e0567ceed082921a5c384cfcfeb154f048) )
	ROM_LOAD16_BYTE( "27c8001-1.bin", 0x200000, 0x100000, CRC(2792692c) SHA1(ed99c589ed15f8c1a4e2ab435a379b35105ba503) )
	ROM_LOAD16_BYTE( "4.bin", 0x200001, 0x080000, CRC(eb551f1a) SHA1(e9d2fc31c3076164c5ee9722ea1b1e60b4f6d663) ) // mx27c4000
	ROM_LOAD16_BYTE( "5.bin", 0x300001, 0x080000, CRC(e9aa57e8) SHA1(699c0132f4be81570f748e5ca2f88fc4fc6802bb) ) // mx27c4000

	ROM_REGION( 0x200000, "gfx2", 0 )   // Layer 1, on lower board
	ROM_LOAD16_BYTE( "27c8001-5.bin", 0x000000, 0x100000, CRC(c83ae34a) SHA1(d7bf49843c443c5b7cb9187404a3518eaed577a6) ) // 1ST AND 2ND HALF IDENTICAL, fa001006.u23 [odd]  IDENTICAL
	ROM_LOAD16_BYTE( "27c8001-6.bin", 0x000001, 0x100000, CRC(8fbb5478) SHA1(247fd080f0ee18282c4d8b918171cfeab4b40d23) ) // 1ST AND 2ND HALF IDENTICAL, fa001006.u23 [even] IDENTICAL

	ROM_REGION( 0x080000, "gfx3", 0 )   // Layer 2, on lower board, identical to the original but split
	ROM_LOAD16_BYTE( "d-8.bin", 0x000001, 0x040000, CRC(a03c8345) SHA1(d3c3f0045ebb3d82d82432c212db4a801cb53b60) ) // 27c020
	ROM_LOAD16_BYTE( "d-9.bin", 0x000000, 0x040000, CRC(cfd47024) SHA1(ab4fbaf258d2694407dd6c896f45d69821ccc408) ) // 27c020

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples, on lower board
	ROM_LOAD( "27c8001-4.bin", 0x000000, 0x100000, CRC(b965f07c) SHA1(ff7827cc80655465ffbb732d55ba81f21f51a5ca) )  // identical to the original
ROM_END

ROM_START( oisipuzl )
	ROM_REGION( 0x180000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_WORD_SWAP( "ss1u200.v10", 0x000000, 0x080000, CRC(f5e53baf) SHA1(057e8b35bc6f65634685b5d0cf38e12f2e62d72c) )
	// Gap of 0x80000 bytes
	ROM_LOAD16_WORD_SWAP( "ss1u201.v10", 0x100000, 0x080000, CRC(7a7ff5ae) SHA1(9e4da7ecc4d833c3ba4ddc6e5870fad53b9b2d2b) )

	ROM_REGION( 0x200000, "gfx1", ROMREGION_INVERT )    // Sprites
	ROM_LOAD( "ss1u306.v10", 0x000000, 0x080000, CRC(ce43a754) SHA1(3991042678badafee716b084c1768a794f144b1e) )
	ROM_LOAD( "ss1u307.v10", 0x080000, 0x080000, CRC(2170b7ec) SHA1(c9f3d12646d4e877bc2b656f977e21d927f241f6) )
	ROM_LOAD( "ss1u304.v10", 0x100000, 0x080000, CRC(546ab541) SHA1(aa96a79e3b0ba71f5e0fbb15e190d219630c2ba3) )
	ROM_LOAD( "ss1u305.v10", 0x180000, 0x080000, CRC(2a33e08b) SHA1(780cfe44a4d57b254bd0cfae8727dc77358027a8) )

	ROM_REGION( 0x100000, "gfx2", 0 )   // Layer 1
	ROM_LOAD16_WORD_SWAP( "ss1u23.v10",  0x000000, 0x080000, CRC(9fa60901) SHA1(3d42e4174ad566b6eeb488c7a4c51db9c1fef7af) )
	ROM_LOAD16_WORD_SWAP( "ss1u24.v10",  0x080000, 0x080000, CRC(c10eb4b3) SHA1(70a82a750b1d9c849cd92d4f73769bbf5962c771) )

	ROM_REGION( 0x080000, "gfx3", 0 )   // Layer 2
	ROM_LOAD16_WORD_SWAP( "ss1u25.v10",  0x000000, 0x080000, CRC(56840728) SHA1(db61539fd84f0de35ee2077238ba3646c4960cc6) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "ss1u26.v10", 0x000000, 0x080000, CRC(d452336b) SHA1(d3bf3cb383c40911758a60546f121c48087868e3) )
	ROM_LOAD( "ss1u27.v10", 0x080000, 0x080000, CRC(17fe921d) SHA1(7fc176b8eefad4f2b8532bfe62e7852d2be185ca) )
ROM_END

ROM_START( triplfun )
	ROM_REGION( 0x180000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "05.bin", 0x000000, 0x40000, CRC(06eb3821) SHA1(51c67c87b5c28e693dfffd32d25cdb6d2a9448cf) )
	ROM_CONTINUE(0x100000,0x40000)
	ROM_LOAD16_BYTE( "04.bin", 0x000001, 0x40000, CRC(37a5c46e) SHA1(80b49b422a7db64d9ba5896da2b01a4588a6cf62) )
	ROM_CONTINUE(0x100001,0x40000)

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "08.bin", 0x000001, 0x80000, CRC(63a8f10f) SHA1(0a045d559b9edc9f335e6ec2d214d70c4959ec50) )
	ROM_LOAD16_BYTE( "09.bin", 0x000000, 0x80000, CRC(98cc8ca5) SHA1(1bd9d2d860e02ee4fea3d9592172690cb9d3acf2) )
	ROM_LOAD16_BYTE( "10.bin", 0x100001, 0x80000, CRC(20b0f282) SHA1(c98de63c1ad9dfe9b24f55966ccc5392c5ae82ba) )
	ROM_LOAD16_BYTE( "11.bin", 0x100000, 0x80000, CRC(276ef724) SHA1(e0c642dfd19542234abb0de68a66f8c36d9cb827) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "02.bin", 0x000001, 0x80000, CRC(4c0d1068) SHA1(cb77309474938765fd0582ab132f19fb5e21fca3) )
	ROM_LOAD16_BYTE( "03.bin", 0x000000, 0x80000, CRC(dba94e18) SHA1(3f54d874287e4ab96b2791503235488164d90cb1) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD16_BYTE( "06.bin", 0x000001, 0x40000, CRC(8944bb72) SHA1(37cd0e2c8e99fb23ea70dc183a8aa0670c5f6b65) )
	ROM_LOAD16_BYTE( "07.bin", 0x000000, 0x40000, CRC(934a5d91) SHA1(aa19d2699b5ebdd99d59004005b0ce0c5140d192) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "01.bin", 0x000000, 0x40000, CRC(c186a930) SHA1(e17e1a620e380f0737b80c7f160ad643979b2799) )
ROM_END

/* There is another Korean set (undumped) with only two noticeable differences: One space on the game title ("숨어있는 덩달이를 찾아 라!"
   instead of "숨어있는 덩달이를 찾아라!") and that there's no bootlegger company name on title screen. */
ROM_START( triplfunk )
	ROM_REGION( 0x180000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "05.bin", 0x000000, 0x40000, CRC(06eb3821) SHA1(51c67c87b5c28e693dfffd32d25cdb6d2a9448cf) )
	ROM_CONTINUE(0x100000,0x40000)
	ROM_LOAD16_BYTE( "04.bin", 0x000001, 0x40000, CRC(37a5c46e) SHA1(80b49b422a7db64d9ba5896da2b01a4588a6cf62) )
	ROM_CONTINUE(0x100001,0x40000)

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "08k.bin", 0x000001, 0x80000, CRC(e9a4b535) SHA1(a1c7ed9e432ab732a42d8ff74c3052917ba2711e) )
	ROM_LOAD16_BYTE( "09k.bin", 0x000000, 0x80000, CRC(06730143) SHA1(69747db906c3a7c896d902d79feb2317f85a9557) )
	ROM_LOAD16_BYTE( "10k.bin", 0x100001, 0x80000, CRC(2cea4898) SHA1(99281943ac45b68f816d518b8daf7feb6f0e2ce0) )
	ROM_LOAD16_BYTE( "11k.bin", 0x100000, 0x80000, CRC(8166e961) SHA1(1c39524197878a3806a26afdc4623c6fdb1108dc) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "02k.bin", 0x000001, 0x80000, CRC(3188b102) SHA1(b5d0bf0b93866c18c72533ac80da3080f8fdced0) )
	ROM_LOAD16_BYTE( "03k.bin", 0x000000, 0x80000, CRC(4a9520a4) SHA1(9e962efcfe21669f3cba2c1b6632975d706ff118) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD16_BYTE( "06k.bin", 0x000001, 0x40000, CRC(f65f72d5) SHA1(d783720a70b0ffabec149550fbd91608181b134f) )
	ROM_LOAD16_BYTE( "07k.bin", 0x000000, 0x40000, CRC(4522829e) SHA1(cd071bcef77f059cd06d7b8315d1f25dc652bcdc) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "01.bin", 0x000000, 0x40000, CRC(c186a930) SHA1(e17e1a620e380f0737b80c7f160ad643979b2799) )
ROM_END

ROM_START( qzkklgy2 )
	ROM_REGION( 0x0c0000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_WORD_SWAP( "fn001001.106", 0x000000, 0x080000, CRC(7bf8eb17) SHA1(f2d1666e22f564d59b37ca00c8db34ca822fd142) )
	ROM_LOAD16_WORD_SWAP( "fn001003.107", 0x080000, 0x040000, CRC(ee6ef111) SHA1(6d9efac46ba01fff8784034801cba10e38b2c923) )

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "fn001004.100", 0x000000, 0x100000, CRC(5ba139a2) SHA1(24fe19a7e5d2cd53bf3b1c71bf05020067f5e956) )

	ROM_REGION( 0x200000, "gfx2", 0 )   // Layer 1
	ROM_LOAD16_WORD_SWAP( "fn001005.104", 0x000000, 0x200000, CRC(95726a63) SHA1(e53ffc2815c4858bbfb5ff452c581bccb41854c9) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "fn001006.105", 0x000000, 0x100000, CRC(83f201e6) SHA1(536e74788ad0e07451300a1ad3b127bc9d2d9063) )
ROM_END

ROM_START( wrofaero )
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "u3.bin",  0x000000, 0x040000, CRC(9b896a97) SHA1(f4e768911705e6def5dc4a43cfc4146c48c80caf) )
	ROM_LOAD16_BYTE( "u4.bin",  0x000001, 0x040000, CRC(dda84846) SHA1(50142692e13190900bc752908b105b65c48ea911) )

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "u64.bin",  0x000000, 0x080000, CRC(f06ccd78) SHA1(1701bdac2c826327441cfe0039b4cadf8f3a4803) )
	ROM_LOAD( "u63.bin",  0x080000, 0x080000, CRC(2a602a1b) SHA1(b04fa743200d62bc25a6aa34efae53209f185f79) )

	ROM_REGION( 0x080000, "gfx2", 0 )   // Layer 1
	ROM_LOAD16_WORD_SWAP( "u66.bin",  0x000000, 0x080000, CRC(c9fc6a0c) SHA1(85ac0726221e3fedd80bd9b426d61471eb20ce46) )

	ROM_REGION( 0x080000, "gfx3", 0 )   // Layer 2
	ROM_LOAD16_WORD_SWAP( "u68.bin",  0x000000, 0x080000, CRC(25c0c483) SHA1(2e705e7f0c66c3bc73e78ffb526606ab8be61d99) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "u69.bin",  0x000000, 0x080000, CRC(957ecd41) SHA1(3b37ba44b8b8f0f0de41c8c26c3dfdb391ba572c) )
	ROM_LOAD( "u70.bin",  0x080000, 0x080000, CRC(8d756fdf) SHA1(d66712a6aa19252f2c915ac66fc27df031fa9512) )

	ROM_REGION( 0xc00, "plds", 0 )
	ROM_LOAD( "m-009.u206", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "m-010.u116", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "m-011.u14",  0x400, 0x104, NO_DUMP )
	ROM_LOAD( "m-012.u35",  0x600, 0x104, NO_DUMP )
	ROM_LOAD( "m-013.u36",  0x800, 0x104, NO_DUMP )
	ROM_LOAD( "m-014.u76",  0xa00, 0x104, NO_DUMP )
ROM_END

ROM_START( jjsquawk ) // PCB stickered  J.J. SQUAWKERS 9401- 1022
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "fe2002001.u3", 0x000000, 0x040000, CRC(7b9af960) SHA1(1718d54b0c12ae148de44f9ccccf90c0182f7b4f) )
	ROM_CONTINUE   (                 0x100000, 0x040000  )
	ROM_LOAD16_BYTE( "fe2002002.u4", 0x000001, 0x040000, CRC(47dd71a3) SHA1(e219d984a1cac484ce1e570b7849562a88e0903e) )
	ROM_CONTINUE   (                 0x100001, 0x040000  )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "fe2001009", 0x000000, 0x080000, CRC(27441cd3) SHA1(5867fc30c158e07f2d36ecab97b1d304383e6f35) ) // These ROMs located on a plug-in PCB
	ROM_LOAD( "fe2001010", 0x080000, 0x080000, CRC(ca2b42c4) SHA1(9b99b6618fe44a6c29a255e89dab72a0a56214df) )
	ROM_LOAD( "fe2001007", 0x100000, 0x080000, CRC(62c45658) SHA1(82b1ea138e8f4b4ade7e44b31843aa2023c9dd71) )
	ROM_LOAD( "fe2001008", 0x180000, 0x080000, CRC(2690c57b) SHA1(b880ded7715dffe12c4fea7ad7cb9c5133b73250) )

	ROM_REGION( 0x180000, "gfx2", 0 )   // Layer 1
	ROM_LOAD24_WORD_SWAP( "fe2001011", 0x000001, 0x080000, CRC(98b9f4b4) SHA1(de96708aebb428ddc413c3649caaec80c0c155bd) ) // This ROM located on a plug-in PCB
	ROM_LOAD24_WORD_SWAP( "fe2001012", 0x0c0001, 0x080000, CRC(d4aa916c) SHA1(d619d20c33f16ab06b529fc1717ad9b703acbabf) ) // This ROM located on a plug-in PCB
	ROM_LOAD24_BYTE     ( "fe2001003", 0x000000, 0x080000, CRC(a5a35caf) SHA1(da4bdb7f0b319f8ff972a552d0134a73e5ac1b87) )

	ROM_REGION( 0x180000, "gfx3", 0 )   // Layer 2
	ROM_LOAD24_WORD_SWAP( "fe2001014", 0x000001, 0x080000, CRC(274bbb48) SHA1(b8db632a9bbb7232d0b1debd67b3b453fd4989e6) ) // This ROM located on a plug-in PCB
	ROM_LOAD24_WORD_SWAP( "fe2001013", 0x0c0001, 0x080000, CRC(51e29871) SHA1(9d33283bd9a3f57602a55cfc9fafa49edd0be8c5) ) // This ROM located on a plug-in PCB
	ROM_LOAD24_BYTE     ( "fe2001004", 0x000000, 0x080000, CRC(a235488e) SHA1(a45d02a4451defbef7fbdab15671955fab8ed76b) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "fe2001005.u69", 0x000000, 0x080000, CRC(d99f2879) SHA1(66e83a6bc9093d19c72bd8ef1ec0523cfe218250) )
	ROM_LOAD( "fe2001006.u70", 0x080000, 0x080000, CRC(9df1e478) SHA1(f41b55821187b417ad09e4a1f439c01a107d2674) )

	ROM_REGION( 0xc00, "plds", 0 )
	ROM_LOAD( "m-009.u206", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "m-010.u116", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "m2-011.u14", 0x400, 0x104, NO_DUMP )
	ROM_LOAD( "m-012.u35",  0x600, 0x104, NO_DUMP )
	ROM_LOAD( "m-013.u36",  0x800, 0x104, NO_DUMP )
	ROM_LOAD( "m-014.u76",  0xa00, 0x104, NO_DUMP )
ROM_END

ROM_START( jjsquawko ) // Official 93111A PCB missing version sticker
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "fe2001001.u3", 0x000000, 0x040000, CRC(921c9762) SHA1(bbc1fb95256f7eb2aa7ad23f38dbcdf502e7da8d) )
	ROM_CONTINUE   (                 0x100000, 0x040000  )
	ROM_LOAD16_BYTE( "fe2001002.u4", 0x000001, 0x040000, CRC(0227a2be) SHA1(8ee0c39f84110865778564f803b4db11bfdfbad7) )
	ROM_CONTINUE   (                 0x100001, 0x040000  )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "fe2001009", 0x000000, 0x080000, CRC(27441cd3) SHA1(5867fc30c158e07f2d36ecab97b1d304383e6f35) ) // These ROMs located on a plug-in PCB
	ROM_LOAD( "fe2001010", 0x080000, 0x080000, CRC(ca2b42c4) SHA1(9b99b6618fe44a6c29a255e89dab72a0a56214df) )
	ROM_LOAD( "fe2001007", 0x100000, 0x080000, CRC(62c45658) SHA1(82b1ea138e8f4b4ade7e44b31843aa2023c9dd71) )
	ROM_LOAD( "fe2001008", 0x180000, 0x080000, CRC(2690c57b) SHA1(b880ded7715dffe12c4fea7ad7cb9c5133b73250) )

	ROM_REGION( 0x180000, "gfx2", 0 )   // Layer 1
	ROM_LOAD24_WORD_SWAP( "fe2001011", 0x000001, 0x080000, CRC(98b9f4b4) SHA1(de96708aebb428ddc413c3649caaec80c0c155bd) ) // This ROM located on a plug-in PCB
	ROM_LOAD24_WORD_SWAP( "fe2001012", 0x0c0001, 0x080000, CRC(d4aa916c) SHA1(d619d20c33f16ab06b529fc1717ad9b703acbabf) ) // This ROM located on a plug-in PCB
	ROM_LOAD24_BYTE     ( "fe2001003", 0x000000, 0x080000, CRC(a5a35caf) SHA1(da4bdb7f0b319f8ff972a552d0134a73e5ac1b87) )

	ROM_REGION( 0x180000, "gfx3", 0 )   // Layer 2
	ROM_LOAD24_WORD_SWAP( "fe2001014", 0x000001, 0x080000, CRC(274bbb48) SHA1(b8db632a9bbb7232d0b1debd67b3b453fd4989e6) ) // This ROM located on a plug-in PCB
	ROM_LOAD24_WORD_SWAP( "fe2001013", 0x0c0001, 0x080000, CRC(51e29871) SHA1(9d33283bd9a3f57602a55cfc9fafa49edd0be8c5) ) // This ROM located on a plug-in PCB
	ROM_LOAD24_BYTE     ( "fe2001004", 0x000000, 0x080000, CRC(a235488e) SHA1(a45d02a4451defbef7fbdab15671955fab8ed76b) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "fe2001005.u69", 0x000000, 0x080000, CRC(d99f2879) SHA1(66e83a6bc9093d19c72bd8ef1ec0523cfe218250) )
	ROM_LOAD( "fe2001006.u70", 0x080000, 0x080000, CRC(9df1e478) SHA1(f41b55821187b417ad09e4a1f439c01a107d2674) )

	ROM_REGION( 0xc00, "plds", 0 )
	ROM_LOAD( "m-009.u206", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "m-010.u116", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "m2-011.u14", 0x400, 0x104, NO_DUMP )
	ROM_LOAD( "m-012.u35",  0x600, 0x104, NO_DUMP )
	ROM_LOAD( "m-013.u36",  0x800, 0x104, NO_DUMP )
	ROM_LOAD( "m-014.u76",  0xa00, 0x104, NO_DUMP )
ROM_END

ROM_START( jjsquawkb )
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_WORD_SWAP( "3", 0x000000, 0x080000, CRC(afd5bd07) SHA1(eee231f596ce5cb9bbf41c7c9e18c11a399d7dfd) )
	ROM_LOAD16_WORD_SWAP( "2", 0x100000, 0x080000, CRC(740a7366) SHA1(2539f9a9b4fed1a1e2c354d144b8d455ed4bc144) )

	ROM_REGION( 0x400000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "4.bin",  0x000000, 0x200000, CRC(969502f7) SHA1(d6cecb38e8b73c61537cc1bdc843fc7cd695c771) ) // sldh
	ROM_LOAD( "2.bin",  0x200000, 0x200000, CRC(765253d1) SHA1(4cbc6f093c87280ef9c17fecfc319cb780d755cc) ) // sldh

	ROM_REGION( 0x400000, "gfxtemp", 0  )
	ROM_LOAD( "3.bin",  0x000000, 0x200000, CRC(b1e3a4bb) SHA1(be2241a4fbb99444487e7b550faac4ee1ee1ad15) ) // sldh
	ROM_LOAD( "1.bin",  0x200000, 0x200000, CRC(a5d37cf7) SHA1(9573777f3cdd6b25f0bd56f65f583fddda21c900) ) // sldh

	ROM_REGION( 0x200000, "gfx2", 0 )   // Layer 1
	ROM_COPY( "gfxtemp", 0x000000, 0x000000, 0x100000 )
	ROM_COPY( "gfxtemp", 0x200000, 0x100000, 0x100000 )

	ROM_REGION( 0x200000, "gfx3", 0 )   // Layer 2
	ROM_COPY( "gfxtemp", 0x100000, 0x000000, 0x100000 )
	ROM_COPY( "gfxtemp", 0x300000, 0x100000, 0x100000 )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "1", 0x000000, 0x100000, CRC(181a55b8) SHA1(6fa404f85bad93cc15e80feb61d19bed84602b82) ) // fe2001005.u69 + fe2001006.u70 from jjsquawk
ROM_END

ROM_START( jjsquawkb2 ) // PCB was P0-078A, which was a Blandia board converted to JJ Squawkers. No labels on any of the ROMs.  Apparently based on jjsquawko set.
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "u3.3a", 0x000000, 0x040000, CRC(f94c913b) SHA1(de6e422c514c787897f8f41d7cd98acb0135c763) ) // 99.999619%
	ROM_CONTINUE   (                0x100000, 0x040000  )
	ROM_LOAD16_BYTE( "u4.4a", 0x000001, 0x040000, CRC(0227a2be) SHA1(8ee0c39f84110865778564f803b4db11bfdfbad7) ) // 99.999809%
	ROM_CONTINUE   (                0x100001, 0x040000  )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "u64.3l",  0x000000, 0x100000, CRC(11d8713a) SHA1(8e3359f605913625191ac0a09222ec465b5fea71) ) // fe2001009 + fe2001010 from jjsquawk
	ROM_LOAD( "u63.2l",  0x100000, 0x100000, CRC(7a385ef0) SHA1(d38e2242532074b58707783608a6ddce42c55a77) ) // fe2001007 + fe2001008 from jjsquawk

	ROM_REGION( 0x200000, "gfx2", 0 )   // Layer 1
	ROM_LOAD24_WORD_SWAP( "u66.5l", 0x000001, 0x100000, CRC(bbaf40c5) SHA1(aecd48176adbe79e76c8febca1d9bb95ff0d6912) ) // fe2001011 + fe2001012 from jjsquawk
	ROM_LOAD24_BYTE     ( "u65.4l", 0x000000, 0x080000, CRC(a5a35caf) SHA1(da4bdb7f0b319f8ff972a552d0134a73e5ac1b87) ) // fe2001003             from jjsquawk

	ROM_REGION( 0x200000, "gfx3", 0 )   // Layer 2
	ROM_LOAD24_WORD_SWAP( "u68.7l", 0x000001, 0x100000, CRC(ae9ae01f) SHA1(1a828d5b8848c5b5d8e5f279f1fde26b972a6332) ) // fe2001014 + fe2001013 from jjsquawk
	ROM_LOAD24_BYTE     ( "u67.6l", 0x000000, 0x080000, CRC(a235488e) SHA1(a45d02a4451defbef7fbdab15671955fab8ed76b) ) // fe2001004             from jjsquawk

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "u70.10l", 0x000000, 0x100000, CRC(181a55b8) SHA1(6fa404f85bad93cc15e80feb61d19bed84602b82) ) // fe2001005.u69 + fe2001006.u70 from jjsquawk
ROM_END

ROM_START( simpsonjr ) // bootleg of J. J. Squawkers by Daigom
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_WORD_SWAP( "4.bin", 0x000000, 0x080000, CRC(469cc203) SHA1(4ecd8dce936f24acb149ef2fdf34595bd4a20a74) ) // sldh
	ROM_LOAD16_WORD_SWAP( "3.bin", 0x100000, 0x080000, CRC(740a7366) SHA1(2539f9a9b4fed1a1e2c354d144b8d455ed4bc144) ) // sldh

	ROM_REGION( 0x800000, "gfxtemp", 0  )
	ROM_LOAD( "5.bin",  0x000000, 0x400000, CRC(82952780) SHA1(83b61c726dd102491fe338036531f7653b0edefc) )
	ROM_LOAD( "6.bin",  0x400000, 0x400000, CRC(5a22bb87) SHA1(e5f91af685eb9331c5f00d81eca6dca177a9c860) )

	ROM_REGION( 0x400000, "gfx1", 0 )   // Sprites
	ROM_COPY( "gfxtemp", 0x600000, 0x000000, 0x200000 )
	ROM_COPY( "gfxtemp", 0x200000, 0x200000, 0x200000 )

	ROM_REGION( 0x200000, "gfx2", 0 )   // Layer 1
	ROM_COPY( "gfxtemp", 0x400000, 0x000000, 0x100000 )
	ROM_COPY( "gfxtemp", 0x000000, 0x100000, 0x100000 )

	ROM_REGION( 0x200000, "gfx3", 0 )   // Layer 2
	ROM_COPY( "gfxtemp", 0x500000, 0x000000, 0x100000 )
	ROM_COPY( "gfxtemp", 0x100000, 0x100000, 0x100000 )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "1.bin", 0x000000, 0x080000, CRC(d99f2879) SHA1(66e83a6bc9093d19c72bd8ef1ec0523cfe218250) ) // sldh
	ROM_LOAD( "2.bin", 0x080000, 0x080000, CRC(9df1e478) SHA1(f41b55821187b417ad09e4a1f439c01a107d2674) ) // sldh
ROM_END

ROM_START( kamenrid )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_WORD_SWAP( "fj001003.25", 0x000000, 0x080000, CRC(9b65d1b9) SHA1(a9183f817dbd1721cbb1a9049ca2bfc6acdf9f4a) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "fj001005.21", 0x000000, 0x100000, CRC(5d031333) SHA1(7b603e7e79c9439b526687021c0be4a5965b4c11) )
	ROM_LOAD( "fj001006.22", 0x100000, 0x100000, CRC(cf28eb78) SHA1(b1b34e0e50b5d54ff3cff908c579031a326890a2) )

	ROM_REGION( 0x80000, "user1", 0 )   // Layers 1+2
	ROM_LOAD16_WORD_SWAP( "fj001007.152", 0x000000, 0x080000, CRC(d9ffe80b) SHA1(c1f919b53cd1b9874a5e5dc5640891e1b227cfc6) )

	ROM_REGION( 0x40000, "gfx2", 0 )    // Layer 1
	ROM_COPY( "user1", 0x000000, 0x000000, 0x040000 )

	ROM_REGION( 0x40000, "gfx3", 0 )    // Layer 2
	ROM_COPY( "user1", 0x040000, 0x000000, 0x040000 )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "fj001008.26", 0x000000, 0x100000, CRC(45e2b329) SHA1(8526afae1aa9178570c906eb96438f174d174f4d) )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "fj-111.u50", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "fj-012.u51", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "fj-013.u52", 0x400, 0x104, NO_DUMP )
	ROM_LOAD( "fj-014.u53", 0x600, 0x104, NO_DUMP )
	ROM_LOAD( "fj-015.u54", 0x800, 0x104, NO_DUMP )
ROM_END

ROM_START( eightfrc )
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "uy2-u4.u3",  0x000000, 0x040000, CRC(f1f249c5) SHA1(5277b7a15934e60e0ca305c318fb02d0ffb99d42) )
	ROM_LOAD16_BYTE( "uy2-u3.u4",  0x000001, 0x040000, CRC(6f2d8618) SHA1(ea243e6064b76bc5d6e831362ac9611a48ac94a7) )

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "u64.bin",  0x000000, 0x080000, CRC(f561ff2e) SHA1(1ed78c90bf876f24c2859a73a71764189cebddbe) )
	ROM_LOAD( "u63.bin",  0x080000, 0x080000, CRC(4c3f8366) SHA1(b25a27a67ae828d8fcf2c8d9d373ebdaacce9c4e) )

	ROM_REGION( 0x100000, "gfx2", 0 )   // Layer 1
	ROM_LOAD16_WORD_SWAP( "u66.bin",  0x000000, 0x100000, CRC(6fad2b7f) SHA1(469d185dc942bd4b54babf1d528e0e420f31d88b) )

	ROM_REGION( 0x100000, "gfx3", 0 )   // Layer 2
	ROM_LOAD16_WORD_SWAP( "u68.bin",  0x000000, 0x100000, CRC(c17aad22) SHA1(eabbae2142cad3eef6a94d542ea03221c8228e94) )

	ROM_REGION( 0x200000, "x1snd", 0 )  // Samples
	ROM_LOAD( "u70.bin",  0x000000, 0x100000, CRC(dfdb67a3) SHA1(0fed6fb498dcfc1276facd0ecd2dfde45ff671f2) )
	ROM_LOAD( "u69.bin",  0x100000, 0x100000, CRC(82ec08f1) SHA1(f17300d3cf990ef5c11056fd922f8cae0b2c918f) )

	ROM_REGION( 0xc00, "plds", 0 )
	ROM_LOAD( "uy-012.u206", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "uy-013.u14",  0x200, 0x104, NO_DUMP )
	ROM_LOAD( "uy-014.u35",  0x400, 0x104, NO_DUMP )
	ROM_LOAD( "uy-015.u36",  0x600, 0x104, NO_DUMP )
	ROM_LOAD( "uy-016.u76",  0x800, 0x104, NO_DUMP )
	ROM_LOAD( "uy-017.u116", 0xa00, 0x104, NO_DUMP )
ROM_END

ROM_START( krzybowl )
	ROM_REGION( 0x080000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "fv001.002", 0x000000, 0x040000, CRC(8c03c75f) SHA1(e56c50440681a0b06d785000018c4213266f2a4e) )
	ROM_LOAD16_BYTE( "fv001.001", 0x000001, 0x040000, CRC(f0630beb) SHA1(1ddd4ab1bc5ab2b6461eb35c8093884185828d7b) )

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "fv001.003", 0x000000, 0x080000, CRC(7de22749) SHA1(933a11f2d45667348b136d72806fc2e2f6f8d944) )
	ROM_LOAD( "fv001.004", 0x080000, 0x080000, CRC(c7d2fe32) SHA1(37291fa78c28be274e1240e081ea253ebe487e5c) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "fv001.005", 0x000000, 0x080000, CRC(5e206062) SHA1(e47cfb6947df178f3547dfe61907571bcb84e4ac) )
	ROM_LOAD( "fv001.006", 0x080000, 0x080000, CRC(572a15e7) SHA1(b6a3e99e14a473b78ff48d1a46b20a0862d128e9) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "fv-007.u22", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "fv-008.u23", 0x200, 0x104, NO_DUMP )
ROM_END

ROM_START( orbs )   // All EPROMs are socketed and labelled (handwritten) "ORBS 10\7\94"
			// most of ROM space is unused (filled with sound samples - same data in all ROMs)

	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "orbs.u10",  0x000000, 0x080000, CRC(10f079c8) SHA1(0baf2b7e1e8be116a6fab609481c87fc7c86f305) )
	ROM_LOAD16_BYTE( "orbs.u9",   0x000001, 0x080000, CRC(f269d16f) SHA1(34f38789cb3256e334b0ac8acd9f339d14481578) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "orbs.u11",  0x000000, 0x080000, CRC(58cb38ba) SHA1(1c6c5f7ccb9c81b71bc1cbad080799b97962f262) )
	ROM_LOAD16_BYTE( "orbs.u12",  0x000001, 0x080000, CRC(b8c352c2) SHA1(7d6fd1425d9d5cf6a14a1ddceba0ad10e472dfa5) )
	ROM_LOAD16_BYTE( "orbs.u13",  0x100000, 0x080000, CRC(784bdc1a) SHA1(de2c5b38561b8ba6bd800126d010b734c2751575) )
	ROM_LOAD16_BYTE( "orbs.u14",  0x100001, 0x080000, CRC(1cc76541) SHA1(d8a233212bfb9a9c686a40e470524f95b34417fa) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "orbs.u15",  0x000000, 0x080000, CRC(bc0e9fe3) SHA1(758a44d07d59af8bbc87602df25dfcdc6cb8d9b3) )
	ROM_LOAD( "orbs.u16",  0x080000, 0x080000, CRC(aecd8373) SHA1(5620bcb281a9ea4920cfe81d163827013289c5bf) )
ROM_END

ROM_START( keroppi )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "keroppi jr. code =u10= v1.0.u10",  0x000000, 0x040000, CRC(1fc2e895) SHA1(08f1f48d4f601cf51583bf8854a9fa7016337cfc) )
	ROM_LOAD16_BYTE( "keroppi jr. code =u9= v1.0.u9",    0x000001, 0x040000, CRC(e0599e7b) SHA1(46b5ecc2864ab9e75540764453df5a2e6b6195e0) )

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "keroppi jr. chr=u11= v1.0.u11",  0x000000, 0x080000, CRC(74148c23) SHA1(26e642e2e0ad2b2af749355e9c46605061b100bc) )
	ROM_LOAD( "keroppi jr. chr=u12= v1.0.u12",  0x080000, 0x080000, CRC(6f4dae98) SHA1(da88837278cea956485f11ef55da8a4e9504c97a) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "keroppi jr. snd =u15= v1.0.u15",  0x000000, 0x080000, CRC(c98dacf0) SHA1(b508433e2383af1e8bd5fda253c9925c48443490) ) // == = ft-001-007.u15
	ROM_LOAD( "keroppi jr. snd =u16= v1.0.u16",  0x080000, 0x080000, CRC(d61e5a32) SHA1(aa2edf39e72ac15a8c8dd016b87bea17472f0f94) )
ROM_END

ROM_START( keroppij )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "ft-001-001.u10",  0x000000, 0x080000, CRC(37861e7d) SHA1(4bf75f119b0ef1420c96844224850867fa8e273f) )
	ROM_LOAD16_BYTE( "ft-001-002.u9",   0x000001, 0x080000, CRC(f531d4ef) SHA1(75a99695679de083765700c250bd1fdfd8be9981) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "ft-001-006.u11",  0x000000, 0x080000, CRC(9c500eae) SHA1(3448adef04c9ad2e0b39a283e4eb9c9bac7d4967) )
	ROM_LOAD16_BYTE( "ft-001-005.u12",  0x000001, 0x080000, CRC(de6432a8) SHA1(afee9b29e0b3db4815fc29456044532aee03597e) )
	ROM_LOAD16_BYTE( "ft-001-004.u13",  0x100000, 0x080000, CRC(69908c98) SHA1(1af069e9330a33cd2f0e1365e05c72eb23c3244e) )
	ROM_LOAD16_BYTE( "ft-001-003.u14",  0x100001, 0x080000, CRC(62fb22fb) SHA1(a67cb46152b73a47c2287c4058d0a22fb7064e7e) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "ft-001-007.u15",  0x000000, 0x080000, CRC(c98dacf0) SHA1(b508433e2383af1e8bd5fda253c9925c48443490) )
	ROM_LOAD( "ft-001-008.u16",  0x080000, 0x080000, CRC(b9c4b637) SHA1(82977d10de1048f71525bab5431b031cca510114) )
ROM_END

ROM_START( extdwnhl )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "fw001002.201",  0x000000, 0x080000, CRC(24d21924) SHA1(9914a68a578f884b06305ffcd9aeed7d83df1c7b) )
	ROM_LOAD16_BYTE( "fw001001.200",  0x000001, 0x080000, CRC(fb12a28b) SHA1(89167c042dc535b5f639057ff04a8e28824790f2) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "fw001003.202", 0x000000, 0x200000, CRC(ac9b31d5) SHA1(d362217ea0c474994e3c79ddcf87ee6688428ea5) )

	ROM_REGION( 0x400000, "gfx2", 0 )   // Layer 1
	ROM_LOAD24_WORD_SWAP( "fw001004.206", 0x000001, 0x200000, CRC(0dcb1d72) SHA1(ffc84f46f06f46750bddd1a303ed83a28fa9572f) )
	ROM_LOAD24_BYTE     ( "fw001005.205", 0x000000, 0x100000, CRC(5c33b2f1) SHA1(9ea848aeaccbba0b71e60b39cf844665bd97928f) )

	ROM_REGION( 0x200000, "gfx3", 0 )   // Layer 2
	ROM_LOAD16_WORD_SWAP( "fw001006.152",  0x000000, 0x200000, CRC(d00e8ddd) SHA1(e13692034afec1a0e86d19abfb9efa518b374147) )   // FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "fw001007.026",  0x080000, 0x080000, CRC(16d84d7a) SHA1(fdc13776ba1ec9c48a33a9f2dfe8a0e55c54d89e) )   // swapped halves
	ROM_CONTINUE(              0x000000, 0x080000  )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "fw-001.u50", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "fw-002.u51", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "fw-003.u52", 0x400, 0x104, NO_DUMP )
	ROM_LOAD( "fw-004.u53", 0x600, 0x104, NO_DUMP )
	ROM_LOAD( "fw-005.u54", 0x800, 0x104, NO_DUMP )
ROM_END

ROM_START( gundhara )
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "bpgh-003.u3",  0x000000, 0x080000, CRC(14e9970a) SHA1(31964bd290cc94c40684adf3a5d129b1c3addc3b) )
	ROM_LOAD16_BYTE( "bpgh-004.u4",  0x000001, 0x080000, CRC(96dfc658) SHA1(f570bc49758535eb00d93ecce9f75832f97a0d8d) )
	ROM_LOAD16_BYTE( "bpgh-002.103", 0x100000, 0x080000, CRC(312f58e2) SHA1(a74819d2f84a00c233489893f12c9ab1a98459cf) )
	ROM_LOAD16_BYTE( "bpgh-001.102", 0x100001, 0x080000, CRC(8d23a23c) SHA1(9e9a6488db424c81a97edcb7115cc070fe35c077) )

	ROM_REGION( 0x800000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "bpgh-008.u64", 0x000000, 0x200000, CRC(7ed9d272) SHA1(2e9243b3ecee27c175234f9bb1893ab498090fce) )
	ROM_LOAD( "bpgh-006.201", 0x200000, 0x200000, CRC(5a81411d) SHA1(ebf90afe027a0dc0fa3022978677fb071b9083d1) )
	ROM_LOAD( "bpgh-007.u63", 0x400000, 0x200000, CRC(aa49ce7b) SHA1(fe0064d533bd895657b88a0ef96e835443a4077f) )
	ROM_LOAD( "bpgh-005.200", 0x600000, 0x200000, CRC(74138266) SHA1(c859acff358a61a32e5810ff369b9d5528137337) )

	ROM_REGION( 0x180000, "gfx2", 0 )   // Layer 1
	ROM_LOAD24_BYTE     ( "bpgh-009.u65", 0x000000, 0x080000, CRC(b768e666) SHA1(473fa52c16c0a9f321e6429947a3e0fc1ef22f7e) )
	ROM_LOAD24_WORD_SWAP( "bpgh-010.u66", 0x000001, 0x100000, CRC(b742f0b8) SHA1(9246846c9ee839d5d84f5e02cf4605afcfd6bf7a) )

	ROM_REGION( 0x300000, "gfx3", 0 )   // Layer 2
	ROM_LOAD24_BYTE     ( "bpgh-011.u67", 0x000000, 0x100000, CRC(49aff270) SHA1(de25209e520cd8747042078440ee20866097d0cb) )
	ROM_LOAD24_WORD_SWAP( "bpgh-012.u68", 0x000001, 0x200000, CRC(edfda595) SHA1(5942181430d59c0c303cd1cbe753910c26c109a2) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "bpgh-013.u70",  0x080000, 0x080000, CRC(0fa5d503) SHA1(fd7a80cd25c23e737cc2c3d11de2291e22313b58) )   // swapped halves
	ROM_CONTINUE(              0x000000, 0x080000  )

	ROM_REGION( 0xc00, "plds", 0 )
	ROM_LOAD( "fx-001.u206", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "fx-002.u116", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "fx-003.u14",  0x400, 0x104, NO_DUMP )
	ROM_LOAD( "fx-004.u35",  0x600, 0x104, NO_DUMP )
	ROM_LOAD( "fx-005.u36",  0x800, 0x104, NO_DUMP )
	ROM_LOAD( "fx-006.u76",  0xa00, 0x104, NO_DUMP )
ROM_END

/* Chinese factory board, possibly bootleg but appears to come from the
   same factory as normal boards same as daiohc.  Modified layout allowing
   split ROMs */
ROM_START( gundharac )
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "4.u3",   0x000000, 0x080000, CRC(14e9970a) SHA1(31964bd290cc94c40684adf3a5d129b1c3addc3b) )
	ROM_LOAD16_BYTE( "2.u4",   0x000001, 0x080000, CRC(96dfc658) SHA1(f570bc49758535eb00d93ecce9f75832f97a0d8d) )
	ROM_LOAD16_BYTE( "3.u103", 0x100000, 0x080000, CRC(312f58e2) SHA1(a74819d2f84a00c233489893f12c9ab1a98459cf) )
	ROM_LOAD16_BYTE( "1.u102", 0x100001, 0x080000, CRC(8d23a23c) SHA1(9e9a6488db424c81a97edcb7115cc070fe35c077) )

	ROM_REGION( 0x800000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "19.u140",   0x000000, 0x080000, CRC(32d92c28) SHA1(7ba67f715f094aacf2dc2399809e4dfc7e4ca241) )
	ROM_LOAD16_BYTE( "23.u142",   0x000001, 0x080000, CRC(ff44db9b) SHA1(76ecd3ce3b6b33f3ae0b0454d58cf37d545dd72c) )
	ROM_LOAD16_BYTE( "21.u141",   0x100000, 0x080000, CRC(1901dc08) SHA1(b19428a7510d6e28a39bdf6ecc9732e3c2d19214) )
	ROM_LOAD16_BYTE( "25.u143",   0x100001, 0x080000, CRC(877289a2) SHA1(7482320e319d7b641fabba5aeeaa1237b693a219) )
	ROM_LOAD16_BYTE( "18.u140-b", 0x200000, 0x080000, CRC(4f023fb0) SHA1(815765c9783e44762bf57a3fbfad4385c316343a) )
	ROM_LOAD16_BYTE( "22.u142-b", 0x200001, 0x080000, CRC(6f3fe7e7) SHA1(71bc347c06678f4ae7850799da6346c6447bf3c0) )
	ROM_LOAD16_BYTE( "20.u141-b", 0x300000, 0x080000, CRC(7f1932e0) SHA1(13262a7322ad29cf7c85461204a3518e900c6145) )
	ROM_LOAD16_BYTE( "24.u143-b", 0x300001, 0x080000, CRC(066a2e2b) SHA1(186729918a89535484ab86dd58caf20ccce81501) )
	ROM_LOAD16_BYTE( "9.u144",    0x400000, 0x080000, CRC(6b4a531f) SHA1(701d6b2d87a742c8a2ab36331bd843dcd3309eae) )
	ROM_LOAD16_BYTE( "13.u146",   0x400001, 0x080000, CRC(45be3df4) SHA1(36667bf5e4b80d17a9d7b6ce4df7498f94681c46) )
	ROM_LOAD16_BYTE( "11.u145",   0x500000, 0x080000, CRC(f5210aa5) SHA1(4834d905f699dbec1cdacea6b320271c291aa2a7) )
	ROM_LOAD16_BYTE( "15.u147",   0x500001, 0x080000, CRC(17003119) SHA1(a2edd65c98bc654b541dad3e3783d90931c97597) )
	ROM_LOAD16_BYTE( "8.u144-b",  0x600000, 0x080000, CRC(ad9d9338) SHA1(33d6c881a20e2150017cc26f929473291e561718) )
	ROM_LOAD16_BYTE( "12.u146-b", 0x600001, 0x080000, CRC(0fd4c062) SHA1(7f418d43d9ba884c504f6fe3c04b11724412ac6b) )
	ROM_LOAD16_BYTE( "10.u145-b", 0x700000, 0x080000, CRC(7c5d12b9) SHA1(6ee45c4da6994540852153752e2818a8ea8ecf1a) )
	ROM_LOAD16_BYTE( "14.u147-b", 0x700001, 0x080000, CRC(5a8af50f) SHA1(3b7937ba720fcbbc5e29c1b95a97c29e8ff5490a) )

	ROM_REGION( 0x180000, "gfx2", 0 )   // Layer 1
	ROM_LOAD24_BYTE( "5.u148", 0x000002, 0x080000, CRC(0c740f9b) SHA1(f6d135c3318ff0d50d40921aa108b1b332c1a086) )
	ROM_LOAD24_BYTE( "6.u150", 0x000001, 0x080000, CRC(ba60eb98) SHA1(7204269816332bbb3401d9f20a513372ffe78500) )
	ROM_LOAD24_BYTE( "7.u154", 0x000000, 0x080000, CRC(b768e666) SHA1(473fa52c16c0a9f321e6429947a3e0fc1ef22f7e) )

	ROM_REGION( 0x300000, "gfx3", 0 )   // Layer 2
	ROM_LOAD24_BYTE( "26.u164", 0x000002, 0x080000, CRC(be3ccaba) SHA1(98f8b83cbed00932866375d21f86ee5c9bddb2a6) )
	ROM_LOAD24_BYTE( "28.u166", 0x000001, 0x080000, CRC(8a650a4e) SHA1(1f6eda27b39ad052e3d9a8a72cb0a072e7be4487) )
	ROM_LOAD24_BYTE( "16.u152", 0x000000, 0x080000, CRC(5ccc500b) SHA1(d3a2a5658cac8d788e0a1189c184309b8394b10a) )
	ROM_LOAD24_BYTE( "27.u165", 0x180002, 0x080000, CRC(47994ff0) SHA1(25211a9af01f77788578bb524619d95b5b86e241) )
	ROM_LOAD24_BYTE( "29.u167", 0x180001, 0x080000, CRC(453c3d3f) SHA1(151528b6b1e7f8c059d67dbaca61e7c382e9ce04) )
	ROM_LOAD24_BYTE( "17.u153", 0x180000, 0x080000, CRC(5586d086) SHA1(e43d5e8834701f40389400f68a99353e67598f6d) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "30.u69", 0x000000, 0x080000, CRC(3111a98a) SHA1(75e17a0113060a10551b2b8c17b19890eb7aa0a6) )
	ROM_LOAD( "31.u70", 0x080000, 0x080000, CRC(30cb2524) SHA1(85deb83262bbe481404705e163e5eb9362985b01) )
ROM_END

ROM_START( sokonuke )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "001-001.bin",  0x000000, 0x080000, CRC(9d0aa3ca) SHA1(f641c46f2c6e7f82bb9184daac62938afb607c09) )
	ROM_LOAD16_BYTE( "001-002.bin",  0x000001, 0x080000, CRC(96f2ef5f) SHA1(264e82e192089230f208edf609dee575bf5c6513) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "001-003.bin", 0x000000, 0x200000, CRC(ab9ba897) SHA1(650c1eadf82f6e2b4c598495c867118277565411) )

	ROM_REGION( 0x180000, "gfx2", 0 )   // Layer 1
	ROM_LOAD24_WORD_SWAP( "001-004.bin", 0x000001, 0x100000, CRC(34ca3540) SHA1(a9b6b395037870033a2a422453e304fd4666b99e) )
	ROM_LOAD24_BYTE     ( "001-005.bin", 0x000000, 0x080000, CRC(2b95d68d) SHA1(2fb480c31a6a7e180a68bd774b5f86348bea0761) )

	ROM_REGION( 0x100, "gfx3", ROMREGION_ERASE )    // Layer 2
	// Unused

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "001-006.bin",   0x080000, 0x080000, CRC(ecfac767) SHA1(3d05bdb2c2a8c7eb5fa77b0c4482f98d3947c6d6) )
	ROM_CONTINUE(              0x000000, 0x080000  )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "fw-001.u50", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "fw-002.u51", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "fw-003.u52", 0x400, 0x104, NO_DUMP )
	ROM_LOAD( "fw-004.u53", 0x600, 0x104, NO_DUMP )
	ROM_LOAD( "fw-005.u54", 0x800, 0x104, NO_DUMP )
ROM_END

ROM_START( zombraid )
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "fy001003.3",   0x000000, 0x080000, CRC(0b34b8f7) SHA1(8c6d7d208ece08695169f2e06806e7e55c595eb2) )
	ROM_LOAD16_BYTE( "fy001004.4",   0x000001, 0x080000, CRC(71bfeb1a) SHA1(75747b0c6e655624a5dc2e4fa8f16a6a51bd8769) )
	ROM_LOAD16_BYTE( "fy001002.103", 0x100000, 0x080000, CRC(313fd68f) SHA1(792733acc72b4719b3f7f79b57fb874c71e8abfb) )
	ROM_LOAD16_BYTE( "fy001001.102", 0x100001, 0x080000, CRC(a0f61f13) SHA1(ba14c5ae0d0b3f217c130eeebd987dfde4c64c0d) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "fy001006.200", 0x000000, 0x200000, CRC(e9ae99f7) SHA1(7ffd62e5db4a48d362c90e8fca991c5b63f22bd8) )

	ROM_REGION( 0x300000, "gfx2", 0 )   // Layer 1
	ROM_LOAD24_WORD_SWAP( "fy001008.66", 0x000001, 0x200000, CRC(73d7b0e1) SHA1(aa332b563005edb1a6e20fbceaba68b56761a634) )
	ROM_LOAD24_BYTE     ( "fy001007.65", 0x000000, 0x100000, CRC(b2fc2c81) SHA1(2c529beccea353c3e90563215ddf3d8931e0fb83) )

	ROM_REGION( 0x300000, "gfx3", 0 )   // Layer 2
	ROM_LOAD24_WORD_SWAP( "fy001010.68", 0x000001, 0x200000, CRC(8b40ed7a) SHA1(05fcd7947a8419cab5ed2305fba9a671911e4850) )
	ROM_LOAD24_BYTE     ( "fy001009.67", 0x000000, 0x100000, CRC(6bcca641) SHA1(49c9106e6f23e25e5b5917af11fc48d34457c61a) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "fy001012.b",  0x000000, 0x200000, CRC(fd30e102) SHA1(ae02f94f69aa301b0c37921ca1117e3ad20467b5) )
	ROM_LOAD( "fy001011.a",  0x200000, 0x200000, CRC(e3c431de) SHA1(1030adacbbfabc00231417e09f3de40e3052f65c) )

	ROM_REGION(0x10000, "nvram", 0)
	ROM_LOAD( "nvram.bin",  0x0000, 0x10000, CRC(1a4b2ee8) SHA1(9a14fb2089fef9d13e0a5fe0a83eb7bae51fe1ae) )

	ROM_REGION( 0xc00, "plds", 0 )
	ROM_LOAD( "fy-001.u206", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "fy-002.u116", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "fy-003.u14",  0x400, 0x104, NO_DUMP )
	ROM_LOAD( "fy-004.u35",  0x600, 0x104, NO_DUMP )
	ROM_LOAD( "fy-005.u36",  0x800, 0x104, NO_DUMP )
	ROM_LOAD( "fy-006.u76",  0xa00, 0x104, NO_DUMP )
ROM_END

/* Notes about the Proto/Test roms:

Each rom was labeled with PCB location, use, SUM16 and final date EX:

LOGO  Zombie Raid

U3 Master USA
PRG E_L DD28 9/28/95


The "LOGO" above means that the actual Sammy logo was printed there.

These look like final prototype or test roms before production and combining the data into larger MASK roms.
*/

ROM_START( zombraidp ) // Prototype or test board version.  Data matches released MASK ROM version
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "u3_master_usa_prg_e_l_dd28.u3",     0x000000, 0x080000, CRC(0b34b8f7) SHA1(8c6d7d208ece08695169f2e06806e7e55c595eb2) ) // These 4 ROMs dated 9/28/95
	ROM_LOAD16_BYTE( "u4_master_usa_prg_o_l_5e2b.u4",     0x000001, 0x080000, CRC(71bfeb1a) SHA1(75747b0c6e655624a5dc2e4fa8f16a6a51bd8769) )
	ROM_LOAD16_BYTE( "u103_master_usa_prg_e_h_789e.u103", 0x100000, 0x080000, CRC(313fd68f) SHA1(792733acc72b4719b3f7f79b57fb874c71e8abfb) )
	ROM_LOAD16_BYTE( "u102_master_usa_prg_o_h_1f25.u102", 0x100001, 0x080000, CRC(a0f61f13) SHA1(ba14c5ae0d0b3f217c130eeebd987dfde4c64c0d) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "u142_master_obj_00_1bb3.u142", 0x000001, 0x040000, CRC(ed6c8541) SHA1(a119ad751184d575e135da0598cf1172025ddd48) ) // These 6 ROMs dated 7/17/95
	ROM_LOAD16_BYTE(             "obj_01",           0x000000, 0x040000, CRC(a423620e) SHA1(8d9351eddeecb444a1ab2f99d0da1d74f01bad88) )
	ROM_LOAD16_BYTE( "u143_master_obj_04_b5aa.u143", 0x080001, 0x040000, CRC(1242670d) SHA1(e6189a1974981cedbd16f5ea0295ff73cfd6a666) )
	ROM_LOAD16_BYTE(             "obj_05",           0x080000, 0x040000, CRC(57fe3e97) SHA1(2f9e79d6984099495c0c025b11bea7c9f72f9ef4) )
	ROM_LOAD16_BYTE( "u146_master_obj_02_6cc6.u146", 0x100001, 0x040000, CRC(7562ee1b) SHA1(bffb1a172e5259687dc0bc84916127419825c1b7) )
	ROM_LOAD16_BYTE( "u144_master_obj_03_1cb5.u144", 0x100000, 0x040000, CRC(a83040f1) SHA1(c7efacaa706e7f07c95f23509c738665775db2d2) )
	ROM_LOAD16_BYTE( "u147_master_obj_06_c3d8.u147", 0x180001, 0x040000, CRC(a32c3da8) SHA1(fd63c0fb13fb546732351dc2ee5fa33c1c275274) )
	ROM_LOAD16_BYTE( "u145_master_obj_07_8ad4.u145", 0x180000, 0x040000, CRC(8071f0b6) SHA1(e596897e52beaf686e95ad643a36beefe311d85c) )

	ROM_REGION( 0x300000, "gfx2", ROMREGION_ERASE00 )   // Layer 1
	ROM_LOAD24_BYTE( "u148_master_scn_1-0_3ef8.u148", 0x000000, 0x080000, CRC(7d722f2a) SHA1(6bcd18fe65a4a94a718f75c3813cea014c80b35a) ) // These 6 ROMs dated 7/17/95
	ROM_LOAD24_BYTE( "u150_master_scn_1-1_89a6.u150", 0x000001, 0x080000, CRC(3c62a8af) SHA1(38d9a32817a928586fe027b4c974f7dde585e5b7) )
	ROM_LOAD24_BYTE( "u154_master_scn_1-2_0f4b.u154", 0x000000, 0x080000, CRC(0a1d647c) SHA1(66dede165438001a34317d5ab29a9553d25530a2) )
	ROM_LOAD24_BYTE( "u149_master_scn_1-3_71bb.u149", 0x180000, 0x080000, CRC(70d6af7f) SHA1(1c922882a90efe83eacf6e25c4fb0c0a9e29a22c) )
	ROM_LOAD24_BYTE( "u151_master_scn_1-4_872e.u151", 0x180001, 0x080000, CRC(83ef4d5f) SHA1(b7e804c3a702caaee320daf8604d0af6f5874946) )
	ROM_LOAD24_BYTE( "u155_master_scn_1-5_daef.u155", 0x180000, 0x080000, CRC(2508f67f) SHA1(43a9d56c49187891007457dd23d3ac696f8ce0fa) )

	ROM_REGION( 0x300000, "gfx3", ROMREGION_ERASE00 )   // Layer 2
	ROM_LOAD24_BYTE( "u164_master_scn_2-0_e79c.u164", 0x000002, 0x080000, CRC(f8c89062) SHA1(08fd32b30923025b3769e56a8601b2ea1f85ebd1) ) // These 6 ROMs dated 7/17/95
	ROM_LOAD24_BYTE( "u166_master_scn_2-1_0b75.u166", 0x000001, 0x080000, CRC(4d7a72d5) SHA1(83b7ca4ea4c83fdab5be3c17d816dfd4033fb89c) )
	ROM_LOAD24_BYTE( "u152_master_scn_2-2_c00e.u152", 0x000000, 0x080000, CRC(0870ad58) SHA1(20e076fa665c24db0e316598a0a5d7fae9fc2f2a) )
	ROM_LOAD24_BYTE( "u165_master_scn_2-3_be68.u165", 0x180002, 0x080000, CRC(8aaaef08) SHA1(12b9c8c170a4acf200e3fba45407a3e38f787926) )
	ROM_LOAD24_BYTE( "u167_master_scn_2-4_c515.u167", 0x180001, 0x080000, CRC(d22ff5c1) SHA1(c488a69c9a241e1ca2119264bf879140d16fe69f) )
	ROM_LOAD24_BYTE( "u153_master_scn_2-5_e1da.u153", 0x180000, 0x080000, CRC(814ac66a) SHA1(1fd13a0bf73b9bdede82a865789413308d989c3a) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "u156_master_snd_0_f630.u156", 0x000000, 0x080000, CRC(bfc467bd) SHA1(a234cb8e0259a21b7bad40a72d99bf379a4b4046) ) // These 8 ROMs dated 7/17/95
	ROM_LOAD( "u157_master_snd_1_c20a.u157", 0x080000, 0x080000, CRC(b449a8ba) SHA1(3248a767132f60dec848fdc21a76481caa428cd2) )
	ROM_LOAD( "u158_master_snd_2_5c69.u158", 0x100000, 0x080000, CRC(ed6de791) SHA1(416c39c03a9ac2214702eaea7716e9aa74c8c228) )
	ROM_LOAD( "u159_master_snd_3_0727.u159", 0x180000, 0x080000, CRC(794cec21) SHA1(5c44286ea14c0e6a7a4588e523015f83d64dd1a7) )
	ROM_LOAD( "u160_master_snd_4_5a70.u160", 0x200000, 0x080000, CRC(e81ace66) SHA1(f4984e855c222e1cf3287538f536f7b0275f03d5) )
	ROM_LOAD( "u161_master_snd_5_599c.u161", 0x280000, 0x080000, CRC(1793dd13) SHA1(1b5b3c50e6df399c3e334c08be5313eef7d7ed95) )
	ROM_LOAD( "u162_master_snd_6_6d2e.u162", 0x300000, 0x080000, CRC(2ece241f) SHA1(1ebe4dd788799ec10c2eddf02f9bdaee8457993b) )
	ROM_LOAD( "u163_master_snd_7_c733.u163", 0x380000, 0x080000, CRC(d90f78b2) SHA1(e847eba6a4d6c1a3044041a9d32b6b534fb45307) )

	ROM_REGION(0x10000, "nvram", 0)
	ROM_LOAD( "nvram.bin",  0x0000, 0x10000, CRC(1a4b2ee8) SHA1(9a14fb2089fef9d13e0a5fe0a83eb7bae51fe1ae) )
ROM_END

ROM_START( zombraidpj ) // Prototype or test board version.  Data matches released MASK rom version
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "u3_master_usa_prg_e_l_dd28.u3",     0x000000, 0x080000, CRC(0b34b8f7) SHA1(8c6d7d208ece08695169f2e06806e7e55c595eb2) ) // These 4 ROMs dated 9/28/95
	ROM_LOAD16_BYTE( "u4_master_jpn_prg_o_l_5e2c.u4",     0x000001, 0x080000, CRC(3cb6bdf0) SHA1(4c1babeb4d7dbf7d26f8e34b552c0338432abd57) )
	ROM_LOAD16_BYTE( "u103_master_usa_prg_e_h_789e.u103", 0x100000, 0x080000, CRC(313fd68f) SHA1(792733acc72b4719b3f7f79b57fb874c71e8abfb) )
	ROM_LOAD16_BYTE( "u102_master_usa_prg_o_h_1f25.u102", 0x100001, 0x080000, CRC(a0f61f13) SHA1(ba14c5ae0d0b3f217c130eeebd987dfde4c64c0d) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "u142_master_obj_00_1bb3.u142", 0x000001, 0x040000, CRC(ed6c8541) SHA1(a119ad751184d575e135da0598cf1172025ddd48) ) // These 6 ROMs dated 7/17/95
	ROM_LOAD16_BYTE(             "obj_01",           0x000000, 0x040000, CRC(a423620e) SHA1(8d9351eddeecb444a1ab2f99d0da1d74f01bad88) )
	ROM_LOAD16_BYTE( "u143_master_obj_04_b5aa.u143", 0x080001, 0x040000, CRC(1242670d) SHA1(e6189a1974981cedbd16f5ea0295ff73cfd6a666) )
	ROM_LOAD16_BYTE(             "obj_05",           0x080000, 0x040000, CRC(57fe3e97) SHA1(2f9e79d6984099495c0c025b11bea7c9f72f9ef4) )
	ROM_LOAD16_BYTE( "u146_master_obj_02_6cc6.u146", 0x100001, 0x040000, CRC(7562ee1b) SHA1(bffb1a172e5259687dc0bc84916127419825c1b7) )
	ROM_LOAD16_BYTE( "u144_master_obj_03_1cb5.u144", 0x100000, 0x040000, CRC(a83040f1) SHA1(c7efacaa706e7f07c95f23509c738665775db2d2) )
	ROM_LOAD16_BYTE( "u147_master_obj_06_c3d8.u147", 0x180001, 0x040000, CRC(a32c3da8) SHA1(fd63c0fb13fb546732351dc2ee5fa33c1c275274) )
	ROM_LOAD16_BYTE( "u145_master_obj_07_8ad4.u145", 0x180000, 0x040000, CRC(8071f0b6) SHA1(e596897e52beaf686e95ad643a36beefe311d85c) )

	ROM_REGION( 0x300000, "gfx2", ROMREGION_ERASE00 )   // Layer 1
	ROM_LOAD24_BYTE( "u148_master_scn_1-0_3ef8.u148", 0x000002, 0x080000, CRC(7d722f2a) SHA1(6bcd18fe65a4a94a718f75c3813cea014c80b35a) ) // These 6 ROMs dated 7/17/95
	ROM_LOAD24_BYTE( "u150_master_scn_1-1_89a6.u150", 0x000001, 0x080000, CRC(3c62a8af) SHA1(38d9a32817a928586fe027b4c974f7dde585e5b7) )
	ROM_LOAD24_BYTE( "u154_master_scn_1-2_0f4b.u154", 0x000000, 0x080000, CRC(0a1d647c) SHA1(66dede165438001a34317d5ab29a9553d25530a2) )
	ROM_LOAD24_BYTE( "u149_master_scn_1-3_71bb.u149", 0x180002, 0x080000, CRC(70d6af7f) SHA1(1c922882a90efe83eacf6e25c4fb0c0a9e29a22c) )
	ROM_LOAD24_BYTE( "u151_master_scn_1-4_872e.u151", 0x180001, 0x080000, CRC(83ef4d5f) SHA1(b7e804c3a702caaee320daf8604d0af6f5874946) )
	ROM_LOAD24_BYTE( "u155_master_scn_1-5_daef.u155", 0x180000, 0x080000, CRC(2508f67f) SHA1(43a9d56c49187891007457dd23d3ac696f8ce0fa) )

	ROM_REGION( 0x300000, "gfx3", ROMREGION_ERASE00 )   // Layer 2
	ROM_LOAD24_BYTE( "u164_master_scn_2-0_e79c.u164", 0x000002, 0x080000, CRC(f8c89062) SHA1(08fd32b30923025b3769e56a8601b2ea1f85ebd1) ) // These 6 ROMs dated 7/17/95
	ROM_LOAD24_BYTE( "u166_master_scn_2-1_0b75.u166", 0x000001, 0x080000, CRC(4d7a72d5) SHA1(83b7ca4ea4c83fdab5be3c17d816dfd4033fb89c) )
	ROM_LOAD24_BYTE( "u152_master_scn_2-2_c00e.u152", 0x000000, 0x080000, CRC(0870ad58) SHA1(20e076fa665c24db0e316598a0a5d7fae9fc2f2a) )
	ROM_LOAD24_BYTE( "u165_master_scn_2-3_be68.u165", 0x180002, 0x080000, CRC(8aaaef08) SHA1(12b9c8c170a4acf200e3fba45407a3e38f787926) )
	ROM_LOAD24_BYTE( "u167_master_scn_2-4_c515.u167", 0x180001, 0x080000, CRC(d22ff5c1) SHA1(c488a69c9a241e1ca2119264bf879140d16fe69f) )
	ROM_LOAD24_BYTE( "u153_master_scn_2-5_e1da.u153", 0x180000, 0x080000, CRC(814ac66a) SHA1(1fd13a0bf73b9bdede82a865789413308d989c3a) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "u156_master_snd_0_f630.u156", 0x000000, 0x080000, CRC(bfc467bd) SHA1(a234cb8e0259a21b7bad40a72d99bf379a4b4046) ) // These 8 ROMs dated 7/17/95
	ROM_LOAD( "u157_master_snd_1_c20a.u157", 0x080000, 0x080000, CRC(b449a8ba) SHA1(3248a767132f60dec848fdc21a76481caa428cd2) )
	ROM_LOAD( "u158_master_snd_2_5c69.u158", 0x100000, 0x080000, CRC(ed6de791) SHA1(416c39c03a9ac2214702eaea7716e9aa74c8c228) )
	ROM_LOAD( "u159_master_snd_3_0727.u159", 0x180000, 0x080000, CRC(794cec21) SHA1(5c44286ea14c0e6a7a4588e523015f83d64dd1a7) )
	ROM_LOAD( "u160_master_snd_4_5a70.u160", 0x200000, 0x080000, CRC(e81ace66) SHA1(f4984e855c222e1cf3287538f536f7b0275f03d5) )
	ROM_LOAD( "u161_master_snd_5_599c.u161", 0x280000, 0x080000, CRC(1793dd13) SHA1(1b5b3c50e6df399c3e334c08be5313eef7d7ed95) )
	ROM_LOAD( "u162_master_snd_6_6d2e.u162", 0x300000, 0x080000, CRC(2ece241f) SHA1(1ebe4dd788799ec10c2eddf02f9bdaee8457993b) )
	ROM_LOAD( "u163_master_snd_7_c733.u163", 0x380000, 0x080000, CRC(d90f78b2) SHA1(e847eba6a4d6c1a3044041a9d32b6b534fb45307) )

	ROM_REGION(0x10000, "nvram", 0)
	ROM_LOAD( "nvram.bin",  0x0000, 0x10000, CRC(1a4b2ee8) SHA1(9a14fb2089fef9d13e0a5fe0a83eb7bae51fe1ae) )
ROM_END

ROM_START( madshark )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "fq001002.201",  0x000000, 0x080000, CRC(4286a811) SHA1(c8d4a28008548fe7d1d70758462205862142c56b) )
	ROM_LOAD16_BYTE( "fq001001.200",  0x000001, 0x080000, CRC(38bfa0ad) SHA1(59398ef69caa01f51cdfb20db23af494db658e5e) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "fq001004.202", 0x100000, 0x100000, CRC(e56a1b5e) SHA1(f0dd34122fd7db15cc56714b72b60d07ccb59222) )
	ROM_CONTINUE(             0x000000, 0x100000 )

	ROM_REGION( 0x300000, "user1", 0 )  // Layers 1+2
	ROM_LOAD24_WORD_SWAP( "fq001006.152", 0x000001, 0x200000, CRC(3bc5e8e4) SHA1(74cdf1bb2e58bef29c6f4371ff40f64472bff3ce) )
	ROM_LOAD24_BYTE     ( "fq001005.205", 0x000000, 0x100000, CRC(5f6c6d4a) SHA1(eed5661738282a14ce89917335fd1b695eb7351e) ) // 1xxxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x180000, "gfx2", 0 )   // Layer 1
	ROM_COPY( "user1", 0x000000, 0x000000, 0x180000 )

	ROM_REGION( 0x180000, "gfx3", 0 )   // Layer 2
	ROM_COPY( "user1", 0x180000, 0x000000, 0x180000 )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "fq001007.26", 0x000000, 0x100000, CRC(e4b33c13) SHA1(c4f9532de7a09c80f5a74c3a386e99a0f546846f) )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "fq-008.u50", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "fq-009.u51", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "fq-010.u52", 0x400, 0x104, NO_DUMP )
	ROM_LOAD( "fq-011.u53", 0x600, 0x104, NO_DUMP )
	ROM_LOAD( "fq-012.u54", 0x800, 0x104, NO_DUMP )
ROM_END

// same PCB as Triple Fun
ROM_START( madsharkbl )
	ROM_REGION( 0x180000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "prog2.040",  0x000000, 0x080000, CRC(210e5771) SHA1(3c392cf34b1fb071ba9267c9a6dfd0e015f71c7a) )
	ROM_LOAD16_BYTE( "prog1.040",  0x000001, 0x080000, CRC(dca34f11) SHA1(a8012bf57cc61f9a8106472c5f4bf768ee1cd633) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "gfx2.3.040", 0x000000, 0x080000, BAD_DUMP CRC(8e1f5e4e) SHA1(e55718cf00aa44bd6e67faa7b69d54a669cc01f8) ) // ROM wouldn't read correctly, taken from original and split
	ROM_LOAD16_BYTE( "gfx2.1.040", 0x000001, 0x080000, BAD_DUMP CRC(97cae9ce) SHA1(8031dfb2429861fea2c30541be48d270a8a021cf) ) // suspect dump, fq001004.202 [odd 2/2]  99.999809%, just one single bit (!) difference
	ROM_LOAD16_BYTE( "gfx2.4.040", 0x100000, 0x080000, BAD_DUMP CRC(921e8b4c) SHA1(d022da7b76cae9413ab64cb9e34f0c000710d408) ) // ROM wouldn't read correctly, taken from original and split
	ROM_LOAD16_BYTE( "gfx2.2.040", 0x100001, 0x080000, CRC(b22c8227) SHA1(295c2377db4402e1ab5d491f1dfd5a30ef022934) )

	ROM_REGION( 0x300000, "user1", ROMREGION_ERASE00 )  // Layers 1+2
	ROM_LOAD24_BYTE( "center.040", 0x000000, 0x080000, CRC(6210413f) SHA1(466580b20ba8e7beac9c386854ffad49fc3956c4) )
	ROM_LOAD24_BYTE( "gfx1.3.040", 0x000001, 0x080000, CRC(91735a7a) SHA1(68f8ea29998a9b56b581a0b5f370c82547b4a331) )
	ROM_LOAD24_BYTE( "gfx1.1.040", 0x000002, 0x080000, CRC(ef10d0ee) SHA1(5245f4e9cc89c8f7d1e43f9e169f1bf177791522) )
	ROM_LOAD24_BYTE( "gfx1.4.040", 0x180001, 0x080000, CRC(8ebf40da) SHA1(0f5885ea04247b3bc1d1a2d6f53fab3e55b7fbeb) )
	ROM_LOAD24_BYTE( "gfx1.2.040", 0x180002, 0x080000, CRC(81502591) SHA1(92c113c1e4d322d56e93ef7e0d7a40495f811884) )

	ROM_REGION( 0x180000, "gfx2", 0 )   // Layer 1
	ROM_COPY( "user1", 0x000000, 0x000000, 0x180000 )

	ROM_REGION( 0x180000, "gfx3", 0 )   // Layer 2
	ROM_COPY( "user1", 0x180000, 0x000000, 0x180000 )

	ROM_REGION( 0x80000, "oki", 0 )  // Samples
	ROM_LOAD( "oki.040", 0x00000, 0x80000, CRC(b12ff374) SHA1(ca959f95bccdf453a13a1796aeeb5364503ad7bc) )
ROM_END

ROM_START( magspeed )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 Code
	ROM_LOAD16_BYTE( "fu001002.201", 0x00000, 0x40000, CRC(bdeb3fcc) SHA1(3a69eae49967fdad1f9bda6a09bffbd824254c92) )
	ROM_LOAD16_BYTE( "fu001001.200", 0x00001, 0x40000, CRC(9b873d46) SHA1(958502dea9f271249da715cd6b1ea5045369cbb9) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "fu001004.21", 0x000000, 0x100000, CRC(7582c5a8) SHA1(3754e3bbac8e4a50f5ca28390357f00b7579182d) )
	ROM_LOAD( "fu001005.22", 0x100000, 0x100000, CRC(fd4b1ff6) SHA1(188b74cdf120e9d6e0fe15b60997383929dfa5cd) )

	ROM_REGION( 0x100000, "user1", 0 )  // Layers 1+2
	ROM_LOAD16_WORD_SWAP( "fu001006.152", 0x000000, 0x100000, CRC(70855139) SHA1(24d635aceb823b0569169c8ecced13ac82c17d6a) )

	ROM_REGION( 0x80000, "gfx2", 0 )    // Layer 1
	ROM_COPY( "user1", 0x000000, 0x00000, 0x80000 )

	ROM_REGION( 0x80000, "gfx3", 0 )    // Layer 2
	ROM_COPY( "user1", 0x80000, 0x00000, 0x80000 )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "fu001007.26", 0x000000, 0x100000, CRC(173463c2) SHA1(f7afc200662f72b3da149e0d17517c89ad66ef67) )
ROM_END

ROM_START( utoukond )
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "93uta010.3",  0x000000, 0x080000, CRC(c486ef5e) SHA1(36e4ef4805d543216269f1161028d8a436f72284) )
	ROM_LOAD16_BYTE( "93uta011.4",  0x000001, 0x080000, CRC(978978f7) SHA1(a7fd3a4ce3a7c6c9d9bdd60df29b4f427abf4f92) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        // Z80 Code
	ROM_LOAD( "93uta009.112", 0x0000, 0x10000, CRC(67f18483) SHA1(d9af58dec09c317ccab65553d53d82c8cce2bfb9) )

	ROM_REGION( 0x400000, "gfx1", ROMREGION_INVERT )    // Sprites
	ROM_LOAD( "93uta04.64",  0x000000, 0x100000, CRC(9cba0538) SHA1(83278918b6ad160d3e53c178b3cad252e7b0edfb) )
	ROM_LOAD( "93uta02.201", 0x100000, 0x100000, CRC(884fedfa) SHA1(3710003bd2e55bba03e2720fcab0fe080163222d) )
	ROM_LOAD( "93uta03.63",  0x200000, 0x100000, CRC(818484a5) SHA1(642252abe56e26aa8376db2e25b192b11586d1e4) )
	ROM_LOAD( "93uta01.200", 0x300000, 0x100000, CRC(364de841) SHA1(a025bd57f60eac05c0d7b4fb69b4b4979f357e6b) )

	ROM_REGION( 0x100000, "gfx2", 0 )   // Layer 1
	ROM_LOAD16_WORD_SWAP( "93uta05.66",  0x000000, 0x100000, CRC(5e640bfb) SHA1(37f30ae6ab9d7860da6ca6a343fa9adf4b3d355c) )

	ROM_REGION( 0x200000, "gfx3", 0 )   // Layer 2
	ROM_LOAD16_WORD_SWAP( "93uta07.68",  0x000000, 0x100000, CRC(67bdd036) SHA1(527b6a67e7a62263bee738dc82d6ff289ab54853) )
	ROM_LOAD16_WORD_SWAP( "93uta06.67",  0x100000, 0x100000, CRC(294c26e4) SHA1(459ec7f8c8db4f1e3906d5db240298405bda991c) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "93uta08.69", 0x000000, 0x100000, CRC(3d50bbcd) SHA1(e9b78d08466e1f9b42f11999bb53b6deceb81a12) )

	ROM_REGION( 0xc00, "plds", 0 )
	ROM_LOAD( "93ut-a12.u206", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "93ut-a13.u14",  0x200, 0x104, NO_DUMP )
	ROM_LOAD( "93ut-a14.u35",  0x400, 0x104, NO_DUMP )
	ROM_LOAD( "93ut-a15.u36",  0x600, 0x104, NO_DUMP )
	ROM_LOAD( "93ut-a16.u110", 0x800, 0x104, NO_DUMP )
	ROM_LOAD( "93ut-a17.u76",  0xa00, 0x104, NO_DUMP )
ROM_END

ROM_START( neobattl )   // 1CC74: "SD GUNDAM v0.00. 1992/11/04 10:04:33"
	ROM_REGION( 0x100000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "bp923001.u45", 0x000000, 0x020000, CRC(0d0aeb73) SHA1(5ca631d5d68e53029f379d9877a056997c6c6afa) ) // Alt label X1-001 which is also a Seta custom chip number
	ROM_LOAD16_BYTE( "bp923002.u46", 0x000001, 0x020000, CRC(9731fbbc) SHA1(196c913fb67496f9da2943ad1e69edf89cb65fdf) ) // Alt label X1-002A which is also a Seta custom chip number

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "bp923-003.u15", 0x00000, 0x80000, CRC(91ca98a1) SHA1(b02b362e3a6118f52d9e1a262ca11aecef887b00) )
	ROM_LOAD( "bp923-004.u9",  0x80000, 0x80000, CRC(15c678e3) SHA1(8c0fa41a1f4e7b4e1c90faaeec7f6c910cc3ad0b) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "bp923-005.u4", 0x000000, 0x100000, CRC(7c0e37be) SHA1(5d5779de948f986971a82db2a5a4302044c3257a) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "bp923-007.u37", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "bp923-008.u38", 0x200, 0x104, NO_DUMP )
ROM_END

ROM_START( pairlove )
	ROM_REGION( 0x040000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "ut2-001-001.1a",  0x000000, 0x010000, CRC(083338b7) SHA1(d775c1618272967713bd3f3164fdfc42dc5c36ca) )
	ROM_LOAD16_BYTE( "ut2-001-002.3a",  0x000001, 0x010000, CRC(39d88aae) SHA1(8498dfb221e9b34a889594fe5ed0431814b733e6) )

	ROM_REGION( 0x100000, "gfx1", 0 )   // Sprites
	ROM_LOAD( "ut2-001-004.5j",  0x000000, 0x080000, CRC(fdc47b26) SHA1(0de51bcf67b909ac9578f0d1b14af8a4c758aacf) )
	ROM_LOAD( "ut2-001-005.5l",  0x080000, 0x080000, CRC(076f94a2) SHA1(94b4b41a497dea1b6db5396bd7cd81ebcb217735) )

	ROM_REGION( 0x80000, "x1snd", 0 )  // Samples
	ROM_LOAD( "ut2-001-003.12a",  0x000000, 0x080000, CRC(900219a9) SHA1(3260a900df25beba597bf947a9fbb6f7392827d7) )
ROM_END

ROM_START( crazyfgt )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 68000 Code
	ROM_LOAD16_BYTE( "rom.u3", 0x00000, 0x40000, CRC(bf333e75) SHA1(be124558ca49963cc56d3255c546587558b61926) )
	ROM_LOAD16_BYTE( "rom.u4", 0x00001, 0x40000, CRC(505e9d47) SHA1(3797d396a24e46b891de4c40aafe960d1cf5f161) )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "rom.u225",  0x000000, 0x80000, CRC(451b4419) SHA1(ab32b3c452b566ddfc64c0a80a257c3baadd8f41) )
	ROM_LOAD16_BYTE( "rom.u226",  0x000001, 0x80000, CRC(ef210e34) SHA1(99241ffcbc8af889c8ab6f0bc67eedef27d455f0) )
	ROM_LOAD16_BYTE( "rom.u227",  0x100000, 0x80000, CRC(7905b5f2) SHA1(633f86bf2be620afbe8012ade5d1e59c359a25d4) )
	ROM_LOAD16_BYTE( "rom.u228",  0x100001, 0x80000, CRC(7181618e) SHA1(57c5aced95b0a11a43dc9bd532290f067113e65a) )

	ROM_REGION( 0xc0000, "gfx2", 0 )    // Layer 1
	ROM_LOAD24_BYTE     ( "rom.u67",  0x000000, 0x40000, CRC(ec8c6831) SHA1(e0ef1c2e539c1780fc5816ec950d33cb2a69d55e) )
	ROM_LOAD24_WORD_SWAP( "rom.u68",  0x000001, 0x80000, CRC(2124312e) SHA1(1c6053c87a975bfdf910e75bd3e38d0898806ea0) )

	ROM_REGION( 0xc0000, "gfx3", 0 )    // Layer 2
	ROM_LOAD24_BYTE     ( "rom.u65",  0x000000, 0x40000, CRC(58448231) SHA1(711f24831777719f6a7b143f4f1bfd14f5a9ed4c) )
	ROM_LOAD24_WORD_SWAP( "rom.u66",  0x000001, 0x80000, CRC(c6f7735b) SHA1(0e77045f82d0bf659be5dbfe21cfc8f223faeee9) )

	ROM_REGION( 0x40000, "oki", 0 ) // OKI samples
	ROM_LOAD( "rom.u85",  0x00000, 0x40000, CRC(7b95d0bb) SHA1(f16dfd639eed6856e3ab93704caef592a07ba367) )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.bin", 0x00, 0x28, CRC(3b5ea5d2) SHA1(bbaad9a879834d1510f20321b97de558d9759ec7) BAD_DUMP ) // handcrafted to pass protection check
ROM_END

ROM_START( jockeyc )
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE(      "ya_007_002.u23", 0x000000, 0x10000, CRC(c499bf4d) SHA1(2417eac2972bbb0f8f0a4a1fd72c9d78537367c7) )
	ROM_LOAD16_BYTE(      "ya_007_003.u33", 0x000001, 0x10000, CRC(e7b0677e) SHA1(90dbd710623ff57b953483240e1006c9bda3fc91) )
	ROM_FILL(                               0x020000, 0xe0000, 0xff )
	ROM_LOAD16_WORD_SWAP( "ya-002-001.u18", 0x100000, 0x80000, CRC(dd108016) SHA1(1554de4cc1a9436a1e62400cd96c9752a2098f99) )
	ROM_FILL(                               0x180000, 0x80000, 0xff )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "ya-001-007-t77.u27", 0x000000, 0x80000, CRC(2dc7a294) SHA1(97f2aa9939a45aaa94d4aeb2fcd5b7f30204b942) )
	ROM_LOAD16_BYTE( "ya-001-006-t76.u22", 0x000001, 0x80000, CRC(bfae01a5) SHA1(3be83972c3987e9bf722cd6db7770f074587301c) )
	ROM_LOAD16_BYTE( "ya-001-005-t75.u17", 0x100000, 0x80000, CRC(4a6c804b) SHA1(b596b9b0b3b453c26f9c7f976ff4d56eac4fac04) )
	ROM_LOAD16_BYTE( "ya-001-004-t74.u10", 0x100001, 0x80000, CRC(eb74d2e0) SHA1(221ff6cc03ce57a7fcbe418f1c12a293990f8a7d) )

	ROM_REGION( 0x80000, "gfx2", 0 )    // Layer 1
	ROM_LOAD16_BYTE( "ya-001-008-t59.u35", 0x000000, 0x40000, CRC(4b890f83) SHA1(fde6544898a0691b550f3045803f2e81cfeb5fe9) )
	ROM_LOAD16_BYTE( "ya-001-009-t60.u41", 0x000001, 0x40000, CRC(caa5e3c1) SHA1(63cccc5479040a02872febc8d7f2d46096e138d1) )

	ROM_REGION( 0x400, "proms", 0 ) // Colours
	ROM_LOAD16_BYTE( "ya1-010.prom", 0x000, 0x200, CRC(778094b3) SHA1(270329a0d544dc7a8240d6dab08ccd54ea87ab70) )
	ROM_LOAD16_BYTE( "ya1-011.prom", 0x001, 0x200, CRC(bd4fe2f6) SHA1(83d9f9db3fbfa2d172f5227c397ea4d5a9687015) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "ya-001-013.u71", 0x00000, 0x80000, CRC(2bccaf47) SHA1(1658643444d575410f11b648e0d7ae6c43fcf1ea) )
	ROM_LOAD( "ya-001-012.u64", 0x80000, 0x80000, CRC(a8015ce6) SHA1(bb0b589856ec82e1fd42be9af89b07ba1d17e595) )
ROM_END

/***************************************************************************

    International Toote II (v1.24, P387.V01)
    (C) 1993 Coinmaster

    Program roms which were in use in Belgium. "International Toote" was the only name allowed in Belgium
    those days for horse racing gambling games, so every horse game was named International Toote
    (which means nothing even for us).

    I don't really know if Coinmaster owned the rights for this game or not, but they reverse engineered it,
    added some protection and made eproms sets for Germany, Belgium etc ... with their name and logo.

    Roms:

    P387_v01.002    new
    P387_v01.003    new
    YA-001-004.u10  same as YA-011-xxx from: International Toote (Germany, P523.V01)
    YA-001-005.u17  ""
    YA-001-006.u22  ""
    YA-001-007.u27  ""
    YA-001-008.u35  ""
    YA-001-009.u41  ""
    YA-001-012.u64  ""
    YA-001-013.u71  ""
    YA-002-001.u18  ""
    YA-010.prom     ""
    YA-011.prom     ""

    This set does not use the "fore" and "back" graphics roms

***************************************************************************/

ROM_START( inttoote2 )
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "p387.v01_horse_prog_2.002", 0x000000, 0x10000, CRC(1ced885e) SHA1(7bb444bbfa3c07c0c54378432186ff3b056b6090) )
	ROM_LOAD16_BYTE( "p387.v01_horse_prog_1.003", 0x000001, 0x10000, CRC(e24592af) SHA1(86ab84cb1c5cbb0dcc73e75c05ce446411fab08a) )
	ROM_FILL(                                     0x020000, 0xe0000, 0xff )
	ROM_LOAD16_WORD_SWAP( "ya_002_001.u18",       0x100000, 0x80000, CRC(dd108016) SHA1(1554de4cc1a9436a1e62400cd96c9752a2098f99) )
	ROM_FILL(                                     0x180000, 0x80000, 0xff )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "ya-001-007-t77.u27", 0x000000, 0x80000, CRC(2dc7a294) SHA1(97f2aa9939a45aaa94d4aeb2fcd5b7f30204b942) )
	ROM_LOAD16_BYTE( "ya-001-006-t76.u22", 0x000001, 0x80000, CRC(bfae01a5) SHA1(3be83972c3987e9bf722cd6db7770f074587301c) )
	ROM_LOAD16_BYTE( "ya-001-005-t75.u17", 0x100000, 0x80000, CRC(4a6c804b) SHA1(b596b9b0b3b453c26f9c7f976ff4d56eac4fac04) )
	ROM_LOAD16_BYTE( "ya-001-004-t74.u10", 0x100001, 0x80000, CRC(eb74d2e0) SHA1(221ff6cc03ce57a7fcbe418f1c12a293990f8a7d) )

	ROM_REGION( 0x80000, "gfx2", 0 )    // Layer 1
	ROM_LOAD16_BYTE( "ya-001-008-t59.u35", 0x000000, 0x40000, CRC(4b890f83) SHA1(fde6544898a0691b550f3045803f2e81cfeb5fe9) )
	ROM_LOAD16_BYTE( "ya-001-009-t60.u41", 0x000001, 0x40000, CRC(caa5e3c1) SHA1(63cccc5479040a02872febc8d7f2d46096e138d1) )

	ROM_REGION( 0x400, "proms", 0 ) // Colours
	ROM_LOAD16_BYTE( "ya-010.prom", 0x000, 0x200, CRC(778094b3) SHA1(270329a0d544dc7a8240d6dab08ccd54ea87ab70) )
	ROM_LOAD16_BYTE( "ya-011.prom", 0x001, 0x200, CRC(bd4fe2f6) SHA1(83d9f9db3fbfa2d172f5227c397ea4d5a9687015) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "ya-001-013.u71", 0x00000, 0x80000, CRC(2bccaf47) SHA1(1658643444d575410f11b648e0d7ae6c43fcf1ea) )
	ROM_LOAD( "ya-001-012.u64", 0x80000, 0x80000, CRC(a8015ce6) SHA1(bb0b589856ec82e1fd42be9af89b07ba1d17e595) )
ROM_END

ROM_START( inttoote )
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "p523.v01_horse_prog_2.002", 0x000000, 0x10000, CRC(6ce6f1ad) SHA1(82e7100721ca5b1a736f6523610b1f1edf225c12) ) // 27/8/98 German
	ROM_LOAD16_BYTE( "p523.v01_horse_prog_1.003", 0x000001, 0x10000, CRC(921fcff5) SHA1(cabc4e9936621132a6fbaa1a925d205c5f04a2ae) ) // ""
	ROM_FILL(                                     0x020000, 0xe0000, 0xff )
	ROM_LOAD16_WORD_SWAP( "ya_002_001.u18",       0x100000, 0x80000, CRC(dd108016) SHA1(1554de4cc1a9436a1e62400cd96c9752a2098f99) )
	ROM_FILL(                                     0x180000, 0x80000, 0xff )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "ya_011_007.u27",             0x000000, 0x80000, CRC(2dc7a294) SHA1(97f2aa9939a45aaa94d4aeb2fcd5b7f30204b942) )
	ROM_LOAD16_BYTE( "ya_011_006.u22",             0x000001, 0x80000, CRC(bfae01a5) SHA1(3be83972c3987e9bf722cd6db7770f074587301c) )
	ROM_LOAD16_BYTE( "p523.v01_horse_fore_3.u133", 0x0e0001, 0x10000, CRC(c38596af) SHA1(d27141e28d8f8352f065c55121412e604c199a9a) )
	ROM_LOAD16_BYTE( "p523.v01_horse_fore_4.u132", 0x0e0000, 0x10000, CRC(64ef345e) SHA1(ef5d9f293ded44a2be91278549f5db8673fc7571) )
	ROM_LOAD16_BYTE( "ya_011_005.u17",             0x100000, 0x80000, CRC(4a6c804b) SHA1(b596b9b0b3b453c26f9c7f976ff4d56eac4fac04) )
	ROM_LOAD16_BYTE( "ya_011_004.u10",             0x100001, 0x80000, CRC(eb74d2e0) SHA1(221ff6cc03ce57a7fcbe418f1c12a293990f8a7d) )
	ROM_LOAD16_BYTE( "p523.v01_horse_fore_2.u134", 0x1e0000, 0x10000, CRC(26fb0339) SHA1(a134ecef00f690c82c8bddf26498b357ccf8d5c3) )
	ROM_LOAD16_BYTE( "p523.v01_horse_fore_1.u135", 0x1e0001, 0x10000, CRC(3a75df30) SHA1(f3b3a7428e3e125921686bc9aacde6b28b1947b5) )

	ROM_REGION( 0xc0000, "gfx2", 0 )    // Layer 1
	ROM_LOAD16_BYTE( "ya_011_008.u35",             0x000000, 0x40000, CRC(4b890f83) SHA1(fde6544898a0691b550f3045803f2e81cfeb5fe9) )
	ROM_LOAD16_BYTE( "ya_011_009.u41",             0x000001, 0x40000, CRC(caa5e3c1) SHA1(63cccc5479040a02872febc8d7f2d46096e138d1) )
	ROM_LOAD16_BYTE( "p523.v01_horse_back_1.u137", 0x080000, 0x20000, CRC(39b221ea) SHA1(3b3367430733ed36d6a981cd2ec6df731d07c089) )
	ROM_LOAD16_BYTE( "p523.v01_horse_back_2.u136", 0x080001, 0x20000, CRC(9c5e32a0) SHA1(964734a626b5c7b9d7130addc642895df520dcb7) )

	ROM_REGION( 0x400, "proms", 0 ) // Colours
	ROM_LOAD16_BYTE( "ya-010.prom", 0x000, 0x200, CRC(778094b3) SHA1(270329a0d544dc7a8240d6dab08ccd54ea87ab70) )
	ROM_LOAD16_BYTE( "ya-011.prom", 0x001, 0x200, CRC(bd4fe2f6) SHA1(83d9f9db3fbfa2d172f5227c397ea4d5a9687015) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "ya_011_013.u71", 0x00000, 0x80000, CRC(2bccaf47) SHA1(1658643444d575410f11b648e0d7ae6c43fcf1ea) )
	ROM_LOAD( "ya_011_012.u64", 0x80000, 0x80000, CRC(a8015ce6) SHA1(bb0b589856ec82e1fd42be9af89b07ba1d17e595) )
ROM_END

// Gran Derby (Spanish hack of Jockey Club)
// CODERE massive production.

ROM_START( gderby )
	ROM_REGION( 0x200000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE(      "2even.new", 0x000000, 0x10000, CRC(9f96a2ed) SHA1(b5d112f6e795863e3daffa4707ca4e059e6fa667) )
	ROM_LOAD16_BYTE(      "3odd.new",  0x000001, 0x10000, CRC(16507ed8) SHA1(26bcebb18baf94307fdf6c67c73714cd4a5beb63) )
	ROM_FILL(                               0x020000, 0xe0000, 0xff )
	ROM_LOAD16_WORD_SWAP( "ya-002-001.u18", 0x100000, 0x80000, CRC(dd108016) SHA1(1554de4cc1a9436a1e62400cd96c9752a2098f99) )
	ROM_FILL(                               0x180000, 0x80000, 0xff )

	ROM_REGION( 0x200000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "007.u27", 0x000000, 0x80000, CRC(66076d4d) SHA1(96af5113c003f2e1f07e527a4fdfd8d01afe177d) )
	ROM_LOAD16_BYTE( "006.u22", 0x000001, 0x80000, CRC(efd15539) SHA1(90b1862eef44394837e90edd84edcb472228ad97) )
	ROM_LOAD16_BYTE( "005.u17", 0x100000, 0x80000, CRC(db177298) SHA1(ba7316df9554b8bd78835f324dc8e755434f709d) )
	ROM_LOAD16_BYTE( "004.u10", 0x100001, 0x80000, CRC(42b3aa98) SHA1(da6dd900902adf5a4bc01e39eab31306602b43ff) )

	ROM_REGION( 0x80000, "gfx2", 0 )    // Layer 1
	ROM_LOAD16_BYTE( "008.u37", 0x000000, 0x40000, CRC(5fbdfa14) SHA1(271dbdf0510a135c54adb90e685ee2abbe6a946f) )
	ROM_LOAD16_BYTE( "009.u41", 0x000001, 0x40000, CRC(86e9c33c) SHA1(e2558533626daf21578215b8bfd950a799f858b9) )

	ROM_REGION( 0x400, "proms", 0 ) // Colours
	ROM_LOAD16_BYTE( "ya1-010.prom", 0x000, 0x200, CRC(778094b3) SHA1(270329a0d544dc7a8240d6dab08ccd54ea87ab70) )
	ROM_LOAD16_BYTE( "ya1-011.prom", 0x001, 0x200, CRC(bd4fe2f6) SHA1(83d9f9db3fbfa2d172f5227c397ea4d5a9687015) )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "ya-001-013.u71", 0x00000, 0x80000, CRC(2bccaf47) SHA1(1658643444d575410f11b648e0d7ae6c43fcf1ea) )
	ROM_LOAD( "ya-001-012.u64", 0x80000, 0x80000, CRC(a8015ce6) SHA1(bb0b589856ec82e1fd42be9af89b07ba1d17e595) )
ROM_END


/***************************************************************************

The Roulette (Visco)

PCB P0-047A

3x8 DSW
SETA X1-004 (826100) (input)
SETA X1-010 (811101) (sound)
MC68B50P (ACIA)
uPD4911C (RTC)
M68000
16MHz OSC, near CPU
SETA X1-002A (sprites, near 005 - 008)
SETA X1-001A (sprites, "")
SETA X1-007 (737100)
SETA X1-011
SETA X1-012
 _____________________________________________________________________________________________________________________
|     W              T  S      R     P   N     M     L   K      J     H     F    E      D            C     B    A     |
|          __________      _______   _______   _______   _______                 ______________________________       |
|         |Hd74LS74AF|    |UF0    | |UF0    | |UF0    | |UF0    |               |                              |      |
|         |__________|    |008    | |007    | |006    | |005    |               |                              |     1|
|  __________________     |       | |       | |       | |       |               |                              |      |
| |UF0_009        W15|    |TOSHIBA| |TOSHIBA| |TOSHIBA| |TOSHIBA|               |                              |      |
| |                  |    |       | |       | |       | |       |               |                              |      |
| |   4M             |    |TMM2725| |TMM2725| |TMM2725| |TMM2725|   __          |______________________________|     2|
| |U13               |    |6AD_20 | |6AD_20 | |6BD_15 | |6BD_15 |  |  |   __                                          |
| |__________________|    |       | |       | |       | |       |  |  |  |  |       __                                |
|  __________________     |       | |       | |       | |       |  |S |  |  |  __  |  |                              3|
| |UF0_010        W16|    |       | |       | |       | |       |  |  |  |T | |  | |  |                               |
| |                  |    |       | |       | |       | |       |  |  |  |  | |  | |U |                               |
| |   4M             |    |_______| |_______| |_______| |_______|  |__|  |  | |A1| |  |                              4|
| |U15               |  __                                               |__| |  | |  |                               |
| |__________________| |  |     _______________________________    _____      |  | |__|        __________________     |
|  __________________  |  |    |                               |  |XTAL |     |__|            |UF1 002           |    |
| |UF0_011        W17| |  |    |             SETA              |  |     |   __                |                  |   5|
| |                  | |  |    |            8820KX             |  |16MHz|  |  |    __   __    |                  |    |
| |   4M             | |  |    |            X1_002A            |  |     |  |  |   |  | |  |   |27512             |    |
| |U22               | |__|    |                               |  |     |  |B |   |  | |  |   |__________________|   6|
| |__________________|         |                               |  |_____|  |  |   |V | |W |    __________________     |
|  __________________          |_______________________________|           |  |   |  | |  |   |UF1 003           |    |
| |UF0_012        W18|  __                                            __   |__|   |  | |  |   |                  |    |
| |                  | |  |     _______________________________      |  |         |__| |__|   |                  |   7|
| |   4M             | |  |    |                               |     |  |                     |27512             |    |
| |U29               | |  |    |             SETA              |     |  |   __     __         |__________________|    |
| |__________________| |  |    |            8836KX             |     |X |  |  |   |  |                                |
|  __________________  |  |    |            X1_001A            |     |  |  |  |   |  |                                |
| |UF0_013        W19| |__|    |                               |     |  |  |D |   |D |                               8|
| |                  |         |                               |     |  |  |  |   |  |         __________________     |
| |   4M             |         |_______________________________|     |  |  |  |   |  |        |   TMM2063AP_12   |    |
| |U37               |                                               |__|  |__|   |__|        |__________________|    |
| |__________________|                                                __    __    __                                  |
|  __________________                                                |  |  |  |  |  |   __                           9|
| |UF0_014        W20|     ____________                              |  |  |  |  |  |  |  |    __________________     |
| |                  |    |            |      __________________     |  |  |  |  |  |  |  |   |                  |    |
| |   4M             |    |    SETA    |     |   TMM2063AP_10   |    |Y |  |A |  |A |  |S |   |__________________|    |
| |U38               |    |   X1_011   |     |__________________|    |  |  |  |  |  |  |  |       _______________     |
| |__________________|    |            |                             |  |  |  |  |  |  |  |      |               |    |
|  __________________     |            |                             |  |  |  |  |  |  |  |      |   MC68B50P    |    |
| |UF0_015        W21|    |____________|                             |  |  |  |  |  |  |__|      |   T6A J8905   |  10|
| |                  |                                               |__|  |__|  |__|            |               |    |
| |   4M             |                                                                           |_______________|    |
| |U40               |                                                                                                |
| |__________________|                               _______          __    __    __                                  |
|  __________________      ____________             |       |        |  |  |  |  |  |                                 |
| |UF0_016        W22|    |            |            |       |        |  |  |  |  |  |                                 |
| |                  |    |    SETA    |            |_______|        |  |  |  |  |  |             ___  O              |
| |                  |    |   X1_012   |                             |A |  |Z |  |A |          O | |#| F            11|
| |                  |    |            |                             |  |  |  |  |  |      __  N |___| F              |
| |__________________|    |            |                             |  |  |  |  |  |     |  |  _________             |
|   ___________           |____________|                             |  |  |  |  |  |     |  | |74HC00AP |            |
|  |           |                                                     |  |  |  |  |  |     |  | |_________|            |
|  |___________|                                                     |__|  |__|  |__|     |A2|                      12|
|   ___________                                                                           |  |                        |
|  |           |                                                                 __       |  |                        |
|  |___________|          __    __    __             _______          __    __  |  |      |__|   ________________     |
|                        |  |  |  |  |  |           |       |        |  |  |  | |  |            |  TMM2063AP_12  |  13|
|  __________________    |  |  |  |  |  |           |       |        |  |  |  | |A |            |________________|    |
| |                  |   |L |  |I |  |K |           |_______|        |C |  |B | |  |           ___________________    |
| |       SETA       |   |  |  |  |  |  |                            |  |  |  | |  |          |UF1 004            |   |
| |      X1_007      |   |  |  |  |  |  |                            |  |  |  | |  |          |                   | 14|
| |      737100      |   |__|  |__|  |__|                            |__|  |__| |__|          |  TC571001D_20     |   |
| |__________________|                                                           __           |                   |   |
|                           __________                                          |  |          |___________________|   |
|                          |          |   __   __   __    __    __    __    __  |  |                         ____   15|
|                          |__________|  |  | |  | |  |  |  |  |  |  |  |  |  | |  |     __________    __   |    |    |
|                                        |  | |  | |  |  |  |  |  |  |  |  |  | |A |    |          |  |  |  |    |    |
|                           __________   |  | |C | |G |  |F |  |E |  |D |  |  | |  |    | SETA     |  |  |  |DIP1|    |
|                ___  O    |          |  |  | |  | |  |  |  |  |  |  |  |  |  | |  |    |  X1_010  |  |H |  |    |  16|
|             O |#| | F    |__________|  |  | |  | |  |  |  |  |  |  |  |  |  | |  |    |  811101  |  |  |  |    |    |
|             N |___| F                  |__| |__| |__|  |__|  |__|  |__|  |__| |__|    |__________|  |  |  |____|    |
|          __________      _______                _________    _________                              |__|   ____     |
|         |    O     |    |   M   |              |    I    |  |    I    |                                   |    |  17|
|         |__________|    |_______|              |_________|  |_________|                 __________   __   |    |    |
|   ___    __________      _______    ________                                           |74HC273AP | |  |  |DIP2|    |
|  |   |  |    O     |    |   N   |  |   I    |   _________                              |__________| |  |  |    |    |
|  |___|  |__________|    |_______|  |________|  |    J    |                              __________  |H |  |    |  18|
|                                                |_________|                             |74HC273AP | |  |  |____|    |
|                                     ________                                           |__________| |  |   ____     |
|                                    |   P    |                                                       |__|  |    |  19|
|                                    |________|                                  _____________________      |    |    |
|          ___             _______    ________    _________    __________       |      SETA X1_004    |     |DIP3|    |
|         |   |           |   Q   |  |   Q    |  |    Q    |  |    P     |      |        826100       |     |    |  20|
|         |___|           |_______|  |________|  |_________|  |__________|      |                     |     |    |    |
|                                                                               |_____________________|     |____|    |
|                                     ________    _________    __________    ___________    _________   _________     |
|                                    |        |  |         |  |    R     |  |     A     |  |    Q    | |    Q    |  21|
|                                    |________|  |_________|  |__________|  |___________|  |_________| |_________|    |
|                                                                                                                     |
|                                                         _   _                                                       |
|                                                        | | | |                                                    22|
|                                                        |_| |_|                                                      |
|                                                                                                                     |
|                                                                                                                   23|
|                        _______ |||||||||||||||||||||||||||||||||||||||||||||||||||||| __ ||||||| _____              |
|                       |       ||||||||||||||||||||||||||||||||||||||||||||||||||||||||  |||||||||     |             |
|_______________________|       |______________________________________________________|  |_______|     |_____________|


A = 74HC245AP      O = TC74HC4520P
B = MB74LS32       P = TC4069UBP
C = 74HC02AP       Q = TC4050BP
D = SN74HC138N     R = MC14504B
E = MC74HC32       S = 74HC367AP
F = TC24HC27P      T = M74LS04P
G = SN74LS260N     U = MC74HC20
H = M74LS253P      V = 74HC161AP
I = 74HC74AP       W = SN74LS146N
J = TC74HC174P     X = MC74F244N
K = TC74HC157P     Y = MC74F374N
L = MB74LS08       Z = M74LS245P
M = HD14040BP      A1= 74HC60AP
N = TC4013BP       A2= D4911C


DIP1:                     DIP2:                     DIP3:
|___________________|     |___________________|     |___________________|
| ON                |     | ON                |     | ON                |
| |_______________| |     | |_______________| |     | |_______________| |
| |_|_|_|_|_|#|#|#| |     | |_|_|#|#|_|_|_|_| |     | |_|_|#|#|_|_|#|_| |
| |#|#|#|#|#| | | | |     | |#|#| | |#|#|#|#| |     | |#|#| | |#|#| |#| |
| |_______________| |     | |_______________| |     | |_______________| |
|  1 2 3 4 5 6 7 8  |     |  1 2 3 4 5 6 7 8  |     |  1 2 3 4 5 6 7 8  |
|___________________|     |___________________|     |___________________|

***************************************************************************/

ROM_START( setaroul )
	ROM_REGION( 0x0c0000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "uf1-002.u14", 0x000000, 0x010000, CRC(b3a622b0) SHA1(bc4a02167002579149c19640e65e679b7c19fa66) )
	ROM_LOAD16_BYTE( "uf1-003.u16", 0x000001, 0x010000, CRC(a6afd769) SHA1(82c54c8a2219f20d08faf9f7afcf821d83511660) )

	ROM_REGION( 0x020000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "uf0-005.u3", 0x010001, 0x008000, CRC(383c2d57) SHA1(3bbf0464f80f657dfa275e885fbce064a0a08f4a) )
	ROM_LOAD16_BYTE( "uf0-006.u4", 0x010000, 0x008000, CRC(90c9dae6) SHA1(a226aab82f5b8174644281fa3efab4f8a8f8d827) )
	ROM_LOAD16_BYTE( "uf0-007.u5", 0x000001, 0x008000, CRC(e72c3dba) SHA1(aaebb484e76d8f3da0ecff26c3c1bad4f3f11ac0) )
	ROM_LOAD16_BYTE( "uf0-008.u6", 0x000000, 0x008000, CRC(e198e602) SHA1(f53fa36d1ea51239e71fe1ea7432bb4b7b8b3466) )

	ROM_REGION( 0x400000, "gfx2", 0 )   // Layer 1 - 8bpp
	ROM_LOAD32_BYTE( "uf0-010.u15", 0x000000, 0x080000, CRC(0af13a56) SHA1(c294b7947d004c0e0b280ca44636e4059e05a57e) )
	ROM_LOAD32_BYTE( "uf0-012.u29", 0x000001, 0x080000, CRC(cba2a6b7) SHA1(8627eda24c6980a0e786fd9dc06176893a33c58f) )
	ROM_LOAD32_BYTE( "uf0-014.u38", 0x000002, 0x080000, CRC(da2bd4e4) SHA1(244af8705f2fa4ab3f3a002af16a0e4d60e03de8) )
	ROM_LOAD32_BYTE( "uf0-015.u40", 0x000003, 0x080000, CRC(11dc19fa) SHA1(e7084f61d075a61249d924a523c32e7993d9ae46) )
	ROM_LOAD32_BYTE( "uf0-009.u13", 0x200000, 0x080000, CRC(20f2d7f5) SHA1(343a8fac76d6ee7f845f9988c491698ebd0150d4) )
	ROM_LOAD32_BYTE( "uf0-011.u22", 0x200001, 0x080000, CRC(af60adf9) SHA1(6505cbce6e066d75b779fdbe2c034ba4daabbefe) )
	ROM_LOAD32_BYTE( "uf0-013.u37", 0x200002, 0x080000, CRC(645ec3c3) SHA1(e9b8056c68bf33b0b7130a5ce2bafd11dfd6c29b) )
	ROM_LOAD32_BYTE( "uf0-016.u48", 0x200003, 0x080000, CRC(10f99fa8) SHA1(7ef9a3f71dd071483cf3513ef57e2fcfe8702994) )

	ROM_REGION( 0x100000, "x1snd", ROMREGION_ERASE00 )  // Samples
	ROM_LOAD( "uf1-004.u52",     0x040000, 0x020000, CRC(6638054d) SHA1(f5c4a4c822ee56cfcbb4e8401253ae0a2c2c1df7) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD16_BYTE( "uf0-017.u50", 0x000, 0x200, CRC(bf50c303) SHA1(31685ed4849e5c27654f02945678db425d54bf5e) )
	ROM_LOAD16_BYTE( "uf0-018.u51", 0x001, 0x200, CRC(1c584d5f) SHA1(f1c7e3da8b108d78b459cae53fabb6e28d3a7ee8) )
ROM_END

// Super Ruleta 36 (hack of The Roulette)
// CODERE Argentina. Massive production.

ROM_START( setaroula )
	ROM_REGION( 0x0c0000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "2_sp_old.bin", 0x000000, 0x010000, CRC(5561caae) SHA1(facab731a8c5fce39be72c9988b8e10ba62a0a37) )
	ROM_LOAD16_BYTE( "3_sp_old.bin", 0x000001, 0x010000, CRC(554aa3e2) SHA1(7d8ef54fc3349be079c60dcb244dd3c2336a6bd1) )

	ROM_REGION( 0x020000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "005_sp_old.bin", 0x010001, 0x008000, CRC(5a289858) SHA1(3f698e5f09fb124c590cf4481b370c2985e92852) )
	ROM_LOAD16_BYTE( "006_sp_old.bin", 0x010000, 0x008000, CRC(07b275ee) SHA1(77064e8f9ff85e8930eac43fc158ccf0a165f81e) )
	ROM_LOAD16_BYTE( "007_sp_old.bin", 0x000001, 0x008000, CRC(deebf76e) SHA1(f9b794e08eabb76f38f991ebe653863e4e23dd83) )
	ROM_LOAD16_BYTE( "008_sp_old.bin", 0x000000, 0x008000, CRC(e7363dc6) SHA1(759f271ec0b0079d686f2bd7b4379b762a7c7331) )

	ROM_REGION( 0x400000, "gfx2", 0 )   // Layer 1 - 8bpp
	ROM_LOAD32_BYTE( "uf0-010.u15", 0x000000, 0x080000, CRC(0af13a56) SHA1(c294b7947d004c0e0b280ca44636e4059e05a57e) )
	ROM_LOAD32_BYTE( "uf0-012.u29", 0x000001, 0x080000, CRC(cba2a6b7) SHA1(8627eda24c6980a0e786fd9dc06176893a33c58f) )
	ROM_LOAD32_BYTE( "uf0-014.u38", 0x000002, 0x080000, CRC(da2bd4e4) SHA1(244af8705f2fa4ab3f3a002af16a0e4d60e03de8) )
	ROM_LOAD32_BYTE( "uf0-015.u40", 0x000003, 0x080000, CRC(11dc19fa) SHA1(e7084f61d075a61249d924a523c32e7993d9ae46) )
	ROM_LOAD32_BYTE( "uf0-009.u13", 0x200000, 0x080000, CRC(20f2d7f5) SHA1(343a8fac76d6ee7f845f9988c491698ebd0150d4) )
	ROM_LOAD32_BYTE( "uf0-011.u22", 0x200001, 0x080000, CRC(af60adf9) SHA1(6505cbce6e066d75b779fdbe2c034ba4daabbefe) )
	ROM_LOAD32_BYTE( "uf0-013.u37", 0x200002, 0x080000, CRC(645ec3c3) SHA1(e9b8056c68bf33b0b7130a5ce2bafd11dfd6c29b) )
	ROM_LOAD32_BYTE( "uf0-016.u48", 0x200003, 0x080000, CRC(10f99fa8) SHA1(7ef9a3f71dd071483cf3513ef57e2fcfe8702994) )

	ROM_REGION( 0x100000, "x1snd", ROMREGION_ERASE00 )  // Samples
	ROM_LOAD( "uf1-004.u52",     0x040000, 0x020000, CRC(6638054d) SHA1(f5c4a4c822ee56cfcbb4e8401253ae0a2c2c1df7) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD16_BYTE( "uf0-017.u50", 0x000, 0x200, CRC(bf50c303) SHA1(31685ed4849e5c27654f02945678db425d54bf5e) )
	ROM_LOAD16_BYTE( "uf0-018.u51", 0x001, 0x200, CRC(1c584d5f) SHA1(f1c7e3da8b108d78b459cae53fabb6e28d3a7ee8) )
ROM_END

ROM_START( setaroulm )
	ROM_REGION( 0x0c0000, "maincpu", 0 )        // 68000 Code
	ROM_LOAD16_BYTE( "uf011.002.5a", 0x000000, 0x010000, CRC(285f41ba) SHA1(b5ff09cae1e178526145f113cc3c85892e35ec34) )
	ROM_LOAD16_BYTE( "uf011.003.7a", 0x000001, 0x010000, CRC(2ab925b0) SHA1(f02de8a6643330c833027dd99006ac2d5d07e2f0) )

	ROM_REGION( 0x020000, "gfx1", 0 )   // Sprites
	ROM_LOAD16_BYTE( "uf1.005.1j", 0x010001, 0x008000, CRC(12ee9729) SHA1(29f621811d52413ae37137035ad687fabfe9e56e) )
	ROM_LOAD16_BYTE( "uf1.006.1l", 0x010000, 0x008000, CRC(5eb35519) SHA1(1af240ae725102f310a101829539d1ca5323e96c) )
	ROM_LOAD16_BYTE( "uf1.007.1n", 0x000001, 0x008000, CRC(b287ddcf) SHA1(70d291fcb6a60be2c45e6ad61f1c3922d45ef7e0) )
	ROM_LOAD16_BYTE( "uf1.008.1r", 0x000000, 0x008000, CRC(6de9a30b) SHA1(308468079b535d1b0ca437251c3135d7f0c91dce) )

	ROM_REGION( 0x400000, "gfx2", 0 )   // Layer 1 - 8bpp, not dumped for this set, but MASK ROM codes match
	ROM_LOAD32_BYTE( "uf0-010.u15", 0x000000, 0x080000, CRC(0af13a56) SHA1(c294b7947d004c0e0b280ca44636e4059e05a57e) )
	ROM_LOAD32_BYTE( "uf0-012.u29", 0x000001, 0x080000, CRC(cba2a6b7) SHA1(8627eda24c6980a0e786fd9dc06176893a33c58f) )
	ROM_LOAD32_BYTE( "uf0-014.u38", 0x000002, 0x080000, CRC(da2bd4e4) SHA1(244af8705f2fa4ab3f3a002af16a0e4d60e03de8) )
	ROM_LOAD32_BYTE( "uf0-015.u40", 0x000003, 0x080000, CRC(11dc19fa) SHA1(e7084f61d075a61249d924a523c32e7993d9ae46) )
	ROM_LOAD32_BYTE( "uf0-009.u13", 0x200000, 0x080000, CRC(20f2d7f5) SHA1(343a8fac76d6ee7f845f9988c491698ebd0150d4) )
	ROM_LOAD32_BYTE( "uf0-011.u22", 0x200001, 0x080000, CRC(af60adf9) SHA1(6505cbce6e066d75b779fdbe2c034ba4daabbefe) )
	ROM_LOAD32_BYTE( "uf0-013.u37", 0x200002, 0x080000, CRC(645ec3c3) SHA1(e9b8056c68bf33b0b7130a5ce2bafd11dfd6c29b) )
	ROM_LOAD32_BYTE( "uf0-016.u48", 0x200003, 0x080000, CRC(10f99fa8) SHA1(7ef9a3f71dd071483cf3513ef57e2fcfe8702994) )

	ROM_REGION( 0x100000, "x1snd", ROMREGION_ERASE00 )  // Samples
	ROM_LOAD( "uf1-004.14a", 0x040000, 0x020000, CRC(d63ea334) SHA1(93aaf58c90c4f704caae19b63785e471b2c1281a) ) // 1xxxxxxxxxxxxxxxx = 0xFF, possibly bad

	ROM_REGION( 0x400, "proms", 0 ) // not dumped for this set, but stickers match
	ROM_LOAD16_BYTE( "uf0-017.u50", 0x000, 0x200, CRC(bf50c303) SHA1(31685ed4849e5c27654f02945678db425d54bf5e) )
	ROM_LOAD16_BYTE( "uf0-018.u51", 0x001, 0x200, CRC(1c584d5f) SHA1(f1c7e3da8b108d78b459cae53fabb6e28d3a7ee8) )
ROM_END


void seta_state::init_bankx1()
{
	m_x1_bank->configure_entries(0, 8, memregion("x1snd")->base(), 0x40000);
}

void seta_state::init_madsharkbl()
{
	m_oki_bank->configure_entries(0, 4, memregion("oki")->base(), 0x20000);
}

void zombraid_state::init_zombraid()
{
	// bank 1 is never explicitly selected, 0 is used in its place
	m_x1_bank->configure_entry(0, memregion("x1snd")->base() + 0x80000);
	m_x1_bank->configure_entries(1, 7, memregion("x1snd")->base() + 0x80000, 0x80000);
}

void seta_state::init_wiggie()
{
	u8 temp[16];
	u8 *src = memregion("maincpu")->base();
	int len = memregion("maincpu")->bytes();
	for (int i = 0; i < len; i += 16)
	{
		std::copy(&src[i], &src[i+16], std::begin(temp));
		for (int j = 0; j < 16; j++)
		{
			static const int convtable[16] =
			{
				0x0, 0x1, 0x8, 0x9,
				0x2, 0x3, 0xa, 0xb,
				0x4, 0x5, 0xc, 0xd,
				0x6, 0x7, 0xe, 0xf
			};

			src[i+j] = temp[convtable[j]];
		}

	}
}

void jockeyc_state::init_inttoote()
{
	// code patches due to unemulated protection (to be removed...)
	u16 *ROM = (u16 *)memregion( "maincpu" )->base();

	ROM[0x4de0/2] = 0x4e71; // hardware test errors
	ROM[0x4de2/2] = 0x4e71;

	ROM[0x368a/2] = 0x50f9; // betting count down
}

} // anonymous namespace


/***************************************************************************

                                Game Drivers

***************************************************************************/

GAME( 1989?, setaroul,  0,        setaroul,  setaroul,  setaroul_state, empty_init,    ROT270, "Visco",                     "The Roulette (Visco)", 0 )
GAME( 1989?, setaroula, setaroul, setaroul,  setaroul,  setaroul_state, empty_init,    ROT270, "hack (CODERE)",             "Super Ruleta 36 (Spanish hack of The Roulette)", 0 )
GAME( 1989?, setaroulm, setaroul, setaroulm, setaroulm, setaroul_state, empty_init,    ROT270, "Visco",                     "The Roulette (Visco, medal)", MACHINE_NOT_WORKING ) // check if game plays correctly, I/O..

GAME( 1989, drgnunit,  0,        drgnunit,  drgnunit,  seta_state,     empty_init,     ROT0,   "Athena / Seta",             "Dragon Unit / Castle of Dragon", 0 ) // Country/License: DSW

GAME( 1989, wits,      0,        wits,      wits,      seta_state,     empty_init,     ROT0,   "Athena (Visco license)",    "Wit's (Japan)" , 0) // Country/License: DSW

GAME( 1990, thunderl,   0,       thunderl,  thunderl,  thunderl_state, empty_init,     ROT270, "Seta",                      "Thunder & Lightning (set 1)" , 0) // Country/License: DSW
GAME( 1990, thunderla,  thunderl,thunderl,  thunderl,  thunderl_state, empty_init,     ROT270, "Seta",                      "Thunder & Lightning (set 2)" , 0) // Country/License: DSW
GAME( 1991, thunderlbl, thunderl,thunderlbl,thunderlbl,thunderl_state, empty_init,     ROT270, "bootleg (Hyogo)",           "Thunder & Lightning (bootleg with Tetris sound, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_NO_COCKTAIL ) // Country/License: DSW
GAME( 1990, thunderlbl2,thunderl,thunderlbl,thunderl,  thunderl_state, empty_init,     ROT270, "bootleg",                   "Thunder & Lightning (bootleg with Tetris sound, set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_NO_COCKTAIL ) // Country/License: DSW

GAME( 1994, wiggie,    0,        wiggie,    thunderl,  seta_state,     init_wiggie,    ROT270, "Promat",                    "Wiggie Waggie", MACHINE_IMPERFECT_GRAPHICS ) // hack of Thunder & Lightning
GAME( 1994, superbar,  wiggie,   superbar,  thunderl,  seta_state,     init_wiggie,    ROT270, "Promat",                    "Super Bar", MACHINE_IMPERFECT_GRAPHICS ) // hack of Thunder & Lightning

GAME( 1990, jockeyc,   0,        jockeyc,   jockeyc,   jockeyc_state,  empty_init,     ROT0,   "Seta (Visco license)",      "Jockey Club (v1.18)", 0 )
GAME( 1993, inttoote2, jockeyc,  jockeyc,   jockeyc,   jockeyc_state,  empty_init,     ROT0,   "bootleg (Coinmaster)",      "International Toote II (v1.24, P387.V01)", 0 )
GAME( 1998, inttoote,  jockeyc,  inttoote,  inttoote,  jockeyc_state,  init_inttoote,  ROT0,   "bootleg (Coinmaster)",      "International Toote (Germany, P523.V01)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
GAME( 1990, gderby,    jockeyc,  jockeyc,   jockeyc,   jockeyc_state,  empty_init,     ROT0,   "hack (CODERE)",             "Gran Derby (Spanish hack of Jockey Club)", 0 )

GAME( 1992, rezon,     0,        rezon,     rezon,     seta_state,     empty_init,     ROT0,   "Allumer",                   "Rezon", 0 ) // License: Jumper
GAME( 1991, rezono,    rezon,    rezon,     rezon,     seta_state,     empty_init,     ROT0,   "Allumer",                   "Rezon (earlier)", 0 ) // ""

GAME( 1991, stg,       0,        stg,       stg,       seta_state,     empty_init,     ROT270, "Athena / Tecmo",            "Strike Gunner S.T.G", 0 )

GAME( 1991, pairlove,  0,        pairlove,  pairlove,  pairlove_state, empty_init,     ROT270, "Athena / Nihon System",     "Pairs Love", 0 ) // Non-explicit Nihon System credit on title screen thru logo

GAME( 1992, blandia,   0,        blandia,   blandia,   seta_state,     init_bankx1,    ROT0,   "Allumer",                   "Blandia", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, blandiap,  blandia,  blandiap,  blandia,   seta_state,     init_bankx1,    ROT0,   "Allumer",                   "Blandia (prototype)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1992, blockcar,  0,        blockcar,  blockcar,  seta_state,     empty_init,     ROT90,  "Visco",                     "Block Carnival / Thunder & Lightning 2" , 0) // Title: DSW
GAME( 1992, blockcarb, blockcar, blockcarb, blockcar,  seta_state,     empty_init,     ROT90,  "bootleg",                   "Block Carnival / Thunder & Lightning 2 (bootleg)", MACHINE_IMPERFECT_SOUND)

GAME( 1992, qzkklogy,  0,        qzkklogy,  qzkklogy,  seta_state,     empty_init,     ROT0,   "Tecmo",                     "Quiz Kokology", 0 )

GAME( 1992, neobattl,  0,        umanclub,  neobattl,  seta_state,     empty_init,     ROT270, "Banpresto",                 "SD Gundam Neo Battling (Japan)", 0 )

GAME( 1992, umanclub,  0,        umanclub,  umanclub,  seta_state,     empty_init,     ROT0,   "Banpresto",                 "Ultraman Club - Tatakae! Ultraman Kyoudai!!", 0 )

GAME( 1992, zingzip,   0,        zingzip,   zingzip,   seta_state,     empty_init,     ROT270, "Allumer / Tecmo",           "Zing Zing Zip (World) / Zhen Zhen Ji Pao (China?)", 0 ) // This set has Chinese Characters in Title screen, it distributed for Chinese market/or Title: DSW?
GAME( 1992, zingzipbl, zingzip,  zingzipbl, zingzip,   seta_state,     empty_init,     ROT270, "bootleg",                   "Zing Zing Zip (bootleg)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // different video registers, bad Oki ROM dump

GAME( 1993, atehate,   0,        atehate,   atehate,   seta_state,     empty_init,     ROT0,   "Athena",                    "Athena no Hatena?", 0 )

GAME( 1993, daioh,     0,        daioh,     daioh,     seta_state,     empty_init,     ROT270, "Athena",                    "Daioh", 0 )
GAME( 1993, daioha,    daioh,    daioh,     daioh,     seta_state,     empty_init,     ROT270, "Athena",                    "Daioh (earlier)", 0 )
GAME( 1993, daiohp,    daioh,    daiohp,    daiohp,    seta_state,     empty_init,     ROT270, "Athena",                    "Daioh (prototype)", 0 )
GAME( 1993, daiohp2,   daioh,    daiohp,    daiohp2,   seta_state,     empty_init,     ROT270, "Athena",                    "Daioh (prototype, earlier)", 0 )
GAME( 1993, daiohp3,   daioh,    daiohp,    daiohp2,   seta_state,     empty_init,     ROT270, "Athena",                    "Daioh (prototype, earliest)", MACHINE_NOT_WORKING ) // believed earlier as it doesn't have the intro, needs correct program ROMs
GAME( 1993, daiohc,    daioh,    wrofaero,  daioh,     seta_state,     empty_init,     ROT270, "Athena",                    "Daioh (93111A PCB conversion)", 0 )

GAME( 1993, jjsquawk,  0,        jjsquawk,  jjsquawk,  seta_state,     empty_init,     ROT0,   "Athena / Able",             "J. J. Squawkers", MACHINE_IMPERFECT_SOUND )
GAME( 1993, jjsquawko, jjsquawk, jjsquawk,  jjsquawk,  seta_state,     empty_init,     ROT0,   "Athena / Able",             "J. J. Squawkers (older)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, jjsquawkb, jjsquawk, jjsquawb,  jjsquawk,  seta_state,     empty_init,     ROT0,   "bootleg",                   "J. J. Squawkers (bootleg)", MACHINE_IMPERFECT_SOUND )
GAME( 1999, jjsquawkb2,jjsquawk, jjsquawk,  jjsquawk,  seta_state,     empty_init,     ROT0,   "bootleg",                   "J. J. Squawkers (bootleg, Blandia Conversion)", MACHINE_IMPERFECT_SOUND )
GAME( 2003, simpsonjr, jjsquawk, jjsquawb,  jjsquawk,  seta_state,     empty_init,     ROT0,   "bootleg (Daigom Games)",    "Simpson Junior (bootleg of J. J. Squawkers)", MACHINE_IMPERFECT_SOUND )

GAME( 1993, kamenrid,  0,        kamenrid,  kamenrid,  seta_state,     empty_init,     ROT0,   "Banpresto",                 "Masked Riders Club Battle Race / Kamen Rider Club Battle Racer", 0 )

GAME( 1993, madshark,  0,        madshark,  madshark,  seta_state,     empty_init,     ROT270, "Allumer",                   "Mad Shark", 0 )
GAME( 1993, madsharkbl,madshark, madsharkbl,madshark,  seta_state,     init_madsharkbl,ROT270, "bootleg",                   "Mad Shark (bootleg)", MACHINE_IMPERFECT_SOUND ) // no BGM. Wrong Oki banking?

// end credits shows Allumer as developer.
GAME( 1993, msgundam,  0,        msgundam,  msgundam,  seta_state,     empty_init,     ROT0,   "Banpresto / Allumer",       "Mobile Suit Gundam", 0 )
GAME( 1993, msgundam1, msgundam, msgundam,  msgundam1, seta_state,     empty_init,     ROT0,   "Banpresto / Allumer",       "Mobile Suit Gundam (Japan)", 0 )
GAME( 1993, msgundamb, msgundam, msgundamb, msgundam,  seta_state,     empty_init,     ROT0,   "bootleg",                   "Mobile Suit Gundam (bootleg)", 0 )

GAME( 1993, oisipuzl,  0,        oisipuzl,  oisipuzl,  seta_state,     empty_init,     ROT0,   "Sunsoft / Atlus",           "Oishii Puzzle Ha Irimasenka", 0 )
GAME( 1993, triplfun,  oisipuzl, triplfun,  oisipuzl,  seta_state,     empty_init,     ROT0,   "bootleg",                   "Triple Fun", 0 )
GAME( 1993, triplfunk, oisipuzl, triplfun,  oisipuzl,  seta_state,     empty_init,     ROT0,   "bootleg (Jin Young)",       "Sum-eoitneun Deongdalireul Chat-ara!", 0 ) // 숨어있는 덩달이를 찾아라! (Find the hiding Deongdari!)

GAME( 1993, qzkklgy2,  0,        qzkklgy2,  qzkklgy2,  seta_state,     empty_init,     ROT0,   "Tecmo",                     "Quiz Kokology 2", 0 )

GAME( 1993, utoukond,  0,        utoukond,  utoukond,  seta_state,     empty_init,     ROT0,   "Banpresto",                 "Ultra Toukon Densetsu (Japan)", 0 )

GAME( 1993, wrofaero,  0,        wrofaero,  wrofaero,  seta_state,     empty_init,     ROT270, "Yang Cheng",                "War of Aero - Project MEIOU", 0 )

GAME( 1994, eightfrc,  0,        eightfrc,  eightfrc,  seta_state,     init_bankx1,    ROT90,  "Tecmo",                     "Eight Forces", 0 )

GAME( 1994, krzybowl,  0,        krzybowl,  krzybowl,  seta_state,     empty_init,     ROT270, "American Sammy",            "Krazy Bowl", 0 )

GAME( 1994, magspeed,  0,        magspeed,  magspeed,  magspeed_state, empty_init,     ROT0,   "Allumer",                   "Magical Speed", 0 )

GAME( 1994, orbs,      0,        orbs,      orbs,      seta_state,     empty_init,     ROT0,   "American Sammy",            "Orbs (10/7/94 prototype?)", 0 )

GAME( 1995, keroppi,   0,        keroppi,   keroppi,   keroppi_state,  empty_init,     ROT0,   "American Sammy",            "Kero Kero Keroppi's Let's Play Together (USA, Version 2.0)", 0 ) // ROM labels are all v1.0 tho.
GAME( 1993, keroppij,  keroppi,  keroppi,   keroppij,  keroppi_state,  empty_init,     ROT0,   "Sammy Industries",          "Kero Kero Keroppi no Isshoni Asobou (Japan)", 0 )

GAME( 1995, extdwnhl,  0,        extdwnhl,  extdwnhl,  seta_state,     empty_init,     ROT0,   "Sammy Industries Japan",    "Extreme Downhill (v1.5)", MACHINE_IMPERFECT_GRAPHICS )

GAME( 1995, gundhara,  0,        gundhara,  gundhara,  seta_state,     empty_init,     ROT270, "Banpresto",                 "Gundhara", 0 )
GAME( 1995, gundharac, gundhara, gundhara,  gundhara,  seta_state,     empty_init,     ROT270, "Banpresto",                 "Gundhara (Chinese, bootleg?)", 0 )

GAME( 1995, sokonuke,  0,        extdwnhl,  sokonuke,  seta_state,     empty_init,     ROT0,   "Sammy Industries",          "Sokonuke Taisen Game (Japan)", MACHINE_IMPERFECT_SOUND )

GAME( 1995, zombraid,  0,        zombraid,  zombraid,  zombraid_state, init_zombraid,  ROT0,   "American Sammy",            "Zombie Raid (9/28/95, US)", MACHINE_NO_COCKTAIL )
GAME( 1995, zombraidp, zombraid, zombraid,  zombraid,  zombraid_state, init_zombraid,  ROT0,   "American Sammy",            "Zombie Raid (9/28/95, US, prototype PCB)", MACHINE_NO_COCKTAIL ) // actual code is same as the released version
GAME( 1995, zombraidpj,zombraid, zombraid,  zombraid,  zombraid_state, init_zombraid,  ROT0,   "Sammy Industries",          "Zombie Raid (9/28/95, Japan, prototype PCB)", MACHINE_NO_COCKTAIL ) // just 3 bytes different from above

GAME( 1996, crazyfgt,  0,        crazyfgt,  crazyfgt,  crazyfgt_state, empty_init,     ROT0,   "Subsino",                   "Crazy Fight", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
