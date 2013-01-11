/***************************************************************************

  gb.c

  Driver file to handle emulation of the Nintendo Game Boy.
  By:

  Hans de Goede               1998
  Anthony Kruize              2002
  Wilbert Pol                 2004 (Megaduck/Cougar Boy)

  TODO list:
  Done entries kept for historical reasons, besides that it's nice to see
  what is already done instead of what has to be done.

Priority:  Todo:                                                  Done:
  2        Replace Marat's  video/gb.c  by Playboy code           *
  2        Clean & speed up video/gb.c                            *
  2        Replace Marat's  Z80gb/Z80gb.c by Playboy code           *
  2        Transform Playboys Z80gb.c to big case method            *
  2        Clean up Z80gb.c                                         *
  2        Fix / optimise halt instruction                          *
  2        Do correct lcd stat timing                               In Progress
  2        Generate lcd stat interrupts                             *
  2        Replace Marat's code in machine/gb.c by Playboy code     ?
  1        Check, and fix if needed flags bug which troubles ffa    ?
  1        Save/restore battery backed ram                          *
  1        Add sound                                                *
  0        Add supergb support                                      *
  0        Add palette editting, save & restore
  0        Add somekind of backdrop support
  0        Speedups if remotly possible

  2 = has to be done before first public release
  1 = should be added later on
  0 = bells and whistles


Timers
======

There seems to be some kind of selectable internal clock divider which is used to drive
the timer increments. This causes the first timer cycle to now always be a full cycle.
For instance in 1024 clock cycle mode, the first timer cycle could easily only take 400
clock cycles. The next timer cycle will take the full 1024 clock cycles though.

Writes to the DIV register seem to cause this internal clock divider/register to be
reset in such a way that the next stimulus cause a timer increment (in any mode).


Interrupts
==========

Taking an interrupt seems to take around 20 clock cycles.


Stat timing
===========

This timing table is accurate within 4 cycles:
           | stat = 2 | stat = 3 | stat = 0 |
No sprites |    80    |    172   |    204   |
1 sprite   |    80    |    182   |    194   |
2 sprites  |    80    |    192   |    184   |
3 sprites  |    80    |    202   |    174   |
4 sprites  |    80    |    212   |    164   |
5 sprites  |    80    |    222   |    154   |
6 sprites  |    80    |    232   |    144   |
7 sprites  |    80    |    242   |    134   |
8 sprites  |    80    |    252   |    124   |
9 sprites  |    80    |    262   |    114   |
10 sprites |    80    |    272   |    104   |

In other words, each sprite on a line makes stat 3 last 10 cycles longer.


For lines 1 - 143 when stat changes to 2 the line counter is incremented.

Line 153 is little odd timing wise. The line counter stays 153 for ~4 clock cycles
and is then rolls over to 0.

When the line counter is changed it gets checked against the lyc register.

Here is a detailed run of the STAT and LY register together with LYC set to 3 on a
dmg and mgb. The time between each sample is 4 clock cycles:
STAT:
22222222 22233333 33333333 33333333 33333333 33333333 33333300 00000000 00000000 00000000
00000000 00000000 00000000 06666666 66666666 66666777 77777777 77777777 77777777 77777777
77777777 44444444 44444444 44444444 44444444 44444444 44444444 44022222 22222222

  LY:
33333333 33333333 33333333 33333333 33333333 33333333 33333333 33333333 33333333 33333333
33333333 33333333 33333333 44444444 44444444 44444444 44444444 44444444 44444444 44444444
44444444 44444444 44444444 44444444 44444444 44444444 44444444 44555555 55555555
                           ^                                     ^

As you can see, it seems as though the LY register is incremented slightly before the STAT
register is changed, resulting in a short period where STAT goes 0 before going to 2. This
bug/feature has been fixed in the CGB and AGB.



Around lines 152-153-0 the picture becomes as follows:
STAT:
11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111
11111111 11111111 11111111 15555555 55555555 55555555 55555555 55555555 55555555 55555555
55555555 55555555 55555555 55555555 55555555 55555555 55555555 55111111 11111111 11111111
11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111
11111111 11110222 22222222 22222222 23333333 33333333 33333333 33333333 33333333

  LY:
77777777 77777777 77777777 77777777 77777777 77777777 77777777 77777777 77777777 77777777
77777777 77777777 77777777 88888888 88888888 88888888 88888888 88888888 88888888 88888888
88888888 88888888 88888888 88888888 88888888 88888888 88888888 88900000 00000000 00000000
00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000



The full STAT/LY value state machine.
=====================================

The timing information below is with sprites disabled.

For STAT we only show the lower 3 bits and for LY only the lower 5 bits of the full
register. Each digit stands for 4 clock cycles (the smallest measurable unit on a
dmg or mgb). When the video hardware is switched on the LY register is set 0 and
the STAT mode is 0. The values for STAT and LY will change as follows:

STAT 000000000000000000003333333333333333333333333333333333333333333000000000000000000000000000000000000000000000000000
  LY 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001  line #0
     ^LY=LYC bit can get set here                                                             LY=LYC bit is reset here^

STAT 222222222222222222223333333333333333333333333333333333333333333000000000000000000000000000000000000000000000000000
  LY 111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111112  line #1

     :
     :

STAT 222222222222222222223333333333333333333333333333333333333333333000000000000000000000000000000000000000000000000000
  LY FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0  line #143

STAT 111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111
  LY 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001  line #144

     :
     :

STAT 111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111
  LY 888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888889  line #152

STAT 111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110
  LY 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000  line #153
     ^
     LY=LYC interrupt for 153 can get triggered here

STAT 222222222222222222223333333333333333333333333333333333333333333000000000000000000000000000000000000000000000000000
  LY 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001  line #0

STAT 222222222222222222223333333333333333333333333333333333333333333000000000000000000000000000000000000000000000000000
  LY 111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111112  line #1

     :
     etc


Mappers used in the Game Boy
===========================

MBC1 Mapper
===========

The MBC1 mapper has two modes: 2MB ROM/8KB RAM or 512KB ROM/32KB RAM.
This mode is selected by writing into the 6000-7FFF memory area:
0bXXXXXXXB - B=0 - 2MB ROM/8KB RAM mode
             B=1 - 512KB ROM/32KB RAM mode
The default behaviour is to be in 2MB ROM/8KB RAM mode.

Writing a value ( 0bXXXBBBBB ) into the 2000-3FFF memory area selects the
lower 5 bits of the ROM bank to select for the 4000-7FFF memory area. If a
value of 0bXXX00000 is written then this will autmatically be changed to
0bXXX00001 by the mbc chip.

Writing a value (0bXXXXXXBB ) into the 4000-5FFF memory area either selects
the RAM bank to use or bits 6 and 7 for the ROM bank to use for the 4000-7FFF
memory area. This behaviour depends on the memory moddel chosen.

The RAM sections are enabled by writing the value 0bXXX1010 into the 0000-1FFF
memory area. Writing any other value disables the RAM section.

Some unanswered cases:
#1 - Set mode 0
   - Set lower bank bits to 1F
   - Set high bank bits to 01  => bank #3F
   - Set mode 1
   - What ROM bank is now at 4000-7FFF, bank #1F or bank #3F?

#2 - Set mode 1
   - Set ram area #1
   - Set mode 0
   - What ram area is now at A000-BFFF, ram bank 00 or ram bank 01?


MBC2 Mapper
===========

The MBC2 mapper includes 512x4bits of builtin RAM.

0000-1FFF - Writing to this area enables (value 0bXXXX1010) or disables (any
            other value than 0bXXXX1010) the RAM. In order to perform this
            function bit 12 of the address must be reset, so usable areas are
            0000-00FF, 0200-02FF, 0400-04FF, 0600-06FF, ..., 1E00-1EFF.
2000-3FFF - Writing to this area selects the rom bank to appear at 4000-7FFF.
            Only bits 3-0 are used to select the bank number. If a value of
            0bXXXX0000 is written then this is automatically changed into
            0bXXXX0001 by the mapper.
            In order to perform the rom banking bit 12 of the address must be
            set, so usable areas are 2100-21FF, 2300-23FF, 2500-25FF, 2700-
            27FF, ..., 3F00-3FFF.

Some unanswered cases:
#1 - Set rom bank to 8 for a 4 bank rom image.
   - What rom bank appears at 4000-7FFF, bank #0 or bank #1 ?


MBC3 Mapper
===========

The MBC3 mapper cartridges can include a RTC chip.

0000-1FFF - Writing to this area enables (value 0x0A) or disables (0x00) the
            RAM and RTC registers.
2000-3FFF - Writing to this area selects the rom bank to appear at 4000-7FFF.
            Bits 6-0 are used  to select the bank number. If a value of
            0bX0000000 is written then this is autmatically changed into
            0bX0000001 by the mapper.
4000-5FFF - Writing to this area selects the RAM bank or the RTC register to
            read.
            XXXX00bb - Select RAM bank bb.
            XXXX1rrr - Select RTC register rrr. Accepted values for rrr are:
                       000 - Seconds (0x00-0x3B)
                       001 - Minutes (0x00-0x3B)
                       010 - Hours (0x00-0x17)
                       011 - Bits 7-0 of the day counter
                       100 - bit 0 - Bit 8 of the day counter
                             bit 6 - Halt RTC timer ( 0 = timer active, 1 = halted)
                             bit 7 - Day counter overflow flag
6000-7FFF - Writing 0x00 followed by 0x01 latches the RTC data. This latching
            method is used for reading the RTC registers.

Some unanswered cases:
#1 - Set rom bank to 8(/16/32/64) for a 4(/8/16/32) bank image.
   - What rom bank appears at 4000-7FFF, bank #0 or bank #1 ?


MBC4 Mapper
===========

Stauts: not supported yet.


MBC5 Mapper
===========

0000-1FFF - Writing to this area enables (0x0A) or disables (0x00) the RAM area.
2000-2FFF - Writing to this area updates bits 7-0 of the rom bank number to
            appear at 4000-7FFF.
3000-3FFF - Writing to this area updates bit 8 of the rom bank number to appear
            at 4000-7FFF.
4000-5FFF - Writing to this area select the RAM bank number to use. If the
            cartridge includes a Rumble Pack then bit 3 is used to control
            rumble motor (0 - disable motor, 1 - enable motor).


MBC7 Mapper (Used by Kirby's Tilt n' Tumble, Command Master)
===========

Status: Partial support (only ROM banking supported at the moment)

The MBC7 mapper has 0x0200(?) bytes of RAM built in.

0000-1FFF - Probably enable/disable RAM
            In order to use this area bit 12 of the address be set.
            Values written: 00, 0A
2000-2FFF - Writing to this area selects the ROM bank to appear at
            4000-7FFF.
            In order to use this area bit 12 of the address be set.
            Values written: 01, 07, 01, 1C
3000-3FFF - Unknown
            In order to use this area bit 12 of the address be set.
            Values written: 00
4000-4FFF - Unknown
            In order to use this area bit 12 of the address be set.
            Values written: 00, 40, 3F


TAMA5 Mapper (Used by Tamagotchi 3)
============

Status: partially supported.

The TAMA5 mapper includes a special RTC chip which communicates through the
RAM area (0xA000-0xBFFF); most notably addresses 0xA000 and 0xA001 seem to
be used. In this setup 0xA001 acts like a control register and 0xA000 like
a data register.

Accepted values by the TAMA5 control register:
0x00 - Writing to 0xA000 will set bits 3-0 for rom bank selection.
0x01 - Writing to 0xA000 will set bits (7-?)4 for rom bank selection.

0x04 - Bits 3-0 of the value to write
0x05 - Bits 4-7 of the value to write
0x06 - Address control hi
       bit 0 - Bit 4 for the address
       bit 3-1 - 000 - Write a byte to the 32 byte memory. The data to be
                       written must be set in registers 0x04 (lo nibble) and
                       0x05 (hi nibble).
               - 001 - Read a byte from the 32 byte memory. The data read
                       will be available in registers 0x0C (lo nibble) and
                       0x0D (hi nibble).
               - 010 - Unknown (occurs just after having started a game and
                       entered a date) (execution at address 1A19)
               - 011 - Unknown (not encountered yet)
               - 100 - Unknown (occurs during booting a game; appears to be
                       some kind of read command as it is followed by a read
                       of the 0x0C register) (execution at address 1B5B)
               - 101 - Unknown (not encountered yet)
               - 110 - Unknown (not encountered yet)
               - 111 - Unknown (not encountered yet)
0x07 - Address control lo
       bit 3-0 - bits 3-0 for the address

0x0A - After writing this the lowest 2 bits of A000 determine whether the
       TAMA5 chip is ready to accept the next command. If the lowest 2 bits
       hold the value 01 then the TAMA5 chip is ready for the next command.

0x0C - Reading from A000 will return bits 3-0 of the data
0x0D - Reading from A000 will return bits 7-4 of the data

0x04 - RTC controls? -> RTC/memory?
0x05 - Write time/memomry?
0x06 - RTC controls?
0x07 - RTC controls?

Unknown sequences:
During booting a game (1B5B:
04 <- 00, 06 <- 08, 07 <- 01, followed by read 0C
when value read from 0C equals 00 followed by the sequence:
04 <- 01, 06 <- 08, 07 <- 01, followed by read 0C
the value read from 0C is checked for non-zero, don't know the consequences for either
yet.

Initialization after starting a game:
At address 1A19:
06 <- 05, 07 <- 02, followed by read 0C, if != 0F => OK, otherwise do something.


MMM01 mapper
============

Used by: Momotarou Collection 2, Taito Pack

Status: not supported yet.

Momotarou Collection 2:

MOMOTARODENGEKI2, 0x00000, blocks 0x00 - 0x1F
0x147: 01 04 00 00 33 00
MOMOTAROU GAIDEN, 0x80000, blocks 0x20 - 0x3F
0x147: 06 03 00 00 18 00

When picking top option:
3FFF <- 20
5FFF <- 40
7FFF <- 21
1FFF <- 3A
1FFF <- 7A

When picking bottom option:
3FFF <- 00
5FFF <- 01
7FFF <- 01
1FFF <- 3A
1FFF <- 7A

Taito Pack (MMM01+RAM, 512KB, 64KB RAM):
1st option (BUBBLE BOBBLE, blocks 0x10 - 0x17, MBC1+RAM, 128KB, 8KB RAM):
  2000 <- 70  01110000  => starting block, 10000
  6000 <- 30  00110000  => 8 blocks
  4000 <- 70  01110000  => ???
  0000 <- 40  01000000  => upper 3 bits determine lower 3 bits of starting block?

2nd option (ELEVATOR ACTION, blocks 0x18 - 0x1B, MBC1, 64KB, 2KB RAM):
  2000 <- 78  01111000  => starting block, 11000
  6000 <- 38  00111000  => 4 blocks
  4000 <- 70  01110000  => ???
  0000 <- 40  01000000  => upper 3 bits determine lower 3 bits of starting block?

3rd option (CHASE HQ, blocks 0x08 - 0x0F, MBC1+RAM, 128KB, 8KB RAM):
  2000 <- 68  01101000  => starting block, 01000
  6000 <- 30  00110000  => 8 blocks
  4000 <- 70  01110000  => ???
  0000 <- 40  01000000  => upper 3 bits determine lower 3 bits of starting block?

4th option (SAGAIA, blocks 0x00 - 0x07, MBC1+RAM, 128KB, 8KB RAM):
  2000 <- 60  01100000  => starting block, 00000
  6000 <- 30  00110000  => 8 blocks
  4000 <- 70  01110000  => ???
  0000 <- 40  01000000  => upper 3 bits determine lower 3 bits of starting block?

Known:
The last 2 banks in a MMM01 dump are actually the starting banks for a MMM01 image.

0000-1FFF => bit6 set => perform mapping

Possible mapping registers:
1FFF - Enable RAM ???
3FFF - xxxbbbbb - Bit0-5 of the rom bank to select at 0x4000-0x7FFF ?


HuC1 mapper
===========

Status: not supported yet.


HuC3 mapper
===========

Status: not supported yet.


Wisdom Tree mapper
==================

The Wisdom Tree mapper is triggered by writes in the 0x0000-0x3FFF area. The
address written to determines the bank to switch in in the 0x000-0x7FFF address
space. This mapper uses 32KB sized banks.


***************************************************************************/

