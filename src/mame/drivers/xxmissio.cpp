// license:BSD-3-Clause
// copyright-holders:Uki
/*****************************************************************************

XX Mission (c) 1986 UPL

    Driver by Uki

    31/Mar/2001 -

*****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "includes/xxmissio.h"


WRITE8_MEMBER(xxmissio_state::bank_sel_w)
{
	membank("bank1")->set_entry(data & 7);
}

CUSTOM_INPUT_MEMBER(xxmissio_state::status_r)
{
	int bit_mask = (FPTR)param;
	return (m_status & bit_mask) ? 1 : 0;
}

WRITE8_MEMBER(xxmissio_state::status_m_w)
{
	switch (data)
	{
		case 0x00:
			m_status |= 0x20;
			break;

		case 0x40:
			m_status &= ~0x08;
			m_subcpu->set_input_line_and_vector(0, HOLD_LINE, 0x10);
			break;

		case 0x80:
			m_status |= 0x04;
			break;
	}
}

WRITE8_MEMBER(xxmissio_state::status_s_w)
{
	switch (data)
	{
		case 0x00:
			m_status |= 0x10;
			break;

		case 0x40:
			m_status |= 0x08;
			break;

		case 0x80:
			m_status &= ~0x04;
			m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0x10);
			break;
	}
}

INTERRUPT_GEN_MEMBER(xxmissio_state::interrupt_m)
{
	m_status &= ~0x20;
	m_maincpu->set_input_line(0, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(xxmissio_state::interrupt_s)
{
	m_status &= ~0x10;
	m_subcpu->set_input_line(0, HOLD_LINE);
}

void xxmissio_state::machine_start()
{
	membank("bank1")->configure_entries(0, 8, memregion("user1")->base(), 0x4000);
	membank("bank1")->set_entry(0);

	save_item(NAME(m_status));
}

/****************************************************************************/

static ADDRESS_MAP_START( map1, AS_PROGRAM, 8, xxmissio_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM

	AM_RANGE(0x8000, 0x8001) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
	AM_RANGE(0x8002, 0x8003) AM_DEVREADWRITE("ym2", ym2203_device, read, write)

	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P1")
	AM_RANGE(0xa001, 0xa001) AM_READ_PORT("P2")
	AM_RANGE(0xa002, 0xa002) AM_READ_PORT("STATUS")
	AM_RANGE(0xa002, 0xa002) AM_WRITE(status_m_w)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(flipscreen_w)

	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("fgram")
	AM_RANGE(0xc800, 0xcfff) AM_READWRITE(bgram_r, bgram_w) AM_SHARE("bgram")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_SHARE("spriteram")

	AM_RANGE(0xd800, 0xdaff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE(0xe000, 0xefff) AM_SHARE("share5") AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_SHARE("share6") AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( map2, AS_PROGRAM, 8, xxmissio_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")

	AM_RANGE(0x8000, 0x8001) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
	AM_RANGE(0x8002, 0x8003) AM_DEVREADWRITE("ym2", ym2203_device, read, write)
	AM_RANGE(0x8006, 0x8006) AM_WRITE(bank_sel_w)

	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P1")
	AM_RANGE(0xa001, 0xa001) AM_READ_PORT("P2")
	AM_RANGE(0xa002, 0xa002) AM_READ_PORT("STATUS")
	AM_RANGE(0xa002, 0xa002) AM_WRITE(status_s_w)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(flipscreen_w)

	AM_RANGE(0xc000, 0xc7ff) AM_SHARE("fgram") AM_RAM
	AM_RANGE(0xc800, 0xcfff) AM_SHARE("bgram") AM_READWRITE(bgram_r, bgram_w)
	AM_RANGE(0xd000, 0xd7ff) AM_SHARE("spriteram") AM_RAM

	AM_RANGE(0xd800, 0xdaff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE(0xe000, 0xefff) AM_SHARE("share6") AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_SHARE("share5") AM_RAM
ADDRESS_MAP_END


/****************************************************************************/

static INPUT_PORTS_START( xxmissio )
	PORT_START("P1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, "Endless Game (Cheat)")   PORT_DIPLOCATION("SW1:7") /* Shown as "Unused" in the manual */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW1:8" )     /* Shown as "Unused" in the manual */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, "First Bonus" )       PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPNAME( 0x18, 0x08, "Bonus Every" )       PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "50000" )
	PORT_DIPSETTING(    0x08, "70000" )
	PORT_DIPSETTING(    0x10, "90000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" ) /* Shown as "Unused" in the manual */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" ) /* Shown as "Unused" in the manual */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" ) /* Shown as "Unused" in the manual */

	PORT_START("STATUS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, xxmissio_state, status_r, (void *)0x01)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, xxmissio_state, status_r, (void *)0x04)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, xxmissio_state, status_r, (void *)0x08)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, xxmissio_state, status_r, (void *)0x10)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, xxmissio_state, status_r, (void *)0x20)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, xxmissio_state, status_r, (void *)0x40)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, xxmissio_state, status_r, (void *)0x80)
INPUT_PORTS_END

/****************************************************************************/

static const gfx_layout charlayout =
{
	16,8,   /* 16*8 characters */
	2048,   /* 2048 characters */
	4,      /* 4 bits per pixel */
	{0,1,2,3},
	{0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60},
	{64*0, 64*1, 64*2, 64*3, 64*4, 64*5, 64*6, 64*7},
	64*8
};

static const gfx_layout spritelayout =
{
	32,16,    /* 32*16 characters */
	512,      /* 512 sprites */
	4,        /* 4 bits per pixel */
	{0,1,2,3},
	{0,4,8,12,16,20,24,28,
		32,36,40,44,48,52,56,60,
		8*64+0,8*64+4,8*64+8,8*64+12,8*64+16,8*64+20,8*64+24,8*64+28,
		8*64+32,8*64+36,8*64+40,8*64+44,8*64+48,8*64+52,8*64+56,8*64+60},
	{64*0, 64*1, 64*2, 64*3, 64*4, 64*5, 64*6, 64*7,
		64*16, 64*17, 64*18, 64*19, 64*20, 64*21, 64*22, 64*23},
	64*8*4
};

static const gfx_layout bglayout =
{
	16,8,   /* 16*8 characters */
	1024,   /* 1024 characters */
	4,      /* 4 bits per pixel */
	{0,1,2,3},
	{0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60},
	{64*0, 64*1, 64*2, 64*3, 64*4, 64*5, 64*6, 64*7},
	64*8
};

static GFXDECODE_START( xxmissio )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,   256,  8 ) /* FG */
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout,   0,  8 ) /* sprite */
	GFXDECODE_ENTRY( "gfx2", 0x0000, bglayout,     512, 16 ) /* BG */
GFXDECODE_END

/****************************************************************************/

static MACHINE_CONFIG_START( xxmissio, xxmissio_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,12000000/4) /* 3.0MHz */
	MCFG_CPU_PROGRAM_MAP(map1)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", xxmissio_state,  interrupt_m)

	MCFG_CPU_ADD("sub", Z80,12000000/4) /* 3.0MHz */
	MCFG_CPU_PROGRAM_MAP(map2)
	MCFG_CPU_PERIODIC_INT_DRIVER(xxmissio_state, interrupt_s, 2*60)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 4*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(xxmissio_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", xxmissio)
	MCFG_PALETTE_ADD("palette", 768)
	MCFG_PALETTE_FORMAT_CLASS(1, xxmissio_state, BBGGRRII)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, 12000000/8)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.40)

	MCFG_SOUND_ADD("ym2", YM2203, 12000000/8)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(xxmissio_state, scroll_x_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(xxmissio_state, scroll_y_w))
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.40)
MACHINE_CONFIG_END

/****************************************************************************/

ROM_START( xxmissio )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* CPU1 */
	ROM_LOAD( "xx1.4l", 0x0000,  0x8000, CRC(86e07709) SHA1(7bfb7540b6509f07a6388ca2da6b3892f5b1df74) )

	ROM_REGION( 0x10000, "sub", 0 ) /* CPU2 */
	ROM_LOAD( "xx2.4b", 0x0000,  0x4000, CRC(13fa7049) SHA1(e8974d9f271a966611b523496ba8cd910e227a23) )

	ROM_REGION( 0x18000, "user1", 0 ) /* BANK */
	ROM_LOAD( "xx3.6a", 0x00000,  0x8000, CRC(16fdacab) SHA1(2158ca9b14c52bc1cd5ef0f4c0180f0519224403) )
	ROM_LOAD( "xx4.6b", 0x08000,  0x8000, CRC(274bd4d2) SHA1(2ddf9b953584e26f221b1c86181d827bdc3dc81b) )
	ROM_LOAD( "xx5.6d", 0x10000,  0x8000, CRC(c5f35535) SHA1(6812b70beb73fc80cf20d2d51f747952ed106887) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* FG/sprites */
	ROM_LOAD16_BYTE( "xx6.8j", 0x00001, 0x8000, CRC(dc954d01) SHA1(73ecbbc859da9db9fead91cd03bb90e5779916e2) )
	ROM_LOAD16_BYTE( "xx8.8f", 0x00000, 0x8000, CRC(a9587cc6) SHA1(5fbcb88505f89c4d8a2a228489612ff66fc5d3af) )
	ROM_LOAD16_BYTE( "xx7.8h", 0x10001, 0x8000, CRC(abe9cd68) SHA1(f3ce9b40e3d9cdc9b77a43f9d5d0411338d88833) )
	ROM_LOAD16_BYTE( "xx9.8e", 0x10000, 0x8000, CRC(854e0e5f) SHA1(b01d6a735b175c2f7ac3fc4053702c9da62c6a4e) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* BG */
	ROM_LOAD16_BYTE( "xx10.4c", 0x0000,  0x8000, CRC(d27d7834) SHA1(60c24dc2ab7e2a33da4002f1f07eaf7898cf387f) )
	ROM_LOAD16_BYTE( "xx11.4b", 0x0001,  0x8000, CRC(d9dd827c) SHA1(aea3a5abd871adf7f75ad4d6cc57eff0833135c7) )
ROM_END

GAME( 1986, xxmissio, 0, xxmissio, xxmissio, driver_device, 0, ROT90, "UPL", "XX Mission", MACHINE_SUPPORTS_SAVE )
