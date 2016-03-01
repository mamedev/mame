// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
/***************************************************************************

Vastar memory map (preliminary)

driver by Allard van der Bas

CPU #1:

0000-7fff ROM
8000-83ff bg #1 attribute RAM
8800-8bff bg #1 video RAM
8c00-8fff bg #1 color RAM
9000-93ff bg #2 attribute RAM
9800-9bff bg #2 video RAM
9c00-9fff bg #2 color RAM
a000-a3ff used only during startup - it's NOT a part of the RAM test
c400-c7ff fg color RAM
c800-cbff fg attribute RAM
cc00-cfff fg video RAM
f000-f7ff RAM (f000-f0ff is shared with CPU #2)

read:
e000      ???

write:
c410-c41f sprites
c430-c43f sprites
c7c0-c7df bg #2 scroll
c7e0-c7ff bg #1 scroll
c810-c81f sprites
c830-c83f sprites
cc10-cc1f sprites
cc30-cc3f sprites
e000      ???

I/O:
read:

write:
02        0 = hold CPU #2?

CPU #2:

0000-1fff ROM
4000-43ff RAM (shared with CPU #1)

read:
8000      IN1
8040      IN0
8080      IN2

write:

I/O:
read:
02        8910 read (port A = DSW0 port B = DSW1)

write:
00        8910 control
01        8910 write

***************************************************************************/

/*

Planet Probe - TRY Corporation? - Copyright 1985
Game probably programmed by same people behind some Kyugo/Orca/Komax/Crux games (see hiscore table, Gyrodine pinout, clocks etc.)

Upper board marked DVL/A-V
Bottom Bord DVL/B-V

The PCB seems to be a bootleg/prototype:
On the upper board there are some pads for jumpers , some empty spaces left unpopulated for additional TTLs and an XTAL.
All 5 sockets for 2732 eproms were modified to accept 2764 eproms.
The AY8910 pin 26 (TEST 2) is grounded with a flying wire
Lack of manufacturer on title/copyright screen (there is room in the ROM - 15 characters - after the year string for a copyright message,
  however it is filled with the 'string termination' character rather than a string.)

Upper board chips:
5x 2764 eproms
1x 2128 static ram (2k ram)
2x z80B
1x AY8910
2x 8 positions dipswitches

Bottom Board chips:
5x 2764 eproms
2x 2128 static ram (2kx8 ram)
4x 93422 DRAM (256x4 dram)
1x 6301 PROM (probably used for background ?)
3x 82s129 Colour PROMS (connected to resistors)

Clocks measured:

Main XTAL 18.432mhz
2x z80 : 18.432 / 6
AY8910 : 18.432 / 12
Vsync : 60.58hz

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/vastar.h"


void vastar_state::machine_start()
{
	save_item(NAME(m_nmi_mask));
}

void vastar_state::machine_reset()
{
	/* we must start with the second CPU halted */
	m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_nmi_mask = 0;
	m_sprite_priority[0] = 0;

	m_spriteram1 = m_fgvideoram + 0x000;
	m_bg1_scroll = m_fgvideoram + 0x3c0;
	m_bg2_scroll = m_fgvideoram + 0x3e0;
	m_spriteram2 = m_fgvideoram + 0x400;
	m_spriteram3 = m_fgvideoram + 0x800;
}

