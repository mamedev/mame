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


Mappers used in the Game Boy
===========================

MBC1 Mapper
===========

The MBC1 mapper has two modes: 2MB ROM/8KB RAM or 512KB ROM/32KB RAM.
Initially, the mapper operates in 2MB ROM/8KB RAM mode.

0000-1FFF - Writing to this area enables (value 0x0A) or disables (not 0x0A) the
            SRAM.
2000-3FFF - Writing a value 0bXXXBBBBB into the 2000-3FFF memory area selects the
            lower 5 bits of the ROM bank to select for the 4000-7FFF memory area.
            If a value of 0bXXX00000 is written then this will autmatically be
            changed to 0bXXX00001 by the mbc chip. Initial value 00.
4000-5FFF - Writing a value 0bXXXXXXBB into the 4000-5FFF memory area either selects
            the RAM bank to use or bits 6 and 7 for the ROM bank to use for the 4000-7FFF
            memory area. This behaviour depends on the memory moddel chosen.
            These address lines are fixed in mode 1 and switch depending on A14 in mode 0.
            In mode 0 these will drive 0 when RB 00 is accessed (A14 low) or the value set
            in 4000-5FFF when RB <> 00 is accessed (A14 high).
            Switching between modes does not clear this register. Initial value 00.
6000-7FFF - Writing a value 0bXXXXXXXB into the 6000-7FFF memory area switches the mode.
            B=0 - 2MB ROM/8KB RAM mode
            B=1 - 512KB ROM/32KB RAM mode

Regular ROM aliasing rules apply.

MBC2 Mapper
===========

The MBC2 mapper includes 512x4bits of builtin RAM.

0000-3FFF - Writing to this area enables (value 0bXXXX1010) or disables (any
            other value than 0bXXXX1010) the RAM. In order to perform this
            function bit 8 of the address must be reset, so usable areas are
            0000-00FF, 0200-02FF, 0400-04FF, 0600-06FF, ..., 3E00-3EFF,
0000-3FFF - Writing to this area selects the rom bank to appear at 4000-7FFF.
            Only bits 3-0 are used to select the bank number. If a value of
            0bXXXX0000 is written then this is automatically changed into
            0bXXXX0001 by the mapper.
            In order to perform the rom banking bit 8 of the address must be
            set, so usable areas are 0100-01FF, 0300-03FF, 0500-05FF, 0700-
            07FF,..., 3F00-3FFF,

Regular ROM aliasing rules apply.

MBC3 Mapper
===========

The MBC3 mapper cartridges can include a RTC chip.

0000-1FFF - Writing to this area enables (value 0x0A) or disables (not 0x0A) the
            SRAM and RTC registers.
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

Regular ROM aliasing rules apply.

MBC5 Mapper
===========

0000-1FFF - Writing to this area enables (0x0A) or disables (not 0x0A) the SRAM area.
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
#include "includes/gb.h"
#include "bus/gameboy/rom.h"
#include "bus/gameboy/mbc.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"


#define DMG_FRAMES_PER_SECOND   59.732155
#define SGB_FRAMES_PER_SECOND   61.17


