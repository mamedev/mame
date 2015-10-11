// license:BSD-3-Clause
// copyright-holders:Luca Elia, Olivier Galibert
/***************************************************************************

TODO:
- Find out how layers are enabled\disabled
- dangar input ports - parent set requires F2 be held for Service Mode
- wrong title screen in ninjemak
- bit 3 of ninjemak_gfxbank_w, there currently is a kludge to clear text RAM
  but it should really copy stuff from the extra ROM.
- Likely missing MCU emulation/simulation for displaying text layer for Ninja Emaki.
  There is no text displayed when you enter Service Mode when there should be.
  Examine $3000+ in file loaded for "gfx5".


Galivan
(C) 1985 Nihon Bussan
driver by
Luca Elia (l.elia@tin.it)
Olivier Galibert


Ninja Emaki (US)
(c)1986 NihonBussan Co.,Ltd.
Youma Ninpou Chou (Japan)
(c)1986 NihonBussan Co.,Ltd.
Driver by
Takahiro Nogi (nogi@kt.rim.or.jp) 1999/12/17 -

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/3526intf.h"
#include "includes/galivan.h"


WRITE8_MEMBER(galivan_state::galivan_sound_command_w)
{
	soundlatch_byte_w(space,0,((data & 0x7f) << 1) | 1);
}

READ8_MEMBER(galivan_state::soundlatch_clear_r)
{
	soundlatch_clear_byte_w(space, 0, 0);
	return 0;
}

READ8_MEMBER(galivan_state::IO_port_c0_r)
{
	return (0x58); /* To Avoid Reset on Ufo Robot dangar */
}



static ADDRESS_MAP_START( galivan_map, AS_PROGRAM, 8, galivan_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM

	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("bank1")
	AM_RANGE(0xd800, 0xdfff) AM_WRITE(galivan_videoram_w) AM_SHARE("videoram")

	AM_RANGE(0xe000, 0xe0ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe100, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ninjemak_map, AS_PROGRAM, 8, galivan_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM

	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("bank1")
	AM_RANGE(0xd800, 0xdfff) AM_WRITE(galivan_videoram_w) AM_SHARE("videoram")

	AM_RANGE(0xe000, 0xe1ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe200, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, galivan_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P2")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW2")
	AM_RANGE(0x40, 0x40) AM_WRITE(galivan_gfxbank_w)
	AM_RANGE(0x41, 0x42) AM_WRITE(galivan_scrollx_w)
	AM_RANGE(0x43, 0x44) AM_WRITE(galivan_scrolly_w)
	AM_RANGE(0x45, 0x45) AM_WRITE(galivan_sound_command_w)
/*  AM_RANGE(0x46, 0x46) AM_WRITENOP */
/*  AM_RANGE(0x47, 0x47) AM_WRITENOP */
	AM_RANGE(0xc0, 0xc0) AM_READ(IO_port_c0_r) /* dangar needs to return 0x58 */
ADDRESS_MAP_END

WRITE8_MEMBER(galivan_state::blit_trigger_w)
{
	m_nb1414m4->exec((m_videoram[0] << 8) | (m_videoram[1] & 0xff),m_videoram,m_scrollx,m_scrolly,m_tx_tilemap);
}

static ADDRESS_MAP_START( ninjemak_io_map, AS_IO, 8, galivan_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x80) AM_READ_PORT("P1") AM_WRITE(ninjemak_gfxbank_w)
	AM_RANGE(0x81, 0x81) AM_READ_PORT("P2")
	AM_RANGE(0x82, 0x82) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x83, 0x83) AM_READ_PORT("SERVICE")
	AM_RANGE(0x84, 0x84) AM_READ_PORT("DSW1")
	AM_RANGE(0x85, 0x85) AM_READ_PORT("DSW2") AM_WRITE(galivan_sound_command_w)
	AM_RANGE(0x86, 0x86) AM_WRITE(blit_trigger_w)         // ??
//  AM_RANGE(0x87, 0x87) AM_WRITENOP         // ??
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, galivan_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, AS_IO, 8, galivan_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ymsnd", ym3526_device, write)
	AM_RANGE(0x02, 0x02) AM_DEVWRITE("dac1", dac_device, write_unsigned8)
	AM_RANGE(0x03, 0x03) AM_DEVWRITE("dac2", dac_device, write_unsigned8)
	AM_RANGE(0x04, 0x04) AM_READ(soundlatch_clear_r)
	AM_RANGE(0x06, 0x06) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END


/***************
   Dip Sitches
 ***************/

#define NIHON_JOYSTICK(_n_) \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(_n_) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(_n_) \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(_n_) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(_n_) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(_n_)


static INPUT_PORTS_START( galivan )
	PORT_START("P1")
	NIHON_JOYSTICK(1)

	PORT_START("P2")
	NIHON_JOYSTICK(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	/* This is how the Bonus Life are defined in Service Mode */
	/* However, to keep the way Bonus Life are defined in MAME, */
	/* below are the same values, but using the MAME way */
//  PORT_DIPNAME( 0x04, 0x04, "1st Bonus Life" )
//  PORT_DIPSETTING(    0x04, "20k" )
//  PORT_DIPSETTING(    0x00, "50k" )
//  PORT_DIPNAME( 0x08, 0x08, "2nd Bonus Life" )
//  PORT_DIPSETTING(    0x08, "every 60k" )
//  PORT_DIPSETTING(    0x00, "every 90k" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "20k and every 60k" )
	PORT_DIPSETTING(    0x08, "50k and every 60k" )
	PORT_DIPSETTING(    0x04, "20k and every 90k" )
	PORT_DIPSETTING(    0x00, "50k and every 90k" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, "Power Invulnerability (Cheat)")  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Life Invulnerability (Cheat)")   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7")
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8")
INPUT_PORTS_END

static INPUT_PORTS_START( dangar )
	PORT_INCLUDE( galivan )

	PORT_MODIFY("SYSTEM")
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )

	PORT_MODIFY("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7")
	PORT_DIPNAME( 0x80, 0x80, "Alternate Enemies")          PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	/* two switches to allow continue... both work */
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, "3 Times" )
	PORT_DIPSETTING(    0x40, "5 Times" )
	PORT_DIPSETTING(    0x00, "99 Times" )
INPUT_PORTS_END

/* different Lives values and last different the last two dips */
static INPUT_PORTS_START( dangar2 )
	PORT_INCLUDE( dangar )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x40, 0x40, "Complete Invulnerability (Cheat)")   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Base Ship Invulnerability (Cheat)")  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* the last two dip switches are different */
static INPUT_PORTS_START( dangarb )
	PORT_INCLUDE( dangar )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x40, 0x40, "Complete Invulnerability (Cheat)")   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Base Ship Invulnerability (Cheat)")  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ninjemak )
	PORT_INCLUDE( galivan )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, "3 Times" )
	PORT_DIPSETTING(    0x40, "5 Times" )
	PORT_DIPSETTING(    0x00, "99 Times" )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Other games have Service here */

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4,0,12,8,20,16,28,24,36,32,44,40,52,48,60,56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	16*16*4
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, RGN_FRAC(1,2)+1*4, RGN_FRAC(1,2)+0*4, 3*4, 2*4, RGN_FRAC(1,2)+3*4, RGN_FRAC(1,2)+2*4,
			5*4, 4*4, RGN_FRAC(1,2)+5*4, RGN_FRAC(1,2)+4*4, 7*4, 6*4, RGN_FRAC(1,2)+7*4, RGN_FRAC(1,2)+6*4},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	64*8
};

static GFXDECODE_START( galivan )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,            0,   8 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,         8*16,  16 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 8*16+16*16, 256 )
GFXDECODE_END



MACHINE_START_MEMBER(galivan_state,galivan)
{
	/* configure ROM banking */
	UINT8 *rombase = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 2, &rombase[0x10000], 0x2000);
	membank("bank1")->set_entry(0);

	/* register for saving */
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_write_layers));
	save_item(NAME(m_layers));
}

