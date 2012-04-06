/***************************************************************************

                          -= ESD 16 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)
                    additions by David Haywood


Main  CPU   :   M68000
Video Chips :   2 x ACTEL A40MX04 (84 Pin Square Socketed) or
                ESD CRTC99 (QFP240) & ACTEL A40MX04

Sound CPU   :   Z80
Sound Chips :   M6295 (AD-65)  +  YM3812 (U6612)  +  YM3014 (U6614)

---------------------------------------------------------------------------
Year + Game            PCB             Notes
---------------------------------------------------------------------------
98  Multi Champ        ESD 11-09-98   (also a year 1999 revision)
99  Multi Champ Deluxe ESD 08-26-1999 (also a year 2000 revision)
99  Head Panic         ESD 05-28-99   (All English version, copyright 1999)
00  Head Panic         ESD 08-26-1999 (All English version, copyright 2000)
00  Head Panic         ESD 08-26-1999 (with Fuuki, Story in Japanese)
00  Deluxe 5           ESD            (no date is marked on PCB)
00  Tang Tang          ESD            (no date is marked on PCB)
01  SWAT Police        ESD            (no date is marked on PCB)
---------------------------------------------------------------------------

Other ESD games:

3 Cushion Billiards (c) 2000 - Undumped
Tang Tang           (c) 2000 - Undumped ESD 05-28-99 version
Fire Hawk           (c) 2001 - see nmk16.c driver
Jumping Pop         (c) 2001 - see tumbleb.c driver

Head Panic
- Maybe the sprite code can be merged again, haven't checked yet.
- Nude / Bikini pics don't show in-game, even when set in test mode?

---------------------------------------------------------------------------

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "machine/eeprom.h"
#include "sound/okim6295.h"
#include "sound/3812intf.h"
#include "includes/esd16.h"
#include "video/decospr.h"

/***************************************************************************


                            Memory Maps - Main CPU


***************************************************************************/

WRITE16_MEMBER(esd16_state::esd16_spriteram_w)
{
	COMBINE_DATA(&m_spriteram[offset]);
}

WRITE16_MEMBER(esd16_state::esd16_sound_command_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_w(space, 0, data & 0xff);
		device_set_input_line(m_audio_cpu, 0, ASSERT_LINE);		// Generate an IRQ
		device_spin_until_time(&space.device(), attotime::from_usec(50));	// Allow the other CPU to reply
	}
}

/*
 Lines starting with an empty comment in the following MemoryReadAddress
 arrays are there for debug (e.g. the game does not read from those ranges
 AFAIK)
*/

