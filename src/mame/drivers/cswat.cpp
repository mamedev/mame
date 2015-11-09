// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Namco Cosmoswat, a lightgun game that uses a projector and a bunch of
  mirrors and motors to move a spotlight(s) on a large projection screen,
  similar to Namco Shoot Away/Shoot Away II.
  The CRT is just used for showing status and keeping scores.

  X1 18.432MHz
  HD68A09EP   - Hitachi M6809 CPU
  2 * M58725P - Mitsubishi 2KB SRAM
  0737        - Namco custom DIP28, clock divider?
  5275        - Namco custom DIP42, sample player

TODO:
- fix gfx/colors (redump needed)
- improve interrupts
- hook up namco 52xx sound
- simulate projector with external artwork?

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/mb88xx/mb88xx.h"
#include "audio/namco52.h"


class cswat_state : public driver_device
{
public:
	cswat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram"),
		m_dips_inp(*this, "DIPS")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<UINT8> m_videoram;
	required_ioport m_dips_inp;

	UINT8 m_nmi_enabled;
	tilemap_t *m_tilemap;

	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_READ8_MEMBER(sensors_r);
	DECLARE_READ8_MEMBER(dipswitch_r);

	INTERRUPT_GEN_MEMBER(nmi_handler);

	TILEMAP_MAPPER_MEMBER(tilemap_scan_rows);
	TILE_GET_INFO_MEMBER(get_tile_info);
	UINT32 screen_update_cswat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void video_start();
	virtual void machine_reset();
	virtual void machine_start();
};


/***************************************************************************

  Video

***************************************************************************/

TILEMAP_MAPPER_MEMBER(cswat_state::tilemap_scan_rows)
{
	// like with pacman, lower and upper parts of vram are left and right columns of the screen
	row += 2;
	col -= 2;
	if (col & 0x20)
		return row + ((col & 0x1f) << 5);
	else
		return col + (row << 5);
}

TILE_GET_INFO_MEMBER(cswat_state::get_tile_info)
{
	int color = m_videoram[tile_index | 0x400];
	int attr = m_videoram[tile_index | 0x800]; // high bits unused
	int code = m_videoram[tile_index] | (attr << 8 & 0x300);
	int flags = TILE_FLIPYX(attr >> 2);

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

void cswat_state::video_start()
{
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cswat_state::get_tile_info),this), tilemap_mapper_delegate(FUNC(cswat_state::tilemap_scan_rows),this), 8, 8, 36, 28);
}

UINT32 cswat_state::screen_update_cswat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/***************************************************************************

  Memory Map, I/O

***************************************************************************/

WRITE8_MEMBER(cswat_state::videoram_w)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(cswat_state::irq_ack_w)
{
	// clear vblank irq and enable nmi?
	m_maincpu->set_input_line(0, CLEAR_LINE);
	m_nmi_enabled = data & 1;
}

READ8_MEMBER(cswat_state::dipswitch_r)
{
	UINT16 dips = m_dips_inp->read();
	return offset ? dips >> 8 : dips & 0xff;
}

READ8_MEMBER(cswat_state::sensors_r)
{
	// ?
	return rand();
}

