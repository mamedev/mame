// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Bosconian  (c) 1981 Namco
Galaga     (c) 1981 Namco
Xevious    (c) 1982 Namco
Dig Dug    (c) 1982 Namco

driver by Nicola Salmoria
based on previous work by Martin Scragg, Mirko Buffoni, Aaron Giles


All these games are based on the same 3xZ80, shared memory, CPU design.
Bosconian and Galaga use the same CPU board, with minor differences
(Galaga has one missing RAM and no 50XX custom)
Xevious is physically different, but logically identical.
Dig Dug is the only one a bit different, because it reads the dip switches
through a custom chip instead of having them mapped in memory.

The video board, on the other hand, is completely different for all the games,
that's why they use separate video/ source files.


Custom ICs:
----------
Bosconian:
---------
CPU board:
06XX     interface to custom 5xXX
07XX     clock divider
08XX(x3) bus controller
50XX     player score control (protection)
51XX     I/O
54XX     explosion sound generator

Video board:
03XX(x2) ?
05XX     starfield generator
06XX     interface to custom 5xXX
07XX     clock divider
50XX     player score control (only used as protection check)
52XX     sample player

Galaga:
------
CPU board:
06XX     interface to custom 5xXX
07XX     clock divider
08XX(x3) bus controller
51XX     I/O
54XX     explosion sound generator

Video board:
00XX     tilemap address generator with scrolling capability (only Super Pacman)
02XX     gfx data shifter and mixer (16-bit in, 4-bit out)
04XX     sprite address generator
05XX     starfield generator
07XX     clock divider

Xevious:
-------
CPU board:
06XX     interface to custom 5xXX
07XX     clock divider
08XX(x3) bus controller
50XX     player score control (only used for a protection check on startup)
51XX     I/O
54XX     explosion sound generator

Video board:
03XX(x2) ?
04XX     sprite address generator
07XX     clock divider
11XX(x2) gfx data shifter and mixer (16-bit in, 4-bit out)
12XX     sprite generator
13XX     dual scrolling tilemap address generator

Dig Dug:
-------
CPU board:
06XX     interface to custom 5xXX
07XX     clock divider
08XX(x3) bus controller
51XX     I/O
53XX     I/O

Video board:
00XX     tilemap address generator
02XX     gfx data shifter and mixer (16-bit in, 4-bit out)
04XX     sprite address generator
07XX     clock divider


Memory maps:
-----------
Bosconian:
---------
MAIN CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 3N    program ROM
0001xxxxxxxxxxxx R   xxxxxxxx ROM 3M    program ROM
0010xxxxxxxxxxxx R   xxxxxxxx ROM 3L    program ROM
0011xxxxxxxxxxxx R   xxxxxxxx ROM 3K    program ROM
the rest of the memory map is common to the other CPUs

SUB CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 3J    program ROM
0001xxxxxxxxxxxx R   xxxxxxxx ROM 3H    program ROM
0010------------              n.c.
0011------------              n.c.
the rest of the memory map is common to the other CPUs

SOUND CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 3E    program ROM
0001------------              n.c.
0010------------              n.c.
0011------------              n.c.
the rest of the memory map is common to the other CPUs

COMMON:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
01000-----------              n.c.
01001-----------              n.c.
01010-----------              n.c.
01011-----------              n.c.
01100-----------              n.c.
01101-----00xxxx   W ----xxxx RAM 2A    \ sound control registers
01101-----01xxxx   W ----xxxx RAM 2B    /
01101-----10-000   W -------x IRQ1      main CPU irq enable/acknowledge
01101-----10-001   W -------x IRQ2      motion CPU irq enable/acknowledge
01101-----10-010   W -------x NMION     sound CPU nmi enable
01101-----10-011   W -------x RESET     reset sub and sound CPU, and 5xXX chips on CPU board
01101-----10-100   W -------x n.c.
01101-----10-101   W -------x MOD 0     unused?
01101-----10-110   W -------x MOD 1     unused?
01101-----10-111   W -------x MOD 2     unused?
01101-----11----   W -------- WDR       watchdog reset
01101-----00-xxx R   -------x DIP SW    dip switch B
01101-----00-xxx R   ------x- DIP SW    dip switch A
01101-----01---- R            n.c.
01101-----10---- R            n.c.
01101-----11---- R            n.c.
01110--0-------- R/W xxxxxxxx I/O       custom 06XX data
01110--1-------- R/W xxxxxxxx I/O       custom 06XX control
01111xxxxxxxxxxx R/W xxxxxxxx RAM 2N    work RAM (not present in Galaga)
10000xxxxxxxxxxx R/W xxxxxxxx DHRAM     tilemap RAM (tile code) [1]
10001xxxxxxxxxxx R/W xxxxxxxx VCRAM     tilemap RAM (tile attr) [1]
10010--0-------- R/W xxxxxxxx EXCS      custom 06XX #2 data
10010--1-------- R/W xxxxxxxx EXCS      custom 06XX #2 control
10011----000xxxx   W ----xxxx SOWR      bullets shape and X pos msb [2]
10011----001----   W xxxxxxxx POSI X    playfield X scroll
10011----010----   W xxxxxxxx POSI Y    playfield Y scroll
10011----011----   W -----xxx STAR      to 05XX: starfield X scroll speed
10011----011----   W --xxx--- STAR      to 05XX: starfield Y scroll speed
10011----100----   W -------- STARCLR   to 05XX: unknown
10011----101----   W          n.c.
10011----110----   W          n.c.
10011----111-000   W -------x FLIP      flip screen
10011----111-001   W -------x n.c.
10011----111-010   W -------x n.c.
10011----111-011   W -------x n.c.
10011----111-100   W -------x BLK 0     \ to 05XX: starfield blink
10011----111-101   W -------x BLK 1     /          (select active subset)
10011----111-110   W -------x n.c.
10011----111-111   W -------x RESET     reset 5xXX chips on video board
10100-----------              n.c.
10101-----------              n.c.
10110-----------              n.c.
10111-----------              n.c.

[1] 1st half is radar + sprite registers, 2nd half is scrolling playfield
[2] SO = Small Objects? Only locations 4-F are used.


Galaga:
------
MAIN CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 3N    program ROM
0001xxxxxxxxxxxx R   xxxxxxxx ROM 3M    program ROM
0010xxxxxxxxxxxx R   xxxxxxxx ROM 3L    program ROM
0011xxxxxxxxxxxx R   xxxxxxxx ROM 3K    program ROM
the rest of the memory map is common to the other CPUs

SUB CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 3J    program ROM
0001------------              n.c.
0010------------              n.c.
0011------------              n.c.
the rest of the memory map is common to the other CPUs

SOUND CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 3E    program ROM
0001------------              n.c.
0010------------              n.c.
0011------------              n.c.
the rest of the memory map is common to the other CPUs

COMMON:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
01000-----------              n.c.
01001-----------              n.c.
01010-----------              n.c.
01011-----------              n.c.
01100-----------              n.c.
01101-----00----   W ----xxxx RAM 2A    \ sound control registers
01101-----01----   W ----xxxx RAM 2B    /
01101-----10-000   W -------x IRQ1      main CPU irq enable/acknowledge
01101-----10-001   W -------x IRQ2      motion CPU irq enable/acknowledge
01101-----10-010   W -------x NMION     sound CPU nmi enable
01101-----10-011   W -------x RESET     reset sub and sound CPU, and 5xXX chips on CPU board
01101-----10-100   W -------x n.c.
01101-----10-101   W -------x MOD 0     unused?
01101-----10-110   W -------x MOD 1     unused?
01101-----10-111   W -------x MOD 2     unused?
01101-----11----   W -------- WDR       watchdog reset
01101-----00-xxx R   -------x DIP SW    dip switch B
01101-----00-xxx R   ------x- DIP SW    dip switch A
01101-----01---- R            n.c.
01101-----10---- R            n.c.
01101-----11---- R            n.c.
01110--0-------- R/W xxxxxxxx I/O       custom 06XX data
01110--1-------- R/W xxxxxxxx I/O       custom 06XX control
10000xxxxxxxxxxx R/W xxxxxxxx RAM 1K    tilemap RAM
10001-xxxxxxxxxx R/W xxxxxxxx RAM 3E/3F work RAM
10001-111xxxxxxx R/W xxxxxxxx           portion holding sprite registers
10010-xxxxxxxxxx R/W xxxxxxxx RAM 3K/3L work RAM
10010-111xxxxxxx R/W xxxxxxxx           portion holding sprite registers
10011-xxxxxxxxxx R/W xxxxxxxx RAM 3H/3J work RAM
10011-111xxxxxxx R/W xxxxxxxx           portion holding sprite registers
10100--------000   W -------x           \
10100--------001   W -------x            > to 05XX: starfield X scroll speed
10100--------010   W -------x           /
10100--------011   W -------x           \ to 05XX: starfield blink
10100--------100   W -------x           /          (select active subset)
10100--------101   W -------x           to 05XX: unknown. It is the same as STARCLR in Bosconian
10100--------110   W -------x n.c.
10100--------111   W -------x FLIP      flip screen
10101-----------              n.c.
10110-----------              n.c.
10111-----------              n.c.


Namco vs Midway ROM names and locations
---------------------------------------
Location  ID         Location  ID
--------  ----       --------  -----
CPU 3P    GG1-1      CPU 3N    3200A
CPU 3M    GG1-2      CPU 3M    3300B
CPU 2M    GG1-3      CPU 3L    3400C
CPU 2L    GG1-4      CPU 3K    3500D
CPU 3F    GG1-5      CPU 3J    3600E
CPU 2C    GG1-7      CPU 3E    3700G
CPU 1D    GG1-1[bpr] CPU 1D
CPU 5C    GG1-2[bpr] CPU 5C

VID 4L    GG1-9      VID 4L    2600J
VID 4F    GG1-10     VID 4F    2700K
VID 4D    GG1-11     VID 4D    2800L
VID 1C    GG1-3[bpr] VID 1C
VID 2N    GG1-4[bpr] VID 2N
VID 5N    GG1-5[bpr] VID 5N


Xevious:
-------
MAIN CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
000xxxxxxxxxxxxx R   xxxxxxxx ROM 1     program ROM
001xxxxxxxxxxxxx R   xxxxxxxx ROM 2     program ROM
the rest of the memory map is common to the other CPUs

MOTION CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
000xxxxxxxxxxxxx R   xxxxxxxx ROM 3     program ROM
the rest of the memory map is common to the other CPUs

SOUND CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
00-xxxxxxxxxxxxx R   xxxxxxxx ROM 4     program ROM
the rest of the memory map is common to the other CPUs

COMMON:
a small part of the decoding for the video board is done by a PAL so it is inferred by program behaviour

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
01000-----------              n.c.
01001-----------              n.c.
01010-----------              n.c.
01011-----------              n.c.
01100-----------              n.c.
01101-----00----   W ----xxxx SRAM 0    \ sound control registers
01101-----01----   W ----xxxx SRAM 1    /
01101-----10-000   W -------x IRQ1      main CPU irq enable/acknowledge
01101-----10-001   W -------x IRQ2      motion CPU irq enable/acknowledge
01101-----10-010   W -------x NMION     sound CPU nmi enable
01101-----10-011   W -------x RESET     reset sub and sound CPU, and 5xXX chips on CPU board
01101-----10-100   W -------x n.c.
01101-----10-101   W -------x n.c.
01101-----10-110   W -------x n.c.
01101-----10-111   W -------x n.c.
01101-----11----   W -------- WDR       watchdog reset
01101-----00-xxx R   -------x DIP SW    dip switch B
01101-----00-xxx R   ------x- DIP SW    dip switch A
01101-----01---- R            n.c.
01101-----10---- R            n.c.
01101-----11---- R            n.c.
01110--0-------- R/W xxxxxxxx I/O       custom 06XX data
01110--1-------- R/W xxxxxxxx I/O       custom 06XX control
01111xxxxxxxxxxx R/W xxxxxxxx           work RAM
1000-xxxxxxxxxxx R/W xxxxxxxx           work RAM
1000-1111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (x, y)
1001-xxxxxxxxxxx R/W xxxxxxxx           work RAM
1001-1111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (flip, size)
1010-xxxxxxxxxxx R/W xxxxxxxx           work RAM
1010-1111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (sprite number & color)
10110xxxxxxxxxxx R/W xxxxxxxx PF0       fg tilemap RAM (tile attributes)
10111xxxxxxxxxxx R/W xxxxxxxx PF1       bg tilemap RAM (tile attributes)
11000xxxxxxxxxxx R/W xxxxxxxx PF2       fg tilemap RAM (tile code)
11001xxxxxxxxxxx R/W xxxxxxxx PF3       bg tilemap RAM (tile code)
1101-----000---x   W xxxxxxxx           bg X scroll (9-bit data: A0 is the msb)
1101-----001---x   W xxxxxxxx           fg X scroll (9-bit data: A0 is the msb)
1101-----010---x   W xxxxxxxx           bg Y scroll (9-bit data: A0 is the msb)
1101-----011---x   W xxxxxxxx           fg Y scroll (9-bit data: A0 is the msb)
1101-----111----   W -------x FLIP      flip screen
1110------------              n.c.
1111-----------0   W xxxxxxxx BS0       \ address to read from background data ROMs
1111-----------1   W xxxxxxxx BS1       / (see xevious_bb_r)
1111-----------0 R   xxxxxxxx BB0       \ read from background data ROMs
1111-----------1 R   xxxxxxxx BB1       /


Namco vs Atari ROM names and locations
--------------------------------------
Location  ID          Location  ID
--------  ----        --------  ----------
CPU 3P    XVI-1       CPU 1M    136018-118
CPU 3M    XVI-2        "   "      "     "
CPU 2M    XVI-3       CPU 1L    136018-119
CPU 2L    XVI-4        "   "      "     "
CPU 3F    XVI-5       CPU 4C    136018-120
CPU 3J    XVI-6        "   "      "     "
CPU 2C    XVI-7       CPU 2C    136018-127
CPU 5N    XVI-1[bpr]  CPU 6M    136018-028
CPU 7N    XVI-2[bpr]  CPU 8M    136018-029

VID 2A    XVI-9       VID 2A    136018-101
VID 2B    XVI-10      VID 2B    136018-102
VID 2C    XVI-11      VID 2C    136018-103
VID 3B    XVI-12      VID 3B    136018-104
VID 3C    XVI-13      VID 3C    136018-105
VID 3D    XVI-14      VID 3D    136018-106
VID 4M    XVI-15      VID 4M    136018-107
VID 4N    XVI-16      VID 4N    136018-108
VID 4P    XVI-17      VID 4P    136018-109
VID 4R    XVI-18      VID 4R    136018-110
VID 3L    XVI-4[bpr]  VID 3L    136018-011
VID 3M    XVI-5[bpr]  VID 3M    136018-012
VID 4F    XVI-6[bpr]  VID 4F    136018-013
VID 4H    XVI-7[bpr]  VID 4H    136018-014
VID 6A    XVI-8[bpr]  VID 6A    136018-015
VID 6D    XVI-9[bpr]  VID 6D    136018-016
VID 6E    XVI-10[bpr] VID 6E    136018-017


Dig Dug:
-------
MAIN CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 0     program ROM
0001xxxxxxxxxxxx R   xxxxxxxx ROM 1     program ROM
0010xxxxxxxxxxxx R   xxxxxxxx ROM 2     program ROM
0011xxxxxxxxxxxx R   xxxxxxxx ROM 3     program ROM
the rest of the memory map is common to the other CPUs

SUB CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 4     program ROM
0001xxxxxxxxxxxx R   xxxxxxxx ROM 5     program ROM
0010------------              n.c.
0011------------              n.c.
the rest of the memory map is common to the other CPUs

SOUND CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM 6     program ROM
0001xxxxxxxxxxxx R   xxxxxxxx ROM 7     program ROM (optional, not used)
0010------------              n.c.
0011------------              n.c.
the rest of the memory map is common to the other CPUs

COMMON:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
01000-----------              n.c.
01001-----------              n.c.
01010-----------              n.c.
01011-----------              n.c.
01100-----------              n.c.
01101-----00----   W ----xxxx AUDIO 0   \ sound control registers
01101-----01----   W ----xxxx AUDIO 1   /
01101-----10-000   W -------x IRQ1      main CPU irq enable/acknowledge
01101-----10-001   W -------x IRQ2      sub CPU irq enable/acknowledge
01101-----10-010   W -------x NMION     sound CPU nmi enable
01101-----10-011   W -------x RESET     reset sub and sound CPU, and 5xXX chips on CPU board
01101-----10-100   W -------x n.c.
01101-----10-101   W -------x MOD 0     \
01101-----10-110   W -------x MOD 1     | to custom 53XX
01101-----10-111   W -------x MOD 2     /
01101-----11----   W -------- WDDIS     watchdog reset
01110--0-------- R/W xxxxxxxx I/O       custom 06XX data
01110--1-------- R/W xxxxxxxx I/O       custom 06XX control
01111-----------              n.c.
10000xxxxxxxxxxx R/W xxxxxxxx RAM 0     tilemap RAM + work RAM
10001-xxxxxxxxxx R/W xxxxxxxx OBJRAM    work RAM
10001-111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (sprite number and color)
10010-xxxxxxxxxx R/W xxxxxxxx POSRAM    work RAM
10010-111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (x and y)
10011-xxxxxxxxxx R/W xxxxxxxx FLPRAM    work RAM
10011-111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (flip)
10100--------000   W -------x           \ background ROM (114) bank select
10100--------001   W -------x           /
10100--------010   W -------x           tilemap color select (low or high 4 bits of tilemap RAM)
10100--------011   W -------x           background enable
10100--------100   W -------x           \ background color lookup PROM (112) bank select
10100--------101   W -------x           /
10100--------110   W -------x n.c.
10100--------111   W -------x FLIP      flip screen
10101-----------              n.c.
10110-----------              n.c.
10111----0xxxxxx   W xxxxxxxx EAROM     non volatile memory address latch and data write
10111----0------ R   xxxxxxxx EAROM     non volatile memory read
10111----1------   W ----xxxx EAROM     non volatile memory control



Namco vs Atari ROM names and locations
--------------------------------------
The Namco version is composed of two boards, while the Atari version is
single board. There are two revisions of the Atari version.

Location  ID        Location  Location  ID
                    (type 1)  (type 2)
--------  ----      --------  --------  ----------
CPU 3P    DD1-1     6L        2C/D      136007-101
CPU 3M    DD1-2     6M        2E        136007-102
CPU 2M    DD1-3     6N/P      2B/C      136007-103
CPU 2L    DD1-4     6R        2A        136007-104
CPU 3F    DD1-5     6C        2P        136007-105
CPU 3J    DD1-6     6D        2N        136007-106
CPU 2C    DD1-7     5L        2K/L      136007-107
CPU 5N    [bpr]     2K/L      10A       136007-109
CPU 7N    [bpr]     2P        11A       136007-110

