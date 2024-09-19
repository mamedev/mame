// license:BSD-3-Clause
// copyright-holders:Luca Elia,Roberto Fresca
/***************************************************************************

  Galaxi (C)2000 B.R.L.

  Driver by Luca Elia.
  Additional work by Roberto Fresca.


  Hardware info (29/07/2008 f205v):

  Chips:
    1x missing main CPU (u1)(from the socket I would say it's a 68000)
    1x A40MX04-PL84 (u29)
    1x AD-65 (equivalent to M6295) (u9)(sound)
    1x MC1458P (u10)(sound)
    1x TDA2003 (u8)(sound)
    1x oscillator 10.000MHz (QZ1)
    1x oscillator 16.000000 (QZ2)
  ROMs:
    1x AT27C020 (1)
    2x M27C4001 (2,3)
    2x AT49F010 (4,5)
    2x DS1230Y (non volatile SRAM)
  Notes:
    1x 28x2 edge connector
    1x trimmer (volume)

  - This hardware is almost identical to that in magic10.cpp

  CPU is a MC68000P10, from the other games boards...

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "galaxi.lh"


namespace {

#define CPU_CLOCK       (XTAL(10'000'000))
#define SND_CLOCK       (XTAL(16'000'000))/16

class galaxi_state : public driver_device
{
public:
	galaxi_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bg1_ram(*this, "bg1_ram"),
		m_bg2_ram(*this, "bg2_ram"),
		m_bg3_ram(*this, "bg3_ram"),
		m_bg4_ram(*this, "bg4_ram"),
		m_fg_ram(*this, "fg_ram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_lamps(*this, "lamp%u", 1U),
		m_bg3_xscroll(8),
		m_bg3_yscroll(0)
	{ }

	void galaxi(machine_config &config);
	void lastfour(machine_config &config);
	void magjoker(machine_config &config);

	int ticket_r();
	int hopper_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_bg1_ram;
	required_shared_ptr<uint16_t> m_bg2_ram;
	required_shared_ptr<uint16_t> m_bg3_ram;
	required_shared_ptr<uint16_t> m_bg4_ram;
	required_shared_ptr<uint16_t> m_fg_ram;
//  uint16_t *  m_nvram;        // currently this uses generic nvram handling

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	output_finder<6> m_lamps;

	/* video-related */
	tilemap_t   *m_bg1_tmap = nullptr;
	tilemap_t   *m_bg2_tmap = nullptr;
	tilemap_t   *m_bg3_tmap = nullptr;
	tilemap_t   *m_bg4_tmap = nullptr;
	tilemap_t   *m_fg_tmap = nullptr;

	uint16_t m_bg3_xscroll;
	uint16_t m_bg3_yscroll;

	/* misc */
	int       m_hopper = 0;
	int       m_ticket = 0;
	uint16_t    m_out = 0;

	void bg1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bg2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bg3_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bg4_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void _500000_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void _500002_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void _500004_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_bg3_tile_info);
	TILE_GET_INFO_MEMBER(get_bg4_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void show_out();

	void galaxi_map(address_map &map) ATTR_COLD;
	void lastfour_map(address_map &map) ATTR_COLD;
};


/***************************************************************************
                                Video Hardware
***************************************************************************/

TILE_GET_INFO_MEMBER(galaxi_state::get_bg1_tile_info)
{
	uint16_t code = m_bg1_ram[tile_index];
	tileinfo.set(0, code, 0x10 + (code >> 12), 0);
}

TILE_GET_INFO_MEMBER(galaxi_state::get_bg2_tile_info)
{
	uint16_t code = m_bg2_ram[tile_index];
	tileinfo.set(0, code, 0x10 + (code >> 12), 0);
}

TILE_GET_INFO_MEMBER(galaxi_state::get_bg3_tile_info)
{
	uint16_t code = m_bg3_ram[tile_index];
	tileinfo.set(0, code, (code >> 12), 0);
}