#include "emu.h"
#include "machine/ram.h"
#include "cpu/lr35902/lr35902.h"
#include "imagedev/cartslot.h"
#include "rendlay.h"
#include "audio/gb.h"
#include "includes/gb.h"


/* Initial value of the cpu registers (hacks until we get bios dumps) */
static const UINT16 mgb_cpu_regs[6] = { 0xFFB0, 0x0013, 0x00D8, 0x014D, 0xFFFE, 0x0100 };   /* Game Boy Pocket / Super Game Boy 2 */
static const UINT16 megaduck_cpu_regs[6] = { 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFE, 0x0000 };  /* Megaduck */

static const struct lr35902_config dmg_cpu_reset = { NULL, LR35902_FEATURE_HALT_BUG, gb_timer_callback };
static const struct lr35902_config sgb_cpu_reset = { NULL, LR35902_FEATURE_HALT_BUG, gb_timer_callback };
static const struct lr35902_config mgb_cpu_reset = { mgb_cpu_regs, LR35902_FEATURE_HALT_BUG, gb_timer_callback };
static const struct lr35902_config cgb_cpu_reset = { NULL, 0, gb_timer_callback };
static const struct lr35902_config megaduck_cpu_reset = { megaduck_cpu_regs, LR35902_FEATURE_HALT_BUG, gb_timer_callback };

