// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Time Pilot

    driver by Nicola Salmoria

****************************************************************************

    memory map (preliminary)

    Main processor memory map.
    0000-5fff ROM
    a000-a3ff Color RAM
    a400-a7ff Video RAM
    a800-afff RAM
    b000-b7ff sprite RAM (only areas 0xb010 and 0xb410 are used).

    memory mapped ports:

    read:
    c000      video scan line. This is used by the program to multiplex the cloud
              sprites, drawing them twice offset by 128 pixels.
    c200      DSW2
    c300      IN0
    c320      IN1
    c340      IN2
    c360      DSW1

    write:
    c000      command for the audio CPU
    c200      watchdog reset
    c300      interrupt enable
    c302      flip screen
    c304      trigger interrupt on audio CPU
    c308      Protection ???  Stuffs in some values computed from ROM content
    c30a      coin counter 1
    c30c      coin counter 2

    interrupts:
    standard NMI at 0x66

    SOUND BOARD:
    same as Pooyan

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/konamipt.h"
#include "audio/timeplt.h"
#include "sound/ay8910.h"
#include "includes/timeplt.h"

#define MASTER_CLOCK         XTAL_18_432MHz

/*************************************
 *
 *  Interrupts
 *
 *************************************/

INTERRUPT_GEN_MEMBER(timeplt_state::timeplt_interrupt)
{
	if (m_nmi_enable)
		device.execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


WRITE8_MEMBER(timeplt_state::timeplt_nmi_enable_w)
{
	m_nmi_enable = data & 1;
	if (!m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}



/*************************************
 *
 *  I/O
 *
 *************************************/

WRITE8_MEMBER(timeplt_state::timeplt_coin_counter_w)
{
	coin_counter_w(machine(), offset >> 1, data);
}

READ8_MEMBER(timeplt_state::psurge_protection_r)
{
	return 0x80;
}

// chkun has access to an extra soundchip via ay2 port a
WRITE8_MEMBER(timeplt_state::chkun_sound_w)
{
	// d0-d3: P0-P3
	// d5: /R (unused?)
	// d6: /W
	if (~data & 0x40)
		m_tc8830f->write_p(data & 0xf);

	// d4 (or d7?): /ACL
	if (~data & 0x10)
		m_tc8830f->reset();
}

CUSTOM_INPUT_MEMBER(timeplt_state::chkun_hopper_status_r)
{
	// temp workaround, needs hopper
	return machine().rand();
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( timeplt_main_map, AS_PROGRAM, 8, timeplt_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0xa000, 0xa3ff) AM_RAM_WRITE(timeplt_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xa400, 0xa7ff) AM_RAM_WRITE(timeplt_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xa800, 0xafff) AM_RAM
	AM_RANGE(0xb000, 0xb0ff) AM_MIRROR(0x0b00) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xb400, 0xb4ff) AM_MIRROR(0x0b00) AM_RAM AM_SHARE("spriteram2")
	AM_RANGE(0xc000, 0xc000) AM_MIRROR(0x0cff) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xc200, 0xc200) AM_MIRROR(0x0cff) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xc300, 0xc300) AM_MIRROR(0x0cf1) AM_WRITE(timeplt_nmi_enable_w)
	AM_RANGE(0xc302, 0xc302) AM_MIRROR(0x0cf1) AM_WRITE(timeplt_flipscreen_w)
	AM_RANGE(0xc304, 0xc304) AM_MIRROR(0x0cf1) AM_DEVWRITE("timeplt_audio", timeplt_audio_device, sh_irqtrigger_w)
	AM_RANGE(0xc30a, 0xc30c) AM_MIRROR(0x0cf1) AM_WRITE(timeplt_coin_counter_w)
	AM_RANGE(0xc000, 0xc000) AM_MIRROR(0x0cff) AM_READ(timeplt_scanline_r)
	AM_RANGE(0xc200, 0xc200) AM_MIRROR(0x0cff) AM_READ_PORT("DSW1")
	AM_RANGE(0xc300, 0xc300) AM_MIRROR(0x0c9f) AM_READ_PORT("IN0")
	AM_RANGE(0xc320, 0xc320) AM_MIRROR(0x0c9f) AM_READ_PORT("IN1")
	AM_RANGE(0xc340, 0xc340) AM_MIRROR(0x0c9f) AM_READ_PORT("IN2")
	AM_RANGE(0xc360, 0xc360) AM_MIRROR(0x0c9f) AM_READ_PORT("DSW0")