TILE_GET_INFO_MEMBER(galaxi_state::get_bg4_tile_info)
{
	uint16_t code = m_bg4_ram[tile_index];
	tileinfo.set(0, code, (code >> 12), 0);
}

TILE_GET_INFO_MEMBER(galaxi_state::get_fg_tile_info)
{
	uint16_t code = m_fg_ram[tile_index];
	tileinfo.set(1, code, 0x20 + (code >> 12), 0);
}

void galaxi_state::bg1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg1_ram[offset]);
	m_bg1_tmap->mark_tile_dirty(offset);
}

void galaxi_state::bg2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg2_ram[offset]);
	m_bg2_tmap->mark_tile_dirty(offset);
}

void galaxi_state::bg3_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg3_ram[offset]);
	m_bg3_tmap->mark_tile_dirty(offset);
}

void galaxi_state::bg4_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg4_ram[offset]);
	m_bg4_tmap->mark_tile_dirty(offset);
}

void galaxi_state::fg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg_ram[offset]);
	m_fg_tmap->mark_tile_dirty(offset);
}

void galaxi_state::video_start()
{
	m_bg1_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxi_state::get_bg1_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 0x20, 0x10);
	m_bg2_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxi_state::get_bg2_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 0x20, 0x10);
	m_bg3_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxi_state::get_bg3_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 0x20, 0x10);
	m_bg4_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxi_state::get_bg4_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 0x20, 0x10);

	m_fg_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxi_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 0x40, 0x20);

	m_bg1_tmap->set_transparent_pen(0);
	m_bg2_tmap->set_transparent_pen(0);
	m_bg3_tmap->set_transparent_pen(0);
	m_bg4_tmap->set_transparent_pen(0);

	m_fg_tmap->set_transparent_pen(0);
}

uint32_t galaxi_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg3_tmap->set_scrollx(m_bg3_xscroll);
	m_bg3_tmap->set_scrolly(m_bg3_yscroll);

	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_R))  // remapped due to inputs changes.
	{
		int msk = 0;
		if (machine().input().code_pressed(KEYCODE_T))  msk |= 1;
		if (machine().input().code_pressed(KEYCODE_Y))  msk |= 2;
		if (machine().input().code_pressed(KEYCODE_U))  msk |= 4;
		if (machine().input().code_pressed(KEYCODE_I))  msk |= 8;
		if (machine().input().code_pressed(KEYCODE_O))  msk |= 16;
		if (msk != 0) layers_ctrl &= msk;
	}
#endif


	if (layers_ctrl & 1)    m_bg1_tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	else                bitmap.fill(m_palette->black_pen(), cliprect);
	if (layers_ctrl & 2)    m_bg2_tmap->draw(screen, bitmap, cliprect, 0, 0);
	if (layers_ctrl & 4)    m_bg3_tmap->draw(screen, bitmap, cliprect, 0, 0);
	if (layers_ctrl & 8)    m_bg4_tmap->draw(screen, bitmap, cliprect, 0, 0);

	if (layers_ctrl & 16)   m_fg_tmap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

/***************************************************************************
                               Handlers
***************************************************************************/

void galaxi_state::show_out(  )
{
//  popmessage("%04x", m_out);
}

void galaxi_state::_500000_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg3_yscroll);
	show_out();
}

void galaxi_state::_500002_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg3_xscroll);
	show_out();
}

void galaxi_state::_500004_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
	/*
	    - Lbits -
	    7654 3210
	    =========
	    ---- ---x  Hold1 lamp.
	    ---- --x-  Hold2 lamp.
	    ---- -x--  Hold3 lamp.
	    ---- x---  Hold4 lamp.
	    ---x ----  Hold5 lamp.
	    --x- ----  Start lamp.
	    -x-- ----  Payout.

	*/
		for (int n = 0; n < 6; n++)
			m_lamps[n] = BIT(data, n);
	}
	if (ACCESSING_BITS_8_15)
	{
		m_ticket = data & 0x0100;
		m_hopper = data & 0x1000;
		machine().bookkeeping().coin_counter_w(0, data & 0x2000);    // coins
	}

	COMBINE_DATA(&m_out);
	show_out();
}

