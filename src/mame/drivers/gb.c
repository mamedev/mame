// license:BSD-3-Clause
// copyright-holders:Anthony Kruize,Wilbert Pol
/***************************************************************************

  gb.c

  Driver file to handle emulation of the Nintendo Game Boy.
  By:

  Hans de Goede               1998
  Anthony Kruize              2002
  Wilbert Pol                 2004 (Megaduck/Cougar Boy)

  TODO list:
  - Do correct lcd stat timing
  - Add Game Boy Light (Japan, 1997) - does it differ from gbpocket?
  - SGB should be moved to SNES driver
  - Emulate OAM corruption bug on 16bit inc/dec in $fe** region


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
#include "rendlay.h"
#include "includes/gb.h"
#include "bus/gameboy/rom.h"
#include "bus/gameboy/mbc.h"
#include "softlist.h"

READ8_MEMBER(gb_state::gb_cart_r)
{
	if (m_bios_disable && m_cartslot)
		return m_cartslot->read_rom(space, offset);
	else
	{
		if (offset < 0x100)
		{
			UINT8 *ROM = m_region_maincpu->base();
			if (m_bios_hack->read())
			{
				// patch out logo and checksum checks
				// (useful to run some pirate carts until we implement
				// their complete functionalities + to test homebrew)
				if (offset == 0xe9 || offset == 0xea)
					return 0x00;
				if (offset == 0xfa || offset == 0xfb)
					return 0x00;
			}
			return ROM[offset];
		}
		else if (m_cartslot)
		{
			return m_cartslot->read_rom(space, offset);
		}
		else
			return 0xff;
	}
}

READ8_MEMBER(gb_state::gbc_cart_r)
{
	if (m_bios_disable && m_cartslot)
		return m_cartslot->read_rom(space, offset);
	else
	{
		if (offset < 0x100)
		{
			UINT8 *ROM = m_region_maincpu->base();
			if (m_bios_hack->read())
			{
				// patch out logo and checksum checks
				// (useful to run some pirate carts until we implement
				// their complete functionalities + to test homebrew)
				if (offset == 0xdb || offset == 0xdc)
					return 0x00;
				if (offset == 0xed || offset == 0xee)
					return 0x00;
			}
			return ROM[offset];
		}
		else if (offset >= 0x200 && offset < 0x900)
		{
			UINT8 *ROM = m_region_maincpu->base();
			return ROM[offset - 0x100];
		}
		else if (m_cartslot)
		{
			return m_cartslot->read_rom(space, offset);
		}
		else
			return 0xff;
	}
}

WRITE8_MEMBER(gb_state::gb_bank_w)
{
	if (m_cartslot)
		m_cartslot->write_bank(space, offset, data);
}

READ8_MEMBER(gb_state::gb_ram_r)
{
	if (m_cartslot)
		return m_cartslot->read_ram(space, offset);
	else
		return 0xff;
}

WRITE8_MEMBER(gb_state::gb_ram_w)
{
	if (m_cartslot)
		m_cartslot->write_ram(space, offset, data);
}

READ8_MEMBER(gb_state::gb_echo_r)
{
	return space.read_byte(0xc000 + offset);
}

WRITE8_MEMBER(gb_state::gb_echo_w)
{
	return space.write_byte(0xc000 + offset, data);
}

READ8_MEMBER(megaduck_state::cart_r)
{
	if (m_cartslot)
		return m_cartslot->read_rom(space, offset);
	else
		return 0xff;
}

WRITE8_MEMBER(megaduck_state::bank1_w)
{
	if (m_cartslot)
		m_cartslot->write_bank(space, offset, data);
}

WRITE8_MEMBER(megaduck_state::bank2_w)
{
	if (m_cartslot)
		m_cartslot->write_ram(space, offset, data); /* used for bankswitch, but we re-use GB name */
}


