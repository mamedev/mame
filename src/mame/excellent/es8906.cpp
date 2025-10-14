// license:BSD-3-Clause
// copyright-holders:

/*
Excellent System's ES-8906B PCB

Main components:
R6502AP CPU
6116ALSP-12 RAM (near CPU)
20.0000 MHz XTAL (near CPU)
HD46505SP CRTC
Excellent ES-8712 custom
2x SN76489A (probably, chips are partially covered in the pic) with a 104K yellow capacitor each
unmarked 22 (?) pin chip with a 104K yellow capacitor
4x 6116ALSP-12 RAM (near ES-8712)
4x bank of 8 DIP switches (SW2-5)
reset button (SW1)
NE555P (near SW1)

TODO:
* colors
* complete inputs
* outputs
* use CRTC device for drawing routines
* is sound complete? what's the unmarked 22 (?) pin chip?
*/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "sound/es8712.h"
#include "sound/sn76496.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class es8906_state : public driver_device
{
public:
	es8906_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_tileram(*this, "tileram%u", 0U),
		m_attrram(*this, "attrram%u", 0U)
	{ }

	void es8906(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr_array<uint8_t, 2> m_tileram;
	required_shared_ptr_array<uint8_t, 2> m_attrram;

	tilemap_t *m_tilemap[2]{};

	template <uint8_t Which> void tileram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void attrram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


void es8906_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(es8906_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(es8906_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[0]->set_transparent_pen(0);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(es8906_state::get_tile_info)
{
	int const tile = m_tileram[Which][tile_index] | ((m_attrram[Which][tile_index] & 0x0f) << 8);
	int const color = (m_attrram[Which][tile_index] & 0xf0) >> 4;

	tileinfo.set(Which, tile, color, 0);
}

template <uint8_t Which>
void es8906_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[Which][offset] = data;
	m_tilemap[Which]->mark_tile_dirty(offset);
}

template <uint8_t Which>
void es8906_state::attrram_w(offs_t offset, uint8_t data)
{
	m_attrram[Which][offset] = data;
	m_tilemap[Which]->mark_tile_dirty(offset);
}

uint32_t es8906_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void es8906_state::program_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0800).w("crtc", FUNC(hd6845s_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0x1000, 0x17ff).ram().w(FUNC(es8906_state::tileram_w<0>)).share(m_tileram[0]);
	map(0x1800, 0x1fff).ram().w(FUNC(es8906_state::attrram_w<0>)).share(m_attrram[0]);
	map(0x2000, 0x27ff).ram().w(FUNC(es8906_state::tileram_w<1>)).share(m_tileram[1]);
	map(0x2800, 0x2fff).ram().w(FUNC(es8906_state::attrram_w<1>)).share(m_attrram[1]);
	map(0x3000, 0x3000).portr("IN1");
	map(0x3010, 0x3010).portr("IN2");
	map(0x3020, 0x3020).portr("IN3");
	map(0x3030, 0x3030).portr("IN4");
	map(0x3040, 0x3040).portr("IN5");
	map(0x3800, 0x3800).portr("SW2").w("sn1", FUNC(sn76489a_device::write));
	map(0x3801, 0x3801).portr("SW3");
	map(0x3802, 0x3802).portr("SW4");
	map(0x3803, 0x3803).portr("SW5");
	map(0x3810, 0x3810).w("sn2", FUNC(sn76489a_device::write));
	map(0x4000, 0xffff).rom();
}


static INPUT_PORTS_START( dream9 ) // TODO: inputs are very incomplete
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Show Rules") // press Start to return to game
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Bet 2") // ??
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE(0x40, IP_ACTIVE_LOW)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )

	// DIP definitions from test mode, not verified if the actually work in game (coinage seems not)
	// defaults unknown, set to all off for now
	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Switch" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x0c, 0x0c, "Credit Limit" ) PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x0c, "1000" )
	PORT_DIPSETTING(    0x08, "5000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4") // effect not shown in test mode
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3") // effect not shown in test mode
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2") // effect not shown in test mode
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1") // effect not shown in test mode
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW3")
	PORT_DIPNAME( 0x01, 0x01, "W-Up Type" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x06, 0x06, "W-Up" ) PORT_DIPLOCATION("SW3:7,6")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x18, 0x18, "Maximum Bet" ) PORT_DIPLOCATION("SW3:5,4")
	PORT_DIPSETTING(    0x18, "8" )
	PORT_DIPSETTING(    0x10, "16" )
	PORT_DIPSETTING(    0x08, "32" )
	PORT_DIPSETTING(    0x00, "64" )
	PORT_DIPNAME( 0xe0, 0xe0, "Rate Of Win" ) PORT_DIPLOCATION("SW3:3,2,1")
	PORT_DIPSETTING(    0x00, "60%" )
	PORT_DIPSETTING(    0x20, "65%" )
	PORT_DIPSETTING(    0x40, "70%" )
	PORT_DIPSETTING(    0x60, "75%" )
	PORT_DIPSETTING(    0x80, "80%" )
	PORT_DIPSETTING(    0xa0, "85%" )
	PORT_DIPSETTING(    0xc0, "90%" )
	PORT_DIPSETTING(    0xe0, "95%" )

	PORT_START("SW4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:8") // effect not shown in test mode
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW4:7,6,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 10C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_50C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW4:4,3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 10C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 5C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_20C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_25C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_50C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_100C ) )

	PORT_START("SW5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW5:8") // effect not shown in test mode
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW5:7") // effect not shown in test mode
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, "Key In" ) PORT_DIPLOCATION("SW5:6,5,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_20C ) )
	PORT_DIPSETTING(    0x08, "1 Coin/40 Credits" )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_50C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_100C ) )
	PORT_DIPSETTING(    0x14, "1 Coin/200 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/250 Credits" )
	PORT_DIPSETTING(    0x1c, "1 Coin/500 Credits" )
	PORT_DIPNAME( 0xe0, 0xe0, "Coin C" ) PORT_DIPLOCATION("SW5:3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_20C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_25C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_50C ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx_es8906 )
	GFXDECODE_ENTRY( "tiles1", 0, gfx_8x8x4_planar, 0, 16 )
	GFXDECODE_ENTRY( "tiles2", 0, gfx_8x8x4_planar, 0, 16 )
GFXDECODE_END


void es8906_state::es8906(machine_config &config)
{
	M6502(config, m_maincpu, 20_MHz_XTAL / 10); // TODO: divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &es8906_state::program_map);

	hd6845s_device &crtc(HD6845S(config, "crtc", 20_MHz_XTAL / 10)); // TODO: verify
	crtc.set_screen(m_screen);
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, 0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // TODO: everything
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(es8906_state::screen_update));
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_es8906);
	PALETTE(config, "palette", palette_device::RGB_444_PROMS, "proms", 256); // TODO

	SPEAKER(config, "speaker").front_center();

	ES8712(config, "essnd", 0);

	SN76489A(config, "sn1", 20_MHz_XTAL / 10).add_route(ALL_OUTPUTS, "speaker", 0.50); // TODO: divider not verified

	SN76489A(config, "sn2", 20_MHz_XTAL / 10).add_route(ALL_OUTPUTS, "speaker", 0.50); // TODO: divider not verified
}


ROM_START( dream9 ) // (C)1989 EXCELLENT SYSTEM SUPER 9 Ver1.52 PROGRAM by KAY/AMTECH Feb,6 1990, but pics show "Dream 9"
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d5-00_excellent.3h", 0x00000, 0x10000, CRC(7c0ad390) SHA1(b173f7d4521a4bea4246b988e6b4fce179ac12bc) ) // 0x0000 - 0x3fff and 0x4000 - 0x7fff are identical

	ROM_REGION( 0x20000, "tiles1", 0 ) // all labels have "エクセレント システム" (means "Excellent System") before what's below
	ROM_LOAD( "9l.9l",   0x00000, 0x10000, CRC(63692c62) SHA1(ef06ffe4204b0c1e40f9685b62f403b94a9b6693) )
	ROM_LOAD( "11l.11l", 0x10000, 0x10000, CRC(9251d1a6) SHA1(ccce3240d825e7611a4e4412b4f37710d0957179) )

	ROM_REGION( 0x20000, "tiles2", 0 ) // idem
	ROM_LOAD( "12l.12l", 0x00000, 0x10000, CRC(25395909) SHA1(d5e8caac642847450896a855e293431ad6620a77) )
	ROM_LOAD( "14l.14l", 0x10000, 0x10000, CRC(45757954) SHA1(bda96f437e5b65058e2f068b73998bad4b0bd5a3) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "r.11b", 0x0000, 0x0100, CRC(b41df116) SHA1(5cae3e6c70775ad63bccbfc4847b4feee5b79ddf) )
	ROM_LOAD( "g.12b", 0x0100, 0x0100, CRC(70ac57cd) SHA1(fe238c363f09f20cdc1c6f7f6279201671664750) )
	ROM_LOAD( "b.13b", 0x0200, 0x0100, CRC(99458711) SHA1(0be119f928b9085e36056b9b6b7e8623765eef44) )
ROM_END

} // anonymous namespace


GAME( 1990, dream9, 0, es8906, dream9, es8906_state, empty_init, ROT0, "Excellent System", "Dream 9 (v1.52)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