int galaxi_state::ticket_r()
{
	return m_ticket && !(m_screen->frame_number() % 10);
}

int galaxi_state::hopper_r()
{
	return m_hopper && !(m_screen->frame_number() % 10);
}


/***************************************************************************
                            Memory Maps
***************************************************************************/

void galaxi_state::galaxi_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();

	map(0x100000, 0x1003ff).ram().w(FUNC(galaxi_state::bg1_w)).share("bg1_ram");
	map(0x100400, 0x1007ff).ram().w(FUNC(galaxi_state::bg2_w)).share("bg2_ram");
	map(0x100800, 0x100bff).ram().w(FUNC(galaxi_state::bg3_w)).share("bg3_ram");
	map(0x100c00, 0x100fff).ram().w(FUNC(galaxi_state::bg4_w)).share("bg4_ram");

	map(0x101000, 0x101fff).ram().w(FUNC(galaxi_state::fg_w)).share("fg_ram");
	map(0x102000, 0x107fff).nopr(); // unknown

	map(0x300000, 0x3007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0x500000, 0x500001).portr("INPUTS");
	map(0x500000, 0x500001).w(FUNC(galaxi_state::_500000_w));
	map(0x500002, 0x500003).w(FUNC(galaxi_state::_500002_w));
	map(0x500004, 0x500005).w(FUNC(galaxi_state::_500004_w));

	map(0x700001, 0x700001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x600000, 0x607fff).ram().share("nvram");   // 2x DS1230Y (non volatile SRAM)
}


void galaxi_state::lastfour_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();

	// bg3+4 / 1+2 seem to be swapped, order, palettes, scroll register etc. all suggest this
	map(0x100000, 0x1003ff).ram().w(FUNC(galaxi_state::bg3_w)).share("bg3_ram");
	map(0x100400, 0x1007ff).ram().w(FUNC(galaxi_state::bg4_w)).share("bg4_ram");
	map(0x100800, 0x100bff).ram().w(FUNC(galaxi_state::bg1_w)).share("bg1_ram");
	map(0x100c00, 0x100fff).ram().w(FUNC(galaxi_state::bg2_w)).share("bg2_ram");

	map(0x101000, 0x101fff).ram().w(FUNC(galaxi_state::fg_w)).share("fg_ram");
	map(0x102000, 0x107fff).nopr(); // unknown

	map(0x300000, 0x3007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0x500000, 0x500001).portr("INPUTS");
	map(0x500000, 0x500001).w(FUNC(galaxi_state::_500000_w));
	map(0x500002, 0x500003).w(FUNC(galaxi_state::_500002_w));
	map(0x500004, 0x500005).w(FUNC(galaxi_state::_500004_w));

	map(0x700001, 0x700001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x600000, 0x607fff).ram().share("nvram");   // 2x DS1230Y (non volatile SRAM)
}


/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( galaxi )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(galaxi_state, hopper_r)   // hopper sensor
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(5)   // coin a
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(5)   // coin b (token)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN3 )   // pin 25LC
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(galaxi_state, ticket_r)  // ticket sensor
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_CUSTOM ) // hopper out (pin 14LS)
	PORT_SERVICE_NO_TOGGLE( 0x2000, IP_ACTIVE_HIGH )    // test
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_CUSTOM ) // (pin 26LC)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) // (pin 15LS)
INPUT_PORTS_END

static INPUT_PORTS_START( magjoker )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(galaxi_state, hopper_r)   // hopper sensor
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(5)   // coin a
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(5)   // coin b (token)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Hopper Refill") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(galaxi_state, ticket_r)  // ticket sensor
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_CUSTOM ) // hopper out (pin 14LS)
	PORT_SERVICE_NO_TOGGLE( 0x2000, IP_ACTIVE_HIGH )    // test
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT )   // (pin 26LC)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) // (pin 15LS)
INPUT_PORTS_END