static ADDRESS_MAP_START( cswat_map, AS_PROGRAM, 8, cswat_state )
	AM_RANGE(0x0000, 0x0bff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM
//  AM_RANGE(0x1800, 0x1800) AM_READNOP // ? reads here after writing to $4000
	AM_RANGE(0x2000, 0x2000) AM_WRITE(irq_ack_w) // writes 1 at end of vblank irq, 0 at gamestart
	AM_RANGE(0x2000, 0x2001) AM_READ(dipswitch_r)
	AM_RANGE(0x2002, 0x2002) AM_WRITE(irq_ack_w) // writes 0 at start of vblank irq
	AM_RANGE(0x2002, 0x2002) AM_READ(sensors_r)
	AM_RANGE(0x2003, 0x2003) AM_READ_PORT("IN0")
//  AM_RANGE(0x4000, 0x4009) AM_NOP // ?
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( cswat )
	PORT_START("DIPS") // SW1 & SW2
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0302, 0x0302, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0102, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0302, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0202, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0404, 0x0404, "Extend Point" )
	PORT_DIPSETTING(      0x0404, "700, 3300, 9000" )
	PORT_DIPSETTING(      0x0400, "800, 3500, 9500" )
	PORT_DIPSETTING(      0x0004, "900, 3800, 10000" )
	PORT_DIPSETTING(      0x0000, "1000, 4000, 10400" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3020, 0x3020, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x3020, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2020, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x4040, 0x4040, "Player Ticket" )
	PORT_DIPSETTING(      0x4040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) ) // dupe
	PORT_DIPSETTING(      0x0000, "Grade 3 or 4" )
	PORT_DIPSETTING(      0x0040, "Grade 4" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Speed" )
	PORT_DIPSETTING(      0x0800, "Slow" )
	PORT_DIPSETTING(      0x0000, "Fast" )
	PORT_DIPNAME( 0x8000, 0x8000, "Freeze" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8,1), STEP4(0*8,1) },
	{ STEP8(0*8,8) },
	16*8
};

static GFXDECODE_START( cswat )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 256 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(cswat_state::nmi_handler)
{
	if (m_nmi_enabled)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

void cswat_state::machine_reset()
{
	m_nmi_enabled = 0;
}

void cswat_state::machine_start()
{
	save_item(NAME(m_nmi_enabled));
}

static MACHINE_CONFIG_START( cswat, cswat_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809E, XTAL_18_432MHz/3/4) // HD68A09EP, 1.5MHz?
	MCFG_CPU_PROGRAM_MAP(cswat_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cswat_state, irq0_line_assert)
	MCFG_CPU_PERIODIC_INT_DRIVER(cswat_state, nmi_handler, 300) // ?

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(36*8, 28*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_SCREEN_UPDATE_DRIVER(cswat_state, screen_update_cswat)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cswat)
	MCFG_PALETTE_ADD("palette", 4*256)

	/* sound hardware */
	// TODO
MACHINE_CONFIG_END


/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( cswat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs2-1.5k",  0x8000, 0x2000, CRC(896604a8) SHA1(a7b2e2253eea8a4db6f6ea049d7a21050ea71d4b) )
	ROM_LOAD( "cs2-2.5j",  0xa000, 0x2000, CRC(746f5afb) SHA1(ebb5ad49d97ea3e4e25104c67168644cb833bd87) )
	ROM_LOAD( "cs2-3.5h",  0xc000, 0x2000, CRC(52af8d0b) SHA1(1087b1dbf8b7734f0497e0ad1f8a7a10a25f4e7b) )
	ROM_LOAD( "cs2-4.5f",  0xe000, 0x2000, CRC(2f88074c) SHA1(a8864b6123f76c0d1f372b2196d4e436bac75c21) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "cs2-5.3l",  0x0000, 0x2000, BAD_DUMP CRC(a09d6894) SHA1(feac4980d5bf6f2a808230962b4140e27767dc9d) ) // bad
	ROM_LOAD( "cs2-6.2l",  0x2000, 0x2000, CRC(95e8180f) SHA1(1d819649b62b194e10decb04d6be68c99c419a93) )

	ROM_REGION( 0x2000, "52xx", 0 ) // digitised speech
	ROM_LOAD( "cs2-7.2b",  0x0000, 0x2000, BAD_DUMP CRC(25a25dc0) SHA1(849e3baa159049754da6fba74c77c86f01952849) ) // FIXED BITS (xxx1xxxx)

	ROM_REGION( 0x0001, "proms", 0 ) // color prom
	ROM_LOAD( "cs2-1.1p",  0x0000, 0x0001, NO_DUMP )
ROM_END


GAME( 1984, cswat, 0, cswat, cswat, driver_device, 0, ROT0, "Namco", "Cosmoswat", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS )
