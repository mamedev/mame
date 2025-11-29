// license:BSD-3-Clause
// copyright-holders:

/*
Fun Tech H8-based hardware

PCB 183-A2, G-EGG, Copyright@ FUN TECH CO.
S/N -

scratched off 80-pin square chip - identified as a H8-303X series chip (thanks OG)
12 MHz XTAL
scratched off 48-pin square CPLD
7X 6264 SRAM
61256 SRAM
4x bank of 8 DIP switches
AD-65 (Oki M6295 clone) sound chip
TDA 1519 sound amplifier

TODO:
- unknown reads / writes;
- reels alignment in most screens;
- only accepts coins in test mode;
- NVRAM;
- outputs (counters, lamps..);
- does the H8 really run so slow or is there something else in play?
*/


#include "emu.h"

#include "cpu/h8/h83032.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class funtech_h8_state : public driver_device
{
public:
	funtech_h8_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_hopper(*this, "hopper"),
		m_gfxdecode(*this, "gfxdecode"),
		m_tileram(*this, "tileram", 0x800U, ENDIANNESS_BIG),
		m_attrram(*this, "attrram", 0x800U, ENDIANNESS_BIG),
		m_reel_tileram(*this, "reel_tileram%u", 0U, 0x200U, ENDIANNESS_BIG),
		m_reel_attrram(*this, "reel_attrram%u", 0U, 0x200U, ENDIANNESS_BIG)
	{ }

	void funtech_h8(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<h83030_device> m_maincpu;
	required_device<hopper_device> m_hopper;
	required_device<gfxdecode_device> m_gfxdecode;

	memory_share_creator<uint8_t> m_tileram;
	memory_share_creator<uint8_t> m_attrram;
	memory_share_array_creator<uint8_t, 4> m_reel_tileram;
	memory_share_array_creator<uint8_t, 4> m_reel_attrram;

	tilemap_t *m_tilemap = nullptr;
	tilemap_t *m_reel_tilemap[4] {};
	uint8_t m_tilebank = 0;

	TILE_GET_INFO_MEMBER(get_tile_info);
	void tileram_w(offs_t offset, uint8_t data);
	void attrram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_reel_tile_info);
	template <uint8_t Which> void reel_tileram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void reel_attrram_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void hopper_w(uint16_t data);

	void program_map(address_map &map) ATTR_COLD;
};