/***************************************************************************
                      Graphics Layout & Graphics Decode
***************************************************************************/

static const gfx_layout layout_8x8x4 =
{
	8, 8,
	0x1000, // 0x1000 tiles are accessible
	4,
	{ STEP4(0,1) },
	{ STEP4(4*4,4), STEP4(0,4) },
	{ STEP8(0,4*8) },
	8*8*4
};

static const gfx_layout layout_16x16x4 =
{
	16, 16,
	0x1000, // 0x1000 tiles are accessible
	4,
	{ STEP4(0,1) },
	{ STEP4(4*4,4), STEP4(0,4), STEP4(4*4+8*16*4,4), STEP4(0+8*16*4,4) },
	{ STEP16(0,4*8) },
	16*16*4
};

static GFXDECODE_START( gfx_galaxi )
	GFXDECODE_ENTRY( "gfx1", 0x00000, layout_16x16x4, 0, 0x400/0x10 )
	GFXDECODE_ENTRY( "gfx1", 0x80000, layout_8x8x4,   0, 0x400/0x10 )
GFXDECODE_END


/***************************************************************************
                           Machine Start & Reset
***************************************************************************/

void galaxi_state::machine_start()
{
	save_item(NAME(m_hopper));
	save_item(NAME(m_ticket));
	save_item(NAME(m_out));

	m_lamps.resolve();
}

void galaxi_state::machine_reset()
{
	m_hopper = 0;
	m_ticket = 0;
	m_out = 0;
}

/***************************************************************************
                              Machine Drivers
***************************************************************************/

void galaxi_state::galaxi(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxi_state::galaxi_map);
	m_maincpu->set_vblank_int("screen", FUNC(galaxi_state::irq4_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(16*5, 512-16*2-1, 16*1, 256-1);
	m_screen->set_screen_update(FUNC(galaxi_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_galaxi);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x400);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", SND_CLOCK, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 1.0);  // ?
}


void galaxi_state::magjoker(machine_config &config)
{
	galaxi(config);

	/* sound hardware */

	/* ADPCM samples are recorded with extremely low volume */
	subdevice<okim6295_device>("oki")->reset_routes();
	subdevice<okim6295_device>("oki")->add_route(ALL_OUTPUTS, "mono", 4.0);
}


void galaxi_state::lastfour(machine_config &config)
{
	galaxi(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxi_state::lastfour_map);
}


/***************************************************************************
                                ROMs Loading
***************************************************************************/

ROM_START( galaxi )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "5.u48", 0x00000, 0x20000, CRC(53d86ed0) SHA1(d04ad4c79b0ae46d3d5820b16481ea95c1370e6d) )
	ROM_LOAD16_BYTE( "4.u47", 0x00001, 0x20000, CRC(ddd67683) SHA1(68f8969949e1db90a765c1f31cb8957eef505d5f) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "3.u34", 0x00000, 0x80000, CRC(4a59ad63) SHA1(34fc1a948fc205f8c55a8e99d143bbdf4d1b220f) )
	ROM_LOAD16_BYTE( "2.u33", 0x00001, 0x80000, CRC(a8b29a97) SHA1(835c6885d5adf0e7600810ad9fcda88c22077495) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "1.u38", 0x00000, 0x40000, CRC(50e289db) SHA1(43c576c014f4c3d22bfa4c932e161d7558d483f6) )
ROM_END