static ADDRESS_MAP_START( multchmp_map, AS_PROGRAM, 16, esd16_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM																		// ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM																		// RAM
	AM_RANGE(0x200000, 0x2005ff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_SHARE("paletteram")	// Palette
/**/AM_RANGE(0x300000, 0x3007ff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)				// Sprites
	AM_RANGE(0x300800, 0x300807) AM_WRITE(esd16_spriteram_w)												// Sprites (Mirrored)
/**/AM_RANGE(0x400000, 0x403fff) AM_RAM_WRITE(esd16_vram_0_w) AM_BASE(m_vram_0)						// Layers
/**/AM_RANGE(0x420000, 0x423fff) AM_RAM_WRITE(esd16_vram_1_w) AM_BASE(m_vram_1)						//
/**/AM_RANGE(0x500000, 0x500003) AM_RAM AM_BASE(m_scroll_0)											// Scroll
/**/AM_RANGE(0x500004, 0x500007) AM_RAM AM_BASE(m_scroll_1)											//
	AM_RANGE(0x500008, 0x500009) AM_WRITEONLY AM_BASE(m_headpanic_platform_x)
	AM_RANGE(0x50000a, 0x50000b) AM_WRITEONLY AM_BASE(m_headpanic_platform_y)
	AM_RANGE(0x50000c, 0x50000d) AM_WRITENOP																// ??
	AM_RANGE(0x50000e, 0x50000f) AM_WRITEONLY AM_BASE(m_head_layersize)								// ??
	AM_RANGE(0x600002, 0x600003) AM_READ_PORT("P1_P2")
	AM_RANGE(0x600004, 0x600005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x600006, 0x600007) AM_READ_PORT("DSW")
	AM_RANGE(0x600008, 0x600009) AM_WRITE(esd16_tilemap0_color_w)											// Flip Screen + Tileamp0 palette banking
	AM_RANGE(0x60000a, 0x60000b) AM_WRITENOP															// ? 2
	AM_RANGE(0x60000c, 0x60000d) AM_WRITE(esd16_sound_command_w)											// To Sound CPU
	AM_RANGE(0x700008, 0x70000b) AM_READNOP															// ? Only read once
ADDRESS_MAP_END

WRITE16_MEMBER(esd16_state::hedpanic_platform_w)
{
	int offsets = m_headpanic_platform_x[0] + 0x40 * m_headpanic_platform_y[0];

	m_vram_1[offsets] = data;
	m_tilemap_1_16x16->mark_tile_dirty(offsets);
}


READ16_MEMBER(esd16_state::esd_eeprom_r)
{
	if (ACCESSING_BITS_8_15)
	{
		return ((m_eeprom->read_bit() & 0x01) << 15);
	}

//  logerror("(0x%06x) unk EEPROM read: %04x\n", cpu_get_pc(&space.device()), mem_mask);
	return 0;
}

WRITE16_MEMBER(esd16_state::esd_eeprom_w)
{
	if (ACCESSING_BITS_8_15)
		input_port_write(machine(), "EEPROMOUT", data, 0xffff);

//  logerror("(0x%06x) Unk EEPROM write: %04x %04x\n", cpu_get_pc(&space.device()), data, mem_mask);
}

static ADDRESS_MAP_START( hedpanic_map, AS_PROGRAM, 16, esd16_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM																		// ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM																		// RAM
	AM_RANGE(0x800000, 0x800fff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_SHARE("paletteram")	// Palette
	AM_RANGE(0x900000, 0x9007ff) AM_WRITEONLY AM_BASE_SIZE(m_spriteram, m_spriteram_size)		// Sprites
	AM_RANGE(0x900800, 0x900807) AM_WRITE(esd16_spriteram_w)												// Sprites (Mirrored)
	AM_RANGE(0xa00000, 0xa03fff) AM_WRITE(esd16_vram_0_w) AM_BASE(m_vram_0)							// Layers
	AM_RANGE(0xa20000, 0xa23fff) AM_WRITE(esd16_vram_1_w) AM_BASE(m_vram_1)							//
	AM_RANGE(0xa24000, 0xa27fff) AM_WRITE(esd16_vram_1_w) AM_BASE(m_vram_1)							// mirror?
	AM_RANGE(0xb00000, 0xb00003) AM_WRITEONLY AM_BASE(m_scroll_0)									// Scroll
	AM_RANGE(0xb00004, 0xb00007) AM_WRITEONLY AM_BASE(m_scroll_1)									//
	AM_RANGE(0xb00008, 0xb00009) AM_WRITEONLY AM_BASE(m_headpanic_platform_x)
	AM_RANGE(0xb0000a, 0xb0000b) AM_WRITEONLY AM_BASE(m_headpanic_platform_y)
	AM_RANGE(0xb0000c, 0xb0000d) AM_WRITENOP																// ??
	AM_RANGE(0xb0000e, 0xb0000f) AM_WRITEONLY AM_BASE(m_head_layersize)								// ??
	AM_RANGE(0xc00000, 0xc00001) AM_WRITENOP // IRQ Ack
	AM_RANGE(0xc00002, 0xc00003) AM_READ_PORT("P1_P2")
	AM_RANGE(0xc00004, 0xc00005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xc00006, 0xc00007) AM_READ(esd_eeprom_r)
	AM_RANGE(0xc00008, 0xc00009) AM_WRITE(esd16_tilemap0_color_w)	// Flip Screen + Tileamp0 palette banking
	AM_RANGE(0xc0000a, 0xc0000b) AM_WRITENOP	// ? 2 not checked
	AM_RANGE(0xc0000c, 0xc0000d) AM_WRITE(esd16_sound_command_w)	// To Sound CPU // ok
	AM_RANGE(0xc0000e, 0xc0000f) AM_WRITE(esd_eeprom_w)
	AM_RANGE(0xd00008, 0xd00009) AM_WRITE(hedpanic_platform_w)
ADDRESS_MAP_END

/* Multi Champ Deluxe, like Head Panic but different addresses */

static ADDRESS_MAP_START( mchampdx_map, AS_PROGRAM, 16, esd16_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM																		// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM																		// RAM
	AM_RANGE(0x300000, 0x303fff) AM_WRITE(esd16_vram_0_w) AM_BASE(m_vram_0)							// Layers
	AM_RANGE(0x320000, 0x323fff) AM_WRITE(esd16_vram_1_w) AM_BASE(m_vram_1)							//
	AM_RANGE(0x324000, 0x327fff) AM_WRITE(esd16_vram_1_w) AM_BASE(m_vram_1)							// mirror?
	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_SHARE("paletteram")	// Palette
	AM_RANGE(0x500000, 0x500001) AM_WRITENOP	// IRQ Ack
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("P1_P2")
	AM_RANGE(0x500004, 0x500005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x500006, 0x500007) AM_READ(esd_eeprom_r)
	AM_RANGE(0x500008, 0x500009) AM_WRITE(esd16_tilemap0_color_w)											// Flip Screen + Tileamp0 palette banking
	AM_RANGE(0x50000a, 0x50000b) AM_WRITENOP																// ? 2 not checked
	AM_RANGE(0x50000c, 0x50000d) AM_WRITE(esd16_sound_command_w)											// To Sound CPU // ok
	AM_RANGE(0x50000e, 0x50000f) AM_WRITE(esd_eeprom_w)
	AM_RANGE(0x600000, 0x6007ff) AM_WRITEONLY AM_BASE_SIZE(m_spriteram, m_spriteram_size)		// Sprites
	AM_RANGE(0x600800, 0x600807) AM_WRITE(esd16_spriteram_w)												// Sprites (Mirrored)
	AM_RANGE(0x700000, 0x700003) AM_WRITEONLY AM_BASE(m_scroll_0)									// Scroll
	AM_RANGE(0x700004, 0x700007) AM_WRITEONLY AM_BASE(m_scroll_1)									//
	AM_RANGE(0x700008, 0x700009) AM_WRITEONLY AM_BASE(m_headpanic_platform_x)							// not used in mchampdx?
	AM_RANGE(0x70000a, 0x70000b) AM_WRITEONLY AM_BASE(m_headpanic_platform_y)							// not used in mchampdx?
	AM_RANGE(0x70000c, 0x70000d) AM_WRITENOP																// ??
	AM_RANGE(0x70000e, 0x70000f) AM_WRITEONLY AM_BASE(m_head_layersize)								// ??
	AM_RANGE(0xd00008, 0xd00009) AM_WRITE(hedpanic_platform_w)												// not used in mchampdx?
ADDRESS_MAP_END

/* Tang Tang & Deluxe 5 - like the others but again with different addresses */

static ADDRESS_MAP_START( tangtang_map, AS_PROGRAM, 16, esd16_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM																		// ROM
	AM_RANGE(0x100000, 0x100fff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_SHARE("paletteram")	// RAM
	AM_RANGE(0x200000, 0x2007ff) AM_WRITEONLY AM_BASE_SIZE(m_spriteram, m_spriteram_size)		// Sprites
	AM_RANGE(0x200800, 0x200807) AM_WRITE(esd16_spriteram_w)												// Sprites (Mirrored)
	AM_RANGE(0x300000, 0x303fff) AM_WRITE(esd16_vram_0_w) AM_BASE(m_vram_0)							// Layers
	AM_RANGE(0x320000, 0x323fff) AM_WRITE(esd16_vram_1_w) AM_BASE(m_vram_1)							//
	AM_RANGE(0x324000, 0x327fff) AM_WRITE(esd16_vram_1_w) AM_BASE(m_vram_1)							// mirror?
	AM_RANGE(0x400000, 0x400003) AM_WRITEONLY AM_BASE(m_scroll_0)									// Scroll
	AM_RANGE(0x400004, 0x400007) AM_WRITEONLY AM_BASE(m_scroll_1)									//
	AM_RANGE(0x400008, 0x400009) AM_WRITEONLY AM_BASE(m_headpanic_platform_x)							// not used in mchampdx?
	AM_RANGE(0x40000a, 0x40000b) AM_WRITEONLY AM_BASE(m_headpanic_platform_y)							// not used in mchampdx?
	AM_RANGE(0x40000c, 0x40000d) AM_WRITENOP																// ??
	AM_RANGE(0x40000e, 0x40000f) AM_WRITEONLY AM_BASE(m_head_layersize)									// ??
	AM_RANGE(0x500000, 0x500001) AM_WRITENOP	// IRQ Ack
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("P1_P2")
	AM_RANGE(0x500004, 0x500005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x500006, 0x500007) AM_READ(esd_eeprom_r)
	AM_RANGE(0x500008, 0x500009) AM_WRITE(esd16_tilemap0_color_w)   										// Flip Screen + Tileamp0 palette banking
	AM_RANGE(0x50000a, 0x50000b) AM_WRITENOP																// ? 2 not checked
	AM_RANGE(0x50000c, 0x50000d) AM_WRITE(esd16_sound_command_w)											// To Sound CPU // ok
	AM_RANGE(0x50000e, 0x50000f) AM_WRITE(esd_eeprom_w)
	AM_RANGE(0x600008, 0x600009) AM_WRITE(hedpanic_platform_w)
	AM_RANGE(0x700000, 0x70ffff) AM_RAM																		// main ram
ADDRESS_MAP_END


/***************************************************************************


                            Memory Maps - Sound CPU


***************************************************************************/

WRITE8_MEMBER(esd16_state::esd16_sound_rombank_w)
{
	int bank = data & 0xf;
	if (data != bank)	logerror("CPU #1 - PC %04X: unknown bank bits: %02X\n", cpu_get_pc(&space.device()), data);
	if (bank >= 3)	bank += 1;
	memory_set_bank(machine(), "bank1", bank);
}

static ADDRESS_MAP_START( multchmp_sound_map, AS_PROGRAM, 8, esd16_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM								// ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")						// Banked ROM
	AM_RANGE(0xf800, 0xffff) AM_RAM								// RAM
ADDRESS_MAP_END

READ8_MEMBER(esd16_state::esd16_sound_command_r)
{

	/* Clear IRQ only after reading the command, or some get lost */
	device_set_input_line(m_audio_cpu, 0, CLEAR_LINE);
	return soundlatch_r(space, 0);
}

static ADDRESS_MAP_START( multchmp_sound_io_map, AS_IO, 8, esd16_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE_LEGACY("ymsnd", ym3812_w)	// YM3812
	AM_RANGE(0x02, 0x02) AM_DEVREADWRITE("oki", okim6295_device, read, write)	// M6295
	AM_RANGE(0x03, 0x03) AM_READ(esd16_sound_command_r)	// From Main CPU
	AM_RANGE(0x04, 0x04) AM_WRITENOP	// ? $00, $30
	AM_RANGE(0x05, 0x05) AM_WRITE(esd16_sound_rombank_w)	// ROM Bank
	AM_RANGE(0x06, 0x06) AM_NOP	// ? At the start / ? 1 (End of NMI routine)
ADDRESS_MAP_END


/***************************************************************************


                                Input Ports


***************************************************************************/

static INPUT_PORTS_START( multchmp )
	PORT_START("P1_P2")	// $600002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )	// Resets the test mode
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")	// $600005.b
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2   )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")	// $600006.w
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, "Coinage Type" )			PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Demo_Sounds ) )		PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Flip_Screen ) )		PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )	PORT_CONDITION("DSW", 0x0002, PORTCOND_EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )	PORT_CONDITION("DSW", 0x0002, PORTCOND_EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )	PORT_CONDITION("DSW", 0x0002, PORTCOND_EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )	PORT_CONDITION("DSW", 0x0002, PORTCOND_EQUALS, 0x0002)
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )	PORT_CONDITION("DSW", 0x0002, PORTCOND_EQUALS, 0x0002)
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )	PORT_CONDITION("DSW", 0x0002, PORTCOND_EQUALS, 0x0002)
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )	PORT_CONDITION("DSW", 0x0002, PORTCOND_EQUALS, 0x0002)
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )	PORT_CONDITION("DSW", 0x0002, PORTCOND_EQUALS, 0x0002)
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )	PORT_CONDITION("DSW", 0x0002, PORTCOND_EQUALS, 0x0002)
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )	PORT_CONDITION("DSW", 0x0002, PORTCOND_EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_4C ) )	PORT_CONDITION("DSW", 0x0002, PORTCOND_EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )	PORT_CONDITION("DSW", 0x0002, PORTCOND_EQUALS, 0x0000)

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )		PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0c00, "3" )
	PORT_DIPSETTING(      0x0800, "4" )
	PORT_DIPSETTING(      0x0400, "5" )
	PORT_DIPNAME( 0x1000, 0x1000, "Selectable Games" )		PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Free_Play ) )		PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( hedpanic )
	PORT_START("P1_P2")	// $600002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")	// $600005.b
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2   )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_SERVICE1  )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0040, IP_ACTIVE_LOW)
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_cs_line)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_clock_line)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, write_bit)
INPUT_PORTS_END


