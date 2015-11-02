// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Jarek Parchanski, Nicola Salmoria
/***************************************************************************

Talbot                     - (c) 1982 Alpha Denshi Co.
Champion Base Ball         - (c) 1983 Alpha Denshi Co.
Champion Base Ball Part-2  - (c) 1983 Alpha Denshi Co.
Exciting Soccer            - (c) 1983 Alpha Denshi Co.
Exciting Soccer II         - (c) 1984 Alpha Denshi Co.

driver by Ernesto Corvi, Jarek Parchanski, Nicola Salmoria

Note: the Champion Baseball II unofficial schematics show a 8302 instead of
the 8201, however the MCU is used like a plain 8201, 830x extra instructions
are not used.

champbbj and champbb2 has Alpha8201 mcu for protection.
champbja is a patched version of champbbj with different protection.
exctsccr has Alpha8302 MCU for protection.

main CPU

0000-5fff ROM
6000-63ff MCU shared RAM
7800-7fff ROM (Champion Baseball 2 only)
8000-83ff Video RAM
8400-87ff Color RAM
8800-8fff RAM

read:
a000      IN0
a040      IN1
a080      DSW
a0a0      ?(same as DSW)
a0c0      COIN

write:
7000      8910 write
7001      8910 control
8ff0-8fff sprites
a000      ?
a006      MCU HALT control
a007      NOP (MCU shared RAM switch)
a060-a06f sprites
a080      command for the sound CPU
a0c0      watchdog reset (watchdog time = 16xvblank)

sub CPU (speech DAC)

read:
0000-5fff   ROM
6000(-7fff) sound latch
e000-e3ff   RAM

write:

8000(-9fff) 4bit status for main CPU
a000(-bfff) clear sound latch
c000(-dfff) DAC
e000-e3ff   RAM


Notes:
------
- Bit 2 of the watchdog counter can be read through an input port. The games check
  it on boot and hang if it is not 0. Also, the Talbot MCU does a security check
  and crashes if the bit doesn't match bit 2 of RAM location 0x8c00.

- The Exciting Soccer bootleg runs on a modified Champion Baseball board. The
  original board has vastly improved sound hardware which is therefore missing
  from the bootleg.

TODO:
-----
- champbb2, sometime mcu err and ACCESS VIOLATION trap.

- Exciting Soccer: interrupt source for sound CPU is unknown.

- Exciting Soccer: sound CPU writes to unknown ports on startup. Timer configure?

- Exciting Soccer: Unknown writes to 8910 I/O ports (filters?)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/alph8201/alph8201.h"
//#include "cpu/hmcs40/hmcs40.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "includes/champbas.h"



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

CUSTOM_INPUT_MEMBER(champbas_state::champbas_watchdog_bit2)
{
	return (0x10 - machine().get_vblank_watchdog_counter()) >> 2 & 1;
}

WRITE8_MEMBER(champbas_state::irq_enable_w)
{
	m_irq_mask = data & 1;

	if (!m_irq_mask)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(champbas_state::exctsccr_sound_irq)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
}

WRITE8_MEMBER(champbas_state::dac1_w)
{
	m_dac1->write_signed8(data << 2);
}

WRITE8_MEMBER(champbas_state::dac2_w)
{
	m_dac2->write_signed8(data << 2);
}


/*************************************
 *
 *  Protection handling
 *
 *************************************/

WRITE8_MEMBER(champbas_state::champbas_mcu_switch_w)
{
	// switch shared RAM between CPU and MCU bus
	// FIXME not implemented
}

WRITE8_MEMBER(champbas_state::champbas_mcu_halt_w)
{
	// MCU not present/not used in champbas
	if (m_mcu == NULL)
		return;

	data &= 1;
	m_mcu->set_input_line(INPUT_LINE_HALT, data ? ASSERT_LINE : CLEAR_LINE);
}


/* champbja another protection */
READ8_MEMBER(champbas_state::champbja_protection_r)
{
	UINT8 data = 0;
	/*
	(68BA) & 0x99 == 0x00
	(6867) & 0x99 == 0x99
	(68AB) & 0x80 == 0x80
	(6854) & 0x99 == 0x19

	BA 1011_1010
	00 0--0_0--0

	54 0101_0100
	19 0--1_1--1

	67 0110_0111
	99 1--1_1--1

	AB 1010_1011
	80 1--0_0--0
	*/

	/* bit7 =  bit0 */
	if ((offset & 0x01))
		data |= 0x80;

	/* bit4,3,0 =  bit6 */
	if ((offset & 0x40))
		data |= 0x19;

	return data;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( talbot_map, AS_PROGRAM, 8, champbas_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x63ff) AM_RAM AM_SHARE("share1") /* MCU shared RAM */
	AM_RANGE(0x7000, 0x7001) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE(0x8000, 0x87ff) AM_RAM_WRITE(tilemap_w) AM_SHARE("vram")
	AM_RANGE(0x8800, 0x8fff) AM_RAM AM_SHARE("mainram")

	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P1")
	AM_RANGE(0xa040, 0xa040) AM_READ_PORT("P2")
	AM_RANGE(0xa080, 0xa080) AM_READ_PORT("DSW")
	AM_RANGE(0xa0c0, 0xa0c0) AM_READ_PORT("SYSTEM")

	AM_RANGE(0xa000, 0xa000) AM_WRITE(irq_enable_w)
	AM_RANGE(0xa001, 0xa001) AM_WRITENOP    // !WORK board output (no use?)
	AM_RANGE(0xa002, 0xa002) AM_WRITENOP
	AM_RANGE(0xa003, 0xa003) AM_WRITE(flipscreen_w)
	AM_RANGE(0xa004, 0xa004) AM_WRITENOP
	AM_RANGE(0xa005, 0xa005) AM_WRITENOP
	AM_RANGE(0xa006, 0xa006) AM_WRITE(champbas_mcu_halt_w)
	AM_RANGE(0xa007, 0xa007) AM_WRITE(champbas_mcu_switch_w)

	AM_RANGE(0xa060, 0xa06f) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xa0c0, 0xa0c0) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( champbas_main_map, AS_PROGRAM, 8, champbas_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x63ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x7000, 0x7001) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE(0x7800, 0x7fff) AM_ROM // champbb2 only
	AM_RANGE(0x8000, 0x87ff) AM_RAM_WRITE(tilemap_w) AM_SHARE("vram")
	AM_RANGE(0x8800, 0x8fff) AM_RAM AM_SHARE("mainram")

	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P1")
	AM_RANGE(0xa040, 0xa040) AM_READ_PORT("P2")
	AM_RANGE(0xa080, 0xa080) AM_READ_PORT("DSW")
	AM_RANGE(0xa0c0, 0xa0c0) AM_READ_PORT("SYSTEM")

	AM_RANGE(0xa000, 0xa000) AM_WRITE(irq_enable_w)
	AM_RANGE(0xa001, 0xa001) AM_WRITENOP    // !WORK board output (no use?)
	AM_RANGE(0xa002, 0xa002) AM_WRITE(gfxbank_w)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(flipscreen_w)
	AM_RANGE(0xa004, 0xa004) AM_WRITE(palette_bank_w)
	AM_RANGE(0xa005, 0xa005) AM_WRITENOP    // n.c.
	AM_RANGE(0xa006, 0xa006) AM_WRITE(champbas_mcu_halt_w)  // MCU not present/not used in champbas
	AM_RANGE(0xa007, 0xa007) AM_WRITE(champbas_mcu_switch_w)    // MCU not present/not used in champbas

	AM_RANGE(0xa060, 0xa06f) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xa080, 0xa080) AM_WRITE(soundlatch_byte_w)