READ8_MEMBER(gb_state::gb_cart_r)
{
	if (m_bios_disable && m_cartslot)
		return m_cartslot->read_rom(space, offset);
	else
	{
		if (offset < 0x100)
		{
			uint8_t *ROM = m_region_maincpu->base();
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
			uint8_t *ROM = m_region_maincpu->base();
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
			uint8_t *ROM = m_region_maincpu->base();
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


void gb_state::gameboy_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rw(FUNC(gb_state::gb_cart_r), FUNC(gb_state::gb_bank_w));
	map(0x8000, 0x9fff).rw(m_ppu, FUNC(dmg_ppu_device::vram_r), FUNC(dmg_ppu_device::vram_w));          /* 8k VRAM */
	map(0xa000, 0xbfff).rw(FUNC(gb_state::gb_ram_r), FUNC(gb_state::gb_ram_w));                                /* 8k switched RAM bank (cartridge) */
	map(0xc000, 0xdfff).ram();                                                          /* 8k low RAM */
	map(0xe000, 0xfdff).rw(FUNC(gb_state::gb_echo_r), FUNC(gb_state::gb_echo_w));
	map(0xfe00, 0xfeff).rw(m_ppu, FUNC(dmg_ppu_device::oam_r), FUNC(dmg_ppu_device::oam_w));            /* OAM RAM */
	map(0xff00, 0xff0f).rw(FUNC(gb_state::gb_io_r), FUNC(gb_state::gb_io_w));                                  /* I/O */
	map(0xff10, 0xff26).rw(m_apu, FUNC(gameboy_sound_device::sound_r), FUNC(gameboy_sound_device::sound_w));  /* sound registers */
	map(0xff27, 0xff2f).noprw();                                                          /* unused */
	map(0xff30, 0xff3f).rw(m_apu, FUNC(gameboy_sound_device::wave_r), FUNC(gameboy_sound_device::wave_w));    /* Wave ram */
	map(0xff40, 0xff7f).r(m_ppu, FUNC(dmg_ppu_device::video_r)).w(FUNC(gb_state::gb_io2_w));   /* Video controller & BIOS flip-flop */
	map(0xff80, 0xfffe).ram();                                                          /* High RAM */
	map(0xffff, 0xffff).rw(FUNC(gb_state::gb_ie_r), FUNC(gb_state::gb_ie_w));                                  /* Interrupt enable register */
}

void gb_state::sgb_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rw(FUNC(gb_state::gb_cart_r), FUNC(gb_state::gb_bank_w));
	map(0x8000, 0x9fff).rw(m_ppu, FUNC(sgb_ppu_device::vram_r), FUNC(sgb_ppu_device::vram_w));          /* 8k VRAM */
	map(0xa000, 0xbfff).rw(FUNC(gb_state::gb_ram_r), FUNC(gb_state::gb_ram_w));                                /* 8k switched RAM bank (cartridge) */
	map(0xc000, 0xdfff).ram();                                                          /* 8k low RAM */
	map(0xe000, 0xfdff).rw(FUNC(gb_state::gb_echo_r), FUNC(gb_state::gb_echo_w));
	map(0xfe00, 0xfeff).rw(m_ppu, FUNC(sgb_ppu_device::oam_r), FUNC(sgb_ppu_device::oam_w));            /* OAM RAM */
	map(0xff00, 0xff0f).rw(FUNC(gb_state::gb_io_r), FUNC(gb_state::sgb_io_w));                                 /* I/O */
	map(0xff10, 0xff26).rw(m_apu, FUNC(gameboy_sound_device::sound_r), FUNC(gameboy_sound_device::sound_w));  /* sound registers */
	map(0xff27, 0xff2f).noprw();                                                          /* unused */
	map(0xff30, 0xff3f).rw(m_apu, FUNC(gameboy_sound_device::wave_r), FUNC(gameboy_sound_device::wave_w));    /* Wave RAM */
	map(0xff40, 0xff7f).r(m_ppu, FUNC(sgb_ppu_device::video_r)).w(FUNC(gb_state::gb_io2_w));   /* Video controller & BIOS flip-flop */
	map(0xff80, 0xfffe).ram();                                                          /* High RAM */
	map(0xffff, 0xffff).rw(FUNC(gb_state::gb_ie_r), FUNC(gb_state::gb_ie_w));                                  /* Interrupt enable register */
}

void gb_state::gbc_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rw(FUNC(gb_state::gbc_cart_r), FUNC(gb_state::gb_bank_w));
	map(0x8000, 0x9fff).rw(m_ppu, FUNC(cgb_ppu_device::vram_r), FUNC(cgb_ppu_device::vram_w));          /* 8k banked VRAM */
	map(0xa000, 0xbfff).rw(FUNC(gb_state::gb_ram_r), FUNC(gb_state::gb_ram_w));                                /* 8k switched RAM bank (cartridge) */
	map(0xc000, 0xcfff).ram();                                                          /* 4k fixed RAM bank */
	map(0xd000, 0xdfff).bankrw("cgb_ram");                                           /* 4k switched RAM bank */
	map(0xe000, 0xfdff).rw(FUNC(gb_state::gb_echo_r), FUNC(gb_state::gb_echo_w));
	map(0xfe00, 0xfeff).rw(m_ppu, FUNC(cgb_ppu_device::oam_r), FUNC(cgb_ppu_device::oam_w));            /* OAM RAM */
	map(0xff00, 0xff0f).rw(FUNC(gb_state::gb_io_r), FUNC(gb_state::gbc_io_w));                                 /* I/O */
	map(0xff10, 0xff26).rw(m_apu, FUNC(gameboy_sound_device::sound_r), FUNC(gameboy_sound_device::sound_w));  /* sound controller */
	map(0xff27, 0xff2f).noprw();                                                          /* unused */
	map(0xff30, 0xff3f).rw(m_apu, FUNC(gameboy_sound_device::wave_r), FUNC(gameboy_sound_device::wave_w));    /* Wave RAM */
	map(0xff40, 0xff7f).rw(FUNC(gb_state::gbc_io2_r), FUNC(gb_state::gbc_io2_w));                              /* Other I/O and video controller */
	map(0xff80, 0xfffe).ram();                                                          /* high RAM */
	map(0xffff, 0xffff).rw(FUNC(gb_state::gb_ie_r), FUNC(gb_state::gb_ie_w));                                  /* Interrupt enable register */
}