ROM_START( magjoker )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "25.u48", 0x00000, 0x20000, CRC(505bdef2) SHA1(9c2a525f2eb3cc39bdd6219bad7c5a1a8bc0b274) )
	ROM_LOAD16_BYTE( "24.u47", 0x00001, 0x20000, CRC(380fd0cd) SHA1(bcd6d23e41e249c7e587b253958eec180440639a) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "23.u34", 0x00000, 0x80000, CRC(952b7c84) SHA1(a28e1b79444331837ffc07c8d3c16c1d9a3c974c) )
	ROM_LOAD16_BYTE( "22.u33", 0x00001, 0x80000, CRC(41866733) SHA1(257d77f89fcf1e8f36fb6a8fcb8ad48b1127e457) )

	ROM_REGION( 0x40000, "oki", 0 ) /* 4-bit ADPCM mono @ 6 kHz.*/
	ROM_LOAD( "21.u38", 0x00000, 0x40000, CRC(199baf33) SHA1(006708d955481fe1ae44555d27896d18e1ff8440) )
ROM_END

/*
  Last Four
  2001, BRL.

  CPUs:
  1x MC68000P10 (u1) 16/32-bit Microprocessor (main).
  1x AD-65      (u9) 4-Channel Mixing ADCPM Voice Synthesis LSI (sound).
  1x MC1458     (u10) Dual Operational Amplifier (sound).
  1x TDA2003    (u8) Audio Amplifier (sound).
  1x 10.000000MHz. oscillator (QZ1).
  1x 16.000MHz. oscillator (QZ2).

  ROMs:
  1x AF49F010 (14) dumped.
  1x Am27C010 (15) dumped.
  1x MC27C100 (21) dumped.
  2x M27C4001 (12, 13) dumped.

  RAMs:
  2x KT76C28K-10 (u16, u17).
  2x HT6264-70 (u26, u27).
  2x DS1230Y-100 (u4, u5) not dumped.

  PLDs:
  1x A40MX04-PL84 (u29) read protected.

  Others:
  1x 28x2 JAMMA edge connector.
  1x trimmer (volume).

  Notes:
  PCB is etched: GALAXI - 26 05 2000 BRL S.R.L. BOLOGNA.

*/
ROM_START( lastfour )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "15.u48", 0x00000, 0x20000, CRC(9168e19c) SHA1(4a2f0d100e457bd33691ba084a0f0549e8bf0790) )
	ROM_LOAD16_BYTE( "14.u47", 0x00001, 0x20000, CRC(b10ce31a) SHA1(8d51ead24319ff775fc873957e6b4de748432a8d) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "13.u34", 0x00000, 0x80000, CRC(d595d4c4) SHA1(7fe8c9f36b03d763965abf325d1ff6d754342100) )
	ROM_LOAD16_BYTE( "12.u33", 0x00001, 0x80000, CRC(5ee5568b) SHA1(6384e5dfa24b5ad4e4419fa3bbffb4d552867465) )

	ROM_REGION( 0x40000, "oki", 0 ) /* 4-bit ADPCM mono @ 6 kHz.*/
	ROM_LOAD( "21.u38", 0x00000, 0x20000, CRC(e48523dd) SHA1(47bc2e5c2164b93d685fa134397845e0ed7aaa5f) )
ROM_END

} // anonymous namespace


/***************************************************************************
                               Game Drivers
***************************************************************************/

//     YEAR  NAME      PARENT  MACHINE   INPUT     CLASS         INIT        ROT   COMPANY   FULLNAME                        FLAGS                   LAYOUT
GAMEL( 2000, galaxi,   0,      galaxi,   galaxi,   galaxi_state, empty_init, ROT0, "B.R.L.", "Galaxi (v2.0)",                MACHINE_SUPPORTS_SAVE,  layout_galaxi )
GAMEL( 2000, magjoker, 0,      magjoker, magjoker, galaxi_state, empty_init, ROT0, "B.R.L.", "Magic Joker (v1.25.10.2000)",  MACHINE_SUPPORTS_SAVE,  layout_galaxi )
GAMEL( 2001, lastfour, 0,      lastfour, magjoker, galaxi_state, empty_init, ROT0, "B.R.L.", "Last Four (09:12 16/01/2001)", MACHINE_SUPPORTS_SAVE,  layout_galaxi )
