// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*

 Enerdyne Technologies Inc. (El Cajon, CA 92020) hardware

 CPU: Z80
 Sound: AY-3-8912 (x3)
 Other: Dallas DS1220Y NVRAM, N8T245N (x2), PAL16L8A-2CN (x2,protected)

 XTAL = 12 MHz

Supported games:

- Progressive Music Trivia  (c) 1985
- Super Trivia Master       (c) 1986

 driver by Pierpaolo Prazzoli, thanks to Tomasz Slanina too.

Notes:

 You can swap the question ROMs arbitrarily on these boards. This means
  the ROMsets in this driver aren't true sets per se, they're just how
  boards were found "in the wild."
 ROMs with music questions come in hi|lo pairs.

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"
#include "video/resnet.h"


class ettrivia_state : public driver_device
{
public:
	ettrivia_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	int m_palreg;
	int m_gfx_bank;
	int m_question_bank;
	int m_b000_val;
	int m_b000_ret;
	int m_b800_prev;
	required_shared_ptr<UINT8> m_fg_videoram;
	required_shared_ptr<UINT8> m_bg_videoram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	DECLARE_WRITE8_MEMBER(ettrivia_fg_w);
	DECLARE_WRITE8_MEMBER(ettrivia_bg_w);
	DECLARE_WRITE8_MEMBER(ettrivia_control_w);
	DECLARE_READ8_MEMBER(ettrivia_question_r);
	DECLARE_WRITE8_MEMBER(b000_w);
	DECLARE_READ8_MEMBER(b000_r);
	DECLARE_WRITE8_MEMBER(b800_w);
	TILE_GET_INFO_MEMBER(get_tile_info_bg);
	TILE_GET_INFO_MEMBER(get_tile_info_fg);
	virtual void video_start();
	DECLARE_PALETTE_INIT(ettrivia);
	UINT32 screen_update_ettrivia(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(ettrivia_interrupt);
	inline void get_tile_info(tile_data &tileinfo, int tile_index, UINT8 *vidram, int gfx_code);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};


WRITE8_MEMBER(ettrivia_state::ettrivia_fg_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(ettrivia_state::ettrivia_bg_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(ettrivia_state::ettrivia_control_w)
{
	machine().tilemap().mark_all_dirty();

	m_palreg  = (data >> 1) & 3;
	m_gfx_bank = (data >> 2) & 1;

	m_question_bank = (data >> 3) & 3;

	coin_counter_w(machine(), 0, data & 0x80);

	flip_screen_set(data & 1);
}

READ8_MEMBER(ettrivia_state::ettrivia_question_r)
{
	UINT8 *QUESTIONS = memregion("user1")->base();
	return QUESTIONS[offset + 0x10000 * m_question_bank];
}

WRITE8_MEMBER(ettrivia_state::b000_w)
{
	m_b000_val = data;
}

READ8_MEMBER(ettrivia_state::b000_r)
{
	if(m_b800_prev)
		return m_b000_ret;
	else
		return m_b000_val;
}

WRITE8_MEMBER(ettrivia_state::b800_w)
{
	switch(data)
	{
		/* special case to return the value written to 0xb000 */
		/* does it reset the chips too ? */
		case 0: break;
		case 0xc4: m_b000_ret = machine().device<ay8910_device>("ay1")->data_r(space, 0);    break;
		case 0x94: m_b000_ret = machine().device<ay8910_device>("ay2")->data_r(space, 0);    break;
		case 0x86: m_b000_ret = machine().device<ay8910_device>("ay3")->data_r(space, 0);    break;

		case 0x80:
			switch(m_b800_prev)
			{
				case 0xe0: machine().device<ay8910_device>("ay1")->address_w(space,0,m_b000_val);    break;
				case 0x98: machine().device<ay8910_device>("ay2")->address_w(space,0,m_b000_val);    break;
				case 0x83: machine().device<ay8910_device>("ay3")->address_w(space,0,m_b000_val);    break;

				case 0xa0: machine().device<ay8910_device>("ay1")->data_w(space,0,m_b000_val);   break;
				case 0x88: machine().device<ay8910_device>("ay2")->data_w(space,0,m_b000_val);   break;
				case 0x81: machine().device<ay8910_device>("ay3")->data_w(space,0,m_b000_val);   break;

			}
		break;
	}

	m_b800_prev = data;
}

static ADDRESS_MAP_START( cpu_map, AS_PROGRAM, 8, ettrivia_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x9000, 0x9000) AM_WRITE(ettrivia_control_w)
	AM_RANGE(0x9800, 0x9800) AM_WRITENOP
	AM_RANGE(0xa000, 0xa000) AM_WRITENOP
	AM_RANGE(0xb000, 0xb000) AM_READ(b000_r) AM_WRITE(b000_w)
	AM_RANGE(0xb800, 0xb800) AM_WRITE(b800_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(ettrivia_fg_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(ettrivia_bg_w) AM_SHARE("bg_videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, ettrivia_state )
	AM_RANGE(0x0000, 0xffff) AM_READ(ettrivia_question_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( ettrivia )
	PORT_START("IN0")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2), RGN_FRAC(0,2) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( ettrivia )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,    0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout, 32*4, 32 )
GFXDECODE_END

void ettrivia_state::get_tile_info(tile_data &tileinfo, int tile_index, UINT8 *vidram, int gfx_code)
{
	int code = vidram[tile_index];
	int color = (code >> 5) + 8 * m_palreg;

	code += m_gfx_bank * 0x100;

	SET_TILE_INFO_MEMBER(gfx_code,code,color,0);
}

TILE_GET_INFO_MEMBER(ettrivia_state::get_tile_info_bg)
{
	get_tile_info(tileinfo, tile_index, m_bg_videoram, 0);
}

TILE_GET_INFO_MEMBER(ettrivia_state::get_tile_info_fg)
{
	get_tile_info(tileinfo, tile_index, m_fg_videoram, 1);
}

PALETTE_INIT_MEMBER(ettrivia_state, ettrivia)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances[2] = { 270, 130 };
	double weights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			2, resistances, weights, 0, 0,
			2, resistances, weights, 0, 0,
			0, 0, 0, 0, 0);

	for (i = 0;i < palette.entries(); i++)
	{
		int bit0, bit1;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i+0x100] >> 0) & 0x01;
		r = combine_2_weights(weights, bit0, bit1);

		/* green component */
		bit0 = (color_prom[i] >> 2) & 0x01;
		bit1 = (color_prom[i+0x100] >> 2) & 0x01;
		g = combine_2_weights(weights, bit0, bit1);

		/* blue component */
		bit0 = (color_prom[i] >> 1) & 0x01;
		bit1 = (color_prom[i+0x100] >> 1) & 0x01;
		b = combine_2_weights(weights, bit0, bit1);

		palette.set_pen_color(BITSWAP8(i,5,7,6,2,1,0,4,3), rgb_t(r, g, b));
	}
}

void ettrivia_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ettrivia_state::get_tile_info_bg),this),TILEMAP_SCAN_ROWS,8,8,64,32 );
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ettrivia_state::get_tile_info_fg),this),TILEMAP_SCAN_ROWS,8,8,64,32 );

	m_fg_tilemap->set_transparent_pen(0);
}