void megaduck_state::megaduck_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rw(FUNC(megaduck_state::cart_r), FUNC(megaduck_state::bank1_w));
	map(0x8000, 0x9fff).rw(m_ppu, FUNC(dmg_ppu_device::vram_r), FUNC(dmg_ppu_device::vram_w));          /* 8k VRAM */
	map(0xa000, 0xafff).noprw();                                                          /* unused? */
	map(0xb000, 0xb000).w(FUNC(megaduck_state::bank2_w));
	map(0xb001, 0xbfff).noprw();                                                          /* unused? */
	map(0xc000, 0xfdff).ram();                                                          /* 8k/16k? RAM */
	map(0xfe00, 0xfeff).rw(m_ppu, FUNC(dmg_ppu_device::oam_r), FUNC(dmg_ppu_device::oam_w));            /* OAM RAM */
	map(0xff00, 0xff0f).rw(FUNC(megaduck_state::gb_io_r), FUNC(megaduck_state::gb_io_w));                                  /* I/O */
	map(0xff10, 0xff1f).rw(FUNC(megaduck_state::megaduck_video_r), FUNC(megaduck_state::megaduck_video_w));                /* video controller */
	map(0xff20, 0xff2f).rw(FUNC(megaduck_state::megaduck_sound_r1), FUNC(megaduck_state::megaduck_sound_w1));              /* sound controller pt1 */
	map(0xff30, 0xff3f).rw(m_apu, FUNC(gameboy_sound_device::wave_r), FUNC(gameboy_sound_device::wave_w));    /* wave ram */
	map(0xff40, 0xff46).rw(FUNC(megaduck_state::megaduck_sound_r2), FUNC(megaduck_state::megaduck_sound_w2));              /* sound controller pt2 */
	map(0xff47, 0xff7f).noprw();                                                          /* unused */
	map(0xff80, 0xfffe).ram();                                                          /* high RAM */
	map(0xffff, 0xffff).rw(FUNC(megaduck_state::gb_ie_r), FUNC(megaduck_state::gb_ie_w));                                  /* interrupt enable register */
}


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

static void gb_cart(device_slot_interface &device)
{
	device.option_add_internal("rom",         GB_STD_ROM);
	device.option_add_internal("rom_mbc1",    GB_ROM_MBC1);
	device.option_add_internal("rom_mbc1col", GB_ROM_MBC1);
	device.option_add_internal("rom_mbc2",    GB_ROM_MBC2);
	device.option_add_internal("rom_mbc3",    GB_ROM_MBC3);
	device.option_add_internal("rom_huc1",    GB_ROM_MBC3);
	device.option_add_internal("rom_huc3",    GB_ROM_MBC3);
	device.option_add_internal("rom_mbc5",    GB_ROM_MBC5);
	device.option_add_internal("rom_mbc6",    GB_ROM_MBC6);
	device.option_add_internal("rom_mbc7",    GB_ROM_MBC7);
	device.option_add_internal("rom_tama5",   GB_ROM_TAMA5);
	device.option_add_internal("rom_mmm01",   GB_ROM_MMM01);
	device.option_add_internal("rom_m161",    GB_ROM_M161);
	device.option_add_internal("rom_sachen1", GB_ROM_SACHEN1);
	device.option_add_internal("rom_sachen2", GB_ROM_SACHEN2);
	device.option_add_internal("rom_wisdom",  GB_ROM_WISDOM);
	device.option_add_internal("rom_yong",    GB_ROM_YONG);
	device.option_add_internal("rom_lasama",  GB_ROM_LASAMA);
	device.option_add_internal("rom_atvrac",  GB_ROM_ATVRAC);
	device.option_add_internal("rom_camera",  GB_ROM_CAMERA);
	device.option_add_internal("rom_188in1",  GB_ROM_188IN1);
	device.option_add_internal("rom_sintax",  GB_ROM_SINTAX);
	device.option_add_internal("rom_chong",   GB_ROM_CHONGWU);
	device.option_add_internal("rom_licheng", GB_ROM_LICHENG);
	device.option_add_internal("rom_digimon", GB_ROM_DIGIMON);
	device.option_add_internal("rom_rock8",   GB_ROM_ROCKMAN8);
	device.option_add_internal("rom_sm3sp",   GB_ROM_SM3SP);
//  device.option_add_internal("rom_dkong5",  GB_ROM_DKONG5);
//  device.option_add_internal("rom_unk01",   GB_ROM_UNK01);
}

static void megaduck_cart(device_slot_interface &device)
{
	device.option_add_internal("rom",  MEGADUCK_ROM);
}



static constexpr rgb_t palette_gb[] =
{
	// Simple black and white palette
	/*  0xff,0xff,0xff,
	 0xb0,0xb0,0xb0,
	 0x60,0x60,0x60,
	 0x00,0x00,0x00 */

	// Possibly needs a little more green in it
	{ 0xff,0xfb,0x87 },     // Background
	{ 0xb1,0xae,0x4e },     // Light
	{ 0x84,0x80,0x4e },     // Medium
	{ 0x4e,0x4e,0x4e },     // Dark

	// Palette for Game Boy Pocket/Light
	{ 0xc4,0xcf,0xa1 },     // Background
	{ 0x8b,0x95,0x6d },     // Light
	{ 0x6b,0x73,0x53 },     // Medium
	{ 0x41,0x41,0x41 },     // Dark
};

static constexpr rgb_t palette_megaduck[] = {
	{ 0x6b, 0xa6, 0x4a }, { 0x43, 0x7a, 0x63 }, { 0x25, 0x59, 0x55 }, { 0x12, 0x42, 0x4c }
};

// Initialise the palettes
void gb_state::gb_palette(palette_device &palette) const
{
	for (int i = 0; i < 4; i++)
		palette.set_pen_color(i, palette_gb[i]);
}

void gb_state::gbp_palette(palette_device &palette) const
{
	for (int i = 0; i < 4; i++)
		palette.set_pen_color(i, palette_gb[i + 4]);
}