MACHINE_START_MEMBER(galivan_state,ninjemak)
{
	/* configure ROM banking */
	UINT8 *rombase = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 4, &rombase[0x10000], 0x2000);
	membank("bank1")->set_entry(0);

	/* register for saving */
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_ninjemak_dispdisable));
}

MACHINE_RESET_MEMBER(galivan_state,galivan)
{
	m_maincpu->reset();

//  m_layers = 0x60;
	m_layers = 0;
	m_write_layers = 0;
	m_galivan_scrollx[0] = m_galivan_scrollx[1] = 0;
	m_galivan_scrolly[0] = m_galivan_scrolly[1] = 0;
}

MACHINE_RESET_MEMBER(galivan_state,ninjemak)
{
	m_maincpu->reset();

	m_scrollx = 0;
	m_scrolly = 0;
	m_ninjemak_dispdisable = 0;
}

static MACHINE_CONFIG_START( galivan, galivan_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz/2)      /* 6 MHz? */
	MCFG_CPU_PROGRAM_MAP(galivan_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galivan_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2)      /* 4 MHz? */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(galivan_state, irq0_line_hold,  XTAL_8MHz/2/512)   // ?

	MCFG_MACHINE_START_OVERRIDE(galivan_state,galivan)
	MCFG_MACHINE_RESET_OVERRIDE(galivan_state,galivan)

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM8_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(galivan_state, screen_update_galivan)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram8_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", galivan)
	MCFG_PALETTE_ADD("palette", 8*16+16*16+256*16)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(galivan_state, galivan)

	MCFG_VIDEO_START_OVERRIDE(galivan_state,galivan)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3526, XTAL_8MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ninjemak, galivan_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz/2)      /* 6 MHz? */
	MCFG_CPU_PROGRAM_MAP(ninjemak_map)
	MCFG_CPU_IO_MAP(ninjemak_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galivan_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2)      /* 4 MHz? */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(galivan_state, irq0_line_hold,  XTAL_8MHz/2/512)   // ?

	MCFG_MACHINE_START_OVERRIDE(galivan_state,ninjemak)
	MCFG_MACHINE_RESET_OVERRIDE(galivan_state,ninjemak)

	MCFG_DEVICE_ADD("nb1414m4", NB1414M4, 0)

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM8_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(galivan_state, screen_update_ninjemak)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram8_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT270)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", galivan)
	MCFG_PALETTE_ADD("palette", 8*16+16*16+256*16)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(galivan_state, galivan)

	MCFG_VIDEO_START_OVERRIDE(galivan_state,ninjemak)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd", YM3526, XTAL_8MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED(youmab, ninjemak)

	MCFG_DEVICE_REMOVE("nb1414m4")
MACHINE_CONFIG_END
/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( galivan )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* main cpu code */
	ROM_LOAD( "1.1b",         0x00000, 0x8000, CRC(1e66b3f8) SHA1(f9d2ac8076aefd85ce6d2ed2d21941f1160767f5) )
	ROM_LOAD( "2.3b",         0x08000, 0x4000, CRC(a45964f1) SHA1(4c4554ff484fbf70a38e1d89d3ae4d2eb4e93ed8) )
	ROM_LOAD( "gv3.4b",       0x10000, 0x4000, CRC(82f0c5e6) SHA1(77dd3927c2161e4fce9e0adba81dc0c875d7e2f4) ) /* 2 banks at c000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu code */
	ROM_LOAD( "gv11.14b",     0x0000, 0x4000, CRC(05f1a0e3) SHA1(c0f579130d64123c889c77d8f2f474ebcc3ba649) )
	ROM_LOAD( "gv12.15b",     0x4000, 0x8000, CRC(5b7a0d6d) SHA1(0c15def9be8014aeb4e14b6967efe8f5abac51f2) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "gv4.13d",      0x00000, 0x4000, CRC(162490b4) SHA1(55592865f208bf1b8f49c8eedc22a3d91ca3578d) ) /* chars */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "gv7.14f",      0x00000, 0x8000, CRC(eaa1a0db) SHA1(ed3b125a7472c0c0a458b28df6476cb4c64b4aa3) ) /* tiles */
	ROM_LOAD( "gv8.15f",      0x08000, 0x8000, CRC(f174a41e) SHA1(38aa7aa3d6ba026478d30b5e404614a0cc7aed52) )
	ROM_LOAD( "gv9.17f",      0x10000, 0x8000, CRC(edc60f5d) SHA1(c743f4af0e0e2c60f59fd01ce0a153108e9f5414) )
	ROM_LOAD( "gv10.19f",     0x18000, 0x8000, CRC(41f27fca) SHA1(3674dbecc2eb1c837159a8dfbb0086088631b2a5) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "gv14.4f",      0x00000, 0x8000, CRC(03e2229f) SHA1(9dace9e04867d1140eb3c794bd4ae54ec3bb4a83) ) /* sprites */
	ROM_LOAD( "gv13.1f",      0x08000, 0x8000, CRC(bca9e66b) SHA1(d84840943748a7b9fd6e141be9971431f69ce1f9) )

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "gv6.19d",      0x0000, 0x4000, CRC(da38168b) SHA1(a12decd55fd1cf32fd192f13bd33d2f1f4129d2c) )
	ROM_LOAD( "gv5.17d",      0x4000, 0x4000, CRC(22492d2a) SHA1(c8d36949abc2fcc8f2b12276eb82b330a940bc38) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "mb7114e.9f",   0x0000, 0x0100, CRC(de782b3e) SHA1(c76da7d5cbd9170be93c9591e525646a4360203c) )    /* red */
	ROM_LOAD( "mb7114e.10f",  0x0100, 0x0100, CRC(0ae2a857) SHA1(cdf84c0c75d483a81013dbc050e7aa8c8503c74c) )    /* green */
	ROM_LOAD( "mb7114e.11f",  0x0200, 0x0100, CRC(7ba8b9d1) SHA1(5942b403eda046e2f2584062443472cbf559db5c) )    /* blue */
	ROM_LOAD( "mb7114e.2d",   0x0300, 0x0100, CRC(75466109) SHA1(6196d12ab7103f6ef991b826d8b93303a61d4c48) )    /* sprite lookup table */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "mb7114e.7f",   0x0000, 0x0100, CRC(06538736) SHA1(a2fb2ecb768686839f3087e691102e2dc2eb65b5) )    /* sprite palette bank */
ROM_END