static INPUT_PORTS_START( swatpolc )
	PORT_START("P1_P2")	// $600002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")	// $600005.b
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2   )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_SERVICE1  )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0040, IP_ACTIVE_LOW)
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_cs_line)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_clock_line)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, write_bit)
INPUT_PORTS_END

/***************************************************************************


                            Graphics Layouts


***************************************************************************/

/* 16x16x5, made of four 8x8 tiles */
static const gfx_layout layout_16x16x5 =
{
	16,16,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(4,5),RGN_FRAC(3,5),RGN_FRAC(2,5),RGN_FRAC(1,5), RGN_FRAC(0,5) },
	{ STEP8(8*16,1), STEP8(0,1) },
	{ STEP16(0,8) },
	16*16
};


static const gfx_layout hedpanic_sprite_16x16x5 =
{
	16,16,
	RGN_FRAC(1,3),
	5,
	{   RGN_FRAC(2,3), RGN_FRAC(0,3), RGN_FRAC(0,3)+8, RGN_FRAC(1,3),RGN_FRAC(1,3)+8 },
	{ 256+0,256+1,256+2,256+3,256+4,256+5,256+6,256+7,0,1,2,3,4,5,6,7 },
	{ 0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	16*32,
};


/* 8x8x8 */
static const gfx_layout layout_8x8x8 =
{
	8,8,
	RGN_FRAC(1,4),
	8,
	{ STEP8(0,1) },
	{ RGN_FRAC(3,4)+0*8,RGN_FRAC(2,4)+0*8,RGN_FRAC(1,4)+0*8,RGN_FRAC(0,4)+0*8,
	  RGN_FRAC(3,4)+1*8,RGN_FRAC(2,4)+1*8,RGN_FRAC(1,4)+1*8,RGN_FRAC(0,4)+1*8 },
	{ STEP8(0,2*8) },
	8*8*2,
};

static const gfx_layout hedpanic_layout_8x8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,2*8,1*8,3*8,4*8,6*8,5*8,7*8 },
	{ 0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64 },
	64*8,
};

static const gfx_layout hedpanic_layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,2*8,1*8,3*8,4*8,6*8,5*8,7*8,
	  64*8+0*8,64*8+2*8,64*8+1*8,64*8+3*8,64*8+4*8,64*8+6*8,64*8+5*8,64*8+7*8 },
	{ 0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64,
	  128*8+0*64,128*8+1*64,128*8+2*64,128*8+3*64,128*8+4*64,128*8+5*64,128*8+6*64,128*8+7*64
	},
	256*8,
};



static GFXDECODE_START( esd16 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x5, 0x200, 8 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x8x8,   0x000, 2 ) // [1] Layers
	GFXDECODE_ENTRY( "gfx2", 0, hedpanic_layout_16x16x8,   0x000, 2 ) // [1] Layers
GFXDECODE_END


static GFXDECODE_START( hedpanic )
	GFXDECODE_ENTRY( "gfx1", 0, hedpanic_sprite_16x16x5, 0x200, 8 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, hedpanic_layout_8x8x8,   0x000, 4 ) // [1] Layers
	GFXDECODE_ENTRY( "gfx2", 0, hedpanic_layout_16x16x8, 0x000, 4 ) // [2] Layers
GFXDECODE_END

static GFXDECODE_START( tangtang )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x5, 0x200, 8 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, hedpanic_layout_8x8x8,   0x000, 4 ) // [1] Layers
	GFXDECODE_ENTRY( "gfx2", 0, hedpanic_layout_16x16x8, 0x000, 4 ) // [2] Layers
GFXDECODE_END

/***************************************************************************


                                Machine Drivers


***************************************************************************/

static MACHINE_START( esd16 )
{
	esd16_state *state = machine.driver_data<esd16_state>();
	UINT8 *AUDIO = machine.region("audiocpu")->base();

	memory_configure_bank(machine, "bank1", 0, 17, &AUDIO[0x0000], 0x4000);

	state->m_audio_cpu = machine.device("audiocpu");
	state->m_eeprom = machine.device<eeprom_device>("eeprom");

	state->save_item(NAME(state->m_tilemap0_color));
}

static MACHINE_RESET( esd16 )
{
	esd16_state *state = machine.driver_data<esd16_state>();

	state->m_tilemap0_color = 0;
}

static UINT16 hedpanic_pri_callback(UINT16 x)
{
	if (x & 0x8000)
		return 0xfffe; // under "tilemap 1"
	else
		return 0; // above everything
}

static MACHINE_CONFIG_START( esd16, esd16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000, 16000000)
	MCFG_CPU_PROGRAM_MAP(multchmp_map)
	MCFG_CPU_VBLANK_INT("screen", irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)	/* ? */
	MCFG_CPU_PROGRAM_MAP(multchmp_sound_map)
	MCFG_CPU_IO_MAP(multchmp_sound_io_map)
	MCFG_CPU_PERIODIC_INT(nmi_line_pulse,32*60)	/* IRQ By Main CPU */

	MCFG_MACHINE_START(esd16)
	MCFG_MACHINE_RESET(esd16)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(0x140, 0x100)
	MCFG_SCREEN_VISIBLE_AREA(0, 0x140-1, 0+8, 0x100-8-1)
	MCFG_SCREEN_UPDATE_STATIC(hedpanic)

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	decospr_device::set_gfx_region(*device, 0);
	decospr_device::set_is_bootleg(*device, true);
	decospr_device::set_pri_callback(*device, hedpanic_pri_callback);
	decospr_device::set_flipallx(*device, 1);

	MCFG_GFXDECODE(esd16)
	MCFG_PALETTE_LENGTH(768)

	MCFG_VIDEO_START(esd16)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, 4000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hedpanic, esd16 )

	/* basic machine hardware */

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hedpanic_map)

	MCFG_EEPROM_93C46_ADD("eeprom")

	MCFG_PALETTE_LENGTH(0x1000/2)

	MCFG_GFXDECODE(hedpanic)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_STATIC(hedpanic)

	MCFG_DEVICE_MODIFY("spritegen")
	decospr_device::set_offsets(*device, -0x18,-0x100);
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( mchampdx, hedpanic )

	/* basic machine hardware */

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mchampdx_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tangtang, hedpanic )

	/* basic machine hardware */

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(tangtang_map)

	MCFG_GFXDECODE(tangtang)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( swatpolc, hedpanic )

	/* basic machine hardware */

	MCFG_GFXDECODE(tangtang)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hedpanio, esd16 )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hedpanic_map)

	MCFG_EEPROM_93C46_ADD("eeprom")

	MCFG_PALETTE_LENGTH(0x1000/2)

	MCFG_GFXDECODE(hedpanic)
MACHINE_CONFIG_END



/***************************************************************************


                                ROMs Loading


***************************************************************************/

/***************************************************************************

                                Multi Champ
PCB Layout
----------

ESD 11-09-98
+-----------------------------------------+
|       YM3812   6116    su10         fu27|
|VOL    YM3014   su06    M6295        fu32|
|           PAL  Z80                  fu26|
|        6116                         fu30|
|J       6116            Actel        fu28|
|A DSWA DSWB    76C256   A40MX04      fu31|
|M              76C256                fu29|
|M    cu03 76C256   6116 Actel    PAL fu33|
|A    cu02 76C256   6116 A40MX04  PAL ju07|
|      68000              6116        ju03|
|       PAL               6116 PAL    ju04|
|       PAL               6116 PAL    ju05|
|16MHz 14MHz              6116 PAL    ju06|
+-----------------------------------------+

(C) ESD 1998, 1999
PCB No. ESD 11-09-98    (Probably the manufacture date)
CPU: MC68HC000FN16 (68000, 68 pin PLCC socketed)
SND: Z80 (Z0840006PSC), U6614/U6612 (YM3014/YM3812), AD-65 (OKI M6295)
OSC: 16.000MHz, 14.000MHz
RAM: 4 x 62256, 9 x 6116
DIPS: 2 x 8 position
Dip info is in Japanese! I will scan and make it available on my site for translation.

Other Chips: 2 x Actel A40MX04-F FPGA (PLCC84)
8 PAL's (not dumped)

ROMS:

MULTCHMP.U02  \   Main Program     MX27C2000
MULTCHMP.U03  /                    MX27C2000
MULTCHMP.U06   -- Sound Program    27C010
MULTCHMP.U10   -- ADPCM Samples    27C010
MULTCHMP.U27 -\                    27C4001
MULTCHMP.U28   \                   27C4001
MULTCHMP.U29    |                  27C4001
MULTCHMP.U30    + Backgrounds      27C4001
MULTCHMP.U31    |                  27C4001
MULTCHMP.U32    |                  27C4001
MULTCHMP.U33   /                   27C4001
MULTCHMP.U34 -/                    27C4001
MULTCHMP.U35 -\                    MX27C2000
MULTCHMP.U36   \                   MX27C2000
MULTCHMP.U37    +- Sprites         MX27C2000
MULTCHMP.U38   /                   MX27C2000
MULTCHMP.U39 -/                    MX27C2000

***************************************************************************/

ROM_START( multchmp )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "esd2.cu02", 0x000000, 0x040000,  CRC(2d1b098a) SHA1(c2f3991f02c611c258219da2c61cad22c9a21f7d) )
	ROM_LOAD16_BYTE( "esd1.cu03", 0x000001, 0x040000,  CRC(10974063) SHA1(854b38b4d4cb529e9928aae4212c86a220615e04) )

	ROM_REGION( 0x24000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "esd3.su06", 0x00000, 0x0c000, CRC(7c178bd7) SHA1(8754d3c70d9b2bf369a5ce0cce4cc0696ed22750) )
	ROM_CONTINUE(          0x10000, 0x14000)

	ROM_REGION( 0x140000, "gfx1", 0 )	/* Sprites, 16x16x5 */
	ROM_LOAD( "esd14.ju03", 0x000000, 0x040000, CRC(a6122225) SHA1(cbcf2b31c4c011daba21f0ae5fd3be63c9a87c00) )
	ROM_LOAD( "esd15.ju04", 0x040000, 0x040000, CRC(88b7a97c) SHA1(0a57ec8f6a44c8e3aa3ef35499a415d6a2b7eb16) )
	ROM_LOAD( "esd16.ju05", 0x080000, 0x040000, CRC(e670a6da) SHA1(47cbe45b6d5d0ca70d0c6787d589dde5d14fdba4) )
	ROM_LOAD( "esd17.ju06", 0x0c0000, 0x040000, CRC(a69d4399) SHA1(06ae6c07cc6b7313e2e2aa3b994f7532d6994e1b) )
	ROM_LOAD( "esd13.ju07", 0x100000, 0x040000, CRC(22071594) SHA1(c79102b250780d1da8c290d065d61fbbfa193366) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* Layers, 16x16x8 */
	ROM_LOAD( "esd5.fu27",  0x000000, 0x080000, CRC(299f32c2) SHA1(274752444f6ddba16eeefc02c3e78525c079b3d8) )
	ROM_LOAD( "esd6.fu32",  0x080000, 0x080000, CRC(e2689bb2) SHA1(1da9b1f7335d5c2d1c2f8353fccf91c0109d2e9d) )
	ROM_LOAD( "esd11.fu29", 0x100000, 0x080000, CRC(9bafd8ee) SHA1(db18be05431d4b6d4207e19fa4ed8701621aaa19) )
	ROM_LOAD( "esd12.fu33", 0x180000, 0x080000, CRC(c6b86001) SHA1(11a63b56df30ab7b85ce4568d2a24e96a125735a) )
	ROM_LOAD( "esd7.fu26",  0x200000, 0x080000, CRC(a783a003) SHA1(1ff61a049485c5b599c458a8bf7f48027d14f8e0) )
	ROM_LOAD( "esd8.fu30",  0x280000, 0x080000, CRC(22861af2) SHA1(1e74e85517cb8fd5fb4bda6e9d9d54046e31f653) )
	ROM_LOAD( "esd9.fu28",  0x300000, 0x080000, CRC(6652c04a) SHA1(178e1d42847506d869ef79db2f7e10df05e9ef76) )
	ROM_LOAD( "esd10.fu31", 0x380000, 0x080000, CRC(d815974b) SHA1(3e528a5df79fa7dc0f38b0ee7f2f3a0ebc97a369) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "esd4.su10", 0x00000, 0x20000, CRC(6e741fcd) SHA1(742e0952916c00f67dd9f8d01e721a9a538d2fc4) )
ROM_END

ROM_START( multchmpk )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "multchmp.u02", 0x000000, 0x040000, CRC(7da8c0df) SHA1(763a3240554a02d8a9a0b13b6bfcd384825a6c57) )
	ROM_LOAD16_BYTE( "multchmp.u03", 0x000001, 0x040000, CRC(5dc62799) SHA1(ff7882985efc20309c3f901a622f1beffa0c47be) )

	ROM_REGION( 0x24000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "esd3.su06", 0x00000, 0x0c000, CRC(7c178bd7) SHA1(8754d3c70d9b2bf369a5ce0cce4cc0696ed22750) )
	ROM_CONTINUE(          0x10000, 0x14000)

	ROM_REGION( 0x140000, "gfx1", 0 )	/* Sprites, 16x16x5 */
	ROM_LOAD( "multchmp.u36", 0x000000, 0x040000, CRC(d8f06fa8) SHA1(f76912f93f99578529612a7f01d82ac7229a8e41) )
	ROM_LOAD( "multchmp.u37", 0x040000, 0x040000, CRC(b1ae7f08) SHA1(37dd9d4cef8b9e1d09d7b46a9794fb2b777c9a01) )
	ROM_LOAD( "multchmp.u38", 0x080000, 0x040000, CRC(88e252e8) SHA1(07d898379798c6be42b636762b0af61b9111a480) )
	ROM_LOAD( "multchmp.u39", 0x0c0000, 0x040000, CRC(51f01067) SHA1(d5ebbc7d358b63724d2f24da8b2ce4a202be37a5) )
	ROM_LOAD( "multchmp.u35", 0x100000, 0x040000, CRC(9d1590a6) SHA1(35f634dbf0df06ec62359c7bae43c7f5d14b0ab2) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* Layers, 16x16x8 */
	ROM_LOAD( "multchmp.u27", 0x000000, 0x080000, CRC(dc42704e) SHA1(58a04a47ffc6d6ae0e4d49e466b1c58b37ad741a) )
	ROM_LOAD( "multchmp.u28", 0x080000, 0x080000, CRC(449991fa) SHA1(fd93e420a04cb8bea5421aa9cbe079bd3e7d4924) )
	ROM_LOAD( "multchmp.u33", 0x100000, 0x080000, CRC(e4c0ec96) SHA1(74152108e4d05f4aff9d38919f212fcb8c87cef3) )
	ROM_LOAD( "multchmp.u34", 0x180000, 0x080000, CRC(bffaaccc) SHA1(d9ab248e2c7c639666e3717cfc5d8c8468a1bde2) )
	ROM_LOAD( "multchmp.u29", 0x200000, 0x080000, CRC(01bd1399) SHA1(b717ccffe0af92a42a0879736d34d3ad71840233) )
	ROM_LOAD( "multchmp.u30", 0x280000, 0x080000, CRC(c6b4cc18) SHA1(d9097b85584272cfe4989a40d622ef1feeee6775) )
	ROM_LOAD( "multchmp.u31", 0x300000, 0x080000, CRC(b1e4e9e3) SHA1(1a7393e9073b028b4170393b3788ad8cb86c0c78) )
	ROM_LOAD( "multchmp.u32", 0x380000, 0x080000, CRC(f05cb5b4) SHA1(1b33e60942238e39d61ae59e9317b99e83595ab1) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "esd4.su10", 0x00000, 0x20000, CRC(6e741fcd) SHA1(742e0952916c00f67dd9f8d01e721a9a538d2fc4) )
ROM_END

/*

Multi Champ Deluxe
------------------

PCB Layout
----------

ESD 08-26-1999
|-----------------------------------------|
|  3014  3812 6116   6295   ESD4.SU10   * |
|VOL      ESD3.SU06  Z80          ROM.JU01|
|             PAL                       * |
|                            6116         |
|       6116            PAL  6116 ROM.JU02|
|       6116           |-------|        * |
|J                PAL  | ESD   |        * |
|A                PAL  |CRTC99 |ESD5.JU07 |
|M     PAL             |       |        * |
|M     PAL             |-------|          |
|A    68000    ESD1.CU03                  |
|              ESD2.CU02  |-------|       |
|                         |ACTEL  | 6116  |
|   93C46                 |A40MX04| 6116  |
|              MCM6206    |       | 6116  |
|              MCM6206    |-------| 6116  |
|SW1 16MHz PAL MCM6206                    |
|SW2 14MHz PAL MCM6206  ROM.FU35 ROM.FU34 |
|-----------------------------------------|

Notes:
      68000 clock 16.000MHz
      Z80 clock 4.000MHz
      M6295 clock 1.000MHz. Sample rate 1000000/132
      YM3812 clock 4.000MHz
      HSync   - 15.625kHz
      VSync   - 60Hz
      MCM6206 - 32k x8 SRAM (SOJ28)
      6116    - 2k x8 SRAM (SOP28)
      A40MX04 - Actel A40MX04-F FPGA (PLCC84)
      CRTC99  - ESD CRTC99 Graphics Controller (QFP240)

      * : Board has positions for 6x standard 32 pin EPROMs but only position ESD5 is populated
          with an EPROM. In between the unpopulated positions are 2x smt pads. These are populated
          with 2x 16M SOP44 smt Mask ROMs.

Note: Some versions of this PCB used larger EPROMs with the data repeated:
      ESD 3 @ SU06 as a 27C040 with data repeated 2x (CRC32 0x2C0C8813)

*/


ROM_START( mchampdx )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "ver0106_esd2.cu02", 0x000000, 0x040000, CRC(ea98b3fd) SHA1(107ee8adea246141fd6fa9209541ce0a7ed1e24c) )
	ROM_LOAD16_BYTE( "ver0106_esd1.cu03", 0x000001, 0x040000, CRC(c6e4546b) SHA1(af9a8edffe94d035f92b36b1cd145c2a5ee66f48) )

	ROM_REGION( 0x44000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "esd3.su06", 0x00000, 0x0c000, CRC(1b22568c) SHA1(5458e1a798357a6785f8ea1fe9da37768cd4761d) )
	ROM_CONTINUE(          0x10000, 0x34000)

	/* this has additional copyright sprites in the flash roms for the (c)2000 message.. */
	ROM_REGION( 0x600000, "gfx1", 0 )	/* Sprites, 16x16x5 */
	ROM_LOAD( "ver0106_ju01.bin", 0x200000, 0x200000, CRC(55841d90) SHA1(52ba3ee9393dcddf28e2d20a50151bc739faaaa4) )
	ROM_LOAD( "ver0106_ju02.bin", 0x000000, 0x200000, CRC(b27a4977) SHA1(b7f94bb04d0046538b3938335e6b0cce330ad79c) )
	/* expand this to take up 0x200000 bytes too so we can decode it */
	ROM_LOAD16_BYTE( "ver0106_esd5.ju07", 0x400000, 0x040000, CRC(7a3ac887) SHA1(3c759f9bed396bbaf6bd7298a8bd2bd76df3aa6f) )
	ROM_FILL(                             0x500000, 0x100000, 0 )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* Layers, 16x16x8 */
	ROM_LOAD16_BYTE( "rom.fu35", 0x000000, 0x200000, CRC(ba46f3dc) SHA1(4ac7695bdf4237654481f7f74f8650d70a51e691) )
	ROM_LOAD16_BYTE( "rom.fu34", 0x000001, 0x200000, CRC(2895cf09) SHA1(88756fcd589af1986c3881d4080f086afc11b498) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "ver0106_esd4.su10", 0x00000, 0x40000, CRC(ac8ae009) SHA1(2c1c30cc4b3e34a5f14d7dfb6f6e18ff21f526f5) )
