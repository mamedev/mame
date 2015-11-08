// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/*

Meadows Warp Speed

Driver by Mariusz Wojcieszek

Notes:
- Circles drawing does not follow the hardware. Roms which come in pairs four times
  on the board are used by circle generators. These roms are thought to contain cosine tables,
  16 bits wide, with g17 being the MSB and g18 being the LSB
- Circles colors are probably not correct
- Starfield is wrong. It is done with tilemap fixed in rom, but rom mapping is not correct.
  Starfield scrolling is missing too
- What are unknown roms used for?

Hardware registers:
0x00 - 0x1f control register for circles generator (8 bytes each)
            0x00, 0x01  circle radius
            0x02, 0x03  circle middle point
            0x04, 0x05  circle middle point
            0x06        circle colour (0-7)
            0x07        unused
0x20        ?
0x21        sound (intro screens have bit 1 toggled for click effect)
0x22        ?
0x23        ?
0x24        ?
0x25        ?
0x26        ?
0x27        ?

Board etched...
    MEADOWS 024-0084
    MADE IN USA

Empty socket at .E3
Z80 processor
6 2102 memory chips
2 2112 memory chips
5Mhz crystal
All PROMS are SN74s474

.L18    no sticker
.L17    stickered       L9, L13
                L17, G17
.L15    stickered       L10, L15
                L18, G18
.L13    stickered       L9, L13
                L17, G17
.L10    stickered       L10, L15
                L18, G18
.L9 stickered (damaged) xxx, L13
                xxx, G1y
.K1 stickered       K1
.G2 no sticker      K1
.E4 no sticker
.E5 stickered       M16
                PRO
                1
.E6 stickered       M16
                PRO
                3
.C3 can't read
.C4 stickered       M16
                PRO
                4
.C5 stickered       M16
                PRO
                0
.C6 stickered       M16
                PRO
                2
.E8 stickered       E8
.E10    stickered       E10
.C12    stickered       C12
.G17    stickered       L9, L13
                L17, G17
.G18    stickered       L10, L15
                L18, G18

L9, L13, L17 and G17 all read the same
L10, L15, L18 and G18 all read the same

*/

#include "emu.h"
#include "cpu/z80/z80.h"