ROM_START( galivan2 )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* main cpu code */
	ROM_LOAD( "gv1.1b",       0x00000, 0x8000, CRC(5e480bfc) SHA1(f444de27d3d8aff579cf196a25b7f0c906617172) )
	ROM_LOAD( "gv2.3b",       0x08000, 0x4000, CRC(0d1b3538) SHA1(aa1ee04ff3516e0121db0cf50cee849ba5058fd5) )
	ROM_LOAD( "gv3.4b",       0x10000, 0x4000, CRC(82f0c5e6) SHA1(77dd3927c2161e4fce9e0adba81dc0c875d7e2f4) ) /* 2 banks at c000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu code */
	ROM_LOAD( "gv11.14b",     0x0000, 0x4000, CRC(05f1a0e3) SHA1(c0f579130d64123c889c77d8f2f474ebcc3ba649) )
	ROM_LOAD( "gv12.15b",     0x4000, 0x8000, CRC(5b7a0d6d) SHA1(0c15def9be8014aeb4e14b6967efe8f5abac51f2) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "gv4.13d",      0x00000, 0x4000, CRC(162490b4) SHA1(55592865f208bf1b8f49c8eedc22a3d91ca3578d) ) /* chars */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "gv7.14f",      0x00000, 0x8000, CRC(eaa1a0db) SHA1(ed3b125a7472c0c0a458b28df6476cb4c64b4aa3) ) /* tiles */
	ROM_LOAD( "gv8.15f",      0x08000, 0x8000, CRC(f174a41e) SHA1(38aa7aa3d6ba026478d30b5e404614a0cc7aed52) )
	ROM_LOAD( "gv9.17f",      0x10000, 0x8000, CRC(edc60f5d) SHA1(c743f4af0e0e2c60f59fd01ce0a153108e9f5414) )
	ROM_LOAD( "gv10.19f",     0x18000, 0x8000, CRC(41f27fca) SHA1(3674dbecc2eb1c837159a8dfbb0086088631b2a5) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "gv14.4f",      0x00000, 0x8000, CRC(03e2229f) SHA1(9dace9e04867d1140eb3c794bd4ae54ec3bb4a83) ) /* sprites */
	ROM_LOAD( "gv13.1f",      0x08000, 0x8000, CRC(bca9e66b) SHA1(d84840943748a7b9fd6e141be9971431f69ce1f9) )

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "gv6.19d",      0x0000, 0x4000, CRC(da38168b) SHA1(a12decd55fd1cf32fd192f13bd33d2f1f4129d2c) )
	ROM_LOAD( "gv5.17d",      0x4000, 0x4000, CRC(22492d2a) SHA1(c8d36949abc2fcc8f2b12276eb82b330a940bc38) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "mb7114e.9f",   0x0000, 0x0100, CRC(de782b3e) SHA1(c76da7d5cbd9170be93c9591e525646a4360203c) )    /* red */
	ROM_LOAD( "mb7114e.10f",  0x0100, 0x0100, CRC(0ae2a857) SHA1(cdf84c0c75d483a81013dbc050e7aa8c8503c74c) )    /* green */
	ROM_LOAD( "mb7114e.11f",  0x0200, 0x0100, CRC(7ba8b9d1) SHA1(5942b403eda046e2f2584062443472cbf559db5c) )    /* blue */
	ROM_LOAD( "mb7114e.2d",   0x0300, 0x0100, CRC(75466109) SHA1(6196d12ab7103f6ef991b826d8b93303a61d4c48) )    /* sprite lookup table */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "mb7114e.7f",   0x0000, 0x0100, CRC(06538736) SHA1(a2fb2ecb768686839f3087e691102e2dc2eb65b5) )    /* sprite palette bank */
ROM_END

ROM_START( galivan3 )
	ROM_REGION( 0x14000, "maincpu", 0 )     /* main cpu code */
	ROM_LOAD( "e-1.1b",       0x00000, 0x8000, CRC(d8cc72b8) SHA1(73a46cd7dda3a912b14075b9b4ebc81a175a1461) )
	ROM_LOAD( "e-2.3b",       0x08000, 0x4000, CRC(9e5b3157) SHA1(1aa5f7f382468af815c929c63866bd39e7a9ac18) )
	ROM_LOAD( "gv3.4b",       0x10000, 0x4000, CRC(82f0c5e6) SHA1(77dd3927c2161e4fce9e0adba81dc0c875d7e2f4) ) /* 2 banks at c000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu code */
	ROM_LOAD( "gv11.14b",     0x0000, 0x4000, CRC(05f1a0e3) SHA1(c0f579130d64123c889c77d8f2f474ebcc3ba649) )
	ROM_LOAD( "gv12.15b",     0x4000, 0x8000, CRC(5b7a0d6d) SHA1(0c15def9be8014aeb4e14b6967efe8f5abac51f2) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "gv4.13d",      0x00000, 0x4000, CRC(162490b4) SHA1(55592865f208bf1b8f49c8eedc22a3d91ca3578d) ) /* chars */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "gv7.14f",      0x00000, 0x8000, CRC(eaa1a0db) SHA1(ed3b125a7472c0c0a458b28df6476cb4c64b4aa3) ) /* tiles */
	ROM_LOAD( "gv8.15f",      0x08000, 0x8000, CRC(f174a41e) SHA1(38aa7aa3d6ba026478d30b5e404614a0cc7aed52) )
	ROM_LOAD( "gv9.17f",      0x10000, 0x8000, CRC(edc60f5d) SHA1(c743f4af0e0e2c60f59fd01ce0a153108e9f5414) )
	ROM_LOAD( "gv10.19f",     0x18000, 0x8000, CRC(41f27fca) SHA1(3674dbecc2eb1c837159a8dfbb0086088631b2a5) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "gv14.4f",      0x00000, 0x8000, CRC(03e2229f) SHA1(9dace9e04867d1140eb3c794bd4ae54ec3bb4a83) ) /* sprites */
	ROM_LOAD( "gv13.1f",      0x08000, 0x8000, CRC(bca9e66b) SHA1(d84840943748a7b9fd6e141be9971431f69ce1f9) )

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "gv6.19d",      0x0000, 0x4000, CRC(da38168b) SHA1(a12decd55fd1cf32fd192f13bd33d2f1f4129d2c) )
	ROM_LOAD( "gv5.17d",      0x4000, 0x4000, CRC(22492d2a) SHA1(c8d36949abc2fcc8f2b12276eb82b330a940bc38) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "mb7114e.9f",   0x0000, 0x0100, CRC(de782b3e) SHA1(c76da7d5cbd9170be93c9591e525646a4360203c) )    /* red */
	ROM_LOAD( "mb7114e.10f",  0x0100, 0x0100, CRC(0ae2a857) SHA1(cdf84c0c75d483a81013dbc050e7aa8c8503c74c) )    /* green */
	ROM_LOAD( "mb7114e.11f",  0x0200, 0x0100, CRC(7ba8b9d1) SHA1(5942b403eda046e2f2584062443472cbf559db5c) )    /* blue */
	ROM_LOAD( "mb7114e.2d",   0x0300, 0x0100, CRC(75466109) SHA1(6196d12ab7103f6ef991b826d8b93303a61d4c48) )    /* sprite lookup table */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "mb7114e.7f",   0x0000, 0x0100, CRC(06538736) SHA1(a2fb2ecb768686839f3087e691102e2dc2eb65b5) )    /* sprite palette bank */
ROM_END

ROM_START( dangar )
	ROM_REGION( 0x14000, "maincpu", 0 )     /* main cpu code */
	ROM_LOAD( "dangar08.1b",  0x00000, 0x8000, CRC(e52638f2) SHA1(6dd3ccb4574a410abf1ac35b4f9518ee21ecac91) )
	ROM_LOAD( "dangar09.3b",  0x08000, 0x4000, CRC(809d280f) SHA1(931f811f1fe3c71ba82fc44f69ef461bdd9cd2d8) )
	ROM_LOAD( "dangar10.5b",  0x10000, 0x4000, CRC(99a3591b) SHA1(45011043ff5620524d79076542bd8c602fe90cf4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu code */
	ROM_LOAD( "dangar13.b14", 0x0000, 0x4000, CRC(3e041873) SHA1(8f9e1ec64509c8a7e9e45add9efc95f98f35fcfc) )
	ROM_LOAD( "dangar14.b15", 0x4000, 0x8000, CRC(488e3463) SHA1(73ff7ab061be54162f3a548f6bd9ef55b9dec5d9) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "dangar05.13d", 0x00000, 0x4000, CRC(40cb378a) SHA1(764596f6845fc0b787b653a87a1778a56ce4f3f8) )   /* chars */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "dangar01.14f", 0x00000, 0x8000, CRC(d59ed1f1) SHA1(e55314b5a078145ad7a5e95cb792b4fd32cfb05d) )  /* tiles */
	ROM_LOAD( "dangar02.15f", 0x08000, 0x8000, CRC(dfdb931c) SHA1(33563160239f221f24ca0cb652d14550e9941afe) )
	ROM_LOAD( "dangar03.17f", 0x10000, 0x8000, CRC(6954e8c3) SHA1(077bcbe9f80df011c9110d8cf6e08b53d035d1c8) )
	ROM_LOAD( "dangar04.19f", 0x18000, 0x8000, CRC(4af6a8bf) SHA1(d004b10b9b8559d1d6d26af35999df2857d87c53) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "dangarxx.f4",  0x00000, 0x8000, CRC(55711884) SHA1(2682ebc8d88d0d6c430b7df34ed362bc81047072) )  /* sprites */
	ROM_LOAD( "dangarxx.f1",  0x08000, 0x8000, CRC(8cf11419) SHA1(79e7a3046878724fde248100ad55a305a427cd46) )

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "dangar07.19d", 0x0000, 0x4000, CRC(6dba32cf) SHA1(e6433f291364202c1291b137d6ee1840ecf7d72d) )
	ROM_LOAD( "dangar06.17d", 0x4000, 0x4000, CRC(6c899071) SHA1(9a776aae897d57e66ebdbcf79f3c673da8b78b05) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "82s129.9f",    0x0000, 0x0100, CRC(b29f6a07) SHA1(17c82f439f314c212470bafd917b3f7e12462d16) )    /* red */
	ROM_LOAD( "82s129.10f",   0x0100, 0x0100, CRC(c6de5ecb) SHA1(d5b6cb784b5df16332c5e2b19b763c8858a0b6a7) )    /* green */
	ROM_LOAD( "82s129.11f",   0x0200, 0x0100, CRC(a5bbd6dc) SHA1(5587844900a24d833500d204f049c05493c4a25a) )    /* blue */
	ROM_LOAD( "82s129.2d",    0x0300, 0x0100, CRC(a4ac95a5) SHA1(3b31cd3fd6caedd89d1bedc606a978081fc5431f) )    /* sprite lookup table */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "82s129.7f",    0x0000, 0x0100, CRC(29bc6216) SHA1(1d7864ad06ad0cd5e3d1905fc6066bee1cd90995) )    /* sprite palette bank */