VID 2C    DD1-9     8R        5K        136007-108
VID 1C    [bpr]     4G        8F        136007-111
VID 2N    [bpr]     10K/L     4N        136007-112
VID 5N    [bpr]     1R        8L        136007-113
VID 2D    DD1-10    9N        4J        136007-114
VID 5C    DD1-11    10C/D     4F        136007-115
VID 5F    DD1-12    7A/B      5B        136007-119
VID 5H    DD1-13    8A/B      5A        136007-118
VID 5J    DD1-14    7C        5C        136007-117
VID 5K    DD1-15    8C        5D        136007-116



Gatsbee (Galaga mod/bootleg)
----------------------------
This game runs on modified bootleg Galaga hardware (blue board with PCB numbers DG-09-02 and DG-07-02)

ROM8: is a 2764. pins 1, 26, 27, 28 tied together.
      pin2 out of socket, has wire that is tied to pin 4 of a LS259 that sits on top of the main Z80
      CPU located at 5B/6B

Z80: There are 2 logic chips sitting on top of it which are wired up to the Z80 and to each other.
     Looks like this....
     |-------------------|
     |  LS32   LS259     <
     |-------------------|

Bend all the legs outwards.
Line up the LS259 so pin 16 is in line with Z80 pin 11
Line up the LS32 so pin 7 is in line with Z80 pin 29
Atach the 2 chips to the top of the Z80 with some glue
Connect like this....

LS32 pin 1 tied to Z80 pin 22
LS32 pin 2 tied to Z80 pin 19
LS32 pin 3,4 tied together
LS32 pin 5 tied to Z80 pin 4
LS32 pin 6 tied to pin 10 LS32
LS32 pin 7 tied to Z80 pin 29 (GND)
LS32 pin 8 tied to LS259 pin 14
LS32 pin 9 tied to Z80 pin 5
LS32 pins 11, 12, 13 have NC
LS32 pin 14 tied to Z80 pin 11 (+5V)

LS259 pin 1 tied to Z80 pin 30
LS259 pin 2 tied to Z80 pin 31
LS259 pin 3 tied to Z80 pin 32
LS259 pin 4 to ROM 8 (as above)
LS259 pins 5, 6, 7 have NC
LS259 pin 8 tied to Z80 pin 29 (GND)
LS259 pins 9, 10, 11, 12 have NC
LS259 pin 13 tied to Z80 pin 14
LS259 pin 15 tied to Z80 pin 26
LS259 pin 16 tied to Z80 pin 11



Easter eggs:
-----------
- Bosconian:
  - enter service mode
  - keep B1 pressed and enter the following sequence:
    5xU 6xR 1xD 4xL
  (c) 1981 NAMCO LTD. will be added at the bottom of the screen.

- Galaga:
  - enter service mode
  - keep B1 pressed and enter the following sequence:
    5xR 6xL 3xR 7xL
  (c) 1981 NAMCO LTD. will appear on the screen.

- Xevious:
  - start a game
  - go to the bottom right of the screen and keep B2 pressed
  NAMCO ORIGINAL
  program by EVEZOO
  will be written at the bottom of the screen
  In Super Xevious this is changed to
  special thanks for you
  by game designer EVEZOO

- Dig Dug:
  - enter service mode
  - keep B1 pressed and enter the following sequence:
    6xU 3xR 4xD 8xL
  (c) 1982 NAMCO LTD. will appear on the screen.


Notes:
-----
- The Cabinet Type "dip switch" actually comes from the edge connector, but is mapped
  in memory in place of dip switch #8. dip switch #8 selects single/dual coin counters
  and is entirely handled by hardware.

- galaga: there is a bug in the sound CPU program. During initialization, it enables
  NMI before clearing RAM, but the NMI handler doesn't save the registers, so it cannot
  interrupt program execution. If the NMI happens before the LDIR that clears RAM has
  finished, the program will crash.

- galaga: there were "fast shoot" hacks available, which are not supported.
  Their effects can be replicated with this line in cheat.dat:
  galaga:1:070D:0D:100:Fast Shoot

- bosco: there appears to be a bug in the code at 0BB1, which handles communication
  with the 06XX custom chip. First it saves in A' the command to write, then if a
  transfer is still in progress it jups to 0BC1, does other things, then restores
  the command from A' and stores it in RAM. At that point (0BE1) it checks again if
  a transfer is in progress. If the trasnfer has terminated, it jumps to 0BEB, which
  restores the command from RAM, and jumps back to 0BBA to send the command. However,
  the instruction at 0BBA is ex af,af', so the command is overwritten with garbage.
  There's also an exx at 0BBB which seems unnecessary but that's harmless.
  Anyway, what this bug means is that we must make sure that the 06XX generates NMIs
  quickly enough to ensure that 0BB1 is usually not called with a transfer still is
    progress. It doesn't seem possible to prevent it altogether though, so we can only
    hope that the transfer doesn't terminate in the middle of the function.

- bosco: we have two dumps of the sound shape ROM, "prom.1d" and "bosco.spr". Music
  changes a lot from one version to the other.
  I'm using the former because it is more similar to the other Namco games. The latter,
  after masking off the unused top 4 bits and inverting bit 3, matches the Galaga one,
  so it might have come from a (bootleg?) conversion.

- bosco & galaga: the Midway arcade cabinet had an optional rapid fire board, using
  a 556 to generate autofire while the button was held. That really makes little
  sense in Galaga! For Bosconian, I guess it was for the boscomdo set I, because the
  other sets have autofire built-in.

- the bosconian video system is (apart from the starfield) almost identical functionally
  to Rally X, but the hardware is quite different: Rally X has no custom ICs.

- digdug: if you enter service mode and press press service coin something like
  the following is written at the bottom of the screen:
  99.9999.9999.9999.9999.
  This is explained in the manual: it is the number of games played, of points, etc.
  The counters start from 999 and count backwards.

- gallag is identical to galagao, apart from the title changed to "GALLAG" and the
  copyright notice changed from "(c) 1981 NAMCO LTD" to "1 9 8 2" (and the Namco logo
  removed from the gfx). The only interesting thing about it is the 4th Z80, used to
  simulate the custom 5xXX chips of the original.
  It also has different explosion and starfield circuitries, to do without the Namco
  custom chips.

- differences between versions of digdug:
  - the background graphics are slightly different in the Atari versions; the earth is
    less regular.

  - "digduga1" is identical to "digdugb", apart from the gfx and copyright notices
    changed from "NAMCO LTD." to "ATARI INC.".

  - "digdug" fixes two bugs that were present in "digdugb":  First, as monster speed
    increased in later rounds it could eventually roll over to 0, causing the monsters
    to stop moving altogether.  Second, "double-killing" a monster by bursting it and
    immediately dropping a rock on the corpse could result in the round not ending
    even after all monsters were killed.
    This set also has the code to save high scores to EEPROM rewritten, though the
    reason for the changes is unclear.

  - "digdugat" is almost identical to "digdug" (apart from the Atari gfx/copyright
    changes), but there are three added instructions in the CPU0 program that change
    the code alignment.  The change eliminates the "kill screen" at round 256 by
    making the round number roll over to 156, and hides the rollover from the player
    by only ever displaying the lower two digits of the round number.  Interestingly,
    "digdug" actually contains all the code to implement the rollover (at $0018-$0026)
    but just doesn't call it, implying that Namco deliberately chose to keep the kill
    screen in this version.

  - "digsid" is intermediate between "digdugb" and "digdug"; it has the changed EEPROM
    handling, but not the gameplay bug fixes.  It has some unique changes as well:
    the initial high scores are 25000 instead of 10000, and the game begins on the
    screen that is round 4 in the other sets, skipping the first three screens.
    The latter change seems likely to have been done by Namco themselves and not by
    Sidam, as it involves insertion of code right in the middle of the CPU0 program
    and realignment of all the code after the insertion.

  - "dzigzag" and "digdugb" are identical, apart from the hacked gfx and the copyright
    notices changed from "NAMCO LTD." to "1 9 8 2".  It's a bootleg of "digdugb", and
    not of "digduga1", because the hidden "NAMCO" string at offset 0x1eea of CPU2 is
    still present, while it is replaced by "ATARI" in digduga1.
    The only interesting thing about the bootleg is the 4th Z80, used to simulate
    the custom 5xXX chips of the original.


TODO:
----
- bosco & galaga:
  - the starfield is wrong.
  - The function of STARCLR is unknown. It is not latched and there are no data bits
    used...

- bosco: is the scrolling tilemap placement correct? It is currently aligned so that
  the test grid shown on startup is correct, but this way an unerased grey strip
  remains on the left of the screen during the title sequence. Alignment of the
  bullets/radar blips is also mysterious. Currently the radar blips are perfectly
  aligned with the radar, but the alignment of the player bullets with the player
  ship differs by one horizontal pixel when the screen is flipped.

- gallag/gatsbee: explosions are not emulated since the bootleg board doesn't have
  the 54XX custom. Should probably use samples like Battles?

- Xevios: emulate the 4th Z80 (ROM dump is complete)

- xevious: I haven't found any Easter egg in service mode. The main loop is very
  simple so there might just not be one, though this would be the only Namco game
  of that era to not have a service mode Easter egg. On the other hand, the service
  mode in this game is VERY spartan when compared to the other Namco games.

- dzigzag: emulate the 4th CPU (should be similar to battles)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mb88xx/mb88xx.h"
#include "machine/atari_vg.h"
#include "machine/namco06.h"
#include "machine/namco50.h"
#include "machine/namco51.h"
#include "machine/namco53.h"
#include "includes/galaga.h"
#include "audio/namco52.h"
#include "machine/rescap.h"
#include "sound/samples.h"
#include "audio/namco54.h"

#define MASTER_CLOCK (XTAL_18_432MHz)



READ8_MEMBER(galaga_state::bosco_dsw_r)
{
	int bit0,bit1;

	bit0 = (ioport("DSWB")->read() >> offset) & 1;
	bit1 = (ioport("DSWA")->read() >> offset) & 1;

	return bit0 | (bit1 << 1);
}

WRITE8_MEMBER(galaga_state::galaga_flip_screen_w)
{
	flip_screen_set(data & 1);
}

WRITE8_MEMBER(bosco_state::bosco_flip_screen_w)
{
	flip_screen_set(~data & 1);
}


WRITE8_MEMBER(galaga_state::bosco_latch_w)
{
	switch (offset)
	{
		case 0x00:  /* IRQ1 */
			m_main_irq_mask = data & 1;
			if (!m_main_irq_mask)
				m_maincpu->set_input_line(0, CLEAR_LINE);
			break;

		case 0x01:  /* IRQ2 */
			m_sub_irq_mask = data & 1;
			if (!m_sub_irq_mask)
				m_subcpu->set_input_line(0, CLEAR_LINE);
			break;

		case 0x02:  /* NMION */
			m_sub2_nmi_mask = !(data & 1);
			break;

		case 0x03:  /* RESET */
			m_subcpu->set_input_line(INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
			m_subcpu2->set_input_line(INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 0x04:  /* n.c. */
			break;

		case 0x05:  /* MOD 0 (xevious: n.c.) */
			m_custom_mod = (m_custom_mod & ~0x01) | ((data & 1) << 0);
			break;

		case 0x06:  /* MOD 1 (xevious: n.c.) */
			m_custom_mod = (m_custom_mod & ~0x02) | ((data & 1) << 1);
			break;

		case 0x07:  /* MOD 2 (xevious: n.c.) */
			m_custom_mod = (m_custom_mod & ~0x04) | ((data & 1) << 2);
			break;
	}
}

CUSTOM_INPUT_MEMBER(digdug_state::shifted_port_r){ return ioport((const char *)param)->read() >> 4; }

WRITE8_MEMBER(galaga_state::out_0)
{
	set_led_status(machine(), 1,data & 1);
	set_led_status(machine(), 0,data & 2);
	coin_counter_w(machine(), 1,~data & 4);
	coin_counter_w(machine(), 0,~data & 8);
}

WRITE8_MEMBER(galaga_state::out_1)
{
	coin_lockout_global_w(machine(), data & 1);
}

READ8_MEMBER(galaga_state::namco_52xx_rom_r)
{
	UINT32 length = memregion("52xx")->bytes();
//printf("ROM read %04X\n", offset);
	if (!(offset & 0x1000))
		offset = (offset & 0xfff) | 0x0000;
	else if (!(offset & 0x2000))
		offset = (offset & 0xfff) | 0x1000;
	else if (!(offset & 0x4000))
		offset = (offset & 0xfff) | 0x2000;
	else if (!(offset & 0x8000))
		offset = (offset & 0xfff) | 0x3000;
	return (offset < length) ? memregion("52xx")->base()[offset] : 0xff;
}

READ8_MEMBER(galaga_state::namco_52xx_si_r)
{
	/* pulled to GND */
	return 0;
}

READ8_MEMBER(galaga_state::custom_mod_r)
{
	/* MOD0-2 is connected to K1-3; K0 is left unconnected */
	return m_custom_mod << 1;
}

TIMER_CALLBACK_MEMBER(galaga_state::cpu3_interrupt_callback)
{
	int scanline = param;

	if(m_sub2_nmi_mask)
		nmi_line_pulse(m_subcpu2);

	scanline = scanline + 128;
	if (scanline >= 272)
		scanline = 64;

	/* the vertical synch chain is clocked by H256 -- this is probably not important, but oh well */
	m_cpu3_interrupt_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


MACHINE_START_MEMBER(galaga_state,galaga)
{
	/* create the interrupt timer */
	m_cpu3_interrupt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(galaga_state::cpu3_interrupt_callback),this));
	m_custom_mod = 0;
	save_item(NAME(m_custom_mod));
	save_item(NAME(m_main_irq_mask));
	save_item(NAME(m_sub_irq_mask));
	save_item(NAME(m_sub2_nmi_mask));
}

void galaga_state::bosco_latch_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int i;

	/* Reset all latches */
	for (i = 0;i < 8;i++)
		bosco_latch_w(space,i,0);
}

MACHINE_RESET_MEMBER(galaga_state,galaga)
{
	/* Reset all latches */
	bosco_latch_reset();

	m_cpu3_interrupt_timer->adjust(m_screen->time_until_pos(64), 64);
}

MACHINE_RESET_MEMBER(xevious_state,battles)
{
	MACHINE_RESET_CALL_MEMBER(galaga);
	battles_customio_init();
}


/* the same memory map is used by all three CPUs; all RAM areas are shared */
static ADDRESS_MAP_START( bosco_map, AS_PROGRAM, 8, bosco_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_WRITENOP         /* the only area different for each CPU */
	AM_RANGE(0x6800, 0x6807) AM_READ(bosco_dsw_r)
	AM_RANGE(0x6800, 0x681f) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x6820, 0x6827) AM_WRITE(bosco_latch_w)                        /* misc latches */
	AM_RANGE(0x6830, 0x6830) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x7000, 0x70ff) AM_DEVREADWRITE("06xx_0", namco_06xx_device, data_r, data_w)
	AM_RANGE(0x7100, 0x7100) AM_DEVREADWRITE("06xx_0", namco_06xx_device, ctrl_r, ctrl_w)
	AM_RANGE(0x7800, 0x7fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x8000, 0x8fff) AM_RAM_WRITE(bosco_videoram_w) AM_SHARE("videoram")/* + sprite registers */
	AM_RANGE(0x9000, 0x90ff) AM_DEVREADWRITE("06xx_1", namco_06xx_device, data_r, data_w)
	AM_RANGE(0x9100, 0x9100) AM_DEVREADWRITE("06xx_1", namco_06xx_device, ctrl_r, ctrl_w)
	AM_RANGE(0x9800, 0x980f) AM_WRITEONLY AM_SHARE("bosco_radarattr")
	AM_RANGE(0x9810, 0x9810) AM_WRITE(bosco_scrollx_w)
	AM_RANGE(0x9820, 0x9820) AM_WRITE(bosco_scrolly_w)
	AM_RANGE(0x9830, 0x9830) AM_WRITEONLY AM_SHARE("starcontrol")
	AM_RANGE(0x9840, 0x9840) AM_WRITE(bosco_starclr_w)
	AM_RANGE(0x9870, 0x9870) AM_WRITE(bosco_flip_screen_w)
	AM_RANGE(0x9874, 0x9875) AM_WRITEONLY AM_SHARE("bosco_starblink")
ADDRESS_MAP_END