ROM_END

ROM_START( mchampdxa )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "esd2.cu02", 0x000000, 0x040000, CRC(4cca802c) SHA1(5e6e81febbb56b7c4630b530e546e7ab59c6c6c1) )
	ROM_LOAD16_BYTE( "esd1.cu03", 0x000001, 0x040000, CRC(0af1cd0a) SHA1(d2befcb596d83d523317d17b4c1c71f99de0d33e) )

	ROM_REGION( 0x44000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "esd3.su06", 0x00000, 0x0c000, CRC(1b22568c) SHA1(5458e1a798357a6785f8ea1fe9da37768cd4761d) )
	ROM_CONTINUE(          0x10000, 0x34000)

	ROM_REGION( 0x600000, "gfx1", 0 )	/* Sprites, 16x16x5 */
	ROM_LOAD( "rom.ju01", 0x200000, 0x200000, CRC(1a749fc2) SHA1(feff4b26ee28244b4d092798a176e33e09d5df2c) )
	ROM_LOAD( "rom.ju02", 0x000000, 0x200000, CRC(7e87e332) SHA1(f90aa00a64a940846d99053c7aa023e3fd5d070b) )
	/* expand this to take up 0x200000 bytes too so we can decode it */
	ROM_LOAD16_BYTE( "esd5.ju07", 0x400000, 0x080000, CRC(6cc871cc) SHA1(710b9695c864e4234686993b88d24590d60e1cb9) )
	ROM_FILL(                     0x500000, 0x100000, 0 )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* Layers, 16x16x8 */
	ROM_LOAD16_BYTE( "rom.fu35", 0x000000, 0x200000, CRC(ba46f3dc) SHA1(4ac7695bdf4237654481f7f74f8650d70a51e691) )
	ROM_LOAD16_BYTE( "rom.fu34", 0x000001, 0x200000, CRC(2895cf09) SHA1(88756fcd589af1986c3881d4080f086afc11b498) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "esd4.su10", 0x00000, 0x40000, CRC(2fbe94ab) SHA1(1bc4a33ec93a80fb598722d2b50bdf3ccaaa984a) )
ROM_END

ROM_START( mchampdxb )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "ver1114_esd2.cu02", 0x000000, 0x040000, CRC(d17b2616) SHA1(2c50c2bf928036678b92b8862d191552e46d9faa) )
	ROM_LOAD16_BYTE( "ver1114_esd1.cu03", 0x000001, 0x040000, CRC(11ff2e94) SHA1(30044bedfff514ae0a855cffa756e5c315fe2124) )

	ROM_REGION( 0x44000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "ver1114_esd3.su06", 0x00000, 0x0c000, CRC(b87a1e85) SHA1(2fcdd7e8b301e3d20e6500a03dc293403b23b471) )
	ROM_CONTINUE(                  0x10000, 0x34000)

	ROM_REGION( 0x600000, "gfx1", 0 )	/* Sprites, 16x16x5 */
	ROM_LOAD( "ver1114_ju01", 0x200000, 0x200000, CRC(0048e687) SHA1(5cc0a35b5f5f8d69b2dc3728ad6d0d505d9e16c5) )  // SMT Flash MX chips
	ROM_LOAD( "ver1114_ju02", 0x000000, 0x200000, CRC(2f9ccff8) SHA1(176240cd247cc5d3efd58fe0630726a8633be2a4) )
	/* expand this to take up 0x200000 bytes too so we can decode it */
	ROM_LOAD16_BYTE( "ver1114_esd5.ju07", 0x400000, 0x040000,  CRC(8175939f) SHA1(cd0132ae0d2e35dc656434989b1f0f255ad562ab) )
	ROM_FILL(                             0x500000, 0x100000, 0 )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* Layers, 16x16x8 */
	ROM_LOAD16_BYTE( "ver1114_fu35", 0x000000, 0x200000, CRC(c515c704) SHA1(c1657534314e66a25c38f70a12f14d2225ab89cc) ) // SMT Flash MX chips
	ROM_LOAD16_BYTE( "ver1114_fu34", 0x000001, 0x200000, CRC(39d448bb) SHA1(07cd6e30a25d1c0caeef0f95f23df0ca6a2c7a26) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "esd4.su10", 0x00000, 0x40000, CRC(2fbe94ab) SHA1(1bc4a33ec93a80fb598722d2b50bdf3ccaaa984a) )
ROM_END