void gb_state::sgb_palette(palette_device &palette) const
{
	for (int i = 0; i < 32768; i++)
	{
		int const r = i & 0x1f;
		int const g = (i >> 5) & 0x1f;
		int const b = (i >> 10) & 0x1f;
		palette.set_pen_color(i, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

void gb_state::gbc_palette(palette_device &palette) const
{
	for (int i = 0; i < 32768; i++)
	{
		int const r = i & 0x1f;
		int const g = (i >> 5) & 0x1f;
		int const b = (i >> 10) & 0x1f;
		palette.set_pen_color(i, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

void megaduck_state::megaduck_palette(palette_device &palette) const
{
	for (int i = 0; i < 4; i++)
		palette.set_pen_color(i, palette_megaduck[i]);
}


void gb_state::gameboy(machine_config &config)
{
	/* basic machine hardware */
	LR35902(config, m_maincpu, XTAL(4'194'304));
	m_maincpu->set_addrmap(AS_PROGRAM, &gb_state::gameboy_map);
	m_maincpu->timer_cb().set(FUNC(gb_state::gb_timer_callback));
	m_maincpu->set_halt_bug(true);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(DMG_FRAMES_PER_SECOND);
	screen.set_vblank_time(0);
	screen.set_screen_update("ppu", FUNC(dmg_ppu_device::screen_update));
	screen.set_palette(m_palette);
//  screen.set_size(20*8, 18*8);
	screen.set_size(458, 154);
	screen.set_visarea(0*8, 20*8-1, 0*8, 18*8-1);

	GFXDECODE(config, "gfxdecode", m_palette, gfxdecode_device::empty);
	PALETTE(config, m_palette, FUNC(gb_state::gb_palette), 4);

	DMG_PPU(config, m_ppu, m_maincpu);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	DMG_APU(config, m_apu, XTAL(4'194'304));
	m_apu->add_route(0, "lspeaker", 0.50);
	m_apu->add_route(1, "rspeaker", 0.50);

	/* cartslot */
	GB_CART_SLOT(config, m_cartslot, gb_cart, nullptr);

	SOFTWARE_LIST(config, "cart_list").set_original("gameboy");
	SOFTWARE_LIST(config, "gbc_list").set_compatible("gbcolor");
}

void gb_state::supergb(machine_config &config)
{
	/* basic machine hardware */
	LR35902(config, m_maincpu, 4295454); /* 4.295454 MHz, derived from SNES xtal */
	m_maincpu->set_addrmap(AS_PROGRAM, &gb_state::sgb_map);
	m_maincpu->timer_cb().set(FUNC(gb_state::gb_timer_callback));
	m_maincpu->set_halt_bug(true);

	MCFG_MACHINE_START_OVERRIDE(gb_state, sgb)
	MCFG_MACHINE_RESET_OVERRIDE(gb_state, sgb)

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_physical_aspect(4, 3); // runs on a TV, not an LCD
	screen.set_refresh_hz(SGB_FRAMES_PER_SECOND);
	screen.set_vblank_time(0);
	screen.set_screen_update("ppu", FUNC(dmg_ppu_device::screen_update));
	screen.set_palette(m_palette);
	screen.set_size(32*8, 28*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 28*8-1);

	GFXDECODE(config, "gfxdecode", m_palette, gfxdecode_device::empty);
	PALETTE(config, m_palette, FUNC(gb_state::sgb_palette), 32768);

	SGB_PPU(config, m_ppu, m_maincpu);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	DMG_APU(config, m_apu, 4295454);
	m_apu->add_route(0, "lspeaker", 0.50);
	m_apu->add_route(1, "rspeaker", 0.50);

	/* cartslot */
	GB_CART_SLOT(config, m_cartslot, gb_cart, nullptr);

	SOFTWARE_LIST(config, "cart_list").set_original("gameboy");
	SOFTWARE_LIST(config, "gbc_list").set_compatible("gbcolor");
}

void gb_state::supergb2(machine_config &config)
{
	gameboy(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &gb_state::sgb_map);

	MCFG_MACHINE_START_OVERRIDE(gb_state, sgb)
	MCFG_MACHINE_RESET_OVERRIDE(gb_state, sgb)

	/* video hardware */
	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_physical_aspect(4, 3); // runs on a TV, not an LCD
	screen.set_size(32*8, 28*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 28*8-1);

	m_palette->set_entries(32768);
	m_palette->set_init(FUNC(gb_state::sgb_palette));

	SGB_PPU(config.replace(), m_ppu, m_maincpu);
}

void gb_state::gbpocket(machine_config &config)
{
	gameboy(config);

	/* video hardware */
	m_palette->set_init(FUNC(gb_state::gbp_palette));

	MGB_PPU(config.replace(), m_ppu, m_maincpu);
}

void gb_state::gbcolor(machine_config &config)
{
	/* basic machine hardware */
	LR35902(config, m_maincpu, XTAL(4'194'304)); // todo XTAL(8'388'000)
	m_maincpu->set_addrmap(AS_PROGRAM, &gb_state::gbc_map);
	m_maincpu->timer_cb().set(FUNC(gb_state::gb_timer_callback));

	MCFG_MACHINE_START_OVERRIDE(gb_state,gbc)
	MCFG_MACHINE_RESET_OVERRIDE(gb_state,gbc)

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(DMG_FRAMES_PER_SECOND);
	screen.set_vblank_time(0);
	screen.set_screen_update("ppu", FUNC(dmg_ppu_device::screen_update));
	screen.set_palette(m_palette);
//  screen.set_size(20*8, 18*8);
	screen.set_size(458, 154);
	screen.set_visarea(0*8, 20*8-1, 0*8, 18*8-1);

	GFXDECODE(config, "gfxdecode", m_palette, gfxdecode_device::empty);
	PALETTE(config, m_palette, FUNC(gb_state::gbc_palette), 32768);

	CGB_PPU(config, m_ppu, m_maincpu);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	CGB04_APU(config, m_apu, XTAL(4'194'304));
	m_apu->add_route(0, "lspeaker", 0.50);
	m_apu->add_route(1, "rspeaker", 0.50);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("48K"); /* 2 pages of 8KB VRAM, 8 pages of 4KB RAM */

	/* cartslot */
	GB_CART_SLOT(config, "gbslot", gb_cart, nullptr);

	SOFTWARE_LIST(config, "cart_list").set_original("gbcolor");
	SOFTWARE_LIST(config, "gb_list").set_compatible("gameboy");
}

void megaduck_state::megaduck(machine_config &config)
{
	/* basic machine hardware */
	LR35902(config, m_maincpu, XTAL(4'194'304)); /* 4.194304 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &megaduck_state::megaduck_map);
	m_maincpu->timer_cb().set(FUNC(gb_state::gb_timer_callback));
	m_maincpu->set_halt_bug(true);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(DMG_FRAMES_PER_SECOND);
	screen.set_vblank_time(0);
	screen.set_screen_update("ppu", FUNC(dmg_ppu_device::screen_update));
	screen.set_palette(m_palette);
	screen.set_size(20*8, 18*8);
	screen.set_visarea(0*8, 20*8-1, 0*8, 18*8-1);

	GFXDECODE(config, "gfxdecode", m_palette, gfxdecode_device::empty);
	PALETTE(config, m_palette, FUNC(megaduck_state::megaduck_palette), 4);

	DMG_PPU(config, m_ppu, m_maincpu);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	DMG_APU(config, m_apu, XTAL(4'194'304));
	m_apu->add_route(0, "lspeaker", 0.50);
	m_apu->add_route(1, "rspeaker", 0.50);

	/* cartslot */
	MEGADUCK_CART_SLOT(config, m_cartslot, megaduck_cart, nullptr);
	SOFTWARE_LIST(config, "cart_list").set_original("megaduck");
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(gameboy)
	ROM_REGION(0x0100, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "dmg", "DMG vX")
	ROMX_LOAD("dmg_boot.bin", 0x0000, 0x0100, CRC(59c8598e) SHA1(4ed31ec6b0b175bb109c0eb5fd3d193da823339f), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "dmg_v0", "DMG v0")
	ROMX_LOAD("dmg_v0.rom", 0x0000, 0x0100, CRC(c2f5cc97) SHA1(8bd501e31921e9601788316dbd3ce9833a97bcbc), ROM_BIOS(1))
ROM_END

ROM_START(supergb)
	ROM_REGION(0x0100, "maincpu", 0)
	ROM_LOAD("sgb_boot.bin", 0x0000, 0x0100, CRC(ec8a83b9) SHA1(aa2f50a77dfb4823da96ba99309085a3c6278515))
ROM_END

ROM_START(supergb2 )
	ROM_REGION(0x0100, "maincpu", 0)
	ROM_LOAD("sgb2_boot.bin", 0x0000, 0x0100, CRC(53d0dd63) SHA1(93407ea10d2f30ab96a314d8eca44fe160aea734))
ROM_END

ROM_START(gbpocket )
	ROM_REGION(0x0100, "maincpu", 0)
	ROM_LOAD("mgb_boot.bin", 0x0000, 0x0100, CRC(e6920754) SHA1(4e68f9da03c310e84c523654b9026e51f26ce7f0))
ROM_END

ROM_START(gbcolor)
	ROM_REGION(0x800, "maincpu", 0)
	ROM_LOAD("gbc_boot.1", 0x0000, 0x0100, CRC(779ea374) SHA1(e4b40c9fd593a97a1618cfb2696f290cf9596a62)) /* Bootstrap code part 1 */
	ROM_LOAD("gbc_boot.2", 0x0100, 0x0700, CRC(f741807d) SHA1(f943b1e0b640cf1d371e1d8f0ada69af03ebb396)) /* Bootstrap code part 2 */
ROM_END


ROM_START(megaduck)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
ROM_END

ROM_START(gamefgtr)
	ROM_REGION(0x0100, "maincpu", 0)
	ROM_LOAD("gamefgtr.bin", 0x0000, 0x0100, CRC(908ba8de) SHA1(a4a36f71bf1b3b587df620d48ae940af93a982a5))
ROM_END

/*   YEAR  NAME      PARENT   COMPAT   MACHINE   INPUT    STATE           INIT        COMPANY     FULLNAME */
CONS(1990, gameboy,  0,       0,       gameboy,  gameboy, gb_state,       empty_init, "Nintendo", "Game Boy", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
CONS(1994, supergb,  gameboy, 0,       supergb,  gameboy, gb_state,       empty_init, "Nintendo", "Super Game Boy", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
CONS(1998, supergb2, gameboy, 0,       supergb2, gameboy, gb_state,       empty_init, "Nintendo", "Super Game Boy 2", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
CONS(1996, gbpocket, gameboy, 0,       gbpocket, gameboy, gb_state,       empty_init, "Nintendo", "Game Boy Pocket", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
CONS(1998, gbcolor,  0,       0,       gbcolor,  gameboy, gb_state,       empty_init, "Nintendo", "Game Boy Color", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)

// Sound is not 100% yet, it generates some sounds which could be ok. Since we're lacking a real system there's no way to verify.
CONS(1993, megaduck, 0,       0,       megaduck, gameboy, megaduck_state, empty_init, "Welback Holdings (Timlex International) / Creatronic / Videojet / Cougar USA", "Mega Duck / Cougar Boy", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

// http://blog.gg8.se/wordpress/2012/11/11/gameboy-clone-game-fighter-teardown/
CONS(1993, gamefgtr, gameboy, 0,       gameboy,  gameboy, gb_state,       empty_init, "bootleg", "Game Fighter (bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