static ADDRESS_MAP_START(gb_map, AS_PROGRAM, 8, gb_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x00ff) AM_ROMBANK("bank5")                    /* BIOS or ROM */
	AM_RANGE(0x0100, 0x01ff) AM_ROMBANK("bank10")                   /* ROM bank */
	AM_RANGE(0x0200, 0x08ff) AM_ROMBANK("bank6")
	AM_RANGE(0x0900, 0x3fff) AM_ROMBANK("bank11")
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank1")                    /* 8KB/16KB switched ROM bank */
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank4")                    /* 8KB/16KB switched ROM bank */
	AM_RANGE(0x8000, 0x9fff) AM_READWRITE(gb_vram_r, gb_vram_w ) /* 8k VRAM */
	AM_RANGE(0xa000, 0xbfff) AM_RAMBANK("bank2")                    /* 8k switched RAM bank (cartridge) */
	AM_RANGE(0xc000, 0xfdff) AM_RAM                     /* 8k low RAM, echo RAM */
	AM_RANGE(0xfe00, 0xfeff) AM_READWRITE(gb_oam_r, gb_oam_w )  /* OAM RAM */
	AM_RANGE(0xff00, 0xff0f) AM_READWRITE(gb_io_r, gb_io_w )        /* I/O */
	AM_RANGE(0xff10, 0xff26) AM_DEVREADWRITE_LEGACY("custom", gb_sound_r, gb_sound_w )      /* sound registers */
	AM_RANGE(0xff27, 0xff2f) AM_NOP                     /* unused */
	AM_RANGE(0xff30, 0xff3f) AM_DEVREADWRITE_LEGACY("custom", gb_wave_r, gb_wave_w )        /* Wave ram */
	AM_RANGE(0xff40, 0xff7f) AM_READWRITE(gb_video_r, gb_io2_w)     /* Video controller & BIOS flip-flop */
	AM_RANGE(0xff80, 0xfffe) AM_RAM                     /* High RAM */
	AM_RANGE(0xffff, 0xffff) AM_READWRITE(gb_ie_r, gb_ie_w )        /* Interrupt enable register */