ROM_END

ROM_START( dangar2 )
	ROM_REGION( 0x14000, "maincpu", 0 )     /* main cpu code */
	ROM_LOAD( "dangar2.016",  0x00000, 0x8000, CRC(743fa2d4) SHA1(55539796967532b57279801374b2f0cf82cfe1ae) )
	ROM_LOAD( "dangar2.017",  0x08000, 0x4000, CRC(1cdc60a5) SHA1(65f776d14c9461f1a6939ad512eacf6a1a9da2c6) )
	ROM_LOAD( "dangar2.018",  0x10000, 0x4000, CRC(db7f6613) SHA1(c55d1f2fdb86e2b9fbdfad0b156d4d084677b750) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu code */
	ROM_LOAD( "dangar13.b14", 0x0000, 0x4000, CRC(3e041873) SHA1(8f9e1ec64509c8a7e9e45add9efc95f98f35fcfc) )
	ROM_LOAD( "dangar14.b15", 0x4000, 0x8000, CRC(488e3463) SHA1(73ff7ab061be54162f3a548f6bd9ef55b9dec5d9) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "dangar2.011",  0x00000, 0x4000, CRC(e804ffe1) SHA1(22f16c23b9a82f104dda24bc8fccc08f3f69cf97) )   /* chars */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "dangar01.14f", 0x00000, 0x8000, CRC(d59ed1f1) SHA1(e55314b5a078145ad7a5e95cb792b4fd32cfb05d) )  /* tiles */
	ROM_LOAD( "dangar02.15f", 0x08000, 0x8000, CRC(dfdb931c) SHA1(33563160239f221f24ca0cb652d14550e9941afe) )
	ROM_LOAD( "dangar03.17f", 0x10000, 0x8000, CRC(6954e8c3) SHA1(077bcbe9f80df011c9110d8cf6e08b53d035d1c8) )
	ROM_LOAD( "dangar04.19f", 0x18000, 0x8000, CRC(4af6a8bf) SHA1(d004b10b9b8559d1d6d26af35999df2857d87c53) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "dangarxx.f4",  0x00000, 0x8000, CRC(55711884) SHA1(2682ebc8d88d0d6c430b7df34ed362bc81047072) )  /* sprites */
	ROM_LOAD( "dangarxx.f1",  0x08000, 0x8000, CRC(8cf11419) SHA1(79e7a3046878724fde248100ad55a305a427cd46) )

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "dangar07.19d", 0x0000, 0x4000, CRC(6dba32cf) SHA1(e6433f291364202c1291b137d6ee1840ecf7d72d) )
	ROM_LOAD( "dangar06.17d", 0x4000, 0x4000, CRC(6c899071) SHA1(9a776aae897d57e66ebdbcf79f3c673da8b78b05) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "82s129.9f",    0x0000, 0x0100, CRC(b29f6a07) SHA1(17c82f439f314c212470bafd917b3f7e12462d16) )    /* red */
	ROM_LOAD( "82s129.10f",   0x0100, 0x0100, CRC(c6de5ecb) SHA1(d5b6cb784b5df16332c5e2b19b763c8858a0b6a7) )    /* green */
	ROM_LOAD( "82s129.11f",   0x0200, 0x0100, CRC(a5bbd6dc) SHA1(5587844900a24d833500d204f049c05493c4a25a) )    /* blue */
	ROM_LOAD( "82s129.2d",    0x0300, 0x0100, CRC(a4ac95a5) SHA1(3b31cd3fd6caedd89d1bedc606a978081fc5431f) )    /* sprite lookup table */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "82s129.7f",    0x0000, 0x0100, CRC(29bc6216) SHA1(1d7864ad06ad0cd5e3d1905fc6066bee1cd90995) )    /* sprite palette bank */
ROM_END

ROM_START( dangarb )
	ROM_REGION( 0x14000, "maincpu", 0 )     /* main cpu code */
	ROM_LOAD( "8",            0x00000, 0x8000, CRC(8136fd10) SHA1(5f2ca08fab0d9431af38ef66922fdb6bd9a132e2) )
	ROM_LOAD( "9",            0x08000, 0x4000, CRC(3ce5ec11) SHA1(bcc0df6167d0b84b9f260435c1999b9d3605fcd4) )
	ROM_LOAD( "dangar2.018",  0x10000, 0x4000, CRC(db7f6613) SHA1(c55d1f2fdb86e2b9fbdfad0b156d4d084677b750) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* sound cpu code */
	ROM_LOAD( "dangar13.b14", 0x0000, 0x4000, CRC(3e041873) SHA1(8f9e1ec64509c8a7e9e45add9efc95f98f35fcfc) )
	ROM_LOAD( "dangar14.b15", 0x4000, 0x8000, CRC(488e3463) SHA1(73ff7ab061be54162f3a548f6bd9ef55b9dec5d9) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "dangar2.011",  0x00000, 0x4000, CRC(e804ffe1) SHA1(22f16c23b9a82f104dda24bc8fccc08f3f69cf97) )   /* chars */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "dangar01.14f", 0x00000, 0x8000, CRC(d59ed1f1) SHA1(e55314b5a078145ad7a5e95cb792b4fd32cfb05d) )  /* tiles */
	ROM_LOAD( "dangar02.15f", 0x08000, 0x8000, CRC(dfdb931c) SHA1(33563160239f221f24ca0cb652d14550e9941afe) )
	ROM_LOAD( "dangar03.17f", 0x10000, 0x8000, CRC(6954e8c3) SHA1(077bcbe9f80df011c9110d8cf6e08b53d035d1c8) )
	ROM_LOAD( "dangar04.19f", 0x18000, 0x8000, CRC(4af6a8bf) SHA1(d004b10b9b8559d1d6d26af35999df2857d87c53) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "dangarxx.f4",  0x00000, 0x8000, CRC(55711884) SHA1(2682ebc8d88d0d6c430b7df34ed362bc81047072) )  /* sprites */
	ROM_LOAD( "dangarxx.f1",  0x08000, 0x8000, CRC(8cf11419) SHA1(79e7a3046878724fde248100ad55a305a427cd46) )

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "dangar07.19d", 0x0000, 0x4000, CRC(6dba32cf) SHA1(e6433f291364202c1291b137d6ee1840ecf7d72d) )
	ROM_LOAD( "dangar06.17d", 0x4000, 0x4000, CRC(6c899071) SHA1(9a776aae897d57e66ebdbcf79f3c673da8b78b05) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "82s129.9f",    0x0000, 0x0100, CRC(b29f6a07) SHA1(17c82f439f314c212470bafd917b3f7e12462d16) )    /* red */
	ROM_LOAD( "82s129.10f",   0x0100, 0x0100, CRC(c6de5ecb) SHA1(d5b6cb784b5df16332c5e2b19b763c8858a0b6a7) )    /* green */
	ROM_LOAD( "82s129.11f",   0x0200, 0x0100, CRC(a5bbd6dc) SHA1(5587844900a24d833500d204f049c05493c4a25a) )    /* blue */
	ROM_LOAD( "82s129.2d",    0x0300, 0x0100, CRC(a4ac95a5) SHA1(3b31cd3fd6caedd89d1bedc606a978081fc5431f) )    /* sprite lookup table */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "82s129.7f",    0x0000, 0x0100, CRC(29bc6216) SHA1(1d7864ad06ad0cd5e3d1905fc6066bee1cd90995) )    /* sprite palette bank */