ADDRESS_MAP_END

static ADDRESS_MAP_START( psurge_main_map, AS_PROGRAM, 8, timeplt_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6004, 0x6004) AM_READ(psurge_protection_r)
	AM_RANGE(0xa000, 0xa3ff) AM_RAM_WRITE(timeplt_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xa400, 0xa7ff) AM_RAM_WRITE(timeplt_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xa800, 0xafff) AM_RAM
	AM_RANGE(0xb000, 0xb0ff) AM_MIRROR(0x0b00) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xb400, 0xb4ff) AM_MIRROR(0x0b00) AM_RAM AM_SHARE("spriteram2")
	AM_RANGE(0xc000, 0xc000) AM_MIRROR(0x0cff) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xc200, 0xc200) AM_MIRROR(0x0cff) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xc302, 0xc302) AM_MIRROR(0x0cf1) AM_WRITE(timeplt_flipscreen_w)
	AM_RANGE(0xc304, 0xc304) AM_MIRROR(0x0cf1) AM_DEVWRITE("timeplt_audio", timeplt_audio_device, sh_irqtrigger_w)
	AM_RANGE(0xc30a, 0xc30c) AM_MIRROR(0x0cf1) AM_WRITE(timeplt_coin_counter_w)
	AM_RANGE(0xc000, 0xc000) AM_MIRROR(0x0cff) AM_READ(timeplt_scanline_r)
	AM_RANGE(0xc200, 0xc200) AM_MIRROR(0x0cff) AM_READ_PORT("DSW1")
	AM_RANGE(0xc300, 0xc300) AM_MIRROR(0x0c9f) AM_READ_PORT("IN0")
	AM_RANGE(0xc320, 0xc320) AM_MIRROR(0x0c9f) AM_READ_PORT("IN1")
	AM_RANGE(0xc340, 0xc340) AM_MIRROR(0x0c9f) AM_READ_PORT("IN2")
	AM_RANGE(0xc360, 0xc360) AM_MIRROR(0x0c9f) AM_READ_PORT("DSW0")
ADDRESS_MAP_END

static ADDRESS_MAP_START( chkun_main_map, AS_PROGRAM, 8, timeplt_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM
	AM_RANGE(0xa000, 0xa3ff) AM_RAM_WRITE(timeplt_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xa400, 0xa7ff) AM_RAM_WRITE(timeplt_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xa800, 0xafff) AM_RAM
	AM_RANGE(0xb000, 0xb0ff) AM_MIRROR(0x0b00) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xb400, 0xb4ff) AM_MIRROR(0x0b00) AM_RAM AM_SHARE("spriteram2")
	AM_RANGE(0xc000, 0xc000) AM_MIRROR(0x0cff) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xc200, 0xc200) AM_MIRROR(0x0cff) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xc300, 0xc300) AM_MIRROR(0x0cf1) AM_WRITE(timeplt_nmi_enable_w)
	AM_RANGE(0xc302, 0xc302) AM_MIRROR(0x0cf1) AM_WRITE(timeplt_flipscreen_w)
	AM_RANGE(0xc304, 0xc304) AM_MIRROR(0x0cf1) AM_DEVWRITE("timeplt_audio", timeplt_audio_device, sh_irqtrigger_w)
	AM_RANGE(0xc30a, 0xc30c) AM_MIRROR(0x0cf1) AM_WRITE(timeplt_coin_counter_w)
	AM_RANGE(0xc000, 0xc000) AM_MIRROR(0x0cff) AM_READ(timeplt_scanline_r)
	AM_RANGE(0xc200, 0xc200) AM_MIRROR(0x0cff) AM_READ_PORT("DSW1")
	AM_RANGE(0xc300, 0xc300) AM_MIRROR(0x0c9f) AM_READ_PORT("IN0")
	AM_RANGE(0xc320, 0xc320) AM_MIRROR(0x0c9f) AM_READ_PORT("IN1")
	AM_RANGE(0xc340, 0xc340) AM_MIRROR(0x0c9f) AM_READ_PORT("IN2")
	AM_RANGE(0xc360, 0xc360) AM_MIRROR(0x0c9f) AM_READ_PORT("DSW0")
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( timeplt )
	PORT_START("IN0")
	KONAMI8_SYSTEM_UNK

	PORT_START("IN1")
	KONAMI8_MONO_B1_UNK

	PORT_START("IN2")
	KONAMI8_COCKTAIL_B1_UNK

	PORT_START("DSW0")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), DEF_STR( Free_Play ), SW1)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "10000 50000" )
	PORT_DIPSETTING(    0x00, "20000 60000" )
	PORT_DIPNAME( 0x70, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,6,7")
	PORT_DIPSETTING(    0x70, "1 (Easiest)" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x50, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x10, "7" )
	PORT_DIPSETTING(    0x00, "8 (Difficult)" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( psurge )
	PORT_INCLUDE(timeplt)

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Initial Energy" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x08, "6" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x10, 0x10, "Infinite Shots (Cheat)")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Stop at Junctions" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( chkun )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Bet 3B")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, timeplt_state, chkun_hopper_status_r, NULL)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Bet 1B")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Bet 2B")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Bet HR")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0") // 12m
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_SERVICE_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" )            PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1") // 13m
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( bikkuric )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, timeplt_state, chkun_hopper_status_r, NULL)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0") // 12m
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSW1") // 13m
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ STEP4(0,1), STEP4(8*8,1) },
	{ STEP8(0,8) },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ STEP4(0,1), STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1) },
	{ STEP8(0,8), STEP8(32*8,8) },
	64*8
};