class warpspeed_state : public driver_device
{
public:
	warpspeed_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram"),
		m_workram(*this, "workram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_workram;

	tilemap_t   *m_text_tilemap;
	tilemap_t   *m_starfield_tilemap;
	UINT8       m_regs[0x28];

	DECLARE_WRITE8_MEMBER(hardware_w);
	DECLARE_WRITE8_MEMBER(vidram_w);

	TILE_GET_INFO_MEMBER(get_text_tile_info);
	TILE_GET_INFO_MEMBER(get_starfield_tile_info);

	virtual void video_start();
	DECLARE_PALETTE_INIT(warpspeed);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_circles(bitmap_ind16 &bitmap);
};

WRITE8_MEMBER(warpspeed_state::hardware_w)
{
	m_regs[offset] = data;
}

TILE_GET_INFO_MEMBER(warpspeed_state::get_text_tile_info)
{
	UINT8 code = m_videoram[tile_index] & 0x3f;
	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}

TILE_GET_INFO_MEMBER(warpspeed_state::get_starfield_tile_info)
{
	UINT8 code = 0x3f;
	if ( tile_index & 1 )
	{
		code = memregion("starfield")->base()[tile_index >> 1] & 0x3f;
	}
	SET_TILE_INFO_MEMBER(1, code, 0, 0);
}

WRITE8_MEMBER(warpspeed_state::vidram_w)
{
	m_videoram[offset] = data;
	m_text_tilemap->mark_tile_dirty(offset);
}

void warpspeed_state::video_start()
{
	m_text_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(warpspeed_state::get_text_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_text_tilemap->set_transparent_pen(0);
	m_starfield_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(warpspeed_state::get_starfield_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_starfield_tilemap->mark_all_dirty();

	save_item(NAME(m_regs));
}

static void draw_circle_line(bitmap_ind16 &bitmap, int x, int y, int l, int color)
{
	if (y >= 0 && y <= bitmap.height() - 1)
	{
		UINT16* pLine = &bitmap.pix16(y);

		int h1 = x - l;
		int h2 = x + l;

		if (h1 < 0)
			h1 = 0;
		if (h2 > bitmap.width() - 1)
			h2 = bitmap.width() - 1;

		for (x = h1; x <= h2; x++)
			pLine[x] = color;
	}
}

static void draw_circle(bitmap_ind16 &bitmap, INT16 cx, INT16 cy, UINT16 radius, UINT8 color )
{
	/* Bresenham's circle algorithm */

	int x = 0;
	int y = radius;

	int d = 3 - 2 * radius;

	while (x <= y)
	{
		draw_circle_line(bitmap, cx, cy - x, y, color);
		draw_circle_line(bitmap, cx, cy + x, y, color);
		draw_circle_line(bitmap, cx, cy - y, x, color);
		draw_circle_line(bitmap, cx, cy + y, x, color);

		x++;

		if (d < 0)
			d += 4 * x + 6;
		else
			d += 4 * (x - y--) + 10;
	}
}

void warpspeed_state::draw_circles(bitmap_ind16 &bitmap)
{
	for (int i = 0; i < 4; i++)
	{
		UINT16 radius = m_regs[i*8] + m_regs[i*8 + 1]*256;
		radius = 0xffff - radius;
		radius = sqrt((float)radius);
		INT16 midx = m_regs[i*8 + 2] + m_regs[i*8 + 3]*256;
		midx -= 0xe70;
		INT16 midy = m_regs[i*8 + 4] + m_regs[i*8 + 5]*256;
		midy -= 0xe70;
		if ( radius == 0 || radius == 0xffff )
		{
			continue;
		}
		draw_circle(bitmap, midx + 128 + 16, midy + 128 + 16, radius, (m_regs[i*8 + 6] & 0x07) + 2);
	}
}

UINT32 warpspeed_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_starfield_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_circles(bitmap);
	m_text_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

static ADDRESS_MAP_START( warpspeed_map, AS_PROGRAM, 8, warpspeed_state )
	AM_RANGE(0x0000, 0x0dff) AM_ROM
	AM_RANGE(0x1800, 0x1bff) AM_RAM_WRITE(vidram_w ) AM_SHARE("videoram")
	AM_RANGE(0x1c00, 0x1cff) AM_RAM AM_SHARE("workram")
ADDRESS_MAP_END

static ADDRESS_MAP_START ( warpspeed_io_map, AS_IO, 8, warpspeed_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("DSW")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("IN2")
	AM_RANGE(0x00, 0x27) AM_WRITE(hardware_w )
ADDRESS_MAP_END

static INPUT_PORTS_START( warpspeed )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Accelerate" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Brake" )
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0x3f) PORT_SENSITIVITY(20) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_REVERSE

	PORT_START("IN1")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0x3f) PORT_SENSITIVITY(20) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_REVERSE

	PORT_START("IN2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x7e, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x00, "Coin/Time" )
	PORT_DIPSETTING(    0x00, "50 sec" )
	PORT_DIPSETTING(    0x01, "75 sec" )
	PORT_DIPSETTING(    0x02, "100 sec" )
	PORT_DIPSETTING(    0x03, "125 sec" )
	PORT_DIPSETTING(    0x04, "150 sec" )
	PORT_DIPSETTING(    0x05, "175 sec" )
	PORT_DIPSETTING(    0x06, "200 sec" )
	PORT_DIPSETTING(    0x07, "225 sec" )

	PORT_DIPUNKNOWN( 0x08, 0x00 )

	PORT_DIPUNKNOWN( 0x10, 0x00 )

	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )

	PORT_DIPUNUSED( 0x80, 0x00 )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	8*8
};

static GFXDECODE_START( warpspeed )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 1  )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   0, 1  )
GFXDECODE_END

PALETTE_INIT_MEMBER(warpspeed_state, warpspeed)
{
	// tilemaps
	palette.set_pen_color(0,rgb_t::black); /* black */
	palette.set_pen_color(1,rgb_t::white); /* white */

	// circles
	for ( int i = 0; i < 8; i++ )
	{
		palette.set_pen_color(2 + i, 0xff*BIT(i,0), 0xff*BIT(i,1), 0xff*BIT(i,2));
	}
}

static MACHINE_CONFIG_START( warpspeed, warpspeed_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_5MHz/2)
	MCFG_CPU_PROGRAM_MAP(warpspeed_map)
	MCFG_CPU_IO_MAP(warpspeed_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", warpspeed_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE((32)*8, (32)*8)
	MCFG_SCREEN_VISIBLE_AREA(4*8, 32*8-1, 8*8, 32*8-1)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_UPDATE_DRIVER(warpspeed_state, screen_update)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", warpspeed)
	MCFG_PALETTE_ADD("palette", 2+8)
	MCFG_PALETTE_INIT_OWNER(warpspeed_state, warpspeed)
MACHINE_CONFIG_END

ROM_START( warpsped )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "m16 pro 0.c5",  0x0000, 0x0200, CRC(81f33dfb) SHA1(5c4adef88e1e7f1a9f7c156f17b98ba522ddee81) )
	ROM_LOAD( "m16 pro 1.e5",  0x0200, 0x0200, CRC(135f7421) SHA1(0cabe9a2590fe4f81976a4d10e9b2a223a2d875e) )
	ROM_LOAD( "m16 pro 2.c6",  0x0400, 0x0200, CRC(0a36d152) SHA1(83a2e8f78a36e512da81c44a5ceca7f865bc3508) )
	ROM_LOAD( "m16 pro 3.e6",  0x0600, 0x0200, CRC(ba416cca) SHA1(afb831a79a4a4334fa4caf4e2244c2d4e2f25853) )
	ROM_LOAD( "m16 pro 4.c4",  0x0800, 0x0200, CRC(fc44f25b) SHA1(185f593f2ec6075fd68869d87ed584908031100a) )
	ROM_LOAD( "m16 pro 5.e4",  0x0a00, 0x0200, CRC(7a16bc2b) SHA1(48f58f0c7469da24a3ebee9183b5aae8c676e6ea) )
	ROM_LOAD( "m16 pro 6.c3",  0x0c00, 0x0200, CRC(e2e7940f) SHA1(78c9df32580784c278675d09b89095781893d48f) )

	ROM_REGION(0x1000, "sprite", 0)
	ROM_LOAD( "l9 l13 l17 g17.g17",  0x0000, 0x0200, CRC(7449aae9) SHA1(1f49dad6f60103da6093592efa2087bc24dc0283) )
	ROM_LOAD( "l10 l15 l18 g18.g18", 0x0200, 0x0200, CRC(5829699c) SHA1(20ffa7b81a0de159408d2668005b9ee8a2e588d1) )
	ROM_LOAD( "l9 l13 l17 g17.l9",   0x0400, 0x0200, CRC(7449aae9) SHA1(1f49dad6f60103da6093592efa2087bc24dc0283) )
	ROM_LOAD( "l10 l15 l18 g18.l10", 0x0600, 0x0200, CRC(5829699c) SHA1(20ffa7b81a0de159408d2668005b9ee8a2e588d1) )
	ROM_LOAD( "l9 l13 l17 g17.l13",  0x0800, 0x0200, CRC(7449aae9) SHA1(1f49dad6f60103da6093592efa2087bc24dc0283) )
	ROM_LOAD( "l10 l15 l18 g18.l15", 0x0a00, 0x0200, CRC(5829699c) SHA1(20ffa7b81a0de159408d2668005b9ee8a2e588d1) )
	ROM_LOAD( "l9 l13 l17 g17.l17",  0x0c00, 0x0200, CRC(7449aae9) SHA1(1f49dad6f60103da6093592efa2087bc24dc0283) )
	ROM_LOAD( "l10 l15 l18 g18.l18", 0x0e00, 0x0200, CRC(5829699c) SHA1(20ffa7b81a0de159408d2668005b9ee8a2e588d1) )

	ROM_REGION(0x200, "starfield", 0)
	ROM_LOAD( "e10.e10", 0x0000, 0x0200, CRC(e0d4b72c) SHA1(ae5fae0df9e0bfc67f586649474ff8a69abd7579) )

	ROM_REGION(0x400, "unknown", 0)
	ROM_LOAD( "c12.c12", 0x0000, 0x0200, CRC(88a8db15) SHA1(3fdf4e23cf75cf5dd4d3bad08e9e71c0268f8d79) )
	ROM_LOAD( "e8.e8",   0x0200, 0x0200, CRC(3ef3a576) SHA1(905f9b8d3cabab944a0f6f0736c5c26d0e36107f) )

	ROM_REGION(0x0200, "gfx1", 0)
	ROM_LOAD( "k1.g1",  0x0000, 0x0200, CRC(63d4fa84) SHA1(3465ce27497e2d4fcae994c022480e37e1345686) )

	ROM_REGION(0x0200, "gfx2", 0)
	ROM_LOAD( "k1.k1",  0x0000, 0x0200, CRC(76b10d47) SHA1(e644a50df06535fe1fbfb8754cfc7b4a49fcb05e) )

ROM_END


GAME( 1979?, warpsped,  0,      warpspeed, warpspeed, driver_device, 0, ROT0, "Meadows Games, Inc.", "Warp Speed (prototype)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE ) // year not shown, 1979 is according to date stamps on PCB chips.