static ADDRESS_MAP_START( galaga_map, AS_PROGRAM, 8, galaga_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_WRITENOP         /* the only area different for each CPU */
	AM_RANGE(0x6800, 0x6807) AM_READ(bosco_dsw_r)
	AM_RANGE(0x6800, 0x681f) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x6820, 0x6827) AM_WRITE(bosco_latch_w)                        /* misc latches */
	AM_RANGE(0x6830, 0x6830) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x7000, 0x70ff) AM_DEVREADWRITE("06xx", namco_06xx_device, data_r, data_w)
	AM_RANGE(0x7100, 0x7100) AM_DEVREADWRITE("06xx", namco_06xx_device, ctrl_r, ctrl_w)
	AM_RANGE(0x8000, 0x87ff) AM_RAM_WRITE(galaga_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x8800, 0x8bff) AM_RAM AM_SHARE("galaga_ram1")
	AM_RANGE(0x9000, 0x93ff) AM_RAM AM_SHARE("galaga_ram2")
	AM_RANGE(0x9800, 0x9bff) AM_RAM AM_SHARE("galaga_ram3")
	AM_RANGE(0xa000, 0xa005) AM_WRITEONLY AM_SHARE("starcontrol")
	AM_RANGE(0xa007, 0xa007) AM_WRITE(galaga_flip_screen_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( xevious_map, AS_PROGRAM, 8, xevious_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_WRITENOP         /* the only area different for each CPU */
	AM_RANGE(0x6800, 0x6807) AM_READ(bosco_dsw_r)
	AM_RANGE(0x6800, 0x681f) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x6820, 0x6827) AM_WRITE(bosco_latch_w)    /* misc latches */
	AM_RANGE(0x6830, 0x6830) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x7000, 0x70ff) AM_DEVREADWRITE("06xx", namco_06xx_device, data_r, data_w)
	AM_RANGE(0x7100, 0x7100) AM_DEVREADWRITE("06xx", namco_06xx_device, ctrl_r, ctrl_w)
	AM_RANGE(0x7800, 0x7fff) AM_RAM AM_SHARE("share1")                          /* work RAM */
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("xevious_sr1") /* work RAM + sprite registers */
	AM_RANGE(0x9000, 0x97ff) AM_RAM AM_SHARE("xevious_sr2") /* work RAM + sprite registers */
	AM_RANGE(0xa000, 0xa7ff) AM_RAM AM_SHARE("xevious_sr3") /* work RAM + sprite registers */
	AM_RANGE(0xb000, 0xb7ff) AM_RAM_WRITE(xevious_fg_colorram_w) AM_SHARE("fg_colorram")
	AM_RANGE(0xb800, 0xbfff) AM_RAM_WRITE(xevious_bg_colorram_w) AM_SHARE("bg_colorram")
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(xevious_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(xevious_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xd000, 0xd07f) AM_WRITE(xevious_vh_latch_w)
	AM_RANGE(0xf000, 0xffff) AM_READWRITE(xevious_bb_r, xevious_bs_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( digdug_map, AS_PROGRAM, 8, digdug_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_WRITENOP         /* the only area different for each CPU */
	AM_RANGE(0x6800, 0x681f) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x6820, 0x6827) AM_WRITE(bosco_latch_w)                        /* misc latches */
	AM_RANGE(0x6830, 0x6830) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x7000, 0x70ff) AM_DEVREADWRITE("06xx", namco_06xx_device, data_r, data_w)
	AM_RANGE(0x7100, 0x7100) AM_DEVREADWRITE("06xx", namco_06xx_device, ctrl_r, ctrl_w)
	AM_RANGE(0x8000, 0x83ff) AM_RAM_WRITE(digdug_videoram_w) AM_SHARE("videoram") /* tilemap RAM (bottom half of RAM 0 */
	AM_RANGE(0x8400, 0x87ff) AM_RAM AM_SHARE("share1")                          /* work RAM (top half for RAM 0 */
	AM_RANGE(0x8800, 0x8bff) AM_RAM AM_SHARE("digdug_objram")   /* work RAM + sprite registers */
	AM_RANGE(0x9000, 0x93ff) AM_RAM AM_SHARE("digdug_posram")   /* work RAM + sprite registers */
	AM_RANGE(0x9800, 0x9bff) AM_RAM AM_SHARE("digdug_flpram")   /* work RAM + sprite registers */
	AM_RANGE(0xa000, 0xa007) AM_READNOP AM_WRITE(digdug_PORT_w)      /* video latches (spurious reads when setting latch bits) */
	AM_RANGE(0xb800, 0xb83f) AM_DEVREADWRITE("earom", atari_vg_earom_device, read, write)   /* non volatile memory data */
	AM_RANGE(0xb840, 0xb840) AM_DEVWRITE("earom", atari_vg_earom_device, ctrl_w)                    /* non volatile memory control */
ADDRESS_MAP_END



/* bootleg 4th CPU replacing the 5xXX chips */
static ADDRESS_MAP_START( galaga_mem4, AS_PROGRAM, 8, galaga_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0x107f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( battles_mem4, AS_PROGRAM, 8, xevious_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x4000, 0x4003) AM_READ(battles_input_port_r)
	AM_RANGE(0x4001, 0x4001) AM_WRITE(battles_CPU4_coin_w)
	AM_RANGE(0x5000, 0x5000) AM_WRITE(battles_noise_sound_w)
	AM_RANGE(0x6000, 0x6000) AM_READWRITE(battles_customio3_r, battles_customio3_w)
	AM_RANGE(0x7000, 0x7000) AM_READWRITE(battles_customio_data3_r, battles_customio_data3_w)
	AM_RANGE(0x8000, 0x80ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( dzigzag_mem4, AS_PROGRAM, 8, galaga_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0x107f) AM_RAM
	AM_RANGE(0x4000, 0x4007) AM_READONLY    // dip switches? bits 0 & 1 used
ADDRESS_MAP_END


static INPUT_PORTS_START( bosco )
	PORT_START("IN0L")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN0H")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )

	PORT_START("IN1L")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY

	PORT_START("IN1H")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) ) // factory default = "Yes"
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Freeze" )                    PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWB:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	/* bonus scores are different for 5 lives */
	PORT_DIPNAME( 0x38, 0x20, "Bonus Fighter" )         PORT_DIPLOCATION("SWA:4,5,6")
	PORT_DIPSETTING(    0x30, "15K and 50K Only" )      PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0) /* Began with 1, 2 or 3 fighters */
	PORT_DIPSETTING(    0x38, "20K and 70K Only" )      PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x08, "10K, 50K, Every 50K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x10, "15K, 50K, Every 50K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x18, "15K, 70K, Every 70K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x20, "20K, 70K, Every 70K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0) // factory default = "20K, 70K, Every70K"
	PORT_DIPSETTING(    0x28, "30K, 100K, Every 100K" ) PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x30, "30K, 100K, Every 100K" ) PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0) /* Began with 5 fighters */
	PORT_DIPSETTING(    0x38, "30K, 120K, Every 120K" ) PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x08, "15K and 70K Only" )      PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x10, "20K and 70K Only" )      PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x18, "20K and 100K Only" )     PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x20, "30K and 120K Only" )     PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x28, "30K, 80K, Every 80K" )   PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" ) // factory default = "3"
	PORT_DIPSETTING(    0xc0, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( boscomd )
	PORT_INCLUDE( bosco )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "2 Credits Game" )            PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x00, "1 Player" )
	PORT_DIPSETTING(    0x01, "2 Players" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" )                    PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( galaga )
	PORT_START("IN0L")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN0H")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )

	PORT_START("IN1L")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_START("IN1H")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL

	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWB:3" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Freeze" )                PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Rack Test" )             PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWA:4,5,6")
	PORT_DIPSETTING(    0x20, "20K, 60K, Every 60K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0) /* Began with 2, 3 or 4 fighters */
	PORT_DIPSETTING(    0x18, "20K and 60K Only" )      PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x10, "20K, 70K, Every 70K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0) // factory default = "20K, 70K, Every70K"
	PORT_DIPSETTING(    0x30, "20K, 80K, Every 80K" )   PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x38, "30K and 80K Only" )      PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x08, "30K, 100K, Every 100K" ) PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x28, "30K, 120K, Every 120K" ) PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWB",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x20, "30K, 100K, Every 100K" ) PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0) /* Began with 5 fighters */
	PORT_DIPSETTING(    0x18, "30K and 150K Only" )     PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x10, "30K, 120K, Every 120K" ) PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x30, "30K, 150K, Every 150K" ) PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x38, "30K Only" )              PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x08, "30K and 100K Only" )     PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x28, "30K and 120K Only" )     PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWB",0xc0,EQUALS,0xc0)
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x80, "3" ) // factory default = "3"
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0xc0, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( galagamw )
	PORT_INCLUDE( galaga )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "2 Credits Game" )        PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x00, "1 Player" )
	PORT_DIPSETTING(    0x01, "2 Players" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hardest ) )
INPUT_PORTS_END

/* the same as galaga but with vertical movement */
static INPUT_PORTS_START( gatsbee )
	PORT_INCLUDE( galaga )

	PORT_MODIFY("IN1L")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )

	PORT_MODIFY("IN1H")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_COCKTAIL
INPUT_PORTS_END


static INPUT_PORTS_START( xevious )
	PORT_START("IN0L")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN0H")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )

	PORT_START("IN1L")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY

	PORT_START("IN1H")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWA:3,4,5")
	PORT_DIPSETTING(    0x18, "10K, 40K, Every 40K" )   PORT_CONDITION("DSWA",0x60,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x14, "10K, 50K, Every 50K" )   PORT_CONDITION("DSWA",0x60,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x10, "20K, 50K, Every 50K" )   PORT_CONDITION("DSWA",0x60,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x1c, "20K, 60K, Every 60K" )   PORT_CONDITION("DSWA",0x60,NOTEQUALS,0x00) // factory default = "20K, 60K, Every60K"
	PORT_DIPSETTING(    0x0c, "20K, 70K, Every 70K" )   PORT_CONDITION("DSWA",0x60,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x08, "20K, 80K, Every 80K" )   PORT_CONDITION("DSWA",0x60,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x04, "20K and 60K Only" )      PORT_CONDITION("DSWA",0x60,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWA",0x60,NOTEQUALS,0x00)
	PORT_DIPSETTING(    0x18, "10K, 50K, Every 50K" )   PORT_CONDITION("DSWA",0x60,EQUALS,0x00)
	PORT_DIPSETTING(    0x14, "20K, 50K, Every 50K" )   PORT_CONDITION("DSWA",0x60,EQUALS,0x00)
	PORT_DIPSETTING(    0x10, "20K, 60K, Every 60K" )   PORT_CONDITION("DSWA",0x60,EQUALS,0x00)
	PORT_DIPSETTING(    0x1c, "20K, 70K, Every 70K" )   PORT_CONDITION("DSWA",0x60,EQUALS,0x00)
	PORT_DIPSETTING(    0x0c, "20K, 80K, Every 80K" )   PORT_CONDITION("DSWA",0x60,EQUALS,0x00)
	PORT_DIPSETTING(    0x08, "30K, 100K, Every 100K" ) PORT_CONDITION("DSWA",0x60,EQUALS,0x00)
	PORT_DIPSETTING(    0x04, "20K and 80K Only" )      PORT_CONDITION("DSWA",0x60,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWA",0x60,EQUALS,0x00)
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:6,7")
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x60, "3" ) // factory default = "3"
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSWB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_DIPNAME( 0x02, 0x02, "Flags Award Bonus Life" )    PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWB:6,7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                    PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* same as xevious but different "Coin B" Dip Switch and "Copyright" Dip Switch instead of "Freeze" */
static INPUT_PORTS_START( xeviousa )
	PORT_INCLUDE( xevious )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	/* when switch is on Namco, high score names are 10 letters long */
	PORT_DIPNAME( 0x80, 0x80, "Copyright" )         PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, "Namco" )
	PORT_DIPSETTING(    0x80, "Atari/Namco" )
INPUT_PORTS_END

/* same as xevious but "Copyright" Dip Switch instead of "Freeze" */
static INPUT_PORTS_START( xeviousb )
	PORT_INCLUDE( xevious )

	PORT_MODIFY("DSWB")
	/* when switch is on Namco, high score names are 10 letters long */
	PORT_DIPNAME( 0x80, 0x80, "Copyright" )         PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, "Namco" )
	PORT_DIPSETTING(    0x80, "Atari/Namco" )
INPUT_PORTS_END

/* same as xevious but different "Coin B" Dip Switch and inverted "Freeze" Dip Switch */
static INPUT_PORTS_START( sxevious )
	PORT_INCLUDE( xevious )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" )            PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( digdug )
	PORT_START("IN0L")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN0H")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )

	PORT_START("IN1L")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY

	PORT_START("IN1H")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x38, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWA:4,5,6")
	PORT_DIPSETTING(    0x20, "10K, 40K, Every 40K" )   PORT_CONDITION("DSWA",0xc0,NOTEQUALS,0xc0) // Atari factory default = "10K, 40K, Every40K"
	PORT_DIPSETTING(    0x10, "10K, 50K, Every 50K" )   PORT_CONDITION("DSWA",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x30, "20K, 60K, Every 60K" )   PORT_CONDITION("DSWA",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x08, "20K, 70K, Every 70K" )   PORT_CONDITION("DSWA",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x28, "10K and 40K Only" )      PORT_CONDITION("DSWA",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x18, "20K and 60K Only" )      PORT_CONDITION("DSWA",0xc0,NOTEQUALS,0xc0) // Namco factory default = "20K, 60K"
	PORT_DIPSETTING(    0x38, "10K Only" )              PORT_CONDITION("DSWA",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWA",0xc0,NOTEQUALS,0xc0)
	PORT_DIPSETTING(    0x20, "20K, 60K, Every 60K" )   PORT_CONDITION("DSWA",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x10, "30K, 80K, Every 80K" )   PORT_CONDITION("DSWA",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x30, "20K and 50K Only" )      PORT_CONDITION("DSWA",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x08, "20K and 60K Only" )      PORT_CONDITION("DSWA",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x28, "30K and 70K Only" )      PORT_CONDITION("DSWA",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x18, "20K Only" )              PORT_CONDITION("DSWA",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x38, "30K Only" )              PORT_CONDITION("DSWA",0xc0,EQUALS,0xc0)
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         PORT_CONDITION("DSWA",0xc0,EQUALS,0xc0)
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" ) // factory default = "3"
	PORT_DIPSETTING(    0xc0, "5" )

	PORT_START("DSWA_HI")
	PORT_BIT( 0x0f, 0x00, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, digdug_state,shifted_port_r, "DSWA")

	PORT_START("DSWB") // reverse order against SWA
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" )                    PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) ) // factory default = "No"
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )

	PORT_START("DSWB_HI")
	PORT_BIT( 0x0f, 0x00, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, digdug_state,shifted_port_r, "DSWB")
INPUT_PORTS_END

/*
static INPUT_PORTS_START( digdugja ) // Namco older?
    PORT_INCLUDE( digdug )

    PORT_MODIFY("DSWB") // same order as SWA
    PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWB:2,1")
    PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
    PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
    PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
    PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
    PORT_DIPNAME( 0x04, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:3")
    PORT_DIPSETTING(    0x04, DEF_STR( No ) ) // Namco factory default = "No"
    PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
    PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWB:4")
    PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x10, 0x10, "Freeze" )                    PORT_DIPLOCATION("SWB:5")
    PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x60, 0x00, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SWB:7,6")
    PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
    PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
    PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( digdugus ) // Atari older?
    PORT_INCLUDE( digdug )

    PORT_MODIFY("DSWB") // reverse order against SWA
    PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SWB:1,2")
    PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0xc0, DEF_STR( 2C_3C ) )
    PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
    PORT_DIPNAME( 0x20, 0x20, "Freeze" )                    PORT_DIPLOCATION("SWB:3")
    PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWB:4")
    PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x08, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:5")
    PORT_DIPSETTING(    0x08, DEF_STR( No ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Yes ) ) // Atari factory default = "Yes"
    PORT_DIPNAME( 0x06, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWB:6,7")
    PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
    PORT_DIPSETTING(    0x04, DEF_STR( Medium ) )
    PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
    PORT_DIPSETTING(    0x06, DEF_STR( Hardest ) )
    PORT_DIPNAME( 0x01, 0x01, "Number Of Coin Counter(s)" ) PORT_DIPLOCATION("SWB:8")
    PORT_DIPSETTING(    0x01, "Two Coin Counters" )
    PORT_DIPSETTING(    0x00, "One Coin Counter" )
INPUT_PORTS_END
*/



static const gfx_layout charlayout_2bpp =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8,1), STEP4(0*8,1) },
	{ STEP8(0*8,8) },
	16*8
};

static const gfx_layout charlayout_xevious =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout charlayout_digdug =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(7,-1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout bgcharlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 0, RGN_FRAC(1,2) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout spritelayout_bosco =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1), STEP4(0*8,1) },
	{ STEP8(0*8,8), STEP8(32*8,8) },
	64*8
};

static const gfx_layout spritelayout_galaga =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(0*8,1), STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1) },
	{ STEP8(0*8,8), STEP8(32*8,8) },
	64*8
};

static const gfx_layout spritelayout_xevious =
{
	16,16,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2)+4, 0, 4 },
	{ STEP4(0*8,1), STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1) },
	{ STEP8(0*8,8), STEP8(32*8,8) },
	64*8
};

static const gfx_layout dotlayout =
{
	4,4,
	8,
	3,  /* 2 bits color + 1 bit transparency */
	{ 5, 6, 7 },
	{ STEP4(0,8) },
	{ STEP4(0,32) },
	16*8
};

static GFXDECODE_START( bosco )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_2bpp,       0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout_bosco, 64*4, 64 )
	GFXDECODE_ENTRY( "gfx3", 0, dotlayout,     64*4+64*4,  1 )
GFXDECODE_END

static GFXDECODE_START( galaga )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_2bpp,        0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout_galaga, 64*4, 64 )
GFXDECODE_END

static GFXDECODE_START( xevious )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_xevious, 128*4+64*8,  64 )
	GFXDECODE_ENTRY( "gfx2", 0, bgcharlayout,                0, 128 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout_xevious,    128*4,  64 )
GFXDECODE_END

static GFXDECODE_START( digdug )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_digdug,         0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout_galaga,    16*2, 64 )
	GFXDECODE_ENTRY( "gfx3", 0, charlayout_2bpp, 64*4 + 16*2, 64 )
GFXDECODE_END


/* The resistance path of the namco sound is 16k compared to
 * the 10k of the highest gain 54xx filter. Giving a 10/16 gain.
 */

static const char *const battles_sample_names[] =
{
	"*battles",
	"explo1",   /* ground target explosion */
	"explo2",   /* Solvalou explosion */
	nullptr   /* end of array */
};

INTERRUPT_GEN_MEMBER(galaga_state::main_vblank_irq)
{
	if(m_main_irq_mask)
		device.execute().set_input_line(0, ASSERT_LINE);
}

INTERRUPT_GEN_MEMBER(galaga_state::sub_vblank_irq)
{
	if(m_sub_irq_mask)
		device.execute().set_input_line(0, ASSERT_LINE);
}