static GFXDECODE_START( timeplt )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,        0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,   32*4, 64 )
GFXDECODE_END

static const gfx_layout chkun_spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(0,1), STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1) },
	{ STEP8(0,8), STEP8(32*8,8) },
	64*8
};

static GFXDECODE_START( chkun )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,        0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, chkun_spritelayout,   32*4, 64 )
GFXDECODE_END

/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void timeplt_state::machine_start()
{
	save_item(NAME(m_nmi_enable));
}

void timeplt_state::machine_reset()
{
	m_nmi_enable = 0;
}

static MACHINE_CONFIG_START( timeplt, timeplt_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/3/2)  /* not confirmed, but common for Konami games of the era */
	MCFG_CPU_PROGRAM_MAP(timeplt_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", timeplt_state,  timeplt_interrupt)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_SCANLINE)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(timeplt_state, screen_update_timeplt)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", timeplt)
	MCFG_PALETTE_ADD("palette", 32*4+64*4)
	MCFG_PALETTE_INIT_OWNER(timeplt_state, timeplt)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(timeplt_sound)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( psurge, timeplt )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(psurge_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", timeplt_state,  nmi_line_pulse)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bikkuric, timeplt )

	MCFG_GFXDECODE_MODIFY("gfxdecode", chkun)

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(chkun_main_map)

	MCFG_VIDEO_START_OVERRIDE(timeplt_state,chkun)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( chkun, bikkuric )

	MCFG_GFXDECODE_MODIFY("gfxdecode", chkun)

	/* sound hardware */
	MCFG_SOUND_MODIFY("ay2")
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(timeplt_state, chkun_sound_w))

	MCFG_TC8830F_ADD("tc8830f", XTAL_512kHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( timeplt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tm1",          0x0000, 0x2000, CRC(1551f1b9) SHA1(c72f30988ac00cbe6549b71c3bcb414511e8b997) )
	ROM_LOAD( "tm2",          0x2000, 0x2000, CRC(58636cb5) SHA1(ab517efa93ae7be780af55faea82a6e83edd828c) )
	ROM_LOAD( "tm3",          0x4000, 0x2000, CRC(ff4e0d83) SHA1(ef98a1abb45b22d7498a0aca520f43bbee248b22) )

	ROM_REGION( 0x10000, "tpsound", 0 )
	ROM_LOAD( "tm7",          0x0000, 0x1000, CRC(d66da813) SHA1(408fca4515e8af84211df3e204c8776b2f8adb23) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tm6",          0x0000, 0x2000, CRC(c2507f40) SHA1(07221875e3f81d9def67c57a7ccd82d52ce65e01) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "tm4",          0x0000, 0x2000, CRC(7e437c3e) SHA1(cbe2ccd2cd503af62f009cd5aab73aa7366230b1) )
	ROM_LOAD( "tm5",          0x2000, 0x2000, CRC(e8ca87b9) SHA1(5dd30d3fb9fd8cf9e6a8e37e7ea858c7fd038a7e) )

	ROM_REGION( 0x0240, "proms", 0 )
	ROM_LOAD( "timeplt.b4",   0x0000, 0x0020, CRC(34c91839) SHA1(f62e279e21fce171231d3139be7adabe1f4b8c2e) ) /* palette */
	ROM_LOAD( "timeplt.b5",   0x0020, 0x0020, CRC(463b2b07) SHA1(9ad275365eba4869f94749f39ff8705d92056a10) ) /* palette */
	ROM_LOAD( "timeplt.e9",   0x0040, 0x0100, CRC(4bbb2150) SHA1(678433b21aae1daa938e32d3293eeed529a42ef9) ) /* sprite lookup table */
	ROM_LOAD( "timeplt.e12",  0x0140, 0x0100, CRC(f7b7663e) SHA1(151bd2dff4e4ef76d6438c1ab2cae71f987b9dad) ) /* char lookup table */
ROM_END

ROM_START( timeplta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cd_e1.bin",         0x0000, 0x2000, CRC(a4513b35) SHA1(1b1944ec5317d71af86e21e0691caae180dee7b5) )
	ROM_LOAD( "cd_e2.bin",         0x2000, 0x2000, CRC(38b0c72a) SHA1(8f0950deb2f9e2b65714318b9e837a1c837f52a9) )
	ROM_LOAD( "cd_e3.bin",         0x4000, 0x2000, CRC(83846870) SHA1(b1741e7e5674f9e63e113ead0cb7f5ef874eac5f) )

	ROM_REGION( 0x10000, "tpsound", 0 )
	ROM_LOAD( "tm7",          0x0000, 0x1000, CRC(d66da813) SHA1(408fca4515e8af84211df3e204c8776b2f8adb23) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tm6",          0x0000, 0x2000, CRC(c2507f40) SHA1(07221875e3f81d9def67c57a7ccd82d52ce65e01) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "tm4",          0x0000, 0x2000, CRC(7e437c3e) SHA1(cbe2ccd2cd503af62f009cd5aab73aa7366230b1) )
	ROM_LOAD( "tm5",          0x2000, 0x2000, CRC(e8ca87b9) SHA1(5dd30d3fb9fd8cf9e6a8e37e7ea858c7fd038a7e) )

	ROM_REGION( 0x0240, "proms", 0 )
	ROM_LOAD( "timeplt.b4",   0x0000, 0x0020, CRC(34c91839) SHA1(f62e279e21fce171231d3139be7adabe1f4b8c2e) ) /* palette */
	ROM_LOAD( "timeplt.b5",   0x0020, 0x0020, CRC(463b2b07) SHA1(9ad275365eba4869f94749f39ff8705d92056a10) ) /* palette */
	ROM_LOAD( "timeplt.e9",   0x0040, 0x0100, CRC(4bbb2150) SHA1(678433b21aae1daa938e32d3293eeed529a42ef9) ) /* sprite lookup table */
	ROM_LOAD( "timeplt.e12",  0x0140, 0x0100, CRC(f7b7663e) SHA1(151bd2dff4e4ef76d6438c1ab2cae71f987b9dad) ) /* char lookup table */
ROM_END

ROM_START( timepltc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cd1y",         0x0000, 0x2000, CRC(83ec72c2) SHA1(f3dbc8362f6bdad1baa65cf5d95611e79de381a4) )
	ROM_LOAD( "cd2y",         0x2000, 0x2000, CRC(0dcf5287) SHA1(c36628367e81ac07f5ace72b45ebb7140b6aa116) )
	ROM_LOAD( "cd3y",         0x4000, 0x2000, CRC(c789b912) SHA1(dead7b20a40769e48738fccc3a17e2266aac445d) )

	ROM_REGION( 0x10000, "tpsound", 0 )
	ROM_LOAD( "tm7",          0x0000, 0x1000, CRC(d66da813) SHA1(408fca4515e8af84211df3e204c8776b2f8adb23) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tm6",          0x0000, 0x2000, CRC(c2507f40) SHA1(07221875e3f81d9def67c57a7ccd82d52ce65e01) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "tm4",          0x0000, 0x2000, CRC(7e437c3e) SHA1(cbe2ccd2cd503af62f009cd5aab73aa7366230b1) )
	ROM_LOAD( "tm5",          0x2000, 0x2000, CRC(e8ca87b9) SHA1(5dd30d3fb9fd8cf9e6a8e37e7ea858c7fd038a7e) )

	ROM_REGION( 0x0240, "proms", 0 )
	ROM_LOAD( "timeplt.b4",   0x0000, 0x0020, CRC(34c91839) SHA1(f62e279e21fce171231d3139be7adabe1f4b8c2e) ) /* palette */
	ROM_LOAD( "timeplt.b5",   0x0020, 0x0020, CRC(463b2b07) SHA1(9ad275365eba4869f94749f39ff8705d92056a10) ) /* palette */
	ROM_LOAD( "timeplt.e9",   0x0040, 0x0100, CRC(4bbb2150) SHA1(678433b21aae1daa938e32d3293eeed529a42ef9) ) /* sprite lookup table */
	ROM_LOAD( "timeplt.e12",  0x0140, 0x0100, CRC(f7b7663e) SHA1(151bd2dff4e4ef76d6438c1ab2cae71f987b9dad) ) /* char lookup table */
ROM_END

ROM_START( spaceplt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sp1",          0x0000, 0x2000, CRC(ac8ca3ae) SHA1(9781138becd17aa70e877138e126ebb1fbff6192) )
	ROM_LOAD( "sp2",          0x2000, 0x2000, CRC(1f0308ef) SHA1(dd88378fc4cefe473f310d4730268c98354a4a44) )
	ROM_LOAD( "sp3",          0x4000, 0x2000, CRC(90aeca50) SHA1(9c6fddfeafa84f5284ec8f7c9d46216b110badc1) )

	ROM_REGION( 0x10000, "tpsound", 0 )
	ROM_LOAD( "tm7",          0x0000, 0x1000, CRC(d66da813) SHA1(408fca4515e8af84211df3e204c8776b2f8adb23) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sp6",          0x0000, 0x2000, CRC(76caa8af) SHA1(f81bb73877d415a6587a32bddaad6db8a8fd4941) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "sp4",          0x0000, 0x2000, CRC(3781ce7a) SHA1(68bb73f67494c3b24f7fd0d79153c9793f4b3a5b) )
	ROM_LOAD( "tm5",          0x2000, 0x2000, CRC(e8ca87b9) SHA1(5dd30d3fb9fd8cf9e6a8e37e7ea858c7fd038a7e) )

	ROM_REGION( 0x0240, "proms", 0 )
	ROM_LOAD( "timeplt.b4",   0x0000, 0x0020, CRC(34c91839) SHA1(f62e279e21fce171231d3139be7adabe1f4b8c2e) ) /* palette */
	ROM_LOAD( "timeplt.b5",   0x0020, 0x0020, CRC(463b2b07) SHA1(9ad275365eba4869f94749f39ff8705d92056a10) ) /* palette */
	ROM_LOAD( "timeplt.e9",   0x0040, 0x0100, CRC(4bbb2150) SHA1(678433b21aae1daa938e32d3293eeed529a42ef9) ) /* sprite lookup table */
	ROM_LOAD( "timeplt.e12",  0x0140, 0x0100, CRC(f7b7663e) SHA1(151bd2dff4e4ef76d6438c1ab2cae71f987b9dad) ) /* char lookup table */
ROM_END


ROM_START( psurge )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p1",           0x0000, 0x2000, CRC(05f9ba12) SHA1(ad88838d1a0c64830281e425d4ad2498ba959098) )
	ROM_LOAD( "p2",           0x2000, 0x2000, CRC(3ff41576) SHA1(9bdbad31c65dff76942967b5a334407b0326f752) )
	ROM_LOAD( "p3",           0x4000, 0x2000, CRC(e8fe120a) SHA1(b6320c9cb1a67097692aa0de7d88b0dfb63dedd7) )

	ROM_REGION( 0x10000, "tpsound", 0 )
	ROM_LOAD( "p6",           0x0000, 0x1000, CRC(b52d01fa) SHA1(9b6cf9ea51d3a87c174f34d42a4b1b5f38b48723) )
	ROM_LOAD( "p7",           0x1000, 0x1000, CRC(9db5c0ce) SHA1(b5bc1d89a7f7d7a0baae64390c37ee11f69a0e76) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "p4",           0x0000, 0x2000, CRC(26fd7f81) SHA1(eb282313a37d7d611bf90f9b0b527adee9ae283f) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "p5",           0x0000, 0x2000, CRC(6066ec8e) SHA1(7f1155cf8a2d63c0740a4b56f1e09e7dfc749302) )
	ROM_LOAD( "tm5",          0x2000, 0x2000, CRC(e8ca87b9) SHA1(5dd30d3fb9fd8cf9e6a8e37e7ea858c7fd038a7e) )

	ROM_REGION( 0x0240, "proms", 0 )
	ROM_LOAD( "timeplt.b4",   0x0000, 0x0020, BAD_DUMP CRC(34c91839) SHA1(f62e279e21fce171231d3139be7adabe1f4b8c2e)  ) /* palette */
	ROM_LOAD( "timeplt.b5",   0x0020, 0x0020, BAD_DUMP CRC(463b2b07) SHA1(9ad275365eba4869f94749f39ff8705d92056a10)  ) /* palette */
	ROM_LOAD( "timeplt.e9",   0x0040, 0x0100, BAD_DUMP CRC(4bbb2150) SHA1(678433b21aae1daa938e32d3293eeed529a42ef9)  ) /* sprite lookup table */
	ROM_LOAD( "timeplt.e12",  0x0140, 0x0100, BAD_DUMP CRC(f7b7663e) SHA1(151bd2dff4e4ef76d6438c1ab2cae71f987b9dad)  ) /* char lookup table */
ROM_END

ROM_START( chkun )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "n1.16a",   0x0000, 0x4000, CRC(c5879f9b) SHA1(68e3a87dfe6b3d1e0cdadd1ed8ad115a9d3055f9) )
	ROM_LOAD( "12.14a",   0x4000, 0x2000, CRC(80cc55da) SHA1(68727721479624cd0d38d895b98dcef4edac13e9) )

	ROM_REGION( 0x12000, "tpsound", 0 )
	ROM_LOAD( "15.3l",    0x0000, 0x2000, CRC(1f1463ca) SHA1(870abbca35236fcce6a2f640a238e20b9e57f10f))

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "13.4d",   0x0000, 0x4000, CRC(776427c0) SHA1(1e8387685f7e86aad31577f2186596b2a2dfc4de) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "14.8h",   0x0000, 0x4000, CRC(0cb76a48) SHA1(0beebbc3d30eb978f6fe7b15c0a7b0c2152815b7) )

	ROM_REGION( 0x20000, "tc8830f", 0 )
	ROM_LOAD( "v1.8k",   0x00000, 0x10000, CRC(d5ca802d) SHA1(0c2867c86132745063e36d03f41e6c3e150fd3ad) )
	ROM_LOAD( "v2.9k",   0x10000, 0x10000, CRC(70e902eb) SHA1(e1b2392446f2878f9e811a63bbbbdc56fd517a9c) )

	ROM_REGION( 0x0240, "proms", 0 )
	ROM_LOAD( "3.2j",        0x0000, 0x0020, CRC(34c91839) SHA1(f62e279e21fce171231d3139be7adabe1f4b8c2e) ) /* palette */
	ROM_LOAD( "2.1h",        0x0020, 0x0020, CRC(463b2b07) SHA1(9ad275365eba4869f94749f39ff8705d92056a10) ) /* palette */
	ROM_LOAD( "4.10h",       0x0040, 0x0100, CRC(4bbb2150) SHA1(678433b21aae1daa938e32d3293eeed529a42ef9) ) /* sprite lookup table */
	ROM_LOAD( "mb7114e.2b",  0x0140, 0x0100, CRC(adfa399a) SHA1(3b18971ddaae7734f0aaa0fcc82d4ddccc282959) ) /* char lookup table */

	ROM_REGION( 0x0200, "pld", 0 )
	ROM_LOAD( "a.16c",  0x0000, 0x00eb, CRC(e0d54999) SHA1(9e1c749873572ade2b925ce1519d31a6e19f841f) )
	ROM_LOAD( "b.9f",   0x0100, 0x00eb, CRC(e3857f83) SHA1(674e70dc960fc02a9fbda4a0ef0770eb8214c466) )
ROM_END

ROM_START( bikkuric )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.a16", 0x00000, 0x04000, CRC(e8d595ab) SHA1(01f6a5321274befcd03a0ec18ed9770aca4527b6) )
	ROM_LOAD( "2.a14", 0x04000, 0x02000, CRC(63fd7d53) SHA1(b1ef666453c5c9e344bee544a0673068d60158fa) )

	ROM_REGION( 0x10000, "tpsound", 0 )
	ROM_LOAD( "5.l3",  0x00000, 0x02000, CRC(bc438531) SHA1(e19badc417b0538010cf535d3f733acc54b0cd96) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "3.d4",  0x00000, 0x08000, CRC(74e8a64b) SHA1(b2542e1f6f4b54d8f7aec8f673cedcf5bff5e429) ) // 1st and 2nd identical, confirmed to be like this

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "4.h8",  0x00000, 0x02000, CRC(d303942d) SHA1(688d43e6dbe505d44fc41fdde74858a02910080d) )

	ROM_REGION( 0x0240, "proms", 0 ) // not dumped, roms taken from timeplt
	ROM_LOAD( "3.2j",  0x0000, 0x0020, BAD_DUMP CRC(34c91839) SHA1(f62e279e21fce171231d3139be7adabe1f4b8c2e) ) /* palette */
	ROM_LOAD( "2.1h",  0x0020, 0x0020, BAD_DUMP CRC(463b2b07) SHA1(9ad275365eba4869f94749f39ff8705d92056a10) ) /* palette */
	ROM_LOAD( "4.10h", 0x0040, 0x0100, BAD_DUMP CRC(4bbb2150) SHA1(678433b21aae1daa938e32d3293eeed529a42ef9) ) /* sprite lookup table */
	ROM_LOAD( "1.2b",  0x0140, 0x0100, BAD_DUMP CRC(f7b7663e) SHA1(151bd2dff4e4ef76d6438c1ab2cae71f987b9dad) ) /* char lookup table */

	ROM_REGION( 0x0200, "pld", 0 )
	ROM_LOAD( "a.16c",  0x0000, 0x00eb, NO_DUMP )
	ROM_LOAD( "b.9f",   0x0100, 0x00eb, NO_DUMP )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1982, timeplt,  0,       timeplt, timeplt, driver_device, 0, ROT90,  "Konami", "Time Pilot", MACHINE_SUPPORTS_SAVE )
GAME( 1982, timepltc, timeplt, timeplt, timeplt, driver_device, 0, ROT90,  "Konami (Centuri license)", "Time Pilot (Centuri)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, timeplta, timeplt, timeplt, timeplt, driver_device, 0, ROT90,  "Konami (Atari license)", "Time Pilot (Atari)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, spaceplt, timeplt, timeplt, timeplt, driver_device, 0, ROT90,  "bootleg", "Space Pilot", MACHINE_SUPPORTS_SAVE )

GAME( 1988, psurge,   0,       psurge,  psurge, driver_device,  0, ROT270, "Vision Electronics", "Power Surge", MACHINE_SUPPORTS_SAVE )

GAME( 1988, chkun,    0,       chkun,   chkun, driver_device,   0, ROT90,  "Peni", "Chance Kun (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1987, bikkuric, 0,       bikkuric,bikkuric, driver_device,0, ROT90,  "Peni", "Bikkuri Card (Japan)", MACHINE_SUPPORTS_SAVE )