ADDRESS_MAP_END

static ADDRESS_MAP_START(sgb_map, AS_PROGRAM, 8, gb_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x00ff) AM_ROMBANK("bank5")                    /* BIOS or ROM */
	AM_RANGE(0x0100, 0x01ff) AM_ROMBANK("bank10")                   /* ROM bank */
	AM_RANGE(0x0200, 0x08ff) AM_ROMBANK("bank6")
	AM_RANGE(0x0900, 0x3fff) AM_ROMBANK("bank11")
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank1")                    /* 8KB/16KB switched ROM bank */
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank4")                    /* 8KB/16KB switched ROM bank */
	AM_RANGE(0x8000, 0x9fff) AM_READWRITE(gb_vram_r, gb_vram_w ) /* 8k VRAM */
	AM_RANGE(0xa000, 0xbfff) AM_RAMBANK("bank2")                    /* 8k switched RAM bank (cartridge) */
	AM_RANGE(0xc000, 0xfdff) AM_RAM                     /* 8k low RAM, echo RAM */
	AM_RANGE(0xfe00, 0xfeff) AM_READWRITE(gb_oam_r, gb_oam_w )  /* OAM RAM */
	AM_RANGE(0xff00, 0xff0f) AM_READWRITE(gb_io_r, sgb_io_w )       /* I/O */
	AM_RANGE(0xff10, 0xff26) AM_DEVREADWRITE_LEGACY("custom", gb_sound_r, gb_sound_w )      /* sound registers */
	AM_RANGE(0xff27, 0xff2f) AM_NOP                     /* unused */
	AM_RANGE(0xff30, 0xff3f) AM_DEVREADWRITE_LEGACY("custom", gb_wave_r, gb_wave_w )        /* Wave RAM */
	AM_RANGE(0xff40, 0xff7f) AM_READWRITE(gb_video_r, gb_io2_w )        /* Video controller & BIOS flip-flop */
	AM_RANGE(0xff80, 0xfffe) AM_RAM                     /* High RAM */
	AM_RANGE(0xffff, 0xffff) AM_READWRITE(gb_ie_r, gb_ie_w )        /* Interrupt enable register */