static ADDRESS_MAP_START(gameboy_map, AS_PROGRAM, 8, gb_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_READWRITE(gb_cart_r, gb_bank_w)
	AM_RANGE(0x8000, 0x9fff) AM_DEVREADWRITE("lcd", gb_lcd_device, vram_r, vram_w)  /* 8k VRAM */
	AM_RANGE(0xa000, 0xbfff) AM_READWRITE(gb_ram_r, gb_ram_w)    /* 8k switched RAM bank (cartridge) */
	AM_RANGE(0xc000, 0xdfff) AM_RAM                               /* 8k low RAM */
	AM_RANGE(0xe000, 0xfdff) AM_READWRITE(gb_echo_r, gb_echo_w)  /* echo RAM */
	AM_RANGE(0xfe00, 0xfeff) AM_DEVREADWRITE("lcd", gb_lcd_device, oam_r, oam_w)    /* OAM RAM */
	AM_RANGE(0xff00, 0xff0f) AM_READWRITE(gb_io_r, gb_io_w)      /* I/O */
	AM_RANGE(0xff10, 0xff26) AM_DEVREADWRITE("custom", gameboy_sound_device, sound_r, sound_w)      /* sound registers */
	AM_RANGE(0xff27, 0xff2f) AM_NOP                     /* unused */
	AM_RANGE(0xff30, 0xff3f) AM_DEVREADWRITE("custom", gameboy_sound_device, wave_r, wave_w)        /* Wave ram */
	AM_RANGE(0xff40, 0xff7f) AM_DEVREAD("lcd", gb_lcd_device, video_r) AM_WRITE(gb_io2_w)     /* Video controller & BIOS flip-flop */
	AM_RANGE(0xff80, 0xfffe) AM_RAM                     /* High RAM */
	AM_RANGE(0xffff, 0xffff) AM_READWRITE(gb_ie_r, gb_ie_w)        /* Interrupt enable register */
ADDRESS_MAP_END

static ADDRESS_MAP_START(sgb_map, AS_PROGRAM, 8, gb_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_READWRITE(gb_cart_r, gb_bank_w)
	AM_RANGE(0x8000, 0x9fff) AM_DEVREADWRITE("lcd", sgb_lcd_device, vram_r, vram_w)  /* 8k VRAM */
	AM_RANGE(0xa000, 0xbfff) AM_READWRITE(gb_ram_r, gb_ram_w)    /* 8k switched RAM bank (cartridge) */
	AM_RANGE(0xc000, 0xdfff) AM_RAM                               /* 8k low RAM */
	AM_RANGE(0xe000, 0xfdff) AM_READWRITE(gb_echo_r, gb_echo_w)  /* echo RAM */
	AM_RANGE(0xfe00, 0xfeff) AM_DEVREADWRITE("lcd", sgb_lcd_device, oam_r, oam_w)    /* OAM RAM */
	AM_RANGE(0xff00, 0xff0f) AM_READWRITE(gb_io_r, sgb_io_w)     /* I/O */
	AM_RANGE(0xff10, 0xff26) AM_DEVREADWRITE("custom", gameboy_sound_device, sound_r, sound_w)      /* sound registers */
	AM_RANGE(0xff27, 0xff2f) AM_NOP                     /* unused */
	AM_RANGE(0xff30, 0xff3f) AM_DEVREADWRITE("custom", gameboy_sound_device, wave_r, wave_w)        /* Wave RAM */
	AM_RANGE(0xff40, 0xff7f) AM_DEVREAD("lcd", sgb_lcd_device, video_r) AM_WRITE(gb_io2_w)        /* Video controller & BIOS flip-flop */
	AM_RANGE(0xff80, 0xfffe) AM_RAM                     /* High RAM */
	AM_RANGE(0xffff, 0xffff) AM_READWRITE(gb_ie_r, gb_ie_w)        /* Interrupt enable register */
ADDRESS_MAP_END

static ADDRESS_MAP_START(gbc_map, AS_PROGRAM, 8, gb_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_READWRITE(gbc_cart_r, gb_bank_w)
	AM_RANGE(0x8000, 0x9fff) AM_DEVREADWRITE("lcd", cgb_lcd_device, vram_r, vram_w) /* 8k VRAM */
	AM_RANGE(0xa000, 0xbfff) AM_READWRITE(gb_ram_r, gb_ram_w)   /* 8k switched RAM bank (cartridge) */
	AM_RANGE(0xc000, 0xcfff) AM_RAM                     /* 4k fixed RAM bank */
	AM_RANGE(0xd000, 0xdfff) AM_RAMBANK("cgb_ram")                    /* 4k switched RAM bank */
	AM_RANGE(0xe000, 0xfdff) AM_READWRITE(gb_echo_r, gb_echo_w)  /* echo RAM */
	AM_RANGE(0xfe00, 0xfeff) AM_DEVREADWRITE("lcd", cgb_lcd_device, oam_r, oam_w)  /* OAM RAM */
	AM_RANGE(0xff00, 0xff0f) AM_READWRITE(gb_io_r, gb_io_w)        /* I/O */
	AM_RANGE(0xff10, 0xff26) AM_DEVREADWRITE("custom", gameboy_sound_device, sound_r, sound_w)      /* sound controller */
	AM_RANGE(0xff27, 0xff2f) AM_NOP                     /* unused */
	AM_RANGE(0xff30, 0xff3f) AM_DEVREADWRITE("custom", gameboy_sound_device, wave_r, wave_w)        /* Wave RAM */
	AM_RANGE(0xff40, 0xff7f) AM_READWRITE(gbc_io2_r, gbc_io2_w)        /* Other I/O and video controller */
	AM_RANGE(0xff80, 0xfffe) AM_RAM                     /* high RAM */
	AM_RANGE(0xffff, 0xffff) AM_READWRITE(gb_ie_r, gb_ie_w)        /* Interrupt enable register */
ADDRESS_MAP_END

static ADDRESS_MAP_START(megaduck_map, AS_PROGRAM, 8, megaduck_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_READWRITE(cart_r, bank1_w)
	AM_RANGE(0x8000, 0x9fff) AM_DEVREADWRITE("lcd", gb_lcd_device, vram_r, vram_w)        /* 8k VRAM */
	AM_RANGE(0xa000, 0xafff) AM_NOP                         /* unused? */
	AM_RANGE(0xb000, 0xb000) AM_WRITE(bank2_w)
	AM_RANGE(0xb001, 0xbfff) AM_NOP                         /* unused? */
	AM_RANGE(0xc000, 0xfe9f) AM_RAM                         /* 8k low RAM, echo RAM */
	AM_RANGE(0xfe00, 0xfeff) AM_DEVREADWRITE("lcd", gb_lcd_device, oam_r, oam_w)      /* OAM RAM */
	AM_RANGE(0xff00, 0xff0f) AM_READWRITE(gb_io_r, gb_io_w)            /* I/O */
	AM_RANGE(0xff10, 0xff1f) AM_READWRITE(megaduck_video_r, megaduck_video_w)  /* video controller */
	AM_RANGE(0xff20, 0xff2f) AM_READWRITE(megaduck_sound_r1, megaduck_sound_w1) /* sound controller pt1 */
	AM_RANGE(0xff30, 0xff3f) AM_DEVREADWRITE("custom", gameboy_sound_device, wave_r, wave_w)            /* wave ram */
	AM_RANGE(0xff40, 0xff46) AM_READWRITE(megaduck_sound_r2, megaduck_sound_w2) /* sound controller pt2 */
	AM_RANGE(0xff47, 0xff7f) AM_NOP                         /* unused */
	AM_RANGE(0xff80, 0xfffe) AM_RAM                         /* high RAM */
	AM_RANGE(0xffff, 0xffff) AM_READWRITE(gb_ie_r, gb_ie_w)            /* interrupt enable register */
ADDRESS_MAP_END

static GFXDECODE_START( gb )
GFXDECODE_END

static INPUT_PORTS_START( gameboy )
	PORT_START("INPUTS")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_NAME("Left")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_NAME("Right")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_NAME("Up")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_NAME("Down")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Button A")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Button B")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START) PORT_NAME("Start")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SELECT) PORT_NAME("Select")

	PORT_START("SKIP_CHECK")
	PORT_CONFNAME( 0x01, 0x00, "[HACK] Skip BIOS Logo check" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )

INPUT_PORTS_END

static SLOT_INTERFACE_START(gb_cart)
	SLOT_INTERFACE_INTERNAL("rom",         GB_STD_ROM)
	SLOT_INTERFACE_INTERNAL("rom_mbc1",    GB_ROM_MBC1)
	SLOT_INTERFACE_INTERNAL("rom_mbc1col", GB_ROM_MBC1)
	SLOT_INTERFACE_INTERNAL("rom_mbc2",    GB_ROM_MBC2)
	SLOT_INTERFACE_INTERNAL("rom_mbc3",    GB_ROM_MBC3)
	SLOT_INTERFACE_INTERNAL("rom_huc1",    GB_ROM_MBC3)
	SLOT_INTERFACE_INTERNAL("rom_huc3",    GB_ROM_MBC3)
	SLOT_INTERFACE_INTERNAL("rom_mbc5",    GB_ROM_MBC5)
	SLOT_INTERFACE_INTERNAL("rom_mbc6",    GB_ROM_MBC6)
	SLOT_INTERFACE_INTERNAL("rom_mbc7",    GB_ROM_MBC7)
	SLOT_INTERFACE_INTERNAL("rom_tama5",   GB_ROM_TAMA5)
	SLOT_INTERFACE_INTERNAL("rom_mmm01",   GB_ROM_MMM01)
	SLOT_INTERFACE_INTERNAL("rom_m161_m12",GB_ROM_M161_M12)
	SLOT_INTERFACE_INTERNAL("rom_sachen1", GB_ROM_SACHEN1)
	SLOT_INTERFACE_INTERNAL("rom_sachen2", GB_ROM_SACHEN2)
	SLOT_INTERFACE_INTERNAL("rom_wisdom",  GB_ROM_WISDOM)
	SLOT_INTERFACE_INTERNAL("rom_yong",    GB_ROM_YONG)
	SLOT_INTERFACE_INTERNAL("rom_lasama",  GB_ROM_LASAMA)
	SLOT_INTERFACE_INTERNAL("rom_atvrac",  GB_ROM_ATVRAC)
	SLOT_INTERFACE_INTERNAL("rom_camera",  GB_STD_ROM)
	SLOT_INTERFACE_INTERNAL("rom_188in1",  GB_ROM_188IN1)
	SLOT_INTERFACE_INTERNAL("rom_sintax",  GB_ROM_SINTAX)
	SLOT_INTERFACE_INTERNAL("rom_chong",   GB_ROM_CHONGWU)
	SLOT_INTERFACE_INTERNAL("rom_licheng", GB_ROM_LICHENG)
	SLOT_INTERFACE_INTERNAL("rom_digimon", GB_ROM_DIGIMON)
	SLOT_INTERFACE_INTERNAL("rom_rock8",   GB_ROM_ROCKMAN8)
	SLOT_INTERFACE_INTERNAL("rom_sm3sp",   GB_ROM_SM3SP)
//  SLOT_INTERFACE_INTERNAL("rom_dkong5",  GB_ROM_DKONG5)
//  SLOT_INTERFACE_INTERNAL("rom_unk01",   GB_ROM_UNK01)
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(megaduck_cart)
	SLOT_INTERFACE_INTERNAL("rom",  MEGADUCK_ROM)
SLOT_INTERFACE_END



static const unsigned char palette_gb[] =
{
	/* Simple black and white palette */
	/*  0xFF,0xFF,0xFF,
	 0xB0,0xB0,0xB0,
	 0x60,0x60,0x60,
	 0x00,0x00,0x00 */

	/* Possibly needs a little more green in it */
	0xFF,0xFB,0x87,     /* Background */
	0xB1,0xAE,0x4E,     /* Light */
	0x84,0x80,0x4E,     /* Medium */
	0x4E,0x4E,0x4E,     /* Dark */

	/* Palette for Game Boy Pocket/Light */
	0xC4,0xCF,0xA1,     /* Background */
	0x8B,0x95,0x6D,     /* Light      */
	0x6B,0x73,0x53,     /* Medium     */
	0x41,0x41,0x41,     /* Dark       */
};

static const unsigned char palette_megaduck[] = {
	0x6B, 0xA6, 0x4A, 0x43, 0x7A, 0x63, 0x25, 0x59, 0x55, 0x12, 0x42, 0x4C
};

/* Initialise the palettes */
PALETTE_INIT_MEMBER(gb_state, gb)
{
	for (int i = 0; i < 4; i++)
		palette.set_pen_color(i, palette_gb[i * 3 + 0], palette_gb[i * 3 + 1], palette_gb[i * 3 + 2]);
}

PALETTE_INIT_MEMBER(gb_state, gbp)
{
	for (int i = 0; i < 4; i++)
		palette.set_pen_color(i, palette_gb[(i + 4) * 3 + 0], palette_gb[(i + 4) * 3 + 1], palette_gb[(i + 4) * 3 + 2]);
}

PALETTE_INIT_MEMBER(gb_state, sgb)
{
	int r, g, b;

	for (int i = 0; i < 32768; i++)
	{
		r = (i & 0x1F) << 3;
		g = ((i >> 5) & 0x1F) << 3;
		b = ((i >> 10) & 0x1F) << 3;
		palette.set_pen_color(i, r, g, b);
	}
}

PALETTE_INIT_MEMBER(gb_state, gbc)
{
	int r, g, b;

	for (int i = 0; i < 32768; i++)
	{
		r = (i & 0x1F) << 3;
		g = ((i >> 5) & 0x1F) << 3;
		b = ((i >> 10) & 0x1F) << 3;
		palette.set_pen_color(i, r, g, b);
	}
}

PALETTE_INIT_MEMBER(megaduck_state, megaduck)
{
	for (int i = 0; i < 4; i++)
		palette.set_pen_color(i, palette_megaduck[i * 3 + 0], palette_megaduck[i * 3 + 1], palette_megaduck[i * 3 + 2]);
}


static MACHINE_CONFIG_START( gameboy, gb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", LR35902, XTAL_4_194304Mhz)
	MCFG_CPU_PROGRAM_MAP(gameboy_map)
	MCFG_LR35902_TIMER_CB( WRITE8( gb_state, gb_timer_callback ) )
	MCFG_LR35902_HALT_BUG

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(DMG_FRAMES_PER_SECOND)
	MCFG_SCREEN_VBLANK_TIME(0)
	MCFG_SCREEN_UPDATE_DEVICE("lcd", gb_lcd_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)
//  MCFG_SCREEN_SIZE(20*8, 18*8)
	MCFG_SCREEN_SIZE( 458, 154 )
	MCFG_SCREEN_VISIBLE_AREA(0*8, 20*8-1, 0*8, 18*8-1)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", gb)
	MCFG_PALETTE_ADD("palette", 4)
	MCFG_PALETTE_INIT_OWNER(gb_state,gb)

	MCFG_GB_LCD_DMG_ADD("lcd")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("custom", GAMEBOY, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	/* cartslot */
	MCFG_GB_CARTRIDGE_ADD("gbslot", gb_cart, NULL)

	MCFG_SOFTWARE_LIST_ADD("cart_list","gameboy")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("gbc_list","gbcolor")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( supergb, gameboy )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", LR35902, 4295454) /* 4.295454 MHz, derived from SNES xtal */
	MCFG_CPU_PROGRAM_MAP(sgb_map)

	MCFG_CPU_MODIFY("maincpu")
	MCFG_LR35902_TIMER_CB( WRITE8(gb_state, gb_timer_callback ) )
	MCFG_LR35902_HALT_BUG

	MCFG_MACHINE_START_OVERRIDE(gb_state, sgb)
	MCFG_MACHINE_RESET_OVERRIDE(gb_state, sgb)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_horizont) /* runs on a TV, not an LCD */

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(32*8, 28*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(32768)
	MCFG_PALETTE_INIT_OWNER(gb_state,sgb)

	MCFG_DEVICE_REMOVE("lcd")
	MCFG_GB_LCD_SGB_ADD("lcd")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( supergb2, gameboy )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sgb_map)

	MCFG_CPU_MODIFY("maincpu")
	MCFG_LR35902_TIMER_CB( WRITE8(gb_state, gb_timer_callback ) )
	MCFG_LR35902_HALT_BUG

	MCFG_MACHINE_START_OVERRIDE(gb_state, sgb)
	MCFG_MACHINE_RESET_OVERRIDE(gb_state, sgb)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_horizont) /* runs on a TV, not an LCD */

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(32*8, 28*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(32768)
	MCFG_PALETTE_INIT_OWNER(gb_state,sgb)

	MCFG_DEVICE_REMOVE("lcd")
	MCFG_GB_LCD_SGB_ADD("lcd")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gbpocket, gameboy )

	/* video hardware */
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(gb_state,gbp)

	MCFG_DEVICE_REMOVE("lcd")
	MCFG_GB_LCD_MGB_ADD("lcd")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( gbcolor, gb_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", LR35902, XTAL_4_194304Mhz) // todo XTAL_8_388MHz
	MCFG_CPU_PROGRAM_MAP(gbc_map)
	MCFG_LR35902_TIMER_CB( WRITE8(gb_state, gb_timer_callback ) )

	MCFG_MACHINE_START_OVERRIDE(gb_state,gbc)
	MCFG_MACHINE_RESET_OVERRIDE(gb_state,gbc)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(DMG_FRAMES_PER_SECOND)
	MCFG_SCREEN_VBLANK_TIME(0)
	MCFG_SCREEN_UPDATE_DEVICE("lcd", gb_lcd_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)
//  MCFG_SCREEN_SIZE(20*8, 18*8)
	MCFG_SCREEN_SIZE( 458, 154 )
	MCFG_SCREEN_VISIBLE_AREA(0*8, 20*8-1, 0*8, 18*8-1)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", gb)

	MCFG_PALETTE_ADD("palette", 32768)
	MCFG_PALETTE_INIT_OWNER(gb_state,gbc)

	MCFG_GB_LCD_CGB_ADD("lcd")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("custom", GAMEBOY, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("48K") /* 2 pages of 8KB VRAM, 8 pages of 4KB RAM */

	/* cartslot */
	MCFG_GB_CARTRIDGE_ADD("gbslot", gb_cart, NULL)

	MCFG_SOFTWARE_LIST_ADD("cart_list","gbcolor")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("gb_list","gameboy")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( megaduck, megaduck_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", LR35902, 4194304) /* 4.194304 MHz */
	MCFG_CPU_PROGRAM_MAP(megaduck_map)
	MCFG_LR35902_TIMER_CB( WRITE8(gb_state, gb_timer_callback ) )
	MCFG_LR35902_HALT_BUG

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(DMG_FRAMES_PER_SECOND)
	MCFG_SCREEN_VBLANK_TIME(0)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_MACHINE_START_OVERRIDE(megaduck_state, megaduck)
	MCFG_MACHINE_RESET_OVERRIDE(megaduck_state, megaduck)

	MCFG_SCREEN_UPDATE_DEVICE("lcd", gb_lcd_device, screen_update)
	MCFG_SCREEN_SIZE(20*8, 18*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 20*8-1, 0*8, 18*8-1)

	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", gb)

	MCFG_PALETTE_ADD("palette", 4)
	MCFG_PALETTE_INIT_OWNER(megaduck_state,megaduck)

	MCFG_GB_LCD_DMG_ADD("lcd")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("custom", GAMEBOY, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	/* cartslot */
	MCFG_MEGADUCK_CARTRIDGE_ADD("duckslot", megaduck_cart, NULL)
	MCFG_SOFTWARE_LIST_ADD("cart_list", "megaduck")
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

ROM_START( supergb2 )
	ROM_REGION( 0x0100, "maincpu", 0 )
	ROM_LOAD( "sgb2_boot.bin", 0x0000, 0x0100, CRC(53d0dd63) SHA1(93407ea10d2f30ab96a314d8eca44fe160aea734) )
ROM_END

ROM_START( gbpocket )
	ROM_REGION( 0x0100, "maincpu", 0 )
	ROM_LOAD( "mgb_boot.bin", 0x0000, 0x0100, CRC(e6920754) SHA1(4e68f9da03c310e84c523654b9026e51f26ce7f0) )
ROM_END

ROM_START( gbcolor )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "gbc_boot.1", 0x0000, 0x0100, CRC(779ea374) SHA1(e4b40c9fd593a97a1618cfb2696f290cf9596a62) ) /* Bootstrap code part 1 */
	ROM_LOAD( "gbc_boot.2", 0x0100, 0x0700, CRC(f741807d) SHA1(f943b1e0b640cf1d371e1d8f0ada69af03ebb396) ) /* Bootstrap code part 2 */
ROM_END


ROM_START( megaduck )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
ROM_END

/*    YEAR  NAME      PARENT   COMPAT   MACHINE   INPUT    INIT  COMPANY     FULLNAME */
CONS( 1990, gameboy,  0,       0,       gameboy,  gameboy, driver_device, 0,    "Nintendo", "Game Boy", MACHINE_SUPPORTS_SAVE )
CONS( 1994, supergb,  gameboy, 0,       supergb,  gameboy, driver_device, 0,    "Nintendo", "Super Game Boy", MACHINE_SUPPORTS_SAVE )
CONS( 1998, supergb2, gameboy, 0,       supergb2, gameboy, driver_device, 0,    "Nintendo", "Super Game Boy 2", MACHINE_SUPPORTS_SAVE )
CONS( 1996, gbpocket, gameboy, 0,       gbpocket, gameboy, driver_device, 0,    "Nintendo", "Game Boy Pocket", MACHINE_SUPPORTS_SAVE )
CONS( 1998, gbcolor,  0,       0,       gbcolor,  gameboy, driver_device, 0,    "Nintendo", "Game Boy Color", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

// Sound is not 100% yet, it generates some sounds which could be ok. Since we're lacking a real system there's no way to verify.
CONS( 1993, megaduck, 0,       0,       megaduck, gameboy, driver_device, 0,    "Welback Holdings (Timlex International) / Creatronic / Videojet / Cougar USA", "Mega Duck / Cougar Boy", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