/***************************************************************************

PCB Layout (Head Panic)
----------

ESD 05-28-99
+-----------------------------------------+
|            3812 3014 W24165 6295 ESD4   |
|VOL                 ESD3                 |
|                   PAL Z80               |
|                                         |
|                       +-------+    SM4  |
|      W24165    62256  | ESD   |         |
|J     W24165    62256  |CRTC99 |    SM3  |
|A               62256  |       |         |
|M               62256  +-------+    PAL  |
|M   68000 ESD1                      PAL  |
|A     PAL ESD2 W24165  +-------+   ESD5  |
|      PAL      W24165  |ACTEL  |         |
|                       |A40MX04|         |
|               W24165  |       |   SM2   |
|               W24165  +-------+         |
|S1 93C46       W24165   PAL        SM1   |
|S2 16MHz 14MHZ W24165   PAL PAL          |
+-----------------------------------------+

Notes:
      68000 (MC68HC000FN16)
      Z80 (Z84C00006FEC)
      YM3014/YM3812 & OKI M6295 (rebaged as U6614/U6612 & AD-65)
      A40MX04 - Actel A40MX04-F FPGA (PLCC84)
      CRTC99  - ESD CRTC99 Graphics Controller (QFP240)

      ESD1, ESD2 are 27C2001
      ESD3 is a 27C2000
      ESD4 is a 27C010 Mask ROM
      ESD5 is a 27C040
      SM1, SM2, SM3 & SM4 are MX29F1610MC 16M SOP44 smt flash ROM

1x connector JAMMA
1x trimmer (volume)
2x pushbutton


ESD 08-26-1999
|-----------------------------------------|
|  3014  3812 6116   6295   ESD4          |
|VOL        ESD3    Z80                 * |
|             PAL                   ESD6  |
|                            6116       * |
|       6116            PAL  6116   ESD7  |
|       6116           |-------|        * |
|J                PAL  | ESD   |        * |
|A                PAL  |CRTC99 |    ESD5  |
|M     PAL             |       |        * |
|M     PAL             |-------|          |
|A            ESD1                        |
|      68000  ESD2        |-------|       |
|                         |ACTEL  | 6116  |
|                         |A40MX04| 6116  |
|   93C46      MCM6206    |       | 6116  |
|              MCM6206    |-------| 6116  |
|SW1 16MHz PAL MCM6206                    |
|SW2 14MHz PAL MCM6206    ESD8 %  ESD9    |
|-----------------------------------------|

Notes:
      HSync: 15.625kHz
      VSync: 60Hz
      MCM6206 is 32kx8 SRAM
      6116 is 8kx8 SRAM
      * : Board has positions for 6x standard 32 pin EPROMs but only position ESD5 is populated
          with an EPROM. In between the unpopulated positions are 2x smt pads. These are populated
          with 2x 16M SOP44 smt Mask ROMs.
      % : ROMs ESD8 and ESD9 are also 16M SOP44 smt Mask ROMs, though these are dedicated smt
          locations (i.e. no option for EPROMs at this location)

Note: Some versions of this PCB used larger EPROMs with the data repeated:
      ESD 3 @ SU06 as a 27C040 with data repeated 2x (CRC32 0xC668D443)
      ESD 4 @ SU10 as a 27C040 with data repeated 4x (CRC32 0x5692FE92)

***************************************************************************/


ROM_START( hedpanic ) /* Story line & game instructions in English */
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "esd2.cu03", 0x000000, 0x040000, CRC(7c7be3bb) SHA1(d43ad7a967e1ef79ee0cf50d3842cc9174fbef3a) )
	ROM_LOAD16_BYTE( "esd1.cu02", 0x000001, 0x040000, CRC(42405e9d) SHA1(0fa088b8bd921e42cedcc4083dfe41bc9888dfd1) )

	ROM_REGION( 0x44000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "esd3.su06", 0x00000, 0x0c000, CRC(a88d4424) SHA1(eefb5ac79632931a36f360713c482cd079891f91) ) /* AT27C020 mask rom */
	ROM_CONTINUE(          0x10000, 0x34000)

	ROM_REGION( 0x600000, "gfx1", 0 )	/* Sprites, 16x16x5 */
	ROM_LOAD( "esd6.ju01", 0x200000, 0x200000, CRC(5858372c) SHA1(dc96112587df681d53cf7449bd39477919978325) )
	ROM_LOAD( "esd7.ju02", 0x000000, 0x200000, CRC(055d525f) SHA1(85ad474691f96e47311a1904015d1c92d3b2d607) )
	/* expand this to take up 0x200000 bytes too so we can decode it */
	ROM_LOAD16_BYTE( "esd5.ju07", 0x400000, 0x080000, CRC(bd785921) SHA1(c8bcb38d5aa6f5a27f0dedf7efd1d6737d59b4ca) )
	ROM_FILL(                     0x500000, 0x100000, 0 )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* Layers, 16x16x8 */
	ROM_LOAD16_BYTE( "esd8.fu35", 0x000000, 0x200000, CRC(23aceb4f) SHA1(35d9ebc33b9e1515e47750cfcdfc0bf8bf44b71d) )
	ROM_LOAD16_BYTE( "esd9.fu34", 0x000001, 0x200000, CRC(76b46cd2) SHA1(679cbf50ae5935e8848868081ecef4ec66424f6c) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "esd4.su10", 0x000000, 0x020000, CRC(3c11c590) SHA1(cb33845c3dc0501fff8055c2d66f412881089df1) ) /* AT27010 mask rom */

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "hedpanic.nv", 0x0000, 0x0080, CRC(e91f4038) SHA1(f492de71170900f87912a272ab4f4a3a37ba31fe) )
ROM_END


ROM_START( hedpanicf ) /* Story line in Japanese, game instructions in English */
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "esd2", 0x000000, 0x040000, CRC(8cccc691) SHA1(d6a5dd6c21a67638b9023182f77780282b9b04e5) ) /* CU03 */
	ROM_LOAD16_BYTE( "esd1", 0x000001, 0x040000, CRC(d8574925) SHA1(bd4990778b90a49aa6b10f8cf6709ce2424f546a) ) /* CU02 */

	ROM_REGION( 0x44000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "esd3.su06", 0x00000, 0x0c000, CRC(a88d4424) SHA1(eefb5ac79632931a36f360713c482cd079891f91) ) /* AT27C020 mask rom */
	ROM_CONTINUE(          0x10000, 0x34000)

	ROM_REGION( 0x600000, "gfx1", 0 )	/* Sprites, 16x16x5 */
	ROM_LOAD( "esd6.ju01", 0x200000, 0x200000, CRC(5858372c) SHA1(dc96112587df681d53cf7449bd39477919978325) )
	ROM_LOAD( "esd7.ju02", 0x000000, 0x200000, CRC(055d525f) SHA1(85ad474691f96e47311a1904015d1c92d3b2d607) )
	/* expand this to take up 0x200000 bytes too so we can decode it */
	ROM_LOAD16_BYTE( "esd5.ju07", 0x400000, 0x080000, CRC(bd785921) SHA1(c8bcb38d5aa6f5a27f0dedf7efd1d6737d59b4ca) )
	ROM_FILL(                     0x500000, 0x100000, 0 )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* Layers, 16x16x8 */
	ROM_LOAD16_BYTE( "esd8.fu35", 0x000000, 0x200000, CRC(23aceb4f) SHA1(35d9ebc33b9e1515e47750cfcdfc0bf8bf44b71d) )
	ROM_LOAD16_BYTE( "esd9.fu34", 0x000001, 0x200000, CRC(76b46cd2) SHA1(679cbf50ae5935e8848868081ecef4ec66424f6c) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "esd4.su10", 0x000000, 0x020000, CRC(3c11c590) SHA1(cb33845c3dc0501fff8055c2d66f412881089df1) ) /* AT27010 mask rom */

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "hedpanic.nv", 0x0000, 0x0080, CRC(e91f4038) SHA1(f492de71170900f87912a272ab4f4a3a37ba31fe) )
ROM_END