static MACHINE_CONFIG_START( bosco, bosco_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/6)    /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(bosco_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galaga_state,  main_vblank_irq)

	MCFG_CPU_ADD("sub", Z80, MASTER_CLOCK/6)    /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(bosco_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galaga_state,  sub_vblank_irq)

	MCFG_CPU_ADD("sub2", Z80, MASTER_CLOCK/6)   /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(bosco_map)

	MCFG_NAMCO_50XX_ADD("50xx_1", MASTER_CLOCK/6/2) /* 1.536 MHz */
	MCFG_NAMCO_50XX_ADD("50xx_2", MASTER_CLOCK/6/2) /* 1.536 MHz */
	MCFG_NAMCO_51XX_ADD("51xx", MASTER_CLOCK/6/2)      /* 1.536 MHz */
	MCFG_NAMCO_51XX_INPUT_0_CB(IOPORT("IN0L"))
	MCFG_NAMCO_51XX_INPUT_1_CB(IOPORT("IN0H"))
	MCFG_NAMCO_51XX_INPUT_2_CB(IOPORT("IN1L"))
	MCFG_NAMCO_51XX_INPUT_3_CB(IOPORT("IN1H"))
	MCFG_NAMCO_51XX_OUTPUT_0_CB(WRITE8(galaga_state,out_0))
	MCFG_NAMCO_51XX_OUTPUT_1_CB(WRITE8(galaga_state,out_1))


	MCFG_NAMCO_52XX_ADD("52xx", MASTER_CLOCK/6/2)      /* 1.536 MHz */
	MCFG_NAMCO_52XX_DISCRETE("discrete")
	MCFG_NAMCO_52XX_BASENODE(NODE_04)
	MCFG_NAMCO_52XX_EXT_CLOCK(ATTOSECONDS_IN_NSEC(PERIOD_OF_555_ASTABLE_NSEC(RES_K(33), RES_K(10), CAP_U(0.0047))))
	MCFG_NAMCO_52XX_ROMREAD_CB(READ8(galaga_state,namco_52xx_rom_r))
	MCFG_NAMCO_52XX_SI_CB(READ8(galaga_state,namco_52xx_si_r))

	MCFG_NAMCO_54XX_ADD("54xx", MASTER_CLOCK/6/2)      /* 1.536 MHz */
	MCFG_NAMCO_54XX_DISCRETE("discrete")
	MCFG_NAMCO_54XX_BASENODE(NODE_01)

	MCFG_NAMCO_06XX_ADD("06xx_0", MASTER_CLOCK/6/64)
	MCFG_NAMCO_06XX_MAINCPU("maincpu")
	MCFG_NAMCO_06XX_READ_0_CB(DEVREAD8("51xx", namco_51xx_device, read))
	MCFG_NAMCO_06XX_WRITE_0_CB(DEVWRITE8("51xx", namco_51xx_device, write))
	MCFG_NAMCO_06XX_READ_2_CB(DEVREAD8("50xx_1", namco_50xx_device, read))
	MCFG_NAMCO_06XX_READ_REQUEST_2_CB(DEVWRITELINE("50xx_1", namco_50xx_device, read_request))
	MCFG_NAMCO_06XX_WRITE_2_CB(DEVWRITE8("50xx_1", namco_50xx_device, write))
	MCFG_NAMCO_06XX_WRITE_3_CB(DEVWRITE8("54xx", namco_54xx_device, write))

	MCFG_NAMCO_06XX_ADD("06xx_1", MASTER_CLOCK/6/64)
	MCFG_NAMCO_06XX_MAINCPU("sub")
	MCFG_NAMCO_06XX_READ_0_CB(DEVREAD8("50xx_2", namco_50xx_device, read))
	MCFG_NAMCO_06XX_READ_REQUEST_0_CB(DEVWRITELINE("50xx_2", namco_50xx_device, read_request))
	MCFG_NAMCO_06XX_WRITE_0_CB(DEVWRITE8("50xx_2", namco_50xx_device, write))
	MCFG_NAMCO_06XX_WRITE_1_CB(DEVWRITE8("52xx", namco_52xx_device, write))

	MCFG_WATCHDOG_VBLANK_INIT(8)
	MCFG_QUANTUM_TIME(attotime::from_hz(6000))  /* 100 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */
	MCFG_MACHINE_START_OVERRIDE(bosco_state,galaga)
	MCFG_MACHINE_RESET_OVERRIDE(bosco_state,galaga)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/3, 384, 0, 288, 264, 16, 224+16)
	MCFG_SCREEN_UPDATE_DRIVER(bosco_state, screen_update_bosco)
	MCFG_SCREEN_VBLANK_DRIVER(bosco_state, screen_eof_bosco)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bosco)
	MCFG_PALETTE_ADD("palette", 64*4+64*4+4+64)
	MCFG_PALETTE_INDIRECT_ENTRIES(32+64)
	MCFG_PALETTE_INIT_OWNER(bosco_state,bosco)
	MCFG_VIDEO_START_OVERRIDE(bosco_state,bosco)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("namco", NAMCO, MASTER_CLOCK/6/32)
	MCFG_NAMCO_AUDIO_VOICES(3)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90 * 10.0 / 16.0)

	/* discrete circuit on the 54XX outputs */
	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(bosco)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( galaga, galaga_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/6)    /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(galaga_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galaga_state,  main_vblank_irq)

	MCFG_CPU_ADD("sub", Z80, MASTER_CLOCK/6)    /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(galaga_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galaga_state,  sub_vblank_irq)

	MCFG_CPU_ADD("sub2", Z80, MASTER_CLOCK/6)   /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(galaga_map)

	MCFG_NAMCO_51XX_ADD("51xx", MASTER_CLOCK/6/2)      /* 1.536 MHz */
	MCFG_NAMCO_51XX_INPUT_0_CB(IOPORT("IN0L"))
	MCFG_NAMCO_51XX_INPUT_1_CB(IOPORT("IN0H"))
	MCFG_NAMCO_51XX_INPUT_2_CB(IOPORT("IN1L"))
	MCFG_NAMCO_51XX_INPUT_3_CB(IOPORT("IN1H"))
	MCFG_NAMCO_51XX_OUTPUT_0_CB(WRITE8(galaga_state,out_0))
	MCFG_NAMCO_51XX_OUTPUT_1_CB(WRITE8(galaga_state,out_1))

	MCFG_NAMCO_54XX_ADD("54xx", MASTER_CLOCK/6/2)      /* 1.536 MHz */
	MCFG_NAMCO_54XX_DISCRETE("discrete")
	MCFG_NAMCO_54XX_BASENODE(NODE_01)

	MCFG_NAMCO_06XX_ADD("06xx", MASTER_CLOCK/6/64)
	MCFG_NAMCO_06XX_MAINCPU("maincpu")
	MCFG_NAMCO_06XX_READ_0_CB(DEVREAD8("51xx", namco_51xx_device, read))
	MCFG_NAMCO_06XX_WRITE_0_CB(DEVWRITE8("51xx", namco_51xx_device, write))
	MCFG_NAMCO_06XX_WRITE_3_CB(DEVWRITE8("54xx", namco_54xx_device, write))

	MCFG_WATCHDOG_VBLANK_INIT(8)
	MCFG_QUANTUM_TIME(attotime::from_hz(6000))  /* 100 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */
	MCFG_MACHINE_START_OVERRIDE(galaga_state,galaga)
	MCFG_MACHINE_RESET_OVERRIDE(galaga_state,galaga)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/3, 384, 0, 288, 264, 0, 224)
	MCFG_SCREEN_UPDATE_DRIVER(galaga_state, screen_update_galaga)
	MCFG_SCREEN_VBLANK_DRIVER(galaga_state, screen_eof_galaga)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", galaga)
	MCFG_PALETTE_ADD("palette", 64*4+64*4+64)
	MCFG_PALETTE_INDIRECT_ENTRIES(32+64)
	MCFG_PALETTE_INIT_OWNER(galaga_state,galaga)
	MCFG_VIDEO_START_OVERRIDE(galaga_state,galaga)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("namco", NAMCO, MASTER_CLOCK/6/32)
	MCFG_NAMCO_AUDIO_VOICES(3)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90 * 10.0 / 16.0)

	/* discrete circuit on the 54XX outputs */
	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(galaga)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( galagab, galaga )

	/* basic machine hardware */

	MCFG_DEVICE_REMOVE("54xx")
	MCFG_DEVICE_REMOVE("06xx")

	/* FIXME: bootlegs should not have any Namco custom chip. However, this workaround is needed atm */
	MCFG_NAMCO_06XX_ADD("06xx", MASTER_CLOCK/6/64)
	MCFG_NAMCO_06XX_MAINCPU("maincpu")
	MCFG_NAMCO_06XX_READ_0_CB(DEVREAD8("51xx", namco_51xx_device, read))
	MCFG_NAMCO_06XX_WRITE_0_CB(DEVWRITE8("51xx", namco_51xx_device, write))

	MCFG_CPU_ADD("sub3", Z80, MASTER_CLOCK/6)   /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(galaga_mem4)

	/* sound hardware */
	MCFG_DEVICE_REMOVE("discrete")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( xevious, xevious_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/6)    /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(xevious_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galaga_state,  main_vblank_irq)

	MCFG_CPU_ADD("sub", Z80,MASTER_CLOCK/6) /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(xevious_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galaga_state,  sub_vblank_irq)

	MCFG_CPU_ADD("sub2", Z80, MASTER_CLOCK/6)   /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(xevious_map)

	MCFG_NAMCO_50XX_ADD("50xx", MASTER_CLOCK/6/2)   /* 1.536 MHz */

	MCFG_NAMCO_51XX_ADD("51xx", MASTER_CLOCK/6/2)      /* 1.536 MHz */
	MCFG_NAMCO_51XX_INPUT_0_CB(IOPORT("IN0L"))
	MCFG_NAMCO_51XX_INPUT_1_CB(IOPORT("IN0H"))
	MCFG_NAMCO_51XX_INPUT_2_CB(IOPORT("IN1L"))
	MCFG_NAMCO_51XX_INPUT_3_CB(IOPORT("IN1H"))
	MCFG_NAMCO_51XX_OUTPUT_0_CB(WRITE8(galaga_state,out_0))
	MCFG_NAMCO_51XX_OUTPUT_1_CB(WRITE8(galaga_state,out_1))

	MCFG_NAMCO_54XX_ADD("54xx", MASTER_CLOCK/6/2)      /* 1.536 MHz */
	MCFG_NAMCO_54XX_DISCRETE("discrete")
	MCFG_NAMCO_54XX_BASENODE(NODE_01)

	MCFG_NAMCO_06XX_ADD("06xx", MASTER_CLOCK/6/64)
	MCFG_NAMCO_06XX_MAINCPU("maincpu")
	MCFG_NAMCO_06XX_READ_0_CB(DEVREAD8("51xx", namco_51xx_device, read))
	MCFG_NAMCO_06XX_WRITE_0_CB(DEVWRITE8("51xx", namco_51xx_device, write))
	MCFG_NAMCO_06XX_READ_2_CB(DEVREAD8("50xx", namco_50xx_device, read))
	MCFG_NAMCO_06XX_READ_REQUEST_2_CB(DEVWRITELINE("50xx", namco_50xx_device, read_request))
	MCFG_NAMCO_06XX_WRITE_2_CB(DEVWRITE8("50xx", namco_50xx_device, write))
	MCFG_NAMCO_06XX_WRITE_3_CB(DEVWRITE8("54xx", namco_54xx_device, write))

	MCFG_WATCHDOG_VBLANK_INIT(8)
	MCFG_QUANTUM_TIME(attotime::from_hz(60000)) /* 1000 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */
	MCFG_MACHINE_START_OVERRIDE(galaga_state,galaga)
	MCFG_MACHINE_RESET_OVERRIDE(galaga_state,galaga)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/3, 384, 0, 288, 264, 0, 224)
	MCFG_SCREEN_UPDATE_DRIVER(xevious_state, screen_update_xevious)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", xevious)
	MCFG_PALETTE_ADD("palette", 128*4+64*8+64*2)
	MCFG_PALETTE_INDIRECT_ENTRIES(128+1)
	MCFG_PALETTE_INIT_OWNER(xevious_state,xevious)
	MCFG_VIDEO_START_OVERRIDE(xevious_state,xevious)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("namco", NAMCO, MASTER_CLOCK/6/32)
	MCFG_NAMCO_AUDIO_VOICES(3)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90 * 10.0 / 16.0)

	/* discrete circuit on the 54XX outputs */
	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(galaga)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( battles, xevious )

	/* basic machine hardware */

	MCFG_DEVICE_REMOVE("50xx")
	MCFG_DEVICE_REMOVE("54xx")
	MCFG_DEVICE_REMOVE("06xx")

	/* FIXME: bootlegs should not have any Namco custom chip. However, this workaround is needed atm */
	MCFG_NAMCO_06XX_ADD("06xx", MASTER_CLOCK/6/64)
	MCFG_NAMCO_06XX_MAINCPU("maincpu")
	MCFG_NAMCO_06XX_READ_0_CB(DEVREAD8("51xx", namco_51xx_device, read))
	MCFG_NAMCO_06XX_WRITE_0_CB(DEVWRITE8("51xx", namco_51xx_device, write))

	MCFG_CPU_ADD("sub3", Z80, MASTER_CLOCK/6)   /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(battles_mem4)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", xevious_state, battles_interrupt_4)

	MCFG_TIMER_DRIVER_ADD("battles_nmi", xevious_state, battles_nmi_generate)

	MCFG_MACHINE_RESET_OVERRIDE(xevious_state,battles)

	/* video hardware */
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(xevious_state,battles)

	/* sound hardware */
	MCFG_DEVICE_REMOVE("discrete")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_NAMES(battles_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( digdug, digdug_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/6)    /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(digdug_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galaga_state,  main_vblank_irq)

	MCFG_CPU_ADD("sub", Z80, MASTER_CLOCK/6)    /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(digdug_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galaga_state,  sub_vblank_irq)

	MCFG_CPU_ADD("sub2", Z80, MASTER_CLOCK/6)   /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(digdug_map)

	MCFG_NAMCO_51XX_ADD("51xx", MASTER_CLOCK/6/2)      /* 1.536 MHz */
	MCFG_NAMCO_51XX_INPUT_0_CB(IOPORT("IN0L"))
	MCFG_NAMCO_51XX_INPUT_1_CB(IOPORT("IN0H"))
	MCFG_NAMCO_51XX_INPUT_2_CB(IOPORT("IN1L"))
	MCFG_NAMCO_51XX_INPUT_3_CB(IOPORT("IN1H"))
	MCFG_NAMCO_51XX_OUTPUT_0_CB(WRITE8(galaga_state,out_0))
	MCFG_NAMCO_51XX_OUTPUT_1_CB(WRITE8(galaga_state,out_1))

	MCFG_NAMCO_53XX_ADD("53xx", MASTER_CLOCK/6/2)      /* 1.536 MHz */
	MCFG_NAMCO_53XX_K_CB(READ8(galaga_state,custom_mod_r))
	MCFG_NAMCO_53XX_INPUT_0_CB(IOPORT("DSWA"))
	MCFG_NAMCO_53XX_INPUT_1_CB(IOPORT("DSWA_HI"))
	MCFG_NAMCO_53XX_INPUT_2_CB(IOPORT("DSWB"))
	MCFG_NAMCO_53XX_INPUT_3_CB(IOPORT("DSWB_HI"))

	MCFG_NAMCO_06XX_ADD("06xx", MASTER_CLOCK/6/64)
	MCFG_NAMCO_06XX_MAINCPU("maincpu")
	MCFG_NAMCO_06XX_READ_0_CB(DEVREAD8("51xx", namco_51xx_device, read))
	MCFG_NAMCO_06XX_WRITE_0_CB(DEVWRITE8("51xx", namco_51xx_device, write))
	MCFG_NAMCO_06XX_READ_1_CB(DEVREAD8("53xx", namco_53xx_device, read))
	MCFG_NAMCO_06XX_READ_REQUEST_1_CB(DEVWRITELINE("53xx", namco_53xx_device, read_request))

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))  /* 100 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */
	MCFG_MACHINE_START_OVERRIDE(galaga_state,galaga)
	MCFG_MACHINE_RESET_OVERRIDE(galaga_state,galaga)

	MCFG_ATARIVGEAROM_ADD("earom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/3, 384, 0, 288, 264, 0, 224)
	MCFG_SCREEN_UPDATE_DRIVER(digdug_state, screen_update_digdug)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", digdug)
	MCFG_PALETTE_ADD("palette", 16*2+64*4+64*4)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(digdug_state,digdug)
	MCFG_VIDEO_START_OVERRIDE(digdug_state,digdug)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("namco", NAMCO, MASTER_CLOCK/6/32)
	MCFG_NAMCO_AUDIO_VOICES(3)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90 * 10.0 / 16.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dzigzag, digdug )

	/* basic machine hardware */

	MCFG_CPU_ADD("sub3", Z80, MASTER_CLOCK/6)   /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(dzigzag_mem4)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

/**********************************************************************************************
  Bosconian & clones
**********************************************************************************************/
/*

Bosconian
Namco/Midway, 1981

*/