ADDRESS_MAP_END

static ADDRESS_MAP_START(gbc_map, AS_PROGRAM, 8, gb_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x00ff) AM_ROMBANK("bank5")                    /* 16k fixed ROM bank */
	AM_RANGE(0x0100, 0x01ff) AM_ROMBANK("bank10")                   /* ROM bank */
	AM_RANGE(0x0200, 0x08ff) AM_ROMBANK("bank6")
	AM_RANGE(0x0900, 0x3fff) AM_ROMBANK("bank11")
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank1")                    /* 8KB/16KB switched ROM bank */
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank4")                    /* 8KB/16KB switched ROM bank */
	AM_RANGE(0x8000, 0x9fff) AM_READWRITE(gb_vram_r, gb_vram_w )        /* 8k switched VRAM bank */
	AM_RANGE(0xa000, 0xbfff) AM_RAMBANK("bank2")                    /* 8k switched RAM bank (on cartridge) */
	AM_RANGE(0xc000, 0xcfff) AM_RAM                     /* 4k fixed RAM bank */
	AM_RANGE(0xd000, 0xdfff) AM_RAMBANK("bank3")                    /* 4k switched RAM bank */
	AM_RANGE(0xe000, 0xfdff) AM_RAM                     /* echo RAM */
	AM_RANGE(0xfe00, 0xfeff) AM_READWRITE(gb_oam_r, gb_oam_w )  /* OAM RAM */
	AM_RANGE(0xff00, 0xff0f) AM_READWRITE(gb_io_r, gb_io_w )        /* I/O */
	AM_RANGE(0xff10, 0xff26) AM_DEVREADWRITE_LEGACY("custom", gb_sound_r, gb_sound_w )      /* sound controller */
	AM_RANGE(0xff27, 0xff2f) AM_NOP                     /* unused */
	AM_RANGE(0xff30, 0xff3f) AM_DEVREADWRITE_LEGACY("custom", gb_wave_r, gb_wave_w )        /* Wave RAM */
	AM_RANGE(0xff40, 0xff7f) AM_READWRITE(gbc_io2_r, gbc_io2_w )        /* Other I/O and video controller */
	AM_RANGE(0xff80, 0xfffe) AM_RAM                     /* high RAM */
	AM_RANGE(0xffff, 0xffff) AM_READWRITE(gb_ie_r, gb_ie_w )        /* Interrupt enable register */