ROM_START( hedpanico ) /* Story line & game instructions in English, copyright year is 1999 - ESD 05-28-99 PCB which uses older style sprites */
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "esd2.rom", 0x000000, 0x040000, CRC(70b08424) SHA1(2ba4fb3b749e31db4239a9173b8509366400152f) ) /* CU03 */
	ROM_LOAD16_BYTE( "esd1.rom", 0x000001, 0x040000, CRC(4e0682c5) SHA1(f4117f31b6426d7bf126a6c62c489b9347885b42) ) /* CU02 */

	ROM_REGION( 0x44000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "esd3.su06", 0x00000, 0x0c000, CRC(a88d4424) SHA1(eefb5ac79632931a36f360713c482cd079891f91) ) /* AT27C020 mask rom */
	ROM_CONTINUE(          0x10000, 0x34000)

	ROM_REGION( 0x600000, "gfx1", 0 )	/* Sprites, 16x16x5 */
	ROM_LOAD( "sm1.ju01", 0x000000, 0x200000, CRC(8083813f) SHA1(9492e7e844e45d59f0506f69d40c338b27bd3ce3) )
	ROM_LOAD( "sm2.ju02", 0x200000, 0x200000, CRC(7a9610e4) SHA1(21ae3ec3fbddfc66416c109b091bd885d5ba0558) )
	/* expand this to take up 0x200000 bytes too so we can decode it */
	ROM_LOAD16_BYTE( "esd5.rom", 0x400000, 0x080000, CRC(82c5727f) SHA1(017f1d0c94475c51d17f12e24895f47a273a2dbb) ) /* JU07 */
	ROM_FILL(                    0x500000, 0x100000, 0 )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* Layers, 16x16x8 */
	ROM_LOAD16_BYTE( "sm3.fu35", 0x000000, 0x200000, CRC(94dd4cfc) SHA1(a3f9c49611f0bc9d26166dafb44e2c5ebbb31127) )
	ROM_LOAD16_BYTE( "sm4.fu34", 0x000001, 0x200000, CRC(6da0fb9e) SHA1(c4e7487953f45c5f6ce2ebe558b4c325f6ec54eb) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "esd4.rom", 0x000000, 0x020000, CRC(d7ca6806) SHA1(8ad668bfb5b7561cc0f3e36dfc3c936b136a4274) ) /* SU10 */

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "hedpanic.nv", 0x0000, 0x0080, CRC(e91f4038) SHA1(f492de71170900f87912a272ab4f4a3a37ba31fe) )
ROM_END



/*

Deluxe 5 (c) 2000 ESD
PCB Layout
----------

ESD made in Korea
|-----------------------------------------|
|     3014 3812 6116  6295   ESD4.SU10    |
|VOL      ESD3.SU06  Z80              JU03|
|             PAL                         |
|                           6116      JU04|
|       6116           PAL  6116          |
|       6116     PAL  |-------|       JU05|
|J               PAL  | ESD   |           |
|A                    |CRTC99 |       JU06|
|M     PAL            | 0016  |           |
|M     PAL            |-------|       JU07|
|A    68000   ESD1.CU03                   |
|             ESD2.CU02   |-------|       |
|                         |ACTEL  |       |
|                         |A40MX04|   6116|
|              MCM6206    |  0008 |   6116|
|              MCM6206    |-------|       |
|SW1 16MHz PAL MCM6206                6116|
|SW2 14MHz PAL MCM6206   FU35   FU34  6116|
|-----------------------------------------|

Notes:
      68000 (MC68HC000FN16)
      Z80 (Z84C00006FEC-Z80CPU)
      OKI6295 label AD65 (sound)
      YM3014 label U6614 (sound)
      YM3812 label U6612 (sound)
      MCM6206 - 32k x8 SRAM (SOJ28)
      6116    - 2k x8 SRAM (SOP28)
      A40MX04 - Actel A40MX04-F FPGA (PLCC84)
      CRTC99  - ESD CRTC99 Graphics Controller (QFP240)
      ESD1-2  - M27C2001
      JU03-8  - AM27C020
      FU34,FU35  -  MX29F1610MC

1x connector JAMMA
1x trimmer (volume)
2x pushbutton

      * : Board has positions for 6x standard 32 pin EPROMs but only 5 positions are populated with an EPROM.

*/

ROM_START( deluxe5 ) /* Deluxe 5 */
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "esd2.cu02", 0x000000, 0x040000,  CRC(d077dc13) SHA1(d83feadb29674d56a5f019641f402798c7ba8d61) ) /* M27C2001 EPROM */
	ROM_LOAD16_BYTE( "esd1.cu03", 0x000001, 0x040000,  CRC(15d6644f) SHA1(cfb8168167389855f906658511d1dc7460e13100) ) /* M27C2001 EPROM */

	ROM_REGION( 0x44000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "esd3.su06", 0x00000, 0x0c000, CRC(31de379a) SHA1(a0c9a9cec7207cc4ba33abb68bef62d7eb8e75e9) ) /* AM27C020 mask rom */
	ROM_CONTINUE(          0x10000, 0x34000)

	ROM_REGION( 0x280000, "gfx1", 0 )	/* Sprites, 16x16x5 */
	ROM_LOAD( "am27c020.ju03", 0x000000, 0x040000, CRC(aa130fd3) SHA1(46a55d8ca59a52e610600fdba76d9729528d2871) ) /* AM27C020 mask roms with no label */
	ROM_LOAD( "am27c020.ju04", 0x080000, 0x040000, CRC(40fa2c2f) SHA1(b9d9bfdc9343f00bad9749c76472f064c509cfce) )
	ROM_LOAD( "am27c020.ju05", 0x100000, 0x040000, CRC(bbe81779) SHA1(750387fb4aaa04b7f4f1d3985896f5e11219e3ea) )
	ROM_LOAD( "am27c020.ju06", 0x180000, 0x040000, CRC(8b853bce) SHA1(fa6e654fc965d88bb426b76cdce3417f357b25f3) )
	ROM_LOAD( "am27c020.ju07", 0x200000, 0x040000, CRC(d414c3af) SHA1(9299b07a8c7a3e30a1bb6028204a049a7cb510f7) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* Layers, 16x16x8 */
	ROM_LOAD16_BYTE( "fu35", 0x000000, 0x200000, CRC(ae10242a) SHA1(f3d18c0cb7951b5f7ee47aa2856b7554088328ed) ) /* No labels on the flash roms */
	ROM_LOAD16_BYTE( "fu34", 0x000001, 0x200000, CRC(248b8c05) SHA1(fe7bcc05ae0dd0a27c6ba4beb4ac155a8f3d7f7e) ) /* No labels on the flash roms */

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "esd4.su10", 0x00000, 0x20000, CRC(23f2b7d9) SHA1(328c951d14674760df68486841c933bad0d59fe3) ) /* AT27C010 mask rom */
ROM_END

/* Tang Tang

Tang Tang (ESD)
------------------------

PCB Layout
----------

ESD made in Korea
|-----------------------------------------|
| U6614 U6612 6116   AD65   ESD4.SU10     |
|VOL      ESD3.SU06  Z80          ROM.JU04|
|             PAL                         |
|                            6116 ROM.JU05|
|       6116            PAL  6116         |
|       6116           |-------|  ROM.JU06|
|J                PAL  | ESD   |          |
|A                PAL  |CRTC99 |  ROM.JU07|
|M     PAL             | 0016  |          |
|M     PAL             |-------|  ROM.JU08|
|A    68000    ESD1.CU03                  |
|              ESD2.CU02  |-------|       |
|                         |ACTEL  | 6116  |
|                         |A40MX04| 6116  |
|              MCM6206    |  0008 | 6116  |
|              MCM6206    |-------| 6116  |
|SW1 16MHz PAL MCM6206                6116|
|SW2 14MHz PAL MCM6206   FU35   FU34  6116|
|-----------------------------------------|

Notes:
      68000 (MC68HC000FN16-2E60R-QQJU9508)
      Z80 (Z84C00006FEC-Z80CPU-9618Z3)
      OKI6295 label AD65 (sound)
      YM3014 label U6614 (sound)
      YM3812 label U6612 (sound)
      MCM6206 - 32k x8 SRAM (SOJ28)
      6116    - 2k x8 SRAM (SOP28)
      A40MX04 - Actel A40MX04-F FPGA (PLCC84)
      CRTC99  - ESD CRTC99 Graphics Controller (QFP240)
      ESD1-2  - 27C2001
      ESD3-4  - 27C2000
      JU04-8  - MX27C2000PC
      FU34,FU35  -  MX29F1610MC
1x connector JAMMA
1x trimmer (volume)
2x pushbutton

      * : Board has positions for 6x standard 32 pin EPROMs but only 5 positions are populated with an EPROM.
      * : Tang Tang also known to be found on a ESD 05-28-99 PCB (see Head Panic above for PCB layout)

*/