ROM_END

ROM_START( ninjemak )
	ROM_REGION( 0x18000, "maincpu", 0 ) /* main cpu code */
	ROM_LOAD( "ninjemak.1",   0x00000, 0x8000, CRC(12b0a619) SHA1(7b42097be6423931256d5b7fdafb98bee1b42e64) )
	ROM_LOAD( "ninjemak.2",   0x08000, 0x4000, CRC(d5b505d1) SHA1(53935549754e8a71f0620630c2e59c21d52edcba) )
	ROM_LOAD( "ninjemak.3",   0x10000, 0x8000, CRC(68c92bf6) SHA1(90633622dab0e450a29230b600e0d60a42f407f4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu code */
	ROM_LOAD( "ninjemak.12",  0x0000, 0x4000, CRC(3d1cd329) SHA1(6abd8e0dbecddfd67c4d358b958c850136fd3c29) )
	ROM_LOAD( "ninjemak.13",  0x4000, 0x8000, CRC(ac3a0b81) SHA1(39f2c305706e313d5256c357a3c8b57bbe45d3d7) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "ninjemak.4",   0x00000, 0x8000, CRC(83702c37) SHA1(c063288cf74dee74005c6d0dea57e9ec3adebc83) )   /* chars */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "ninjemak.8",   0x00000, 0x8000, CRC(655f0a58) SHA1(8ffe73cec68d52c7b09651b546289613d6d4dde4) ) /* tiles */
	ROM_LOAD( "ninjemak.9",   0x08000, 0x8000, CRC(934e1703) SHA1(451f8d01d9035d91c969cdc3fb582a00007da7df) )
	ROM_LOAD( "ninjemak.10",  0x10000, 0x8000, CRC(955b5c45) SHA1(936bfe2599228dd0861bbcfe15152ac5e9b906d1) )
	ROM_LOAD( "ninjemak.11",  0x18000, 0x8000, CRC(bbd2e51c) SHA1(51bc266cf8161610204e5d98e56346b1d8d3c009) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "ninjemak.16",  0x00000, 0x8000, CRC(8df93fed) SHA1(ef37c78d4abbdbe9f427e3d9345f52464261116d) )  /* sprites */
	ROM_LOAD( "ninjemak.17",  0x08000, 0x8000, CRC(a3efd0fc) SHA1(69d40707b0570c2f1be6247f0209ba9e60a83ed0) )
	ROM_LOAD( "ninjemak.14",  0x10000, 0x8000, CRC(bff332d3) SHA1(d277ba18034b083eaafa969d90685563994416fa) )
	ROM_LOAD( "ninjemak.15",  0x18000, 0x8000, CRC(56430ed4) SHA1(68356a0f68404ef70d8dc17d5cbdf5e1f28badcf) )

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "ninjemak.7",   0x0000, 0x4000, CRC(80c20d36) SHA1(f20724754824030d62059388f3ea2224f5b7a60e) )
	ROM_LOAD( "ninjemak.6",   0x4000, 0x4000, CRC(1da7a651) SHA1(5307452058164a0bc39d144dd204627a9ead7543) )

	ROM_REGION( 0x4000, "nb1414m4", 0 )    /* data for mcu/blitter? */
	ROM_LOAD( "ninjemak.5",   0x0000, 0x4000, CRC(5f91dd30) SHA1(3513c0a2e4ca83f602cacad6af9c07fe9e4b16a1) )    /* text layer data */

	ROM_REGION( 0x0400, "proms", 0 )    /* Region 3 - color data */
	ROM_LOAD( "ninjemak.pr1", 0x0000, 0x0100, CRC(8a62d4e4) SHA1(99ca4da01ea1b5585f6e3ebf162c3f988ab317e5) )    /* red */
	ROM_LOAD( "ninjemak.pr2", 0x0100, 0x0100, CRC(2ccf976f) SHA1(b804ee761793697087fbe3372352f301a22feeab) )    /* green */
	ROM_LOAD( "ninjemak.pr3", 0x0200, 0x0100, CRC(16b2a7a4) SHA1(53c410b439c8a835447f15f2ab250b363b3f7888) )    /* blue */
	ROM_LOAD( "yncp-2d.bin",  0x0300, 0x0100, BAD_DUMP CRC(23bade78) SHA1(7e2de5eb08d888f97830807b6dbe85d09bb3b7f8)  )  /* sprite lookup table */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "yncp-7f.bin",  0x0000, 0x0100, BAD_DUMP CRC(262d0809) SHA1(a67281af02cef082023c0d7d57e3824aeef67450)  )  /* sprite palette bank */
ROM_END

ROM_START( youma )
	ROM_REGION( 0x18000, "maincpu", 0 ) /* main cpu code */
	ROM_LOAD( "ync-1.bin",    0x00000, 0x8000, CRC(0552adab) SHA1(183cf88d288875fbb2b60e2712e5a1671511351d) )
	ROM_LOAD( "ync-2.bin",    0x08000, 0x4000, CRC(f961e5e6) SHA1(cbf9d3a256937da9e17734f89652e049242910b8) )
	ROM_LOAD( "ync-3.bin",    0x10000, 0x8000, CRC(9ad50a5e) SHA1(2532b10e2468b1c74440fd8090489142e5fc240b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu code */
	ROM_LOAD( "ninjemak.12",  0x0000, 0x4000, CRC(3d1cd329) SHA1(6abd8e0dbecddfd67c4d358b958c850136fd3c29) )
	ROM_LOAD( "ninjemak.13",  0x4000, 0x8000, CRC(ac3a0b81) SHA1(39f2c305706e313d5256c357a3c8b57bbe45d3d7) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "ync-4.bin",    0x00000, 0x8000, CRC(a1954f44) SHA1(b10a22b51bd1a02c0d7b116b4d7390003c41decf) )   /* chars */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "ninjemak.8",   0x00000, 0x8000, CRC(655f0a58) SHA1(8ffe73cec68d52c7b09651b546289613d6d4dde4) ) /* tiles */
	ROM_LOAD( "ninjemak.9",   0x08000, 0x8000, CRC(934e1703) SHA1(451f8d01d9035d91c969cdc3fb582a00007da7df) )
	ROM_LOAD( "ninjemak.10",  0x10000, 0x8000, CRC(955b5c45) SHA1(936bfe2599228dd0861bbcfe15152ac5e9b906d1) )
	ROM_LOAD( "ninjemak.11",  0x18000, 0x8000, CRC(bbd2e51c) SHA1(51bc266cf8161610204e5d98e56346b1d8d3c009) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "ninjemak.16",  0x00000, 0x8000, CRC(8df93fed) SHA1(ef37c78d4abbdbe9f427e3d9345f52464261116d) )  /* sprites */
	ROM_LOAD( "ninjemak.17",  0x08000, 0x8000, CRC(a3efd0fc) SHA1(69d40707b0570c2f1be6247f0209ba9e60a83ed0) )
	ROM_LOAD( "ninjemak.14",  0x10000, 0x8000, CRC(bff332d3) SHA1(d277ba18034b083eaafa969d90685563994416fa) )
	ROM_LOAD( "ninjemak.15",  0x18000, 0x8000, CRC(56430ed4) SHA1(68356a0f68404ef70d8dc17d5cbdf5e1f28badcf) )

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "ninjemak.7",   0x0000, 0x4000, CRC(80c20d36) SHA1(f20724754824030d62059388f3ea2224f5b7a60e) )
	ROM_LOAD( "ninjemak.6",   0x4000, 0x4000, CRC(1da7a651) SHA1(5307452058164a0bc39d144dd204627a9ead7543) )

	ROM_REGION( 0x4000, "nb1414m4", 0 )    /* data for mcu/blitter? */
	ROM_LOAD( "ync-5.bin",    0x0000, 0x4000, CRC(993e4ab2) SHA1(aceafc83b36db4db923d27f77ad045e626678bae) )    /* text layer data */

	ROM_REGION( 0x0400, "proms", 0 )    /* Region 3 - color data */
	ROM_LOAD( "yncp-6e.bin",  0x0000, 0x0100, CRC(ea47b91a) SHA1(9921aa1ef882fb664d85d3e065223610262ca112) )    /* red */
	ROM_LOAD( "yncp-7e.bin",  0x0100, 0x0100, CRC(e94c0fed) SHA1(68581c91e9aa485f78af6b6a5c98612372cd5b17) )    /* green */
	ROM_LOAD( "yncp-8e.bin",  0x0200, 0x0100, CRC(ffb4b287) SHA1(c3c7018e6d5e18cc2db135812d0dc3824710ab4c) )    /* blue */
	ROM_LOAD( "yncp-2d.bin",  0x0300, 0x0100, CRC(23bade78) SHA1(7e2de5eb08d888f97830807b6dbe85d09bb3b7f8) )    /* sprite lookup table */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "yncp-7f.bin",  0x0000, 0x0100, CRC(262d0809) SHA1(a67281af02cef082023c0d7d57e3824aeef67450) )    /* sprite palette bank */