WRITE8_MEMBER(vastar_state::hold_cpu2_w)
{
	/* I'm not sure that this works exactly like this */
	m_subcpu->set_input_line(INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
}

WRITE8_MEMBER(vastar_state::flip_screen_w)
{
	flip_screen_set(data);
}

WRITE8_MEMBER(vastar_state::nmi_mask_w)
{
	m_nmi_mask = data & 1;
}


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, vastar_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM_WRITE(bg2videoram_w) AM_SHARE("bg2videoram") AM_MIRROR(0x2000)
	AM_RANGE(0x9000, 0x9fff) AM_RAM_WRITE(bg1videoram_w) AM_SHARE("bg1videoram") AM_MIRROR(0x2000)
	AM_RANGE(0xc000, 0xc000) AM_WRITEONLY AM_SHARE("sprite_priority")   /* sprite/BG priority */
	AM_RANGE(0xc400, 0xcfff) AM_RAM_WRITE(fgvideoram_w) AM_SHARE("fgvideoram") // fg videoram + sprites
	AM_RANGE(0xe000, 0xe000) AM_READWRITE(watchdog_reset_r, watchdog_reset_w)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("sharedram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_port_map, AS_IO, 8, vastar_state )
	ADDRESS_MAP_GLOBAL_MASK(0x0f)
	AM_RANGE(0x00, 0x00) AM_WRITE(nmi_mask_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(flip_screen_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(hold_cpu2_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( cpu2_map, AS_PROGRAM, 8, vastar_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_SHARE("sharedram")
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("P2")
	AM_RANGE(0x8040, 0x8040) AM_READ_PORT("P1")
	AM_RANGE(0x8080, 0x8080) AM_READ_PORT("SYSTEM")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu2_port_map, AS_IO, 8, vastar_state )
	ADDRESS_MAP_GLOBAL_MASK(0x0f)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0x02, 0x02) AM_DEVREAD("aysnd", ay8910_device, data_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( vastar )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Show Author Credits" )   PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Slow Motion (Cheat)")    PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, "20000 50000" )
	PORT_DIPSETTING(    0x00, "40000 70000" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("DSW2:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( vastar4 )
	PORT_INCLUDE(vastar)
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START( pprobe )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Player Controls Demo (Cheat)" )      PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Invulnerability (Cheat)" )   PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW1:5") // unused?
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, "20000 then every 40000" )
	PORT_DIPSETTING(    0x00, "30000 then every 70000" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Rom Test / STOP" )       PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("DSW2:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("DSW2:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};

static const gfx_layout spritelayoutdw =
{
	16,32,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8,
			64*8, 65*8, 66*8, 67*8, 68*8, 69*8, 70*8, 71*8,
			96*8, 97*8, 98*8, 99*8, 100*8, 101*8, 102*8, 103*8 },
	128*8
};

static GFXDECODE_START( vastar )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,   0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayoutdw, 0, 64 )
	GFXDECODE_ENTRY( "gfx3", 0, charlayout,     0, 64 )
	GFXDECODE_ENTRY( "gfx4", 0, charlayout,     0, 64 )
GFXDECODE_END


INTERRUPT_GEN_MEMBER(vastar_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( vastar, vastar_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_18_432MHz/6)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_port_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vastar_state,  vblank_irq)

	MCFG_CPU_ADD("sub", Z80, XTAL_18_432MHz/6)
	MCFG_CPU_PROGRAM_MAP(cpu2_map)
	MCFG_CPU_IO_MAP(cpu2_port_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(vastar_state, irq0_line_hold, 242) /* 4 * vsync_freq(60.58) measured, it is not known yet how long it is asserted so we'll use HOLD_LINE for now */

	MCFG_QUANTUM_TIME(attotime::from_hz(600))   /* 10 CPU slices per frame - seems enough to ensure proper synchronization of the CPUs */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(vastar_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", vastar)
	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_18_432MHz/12)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( vastar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e_f4.rom",     0x0000, 0x1000, CRC(45fa5075) SHA1(99c3d7414f3bc3a84430067a71dd00d260bbcdab) )
	ROM_LOAD( "e_k4.rom",     0x1000, 0x1000, CRC(84531982) SHA1(bf2fd92d821734f64ad72e13f4e1aae8e055aa43) )
	ROM_LOAD( "e_h4.rom",     0x2000, 0x1000, CRC(94a4f778) SHA1(d52b3d6ed4953cff6dde1884dec9f9cc94847cb2) )
	ROM_LOAD( "e_l4.rom",     0x3000, 0x1000, CRC(40e4d57b) SHA1(3f073574f430791518283314ce325e48d8daa246) )
	ROM_LOAD( "e_j4.rom",     0x4000, 0x1000, CRC(bd607651) SHA1(23d3c7d2a0c17a780286a01a93e480aafcdb4b05) )
	ROM_LOAD( "e_n4.rom",     0x5000, 0x1000, CRC(7a3779a4) SHA1(98e7092ed4eaec1ab129a7bede6ea1cf16e329f0) )
	ROM_LOAD( "e_n7.rom",     0x6000, 0x1000, CRC(31b6be39) SHA1(be0d03db9c6c8982b2f38ad534a6e213bbde1802) )
	ROM_LOAD( "e_n5.rom",     0x7000, 0x1000, CRC(f63f0e78) SHA1(a029e340b11b358dbe0dcf2d1a0e6c6c093bbc9d) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "e_f2.rom",     0x0000, 0x1000, CRC(713478d8) SHA1(9cbd1fb689d93a8964f48e59d4effaa4878b2945) )
	ROM_LOAD( "e_j2.rom",     0x1000, 0x1000, CRC(e4535442) SHA1(280d93bec5cf6183250827ce70ed5ddff968bba5) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "c_c9.rom",     0x0000, 0x2000, CRC(34f067b6) SHA1(45d7f8be5bd1dc9e5e511aa2e99c216c5ff12273) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "c_f7.rom",     0x0000, 0x2000, CRC(edbf3b13) SHA1(9d6ddf16e83c68c831fec28607584471b5cbcbd2) )
	ROM_LOAD( "c_f9.rom",     0x2000, 0x2000, CRC(8f309e22) SHA1(f5bbc5cf70687415061a0674e273e20fbfcc1f8f) )

	ROM_REGION( 0x2000, "gfx3", 0 )
	ROM_LOAD( "c_n4.rom",     0x0000, 0x2000, CRC(b5f9c866) SHA1(17fc38cd40638e4f5d25c0cae70df3b8f03425dd) )

	ROM_REGION( 0x2000, "gfx4", 0 )
	ROM_LOAD( "c_s4.rom",     0x0000, 0x2000, CRC(c9fbbfc9) SHA1(7c6ace0e2eae8420a31d9054ad5dd94924273d5f) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "tbp24s10.6p",  0x0000, 0x0100, CRC(a712d73a) SHA1(a65fa5928431d8631fb04e01ad0a0d2de849bf1d) )    /* red component */
	ROM_LOAD( "tbp24s10.6s",  0x0100, 0x0100, CRC(0a7d48ec) SHA1(400e0b271c241712e7b7502e96e4f8a609e078e1) )    /* green component */
	ROM_LOAD( "tbp24s10.6m",  0x0200, 0x0100, CRC(4c3db907) SHA1(03bcbc4763dcf49f4a06f499042e36183aa8b762) )    /* blue component */

	ROM_REGION( 0x0100, "unkprom", 0 )
	ROM_LOAD( "tbp24s10.8n",  0x0000, 0x0100, CRC(b5297a3b) SHA1(a5a512f86097b7d892f6d11e8492e8a379c07f60) )    /* ???? */
ROM_END

ROM_START( vastar2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.4f",         0x0000, 0x1000, CRC(6741ff9c) SHA1(d83e8233626845962b4cf9302d4aa75915017f36) )
	ROM_LOAD( "6.4k",         0x1000, 0x1000, CRC(5027619b) SHA1(5fa1d53f6ee125048d4ef3bc3bff5655648c5bd6) )
	ROM_LOAD( "4.4h",         0x2000, 0x1000, CRC(fdaa44e6) SHA1(7e4dbd924d001d1d3ffb86dd0e88d363ef32fa5f) )
	ROM_LOAD( "7.4l",         0x3000, 0x1000, CRC(29bef91c) SHA1(bc8eacac39c73b92ee84ea20c32e6987c4dd450b) )
	ROM_LOAD( "5.4j",         0x4000, 0x1000, CRC(c17c2458) SHA1(585022ca6df8568d0bf6fc4dc2e77909b3c8ab54) )
	ROM_LOAD( "8.4n",         0x5000, 0x1000, CRC(8ca25c37) SHA1(c8307a8453c426075927a4a8a20edd48c6c74f05) )
	ROM_LOAD( "10.6n",        0x6000, 0x1000, CRC(80df74ba) SHA1(5cbc75fb96ad6d63186ec42a5e9af6aae209d78f) )
	ROM_LOAD( "9.5n",         0x7000, 0x1000, CRC(239ec84e) SHA1(8b516c63d858d5c4acc3701a9abf9c3d53ddf7ff) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "e_f2.rom",     0x0000, 0x1000, CRC(713478d8) SHA1(9cbd1fb689d93a8964f48e59d4effaa4878b2945) )
	ROM_LOAD( "e_j2.rom",     0x1000, 0x1000, CRC(e4535442) SHA1(280d93bec5cf6183250827ce70ed5ddff968bba5) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "c_c9.rom",     0x0000, 0x2000, CRC(34f067b6) SHA1(45d7f8be5bd1dc9e5e511aa2e99c216c5ff12273) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "c_f7.rom",     0x0000, 0x2000, CRC(edbf3b13) SHA1(9d6ddf16e83c68c831fec28607584471b5cbcbd2) )
	ROM_LOAD( "c_f9.rom",     0x2000, 0x2000, CRC(8f309e22) SHA1(f5bbc5cf70687415061a0674e273e20fbfcc1f8f) )

	ROM_REGION( 0x2000, "gfx3", 0 )
	ROM_LOAD( "c_n4.rom",     0x0000, 0x2000, CRC(b5f9c866) SHA1(17fc38cd40638e4f5d25c0cae70df3b8f03425dd) )

	ROM_REGION( 0x2000, "gfx4", 0 )
	ROM_LOAD( "c_s4.rom",     0x0000, 0x2000, CRC(c9fbbfc9) SHA1(7c6ace0e2eae8420a31d9054ad5dd94924273d5f) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "tbp24s10.6p",  0x0000, 0x0100, CRC(a712d73a) SHA1(a65fa5928431d8631fb04e01ad0a0d2de849bf1d) )    /* red component */
	ROM_LOAD( "tbp24s10.6s",  0x0100, 0x0100, CRC(0a7d48ec) SHA1(400e0b271c241712e7b7502e96e4f8a609e078e1) )    /* green component */
	ROM_LOAD( "tbp24s10.6m",  0x0200, 0x0100, CRC(4c3db907) SHA1(03bcbc4763dcf49f4a06f499042e36183aa8b762) )    /* blue component */

	ROM_REGION( 0x0100, "unkprom", 0 )
	ROM_LOAD( "tbp24s10.8n",  0x0000, 0x0100, CRC(b5297a3b) SHA1(a5a512f86097b7d892f6d11e8492e8a379c07f60) )    /* ???? */
ROM_END

ROM_START( vastar3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vst_2.4f",     0x0000, 0x2000, CRC(ad4e512a) SHA1(aee11703fb386067dea809b09466719a9675448e) )
	ROM_LOAD( "vst_3.4h",     0x2000, 0x2000, CRC(2276c5d0) SHA1(1070a952c4e8a8d97036511b48656602ce8e6848) )
	ROM_LOAD( "vst_4.4j",     0x4000, 0x2000, CRC(deca2aa1) SHA1(88920ae4c4094a748d3f3c37093186e05f1ed284) )
	ROM_LOAD( "vst_5.6n",     0x6000, 0x2000, CRC(743ed1c7) SHA1(34b2e952113c6c2137dc0c8916276ae344a7c9df) )
	/* same roms but split??
	ROM_LOAD( "e_f4.rom",     0x0000, 0x1000, CRC(fecb46d6) SHA1(2d03af431f44ff13f535e1659c1cb15cd99da4a8) )
	ROM_LOAD( "e_k4.rom",     0x1000, 0x1000, CRC(cd45a64d) SHA1(dd08f12df013c36218a827b6423acd33b7aa6cbf) )
	ROM_LOAD( "e_h4.rom",     0x2000, 0x1000, CRC(9b0aee71) SHA1(0439706e5f7029dea316a497fb2a0c60a358c9f5) )
	ROM_LOAD( "e_l4.rom",     0x3000, 0x1000, CRC(d0a79879) SHA1(ec22a9f1de1f536d2eeda99a050a55b8b5078673) )
	ROM_LOAD( "e_j4.rom",     0x4000, 0x1000, CRC(d44c72ca) SHA1(6b4c0f29a9c12a64bfdbfd6ee2e5ca43bd94b01f) )
	ROM_LOAD( "e_n4.rom",     0x5000, 0x1000, CRC(03f68cad) SHA1(f2b09e9091d580cf1f90cd45968942fc6486a9b5) )
	ROM_LOAD( "e_n7.rom",     0x6000, 0x1000, CRC(1e88270d) SHA1(2ef20d781ed87928553cb6142ac5acb2ea700474) )
	ROM_LOAD( "e_n5.rom",     0x7000, 0x1000, CRC(896af6c8) SHA1(b262ee3b161ec00541d629f02ece6f978beea0ac) )
	*/

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "vst_0.2f",     0x0000, 0x1000, CRC(713478d8) SHA1(9cbd1fb689d93a8964f48e59d4effaa4878b2945) )
	ROM_LOAD( "vst_1.2j",     0x1000, 0x1000, CRC(e4535442) SHA1(280d93bec5cf6183250827ce70ed5ddff968bba5) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "c_c9.rom",     0x0000, 0x2000, CRC(34f067b6) SHA1(45d7f8be5bd1dc9e5e511aa2e99c216c5ff12273) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "c_f7.rom",     0x0000, 0x2000, CRC(edbf3b13) SHA1(9d6ddf16e83c68c831fec28607584471b5cbcbd2) )
	ROM_LOAD( "c_f9.rom",     0x2000, 0x2000, CRC(8f309e22) SHA1(f5bbc5cf70687415061a0674e273e20fbfcc1f8f) )

	ROM_REGION( 0x2000, "gfx3", 0 )
	ROM_LOAD( "c_n4.rom",     0x0000, 0x2000, CRC(b5f9c866) SHA1(17fc38cd40638e4f5d25c0cae70df3b8f03425dd) )

	ROM_REGION( 0x2000, "gfx4", 0 )
	ROM_LOAD( "c_s4.rom",     0x0000, 0x2000, CRC(c9fbbfc9) SHA1(7c6ace0e2eae8420a31d9054ad5dd94924273d5f) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "tbp24s10.6p",  0x0000, 0x0100, CRC(a712d73a) SHA1(a65fa5928431d8631fb04e01ad0a0d2de849bf1d) )    /* red component */
	ROM_LOAD( "tbp24s10.6s",  0x0100, 0x0100, CRC(0a7d48ec) SHA1(400e0b271c241712e7b7502e96e4f8a609e078e1) )    /* green component */
	ROM_LOAD( "tbp24s10.6m",  0x0200, 0x0100, CRC(4c3db907) SHA1(03bcbc4763dcf49f4a06f499042e36183aa8b762) )    /* blue component */

	ROM_REGION( 0x0100, "unkprom", 0 )
	ROM_LOAD( "tbp24s10.8n",  0x0000, 0x0100, CRC(b5297a3b) SHA1(a5a512f86097b7d892f6d11e8492e8a379c07f60) )    /* ???? */
ROM_END

ROM_START( vastar4 ) /* minimal changes (2 bytes) from parent set */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.bin",        0x0000, 0x1000, CRC(d2b8f177) SHA1(c45941cc59873d9d2fc1ae0ce36bf76c9b8ed040) )
	ROM_LOAD( "e_k4.rom",     0x1000, 0x1000, CRC(84531982) SHA1(bf2fd92d821734f64ad72e13f4e1aae8e055aa43) )
	ROM_LOAD( "e_h4.rom",     0x2000, 0x1000, CRC(94a4f778) SHA1(d52b3d6ed4953cff6dde1884dec9f9cc94847cb2) )
	ROM_LOAD( "e_l4.rom",     0x3000, 0x1000, CRC(40e4d57b) SHA1(3f073574f430791518283314ce325e48d8daa246) )
	ROM_LOAD( "e_j4.rom",     0x4000, 0x1000, CRC(bd607651) SHA1(23d3c7d2a0c17a780286a01a93e480aafcdb4b05) )
	ROM_LOAD( "e_n4.rom",     0x5000, 0x1000, CRC(7a3779a4) SHA1(98e7092ed4eaec1ab129a7bede6ea1cf16e329f0) )
	ROM_LOAD( "e_n7.rom",     0x6000, 0x1000, CRC(31b6be39) SHA1(be0d03db9c6c8982b2f38ad534a6e213bbde1802) )
	ROM_LOAD( "e_n5.rom",     0x7000, 0x1000, CRC(f63f0e78) SHA1(a029e340b11b358dbe0dcf2d1a0e6c6c093bbc9d) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "e_f2.rom",     0x0000, 0x1000, CRC(713478d8) SHA1(9cbd1fb689d93a8964f48e59d4effaa4878b2945) )
	ROM_LOAD( "e_j2.rom",     0x1000, 0x1000, CRC(e4535442) SHA1(280d93bec5cf6183250827ce70ed5ddff968bba5) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "c_c9.rom",     0x0000, 0x2000, CRC(34f067b6) SHA1(45d7f8be5bd1dc9e5e511aa2e99c216c5ff12273) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "c_f7.rom",     0x0000, 0x2000, CRC(edbf3b13) SHA1(9d6ddf16e83c68c831fec28607584471b5cbcbd2) )
	ROM_LOAD( "c_f9.rom",     0x2000, 0x2000, CRC(8f309e22) SHA1(f5bbc5cf70687415061a0674e273e20fbfcc1f8f) )

	ROM_REGION( 0x2000, "gfx3", 0 )
	ROM_LOAD( "c_n4.rom",     0x0000, 0x2000, CRC(b5f9c866) SHA1(17fc38cd40638e4f5d25c0cae70df3b8f03425dd) )

	ROM_REGION( 0x2000, "gfx4", 0 )
	ROM_LOAD( "c_s4.rom",     0x0000, 0x2000, CRC(c9fbbfc9) SHA1(7c6ace0e2eae8420a31d9054ad5dd94924273d5f) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "tbp24s10.6p",  0x0000, 0x0100, CRC(a712d73a) SHA1(a65fa5928431d8631fb04e01ad0a0d2de849bf1d) )    /* red component */
	ROM_LOAD( "tbp24s10.6s",  0x0100, 0x0100, CRC(0a7d48ec) SHA1(400e0b271c241712e7b7502e96e4f8a609e078e1) )    /* green component */
	ROM_LOAD( "tbp24s10.6m",  0x0200, 0x0100, CRC(4c3db907) SHA1(03bcbc4763dcf49f4a06f499042e36183aa8b762) )    /* blue component */

	ROM_REGION( 0x0100, "unkprom", 0 )
	ROM_LOAD( "tbp24s10.8n",  0x0000, 0x0100, CRC(b5297a3b) SHA1(a5a512f86097b7d892f6d11e8492e8a379c07f60) )    /* ???? */
ROM_END


ROM_START( pprobe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pb2.bin",   0x0000, 0x2000, CRC(a88592aa) SHA1(98e8e6233b85e678718f532708d57ec946b9fd88) )
	ROM_LOAD( "pb3.bin",   0x2000, 0x2000, CRC(e4e20f74) SHA1(53b4d0499127cca149a3dd03af4f05de552cff57) )
	ROM_LOAD( "pb4.bin",   0x4000, 0x2000, CRC(4e40e3fe) SHA1(ccb3c5828508efc9f0df44bf3408e807d5ef58a0) )
	ROM_LOAD( "pb5.bin",   0x6000, 0x2000, CRC(b26ff0fd) SHA1(c64966ee91557f8982b9b7fd17306508228f1e15) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "pb1.bin",   0x0000, 0x2000, CRC(cd624df9) SHA1(0645ce8dc1b361904da4f6e7adc9b7de109b2d14) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pb9.bin",  0x0000, 0x2000, CRC(82294dd6) SHA1(24b8eac3d476d4a4d91dd169e26bd075b0d1bf45) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "pb8.bin",  0x0000, 0x2000, CRC(8d809e45) SHA1(70f99626acdceaadbe03de49bcf778266ddff893) )
	ROM_LOAD( "pb10.bin", 0x2000, 0x2000, CRC(895f9dd3) SHA1(919861482598aa35a9ad476da19f9efa30904cd4) )

	ROM_REGION( 0x2000, "gfx3", 0 )
	ROM_LOAD( "pb6.bin",  0x0000, 0x2000, CRC(ff309239) SHA1(4e52833fafd54d4502ad09091fbfb1a8a2ff8828) )

	ROM_REGION( 0x2000, "gfx4", 0 )
	ROM_LOAD( "pb7.bin",  0x0000, 0x2000, CRC(439978f7) SHA1(ba80dd919a9bb6f8c516d4eb794c02ae0f0dea00) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "n82s129.3",   0x0000, 0x0100, CRC(dfb6b97c) SHA1(e35eda4f3022e661b021b952c53054d96481fb49) )
	ROM_LOAD( "n82s129.1",   0x0100, 0x0100, CRC(3cc696a2) SHA1(0a1407c19c63ee0f02c3e8b95b0c199b9aec3ce5) )
	ROM_LOAD( "dm74s287.2",  0x0200, 0x0100, CRC(64fea033) SHA1(19bbb325f71cb17ea069958b3c246fa908f0008e) )

	ROM_REGION( 0x0100, "unkprom", 0 )
	ROM_LOAD( "mmi6301-1.bin",  0x0000, 0x0100, CRC(b5297a3b) SHA1(a5a512f86097b7d892f6d11e8492e8a379c07f60) )  /* ???? == vastar - tbp24s10.8n */
ROM_END


GAME( 1983, vastar,  0,      vastar, vastar, driver_device, 0, ROT90, "Orca (Sesame Japan license)", "Vastar (set 1)", MACHINE_SUPPORTS_SAVE ) // Sesame Japan was a brand of Fujikousan
GAME( 1983, vastar2, vastar, vastar, vastar, driver_device, 0, ROT90, "Orca (Sesame Japan license)", "Vastar (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, vastar3, vastar, vastar, vastar, driver_device, 0, ROT90, "Orca (Sesame Japan license)", "Vastar (set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, vastar4, vastar, vastar, vastar4,driver_device, 0, ROT90, "Orca (Sesame Japan license)", "Vastar (set 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, pprobe,  0,      vastar, pprobe, driver_device, 0, ROT90, "Crux / Kyugo?", "Planet Probe (prototype?)", MACHINE_SUPPORTS_SAVE ) // has no Copyright, probably because Crux didn't have a trading name at this point?