ROM_START( tangtang )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "esd2.cu02", 0x000000, 0x040000,  CRC(b6dd6e3d) SHA1(44d2663827c45267eb154c873f3bd2e9e2bf3d3f) )
	ROM_LOAD16_BYTE( "esd1.cu03", 0x000001, 0x040000,  CRC(b6c0f2f4) SHA1(68ad76e7e380c728dda200a852729e034d9c9f4c) )

	ROM_REGION( 0x44000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "esd3.su06", 0x00000, 0x0c000, CRC(d48ecc5c) SHA1(5015dd775980542eb29a08bffe1a09ea87d56272) )
	ROM_CONTINUE(          0x10000, 0x34000)

	ROM_REGION( 0x140000, "gfx1", 0 )	/* Sprites, 16x16x5 */
	ROM_LOAD( "xju04.bin", 0x000000, 0x040000, CRC(f999b9d7) SHA1(9e4d0e68cdc429c7563b8ad51c072d68ffed09dc) )
	ROM_LOAD( "xju05.bin", 0x040000, 0x040000, CRC(679302cf) SHA1(911c2f7e0e809ee28e4f2364788fd51d2bcef24e) )
	ROM_LOAD( "xju06.bin", 0x080000, 0x040000, CRC(01f59ff7) SHA1(a62a2d5c2d107f67fecfc08fdb5d801ee39c3875) )
	ROM_LOAD( "xju07.bin", 0x0c0000, 0x040000, CRC(556acac3) SHA1(10e919e63b434da80fb261db1d8967cb11e95e00) )
	ROM_LOAD( "xju08.bin", 0x100000, 0x040000, CRC(ecc2d8c7) SHA1(1aabdf7204fcdff8d46cb50de8b097e3775dddf3) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* Layers, 16x16x8 */
	ROM_LOAD16_BYTE( "fu35.bin", 0x000000, 0x200000, CRC(84f3f833) SHA1(f84e41d93dc47a58ada800b921a7e5902b7631cd) )
	ROM_LOAD16_BYTE( "fu34.bin", 0x000001, 0x200000, CRC(bf91f543) SHA1(7c149fed8b8044850cd6b798622a91c45336cd47) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "esd4.su10", 0x00000, 0x20000, CRC(f2dfb02d) SHA1(04001488697aad3e5b2d15c9f5a81dc2b7d0952c) )
ROM_END

/*

SWAT Police (c) 2001 ESD
PCB Layout
----------

ESD made in Korea
|-----------------------------------------|
|     3014 3812 6116  6295   AT27C020     |
|VOL      ESD3.SU06  Z80         ESD1.JU03|
|             PAL                         |
|                           6116 ESD2.JU04|
|       6116           PAL  6116          |
|       6116     PAL  |-------|  ESD3.JU05|
|J               PAL  | ESD   |           |
|A                    |CRTC99 |  ESD4.JU06|
|M     PAL            | 0016  |           |
|M     PAL            |-------|  ESD5.JU07|
|A    68000   ESD.CU03                    |
|             ESD.CU02    |-------|       |
|                         |ACTEL  |       |
|                         |A40MX04|   6116|
|              MCM6206    |  0008 |   6116|
|              MCM6206    |-------|       |
|SW1 16MHz PAL MCM6206                6116|
|SW2 14MHz PAL MCM6206   FU35   FU34  6116|
|-----------------------------------------|

Notes:
      68000 (MC68HC000FN16)
      Z80 (Z84C00006FEC-Z80CPU)
      OKI6295 label AD65 (sound)
      YM3014 label U6614 (sound)
      YM3812 label U6612 (sound)
      MCM6206 - 32k x8 SRAM (SOJ28)
      6116    - 2k x8 SRAM (SOP28)
      A40MX04 - Actel A40MX04-F FPGA (PLCC84)
      CRTC99  - ESD CRTC99 Graphics Controller (QFP240)
      CU02-3  - 27C2001
      JU03-8  - 27C040
      FU34,FU35  -  MX29F1610MC

1x connector JAMMA
1x trimmer (volume)
2x pushbutton

      * : Board has positions for 6x standard 32 pin EPROMs but only 5 positions are populated with an EPROM.

*/

ROM_START( swatpolc ) /* SWAT Police */
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "esd.cu02", 0x000000, 0x040000,  CRC(29e0c126) SHA1(7c0356eed4ffdc056b7ec5c1ac07f1c9cc6aeffa) ) /* ESD labels but not numbered */
	ROM_LOAD16_BYTE( "esd.cu03", 0x000001, 0x040000,  CRC(1070208b) SHA1(1e058774c5aee1de15ffcd26d530b23592286db1) ) /* ESD labels but not numbered */

	ROM_REGION( 0x44000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "esd3.su06", 0x00000, 0x0c000, CRC(80e97dbe) SHA1(d6fae689cd3737777f36c980b9a7d9e42b06a467) ) /* 2 roms on PCB with an ESD3 label */
	ROM_CONTINUE(          0x10000, 0x34000)

	ROM_REGION( 0x280000, "gfx1", 0 )	/* Sprites, 16x16x5 */
	ROM_LOAD( "esd1.ju03", 0x000000, 0x080000, CRC(17fcc5e7) SHA1(ad57d2b0c0062f6f8c7732df57e4d12ca47c1bb8) )
	ROM_LOAD( "esd2.ju04", 0x080000, 0x080000, CRC(9c1752f2) SHA1(2e8c377137258498564749413b49e156180e806a) )
	ROM_LOAD( "esd3.ju05", 0x100000, 0x080000, CRC(e8d9c092) SHA1(80e1f1d4dad48c7be3d4b72c4a82d5388fd493c7) )
	ROM_LOAD( "esd4.ju06", 0x180000, 0x080000, CRC(bde1b130) SHA1(e45a2257f8c4d107dfb7401b5ae1b79951052bc6) )
	ROM_LOAD( "esd5.ju07", 0x200000, 0x080000, CRC(d2c27f03) SHA1(7cbdf7f7ff17df16ca81823f69e82ae1cf96b714) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* Layers, 16x16x8 */
	ROM_LOAD16_BYTE( "fu35", 0x000000, 0x200000, CRC(c55897c5) SHA1(f6e0ef1c2fcfe6a511fe787a3abeff4da16d1b54) ) /* No labels on the flash roms */
	ROM_LOAD16_BYTE( "fu34", 0x000001, 0x200000, CRC(7117a6a2) SHA1(17c0ab02698cffa0582ed2d2b7dbb7fed8cd9393) ) /* No labels on the flash roms */

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "at27c020.su10", 0x00000, 0x40000, CRC(c43efec2) SHA1(4ef328d8703b81328de09ecc4328763aba06e883) ) /* AT27C020 mask rom with no label */
ROM_END

/***************************************************************************


                                Game Drivers


***************************************************************************/

/* ESD 11-09-98 */
GAME( 1999, multchmp, 0,        esd16,    multchmp, 0, ROT0, "ESD",         "Multi Champ (World, ver. 2.5)", GAME_SUPPORTS_SAVE )
GAME( 1998, multchmpk,multchmp, esd16,    multchmp, 0, ROT0, "ESD",         "Multi Champ (Korea)", GAME_SUPPORTS_SAVE )

/* ESD 05-28-99 */
GAME( 1999, hedpanico,hedpanic, hedpanio, hedpanic, 0, ROT0, "ESD",         "Head Panic (ver. 0615, 15/06/1999)", GAME_SUPPORTS_SAVE )

/* ESD 08-26-1999 */
GAME( 2000, mchampdx, 0,        mchampdx, hedpanic, 0, ROT0, "ESD",         "Multi Champ Deluxe (ver. 0106, 06/01/2000)", GAME_SUPPORTS_SAVE )
GAME( 1999, mchampdxa,mchampdx, mchampdx, hedpanic, 0, ROT0, "ESD",         "Multi Champ Deluxe (ver. 1126, 26/11/1999)", GAME_SUPPORTS_SAVE )
GAME( 1999, mchampdxb,mchampdx, mchampdx, hedpanic, 0, ROT0, "ESD",         "Multi Champ Deluxe (ver. 1114, 14/11/1999)", GAME_SUPPORTS_SAVE )
GAME( 2000, hedpanic, 0,        hedpanic, hedpanic, 0, ROT0, "ESD",         "Head Panic (ver. 0117, 17/01/2000)", GAME_SUPPORTS_SAVE )
GAME( 2000, hedpanicf,hedpanic, hedpanic, hedpanic, 0, ROT0, "ESD / Fuuki", "Head Panic (ver. 0315, 15/03/2000)", GAME_SUPPORTS_SAVE )

/* ESD - This PCB looks identical to the ESD 08-26-1999 PCB */
GAME( 2000, deluxe5,  0,        tangtang, hedpanic, 0, ROT0, "ESD",         "Deluxe 5 (ver. 0107, 07/01/2000)", GAME_SUPPORTS_SAVE )
GAME( 2000, tangtang, 0,        tangtang, hedpanic, 0, ROT0, "ESD",         "Tang Tang (ver. 0526, 26/05/2000)", GAME_SUPPORTS_SAVE )
GAME( 2001, swatpolc, 0,        swatpolc, swatpolc, 0, ROT0, "ESD",         "SWAT Police", GAME_SUPPORTS_SAVE )