ADDRESS_MAP_END

static ADDRESS_MAP_START(megaduck_map, AS_PROGRAM, 8, gb_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROMBANK("bank10")                       /* 16k switched ROM bank */
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")                        /* 16k switched ROM bank */
	AM_RANGE(0x8000, 0x9fff) AM_READWRITE(gb_vram_r, gb_vram_w )        /* 8k VRAM */
	AM_RANGE(0xa000, 0xbfff) AM_NOP                         /* unused? */
	AM_RANGE(0xc000, 0xfe9f) AM_RAM                         /* 8k low RAM, echo RAM */
	AM_RANGE(0xfe00, 0xfeff) AM_READWRITE(gb_oam_r, gb_oam_w )      /* OAM RAM */
	AM_RANGE(0xff00, 0xff0f) AM_READWRITE(gb_io_r, gb_io_w )            /* I/O */
	AM_RANGE(0xff10, 0xff1f) AM_READWRITE(megaduck_video_r, megaduck_video_w )  /* video controller */
	AM_RANGE(0xff20, 0xff2f) AM_READWRITE(megaduck_sound_r1, megaduck_sound_w1) /* sound controller pt1 */
	AM_RANGE(0xff30, 0xff3f) AM_DEVREADWRITE_LEGACY("custom", gb_wave_r, gb_wave_w )            /* wave ram */
	AM_RANGE(0xff40, 0xff46) AM_READWRITE(megaduck_sound_r2, megaduck_sound_w2) /* sound controller pt2 */
	AM_RANGE(0xff47, 0xff7f) AM_NOP                         /* unused */
	AM_RANGE(0xff80, 0xfffe) AM_RAM                         /* high RAM */
	AM_RANGE(0xffff, 0xffff) AM_READWRITE(gb_ie_r, gb_ie_w )            /* interrupt enable register */
ADDRESS_MAP_END

static GFXDECODE_START( gb )
GFXDECODE_END

static INPUT_PORTS_START( gameboy )
	PORT_START("INPUTS")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_NAME("Left")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_NAME("Right")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_NAME("Up")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_NAME("Down")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Button A")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Button B")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START) PORT_NAME("Start")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SELECT) PORT_NAME("Select")