UINT32 ettrivia_state::screen_update_ettrivia(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

INTERRUPT_GEN_MEMBER(ettrivia_state::ettrivia_interrupt)
{
	if( ioport("COIN")->read() & 0x01 )
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	else
		device.execute().set_input_line(0, HOLD_LINE);
}

static MACHINE_CONFIG_START( ettrivia, ettrivia_state )
	MCFG_CPU_ADD("maincpu", Z80,12000000/4-48000) //should be ok, it gives the 300 interrupts expected
	MCFG_CPU_PROGRAM_MAP(cpu_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", ettrivia_state,  ettrivia_interrupt)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(ettrivia_state, screen_update_ettrivia)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ettrivia)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(ettrivia_state, ettrivia)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 1500000)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("IN1"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay3", AY8910, 1500000)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("IN0"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

ROM_START( promutrv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u16.u16",      0x0000, 0x8000, CRC(e37d48be) SHA1(1d700cff0c28e50fa2851e0c46de21aa47a23416) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mt44ic.44",    0x0000, 0x1000, CRC(8d543ea4) SHA1(86ab848a45851540d5d3315e15b92f7b2ac0b77c) )
	ROM_LOAD( "mt46ic.46",    0x1000, 0x1000, CRC(6d6e1f68) SHA1(e8196ecd915a2528122407d31a7078f177be0beb) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "mt48ic.48",    0x0000, 0x1000, CRC(f2efe300) SHA1(419e889b2f4d038ae64e3ccf4e2498add80b4c9f) )
	ROM_LOAD( "mt50ic.50",    0x1000, 0x1000, CRC(ee89d24e) SHA1(e3536df549278040255657201433ab23e0386533) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic64.prm",     0x0000, 0x0100, CRC(1cf9c914) SHA1(4c39b10c1be889d6ef4313b2112f4216d34f7327) ) /* palette low bits */
	ROM_LOAD( "ic63.prm",     0x0100, 0x0100, CRC(749da5a8) SHA1(8e30f5b014bc8ff2dc4986ef35a979e525681cb9) ) /* palette high bits */

	ROM_REGION( 0x40000, "user1", 0 ) /* Question roms */
	ROM_LOAD( "movie-tv.lo0", 0x00000, 0x8000, CRC(dbf03e62) SHA1(0210442ff80cce8fe39ba5e373bca0f47bb389c4) )
	ROM_LOAD( "movie-tv.hi0", 0x08000, 0x8000, CRC(77f09aab) SHA1(007ae0ec1f37b575412fa71c92d1891a62069089) )
	ROM_LOAD( "scifi.lo1",    0x10000, 0x8000, CRC(b5595f81) SHA1(5e7fa334f6541860a5c04e5f345673ea12efafb4) )
	ROM_LOAD( "enter3.hi1",   0x18000, 0x8000, CRC(a8cf603b) SHA1(6efa5753d8d252452b3f5be8635a28364e4d8de1) )
	ROM_LOAD( "sports3.lo2",  0x20000, 0x8000, CRC(bb28fa92) SHA1(a3c4c67be0e31793d68b0b048f3a73e9ce1d5859) )
	ROM_LOAD( "life-sci.hi2", 0x28000, 0x8000, CRC(975d48f4) SHA1(8c702da2178b1429b3c055a33917f44ca46aedb9) )
	ROM_LOAD( "wars.lo3",     0x30000, 0x8000, CRC(c437f9a8) SHA1(c625c46723e279474b52b05c4ec95f1df428505d) )
	ROM_LOAD( "soaps.hi3",    0x38000, 0x8000, CRC(9e20614d) SHA1(02121f2c17768763658e77bf19ccaae38a07e509) )
ROM_END

ROM_START( promutrva )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u16.u16",      0x0000, 0x8000, CRC(e37d48be) SHA1(1d700cff0c28e50fa2851e0c46de21aa47a23416) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mt44ic.44",    0x0000, 0x1000, CRC(8d543ea4) SHA1(86ab848a45851540d5d3315e15b92f7b2ac0b77c) )
	ROM_LOAD( "mt46ic.46",    0x1000, 0x1000, CRC(6d6e1f68) SHA1(e8196ecd915a2528122407d31a7078f177be0beb) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "mt48ic.48",    0x0000, 0x1000, CRC(f2efe300) SHA1(419e889b2f4d038ae64e3ccf4e2498add80b4c9f) )
	ROM_LOAD( "mt50ic.50",    0x1000, 0x1000, CRC(ee89d24e) SHA1(e3536df549278040255657201433ab23e0386533) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "ic64.prm",     0x0000, 0x0100, CRC(1cf9c914) SHA1(4c39b10c1be889d6ef4313b2112f4216d34f7327) ) /* palette low bits */
	ROM_LOAD( "ic63.prm",     0x0100, 0x0100, CRC(749da5a8) SHA1(8e30f5b014bc8ff2dc4986ef35a979e525681cb9) ) /* palette high bits */

	ROM_REGION( 0x40000, "user1", 0 ) /* Question roms */
	ROM_LOAD( "movie-tv.lo0", 0x00000, 0x8000, CRC(dbf03e62) SHA1(0210442ff80cce8fe39ba5e373bca0f47bb389c4) )
	ROM_LOAD( "movie-tv.hi0", 0x08000, 0x8000, CRC(77f09aab) SHA1(007ae0ec1f37b575412fa71c92d1891a62069089) )
	ROM_LOAD( "rock-pop.lo1", 0x10000, 0x8000, CRC(4252bc23) SHA1(d6c5b3c5f227b043f298cea585bcb934538b8880) )
	ROM_LOAD( "rock-pop.hi1", 0x18000, 0x8000, CRC(272aba66) SHA1(305866b07c6bb2d71ee169b0e8c75f95896d4484) )
	ROM_LOAD( "country.lo2",  0x20000, 0x8000, CRC(44673138) SHA1(4e5a3181300bd5f0e9336c2d0ddf900a9b4256d9) )
	ROM_LOAD( "country.hi2",  0x28000, 0x8000, CRC(3d35a612) SHA1(9d17477c8097b1110ed752caa6d280160368eac1) )
	ROM_LOAD( "sex.lo3",      0x30000, 0x8000, CRC(397b9c47) SHA1(bbb05f2ef22be0c099bb21139d21005039c61c31) )
	ROM_LOAD( "enter3.hi3",   0x38000, 0x8000, CRC(a8cf603b) SHA1(6efa5753d8d252452b3f5be8635a28364e4d8de1) )
ROM_END

ROM_START( promutrvb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u16.u16",      0x0000, 0x8000, CRC(e37d48be) SHA1(1d700cff0c28e50fa2851e0c46de21aa47a23416) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mt44.ic44",    0x0000, 0x1000, CRC(8d543ea4) SHA1(86ab848a45851540d5d3315e15b92f7b2ac0b77c) )
	ROM_LOAD( "mt46.ic46",    0x1000, 0x1000, CRC(6d6e1f68) SHA1(e8196ecd915a2528122407d31a7078f177be0beb) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "mt48.ic48",    0x0000, 0x1000, CRC(f2efe300) SHA1(419e889b2f4d038ae64e3ccf4e2498add80b4c9f) )
	ROM_LOAD( "mt50.ic50",    0x1000, 0x1000, CRC(ee89d24e) SHA1(e3536df549278040255657201433ab23e0386533) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "dm74s287n.ic64",     0x0000, 0x0100, CRC(1cf9c914) SHA1(4c39b10c1be889d6ef4313b2112f4216d34f7327) ) /* palette low bits */
	ROM_LOAD( "dm74s287n.ic63",     0x0100, 0x0100, CRC(749da5a8) SHA1(8e30f5b014bc8ff2dc4986ef35a979e525681cb9) ) /* palette high bits */

	ROM_REGION( 0x40000, "user1", 0 ) /* Question roms */
	ROM_LOAD( "movie-tv.lo0.u8", 0x00000, 0x8000, CRC(dbf03e62) SHA1(0210442ff80cce8fe39ba5e373bca0f47bb389c4) )
	ROM_LOAD( "movie-tv.hi0.u7", 0x08000, 0x8000, CRC(77f09aab) SHA1(007ae0ec1f37b575412fa71c92d1891a62069089) )
	ROM_LOAD( "rock-pop.lo1.u6", 0x10000, 0x8000, CRC(4252bc23) SHA1(d6c5b3c5f227b043f298cea585bcb934538b8880) )
	ROM_LOAD( "rock-pop.hi1.u5", 0x18000, 0x8000, CRC(272aba66) SHA1(305866b07c6bb2d71ee169b0e8c75f95896d4484) )
	ROM_LOAD( "country.lo2.u4",  0x20000, 0x8000, CRC(44673138) SHA1(4e5a3181300bd5f0e9336c2d0ddf900a9b4256d9) )
	ROM_LOAD( "country.hi2.u3",  0x28000, 0x8000, CRC(3d35a612) SHA1(9d17477c8097b1110ed752caa6d280160368eac1) )
	ROM_LOAD( "enter3.lo3.u2",   0x30000, 0x8000, CRC(a8cf603b) SHA1(6efa5753d8d252452b3f5be8635a28364e4d8de1) )
	ROM_LOAD( "geninfo.hi3.u1",  0x38000, 0x8000, CRC(2747fd74) SHA1(d34ac30349dc965ecd8b05b3f1cb7ee24627f369) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16l8a-ep-0.u10", 0x0000, 0x0104, CRC(ccbd5f41) SHA1(49e815dc3377b7ed4312c3c9c215c1a6fbce2769) )
	ROM_LOAD( "pal16l8a-ep-0.u9",  0x0200, 0x0104, CRC(180e95ad) SHA1(9c8dbe159aaf2595b9934fd4afff16b2e9ab584c) )
ROM_END

ROM_START( promutrvc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u16.u16",      0x0000, 0x8000, CRC(e37d48be) SHA1(1d700cff0c28e50fa2851e0c46de21aa47a23416) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mt44.ic44",    0x0000, 0x1000, CRC(8d543ea4) SHA1(86ab848a45851540d5d3315e15b92f7b2ac0b77c) )
	ROM_LOAD( "mt46.ic46",    0x1000, 0x1000, CRC(6d6e1f68) SHA1(e8196ecd915a2528122407d31a7078f177be0beb) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "mt48.ic48",    0x0000, 0x1000, CRC(f2efe300) SHA1(419e889b2f4d038ae64e3ccf4e2498add80b4c9f) )
	ROM_LOAD( "mt50.ic50",    0x1000, 0x1000, CRC(ee89d24e) SHA1(e3536df549278040255657201433ab23e0386533) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "dm74s287n.ic64",     0x0000, 0x0100, CRC(1cf9c914) SHA1(4c39b10c1be889d6ef4313b2112f4216d34f7327) ) /* palette low bits */
	ROM_LOAD( "dm74s287n.ic63",     0x0100, 0x0100, CRC(749da5a8) SHA1(8e30f5b014bc8ff2dc4986ef35a979e525681cb9) ) /* palette high bits */

	ROM_REGION( 0x40000, "user1", 0 ) /* Question roms */
	ROM_LOAD( "sports.lo0.u8",   0x00000, 0x8000, CRC(bb28fa92) SHA1(a3c4c67be0e31793d68b0b048f3a73e9ce1d5859) )
	ROM_LOAD( "sports2.hi0.u7",  0x08000, 0x8000, CRC(4d0107d7) SHA1(4cbef1bc5faaca52ce6bb490560f213d60a96191) )
	ROM_LOAD( "expert.lo1.u6",   0x10000, 0x8000, CRC(19153d1a) SHA1(a2f2bbabbd1c68aae58ff29a43cb02b0e8867f5a) )
	ROM_LOAD( "potpouri.hi1.u5", 0x18000, 0x8000, CRC(cbfa6491) SHA1(74120ea6b3678d54737c37ec3b4b309c346c460e) )
	ROM_LOAD( "country.lo2.u4",  0x20000, 0x8000, CRC(44673138) SHA1(4e5a3181300bd5f0e9336c2d0ddf900a9b4256d9) )
	ROM_LOAD( "country.hi2.u3",  0x28000, 0x8000, CRC(3d35a612) SHA1(9d17477c8097b1110ed752caa6d280160368eac1) )
	ROM_LOAD( "sex3.lo3.u2",     0x30000, 0x8000, CRC(1a2322be) SHA1(22f930dc29e2b9a2c5fbf16479bc213e94df5620) )
	ROM_LOAD( "geninfo.hi3.u1",  0x38000, 0x8000, CRC(2747fd74) SHA1(d34ac30349dc965ecd8b05b3f1cb7ee24627f369) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16l8a-ep-0.u10", 0x0000, 0x0104, CRC(ccbd5f41) SHA1(49e815dc3377b7ed4312c3c9c215c1a6fbce2769) )
	ROM_LOAD( "pal16l8a-ep-0.u9",  0x0200, 0x0104, CRC(180e95ad) SHA1(9c8dbe159aaf2595b9934fd4afff16b2e9ab584c) )
ROM_END

ROM_START( strvmstr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stm16.u16",    0x0000, 0x8000, CRC(ae734db9) SHA1(1bacdfdebaa1f250bfbd49053c3910f1396afe11) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "stm44.rom",    0x0000, 0x1000, CRC(e69da710) SHA1(218a9d7600d67858d1f21282a0cebec0ae93e0ff) )
	ROM_LOAD( "stm46.rom",    0x1000, 0x1000, CRC(d927a1f1) SHA1(63a49a61107deaf7a9f28b9653c310c5331f5143) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "stm48.rom",    0x0000, 0x1000, CRC(51719714) SHA1(fdecbd22ea65eec7b4b5138f89ddc5876b05def6) )
	ROM_LOAD( "stm50.rom",    0x1000, 0x1000, CRC(cfc1a1d1) SHA1(9ef38f12360dd946651e67770742ca72fa6846f1) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "stm64.prm",    0x0000, 0x0100, BAD_DUMP CRC(69ebc0b8) SHA1(de2b936e3246e3bfc7e2ff9546c1854ec3504cc2) ) /* palette low bits */
	ROM_LOAD( "stm63.prm",    0x0100, 0x0100, BAD_DUMP CRC(305271cf) SHA1(6fd5fe085d79ca7aa57010cffbdb2a85b9c24701) ) /* palette high bits */

	ROM_REGION( 0x40000, "user1", 0 ) /* Question roms */
	ROM_LOAD( "sex2.lo0",     0x00000, 0x8000, CRC(9c68b277) SHA1(34bc9d7b973fe482abd5e34a058b72eb5ec8db64) )
	ROM_LOAD( "sports.hi0",   0x08000, 0x8000, CRC(3678fb79) SHA1(4e40cc20707195c0e88e595f752a2982b531b57e) )
	ROM_LOAD( "movies.lo1",   0x10000, 0x8000, CRC(16cba1b7) SHA1(8aa3eff72d1ec8dac906f2e803a88578a9fe763c) )
	ROM_LOAD( "rock-pop.hi1", 0x18000, 0x8000, CRC(e2954db6) SHA1(d545236a844b63c85937ee8fb8e65bcd74b1bf43) )
	ROM_LOAD( "sci-fi.lo2",   0x20000, 0x8000, CRC(b5595f81) SHA1(5e7fa334f6541860a5c04e5f345673ea12efafb4) )
	ROM_LOAD( "cars.hi2",     0x28000, 0x8000, CRC(50310557) SHA1(7559c603625e4df442b440b8b08e6efef06e2781) )
	ROM_LOAD( "potprri.lo3",  0x30000, 0x8000, CRC(427eada9) SHA1(bac29ec637a17db95507c68fd73a8ce52744bf8e) )
	ROM_LOAD( "entrtn.hi3",   0x38000, 0x8000, CRC(a8cf603b) SHA1(6efa5753d8d252452b3f5be8635a28364e4d8de1) )
ROM_END

GAME( 1985, promutrv, 0,        ettrivia, ettrivia, driver_device, 0, ROT270, "Enerdyne Technologies Inc.", "Progressive Music Trivia (Question set 1)", 0 )
GAME( 1985, promutrva,promutrv, ettrivia, ettrivia, driver_device, 0, ROT270, "Enerdyne Technologies Inc.", "Progressive Music Trivia (Question set 2)", 0 )
GAME( 1985, promutrvb,promutrv, ettrivia, ettrivia, driver_device, 0, ROT270, "Enerdyne Technologies Inc.", "Progressive Music Trivia (Question set 3)", 0 )
GAME( 1985, promutrvc,promutrv, ettrivia, ettrivia, driver_device, 0, ROT270, "Enerdyne Technologies Inc.", "Progressive Music Trivia (Question set 4)", 0 )
GAME( 1986, strvmstr, 0,        ettrivia, ettrivia, driver_device, 0, ROT270, "Enerdyne Technologies Inc.", "Super Trivia Master", MACHINE_WRONG_COLORS )