/*  AM_RANGE(0xa0a0, 0xa0a0)    ???? */
	AM_RANGE(0xa0c0, 0xa0c0) AM_WRITE(watchdog_reset_w)

	/* champbja only */
	AM_RANGE(0x6800, 0x68ff) AM_READ(champbja_protection_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( exctsccrb_main_map, AS_PROGRAM, 8, champbas_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
//  AM_RANGE(0x6000, 0x63ff) AM_RAM AM_SHARE("share1") // MCU not used (though it's present on the board)
	AM_RANGE(0x7000, 0x7001) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
//  AM_RANGE(0x7800, 0x7fff) AM_ROM // champbb2 only
	AM_RANGE(0x8000, 0x87ff) AM_RAM_WRITE(tilemap_w) AM_SHARE("vram")
	AM_RANGE(0x8800, 0x8fff) AM_RAM AM_SHARE("mainram")

	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P1")
	AM_RANGE(0xa040, 0xa040) AM_READ_PORT("P2")
	AM_RANGE(0xa080, 0xa080) AM_READ_PORT("DSW")
	AM_RANGE(0xa0c0, 0xa0c0) AM_READ_PORT("SYSTEM")

	AM_RANGE(0xa000, 0xa000) AM_WRITE(irq_enable_w)
	AM_RANGE(0xa001, 0xa001) AM_WRITENOP    /* ??? */
	AM_RANGE(0xa002, 0xa002) AM_WRITE(gfxbank_w)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(flipscreen_w)
	AM_RANGE(0xa006, 0xa006) AM_WRITENOP    /* MCU is not used, but some leftover code still writes here */
	AM_RANGE(0xa007, 0xa007) AM_WRITENOP    /* MCU is not used, but some leftover code still writes here */

	AM_RANGE(0xa040, 0xa04f) AM_WRITEONLY AM_SHARE("spriteram2")
	AM_RANGE(0xa060, 0xa06f) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xa080, 0xa080) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xa0c0, 0xa0c0) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( exctsccr_main_map, AS_PROGRAM, 8, champbas_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x63ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x7c00, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0x87ff) AM_RAM_WRITE(tilemap_w) AM_SHARE("vram")
	AM_RANGE(0x8800, 0x8fff) AM_RAM AM_SHARE("mainram")

	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P1")
	AM_RANGE(0xa040, 0xa040) AM_READ_PORT("P2")
	AM_RANGE(0xa080, 0xa080) AM_READ_PORT("DSW")
	AM_RANGE(0xa0c0, 0xa0c0) AM_READ_PORT("SYSTEM")

	AM_RANGE(0xa000, 0xa000) AM_WRITE(irq_enable_w)
//  AM_RANGE(0xa001, 0xa001) AM_WRITENOP    /* ??? */
	AM_RANGE(0xa002, 0xa002) AM_WRITE(gfxbank_w)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(flipscreen_w)
	AM_RANGE(0xa006, 0xa006) AM_WRITE(champbas_mcu_halt_w)
	AM_RANGE(0xa007, 0xa007) AM_WRITENOP /* This is also MCU control, but I don't need it */

	AM_RANGE(0xa040, 0xa04f) AM_WRITEONLY AM_SHARE("spriteram2")
	AM_RANGE(0xa060, 0xa06f) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xa080, 0xa080) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xa0c0, 0xa0c0) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END



static ADDRESS_MAP_START( champbas_sub_map, AS_PROGRAM, 8, champbas_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x1fff) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x8000, 0x8000) AM_MIRROR(0x1fff) AM_WRITENOP // 4-bit return code to main CPU (not used)
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x1fff) AM_WRITE(soundlatch_clear_byte_w)
	AM_RANGE(0xc000, 0xc000) AM_MIRROR(0x1fff) AM_WRITE(dac1_w)
	AM_RANGE(0xe000, 0xe3ff) AM_MIRROR(0x1c00) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( exctsccr_sub_map, AS_PROGRAM, 8, champbas_state )
	AM_RANGE(0x0000, 0x8fff) AM_ROM
	AM_RANGE(0xa000, 0xa7ff) AM_RAM
	AM_RANGE(0xc008, 0xc008) AM_WRITE(dac1_w)
	AM_RANGE(0xc009, 0xc009) AM_WRITE(dac2_w)
	AM_RANGE(0xc00c, 0xc00c) AM_WRITE(soundlatch_clear_byte_w)
	AM_RANGE(0xc00d, 0xc00d) AM_READ(soundlatch_byte_r)
//  AM_RANGE(0xc00f, 0xc00f) AM_WRITENOP /* ??? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( exctsccr_sound_io_map, AS_IO, 8, champbas_state )
	ADDRESS_MAP_GLOBAL_MASK( 0x00ff )
	AM_RANGE(0x82, 0x83) AM_DEVWRITE("ay1", ay8910_device, data_address_w)
	AM_RANGE(0x86, 0x87) AM_DEVWRITE("ay2", ay8910_device, data_address_w)
	AM_RANGE(0x8a, 0x8b) AM_DEVWRITE("ay3", ay8910_device, data_address_w)
	AM_RANGE(0x8e, 0x8f) AM_DEVWRITE("ay4", ay8910_device, data_address_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( mcu_map, AS_PROGRAM, 8, champbas_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("share1") /* main CPU shared RAM */
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( talbot )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, champbas_state, champbas_watchdog_bit2, NULL) // bit 2 of the watchdog counter

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( champbas )
	PORT_INCLUDE( talbot )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // throw (red)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) // changes (blue)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) // steal (yellow)

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL // steal (yellow)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL // changes (blue)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL // throw (red)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, "A 2/1 B 3/2" )
	PORT_DIPSETTING(    0x02, "A 1/1 B 2/1")
	PORT_DIPSETTING(    0x01, "A 1/2 B 1/6" )
	PORT_DIPSETTING(    0x00, "A 1/3 B 1/6")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ))
	PORT_DIPUNKNOWN( 0x40, 0x00 )
INPUT_PORTS_END

static INPUT_PORTS_START( exctsccr )
	PORT_INCLUDE( talbot )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, "A 1C/1C B 3C/1C" )
	PORT_DIPSETTING(    0x01, "A 1C/2C B 1C/4C" )
	PORT_DIPSETTING(    0x00, "A 1C/3C B 1C/6C" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Game_Time ) )
	PORT_DIPSETTING(    0x20, "1 Min." )
	PORT_DIPSETTING(    0x00, "2 Min." )
	PORT_DIPSETTING(    0x60, "3 Min." )
	PORT_DIPSETTING(    0x40, "4 Min." )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8,1), STEP4(0,1) },
	{ STEP8(0,8) },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1), STEP4(0,1) },
	{ STEP8(0,8), STEP8(32*8,8) },
	64*8
};

static GFXDECODE_START( talbot )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x100, 0x100>>2 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0x000, 0x100>>2 )
GFXDECODE_END

static GFXDECODE_START( champbas )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 0x200>>2 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 0x200>>2 )
GFXDECODE_END


static const gfx_layout charlayout_3bpp =
{
	8,8,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2)+4, 0, 4 },
	{ STEP4(8*8,1), STEP4(0,1) },
	{ STEP8(0,8) },
	16*8
};

static const gfx_layout spritelayout_3bpp =
{
	16,16,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2)+4, 0, 4 },
	{ STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1), STEP4(0,1) },
	{ STEP8(0,8), STEP8(32*8,8) },
	64*8
};

static const gfx_layout spritelayout_4bpp =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1), STEP4(0,1) },
	{ STEP8(0,8), STEP8(32*8,8) },
	64*8
};

static GFXDECODE_START( exctsccr )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_3bpp,   0x000, 0x080>>3 ) /* chars */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout_3bpp, 0x080, 0x080>>3 ) /* sprites */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout_4bpp, 0x100, 0x100>>4 ) /* sprites */
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void champbas_state::machine_start()
{
	// zerofill
	m_irq_mask = 0;
	m_palette_bank = 0;
	m_gfx_bank = 0;

	// register for savestates
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_gfx_bank));
}

void champbas_state::machine_reset()
{
	// 74LS259 is auto CLR on reset
	for (int i = 0; i < 8; i++)
		m_maincpu->space(AS_PROGRAM).write_byte(0xa000 + i, 0);
}

INTERRUPT_GEN_MEMBER(champbas_state::vblank_irq)
{
	if (m_irq_mask)
		device.execute().set_input_line(0, ASSERT_LINE);
}


static MACHINE_CONFIG_START( talbot, champbas_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_18_432MHz/6)
	MCFG_CPU_PROGRAM_MAP(talbot_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", champbas_state, vblank_irq)

	MCFG_CPU_ADD("mcu", ALPHA8201L, XTAL_18_432MHz/6/8)
	MCFG_CPU_PROGRAM_MAP(mcu_map)

	MCFG_WATCHDOG_VBLANK_INIT(0x10)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(champbas_state, screen_update_champbas)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", talbot)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(champbas_state,champbas)
	MCFG_VIDEO_START_OVERRIDE(champbas_state,champbas)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_18_432MHz/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( champbas, champbas_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_18_432MHz/6)
	MCFG_CPU_PROGRAM_MAP(champbas_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", champbas_state, vblank_irq)

	MCFG_CPU_ADD("sub", Z80, XTAL_18_432MHz/6)
	MCFG_CPU_PROGRAM_MAP(champbas_sub_map)

	MCFG_WATCHDOG_VBLANK_INIT(0x10)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(champbas_state, screen_update_champbas)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", champbas)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(champbas_state,champbas)
	MCFG_VIDEO_START_OVERRIDE(champbas_state,champbas)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_18_432MHz/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( champmcu, champbas )

	/* basic machine hardware */

	/* MCU */
	MCFG_CPU_ADD("mcu", ALPHA8201L, XTAL_18_432MHz/6/8)
	MCFG_CPU_PROGRAM_MAP(mcu_map)

	/* to MCU timeout champbbj */
	MCFG_QUANTUM_TIME(attotime::from_hz(3000))
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( exctsccr, champbas_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_18_432MHz/6 )
	MCFG_CPU_PROGRAM_MAP(exctsccr_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", champbas_state, vblank_irq)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_14_31818MHz/4 )
	MCFG_CPU_PROGRAM_MAP(exctsccr_sub_map)
	MCFG_CPU_IO_MAP(exctsccr_sound_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(champbas_state, nmi_line_pulse, 4000) // 4 kHz, updates the dac

	MCFG_TIMER_DRIVER_ADD_PERIODIC("exc_snd_irq", champbas_state, exctsccr_sound_irq, attotime::from_hz(75)) // irq source unknown, determines music tempo
	MCFG_TIMER_START_DELAY(attotime::from_hz(75))

	/* MCU */
	MCFG_CPU_ADD("mcu", ALPHA8301L, XTAL_18_432MHz/6/8)     /* Actually 8302 */
	MCFG_CPU_PROGRAM_MAP(mcu_map)

	MCFG_WATCHDOG_VBLANK_INIT(0x10)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.54)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(champbas_state, screen_update_exctsccr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", exctsccr)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(champbas_state,exctsccr)
	MCFG_VIDEO_START_OVERRIDE(champbas_state,exctsccr)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	/* AY (melody) clock is specified by a VR (0.9 - 3.9 MHz) */
	MCFG_SOUND_ADD("ay1", AY8910, 1940000) /* VR has a factory mark and this is the value read */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.08)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_14_31818MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.08)

	MCFG_SOUND_ADD("ay3", AY8910, XTAL_14_31818MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.08)

	MCFG_SOUND_ADD("ay4", AY8910, XTAL_14_31818MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.08)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

/* Bootleg running on a modified Champion Baseball board */
static MACHINE_CONFIG_START( exctsccrb, champbas_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_18_432MHz/6)
	MCFG_CPU_PROGRAM_MAP(exctsccrb_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", champbas_state, vblank_irq)

	MCFG_CPU_ADD("sub", Z80, XTAL_18_432MHz/6)
	MCFG_CPU_PROGRAM_MAP(champbas_sub_map)

	MCFG_WATCHDOG_VBLANK_INIT(0x10)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(champbas_state, screen_update_exctsccr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", exctsccr)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(champbas_state,exctsccr)
	MCFG_VIDEO_START_OVERRIDE(champbas_state,exctsccr)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_18_432MHz/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( talbot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11.10g", 0x0000, 0x1000, CRC(0368607d) SHA1(275a29fb018bd327e64cf4fcc04590099c90290a) )
	ROM_LOAD( "12.11g", 0x1000, 0x1000, CRC(400e633b) SHA1(8d76df34174286e2b0c9341bbc141c9e77533f06) )
	ROM_LOAD( "13.10h", 0x2000, 0x1000, CRC(be575d9e) SHA1(17d3bbdc755920b5a6e1e81cbb7d51be20257ff1) )
	ROM_LOAD( "14.11h", 0x3000, 0x1000, CRC(56464614) SHA1(21cfcf3212e0a74c695ce1d6412d630a7141b2c9) )
	ROM_LOAD( "15.10i", 0x4000, 0x1000, CRC(0225b7ef) SHA1(9adee4831eb633b0a31580596205a655df94c2b2) )
	ROM_LOAD( "16.11i", 0x5000, 0x1000, CRC(1612adf5) SHA1(9adeb21d5d1692f6e31460062f03f2008076b307) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8201_44801a75_2f25.bin", 0x0000, 0x2000, CRC(b77931ac) SHA1(405b02585e80d95a2821455538c5c2c31ce262d1) )

	ROM_REGION( 0x1000, "gfx1", 0 ) // chars
	ROM_LOAD( "7.6a", 0x0000, 0x1000, CRC(bde14194) SHA1(f8f569342a3094eb5450a30b8ab87901b98e6061) )

	ROM_REGION( 0x1000, "gfx2", 0 ) // sprites
	ROM_LOAD( "8.6b", 0x0000, 0x1000, CRC(ddcd227a) SHA1(c44de36311cd173afb3eebf8487305b06e069c0f) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "mb7051.7h", 0x0000, 0x0020, CRC(7a153c60) SHA1(4b147c63e467cca7359acb5f3652ed9db9a36cc8) )
	ROM_LOAD( "mb7052.5e", 0x0020, 0x0100, CRC(a3189986) SHA1(f113c1253ba2f8f213c600e93a39c0957a933306) )
ROM_END

ROM_START( champbas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "champbb.1", 0x0000, 0x2000, CRC(218de21e) SHA1(7577fd04bdda4666c017f3b36e81ec23bcddd845) )
	ROM_LOAD( "champbb.2", 0x2000, 0x2000, CRC(5ddd872e) SHA1(68e21572e27707c991180b1bd0a6b31f7b64abf6) )
	ROM_LOAD( "champbb.3", 0x4000, 0x2000, CRC(f39a7046) SHA1(3097bffe84ac74ce9e6481028a0ebbe8b1d6eaf9) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "champbb.6", 0x0000, 0x2000, CRC(26ab3e16) SHA1(019b9d34233a6b7a53e204154b782ceb42915d2b) )
	ROM_LOAD( "champbb.7", 0x2000, 0x2000, CRC(7c01715f) SHA1(b15b2001b8c110f2599eee3aeed79f67686ebd7e) )
	ROM_LOAD( "champbb.8", 0x4000, 0x2000, CRC(3c911786) SHA1(eea0c467e213d237b5bb9d04b19a418d6090c2dc) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "champbb.4", 0x0000, 0x2000, CRC(1930fb52) SHA1(cae0b2701c2b53b79e9df3a7496442ba3472e996) )

	ROM_REGION( 0x2000, "gfx2", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "champbb.5", 0x0000, 0x2000, CRC(a4cef5a1) SHA1(fa00ed0d075e00992a1ddce3c1327ed74770a735) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "champbb.pr2", 0x0000, 0x020, CRC(2585ffb0) SHA1(ce7f62f37955c2bbb4f82b139cc716978b084767) ) /* palette */
	ROM_LOAD( "champbb.pr1", 0x0020, 0x100, CRC(872dd450) SHA1(6c1e2c4a2fc072f4bf4996c731adb0b01b347506) ) /* look-up table */
ROM_END

ROM_START( champbasj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11.2e", 0x0000, 0x2000, CRC(e2dfc166) SHA1(482e084d7d21b1cf2d17431699e6bab4c4b6ac15) )
	ROM_LOAD( "12.2g", 0x2000, 0x2000, CRC(7b4e5faa) SHA1(b7201816a819ef313ddc81f312d26982b83ef1c7) )
	ROM_LOAD( "13.2h", 0x4000, 0x2000, CRC(b201e31f) SHA1(bba3b611ff60ad8d5dd8484df4cfc2026f4fd344) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "16.2k", 0x0000, 0x2000, CRC(24c482ee) SHA1(c25bdf77014e095fc11a9a6b17f16858f19db451) )
	ROM_LOAD( "17.2l", 0x2000, 0x2000, CRC(f10b148b) SHA1(d66516d509f6f16e51ee59d27c4867e276064c3f) )
	ROM_LOAD( "18.2n", 0x4000, 0x2000, CRC(2dc484dd) SHA1(28bd68c787d7e6989849ca52009948dbd5cdcc79) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8201_44801a75_2f25.bin", 0x0000, 0x2000, CRC(b77931ac) SHA1(405b02585e80d95a2821455538c5c2c31ce262d1) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "14.5e", 0x0000, 0x2000, CRC(1b8202b3) SHA1(889b77fc3d0cb029baf8c47be260f513f3ed59bd) )

	ROM_REGION( 0x2000, "gfx2", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "15.5g", 0x0000, 0x2000, CRC(a67c0c40) SHA1(3845839eff8c1624d26937f28ffde67a5fcb4805) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "1e.bpr", 0x0000, 0x0020, CRC(f5ce825e) SHA1(956f580840f1a7d24bfbd72b2929d14e9ee1b660) ) /* palette */
	ROM_LOAD( "5k.bpr", 0x0020, 0x0100, CRC(2e481ffa) SHA1(bc8979efd43bee8be0ce96ebdacc873a5821e06e) ) /* look-up table */
ROM_END

ROM_START( champbasja )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10", 0x0000, 0x2000, CRC(f7cdaf8e) SHA1(d4c840f2107394fadbcf822d64aaa381ac900367) )
	ROM_LOAD( "09", 0x2000, 0x2000, CRC(9d39e5b3) SHA1(11c1a1d2296c0bf16d7610eaa79b034bfd813740) )
	ROM_LOAD( "08", 0x4000, 0x2000, CRC(53468a0f) SHA1(d4b5ea48b27754eebe593c8b4fcf5bf117f27ae4) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "16.2k", 0x0000, 0x2000, CRC(24c482ee) SHA1(c25bdf77014e095fc11a9a6b17f16858f19db451) )
	ROM_LOAD( "17.2l", 0x2000, 0x2000, CRC(f10b148b) SHA1(d66516d509f6f16e51ee59d27c4867e276064c3f) )
	ROM_LOAD( "18.2n", 0x4000, 0x2000, CRC(2dc484dd) SHA1(28bd68c787d7e6989849ca52009948dbd5cdcc79) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "14.5e", 0x0000, 0x2000, CRC(1b8202b3) SHA1(889b77fc3d0cb029baf8c47be260f513f3ed59bd) )

	ROM_REGION( 0x2000, "gfx2", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "15.5g", 0x0000, 0x2000, CRC(a67c0c40) SHA1(3845839eff8c1624d26937f28ffde67a5fcb4805) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "clr",    0x0000, 0x0020, CRC(8f989357) SHA1(d0916fb5ef4b43bdf84663cd403418ffc5e98c17) ) /* palette */
	ROM_LOAD( "5k.bpr", 0x0020, 0x0100, CRC(2e481ffa) SHA1(bc8979efd43bee8be0ce96ebdacc873a5821e06e) ) /* look-up table */
ROM_END

ROM_START( champbb2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr5932", 0x0000, 0x2000, CRC(528e3c78) SHA1(ee300201580c1bace783f1340bd4f1ea2a00dffa) )
	ROM_LOAD( "epr5929", 0x2000, 0x2000, CRC(17b6057e) SHA1(67c5aed950acf4d045edf39019066af2896265e1) )
	ROM_LOAD( "epr5930", 0x4000, 0x2000, CRC(b6570a90) SHA1(5a2651aeac986000913b5854792b2d81df6b2fc6) )
	ROM_LOAD( "epr5931", 0x7800, 0x0800, CRC(0592434d) SHA1(a7f61546c39ffdbff46c4db485c9b3f6eefcf1ac) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "epr5933", 0x0000, 0x2000, CRC(26ab3e16) SHA1(019b9d34233a6b7a53e204154b782ceb42915d2b) )
	ROM_LOAD( "epr5934", 0x2000, 0x2000, CRC(7c01715f) SHA1(b15b2001b8c110f2599eee3aeed79f67686ebd7e) )
	ROM_LOAD( "epr5935", 0x4000, 0x2000, CRC(3c911786) SHA1(eea0c467e213d237b5bb9d04b19a418d6090c2dc) )

	// the pcb has a 8302 on it, though only the 8201 instructions are used
	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8302_44801b35.bin", 0x0000, 0x2000, CRC(edabac6c) SHA1(eaf1c51b63023256df526b0d3fd53cffc919c901) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "epr5936", 0x0000, 0x2000, CRC(c4a4df75) SHA1(7b85dbf405697b0b8881f910c08f6db6c828b19a) )

	ROM_REGION( 0x2000, "gfx2", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "epr5937", 0x0000, 0x2000, CRC(5c80ec42) SHA1(9b79737577e48a6b2ec20ce145252545955e82c3) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "pr5957", 0x0000, 0x020, CRC(f5ce825e) SHA1(956f580840f1a7d24bfbd72b2929d14e9ee1b660) ) /* palette */
	ROM_LOAD( "pr5956", 0x0020, 0x100, CRC(872dd450) SHA1(6c1e2c4a2fc072f4bf4996c731adb0b01b347506) ) /* look-up table */
ROM_END

ROM_START( champbb2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin", 0x0000, 0x2000, CRC(9b75b44d) SHA1(35b67638a5e48cbe999907e3c9c3a33da9d76bba) )
	ROM_LOAD( "2.bin", 0x2000, 0x2000, CRC(736a1b62) SHA1(24c2d57506754ca789b378a595c03b7591eb5b5c) )
	ROM_LOAD( "3.bin", 0x4000, 0x2000, CRC(cf5f28cb) SHA1(d553f2085c9c8c77b241b4239cc1ad1764b490d0) )
	ROM_LOAD( "4.bin", 0x7800, 0x0800, NO_DUMP )

	/* not in this set, but probably the same */
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "epr5933", 0x0000, 0x2000, CRC(26ab3e16) SHA1(019b9d34233a6b7a53e204154b782ceb42915d2b) )
	ROM_LOAD( "epr5934", 0x2000, 0x2000, CRC(7c01715f) SHA1(b15b2001b8c110f2599eee3aeed79f67686ebd7e) )
	ROM_LOAD( "epr5935", 0x4000, 0x2000, CRC(3c911786) SHA1(eea0c467e213d237b5bb9d04b19a418d6090c2dc) )

	// the pcb has a 8302 on it, though only the 8201 instructions are used
	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8302_44801b35.bin", 0x0000, 0x2000, CRC(edabac6c) SHA1(eaf1c51b63023256df526b0d3fd53cffc919c901) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "epr5936", 0x0000, 0x2000, CRC(c4a4df75) SHA1(7b85dbf405697b0b8881f910c08f6db6c828b19a) )

	ROM_REGION( 0x2000, "gfx2", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "epr5937", 0x0000, 0x2000, CRC(5c80ec42) SHA1(9b79737577e48a6b2ec20ce145252545955e82c3) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "pr5957", 0x0000, 0x020, CRC(f5ce825e) SHA1(956f580840f1a7d24bfbd72b2929d14e9ee1b660) ) /* palette */
	ROM_LOAD( "pr5956", 0x0020, 0x100, CRC(872dd450) SHA1(6c1e2c4a2fc072f4bf4996c731adb0b01b347506) ) /* look-up table */
ROM_END

ROM_START( champbb2j )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.10g", 0x0000, 0x2000, CRC(c76056d5) SHA1(2aaec9ede0f85bf67f6cf04fabdba66f7e0d6004) )
	ROM_LOAD( "2.10h", 0x2000, 0x2000, CRC(7a1ea3ea) SHA1(ce2f61be4cc7cd7b739d89a4838408912c2e2784) )
	ROM_LOAD( "3.10i", 0x4000, 0x2000, CRC(4b2f6ac4) SHA1(367b25665fc37140dc38be8bb525859c20bd2529) )
	ROM_LOAD( "0.11g", 0x7800, 0x0800, CRC(be0e180d) SHA1(7a8915e00920faa08344d752404a6f98d8fb303b) )

	/* not in this set, but probably the same */
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "6.15c", 0x0000, 0x2000, CRC(24c482ee) SHA1(c25bdf77014e095fc11a9a6b17f16858f19db451) )
	ROM_LOAD( "7.15d", 0x2000, 0x2000, CRC(f10b148b) SHA1(d66516d509f6f16e51ee59d27c4867e276064c3f) )
	ROM_LOAD( "8.15e", 0x4000, 0x2000, CRC(2dc484dd) SHA1(28bd68c787d7e6989849ca52009948dbd5cdcc79) )

	// the pcb has a 8302 on it, though only the 8201 instructions are used
	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8302_44801b35.bin", 0x0000, 0x2000, CRC(edabac6c) SHA1(eaf1c51b63023256df526b0d3fd53cffc919c901) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "4.6a", 0x0000, 0x2000, CRC(c4a4df75) SHA1(7b85dbf405697b0b8881f910c08f6db6c828b19a) )

	ROM_REGION( 0x2000, "gfx2", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "5.6b", 0x0000, 0x2000, CRC(5c80ec42) SHA1(9b79737577e48a6b2ec20ce145252545955e82c3) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "bpr.7h", 0x0000, 0x020, CRC(500db3d9) SHA1(7317ba2c6ffef3561acb3b2adb811def846756cf) ) /* palette */
	ROM_LOAD( "bpr.5e", 0x0020, 0x100, CRC(2e481ffa) SHA1(bc8979efd43bee8be0ce96ebdacc873a5821e06e) ) /* look-up table */
ROM_END

ROM_START( exctsccr ) /* Teams: ITA AUS GBR FRA FRG BRA */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1_g10.bin",    0x0000, 0x2000, CRC(aa68df66) SHA1(f10cac5a4c5aad1e1eb8835174dc8d517bb2921a) )
	ROM_LOAD( "2_h10.bin",    0x2000, 0x2000, CRC(2d8f8326) SHA1(8809e7b081fa2a1966cb51ac969fd7b468d35be0) )
	ROM_LOAD( "3_j10.bin",    0x4000, 0x2000, CRC(dce4a04d) SHA1(9c015e4597ec8921bea213d9841fc69c776a4e6d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "0_h6.bin",     0x0000, 0x2000, CRC(3babbd6b) SHA1(b81bd47c4449f4f21f2d55d01eb9cb6db10664c7) )
	ROM_LOAD( "9_f6.bin",     0x2000, 0x2000, CRC(639998f5) SHA1(c4ff5e5e75d53dea38449f323186d08d5b57bf90) )
	ROM_LOAD( "8_d6.bin",     0x4000, 0x2000, CRC(88651ee1) SHA1(2052e1b3f9784439369f464e31f4a2b0d1bb0565) )
	ROM_LOAD( "7_c6.bin",     0x6000, 0x2000, CRC(6d51521e) SHA1(2809bd2e61f40dcd31d43c62520982bdcfb0a865) )
	ROM_LOAD( "1_a6.bin",     0x8000, 0x1000, CRC(20f2207e) SHA1(b1ed2237d0bd50ddbe593fd2fbff9f1d67c1eb11) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8302_44801b35.bin", 0x0000, 0x2000, CRC(edabac6c) SHA1(eaf1c51b63023256df526b0d3fd53cffc919c901) )

	ROM_REGION( 0x04000, "gfx1", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "4_a5.bin",     0x0000, 0x2000, CRC(c342229b) SHA1(a989d6c12521c77882a7e17d4d80afe7eae05906) ) /* planes 0,1 */
	ROM_LOAD( "6_c5.bin",     0x2000, 0x2000, CRC(eda40e32) SHA1(6c08fd4f4fb35fd354d02e04548e960c545f6a88) ) /* plane 3 */

	ROM_REGION( 0x04000, "gfx2", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "5_b5.bin",     0x0000, 0x2000, CRC(35f4f8c9) SHA1(cdf5bbfea9abdd338938e5f4499d2d71ce3c6237) ) /* planes 0,1 */

	ROM_REGION( 0x02000, "gfx3", 0 )    // 4bpp sprites
	ROM_LOAD( "2.5k",     0x0000, 0x1000, CRC(7f9cace2) SHA1(bf05a31716f3ca1c2fd1034cd1f39e2d21cdaed3) )
	ROM_LOAD( "3.5l",     0x1000, 0x1000, CRC(db2d9e0d) SHA1(6ec09a47f7aea6bf31eb0ee78f44012f4d92de8a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom3.k5",     0x0020, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
	ROM_LOAD( "prom2.8r",     0x0120, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
ROM_END

ROM_START( exctsccra ) /* Teams: ITA AUS GBR FRA FRG BRA */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1_g10.bin",    0x0000, 0x2000, CRC(aa68df66) SHA1(f10cac5a4c5aad1e1eb8835174dc8d517bb2921a) )
	ROM_LOAD( "2_h10.bin",    0x2000, 0x2000, CRC(2d8f8326) SHA1(8809e7b081fa2a1966cb51ac969fd7b468d35be0) )
	ROM_LOAD( "3_j10.bin",    0x4000, 0x2000, CRC(dce4a04d) SHA1(9c015e4597ec8921bea213d9841fc69c776a4e6d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "exctsccc.000", 0x0000, 0x2000, CRC(642fc42f) SHA1(cfc849d18e347e3e23fc31c1ce7f2580d5d9b2b0) )
	ROM_LOAD( "exctsccc.009", 0x2000, 0x2000, CRC(d88b3236) SHA1(80f083fb15243e9e68978677caed8aee8e3109a0) )
	ROM_LOAD( "8_d6.bin",     0x4000, 0x2000, CRC(88651ee1) SHA1(2052e1b3f9784439369f464e31f4a2b0d1bb0565) )
	ROM_LOAD( "7_c6.bin",     0x6000, 0x2000, CRC(6d51521e) SHA1(2809bd2e61f40dcd31d43c62520982bdcfb0a865) )
	ROM_LOAD( "1_a6.bin",     0x8000, 0x1000, CRC(20f2207e) SHA1(b1ed2237d0bd50ddbe593fd2fbff9f1d67c1eb11) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8302_44801b35.bin", 0x0000, 0x2000, CRC(edabac6c) SHA1(eaf1c51b63023256df526b0d3fd53cffc919c901) )

	ROM_REGION( 0x04000, "gfx1", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "4_a5.bin",     0x0000, 0x2000, CRC(c342229b) SHA1(a989d6c12521c77882a7e17d4d80afe7eae05906) ) /* planes 0,1 */
	ROM_LOAD( "6_c5.bin",     0x2000, 0x2000, CRC(eda40e32) SHA1(6c08fd4f4fb35fd354d02e04548e960c545f6a88) ) /* plane 3 */

	ROM_REGION( 0x04000, "gfx2", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "5_b5.bin",     0x0000, 0x2000, CRC(35f4f8c9) SHA1(cdf5bbfea9abdd338938e5f4499d2d71ce3c6237) ) /* planes 0,1 */

	ROM_REGION( 0x02000, "gfx3", 0 )    // 4bpp sprites
	ROM_LOAD( "2.5k",     0x0000, 0x1000, CRC(7f9cace2) SHA1(bf05a31716f3ca1c2fd1034cd1f39e2d21cdaed3) )
	ROM_LOAD( "3.5l",     0x1000, 0x1000, CRC(db2d9e0d) SHA1(6ec09a47f7aea6bf31eb0ee78f44012f4d92de8a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom3.k5",     0x0020, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
	ROM_LOAD( "prom2.8r",     0x0120, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
ROM_END

ROM_START( exctsccru ) /* Teams: ITA USA GBR FRA FRG BRA */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vr1u.g10",    0x0000, 0x2000, CRC(ef39676d) SHA1(f7728d86b2c68bb93a0fcf02931cda8cb65e6d48) ) /* Team USA in place of Austria */
	ROM_LOAD( "vr2u.h10",    0x2000, 0x2000, CRC(37994b86) SHA1(681a27a009909cc8d26f8046c54532ec56145f97) )
	ROM_LOAD( "vr3u.j10",    0x4000, 0x2000, CRC(2ed3c6bb) SHA1(d3bad24cbbb34eb6c43cb603cbf66ab35be2c845) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "vr0u.6h",      0x0000, 0x2000, CRC(cbb035c6) SHA1(e89e69678335edf1cadc7b7949f8cfe47dfabc46) )
	ROM_LOAD( "9_f6.bin",     0x2000, 0x2000, CRC(639998f5) SHA1(c4ff5e5e75d53dea38449f323186d08d5b57bf90) )
	ROM_LOAD( "8_d6.bin",     0x4000, 0x2000, CRC(88651ee1) SHA1(2052e1b3f9784439369f464e31f4a2b0d1bb0565) )
	ROM_LOAD( "7_c6.bin",     0x6000, 0x2000, CRC(6d51521e) SHA1(2809bd2e61f40dcd31d43c62520982bdcfb0a865) )
	ROM_LOAD( "1_a6.bin",     0x8000, 0x1000, CRC(20f2207e) SHA1(b1ed2237d0bd50ddbe593fd2fbff9f1d67c1eb11) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8302_44801b35.bin", 0x0000, 0x2000, CRC(edabac6c) SHA1(eaf1c51b63023256df526b0d3fd53cffc919c901) )

	ROM_REGION( 0x04000, "gfx1", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "vr4u.a5",     0x0000, 0x2000, CRC(103bb739) SHA1(335d89b3a374daa3fd1bd3fd66a82e7310303051) ) /* planes 0,1 */
	ROM_LOAD( "vr6u.c5",     0x2000, 0x2000, CRC(a5b2b303) SHA1(0dd1912baa8236cba2baa4bc3d2955fd19617be9) ) /* plane 3 */

	ROM_REGION( 0x04000, "gfx2", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "5_b5.bin",     0x0000, 0x2000, CRC(35f4f8c9) SHA1(cdf5bbfea9abdd338938e5f4499d2d71ce3c6237) ) /* planes 0,1 */

	ROM_REGION( 0x02000, "gfx3", 0 )    // 4bpp sprites
	ROM_LOAD( "2.5k",     0x0000, 0x1000, CRC(7f9cace2) SHA1(bf05a31716f3ca1c2fd1034cd1f39e2d21cdaed3) )
	ROM_LOAD( "3.5l",     0x1000, 0x1000, CRC(db2d9e0d) SHA1(6ec09a47f7aea6bf31eb0ee78f44012f4d92de8a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom3.k5",     0x0020, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
	ROM_LOAD( "prom2.8r",     0x0120, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
ROM_END

ROM_START( exctsccrj ) /* Teams: JPN USA GBR FRA FRG BRA */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1_10g.bin",    0x0000, 0x2000, CRC(310298a2) SHA1(b05a697ec2ed1bf4947fcc5f823ed9cb8daeee15) ) /* Corrects "ENG" to GBR & "GFR" to FGR */
	ROM_LOAD( "2_10h.bin",    0x2000, 0x2000, CRC(030fd0b7) SHA1(a4c57c5eb1c76dc7e5d9be48036f21331f9529d9) )
	ROM_LOAD( "3_10j.bin",    0x4000, 0x2000, CRC(1a51ff1f) SHA1(2a657f95807bfbf172f7d22e20b9ce75f453d028) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "0_h6.bin",     0x0000, 0x2000, CRC(3babbd6b) SHA1(b81bd47c4449f4f21f2d55d01eb9cb6db10664c7) )
	ROM_LOAD( "9_f6.bin",     0x2000, 0x2000, CRC(639998f5) SHA1(c4ff5e5e75d53dea38449f323186d08d5b57bf90) )
	ROM_LOAD( "8_d6.bin",     0x4000, 0x2000, CRC(88651ee1) SHA1(2052e1b3f9784439369f464e31f4a2b0d1bb0565) )
	ROM_LOAD( "7_c6.bin",     0x6000, 0x2000, CRC(6d51521e) SHA1(2809bd2e61f40dcd31d43c62520982bdcfb0a865) )
	ROM_LOAD( "1_a6.bin",     0x8000, 0x1000, CRC(20f2207e) SHA1(b1ed2237d0bd50ddbe593fd2fbff9f1d67c1eb11) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8302_44801b35.bin", 0x0000, 0x2000, CRC(edabac6c) SHA1(eaf1c51b63023256df526b0d3fd53cffc919c901) )

	ROM_REGION( 0x04000, "gfx1", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "4_5a.bin",     0x0000, 0x2000, CRC(74cc71d6) SHA1(ff3d59845bc66ec3335eadf81d799a684182c66f) ) /* planes 0,1 */
	ROM_LOAD( "6_5c.bin",     0x2000, 0x2000, CRC(7c4cd1b6) SHA1(141e67fec9b6d6b4380cb941b4d79341787680e3) ) /* plane 3 */

	ROM_REGION( 0x04000, "gfx2", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "5_5b.bin",     0x0000, 0x2000, CRC(35f4f8c9) SHA1(cdf5bbfea9abdd338938e5f4499d2d71ce3c6237) ) /* planes 0,1 */

	ROM_REGION( 0x02000, "gfx3", 0 )    // 4bpp sprites
	ROM_LOAD( "2.5k",         0x0000, 0x1000, CRC(7f9cace2) SHA1(bf05a31716f3ca1c2fd1034cd1f39e2d21cdaed3) )
	ROM_LOAD( "3.5l",         0x1000, 0x1000, CRC(db2d9e0d) SHA1(6ec09a47f7aea6bf31eb0ee78f44012f4d92de8a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom3.k5",     0x0020, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
	ROM_LOAD( "prom2.8r",     0x0120, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
ROM_END

ROM_START( exctsccrjo ) /* Teams: JPN USA ENG FRA GFR BRA */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.10g",    0x0000, 0x2000, CRC(d1bfdf75) SHA1(a4a9bb340712401b1d24705c26d996a798776d4f) )
	ROM_LOAD( "2.10h",    0x2000, 0x2000, CRC(5c61f0fe) SHA1(8c6751b80f89d8744d3eaa2a6da2cafdde968ed2) )
	ROM_LOAD( "3.10j",    0x4000, 0x2000, CRC(8f213b10) SHA1(5bffaee2725fe34b0614fcf1b4dc1c9a2f2df36c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "0.6h",     0x0000, 0x2000, CRC(548b08a2) SHA1(4cdcc67e34e56cbac5d07e9603650073de0bb5d1) )
	ROM_LOAD( "9.f6",     0x2000, 0x2000, CRC(639998f5) SHA1(c4ff5e5e75d53dea38449f323186d08d5b57bf90) )
	ROM_LOAD( "8.6d",     0x4000, 0x2000, CRC(b6b209a5) SHA1(e49a0db65b29337ac6b919237067b1990f2233ab) )
	ROM_LOAD( "7.6c",     0x6000, 0x2000, CRC(8856452a) SHA1(4494c225c9df97da09c180caadb4dda49d0d5392) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8302_44801b35.bin", 0x0000, 0x2000, CRC(edabac6c) SHA1(eaf1c51b63023256df526b0d3fd53cffc919c901) )

	ROM_REGION( 0x04000, "gfx1", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "4.5a",     0x0000, 0x2000, CRC(c4259307) SHA1(7bd4e229a5e1a5136826a57aa61810fcdf9c5027) ) /* planes 0,1 */
	ROM_LOAD( "6.5c",     0x2000, 0x2000, CRC(cca53367) SHA1(f06ebf2ab8f8f10cfe118af490017972990e3073) ) /* plane 3 */

	ROM_REGION( 0x04000, "gfx2", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "5.5b",     0x0000, 0x2000, CRC(851d1a18) SHA1(2cfad530c8f9d95094fd0aacd2e0965b0300898c) ) /* planes 0,1 */

	ROM_REGION( 0x02000, "gfx3", 0 )    // 4bpp sprites
	ROM_LOAD( "2.5k",     0x0000, 0x1000, CRC(7f9cace2) SHA1(bf05a31716f3ca1c2fd1034cd1f39e2d21cdaed3) )
	ROM_LOAD( "3.5l",     0x1000, 0x1000, CRC(db2d9e0d) SHA1(6ec09a47f7aea6bf31eb0ee78f44012f4d92de8a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom3.k5",     0x0020, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
	ROM_LOAD( "prom2.8r",     0x0120, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
ROM_END

/*
The Kazutomi bootleg board is a conversion from Champion Baseball:
Alpha denshi co. LTD made in Japan
cpu board 58AS1
Display board 58AS2
Voice board 58AS3
KAZUTOMI board
1x Alpha8201 (CPU board)
1x AY-3-8910 (CPU board)
1x Sharp Z80ACPU (CPU board)
1x Sharp Z80ACPU (VOICE board)
3x 2764 (CPU board) (1-2-3)
3x 2764 (VOICE board) (a-b-c)
2x 2764 (DISPLAY board) (4-5)
1x 2764 (daughter board kazutomi) (6)
2x 2732 (daughter board kazutomi) (7-8)
*/
ROM_START( exctsccrb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "es-1.e2",      0x0000, 0x2000, CRC(997c6a82) SHA1(60fe27a12eedd22c775b7e65c5ba692cfcf5ac74) )
	ROM_LOAD( "es-2.g2",      0x2000, 0x2000, CRC(5c66e792) SHA1(f7a7f32806965fa926261217cee3159ccd198d49) )
	ROM_LOAD( "es-3.h2",      0x4000, 0x2000, CRC(e0d504c0) SHA1(d9a9f37b3a44a05a3f3389aa9617c419a2cee661) )

	ROM_REGION( 0x10000, "sub", 0 ) /* sound */
	ROM_LOAD( "es-a.k2",      0x0000, 0x2000, CRC(99e87b78) SHA1(f12006ff3f6f3c706e06288c97a1446141373432) )
	ROM_LOAD( "es-b.l2",      0x2000, 0x2000, CRC(8b3db794) SHA1(dbfed2357c7631bfca6bbd63a23617bc3abf6ca3) )
	ROM_LOAD( "es-c.m2",      0x4000, 0x2000, CRC(7bed2f81) SHA1(cbbb0480519cc04a99e8983228b18c9e49a9985d) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8201_44801a75_2f25.bin", 0x0000, 0x2000, CRC(b77931ac) SHA1(405b02585e80d95a2821455538c5c2c31ce262d1) )

	/* the national flags are wrong. This happens on the real board */
	ROM_REGION( 0x04000, "gfx1", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "4_a5.bin",     0x0000, 0x2000, CRC(c342229b) SHA1(a989d6c12521c77882a7e17d4d80afe7eae05906) ) /* planes 0,1 */
	ROM_LOAD( "6_c5.bin",     0x2000, 0x2000, CRC(eda40e32) SHA1(6c08fd4f4fb35fd354d02e04548e960c545f6a88) ) /* plane 3 */

	ROM_REGION( 0x04000, "gfx2", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "5_b5.bin",     0x0000, 0x2000, CRC(35f4f8c9) SHA1(cdf5bbfea9abdd338938e5f4499d2d71ce3c6237) ) /* planes 0,1 */

	ROM_REGION( 0x02000, "gfx3", 0 )    // 4bpp sprites
	ROM_LOAD( "2_k5.bin",     0x0000, 0x1000, CRC(7f9cace2) SHA1(bf05a31716f3ca1c2fd1034cd1f39e2d21cdaed3) )
	ROM_LOAD( "3_l5.bin",     0x1000, 0x1000, CRC(db2d9e0d) SHA1(6ec09a47f7aea6bf31eb0ee78f44012f4d92de8a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom3.k5",     0x0020, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
	ROM_LOAD( "prom2.8r",     0x0120, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
ROM_END

ROM_START( exctscc2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vr.3j",        0x0000, 0x2000, CRC(c6115362) SHA1(6a258631abd72ef6b8d7968bb4b2bc88e89e597d) )
	ROM_LOAD( "vr.3k",        0x2000, 0x2000, CRC(de36ba00) SHA1(0a0d92e710b8c749f145571bc8a204609456d19d) )
	ROM_LOAD( "vr.3l",        0x4000, 0x2000, CRC(1ddfdf65) SHA1(313d0a7f13fc2de15aa32492c38a59fbafad9f01) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "vr.7d",        0x0000, 0x2000, CRC(2c675a43) SHA1(aa0a8dbcae955e3da92c435202f2a1ed238c377e) )
	ROM_LOAD( "vr.7e",        0x2000, 0x2000, CRC(e571873d) SHA1(2dfff24f5dac86e92612f40cf3642005c7f36ad3) )
	ROM_LOAD( "8_d6.bin",     0x4000, 0x2000, CRC(88651ee1) SHA1(2052e1b3f9784439369f464e31f4a2b0d1bb0565) )    /* vr.7f */
	ROM_LOAD( "7_c6.bin",     0x6000, 0x2000, CRC(6d51521e) SHA1(2809bd2e61f40dcd31d43c62520982bdcfb0a865) )    /* vr.7h */
	ROM_LOAD( "1_a6.bin",     0x8000, 0x1000, CRC(20f2207e) SHA1(b1ed2237d0bd50ddbe593fd2fbff9f1d67c1eb11) )    /* vr.7k */

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8303_44801b42.bin", 0x0000, 0x2000, CRC(66adcb37) SHA1(e1c72ecb161129dcbddc0b16dd90e716d0c79311) )

	ROM_REGION( 0x04000, "gfx1", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "vr.5a",        0x0000, 0x2000, CRC(4ff1783d) SHA1(c45074864c3a4bcbf3a87d164027ae16dca53d9c) ) /* planes 0,1 */
	ROM_LOAD( "vr.5c",        0x2000, 0x2000, CRC(1fb84ee6) SHA1(56ceb86c509be783f806403ac21e7c9684760d5f) ) /* plane 3 */

	ROM_REGION( 0x04000, "gfx2", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "vr.5b",        0x0000, 0x2000, CRC(5605b60b) SHA1(19d5909896ae4a3d7552225c369d30475c56793b) ) /* planes 0,1 */

	ROM_REGION( 0x02000, "gfx3", 0 )    // 4bpp sprites
	ROM_LOAD( "vr.5k",        0x0000, 0x1000, CRC(1d37edfa) SHA1(184fa6dd7b1b3fff4c5fc19b42301ccb7979ac84) )
	ROM_LOAD( "vr.5l",        0x1000, 0x1000, CRC(b97f396c) SHA1(4ffe512acf047230bd593911a615fc0ef66b481d) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom3.k5",     0x0020, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
	ROM_LOAD( "prom2.8r",     0x0120, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(champbas_state,champbas)
{
	// chars and sprites are mixed in the same ROMs, so rearrange them for easier decoding
	UINT8 *rom1 = memregion("gfx1")->base();
	UINT8 *rom2 = memregion("gfx2")->base();
	int len = memregion("gfx1")->bytes();

	for (int i = 0; i < len/2; i++)
	{
		UINT8 t = rom1[i + len/2];
		rom1[i + len/2] = rom2[i];
		rom2[i] = t;
	}
}


DRIVER_INIT_MEMBER(champbas_state,exctsccr)
{
	// chars and sprites are mixed in the same ROMs, so rearrange them for easier decoding
	UINT8 *rom1 = memregion("gfx1")->base();
	UINT8 *rom2 = memregion("gfx2")->base();

	// planes 0,1
	for (int i = 0; i < 0x1000; i++)
	{
		UINT8 t = rom1[i + 0x1000];
		rom1[i + 0x1000] = rom2[i];
		rom2[i] = t;
	}

	// plane 3
	for (int i = 0; i < 0x1000; i++)
	{
		rom2[i + 0x3000] = rom1[i + 0x3000] >> 4;
		rom2[i + 0x2000] = rom1[i + 0x3000] & 0x0f;
	}
	for (int i = 0; i < 0x1000; i++)
	{
		rom1[i + 0x3000] = rom1[i + 0x2000] >> 4;
		rom1[i + 0x2000] &= 0x0f;
	}
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1982, talbot,     0,        talbot,   talbot,   driver_device,  0,        ROT270, "Alpha Denshi Co. (Volt Electronics license)", "Talbot", MACHINE_SUPPORTS_SAVE )

GAME( 1983, champbas,   0,        champbas, champbas, champbas_state, champbas, ROT0,   "Alpha Denshi Co. (Sega license)", "Champion Base Ball", MACHINE_SUPPORTS_SAVE )
GAME( 1983, champbasj,  champbas, champmcu, champbas, champbas_state, champbas, ROT0,   "Alpha Denshi Co.", "Champion Base Ball (Japan set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, champbasja, champbas, champbas, champbas, champbas_state, champbas, ROT0,   "Alpha Denshi Co.", "Champion Base Ball (Japan set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, champbb2,   0,        champmcu, champbas, champbas_state, champbas, ROT0,   "Alpha Denshi Co. (Sega license)", "Champion Base Ball Part-2: Pair Play (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, champbb2a,  champbb2, champmcu, champbas, champbas_state, champbas, ROT0,   "Alpha Denshi Co.", "Champion Baseball II (set 2)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // no dump
GAME( 1983, champbb2j,  champbb2, champmcu, champbas, champbas_state, champbas, ROT0,   "Alpha Denshi Co.", "Champion Baseball II (Japan)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

GAME( 1983, exctsccr,   0,        exctsccr, exctsccr, champbas_state, exctsccr, ROT270, "Alpha Denshi Co.", "Exciting Soccer", MACHINE_SUPPORTS_SAVE )
GAME( 1983, exctsccru,  exctsccr, exctsccr, exctsccr, champbas_state, exctsccr, ROT270, "Alpha Denshi Co.", "Exciting Soccer (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, exctsccra,  exctsccr, exctsccr, exctsccr, champbas_state, exctsccr, ROT270, "Alpha Denshi Co.", "Exciting Soccer (alternate music)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, exctsccrj,  exctsccr, exctsccr, exctsccr, champbas_state, exctsccr, ROT270, "Alpha Denshi Co.", "Exciting Soccer (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, exctsccrjo, exctsccr, exctsccr, exctsccr, champbas_state, exctsccr, ROT270, "Alpha Denshi Co.", "Exciting Soccer (Japan, older)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, exctsccrb,  exctsccr, exctsccrb,exctsccr, champbas_state, exctsccr, ROT270, "bootleg (Kazutomi)", "Exciting Soccer (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, exctscc2,   0,        exctsccr, exctsccr, champbas_state, exctsccr, ROT270, "Alpha Denshi Co.", "Exciting Soccer II", MACHINE_SUPPORTS_SAVE )