void funtech_h8_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(funtech_h8_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_reel_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(funtech_h8_state::get_reel_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(funtech_h8_state::get_reel_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(funtech_h8_state::get_reel_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(funtech_h8_state::get_reel_tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	m_tilemap->set_transparent_pen(0);
	m_reel_tilemap[0]->set_transparent_pen(0);
	m_reel_tilemap[1]->set_transparent_pen(0);
	m_reel_tilemap[2]->set_transparent_pen(0);
	m_reel_tilemap[3]->set_transparent_pen(0);
}

TILE_GET_INFO_MEMBER(funtech_h8_state::get_tile_info)
{
	uint16_t const tile = m_tileram[tile_index] | ((m_attrram[tile_index] & 0x0f) << 8) | (m_tilebank << 12);
	uint16_t const color = (m_attrram[tile_index] >> 4);

	tileinfo.set(0, tile, color, 0);
}

void funtech_h8_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void funtech_h8_state::attrram_w(offs_t offset, uint8_t data)
{
	m_attrram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(funtech_h8_state::get_reel_tile_info)
{
	uint16_t const tile = m_reel_tileram[Which][tile_index] | ((m_reel_attrram[Which][tile_index] & 0x0f) << 8);
	uint16_t const color = (m_reel_attrram[Which][tile_index] >> 4);

	tileinfo.set(1, tile, color, 0);
}

template <uint8_t Which>
void funtech_h8_state::reel_tileram_w(offs_t offset, uint8_t data)
{
	m_reel_tileram[Which][offset] = data;
	m_reel_tilemap[Which]->mark_tile_dirty(offset);
}

template <uint8_t Which>
void funtech_h8_state::reel_attrram_w(offs_t offset, uint8_t data)
{
	m_reel_attrram[Which][offset] = data;
	m_reel_tilemap[Which]->mark_tile_dirty(offset);
}

uint32_t funtech_h8_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	m_reel_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_reel_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_reel_tilemap[2]->draw(screen, bitmap, cliprect, 0, 0);
	m_reel_tilemap[3]->draw(screen, bitmap, cliprect, 0, 0);

	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void funtech_h8_state::machine_start()
{
	save_item(NAME(m_tilebank));
}

void funtech_h8_state::hopper_w(uint16_t data)
{
	m_hopper->motor_w(BIT(data, 7));

	if (data & 0xff7f)
		logerror("%s hopper_w unknown bits written: %04x\n", machine().describe_context(), data);
}


void funtech_h8_state::program_map(address_map &map)
{
	map(0x00000, 0x3ffff).rom();
	map(0x40000, 0x47fff).ram(); // NVRAM?
	map(0x48000, 0x48001).w(FUNC(funtech_h8_state::hopper_w));
	map(0x48008, 0x48009).portr("IN0").nopw(); // TODO: continuously, alternatively writes 0xff00 and 0x00ff
	map(0x4800a, 0x4800b).portr("DSW3_4");
	// map(0x4800c, 0x4800d).w // writes here sometimes
	map(0x4800e, 0x4800e).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	// map(0x48ffe, 0x48fff).r // reads here sometimes
	map(0xa0000, 0xa01ff).ram().lr8(NAME([this] (offs_t offset) -> uint8_t { return m_reel_tileram[0][offset]; })).w(FUNC(funtech_h8_state::reel_tileram_w<0>));
	map(0xa0200, 0xa03ff).ram().lr8(NAME([this] (offs_t offset) -> uint8_t { return m_reel_tileram[1][offset]; })).w(FUNC(funtech_h8_state::reel_tileram_w<1>));
	map(0xa0400, 0xa05ff).ram().lr8(NAME([this] (offs_t offset) -> uint8_t { return m_reel_tileram[2][offset]; })).w(FUNC(funtech_h8_state::reel_tileram_w<2>));
	map(0xa0600, 0xa07ff).ram().lr8(NAME([this] (offs_t offset) -> uint8_t { return m_reel_tileram[3][offset]; })).w(FUNC(funtech_h8_state::reel_tileram_w<3>));
	map(0xa8000, 0xa81ff).ram().lr8(NAME([this] (offs_t offset) -> uint8_t { return m_reel_attrram[0][offset]; })).w(FUNC(funtech_h8_state::reel_attrram_w<0>));
	map(0xa8200, 0xa83ff).ram().lr8(NAME([this] (offs_t offset) -> uint8_t { return m_reel_attrram[1][offset]; })).w(FUNC(funtech_h8_state::reel_attrram_w<1>));
	map(0xa8400, 0xa85ff).ram().lr8(NAME([this] (offs_t offset) -> uint8_t { return m_reel_attrram[2][offset]; })).w(FUNC(funtech_h8_state::reel_attrram_w<2>));
	map(0xa8600, 0xa87ff).ram().lr8(NAME([this] (offs_t offset) -> uint8_t { return m_reel_attrram[3][offset]; })).w(FUNC(funtech_h8_state::reel_attrram_w<3>));
	map(0xc0000, 0xc01ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0xc8000, 0xc81ff).ram().w("palette", FUNC(palette_device::write16_ext)).share("palette_ext");
	map(0xd8000, 0xd87ff).ram(); // seems unused by this game, only initialized
	map(0xe0000, 0xe07ff).ram().lr8(NAME([this] (offs_t offset) -> uint8_t { return m_tileram[offset]; })).w(FUNC(funtech_h8_state::tileram_w));
	map(0xe0800, 0xe083f).ram(); // ??
	map(0xe8000, 0xe87ff).ram().lr8(NAME([this] (offs_t offset) -> uint8_t { return m_attrram[offset]; })).w(FUNC(funtech_h8_state::attrram_w));
	map(0xe8800, 0xe883f).ram(); // ??
}


static INPUT_PORTS_START( goldnegg )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_SERVICE_NO_TOGGLE( 0x0020, IP_ACTIVE_LOW )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )

	PORT_START("DSW1" )
	PORT_DIPNAME( 0x03, 0x03, "Main Game Rate" )  PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "45" )
	PORT_DIPSETTING(    0x02, "50" )
	PORT_DIPSETTING(    0x01, "55" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPNAME( 0x04, 0x04, "Double Game Rate" )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "80" )
	PORT_DIPSETTING(    0x04, "90" )
	PORT_DIPNAME( 0x18, 0x18, "Max Play" )  PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, "8" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x08, "64" )
	PORT_DIPSETTING(    0x00, "80" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6") // no effect shown in test mode
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Min Play for Bonus" )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "8" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8") // no effect shown in test mode
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2" )
	PORT_DIPNAME( 0x01, 0x01, "Double Game" )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2") // no effect shown in test mode
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3") // no effect shown in test mode
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_A ) )  PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_25C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:6") // no effect shown in test mode
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )  PORT_DIPLOCATION("SW2:7,8") // notes
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_100C ) )
	PORT_DIPSETTING(    0x40, "1 Coin/500 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/1000 Credits" )

	PORT_START("DSW3_4" ) // no effect shown in test mode for most
	PORT_DIPNAME(           0x0003, 0x0003, "Min Play" )  PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(                0x0003, "1" )
	PORT_DIPSETTING(                0x0002, "8" )
	PORT_DIPSETTING(                0x0001, "16" )
	PORT_DIPSETTING(                0x0000, "32" )
	PORT_DIPNAME(           0x000c, 0x000c, "Coin Limit" )  PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(                0x000c, "1000" )
	PORT_DIPSETTING(                0x0008, "5000" )
	PORT_DIPSETTING(                0x0004, "10000" )
	PORT_DIPSETTING(                0x0000, "90000" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, 0x0020, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, 0x0040, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "SW3:8" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0100, 0x0100, "SW4:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW4:2" )
	PORT_DIPNAME(           0x0400, 0x0400, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(                0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(                0x0400, DEF_STR( On ) )
	PORT_DIPNAME(           0x1800, 0x1800, "Pool Bonus" )  PORT_DIPLOCATION("SW4:4,5")
	PORT_DIPSETTING(                0x1800, "1000" )
	PORT_DIPSETTING(                0x1000, "3000" )
	PORT_DIPSETTING(                0x0800, "7000" )
	PORT_DIPSETTING(                0x0000, "Unlimited" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW4:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW4:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW4:8" )
INPUT_PORTS_END


static GFXDECODE_START( gfx_goldnegg )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x4_packed_lsb, 0x000, 0x10 )
	GFXDECODE_ENTRY( "reels", 0, gfx_16x16x4_packed_lsb, 0x100, 0x10 )
GFXDECODE_END


void funtech_h8_state::funtech_h8(machine_config &config)
{
	H83030(config, m_maincpu, 12_MHz_XTAL / 6); // minimal speed for an H8 is 2 MHz and this seems to match available reference, but strange
	m_maincpu->set_addrmap(AS_PROGRAM, &funtech_h8_state::program_map);
	m_maincpu->read_port7().set_ioport("DSW1");
	m_maincpu->read_porta().set_ioport("DSW2");
	m_maincpu->write_portb().set([this] (uint8_t data) { m_tilebank = (data & 0x06) >> 1; }); // TODO: Bit 5 also used.

	HOPPER(config, m_hopper, attotime::from_msec(50));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(funtech_h8_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_goldnegg);

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 0x200);

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 12_MHz_XTAL / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // pin 7 and clock not verified
}


ROM_START( goldnegg )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "45_ge_cs_4c38.u6", 0x00000, 0x40000, CRC(8e0b0ee1) SHA1(2345182339c38bf6decc256c18ef905f25a6d2f2) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "golden_u29_usa.u29", 0x00000, 0x80000, CRC(9f8c893d) SHA1(d1ab196ae6859c02d8517b40668c3cba9b5a18a4) )

	ROM_REGION( 0x80000, "reels", 0 )
	ROM_LOAD( "golden_u41_usa.u41", 0x00000, 0x80000, CRC(d5738c18) SHA1(856594dc76f930beb423fffab2c36e1905c8242c) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "golden_u72_usa.u72", 0x00000, 0x40000, CRC(a636f06b) SHA1(57a4a809ea0f955482f04cc7aedb5bed1c83aca8) )
ROM_END

} // anonymous namespace


// puts the following strings in RAM: 1999-05-24, -TAIWAN-GAMEMAX-, --VERSION:U1.7--
GAME( 1999, goldnegg, 0, funtech_h8, goldnegg, funtech_h8_state, empty_init, ROT0, "LSE", "Golden Egg (version U1.8)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )
