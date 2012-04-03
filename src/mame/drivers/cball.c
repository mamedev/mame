/***************************************************************************

    Atari Cannonball (prototype) driver

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"


class cball_state : public driver_device
{
public:
	cball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_video_ram;

	/* video-related */
	tilemap_t* m_bg_tilemap;

	/* devices */
	device_t *m_maincpu;
	DECLARE_WRITE8_MEMBER(cball_vram_w);
	DECLARE_READ8_MEMBER(cball_wram_r);
	DECLARE_WRITE8_MEMBER(cball_wram_w);
};


static TILE_GET_INFO( get_tile_info )
{
	cball_state *state = machine.driver_data<cball_state>();
	UINT8 code = state->m_video_ram[tile_index];

	SET_TILE_INFO(0, code, code >> 7, 0);
}


WRITE8_MEMBER(cball_state::cball_vram_w)
{

	m_video_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


static VIDEO_START( cball )
{
	cball_state *state = machine.driver_data<cball_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}


static SCREEN_UPDATE_IND16( cball )
{
	cball_state *state = screen.machine().driver_data<cball_state>();

	/* draw playfield */
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	/* draw sprite */
	drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[1],
		state->m_video_ram[0x399] >> 4,
		0,
		0, 0,
		240 - state->m_video_ram[0x390],
		240 - state->m_video_ram[0x398], 0);
	return 0;
}


static TIMER_CALLBACK( interrupt_callback )
{
	cball_state *state = machine.driver_data<cball_state>();
	int scanline = param;

	generic_pulse_irq_line(state->m_maincpu, 0, 1);

	scanline = scanline + 32;

	if (scanline >= 262)
		scanline = 16;

	machine.scheduler().timer_set(machine.primary_screen->time_until_pos(scanline), FUNC(interrupt_callback), scanline);
}


static MACHINE_START( cball )
{
	cball_state *state = machine.driver_data<cball_state>();
	state->m_maincpu = machine.device("maincpu");
}

static MACHINE_RESET( cball )
{
	machine.scheduler().timer_set(machine.primary_screen->time_until_pos(16), FUNC(interrupt_callback), 16);
}


static PALETTE_INIT( cball )
{
	palette_set_color(machine, 0, MAKE_RGB(0x80, 0x80, 0x80));
	palette_set_color(machine, 1, MAKE_RGB(0x00, 0x00, 0x00));
	palette_set_color(machine, 2, MAKE_RGB(0x80, 0x80, 0x80));
	palette_set_color(machine, 3, MAKE_RGB(0xff, 0xff, 0xff));
	palette_set_color(machine, 4, MAKE_RGB(0x80, 0x80, 0x80));
	palette_set_color(machine, 5, MAKE_RGB(0xc0, 0xc0, 0xc0));
}


READ8_MEMBER(cball_state::cball_wram_r)
{

	return m_video_ram[0x380 + offset];
}


WRITE8_MEMBER(cball_state::cball_wram_w)
{

	m_video_ram[0x380 + offset] = data;
}



static ADDRESS_MAP_START( cpu_map, AS_PROGRAM, 8, cball_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)

	AM_RANGE(0x0000, 0x03ff) AM_READ(cball_wram_r) AM_MASK(0x7f)
	AM_RANGE(0x0400, 0x07ff) AM_READONLY
	AM_RANGE(0x1001, 0x1001) AM_READ_PORT("1001")
	AM_RANGE(0x1003, 0x1003) AM_READ_PORT("1003")
	AM_RANGE(0x1020, 0x1020) AM_READ_PORT("1020")
	AM_RANGE(0x1040, 0x1040) AM_READ_PORT("1040")
	AM_RANGE(0x1060, 0x1060) AM_READ_PORT("1060")
	AM_RANGE(0x2000, 0x2001) AM_NOP
	AM_RANGE(0x2800, 0x2800) AM_READ_PORT("2800")

	AM_RANGE(0x0000, 0x03ff) AM_WRITE(cball_wram_w) AM_MASK(0x7f)
	AM_RANGE(0x0400, 0x07ff) AM_WRITE(cball_vram_w) AM_BASE(m_video_ram)
	AM_RANGE(0x1800, 0x1800) AM_NOP /* watchdog? */
	AM_RANGE(0x1810, 0x1811) AM_NOP
	AM_RANGE(0x1820, 0x1821) AM_NOP
	AM_RANGE(0x1830, 0x1831) AM_NOP
	AM_RANGE(0x1840, 0x1841) AM_NOP
	AM_RANGE(0x1850, 0x1851) AM_NOP
	AM_RANGE(0x1870, 0x1871) AM_NOP

	AM_RANGE(0x7000, 0x7fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( cball )
	PORT_START("1001")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "2 Coins each" )
	PORT_DIPSETTING(    0xc0, "1 Coin each" )
	PORT_DIPSETTING(    0x80, "1 Coin 1 Game" )
	PORT_DIPSETTING(    0x40, "1 Coin 2 Games" )

	PORT_START("1003")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "7" )
	PORT_DIPSETTING(    0x00, "9" )

	PORT_START("1020")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("1040")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("1060")
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("2800")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )
INPUT_PORTS_END


static const gfx_layout tile_layout =
{
	8, 8,
	64,
	1,
	{ 0 },
	{
		0, 1, 2, 3, 4, 5, 6, 7
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38
	},
	0x40
};


static const gfx_layout sprite_layout =
{
	16, 16,
	16,
	1,
	{ 0 },
	{
		0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0,
		0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
		0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0
	},
	0x100
};


static GFXDECODE_START( cball )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, sprite_layout, 4, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( cball, cball_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_12_096MHz / 16) /* ? */
	MCFG_CPU_PROGRAM_MAP(cpu_map)

	MCFG_MACHINE_START(cball)
	MCFG_MACHINE_RESET(cball)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(256, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 0, 223)
	MCFG_SCREEN_UPDATE_STATIC(cball)

	MCFG_GFXDECODE(cball)
	MCFG_PALETTE_LENGTH(6)

	MCFG_PALETTE_INIT(cball)
	MCFG_VIDEO_START(cball)

	/* sound hardware */
MACHINE_CONFIG_END


ROM_START( cball )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "canball.1e", 0x7400, 0x0400, CRC(0b34823b) SHA1(0db6b9f78f7c07ee7d35f2bf048ba61fe43b1e26) )
	ROM_LOAD_NIB_HIGH( "canball.1l", 0x7400, 0x0400, CRC(b43ca275) SHA1(a03e03f6366877cfdcec71030a5fb2c5171c8d8a) )
	ROM_LOAD_NIB_LOW ( "canball.1f", 0x7800, 0x0400, CRC(29b4e1f7) SHA1(8cef944b6e0153c304aa2d4cfdc530b8a4eef021) )
	ROM_LOAD_NIB_HIGH( "canball.1k", 0x7800, 0x0400, CRC(a4d1cf12) SHA1(99de6470efd16e57d72019e065f55bc740f3c7fc) )
	ROM_LOAD_NIB_LOW ( "canball.1h", 0x7c00, 0x0400, CRC(13f55937) SHA1(7514c27e60944c4e00992c8ecbc5115f8ff948bb) )
	ROM_LOAD_NIB_HIGH( "canball.1j", 0x7c00, 0x0400, CRC(5b905d69) SHA1(2408dd6e44c51c0c9bdb82d2d33826c03f8308c4) )

	ROM_REGION( 0x0200, "gfx1", 0 ) /* tiles */
	ROM_LOAD_NIB_LOW ( "canball.6m", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD_NIB_HIGH( "canball.6l", 0x0000, 0x0200, CRC(5b1c9e88) SHA1(6e9630db9907170c53942a21302bcf8b721590a3) )

	ROM_REGION( 0x0200, "gfx2", 0 ) /* sprites */
	ROM_LOAD_NIB_LOW ( "canball.5l", 0x0000, 0x0200, CRC(3d0d1569) SHA1(1dfcf5cf9468d476c4b7d76a261c6fec87a99f93) )
	ROM_LOAD_NIB_HIGH( "canball.5k", 0x0000, 0x0200, CRC(c5fdd3c8) SHA1(5aae148439683ff1cf0005a810c81fdcbed525c3) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "canball.6h", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) ) /* sync */
ROM_END


GAME( 1976, cball, 0, cball, cball, 0, ROT0, "Atari", "Cannonball (Atari, prototype)", GAME_NO_SOUND | GAME_WRONG_COLORS | GAME_IMPERFECT_GRAPHICS )