ROM_END

ROM_START( youma2 )
	ROM_REGION( 0x18000, "maincpu", 0 ) /* main cpu code */
	ROM_LOAD( "1.1d",    0x00000, 0x8000, CRC(171dbe99) SHA1(c9fdca3849e20ab702415984b4039cf2cfa34cb8) )    // x
	ROM_LOAD( "2.3d",    0x08000, 0x4000, CRC(e502d62a) SHA1(fdfb44c17557a513fe855b14140fe48921d6802b) )    // x
	ROM_LOAD( "3.4d",    0x10000, 0x8000, CRC(cb84745c) SHA1(a961c329be26c423212078d04d5f783c796136b4) )    // x

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu code */
	ROM_LOAD( "12.14b",  0x0000, 0x4000, CRC(3d1cd329) SHA1(6abd8e0dbecddfd67c4d358b958c850136fd3c29) )
	ROM_LOAD( "13.15b",  0x4000, 0x8000, CRC(ac3a0b81) SHA1(39f2c305706e313d5256c357a3c8b57bbe45d3d7) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "4.7d",    0x00000, 0x8000, CRC(40aeffd8) SHA1(f31e723323a0cdb8efa8b320f1c4efd646401ca4) )    /* chars x */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "ninjemak.8",   0x00000, 0x8000, CRC(655f0a58) SHA1(8ffe73cec68d52c7b09651b546289613d6d4dde4) ) /* tiles */
	ROM_LOAD( "ninjemak.9",   0x08000, 0x8000, CRC(934e1703) SHA1(451f8d01d9035d91c969cdc3fb582a00007da7df) )
	ROM_LOAD( "ninjemak.10",  0x10000, 0x8000, CRC(955b5c45) SHA1(936bfe2599228dd0861bbcfe15152ac5e9b906d1) )
	ROM_LOAD( "ninjemak.11",  0x18000, 0x8000, CRC(bbd2e51c) SHA1(51bc266cf8161610204e5d98e56346b1d8d3c009) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "ninjemak.16",  0x00000, 0x8000, CRC(8df93fed) SHA1(ef37c78d4abbdbe9f427e3d9345f52464261116d) )  /* sprites */
	ROM_LOAD( "ninjemak.17",  0x08000, 0x8000, CRC(a3efd0fc) SHA1(69d40707b0570c2f1be6247f0209ba9e60a83ed0) )
	ROM_LOAD( "ninjemak.14",  0x10000, 0x8000, CRC(bff332d3) SHA1(d277ba18034b083eaafa969d90685563994416fa) )
	ROM_LOAD( "ninjemak.15",  0x18000, 0x8000, CRC(56430ed4) SHA1(68356a0f68404ef70d8dc17d5cbdf5e1f28badcf) )

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "ninjemak.7",   0x0000, 0x4000, CRC(80c20d36) SHA1(f20724754824030d62059388f3ea2224f5b7a60e) )
	ROM_LOAD( "ninjemak.6",   0x4000, 0x4000, CRC(1da7a651) SHA1(5307452058164a0bc39d144dd204627a9ead7543) )

	ROM_REGION( 0x4000, "nb1414m4", 0 )    /* data for mcu/blitter? */
	ROM_LOAD( "5.15d",    0x0000, 0x4000, CRC(1b4f64aa) SHA1(2cb2db946bf93e0928d6aa2e2dd29acb92981567) )    /* text layer data x */

	ROM_REGION( 0x0400, "proms", 0 )    /* Region 3 - color data */
	ROM_LOAD( "bpr.6e",  0x0000, 0x0100, CRC(8a62d4e4) SHA1(99ca4da01ea1b5585f6e3ebf162c3f988ab317e5) ) /* red x */
	ROM_LOAD( "bpr.7e",  0x0100, 0x0100, CRC(2ccf976f) SHA1(b804ee761793697087fbe3372352f301a22feeab) ) /* green x */
	ROM_LOAD( "bpr.8e",  0x0200, 0x0100, CRC(16b2a7a4) SHA1(53c410b439c8a835447f15f2ab250b363b3f7888) ) /* blue x */
	ROM_LOAD( "bpr.2d",  0x0300, 0x0100, CRC(23bade78) SHA1(7e2de5eb08d888f97830807b6dbe85d09bb3b7f8) ) /* sprite lookup table */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "bpr.7f",  0x0000, 0x0100, CRC(262d0809) SHA1(a67281af02cef082023c0d7d57e3824aeef67450) ) /* sprite palette bank */
ROM_END

ROM_START( youmab )
	ROM_REGION( 0x18000, "maincpu", 0 ) /* main cpu code */
	ROM_LOAD( "electric1.3u", 0x00000, 0x8000, CRC(cc4fdb92) SHA1(9ce963db23f91f91e775a0b9a819f00db869120f) )
	ROM_LOAD( "electric3.3r", 0x10000, 0x8000, CRC(c1bc7387) SHA1(ad05bff02ece515465a9506e09c252c446c8f81d) )

	ROM_REGION( 0x10000, "user2", 0 )   /* main cpu code */
	/* This rom is double the size of the original one, appears to have extra (banked) code for 0x8000 */
	ROM_LOAD( "electric2.3t", 0x00000, 0x8000, CRC(99aee3bc) SHA1(5ffd60b959dda3fd41609c89a3486a989b1e2530) )


	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu code */
	ROM_LOAD( "electric12.5e",  0x0000, 0x4000, CRC(3d1cd329) SHA1(6abd8e0dbecddfd67c4d358b958c850136fd3c29) )
	ROM_LOAD( "electric13.5d",  0x4000, 0x8000, CRC(ac3a0b81) SHA1(39f2c305706e313d5256c357a3c8b57bbe45d3d7) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "electric4.3m",    0x00000, 0x8000, CRC(a1954f44) SHA1(b10a22b51bd1a02c0d7b116b4d7390003c41decf) )    /* chars */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "electric8.1f",  0x00000, 0x8000, CRC(655f0a58) SHA1(8ffe73cec68d52c7b09651b546289613d6d4dde4) ) /* tiles */
	ROM_LOAD( "electric9.1d",  0x08000, 0x8000, CRC(77a964c1) SHA1(47eb2d4df240e5493951b0a170cd07b2d5ecc18a) ) // different (bad?)
	ROM_LOAD( "electric10.1b", 0x10000, 0x8000, CRC(955b5c45) SHA1(936bfe2599228dd0861bbcfe15152ac5e9b906d1) )
	ROM_LOAD( "electric11.1a", 0x18000, 0x8000, CRC(bbd2e51c) SHA1(51bc266cf8161610204e5d98e56346b1d8d3c009) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "electric16.1p",  0x00000, 0x8000, CRC(8df93fed) SHA1(ef37c78d4abbdbe9f427e3d9345f52464261116d) )  /* sprites */
	ROM_LOAD( "electric17.1m",  0x08000, 0x8000, CRC(a3efd0fc) SHA1(69d40707b0570c2f1be6247f0209ba9e60a83ed0) )
	ROM_LOAD( "electric14.1t",  0x10000, 0x8000, CRC(bff332d3) SHA1(d277ba18034b083eaafa969d90685563994416fa) )
	ROM_LOAD( "electric15.1r",  0x18000, 0x8000, CRC(56430ed4) SHA1(68356a0f68404ef70d8dc17d5cbdf5e1f28badcf) )

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "electric7.3a",   0x0000, 0x4000, CRC(80c20d36) SHA1(f20724754824030d62059388f3ea2224f5b7a60e) )
	ROM_LOAD( "electric6.3b",   0x4000, 0x4000, CRC(1da7a651) SHA1(5307452058164a0bc39d144dd204627a9ead7543) )

	ROM_REGION( 0x0400, "proms", 0 )    /* Region 3 - color data */
	ROM_LOAD( "prom82s129.2n",  0x0000, 0x0100, CRC(ea47b91a) SHA1(9921aa1ef882fb664d85d3e065223610262ca112) )  /* red */
	ROM_LOAD( "prom82s129.2m",  0x0100, 0x0100, CRC(e94c0fed) SHA1(68581c91e9aa485f78af6b6a5c98612372cd5b17) )  /* green */
	ROM_LOAD( "prom82s129.2l",  0x0200, 0x0100, CRC(ffb4b287) SHA1(c3c7018e6d5e18cc2db135812d0dc3824710ab4c) )  /* blue */
	ROM_LOAD( "prom82s129.3s",  0x0300, 0x0100, CRC(23bade78) SHA1(7e2de5eb08d888f97830807b6dbe85d09bb3b7f8) )  /* sprite lookup table */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "prom82s129.1l",  0x0000, 0x0100, CRC(262d0809) SHA1(a67281af02cef082023c0d7d57e3824aeef67450) )  /* sprite palette bank */
ROM_END


/*
Youma Ninpou Chou (bootleg hardware)
Nichibutsu, 1986

Top PCB
-------
CPU  : Z80B
OSC  : 12.0000MHz
RAM  : TMM2015 (x1), HM6264 (x1)
DIPSW: 8 position (x2)
ROMs :
       1 - 4  near Z80B (type 27c256)
       5 - 6  on opposite side of PCB (type 27c128)
       7 - 10 adjacent to 5 & 6 (type 27c256)

PROMs: pr.6e \
       pr.7e  | Type 82s129, near ROMs 1 - 4
       pr.8e /

Bottom PCB
----------
CPU  : Z80A
OSC  : 21.400MHz, 8.000MHz
RAM  : HM6116 (x1, near Z80), UM6114 (=2148, x4)
SOUND: YM3526, Y3014, LM324 (x2)
ROMs :
       11 - 12 near Z80 (11 = type 27C128, 12 = 27c256)
       13 - 16 on opposite side of PCB (type 27c256)

PROMs: pr.7h \
       pr.2e /  type 82s129, near ROMs 13 - 16

ROMIDENT Reference:
-------------------
1.1D         [692ae497] NOT FOUND!
10.18F       [bbd2e51c] = NINJEMAK.11  from Ninja Emaki (Nichibutsu)
                        = YNC-11.BIN   from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)
11.13B       [3d1cd329] = NINJEMAK.12  from Ninja Emaki (Nichibutsu)
                        = YNC-12.BIN   from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)
12.15B       [ac3a0b81] = NINJEMAK.13  from Ninja Emaki (Nichibutsu)
                        = YNC-13.BIN   from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)
13.1H        [bff332d3] = NINJEMAK.14  from Ninja Emaki (Nichibutsu)
                        = YNC-14.BIN   from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)
14.3H        [56430ed4] = NINJEMAK.15  from Ninja Emaki (Nichibutsu)
                        = YNC-15.BIN   from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)
15.4H        [8df93fed] = NINJEMAK.16  from Ninja Emaki (Nichibutsu)
                        = YNC-16.BIN   from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)
16.6H        [a3efd0fc] = NINJEMAK.17  from Ninja Emaki (Nichibutsu)
                        = YNC-17.BIN   from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)
2.2D         [99aee3bc] NOT FOUND!
3.4D         [ebf61afc] NOT FOUND!
4.7D         [a1954f44] = YNC-4.BIN    from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)
5.17D        [1da7a651] = NINJEMAK.6   from Ninja Emaki (Nichibutsu)
                        = YNC-6.BIN    from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)
6.18D        [80c20d36] = NINJEMAK.7   from Ninja Emaki (Nichibutsu)
                        = YNC-7.BIN    from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)
7.13F        [655f0a58] = YNC-8.BIN    from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)
8.15F        [934e1703] = NINJEMAK.9   from Ninja Emaki (Nichibutsu)
                        = YNC-9.BIN    from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)
9.16F        [955b5c45] = NINJEMAK.10  from Ninja Emaki (Nichibutsu)
                        = YNC-10.BIN   from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)
PR.2E        [23bade78] = YNCP-2D.BIN  from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)
PR.6E        [ea47b91a] = YNCP-6E.BIN  from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)
PR.7E        [6d66da81] NOT FOUND!
PR.7H        [262d0809] = YNCP-7F.BIN  from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)
PR.8E        [ffb4b287] = YNCP-8E.BIN  from Youma Ninpou Chou (Nichibutsu, Ninja Emaki jpn)

*/