ROM_START( bosco )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "bos3_1.3n",    0x0000, 0x1000, CRC(96021267) SHA1(bd49b0caabcccf9df45a272d767456a4fc8a7c07) )
	ROM_LOAD( "bos1_2.3m",    0x1000, 0x1000, CRC(2d8f3ebe) SHA1(75de1cba7531ae4bf7fbbef7b8e37b9fec4ed0d0) )
	ROM_LOAD( "bos1_3.3l",    0x2000, 0x1000, CRC(c80ccfa5) SHA1(f2bbec2ea9846d4601f06c0b4242744447a88fda) )
	ROM_LOAD( "bos1_4b.3k",   0x3000, 0x1000, CRC(a3f7f4ab) SHA1(eb26184311bae0767c7a5593926e6eadcbcb680e) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "bos1_5c.3j",   0x0000, 0x1000, CRC(a7c8e432) SHA1(3607be75daa10f1f98dbfd9e600c5ba513130d44) )
	ROM_LOAD( "bos3_6.3h",    0x1000, 0x1000, CRC(4543cf82) SHA1(50ad7d1ab6694eb8fab88d0fa79ee04f6984f3ca) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "bos1_7.3e",    0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "bos1_14.5d",   0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "bos1_13.5e",   0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "bos1-4.2r",    0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bos1-6.6b",    0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bos1-5.4m",    0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "bos1-3.2d",    0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "bos1-7.7h",    0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "bos1-1.1d",    0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "bos1-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "bos1_9.5n",    0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "bos1_10.5m",   0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "bos1_11.5k",   0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )
ROM_END

ROM_START( boscoo )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "bos1_1.3n",    0x0000, 0x1000, CRC(0d9920e7) SHA1(e7633233f603ccb5b7a970ed5b58ef361ef2c94e) )
	ROM_LOAD( "bos1_2.3m",    0x1000, 0x1000, CRC(2d8f3ebe) SHA1(75de1cba7531ae4bf7fbbef7b8e37b9fec4ed0d0) )
	ROM_LOAD( "bos1_3.3l",    0x2000, 0x1000, CRC(c80ccfa5) SHA1(f2bbec2ea9846d4601f06c0b4242744447a88fda) )
	ROM_LOAD( "bos1_4b.3k",   0x3000, 0x1000, CRC(a3f7f4ab) SHA1(eb26184311bae0767c7a5593926e6eadcbcb680e) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "bos1_5c.3j",   0x0000, 0x1000, CRC(a7c8e432) SHA1(3607be75daa10f1f98dbfd9e600c5ba513130d44) )
	ROM_LOAD( "bos1_6.3h",    0x1000, 0x1000, CRC(31b8c648) SHA1(de0db24d385d2361ec989bf32388df8202ad535c) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "bos1_7.3e",    0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "bos1_14.5d",   0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "bos1_13.5e",   0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "bos1-4.2r",    0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bos1-6.6b",    0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bos1-5.4m",    0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "bos1-3.2d",    0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "bos1-7.7h",    0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "bos1-1.1d",    0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "bos1-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "bos1_9.5n",    0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "bos1_10.5m",   0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "bos1_11.5k",   0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )
ROM_END

ROM_START( boscoo2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "bos1_1.3n",    0x0000, 0x1000, CRC(0d9920e7) SHA1(e7633233f603ccb5b7a970ed5b58ef361ef2c94e) )
	ROM_LOAD( "bos1_2.3m",    0x1000, 0x1000, CRC(2d8f3ebe) SHA1(75de1cba7531ae4bf7fbbef7b8e37b9fec4ed0d0) )
	ROM_LOAD( "bos1_3.3l",    0x2000, 0x1000, CRC(c80ccfa5) SHA1(f2bbec2ea9846d4601f06c0b4242744447a88fda) )
	ROM_LOAD( "bos1_4.3k",    0x3000, 0x1000, CRC(7ebea2b8) SHA1(92fc66526ed77f3efd947b7d321b255aba4a0140) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "bos1_5b.3j",   0x0000, 0x1000, CRC(3d6955a8) SHA1(f89860d74865da5ced2f5b2196bdaa8eeb5e2322) )
	ROM_LOAD( "bos1_6.3h",    0x1000, 0x1000, CRC(31b8c648) SHA1(de0db24d385d2361ec989bf32388df8202ad535c) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "bos1_7.3e",    0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "bos1_14.5d",   0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "bos1_13.5e",   0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "bos1-4.2r",    0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bos1-6.6b",    0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bos1-5.4m",    0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "bos1-3.2d",    0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "bos1-7.7h",    0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "bos1-1.1d",    0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "bos1-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "bos1_9.5n",    0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "bos1_10.5m",   0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "bos1_11.5k",   0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )
ROM_END

/*
    Bosconian - Midway Version

    CPU/Sound Board: A084-91412-B550
    Video Board:     A084-91413-B550
*/

ROM_START( boscomd )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "3n",       0x0000, 0x1000, CRC(441b501a) SHA1(7b4921ff40b3c56950fd32aa0ec5563b02a00929) )
	ROM_LOAD( "3m",       0x1000, 0x1000, CRC(a3c5c7ef) SHA1(70a095a8dbca857245a70404f803916f519e0cbc) )
	ROM_LOAD( "3l",       0x2000, 0x1000, CRC(6ca9a0cf) SHA1(8f70e29beae921e63cd65689a618ca678dd14614) )
	ROM_LOAD( "3k",       0x3000, 0x1000, CRC(d83bacc5) SHA1(cf2fbfa81dabb9b6bcf436d61992e705723776fb) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "3j",       0x0000, 0x1000, CRC(4374e39a) SHA1(7571fd5961f49a0e9ba4301ddd0aca52e94e2f8b) )
	ROM_LOAD( "3h",       0x1000, 0x1000, CRC(04e9fcef) SHA1(2115a9718d511854848704e2693f9efa1c80a307) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "2900.3e",      0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5300.5d",      0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "5200.5e",      0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "prom.2d",      0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bosco.6b",     0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bosco.4m",     0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "prom.2r",      0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "prom.7h",      0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom.1d",      0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "prom.5c",      0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "4900.5n",      0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "5000.5m",      0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "5100.5l",      0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )

	ROM_REGION( 0x0001, "pal_vidbd", 0 ) /* PAL located on the video board */
	ROM_LOAD( "0066-005xx-xxqx.5a", 0x00000, 0x00001, NO_DUMP ) /* According to the manual it's a PAL. What type is unknown. */
ROM_END

ROM_START( boscomdo )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "2300.3n",      0x0000, 0x1000, CRC(db6128b0) SHA1(ddd285f7e00d5e58ab9b15838528e0020d47fcd2) )
	ROM_LOAD( "2400.3m",      0x1000, 0x1000, CRC(86907614) SHA1(3295ab6c5171a069875c2239b3325296c1df6031) )
	ROM_LOAD( "2500.3l",      0x2000, 0x1000, CRC(a21fae11) SHA1(dff38d90ee30558274d2d399edc3281c2ef5cb69) )
	ROM_LOAD( "2600.3k",      0x3000, 0x1000, CRC(11d6ae23) SHA1(f2f72f5c777b684f7ffd53b9c034560211113499) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "2700.3j",      0x0000, 0x1000, CRC(7254e65e) SHA1(c2ee29fcb5173e8d46a80a8a1b931a53dbdeae66) )
	ROM_LOAD( "2800.3h",      0x1000, 0x1000, CRC(31b8c648) SHA1(de0db24d385d2361ec989bf32388df8202ad535c) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "2900.3e",      0x0000, 0x1000, CRC(d45a4911) SHA1(547236adca9174f5cc0ec05b9649618bb92ba630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5300.5d",      0x0000, 0x1000, CRC(a956d3c5) SHA1(c5a9d7b1f9b4acda8fb9762414e085cb5fb80c9e) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "5200.5e",      0x0000, 0x1000, CRC(e869219c) SHA1(425614cd0642743a82ef9c1aada29774a92203ea) )

	ROM_REGION( 0x0100, "gfx3", 0 )
	ROM_LOAD( "prom.2d",      0x0000, 0x0100, CRC(9b69b543) SHA1(47af3f67e50794e839b74fe61197af2228084efd) )    /* dots */

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "bosco.6b",     0x0000, 0x0020, CRC(d2b96fb0) SHA1(54c100ec9d173d7dd48a453ebed5f625053cb6e0) )    /* palette */
	ROM_LOAD( "bosco.4m",     0x0020, 0x0100, CRC(4e15d59c) SHA1(3542ead6421d169c3569e121ec2be304e108787c) )    /* lookup table */
	ROM_LOAD( "prom.2r",      0x0120, 0x0020, CRC(b88d5ba9) SHA1(7b97a38a540b7ca4b7d9ae338ec38b9b1a337846) )    /* video layout (not used) */
	ROM_LOAD( "prom.7h",      0x0140, 0x0020, CRC(87d61353) SHA1(c7493e52662c921625676a4a4e8cf4371bd938b7) )    /* video timing (not used) */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom.1d",      0x0000, 0x0100, CRC(de2316c6) SHA1(0e55c56046331888d1d3f0d9823d2ceb203e7d3f) )
	ROM_LOAD( "prom.5c",      0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "52xx", 0 ) /* ROMs for digitised speech */
	ROM_LOAD( "4900.5n",      0x0000, 0x1000, CRC(09acc978) SHA1(2b264aaeb6eba70ad91593413dca733990e5467b) )
	ROM_LOAD( "5000.5m",      0x1000, 0x1000, CRC(e571e959) SHA1(9c81d7bec73bc605f7dd9a089171b0f34c4bb09a) )
	ROM_LOAD( "5100.5l",      0x2000, 0x1000, CRC(17ac9511) SHA1(266f3fae90d2fe38d109096d352863a52b379899) )

	ROM_REGION( 0x0001, "pal_vidbd", 0 ) /* PAL located on the video board */
	ROM_LOAD( "0066-005xx-xxqx.5a", 0x00000, 0x00001, NO_DUMP ) /* According to the manual it's a PAL. What type is unknown. */
ROM_END

/**********************************************************************************************
  Galaga & clones
**********************************************************************************************/
/*

Galaga
Namco/Midway, 1982

PCB Layout
----------

Top board

23149611 (23149631
|------------------------------------------|
|                         LM324            |
|          04M_G01.3N                      |
|                       Z80        5400    |
|          04K_G02.3M                      |
|                                          |
|   0600   04J_G03.3L                    |-|
|                                 DSW1   |
|          04H_G04.3K             DSW2   |-|
|                                          |
|   0801   04E_G05.3J   Z80               4|
|                                         4|
|                                         W|
|          *            5100              A|
|   0801                                  Y|
|                                TD62064   |
|          04D_G06.3E   Z80                |
|                                        |-|
|   0801                                 |
|          *                             |-|
|                       0702     VOL       |
|                                    MB3730|
|GG1-1.1D                                  |
|                       GG1-2.5C           |
|       3101                               |
|       3101  4066                18.432MHz|
|------------------------------------------|
Notes:
      GG1-1.1D & GG1-2.5C are PROMs, type MB7052 (equivalent to TBP24S10 and 82S129).
      All other ROMs are 2732 EPROMs (i.e. 04*.*).
      *: Unpopulated sockets

      VSync           : 60.606060Hz
      Z80 clocks (all): 1.536MHz
      5400 clock      : 1.536MHz
      5100 clock      : 1.536MHz

      3101   : 16bytes x4 bit Bipolar SRAM, compatible with 7489, MB461 & AM31L01 (trivia - This was Intel's first product, released in 1969!)
      MB3730 : Sound AMP
      TD62064: Darlington transistor for driving coin counters.
      4066   : Quad Bilateral Switch logic IC, used to mix several sound sources to one output.

      NAMCO customs:
                    0600 (DIP28): Bus Interface IC
                    0801 (DIP28): Multi CPU Bus Controller IC
                    5100 (DIP42): Controls player input, coins, DSW's (custom 4 bit I/O Microcontroller)
                    0702 (DIP28): Sync Generator/Clock Divider IC
                    5400 (DIP28): MUX 4-channel Audio Generator IC. Generates 'death bang'.
                                  This is not a Z80 with swapped pins as many sites have reported.

      Pinouts:
      Galaga PCB edge connector pinouts

      Parts Side    Pin   Pin Solder Side
      ----------------------------------
      Logic Ground   A     1  Logic Ground
      Speaker +      B     2  Speaker -
                     C     3  Coin Counter 1
      P1 Start Lamp  D     4  P2 Start Lamp
      +12            E     5  +12
      +5             F     6  +5
      Ground         H     7  Ground
      Service Credit J     8  Test
      Coin 1         K     9  Coin 2
      Player 1 Start L     10 Player 2 Start
      P1 Fire        M     11 P2 Fire
      P1 Left        N     12 P2 Left
                     P     13
      P1 Right       R     14 P2 Right
                     S     15
                     T     16
                     U     17
                     V     18
                     W     19
                     X     20
      Coin Counter 2 Y     21 Cocktail Mode
      Ground         Z     22 Ground

      Pin21: Ground this pin for cocktail mode


Bottom board

23149612 (23149632
|------------------------------------------|
| 0700     GG1-4.2N           GG1-5.5N     |
|                        *             RGBS|
| 0015                                     |
|                                          |
|              2114                        |
| 6116                               8147  |
|              2114    07M_G08.4L          |
|                                    8147  |
|              2114                        |
| 0400                               8147  |
|              2114                        |
|                                    8147  |
|              2114    0200                |
|                                          |
|              2114                        |
|                      07H_G09.4F    8147  |
|                                          |
|                                    8147  |
|                                          |
|                      07E_G10.4D    8147  |
|                                          |
|                                    8147  |
| GG1-3.1C                                 |
|                                          |
|                                          |
|------------------------------------------|
Notes:
      RGBS: Video output socket (Red, Green Blue, Sync to monitor)
      GG1*  are PROMs, type MB7052 (equivalent to TBP24S10 and 82S129).
      All other ROMs are 2732 EPROMs.
      *: Unpopulated socket

      2114    : 1K x4 SRAM
      6116    : 2K x8 SRAM
      8147    : 4K x1 SRAM (Note - you can remove the eight 8147 RAMs and install two 2148s (1K x 4) in their place at positions 6H and 6B.

      Bootup RAM Errors
      Error Code    Meaning
      RAM OK        All RAMs are good
      RAM 0L        RAM located on Video PC board at position 1K is bad
      RAM 0H        RAM located on Video PC board at position 1K is bad
      RAM 1L        RAM located on Video PC board at position 1K is bad
      RAM 1H        RAM located on Video PC board at position 1K is bad
      RAM 2L        RAM located on Video PC board at position 3E is bad
      RAM 2H        RAM located on Video PC board at position 3F is bad
      RAM 3L        RAM located on Video PC board at position 3K is bad
      RAM 3H        RAM located on Video PC board at position 3L is bad
      RAM 4L        RAM located on Video PC board at position 3H is bad
      RAM 4H        RAM located on Video PC board at position 3J is bad

      Bootup ROM Errors
      Error Code   Meaning
      ROM OK       All ROMs are good
      ROM 01       ROM located on CPU PC board at position 3N is bad
      ROM 02       ROM located on CPU PC board at position 3M is bad
      ROM 03       ROM located on CPU PC board at position 3L is bad
      ROM 04       ROM located on CPU PC board at position 3K is bad
      ROM 11       ROM located on CPU PC board at position 3J is bad
      ROM 21       ROM located on CPU PC board at position 3E is bad

      NAMCO customs:
                    0015 (DIP28): Video RAM addresser IC
                    0200 (DIP28): Graphics ROM Data Custom Shift Register IC
                    0400 (DIP28): Motion Object and Scratch RAM to CPU Bus Interface IC
                    0702 (DIP28): Sync Generator/Clock Divider IC

*/

ROM_START( galaga )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "gg1_1b.3p",    0x0000, 0x1000, CRC(ab036c9f) SHA1(ca7f5da42d4e76fd89bb0b35198a23c01462fbfe) )
	ROM_LOAD( "gg1_2b.3m",    0x1000, 0x1000, CRC(d9232240) SHA1(ab202aa259c3d332ef13dfb8fc8580ce2a5a253d) )
	ROM_LOAD( "gg1_3.2m",     0x2000, 0x1000, CRC(753ce503) SHA1(481f443aea3ed3504ec2f3a6bfcf3cd47e2f8f81) )
	ROM_LOAD( "gg1_4b.2l",    0x3000, 0x1000, CRC(499fcc76) SHA1(ddb8b121903646c320939c7d13f4aa4ebb130378) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "gg1_5b.3f",    0x0000, 0x1000, CRC(bb5caae3) SHA1(e957a581463caac27bc37ca2e2a90f27e4f62b6f) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "gg1_7b.2c",    0x0000, 0x1000, CRC(d016686b) SHA1(44c1a04fba3c7c826ff484185cb881b4b22e6657) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gg1_9.4l",     0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gg1_11.4d",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "gg1_10.4f",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( galagao )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "gg1-1.3p",     0x0000, 0x1000, CRC(a3a0f743) SHA1(6907773db7c002ecde5e41853603d53387c5c7cd) )
	ROM_LOAD( "gg1-2.3m",     0x1000, 0x1000, CRC(43bb0d5c) SHA1(666975aed5ce84f09794c54b550d64d95ab311f0) )
	ROM_LOAD( "gg1-3.2m",     0x2000, 0x1000, CRC(753ce503) SHA1(481f443aea3ed3504ec2f3a6bfcf3cd47e2f8f81) )
	ROM_LOAD( "gg1-4.2l",     0x3000, 0x1000, CRC(83874442) SHA1(366cb0dbd31b787e64f88d182108b670d03b393e) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "gg1-5.3f",     0x0000, 0x1000, CRC(3102fccd) SHA1(d29b68d6aab3217fa2106b3507b9273ff3f927bf) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "gg1-7.2c",     0x0000, 0x1000, CRC(8995088d) SHA1(d6cb439de0718826d1a0363c9d77de8740b18ecf) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gg1-9.4l",     0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gg1-11.4d",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "gg1-10.4f",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( galagamw )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "3200a.bin",    0x0000, 0x1000, CRC(3ef0b053) SHA1(0c04a362b737998c0952a753fb3fd8c8a17e9b46) )
	ROM_LOAD( "3300b.bin",    0x1000, 0x1000, CRC(1b280831) SHA1(f7ea12e61929717ebe43a4198a97f109845a2c62) )
	ROM_LOAD( "3400c.bin",    0x2000, 0x1000, CRC(16233d33) SHA1(a7eb799be5e23058754a92b15e6527bfbb47a354) )
	ROM_LOAD( "3500d.bin",    0x3000, 0x1000, CRC(0aaf5c23) SHA1(3f4b0bb960bf002261e9c1278c88f594c6aa8ab6) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "3600e.bin",    0x0000, 0x1000, CRC(bc556e76) SHA1(0d3d68243c4571d985b4d8f7e0ea9f6fcffa2116) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "3700g.bin",    0x0000, 0x1000, CRC(b07f0aa4) SHA1(7528644a8480d0be2d0d37069515ed319e94778f) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "2600j.bin",    0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "2800l.bin",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "2700k.bin",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( galagamf )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "3200a.bin",    0x0000, 0x1000, CRC(3ef0b053) SHA1(0c04a362b737998c0952a753fb3fd8c8a17e9b46) )
	ROM_LOAD( "3300b.bin",    0x1000, 0x1000, CRC(1b280831) SHA1(f7ea12e61929717ebe43a4198a97f109845a2c62) )
	ROM_LOAD( "3400c.bin",    0x2000, 0x1000, CRC(16233d33) SHA1(a7eb799be5e23058754a92b15e6527bfbb47a354) )
	ROM_LOAD( "3500d.bin",    0x3000, 0x1000, CRC(0aaf5c23) SHA1(3f4b0bb960bf002261e9c1278c88f594c6aa8ab6) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "3600fast.bin", 0x0000, 0x1000, CRC(23d586e5) SHA1(43346c69385e9091e64cff6c027ac2689cafcbb9) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "3700g.bin",    0x0000, 0x1000, CRC(b07f0aa4) SHA1(7528644a8480d0be2d0d37069515ed319e94778f) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "2600j.bin",    0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "2800l.bin",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "2700k.bin",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( galagamk )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "mk2-1",        0x0000, 0x1000, CRC(23cea1e2) SHA1(18db33ade0ca6e47cc48aa151d2ccbb4646e3ae3) )
	ROM_LOAD( "mk2-2",        0x1000, 0x1000, CRC(89695b1a) SHA1(fda5557018884e903f855bf3b69a25d75ed8a767) )
	ROM_LOAD( "3400c.bin",    0x2000, 0x1000, CRC(16233d33) SHA1(a7eb799be5e23058754a92b15e6527bfbb47a354) )
	ROM_LOAD( "mk2-4",        0x3000, 0x1000, CRC(24b767f5) SHA1(d4c03e2ed582cfa7f8168ac352f790ef7af54cb8) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "gg1-5.3f",     0x0000, 0x1000, CRC(3102fccd) SHA1(d29b68d6aab3217fa2106b3507b9273ff3f927bf) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "gg1-7b.2c",    0x0000, 0x1000, CRC(d016686b) SHA1(44c1a04fba3c7c826ff484185cb881b4b22e6657) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gg1-9.4l",     0x0000, 0x1000, CRC(58b2f47c) SHA1(62f1279a784ab2f8218c4137c7accda00e6a3490) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gg1-11.4d",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "gg1-10.4f",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( gallag )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "gallag.1",     0x0000, 0x1000, CRC(a3a0f743) SHA1(6907773db7c002ecde5e41853603d53387c5c7cd) )
	ROM_LOAD( "gallag.2",     0x1000, 0x1000, CRC(5eda60a7) SHA1(853d7b974dd04abd7af3a8ba2681dfabce4dce18) )
	ROM_LOAD( "gallag.3",     0x2000, 0x1000, CRC(753ce503) SHA1(481f443aea3ed3504ec2f3a6bfcf3cd47e2f8f81) )
	ROM_LOAD( "gallag.4",     0x3000, 0x1000, CRC(83874442) SHA1(366cb0dbd31b787e64f88d182108b670d03b393e) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "gallag.5",     0x0000, 0x1000, CRC(3102fccd) SHA1(d29b68d6aab3217fa2106b3507b9273ff3f927bf) )

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "gallag.7",     0x0000, 0x1000, CRC(8995088d) SHA1(d6cb439de0718826d1a0363c9d77de8740b18ecf) )

	ROM_REGION( 0x10000, "sub3", 0 )    /* 64k for a Z80 which emulates the custom I/O chip (not used) */
	ROM_LOAD( "gallag.6",     0x0000, 0x1000, CRC(001b70bc) SHA1(b465eee91e75257b7b049d49c0064ab5fd66c576) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gallag.8",     0x0000, 0x1000, CRC(169a98a4) SHA1(edbeb11076061e744ea88d9899dbdfe0964c7e78) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gallag.a",    0x0000, 0x1000, CRC(ad447c80) SHA1(e697c180178cabd1d32483c5d8889a40633f7857) )
	ROM_LOAD( "gallag.9",    0x1000, 0x1000, CRC(dd6f1afc) SHA1(c340ed8c25e0979629a9a1730edc762bd72d0cff) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( gatsbee )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code for the first CPU  */
	ROM_LOAD( "1.4b",         0x0000, 0x1000, CRC(9fb8e28b) SHA1(7171e3fb37b0d6cc8f7a023c1775080d5986de99) )
	ROM_LOAD( "2.4c",         0x1000, 0x1000, CRC(bf6cb840) SHA1(5763140d32d35a38cdcb49e6de1fd5b07a9e8cc2) )
	ROM_LOAD( "3.4d",         0x2000, 0x1000, CRC(3604e2dd) SHA1(1736cf8497f7ac28e92ca94fa137c144353dc192) )
	ROM_LOAD( "4.4e",         0x3000, 0x1000, CRC(bf9f613b) SHA1(41c852fc77f0f35bf48a5b81a19234ed99871c89) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "gg1-5.3f",     0x0000, 0x1000, CRC(3102fccd) SHA1(d29b68d6aab3217fa2106b3507b9273ff3f927bf) )    // 5.4j

	ROM_REGION( 0x10000, "sub2", 0 )     /* 64k for the third CPU  */
	ROM_LOAD( "gg1-7.2c",     0x0000, 0x1000, CRC(8995088d) SHA1(d6cb439de0718826d1a0363c9d77de8740b18ecf) )    // 7.4k

	ROM_REGION( 0x10000, "sub3", 0 )    /* 64k for a Z80 which emulates the custom I/O chip (not used) */
	ROM_LOAD( "gallag.6",     0x0000, 0x1000, CRC(001b70bc) SHA1(b465eee91e75257b7b049d49c0064ab5fd66c576) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "8.5r",  0x0000, 0x2000, CRC(b324f650) SHA1(7bcb254f7cf03bd84291b9fdc27b8962b3e12aa4) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "9.6a",         0x0000, 0x1000, CRC(22e339d5) SHA1(9ac2887ede802d28daa4ad0a0a54bcf7b1155a2e) )
	ROM_LOAD( "10.7a",        0x1000, 0x1000, CRC(60dcf940) SHA1(6530aa5b4afef4a8422ece76a93d0c5b1d93355e) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom-5.5n",    0x0000, 0x0020, CRC(54603c6b) SHA1(1a6dea13b4af155d9cb5b999a75d4f1eb9c71346) )    /* palette */
	ROM_LOAD( "prom-4.2n",    0x0020, 0x0100, CRC(59b6edab) SHA1(0281de86c236c88739297ff712e0a4f5c8bf8ab9) )    /* char lookup table */
	ROM_LOAD( "prom-3.1c",    0x0120, 0x0100, CRC(4a04bb6b) SHA1(cdd4bc1013f5c11984fdc4fd10e2d2e27120c1e5) )    /* sprite lookup table */

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "prom-1.1d",    0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "prom-2.5c",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

/**********************************************************************************************
  Xevious & clones
**********************************************************************************************/

/*
    Xevious - Namco Version

    Single/Dual Board?
*/

ROM_START( xevious )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "xvi_1.3p",     0x0000, 0x1000, CRC(09964dda) SHA1(4882b25b0938a903f3a367455ba788a30759b5b0) )
	ROM_LOAD( "xvi_2.3m",     0x1000, 0x1000, CRC(60ecce84) SHA1(8adc60a5fcbca74092518dbc570ffff0f04c5b17) )
	ROM_LOAD( "xvi_3.2m",     0x2000, 0x1000, CRC(79754b7d) SHA1(c6a154858716e1f073b476824b183de20e06d093) )
	ROM_LOAD( "xvi_4.2l",     0x3000, 0x1000, CRC(c7d4bbf0) SHA1(4b846de204d08651253d3a141677c8a31626af07) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "xvi_5.3f",     0x0000, 0x1000, CRC(c85b703f) SHA1(15f1c005b9d806a384ab1f2240b9c580bfe83893) )
	ROM_LOAD( "xvi_6.3j",     0x1000, 0x1000, CRC(e18cdaad) SHA1(6b79efee1a9642edb9f752101737132401248aed) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "xvi_7.2c",     0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "xvi_12.3b",    0x0000, 0x1000, CRC(088c8b26) SHA1(9c3b61dfca2f84673a78f7f66e363777a8f47a59) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "xvi_13.3c",    0x0000, 0x1000, CRC(de60ba25) SHA1(32bc09be5ff8b52ee3a26e0ac3ebc2d4107badb7) )    /* bg pattern B0 */
	ROM_LOAD( "xvi_14.3d",    0x1000, 0x1000, CRC(535cdbbc) SHA1(fb9ffe5fc43e0213231267e98d605d43c15f61e8) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 )
	ROM_LOAD( "xvi_15.4m",    0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "xvi_17.4p",    0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "xvi_16.4n",    0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "xvi_18.4r",    0x5000, 0x2000, CRC(02417d19) SHA1(b5f830dd2cf25cf154308d2e640f0ecdcda5d8cd) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                 0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "xvi_9.2a",     0x0000, 0x1000, CRC(57ed9879) SHA1(3106d1aacff06cf78371bd19967141072b32b7d7) )
	ROM_LOAD( "xvi_10.2b",    0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "xvi_11.2c",    0x3000, 0x1000, CRC(31e244dd) SHA1(3f7eac12863697a98e1122111801606759e44b2a) )

	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "xvi-8.6a",     0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "xvi-9.6d",     0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "xvi-10.6e",    0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "xvi-7.4h",     0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "xvi-6.4f",     0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "xvi-4.3l",     0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "xvi-5.3m",     0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "pals_vidbd", 0) /* PAL's located on the video board */
	ROM_LOAD( "XVI-3.1F",     0x0000, 0x0117, CRC(9192d57a) SHA1(5f36db93b6083767f93aa3a0e4bc2d4fc7e27f9c) ) /* N82S153N */

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "xvi-2.7n",     0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "xvi-1.5n",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

/*
    Xevious - Atari Version

    CPU/Sound Board: A039785
    Video Board:     A039787
*/

ROM_START( xeviousa )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "xea-1m-a.bin", 0x0000, 0x2000, CRC(8c2b50ec) SHA1(f770873b711d838556dde67a8aac8a7f572fcc5b) )
	ROM_LOAD( "xea-1l-a.bin", 0x2000, 0x2000, CRC(0821642b) SHA1(c6c322c61d0985a2ac59f5e92d4e351107afb9eb) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "xea-4c-a.bin", 0x0000, 0x2000, CRC(14d8fa03) SHA1(e8114141394adda86184b146f2497cfeef7fc2eb) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "xvi_7.2c",     0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "xvi_12.3b",    0x0000, 0x1000, CRC(088c8b26) SHA1(9c3b61dfca2f84673a78f7f66e363777a8f47a59) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "xvi_13.3c",    0x0000, 0x1000, CRC(de60ba25) SHA1(32bc09be5ff8b52ee3a26e0ac3ebc2d4107badb7) )    /* bg pattern B0 */
	ROM_LOAD( "xvi_14.3d",    0x1000, 0x1000, CRC(535cdbbc) SHA1(fb9ffe5fc43e0213231267e98d605d43c15f61e8) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 )
	ROM_LOAD( "xvi_15.4m",    0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "xvi_17.4p",    0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "xvi_16.4n",    0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "xvi_18.4r",    0x5000, 0x2000, CRC(02417d19) SHA1(b5f830dd2cf25cf154308d2e640f0ecdcda5d8cd) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                 0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "xvi_9.2a",     0x0000, 0x1000, CRC(57ed9879) SHA1(3106d1aacff06cf78371bd19967141072b32b7d7) )
	ROM_LOAD( "xvi_10.2b",    0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "xvi_11.2c",    0x3000, 0x1000, CRC(31e244dd) SHA1(3f7eac12863697a98e1122111801606759e44b2a) )

	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "xvi-8.6a",     0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "xvi-9.6d",     0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "xvi-10.6e",    0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "xvi-7.4h",     0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "xvi-6.4f",     0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "xvi-4.3l",     0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "xvi-5.3m",     0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "pals_vidbd", 0) /* PAL's located on the video board */
	ROM_LOAD( "XVI-3.1F",     0x0000, 0x0117, CRC(9192d57a) SHA1(5f36db93b6083767f93aa3a0e4bc2d4fc7e27f9c) ) /* N82S153N - 137294-001*/

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "xvi-2.7n",     0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "xvi-1.5n",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( xeviousb )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "1m.bin",       0x0000, 0x2000, CRC(e82a22f6) SHA1(6fd09a7fb263cda3d5268cc6d7bfe71a57ac4b47) )
	ROM_LOAD( "1l.bin",       0x2000, 0x2000, CRC(13831df9) SHA1(a7892d1d98868a83a5d1092976873b82577e9e94) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "4c.bin",       0x0000, 0x2000, CRC(827e7747) SHA1(d22645d71b164613834336e26e6942506a0e7eaa) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "xvi_7.2c",     0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "xvi_12.3b",    0x0000, 0x1000, CRC(088c8b26) SHA1(9c3b61dfca2f84673a78f7f66e363777a8f47a59) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "xvi_13.3c",    0x0000, 0x1000, CRC(de60ba25) SHA1(32bc09be5ff8b52ee3a26e0ac3ebc2d4107badb7) )    /* bg pattern B0 */
	ROM_LOAD( "xvi_14.3d",    0x1000, 0x1000, CRC(535cdbbc) SHA1(fb9ffe5fc43e0213231267e98d605d43c15f61e8) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 )
	ROM_LOAD( "xvi_15.4m",    0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "xvi_17.4p",    0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "xvi_16.4n",    0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "xvi_18.4r",    0x5000, 0x2000, CRC(02417d19) SHA1(b5f830dd2cf25cf154308d2e640f0ecdcda5d8cd) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                 0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "xvi_9.2a",     0x0000, 0x1000, CRC(57ed9879) SHA1(3106d1aacff06cf78371bd19967141072b32b7d7) )
	ROM_LOAD( "xvi_10.2b",    0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "xvi_11.2c",    0x3000, 0x1000, CRC(31e244dd) SHA1(3f7eac12863697a98e1122111801606759e44b2a) )

	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "xvi-8.6a",     0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "xvi-9.6d",     0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "xvi-10.6e",    0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "xvi-7.4h",     0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "xvi-6.4f",     0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "xvi-4.3l",     0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "xvi-5.3m",     0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "pals_vidbd", 0) /* PAL's located on the video board */
	ROM_LOAD( "XVI-3.1F",     0x0000, 0x0117, CRC(9192d57a) SHA1(5f36db93b6083767f93aa3a0e4bc2d4fc7e27f9c) ) /* N82S153N - 137294-001*/

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "xvi-2.7n",     0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "xvi-1.5n",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( xeviousc )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "xvi_u_.3p",    0x0000, 0x1000, CRC(7b203868) SHA1(3bafaa42bccddfaf8d9197e93416a731b7f8fb94) )
	ROM_LOAD( "xv_2-2.3m",    0x1000, 0x1000, CRC(b6fe738e) SHA1(23cdf1f2c2642f9bc3f843b5c338372027032380) )
	ROM_LOAD( "xv_2-3.2m",    0x2000, 0x1000, CRC(dbd52ff5) SHA1(eb42393720fc1fd4a1f6cdba87ac4177fd5827fe) )
	ROM_LOAD( "xvi_u_.2l",    0x3000, 0x1000, CRC(ad12af53) SHA1(ff3a96d6f7357fb2d33cd9d77d53477b9071ffc9) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "xv2_5.3f",     0x0000, 0x1000, CRC(f8cc2861) SHA1(9b02c00cff6c771d46776416295f9e12a2166cc5) )
	ROM_LOAD( "xvi_6.3j",     0x1000, 0x1000, CRC(e18cdaad) SHA1(6b79efee1a9642edb9f752101737132401248aed) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "xvi_7.2c",     0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "xvi_12.3b",    0x0000, 0x1000, CRC(088c8b26) SHA1(9c3b61dfca2f84673a78f7f66e363777a8f47a59) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "xvi_13.3c",    0x0000, 0x1000, CRC(de60ba25) SHA1(32bc09be5ff8b52ee3a26e0ac3ebc2d4107badb7) )    /* bg pattern B0 */
	ROM_LOAD( "xvi_14.3d",    0x1000, 0x1000, CRC(535cdbbc) SHA1(fb9ffe5fc43e0213231267e98d605d43c15f61e8) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 )
	ROM_LOAD( "xvi_15.4m",    0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "xvi_17.4p",    0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "xvi_16.4n",    0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "xvi_18.4r",    0x5000, 0x2000, CRC(02417d19) SHA1(b5f830dd2cf25cf154308d2e640f0ecdcda5d8cd) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                 0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "xvi_9.2a",     0x0000, 0x1000, CRC(57ed9879) SHA1(3106d1aacff06cf78371bd19967141072b32b7d7) )
	ROM_LOAD( "xvi_10.2b",    0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "xvi_11.2c",    0x3000, 0x1000, CRC(31e244dd) SHA1(3f7eac12863697a98e1122111801606759e44b2a) )

	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "xvi-8.6a",     0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "xvi-9.6d",     0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "xvi-10.6e",    0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "xvi-7.4h",     0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "xvi-6.4f",     0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "xvi-4.3l",     0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "xvi-5.3m",     0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "pals_vidbd", 0) /* PAL's located on the video board */
	ROM_LOAD( "XVI-3.1F",     0x0000, 0x0117, CRC(9192d57a) SHA1(5f36db93b6083767f93aa3a0e4bc2d4fc7e27f9c) ) /* N82S153N - 137294-001*/

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "xvi-2.7n",     0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "xvi-1.5n",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

/*
    Xevious Bootleg

    Dual Boards with no markings except row/column designations
*/

ROM_START( xevios )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "4.7h",         0x0000, 0x1000, CRC(1f8ca4c0) SHA1(9fdaa2e0016c07e274544f8334778fe81b8344a5) )
	ROM_LOAD( "5.6h",         0x1000, 0x1000, CRC(2e47ce8f) SHA1(fb35dd086e98279a5f17036f624ef5294c777d84) )
	ROM_LOAD( "6.5h",         0x2000, 0x1000, CRC(79754b7d) SHA1(c6a154858716e1f073b476824b183de20e06d093) )
	ROM_LOAD( "7.4h",         0x3000, 0x1000, CRC(17f48277) SHA1(ffe590acf07985355ef91fbe0fc3dcf6e8fd62fd) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "8.2h",         0x0000, 0x1000, CRC(c85b703f) SHA1(15f1c005b9d806a384ab1f2240b9c580bfe83893) )
	ROM_LOAD( "9.1h",         0x1000, 0x1000, CRC(e18cdaad) SHA1(6b79efee1a9642edb9f752101737132401248aed) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "3.9h",         0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "17.8f",        0x0000, 0x1000, CRC(088c8b26) SHA1(9c3b61dfca2f84673a78f7f66e363777a8f47a59) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "18.9f",        0x0000, 0x1000, CRC(de60ba25) SHA1(32bc09be5ff8b52ee3a26e0ac3ebc2d4107badb7) )    /* bg pattern B0 */
	ROM_LOAD( "19.11f",       0x1000, 0x1000, CRC(535cdbbc) SHA1(fb9ffe5fc43e0213231267e98d605d43c15f61e8) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 )
	ROM_LOAD( "13.4d",        0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "15.7d",        0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "14.6d",        0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "16.8d",        0x5000, 0x2000, CRC(44262c04) SHA1(4291f83193d11064c2ba6a9af27951b93bb945c3) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                 0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "10.1d",        0x0000, 0x1000, CRC(10baeebb) SHA1(c544c9e0bb7a1ef93b3f2c2c1397f659d5334373) )
	ROM_LOAD( "11.2d",        0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "12.3d",        0x3000, 0x1000, CRC(51a4e83b) SHA1(fbf3b1e47b75c5e0b297ee2cd6597b1dfd80bc6f) )

	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "8.12h",        0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "9.13h",        0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "10.14h",       0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "7.14f",        0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "6.13f",        0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "6.5c",         0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "5.6c",         0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "1.10f",        0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "2.10c",        0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x3000, "user1", 0 ) /* unknown roms */
	/* extra ROMs (function unknown, could be emulation of the custom I/O */
	/* chip with a Z80): */
	ROM_LOAD( "1.16j",        0x0000, 0x1000, CRC(2618f0ce) SHA1(54e8644b5609d6f6ec717a7469c76901eb79f26e) )
	ROM_LOAD( "2.17b",        0x1000, 0x2000, CRC(de359fac) SHA1(a55df9984bfffafeadae8a5a63b07f1fa9c5eebf) )

	ROM_REGION( 0x002c, "pals", 0 ) /* Located on the video board */
	ROM_LOAD( "pal10l8.16a.bin", 0x0000, 0x002c, CRC(6fb9bd9a) SHA1(698b5fc19f5873b02a4bed7d9ec1f24763a6fef7) )
ROM_END

/*
    Battles (Xevious Bootleg)

    Three Boards with no markings except row/column designations
*/