INPUT_PORTS_END

static MACHINE_CONFIG_START( gb_common, gb_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", LR35902, 4194304)           /* 4.194304 MHz */
	MCFG_CPU_PROGRAM_MAP(gb_map)
	MCFG_LR35902_CONFIG(dmg_cpu_reset)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gb_state,  gb_scanline_interrupt)  /* 1 dummy int each frame */

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(gb_state, gb )
	MCFG_MACHINE_RESET_OVERRIDE(gb_state, gb )

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(DMG_FRAMES_PER_SECOND)
	MCFG_SCREEN_VBLANK_TIME(0)
	MCFG_SCREEN_UPDATE_DRIVER(gb_state, screen_update)
	MCFG_DEFAULT_LAYOUT(layout_lcd)
//  MCFG_SCREEN_SIZE(20*8, 18*8)
	MCFG_SCREEN_SIZE( 458, 154 )
	MCFG_SCREEN_VISIBLE_AREA(0*8, 20*8-1, 0*8, 18*8-1)

	MCFG_GFXDECODE(gb)
	MCFG_PALETTE_LENGTH(4)
	MCFG_PALETTE_INIT_OVERRIDE(gb_state,gb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("custom", GAMEBOY, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gameboy, gb_common )

	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("gb,gmb,cgb,gbc,sgb,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("gameboy_cart")
	MCFG_CARTSLOT_START(gb_cart)
	MCFG_CARTSLOT_LOAD(gb_cart)
	MCFG_SOFTWARE_LIST_ADD("cart_list","gameboy")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("gbc_list","gbcolor")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( supergb, gameboy )
	MCFG_CPU_REPLACE("maincpu", LR35902, 4295454)   /* 4.295454 MHz */
	MCFG_CPU_PROGRAM_MAP(sgb_map)

	MCFG_CPU_MODIFY("maincpu")
	MCFG_LR35902_CONFIG(sgb_cpu_reset)

	MCFG_MACHINE_START_OVERRIDE(gb_state, sgb )
	MCFG_MACHINE_RESET_OVERRIDE(gb_state, sgb )

	MCFG_DEFAULT_LAYOUT(layout_horizont)    /* runs on a TV, not an LCD */

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(32*8, 28*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_PALETTE_LENGTH(32768)
	MCFG_PALETTE_INIT_OVERRIDE(gb_state,sgb)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gbpocket, gameboy )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_LR35902_CONFIG(mgb_cpu_reset)
	MCFG_MACHINE_RESET_OVERRIDE(gb_state, gbpocket )
	MCFG_PALETTE_INIT_OVERRIDE(gb_state,gbp)

	MCFG_CARTSLOT_MODIFY("cart")
	MCFG_CARTSLOT_MANDATORY
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gbcolor, gb_common )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP( gbc_map)
	MCFG_LR35902_CONFIG(cgb_cpu_reset)

	MCFG_MACHINE_START_OVERRIDE(gb_state,gbc)
	MCFG_MACHINE_RESET_OVERRIDE(gb_state,gbc)

	MCFG_PALETTE_LENGTH(32768)
	MCFG_PALETTE_INIT_OVERRIDE(gb_state,gbc)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("48K") /* 2 pages of 8KB VRAM, 8 pages of 4KB RAM */

	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("gb,gmb,cgb,gbc,sgb,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("gameboy_cart")
	MCFG_CARTSLOT_START(gb_cart)
	MCFG_CARTSLOT_LOAD(gb_cart)
	MCFG_SOFTWARE_LIST_ADD("cart_list","gbcolor")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("gb_list","gameboy")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( megaduck, gb_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", LR35902, 4194304)           /* 4.194304 MHz */
	MCFG_CPU_PROGRAM_MAP( megaduck_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gb_state,  gb_scanline_interrupt)  /* 1 int each scanline ! */
	MCFG_LR35902_CONFIG(megaduck_cpu_reset)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(DMG_FRAMES_PER_SECOND)
	MCFG_SCREEN_VBLANK_TIME(0)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(gb_state, megaduck )
	MCFG_MACHINE_RESET_OVERRIDE(gb_state, megaduck )

	MCFG_SCREEN_UPDATE_DRIVER(gb_state, screen_update)
	MCFG_SCREEN_SIZE(20*8, 18*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 20*8-1, 0*8, 18*8-1)

	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_GFXDECODE(gb)
	MCFG_PALETTE_LENGTH(4)
	MCFG_PALETTE_INIT_OVERRIDE(gb_state,megaduck)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("custom", GAMEBOY, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("megaduck_cart")
	MCFG_CARTSLOT_LOAD(megaduck_cart)
	MCFG_SOFTWARE_LIST_ADD("cart_list","megaduck")
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( gameboy )
	ROM_REGION( 0x0100, "maincpu", 0 )
	ROM_LOAD( "dmg_boot.bin", 0x0000, 0x0100, CRC(59c8598e) SHA1(4ed31ec6b0b175bb109c0eb5fd3d193da823339f) )
ROM_END

ROM_START( supergb )
	ROM_REGION( 0x0100, "maincpu", 0 )
	ROM_LOAD( "sgb_boot.bin", 0x0000, 0x0100, CRC(ec8a83b9) SHA1(aa2f50a77dfb4823da96ba99309085a3c6278515) )
ROM_END

ROM_START( gbpocket )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
/*  ROM_LOAD( "gbp_boot.bin", 0x0000, 0x0100, NO_DUMP ) */
ROM_END

ROM_START( gblight )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
/*  ROM_LOAD( "gbl_boot.bin", 0x0000, 0x0100, NO_DUMP ) */
ROM_END

ROM_START( gbcolor )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "gbc_boot.1", 0x0000, 0x0100, CRC(779ea374) SHA1(e4b40c9fd593a97a1618cfb2696f290cf9596a62) )  /* Bootstrap code part 1 */
	ROM_LOAD( "gbc_boot.2", 0x0100, 0x0700, CRC(f741807d) SHA1(f943b1e0b640cf1d371e1d8f0ada69af03ebb396) ) /* Bootstrap code part 2 */
ROM_END


ROM_START( megaduck )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
ROM_END

/*    YEAR  NAME      PARENT   COMPAT   MACHINE   INPUT    INIT  COMPANY     FULLNAME */
CONS( 1990, gameboy,  0,       0,       gameboy,  gameboy, driver_device, 0,    "Nintendo", "Game Boy", 0)
CONS( 1994, supergb,  gameboy, 0,       supergb,  gameboy, driver_device, 0,    "Nintendo", "Super Game Boy", 0)
CONS( 1996, gbpocket, gameboy, 0,       gbpocket, gameboy, driver_device, 0,    "Nintendo", "Game Boy Pocket", 0)
CONS( 1997, gblight,  gameboy, 0,       gbpocket, gameboy, driver_device, 0,    "Nintendo", "Game Boy Light", 0)
CONS( 1998, gbcolor,  gameboy, 0,       gbcolor,  gameboy, driver_device, 0,    "Nintendo", "Game Boy Color", GAME_IMPERFECT_GRAPHICS)

/* Sound is not 100% yet, it generates some sounds which could be ok. Since we're lacking a real
   system there's no way to verify. Same goes for the colors of the LCD. We are no using the default
   Game Boy green colors */
CONS( 1993, megaduck, 0,       0,       megaduck, gameboy, driver_device, 0,    "Creatronic/Videojet/Timlex/Cougar",  "MegaDuck/Cougar Boy" , 0)