ROM_START( youmab2 )
	ROM_REGION( 0x18000, "maincpu", 0 ) /* main cpu code */
	ROM_LOAD( "1.1d",     0x00000, 0x8000, CRC(692ae497) SHA1(572e5a1eae9b0bb48f65dce5de2df5c5ae95a3bd) ) // sldh
	ROM_LOAD( "3.4d",     0x10000, 0x8000, CRC(ebf61afc) SHA1(30235a90e8316f5033d44d31f02cca97c64f2d5e) ) // sldh

	ROM_REGION( 0x10000, "user2", 0 )   /* main cpu code */
	/* This rom is double the size of the original one, appears to have extra (banked) code for 0x8000 */
	ROM_LOAD( "2.2d", 0x00000, 0x8000, CRC(99aee3bc) SHA1(5ffd60b959dda3fd41609c89a3486a989b1e2530) ) // same as first bootleg

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound cpu code */
	ROM_LOAD( "11.13b",  0x0000, 0x4000, CRC(3d1cd329) SHA1(6abd8e0dbecddfd67c4d358b958c850136fd3c29) )
	ROM_LOAD( "12.15b",  0x4000, 0x8000, CRC(ac3a0b81) SHA1(39f2c305706e313d5256c357a3c8b57bbe45d3d7) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "4.7d",     0x00000, 0x8000, CRC(a1954f44) SHA1(b10a22b51bd1a02c0d7b116b4d7390003c41decf) ) /* chars */ // sldh

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "7.13f",   0x00000, 0x8000, CRC(655f0a58) SHA1(8ffe73cec68d52c7b09651b546289613d6d4dde4) ) /* tiles */
	ROM_LOAD( "8.15f",   0x08000, 0x8000, CRC(934e1703) SHA1(451f8d01d9035d91c969cdc3fb582a00007da7df) )
	ROM_LOAD( "9.16f",   0x10000, 0x8000, CRC(955b5c45) SHA1(936bfe2599228dd0861bbcfe15152ac5e9b906d1) )
	ROM_LOAD( "10.18f",  0x18000, 0x8000, CRC(bbd2e51c) SHA1(51bc266cf8161610204e5d98e56346b1d8d3c009) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "15.4h",  0x00000, 0x8000, CRC(8df93fed) SHA1(ef37c78d4abbdbe9f427e3d9345f52464261116d) )  /* sprites */
	ROM_LOAD( "16.6h",  0x08000, 0x8000, CRC(a3efd0fc) SHA1(69d40707b0570c2f1be6247f0209ba9e60a83ed0) )
	ROM_LOAD( "13.1h",  0x10000, 0x8000, CRC(bff332d3) SHA1(d277ba18034b083eaafa969d90685563994416fa) )
	ROM_LOAD( "14.3h",  0x18000, 0x8000, CRC(56430ed4) SHA1(68356a0f68404ef70d8dc17d5cbdf5e1f28badcf) )

	ROM_REGION( 0x8000, "gfx4", 0 ) /* background tilemaps */
	ROM_LOAD( "6.18d",   0x0000, 0x4000, CRC(80c20d36) SHA1(f20724754824030d62059388f3ea2224f5b7a60e) )
	ROM_LOAD( "5.17d",   0x4000, 0x4000, CRC(1da7a651) SHA1(5307452058164a0bc39d144dd204627a9ead7543) )

	ROM_REGION( 0x0400, "proms", 0 )    /* Region 3 - color data */
	ROM_LOAD( "pr.6e",  0x0000, 0x0100, CRC(ea47b91a) SHA1(9921aa1ef882fb664d85d3e065223610262ca112) )  /* red */
	ROM_LOAD( "pr.7e",  0x0100, 0x0100, CRC(6d66da81) SHA1(ffdd1778ce5b7614b90b5da85589c5871405d3fe) )  /* green */ // different (bad?)
	ROM_LOAD( "pr.8e",  0x0200, 0x0100, CRC(ffb4b287) SHA1(c3c7018e6d5e18cc2db135812d0dc3824710ab4c) )  /* blue */
	ROM_LOAD( "pr.2e",  0x0300, 0x0100, CRC(23bade78) SHA1(7e2de5eb08d888f97830807b6dbe85d09bb3b7f8) )  /* sprite lookup table */

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "pr.7h",  0x0000, 0x0100, CRC(262d0809) SHA1(a67281af02cef082023c0d7d57e3824aeef67450) )  /* sprite palette bank */
ROM_END


WRITE8_MEMBER(galivan_state::youmab_extra_bank_w)
{
	if (data == 0xff)
		membank("bank2")->set_entry(1);
	else if (data == 0x00)
		membank("bank2")->set_entry(0);
	else
		printf("data %03x\n", data);
}

READ8_MEMBER(galivan_state::youmab_8a_r)
{
	return machine().rand();
}

WRITE8_MEMBER(galivan_state::youmab_81_w)
{
	// ??
}

/* scrolling is tied to a serial port, reads from 0xe43d-0xe43e-0xe43f-0xe440 */
WRITE8_MEMBER(galivan_state::youmab_84_w)
{
	m_shift_val &= ~((0x80 >> 7) << m_shift_scroll);
	m_shift_val |= (((data & 0x80) >> 7) << m_shift_scroll);

	m_shift_scroll++;

	//popmessage("%08x",m_shift_val);

	//if(m_shift_scroll == 25)
}

WRITE8_MEMBER(galivan_state::youmab_86_w)
{
	/* latch values */
	{
		m_scrolly = (m_shift_val & 0x0003ff);
		m_scrollx = (m_shift_val & 0x7ffc00) >> 10;

		//popmessage("%08x %08x %08x",m_scrollx,m_scrolly,m_shift_val);
	}

	m_shift_val = 0;
	m_shift_scroll = 0;
}

DRIVER_INIT_MEMBER(galivan_state,youmab)
{
	m_maincpu->space(AS_IO).install_write_handler(0x82, 0x82, write8_delegate(FUNC(galivan_state::youmab_extra_bank_w),this)); // banks rom at 0x8000? writes 0xff and 0x00 before executing code there
	m_maincpu->space(AS_PROGRAM).install_read_bank(0x0000, 0x7fff, "bank3");
	membank("bank3")->set_base(memregion("maincpu")->base());

	m_maincpu->space(AS_PROGRAM).install_read_bank(0x8000, 0xbfff, "bank2");
	membank("bank2")->configure_entries(0, 2, memregion("user2")->base(), 0x4000);
	membank("bank2")->set_entry(0);

	m_maincpu->space(AS_IO).install_write_handler(0x81, 0x81, write8_delegate(FUNC(galivan_state::youmab_81_w),this)); // ?? often, alternating values
	m_maincpu->space(AS_IO).install_write_handler(0x84, 0x84, write8_delegate(FUNC(galivan_state::youmab_84_w),this)); // ?? often, sequence..

	m_maincpu->space(AS_PROGRAM).nop_write(0xd800, 0xd81f); // scrolling isn't here..

	m_maincpu->space(AS_IO).install_read_handler(0x8a, 0x8a, read8_delegate(FUNC(galivan_state::youmab_8a_r),this)); // ???

	m_maincpu->space(AS_IO).install_write_handler(0x86, 0x86, write8_delegate(FUNC(galivan_state::youmab_86_w),this));

}

GAME( 1985, galivan,  0,        galivan,  galivan, driver_device,  0, ROT270, "Nichibutsu",   "Cosmo Police Galivan (12/26/1985)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, galivan2, galivan,  galivan,  galivan, driver_device,  0, ROT270, "Nichibutsu",   "Cosmo Police Galivan (12/16/1985)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, galivan3, galivan,  galivan,  galivan, driver_device,  0, ROT270, "Nichibutsu",   "Cosmo Police Galivan (12/11/1985)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, dangar,   0,        galivan,  dangar, driver_device,   0, ROT270, "Nichibutsu",   "Ufo Robo Dangar (12/1/1986)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, dangar2,  dangar,   galivan,  dangar2, driver_device,  0, ROT270, "Nichibutsu",   "Ufo Robo Dangar (9/26/1986)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, dangarb,  dangar,   galivan,  dangarb, driver_device,  0, ROT270, "bootleg",      "Ufo Robo Dangar (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, ninjemak, 0,        ninjemak, ninjemak, driver_device, 0, ROT270, "Nichibutsu",   "Ninja Emaki (US)", MACHINE_SUPPORTS_SAVE|MACHINE_UNEMULATED_PROTECTION )
GAME( 1986, youma,    ninjemak, ninjemak, ninjemak, driver_device, 0, ROT270, "Nichibutsu",   "Youma Ninpou Chou (Japan)", MACHINE_SUPPORTS_SAVE|MACHINE_UNEMULATED_PROTECTION )
GAME( 1986, youma2,   ninjemak, ninjemak, ninjemak, driver_device, 0, ROT270, "Nichibutsu",   "Youma Ninpou Chou (Japan, alt)", MACHINE_SUPPORTS_SAVE|MACHINE_UNEMULATED_PROTECTION )
GAME( 1986, youmab,   ninjemak, youmab,   ninjemak, galivan_state, youmab, ROT270, "bootleg", "Youma Ninpou Chou (Game Electronics bootleg, set 1)", MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE|MACHINE_UNEMULATED_PROTECTION ) // player is invincible
GAME( 1986, youmab2,  ninjemak, youmab,   ninjemak, galivan_state, youmab, ROT270, "bootleg", "Youma Ninpou Chou (Game Electronics bootleg, set 2)", MACHINE_NOT_WORKING|MACHINE_SUPPORTS_SAVE|MACHINE_UNEMULATED_PROTECTION ) // ""