ROM_START( battles )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU (Located on the top board) */
	ROM_LOAD( "bg1.d9",      0x0000, 0x2000, CRC(b6e4f4f3) SHA1(ceaaa63b50e75dcb05aeb68574336dfe56a8434a) )
	ROM_LOAD( "bg2.d10",     0x2000, 0x2000, CRC(47017bc8) SHA1(0da73ae079fb6a64eed56197e2c88609ef34166c) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU (Located on the top board) */
	ROM_LOAD( "bg3.d12",     0x0000, 0x2000, CRC(0ede5706) SHA1(65b235c5abe487612e11d0235410f1ca59b06e95) )

	ROM_REGION( 0x10000, "sub2", 0 ) /* (Located on the top board) */
	ROM_LOAD( "bg4.d13",     0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x10000, "sub3", 0 ) /* 64k for the CUSTOM I/O Emulation CPU (Located on the top board) */
	ROM_LOAD( "bg5.h5",      0x0000, 0x1000, CRC(23107dfb) SHA1(74c49a5648faab632ae5ed8dd18a1d8b39837e2d) )

	ROM_REGION( 0x1000, "gfx1", 0 ) /* (Located on the middle board) */
	ROM_LOAD( "bg9.c10",     0x0000, 0x1000, CRC(5bd6e9ae) SHA1(f16c7eec39fce856c775b2b81ab55fb42376850e) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 ) /* (Located on the middle board) */
	ROM_LOAD( "bg10.c8",     0x0000, 0x1000, CRC(b43ea55d) SHA1(06f4c4e7fc71b9e173c3bdf91c40f47750051b5e) )    /* bg pattern B0 */
	ROM_LOAD( "bg11.c7",     0x1000, 0x1000, CRC(73603931) SHA1(1f7824b107a5a3d5c3434f02f17173a1f85fd29c) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 ) /* (Located on the bottom board) */
	ROM_LOAD( "bg13.c6",    0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "bg14.c4",    0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "bg12.c8",    0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "bg15.c2",    0x5000, 0x2000, CRC(02417d19) SHA1(b5f830dd2cf25cf154308d2e640f0ecdcda5d8cd) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps (Located on the middle board) */
	ROM_LOAD( "bg6.b14",     0x0000, 0x1000, CRC(57ed9879) SHA1(3106d1aacff06cf78371bd19967141072b32b7d7) )
	ROM_LOAD( "bg7.b12",     0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "bg8.b11",     0x3000, 0x1000, CRC(31e244dd) SHA1(3f7eac12863697a98e1122111801606759e44b2a) )

	ROM_REGION( 0x1400, "proms", 0 )
	ROM_LOAD( "xvi-8.6a",    0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "xvi-9.6d",    0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "xvi-10.6e",   0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "b_-bpr.bin",  0x0300, 0x0400, CRC(d2d208b1) SHA1(6c8d29912c03ee93759e24085bc66ab738768bcc) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "b_6bpr.bin",  0x0700, 0x0400, CRC(0260c041) SHA1(1a7516e8b18ffdd9789eec8b834c17b3ba312afe) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "b_4bpr.bin",  0x0b00, 0x0400, CRC(33764974) SHA1(567b048b8a93e30090ccee4f6aadc0353524d8d1) ) /* sprite lookup table low bits */
	ROM_LOAD( "b_5bpr.bin",  0x0f00, 0x0400, CRC(43674c7e) SHA1(94c19a9da81839cb1dfde3f11b2fd82ffe45efb9) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "xvi-2.7n",    0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "xvi-1.5n",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( sxevious )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "cpu_3p.rom",   0x0000, 0x1000, CRC(1c8d27d5) SHA1(2c41303d8c74acb5840295a4b460a39a9a8e21bb) )
	ROM_LOAD( "cpu_3m.rom",   0x1000, 0x1000, CRC(fd04e615) SHA1(7169e7f3bd1e9cfae9671b89f2a45f56b968e1ff) )
	ROM_LOAD( "xv3_3.2m",     0x2000, 0x1000, CRC(294d5404) SHA1(ecc39fb2c0065a36f20541747089b4e30dfb99b1) )
	ROM_LOAD( "xv3_4.2l",     0x3000, 0x1000, CRC(6a44bf92) SHA1(0ca726f7f9528789f2a718df55e59406a283cdfa) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "xv3_5.3f",     0x0000, 0x1000, CRC(d4bd3d81) SHA1(5831bb306bd650779207936bfd00f25864733abb) )
	ROM_LOAD( "xv3_6.3j",     0x1000, 0x1000, CRC(af06be5f) SHA1(5a020822387ab8c69214db961180760fa9853e6e) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "xvi_7.2c",     0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "xvi_12.3b",    0x0000, 0x1000, CRC(088c8b26) SHA1(9c3b61dfca2f84673a78f7f66e363777a8f47a59) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "xvi_13.3c",    0x0000, 0x1000, CRC(de60ba25) SHA1(32bc09be5ff8b52ee3a26e0ac3ebc2d4107badb7) )    /* bg pattern B0 */
	ROM_LOAD( "xvi_14.3d",    0x1000, 0x1000, CRC(535cdbbc) SHA1(fb9ffe5fc43e0213231267e98d605d43c15f61e8) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 )
	ROM_LOAD( "xvi_15.4m",    0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "xvi_17.4p",    0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "xvi_16.4n",    0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "xvi_18.4r",    0x5000, 0x2000, CRC(02417d19) SHA1(b5f830dd2cf25cf154308d2e640f0ecdcda5d8cd) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                 0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "xvi_9.2a",     0x0000, 0x1000, CRC(57ed9879) SHA1(3106d1aacff06cf78371bd19967141072b32b7d7) )
	ROM_LOAD( "xvi_10.2b",    0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "xvi_11.2c",    0x3000, 0x1000, CRC(31e244dd) SHA1(3f7eac12863697a98e1122111801606759e44b2a) )

	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "xvi-8.6a",     0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "xvi-9.6d",     0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "xvi-10.6e",    0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "xvi-7.4h",     0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "xvi-6.4f",     0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "xvi-4.3l",     0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "xvi-5.3m",     0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "xvi-2.7n",     0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "xvi-1.5n",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( sxeviousj )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for the first CPU */
	ROM_LOAD( "xv3_1.3p",     0x0000, 0x1000, CRC(afbc3372) SHA1(9001856aad0f31b40443f21b7a895e4101684307) )
	ROM_LOAD( "xv3_2.3m",     0x1000, 0x1000, CRC(1854a5ee) SHA1(2fb4034d9d757376df59378df539bf41d99ed43e) )
	ROM_LOAD( "xv3_3.2m",     0x2000, 0x1000, CRC(294d5404) SHA1(ecc39fb2c0065a36f20541747089b4e30dfb99b1) )
	ROM_LOAD( "xv3_4.2l",     0x3000, 0x1000, CRC(6a44bf92) SHA1(0ca726f7f9528789f2a718df55e59406a283cdfa) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "xv3_5.3f",     0x0000, 0x1000, CRC(d4bd3d81) SHA1(5831bb306bd650779207936bfd00f25864733abb) )
	ROM_LOAD( "xv3_6.3j",     0x1000, 0x1000, CRC(af06be5f) SHA1(5a020822387ab8c69214db961180760fa9853e6e) )

	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD( "xvi_7.2c",     0x0000, 0x1000, CRC(dd35cf1c) SHA1(f8d1f8e019d8198308443c2e7e815d0d04b23d14) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "xvi_12.3b",    0x0000, 0x1000, CRC(088c8b26) SHA1(9c3b61dfca2f84673a78f7f66e363777a8f47a59) )    /* foreground characters */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "xvi_13.3c",    0x0000, 0x1000, CRC(de60ba25) SHA1(32bc09be5ff8b52ee3a26e0ac3ebc2d4107badb7) )    /* bg pattern B0 */
	ROM_LOAD( "xvi_14.3d",    0x1000, 0x1000, CRC(535cdbbc) SHA1(fb9ffe5fc43e0213231267e98d605d43c15f61e8) )    /* bg pattern B1 */

	ROM_REGION( 0xa000, "gfx3", 0 )
	ROM_LOAD( "xvi_15.4m",    0x0000, 0x2000, CRC(dc2c0ecb) SHA1(19ddbd9805f77f38c9a9a1bb30dba6c720b8609f) )    /* sprite set #1, planes 0/1 */
	ROM_LOAD( "xvi_17.4p",    0x2000, 0x2000, CRC(dfb587ce) SHA1(acff2bf5cde85a16cdc98a52cdea11f77fadf25a) )    /* sprite set #2, planes 0/1 */
	ROM_LOAD( "xvi_16.4n",    0x4000, 0x1000, CRC(605ca889) SHA1(3bf380ef76c03822a042ecc73b5edd4543c268ce) )    /* sprite set #3, planes 0/1 */
	ROM_LOAD( "xvi_18.4r",    0x5000, 0x2000, CRC(02417d19) SHA1(b5f830dd2cf25cf154308d2e640f0ecdcda5d8cd) )    /* sprite set #1, plane 2, set #2, plane 2 */
	/* 0x7000-0x8fff  will be unpacked from 0x5000-0x6fff */
	ROM_FILL(                 0x9000, 0x1000, 0x00 )    // empty space to decode sprite set #3 as 3 bits per pixel

	ROM_REGION( 0x4000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "xvi_9.2a",     0x0000, 0x1000, CRC(57ed9879) SHA1(3106d1aacff06cf78371bd19967141072b32b7d7) )
	ROM_LOAD( "xvi_10.2b",    0x1000, 0x2000, CRC(ae3ba9e5) SHA1(49064b25667ffcd81137cd5e800df4b78b182a46) )
	ROM_LOAD( "xvi_11.2c",    0x3000, 0x1000, CRC(31e244dd) SHA1(3f7eac12863697a98e1122111801606759e44b2a) )

	ROM_REGION( 0x0b00, "proms", 0 )
	ROM_LOAD( "xvi-8.6a",     0x0000, 0x0100, CRC(5cc2727f) SHA1(0dc1e63a47a4cb0ba75f6f1e0c15e408bb0ee2a1) ) /* palette red component */
	ROM_LOAD( "xvi-9.6d",     0x0100, 0x0100, CRC(5c8796cc) SHA1(63015e3c0874afc6b1ca032f1ffb8f90562c77c8) ) /* palette green component */
	ROM_LOAD( "xvi-10.6e",    0x0200, 0x0100, CRC(3cb60975) SHA1(c94d5a5dd4d8a08d6d39c051a4a722581b903f45) ) /* palette blue component */
	ROM_LOAD( "xvi-7.4h",     0x0300, 0x0200, CRC(22d98032) SHA1(ec6626828c79350417d08b98e9631ad35edd4a41) ) /* bg tiles lookup table low bits */
	ROM_LOAD( "xvi-6.4f",     0x0500, 0x0200, CRC(3a7599f0) SHA1(a4bdf58c190ca16fc7b976c97f41087a61fdb8b8) ) /* bg tiles lookup table high bits */
	ROM_LOAD( "xvi-4.3l",     0x0700, 0x0200, CRC(fd8b9d91) SHA1(87ddf0b9d723aabb422d6d416aa9ec6bc246bf34) ) /* sprite lookup table low bits */
	ROM_LOAD( "xvi-5.3m",     0x0900, 0x0200, CRC(bf906d82) SHA1(776168a73d3b9f0ce05610acc8a623deae0a572b) ) /* sprite lookup table high bits */

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "xvi-2.7n",     0x0000, 0x0100, CRC(550f06bc) SHA1(816a0fafa0b084ac11ae1af70a5186539376fc2a) )
	ROM_LOAD( "xvi-1.5n",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

/**********************************************************************************************
  Dig Dug & clones
**********************************************************************************************/

ROM_START( digdug )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "dd1a.1",       0x0000, 0x1000, CRC(a80ec984) SHA1(86689980410b9429cd7582c7a76342721c87d030) )
	ROM_LOAD( "dd1a.2",       0x1000, 0x1000, CRC(559f00bd) SHA1(fde17785df21956d6fd06bcfe675c392dadb1524) )
	ROM_LOAD( "dd1a.3",       0x2000, 0x1000, CRC(8cbc6fe1) SHA1(57b8a5777f8bb9773caf0cafe5408c8b9768cb25) )
	ROM_LOAD( "dd1a.4",       0x3000, 0x1000, CRC(d066f830) SHA1(b0a615fe4a5c8742c1e4ef234ef34c369d2723b9) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "dd1a.5",       0x0000, 0x1000, CRC(6687933b) SHA1(c16144de7633595ddc1450ddce379f48e7b2195a) )
	ROM_LOAD( "dd1a.6",       0x1000, 0x1000, CRC(843d857f) SHA1(89b2ead7e478e119d33bfd67376cdf28f83de67a) )

	ROM_REGION( 0x10000, "sub2", 0 ) /* 64k for the third CPU  */
	ROM_LOAD( "dd1.7",        0x0000, 0x1000, CRC(a41bce72) SHA1(2b9b74f56aa7939d9d47cf29497ae11f10d78598) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "dd1.9",        0x0000, 0x0800, CRC(f14a6fe1) SHA1(0aa63300c2cb887196de590aceb98f3cf06fead4) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "dd1.15",       0x0000, 0x1000, CRC(e22957c8) SHA1(4700c63f4f680cb8ab8c44e6f3e1712aabd5daa4) )
	ROM_LOAD( "dd1.14",       0x1000, 0x1000, CRC(2829ec99) SHA1(3e435c1afb2e44487cd7ba28a93ada2e5ccbb86d) )
	ROM_LOAD( "dd1.13",       0x2000, 0x1000, CRC(458499e9) SHA1(578bd839f9218c3cf4feee1223a461144e455df8) )
	ROM_LOAD( "dd1.12",       0x3000, 0x1000, CRC(c58252a0) SHA1(bd79e39e8a572d2b5c205e6de27ca23e43ec9f51) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "dd1.11",       0x0000, 0x1000, CRC(7b383983) SHA1(57f1e8f5171d13f9f76bd091d81b4423b59f6b42) )

	ROM_REGION( 0x1000, "gfx4", 0 ) /* 4k for the playfield graphics */
	ROM_LOAD( "dd1.10b",      0x0000, 0x1000, CRC(2cf399c2) SHA1(317c48818992f757b1bd0e3997fa99937f81b52c) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "136007.113",   0x0000, 0x0020, CRC(4cb9da99) SHA1(91a5852a15d4672c29fdcbae75921794651f960c) )
	ROM_LOAD( "136007.111",   0x0020, 0x0100, CRC(00c7c419) SHA1(7ea149e8eb36920c3b84984b5ce623729d492fd3) )
	ROM_LOAD( "136007.112",   0x0120, 0x0100, CRC(e9b3e08e) SHA1(a294cc4da846eb702d61678396bfcbc87d30ea95) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound prom */
	ROM_LOAD( "136007.110",   0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "136007.109",   0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( digdug1 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "dd1.1",        0x0000, 0x1000, CRC(b9198079) SHA1(1d3fe04020f584ed250e32fdc6f6a3b769342884) )
	ROM_LOAD( "dd1.2",        0x1000, 0x1000, CRC(b2acbe49) SHA1(c8f713e8cfa70d3bc64d3002ff7bffc65ee138e2) )
	ROM_LOAD( "dd1.3",        0x2000, 0x1000, CRC(d6407b49) SHA1(0e71a8f02778286488865e20439776dbb2a8ec78) )
	ROM_LOAD( "dd1.4b",       0x3000, 0x1000, CRC(f4cebc16) SHA1(19b568f92069a1cfe1c07287408efe3b0e253375) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "dd1.5b",       0x0000, 0x1000, CRC(370ef9b4) SHA1(746b1fa15f5f2cfd69d8b5a7d6fb8c770abc3b4d) )
	ROM_LOAD( "dd1.6b",       0x1000, 0x1000, CRC(361eeb71) SHA1(372c97c666411c3590d790213ae6fa1ccb5ffa1c) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "dd1.7",        0x0000, 0x1000, CRC(a41bce72) SHA1(2b9b74f56aa7939d9d47cf29497ae11f10d78598) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "dd1.9",        0x0000, 0x0800, CRC(f14a6fe1) SHA1(0aa63300c2cb887196de590aceb98f3cf06fead4) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "dd1.15",       0x0000, 0x1000, CRC(e22957c8) SHA1(4700c63f4f680cb8ab8c44e6f3e1712aabd5daa4) )
	ROM_LOAD( "dd1.14",       0x1000, 0x1000, CRC(2829ec99) SHA1(3e435c1afb2e44487cd7ba28a93ada2e5ccbb86d) )
	ROM_LOAD( "dd1.13",       0x2000, 0x1000, CRC(458499e9) SHA1(578bd839f9218c3cf4feee1223a461144e455df8) )
	ROM_LOAD( "dd1.12",       0x3000, 0x1000, CRC(c58252a0) SHA1(bd79e39e8a572d2b5c205e6de27ca23e43ec9f51) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "dd1.11",       0x0000, 0x1000, CRC(7b383983) SHA1(57f1e8f5171d13f9f76bd091d81b4423b59f6b42) )

	ROM_REGION( 0x1000, "gfx4", 0 ) /* 4k for the playfield graphics */
	ROM_LOAD( "dd1.10b",      0x0000, 0x1000, CRC(2cf399c2) SHA1(317c48818992f757b1bd0e3997fa99937f81b52c) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "136007.113",   0x0000, 0x0020, CRC(4cb9da99) SHA1(91a5852a15d4672c29fdcbae75921794651f960c) )
	ROM_LOAD( "136007.111",   0x0020, 0x0100, CRC(00c7c419) SHA1(7ea149e8eb36920c3b84984b5ce623729d492fd3) )
	ROM_LOAD( "136007.112",   0x0120, 0x0100, CRC(e9b3e08e) SHA1(a294cc4da846eb702d61678396bfcbc87d30ea95) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound prom */
	ROM_LOAD( "136007.110",   0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "136007.109",   0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

/*
    Dig Dug - Atari Version

    There are two revisions of the board and the placement of the components
    are different between the two versions.

    Revision A:
        * The letter "A" is silkscreened in the A10 corner of the board.
        * The 1st, 2nd and 3rd edition TM-203 and SP-203 manuals cover this board.

    Revision B:
        * The letter "B" is silkscreened in the P12 corner (on right side of
          the edge connector).
        * Also, the three Z80's are located on the opposite side of the edge connector and
          they are stacked in a column.  (The Z80's are oriented vertically instead of
          horizontal as the other chips are.)
        * The 4th edition TM-203 and SP-203 manuals cover this board.
*/

ROM_START( digdugat )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "136007.201",   0x0000, 0x1000, CRC(23d0b1a4) SHA1(a118d55e03a9ccf069f37c7bac2c9044dccd1f5e) )
	ROM_LOAD( "136007.202",   0x1000, 0x1000, CRC(5453dc1f) SHA1(8be091dd53e9b44e80e1ac9b1751efbe832db78d) )
	ROM_LOAD( "136007.203",   0x2000, 0x1000, CRC(c9077dfa) SHA1(611b3e1b575a51639530917366557773534c80aa) )
	ROM_LOAD( "136007.204",   0x3000, 0x1000, CRC(a8fc8eac) SHA1(7a24197f4ec5989bc4d635b27b6578f4d62cb5f4) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "136007.205",   0x0000, 0x1000, CRC(5ba385c5) SHA1(f4577bddff74a14b13b212f5553fa13fe9ae4bcc) )
	ROM_LOAD( "136007.206",   0x1000, 0x1000, CRC(382b4011) SHA1(2b79ddcf48177c99b5fa1f957374f4baa2bec143) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "136007.107",   0x0000, 0x1000, CRC(a41bce72) SHA1(2b9b74f56aa7939d9d47cf29497ae11f10d78598) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "136007.108",   0x0000, 0x0800, CRC(3d24a3af) SHA1(857ae93e2a41258a129dcecbaed2df359540b735) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "136007.116",   0x0000, 0x1000, CRC(e22957c8) SHA1(4700c63f4f680cb8ab8c44e6f3e1712aabd5daa4) )
	ROM_LOAD( "136007.117",   0x1000, 0x1000, CRC(a3bbfd85) SHA1(2105455762e0de120f2d943f9010a7d06c6b6448) )
	ROM_LOAD( "136007.118",   0x2000, 0x1000, CRC(458499e9) SHA1(578bd839f9218c3cf4feee1223a461144e455df8) )
	ROM_LOAD( "136007.119",   0x3000, 0x1000, CRC(c58252a0) SHA1(bd79e39e8a572d2b5c205e6de27ca23e43ec9f51) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "136007.115",   0x0000, 0x1000, CRC(754539be) SHA1(466ae754eb4721df8814d4d33a31d867507d45b3) )

	ROM_REGION( 0x1000, "gfx4", 0 ) /* 4k for the playfield graphics */
	ROM_LOAD( "136007.114",   0x0000, 0x1000, CRC(d6822397) SHA1(055ca6514141323f1e6dfcf91451507c04114d41) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "136007.113",   0x0000, 0x0020, CRC(4cb9da99) SHA1(91a5852a15d4672c29fdcbae75921794651f960c) )
	ROM_LOAD( "136007.111",   0x0020, 0x0100, CRC(00c7c419) SHA1(7ea149e8eb36920c3b84984b5ce623729d492fd3) )
	ROM_LOAD( "136007.112",   0x0120, 0x0100, CRC(e9b3e08e) SHA1(a294cc4da846eb702d61678396bfcbc87d30ea95) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound prom */
	ROM_LOAD( "136007.110",   0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "136007.109",   0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( digdugat1 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "136007.101",   0x0000, 0x1000, CRC(b9198079) SHA1(1d3fe04020f584ed250e32fdc6f6a3b769342884) )
	ROM_LOAD( "136007.102",   0x1000, 0x1000, CRC(b2acbe49) SHA1(c8f713e8cfa70d3bc64d3002ff7bffc65ee138e2) )
	ROM_LOAD( "136007.103",   0x2000, 0x1000, CRC(d6407b49) SHA1(0e71a8f02778286488865e20439776dbb2a8ec78) )
	ROM_LOAD( "136007.104",   0x3000, 0x1000, CRC(b3ad42c3) SHA1(83ea80f0dd42ec1cb62e6ed45d5dda43ed21f567) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "136007.105",   0x0000, 0x1000, CRC(0a2aef4a) SHA1(ef40974fde8e8c305059e1dd03ea811a6aaca737) )
	ROM_LOAD( "136007.106",   0x1000, 0x1000, CRC(a2876d6e) SHA1(08e8ac50918ae32dd6fb34e65534652beb0395b2) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "136007.107",   0x0000, 0x1000, CRC(a41bce72) SHA1(2b9b74f56aa7939d9d47cf29497ae11f10d78598) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "136007.108",   0x0000, 0x0800, CRC(3d24a3af) SHA1(857ae93e2a41258a129dcecbaed2df359540b735) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "136007.116",   0x0000, 0x1000, CRC(e22957c8) SHA1(4700c63f4f680cb8ab8c44e6f3e1712aabd5daa4) )
	ROM_LOAD( "136007.117",   0x1000, 0x1000, CRC(a3bbfd85) SHA1(2105455762e0de120f2d943f9010a7d06c6b6448) )
	ROM_LOAD( "136007.118",   0x2000, 0x1000, CRC(458499e9) SHA1(578bd839f9218c3cf4feee1223a461144e455df8) )
	ROM_LOAD( "136007.119",   0x3000, 0x1000, CRC(c58252a0) SHA1(bd79e39e8a572d2b5c205e6de27ca23e43ec9f51) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "136007.115",   0x0000, 0x1000, CRC(754539be) SHA1(466ae754eb4721df8814d4d33a31d867507d45b3) )

	ROM_REGION( 0x1000, "gfx4", 0 ) /* 4k for the playfield graphics */
	ROM_LOAD( "136007.114",   0x0000, 0x1000, CRC(d6822397) SHA1(055ca6514141323f1e6dfcf91451507c04114d41) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "136007.113",   0x0000, 0x0020, CRC(4cb9da99) SHA1(91a5852a15d4672c29fdcbae75921794651f960c) )
	ROM_LOAD( "136007.111",   0x0020, 0x0100, CRC(00c7c419) SHA1(7ea149e8eb36920c3b84984b5ce623729d492fd3) )
	ROM_LOAD( "136007.112",   0x0120, 0x0100, CRC(e9b3e08e) SHA1(a294cc4da846eb702d61678396bfcbc87d30ea95) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound prom */
	ROM_LOAD( "136007.110",   0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "136007.109",   0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

/*
    Zig Zag (Dig Dug bootleg)
*/

ROM_START( dzigzag )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "136007.101",   0x0000, 0x1000, CRC(b9198079) SHA1(1d3fe04020f584ed250e32fdc6f6a3b769342884) )
	ROM_LOAD( "136007.102",   0x1000, 0x1000, CRC(b2acbe49) SHA1(c8f713e8cfa70d3bc64d3002ff7bffc65ee138e2) )
	ROM_LOAD( "136007.103",   0x2000, 0x1000, CRC(d6407b49) SHA1(0e71a8f02778286488865e20439776dbb2a8ec78) )
	ROM_LOAD( "zigzag4",      0x3000, 0x1000, CRC(da20d2f6) SHA1(4eafe5ee917060d01d9df92d678c455edbbf27a6) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "zigzag5",      0x0000, 0x2000, CRC(f803c748) SHA1(a4c7dde0b794366cbfd03f339de980a6575a42fc) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "136007.107",   0x0000, 0x1000, CRC(a41bce72) SHA1(2b9b74f56aa7939d9d47cf29497ae11f10d78598) )

	ROM_REGION( 0x10000, "sub3", 0 )    /* 64k for a Z80 which emulates the custom I/O chip (not used) */
	ROM_LOAD( "zigzag7",      0x0000, 0x1000, CRC(24c3510c) SHA1(3214a16f697f88d23f3441e58c56110930d7c341) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "zigzag8",      0x0000, 0x0800, CRC(86120541) SHA1(c974441ee0421a38c25bc7c3edbc6b510b7df473) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "136007.116",   0x0000, 0x1000, CRC(e22957c8) SHA1(4700c63f4f680cb8ab8c44e6f3e1712aabd5daa4) )
	ROM_LOAD( "zigzag12",     0x1000, 0x1000, CRC(386a0956) SHA1(79f5d6af1fdc467a503216a588cb03535c823a40) )
	ROM_LOAD( "zigzag13",     0x2000, 0x1000, CRC(69f6e395) SHA1(10a7518e963f2cecb494d77137e01a068116e20b) )
	ROM_LOAD( "136007.119",   0x3000, 0x1000, CRC(c58252a0) SHA1(bd79e39e8a572d2b5c205e6de27ca23e43ec9f51) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "dd1.11",       0x0000, 0x1000, CRC(7b383983) SHA1(57f1e8f5171d13f9f76bd091d81b4423b59f6b42) )

	ROM_REGION( 0x1000, "gfx4", 0 ) /* 4k for the playfield graphics */
	ROM_LOAD( "dd1.10b",      0x0000, 0x1000, CRC(2cf399c2) SHA1(317c48818992f757b1bd0e3997fa99937f81b52c) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "136007.113",   0x0000, 0x0020, CRC(4cb9da99) SHA1(91a5852a15d4672c29fdcbae75921794651f960c) )
	ROM_LOAD( "136007.111",   0x0020, 0x0100, CRC(00c7c419) SHA1(7ea149e8eb36920c3b84984b5ce623729d492fd3) )
	ROM_LOAD( "136007.112",   0x0120, 0x0100, CRC(e9b3e08e) SHA1(a294cc4da846eb702d61678396bfcbc87d30ea95) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound prom */
	ROM_LOAD( "136007.110",   0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "136007.109",   0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

/*

Year:  1982
Manufacturer:  Sidam

CPUs:

on main PCB (Sidam 11500):

3x MK3880-4IRL-Z80CPU (main)
1x LM324N (sound)
1x TDA2003 (sound)
1x custom 0640 (DIL28)(interface to custom 5303)
1x custom 0748 (DIL28)(clock divider)
3x custom 0883 (DIL28)(bus controller)
1x custom 5156 (DIL42)(I/O)
1x custom 5303 (DIL42)(I/O)
1x oscillator 18432

on bottom PCB (Sidam 11510):

1x custom 0037 (DIL28)(unknown)
1x custom 0228 (DIL28)(gfx data shifter and mixer(16-bit in, 4-bit out))
1x custom 0425 (DIL28)(sprite address generator)
1x custom 0764 (DIL28)(clock divider)
1x custom DD1-6 (DIL20 300mil)(unknown)

ROMs:

on main PCB (Sidam 11500):

7x TMS2531JL
2x TBP24S10N (11220, 11221)

on bottom PCB (Sidam 11510):

1x TMS2516JL (8)
6x D2732A
1x TBP24S10N (11523)
1x SN74S288N (11524)

Notes:

on main PCB (Sidam 11500):
1x 22x2 edge connector
1x 3 legs power connector
1x 50 pins flat cable connector to bottom
1x trimmer (volume)
2x 8x2 switches DIP

on bottom PCB (Sidam 11510):
1x 50 pins flat cable connector to main
1x 6 legs connector

*/

ROM_START( digsid )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code for the first CPU  */
	ROM_LOAD( "digdug0.0",   0x0000, 0x1000, CRC(602197f0) SHA1(bea3b98a3f0f89d3b9e87aa38550ddd6f7883921) )
	ROM_LOAD( "digdug1.1",   0x1000, 0x1000, CRC(c6c8306b) SHA1(53e63ccb7edfdeea75df961ac69ebe882d808920) )
	ROM_LOAD( "digdug2.2",   0x2000, 0x1000, CRC(b695ec17) SHA1(46811106dbb686df6dc73b29e9e7db97b8c0d412) )
	ROM_LOAD( "digdug3.3",   0x3000, 0x1000, CRC(17bbfa40) SHA1(d3c7bf986d1d2b1961cea0c5e548245e84d74924) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "digdug4.4",       0x0000, 0x1000, CRC(370ef9b4) SHA1(746b1fa15f5f2cfd69d8b5a7d6fb8c770abc3b4d) )
	ROM_LOAD( "digdug5.5",       0x1000, 0x1000, CRC(d751df5d) SHA1(b08becb0176849a0fd1a706d6fae862684ff00b9) )

	ROM_REGION( 0x10000, "sub2", 0 )    /* 64k for the third CPU  */
	ROM_LOAD( "digdug6.6",   0x0000, 0x1000, CRC(a41bce72) SHA1(2b9b74f56aa7939d9d47cf29497ae11f10d78598) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "digdug8.8",        0x0000, 0x0800, CRC(f14a6fe1) SHA1(0aa63300c2cb887196de590aceb98f3cf06fead4) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "digdug14.14",   0x0000, 0x1000, CRC(e22957c8) SHA1(4700c63f4f680cb8ab8c44e6f3e1712aabd5daa4) )
	ROM_LOAD( "digdug13.13",   0x1000, 0x1000, CRC(2829ec99) SHA1(3e435c1afb2e44487cd7ba28a93ada2e5ccbb86d) )
	ROM_LOAD( "digdug12.12",   0x2000, 0x1000, CRC(458499e9) SHA1(578bd839f9218c3cf4feee1223a461144e455df8) )
	ROM_LOAD( "digdug11.11",   0x3000, 0x1000, CRC(c58252a0) SHA1(bd79e39e8a572d2b5c205e6de27ca23e43ec9f51) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "digdug10.10",       0x0000, 0x1000, CRC(7b383983) SHA1(57f1e8f5171d13f9f76bd091d81b4423b59f6b42) )

	ROM_REGION( 0x1000, "gfx4", 0 ) /* 4k for the playfield graphics */
	ROM_LOAD( "digdug9.9",      0x0000, 0x1000, CRC(2cf399c2) SHA1(317c48818992f757b1bd0e3997fa99937f81b52c) )

	/* Proms were not dumped with this set */
	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "136007.113",   0x0000, 0x0020, CRC(4cb9da99) SHA1(91a5852a15d4672c29fdcbae75921794651f960c) )
	ROM_LOAD( "136007.111",   0x0020, 0x0100, CRC(00c7c419) SHA1(7ea149e8eb36920c3b84984b5ce623729d492fd3) )
	ROM_LOAD( "136007.112",   0x0120, 0x0100, CRC(e9b3e08e) SHA1(a294cc4da846eb702d61678396bfcbc87d30ea95) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound prom */
	ROM_LOAD( "136007.110",   0x0000, 0x0100, CRC(7a2815b4) SHA1(085ada18c498fdb18ecedef0ea8fe9217edb7b46) )
	ROM_LOAD( "136007.109",   0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

DRIVER_INIT_MEMBER(galaga_state,galaga)
{
	/* swap bytes for flipped character so we can decode them together with normal characters */
	UINT8 *rom = memregion("gfx1")->base();
	int i, len = memregion("gfx1")->bytes();

	for (i = 0;i < len;i++)
	{
		if ((i & 0x0808) == 0x0800)
		{
			int t = rom[i];
			rom[i] = rom[i+8];
			rom[i+8] = t;
		}
	}
}

DRIVER_INIT_MEMBER(galaga_state,gatsbee)
{
	DRIVER_INIT_CALL(galaga);

	/* Gatsbee has a larger character ROM, we need a handler for banking */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x1000, 0x1000, write8_delegate(FUNC(galaga_state::gatsbee_bank_w),this));
}


DRIVER_INIT_MEMBER(xevious_state,xevious)
{
	UINT8 *rom;
	int i;

	rom = memregion("gfx3")->base() + 0x5000;
	for (i = 0;i < 0x2000;i++)
		rom[i + 0x2000] = rom[i] >> 4;
}

DRIVER_INIT_MEMBER(xevious_state,xevios)
{
	int A;
	UINT8 *rom;


	/* convert one of the sprite ROMs to the format used by Xevious */
	rom = memregion("gfx3")->base();
	for (A = 0x5000;A < 0x7000;A++)
	{
		rom[A] = BITSWAP8(rom[A],1,3,5,7,0,2,4,6);
	}

	/* convert one of tile map ROMs to the format used by Xevious */
	rom = memregion("gfx4")->base();
	for (A = 0x0000;A < 0x1000;A++)
	{
		rom[A] = BITSWAP8(rom[A],3,7,5,1,2,6,4,0);
	}

	DRIVER_INIT_CALL(xevious);
}


DRIVER_INIT_MEMBER(xevious_state,battles)
{
	/* replace the Namco I/O handlers with interface to the 4th CPU */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x7000, 0x700f, read8_delegate(FUNC(xevious_state::battles_customio_data0_r),this), write8_delegate(FUNC(xevious_state::battles_customio_data0_w),this) );
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x7100, 0x7100, read8_delegate(FUNC(xevious_state::battles_customio0_r),this), write8_delegate(FUNC(xevious_state::battles_customio0_w),this) );

	DRIVER_INIT_CALL(xevious);
}


/* Original Namco hardware, with Namco Customs */

//    YEAR, NAME,      PARENT,  MACHINE, INPUT,    INIT,    MONITOR,COMPANY,FULLNAME,FLAGS
GAME( 1981, bosco,     0,       bosco,   bosco, driver_device,    0,       ROT0,   "Namco", "Bosconian (new version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1981, boscoo,    bosco,   bosco,   bosco, driver_device,    0,       ROT0,   "Namco", "Bosconian (old version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1981, boscoo2,   bosco,   bosco,   bosco, driver_device,    0,       ROT0,   "Namco", "Bosconian (older version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1981, boscomd,   bosco,   bosco,   boscomd, driver_device,  0,       ROT0,   "Namco (Midway license)", "Bosconian (Midway, new version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1981, boscomdo,  bosco,   bosco,   boscomd, driver_device,  0,       ROT0,   "Namco (Midway license)", "Bosconian (Midway, old version)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1981, galaga,    0,       galaga,  galaga, galaga_state,   galaga,  ROT90,  "Namco", "Galaga (Namco rev. B)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1981, galagao,   galaga,  galaga,  galaga, galaga_state,   galaga,  ROT90,  "Namco", "Galaga (Namco)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1981, galagamw,  galaga,  galaga,  galagamw, galaga_state, galaga,  ROT90,  "Namco (Midway license)", "Galaga (Midway set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1981, galagamk,  galaga,  galaga,  galaga, galaga_state,   galaga,  ROT90,  "Namco (Midway license)", "Galaga (Midway set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1981, galagamf,  galaga,  galaga,  galaga, galaga_state,   galaga,  ROT90,  "Namco (Midway license)", "Galaga (Midway set 1 with fast shoot hack)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1982, xevious,   0,       xevious, xevious, xevious_state,  xevious, ROT90,  "Namco", "Xevious (Namco)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, xeviousa,  xevious, xevious, xeviousa, xevious_state, xevious, ROT90,  "Namco (Atari license)", "Xevious (Atari, harder)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, xeviousb,  xevious, xevious, xeviousb, xevious_state, xevious, ROT90,  "Namco (Atari license)", "Xevious (Atari)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, xeviousc,  xevious, xevious, xeviousa, xevious_state, xevious, ROT90,  "Namco (Atari license)", "Xevious (Atari, Namco PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, sxevious,  xevious, xevious, sxevious, xevious_state, xevious, ROT90,  "Namco", "Super Xevious", MACHINE_SUPPORTS_SAVE )
GAME( 1984, sxeviousj, xevious, xevious, sxevious, xevious_state, xevious, ROT90,  "Namco", "Super Xevious (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1982, digdug,    0,       digdug,  digdug, driver_device,   0,       ROT90,  "Namco", "Dig Dug (rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, digdug1,   digdug,  digdug,  digdug, driver_device,   0,       ROT90,  "Namco", "Dig Dug (rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, digdugat,  digdug,  digdug,  digdug, driver_device,   0,       ROT90,  "Namco (Atari license)", "Dig Dug (Atari, rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, digdugat1, digdug,  digdug,  digdug, driver_device,   0,       ROT90,  "Namco (Atari license)", "Dig Dug (Atari, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, digsid,    digdug,  digdug,  digdug, driver_device,   0,       ROT90,  "Namco (Sidam license)", "Dig Dug (manufactured by Sidam)", MACHINE_SUPPORTS_SAVE )

/* Bootlegs with replacement I/O chips */

GAME( 1981, gallag,    galaga,  galagab, galaga, galaga_state,   galaga,  ROT90,  "bootleg", "Gallag", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1984, gatsbee,   galaga,  galagab, gatsbee, galaga_state,  gatsbee, ROT90,  "hack", "Gatsbee", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

GAME( 1982, xevios,    xevious, xevious, xevious, xevious_state,  xevios,  ROT90,  "bootleg", "Xevios", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, battles,   xevious, battles, xevious, xevious_state,  battles, ROT90,  "bootleg", "Battles", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1982, dzigzag,   digdug,  dzigzag, digdug, driver_device,   0,       ROT90,  "bootleg", "Zig Zag (Dig Dug hardware)", MACHINE_SUPPORTS_SAVE )
