// license:BSD-3-Clause
// copyright-holders:

/****************************************************************************************

Unknown Color Poker Game
------------------------------

8080A CPU

Four 2716 eproms

Six 2102 Rams

Two 2112 Rams

Two 5101 Rams (Low Power Versions, one connected to Battery)

Four position DIP Switch - DIP 1 changes on screen text from
normal to highlighted as seen in pics. Other DIPs unknown.

Sound?

Date of manufacture unknown. Latest date codes on logic chips is 1980.

Chaneman 3/20/2019


TODO: stuck if it isn't immediately coined up at boot.
      Game can be tested by keeping COIN1 (5 by default) pressed at boot.

*******************************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "machine/nvram.h"

#include "screen.h"
#include "emupal.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class unkpoker_state : public driver_device
{
public:
	unkpoker_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram")
	{ }

	void unkpoker(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_videoram;

	tilemap_t *m_tilemap = nullptr;

	void videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void unkpoker_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(unkpoker_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void unkpoker_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data & 0x7f;
	m_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(unkpoker_state::get_tile_info)
{
	int const code = m_videoram[tile_index];
	tileinfo.set(0, code, 0, 0);
}

uint32_t unkpoker_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void unkpoker_state::mem_map(address_map &map)
{
	map(0x0000,0x0fff).rom();
	map(0x1000,0x1fff).ram();
	map(0x8000,0x83ff).ram().w(FUNC(unkpoker_state::videoram_w)).share(m_videoram);
}

void unkpoker_state::io_map(address_map &map)
{
	map(0x01,0x01).portr("IN1"); // writes here
	map(0x02,0x02).portr("DSW"); // writes here, seems sound related
	map(0x04,0x04).portr("IN2");
}


static INPUT_PORTS_START( unkpoker )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // does something
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Ante")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // does something
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )

	PORT_START("DSW") // seem to change coinage, but DSW:1 should do something different (see notes on top of the driver)
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW:4")
INPUT_PORTS_END


static GFXDECODE_START( gfx_unkpoker )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x1, 0, 1 )
GFXDECODE_END


void unkpoker_state::unkpoker(machine_config &config)
{
	// basic machine hardware
	I8080A(config, m_maincpu, 2'000'000); // guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &unkpoker_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &unkpoker_state::io_map);

	//NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 255, 0, 255);
	screen.set_screen_update(FUNC(unkpoker_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_unkpoker);
	PALETTE(config, "palette", palette_device::RGB_3BIT);

	// sound hardware
	SPEAKER(config, "mono").front_center();
}


ROM_START( unkpoker )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "cdpat.13b",  0x0000, 0x0800, CRC(c3f3040f) SHA1(ed1916bcd9e1e80502fcff5ddd599c101a226e7c) )
	ROM_LOAD( "cdpat.11b",  0x0800, 0x0800, CRC(16a97398) SHA1(737192dd0e1b3083f1facd327a83d98a0b7f4d66) )

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "cd.3b", 0x0000, 0x0800, CRC(e3997d7d) SHA1(6c595c70afedc7aef024215d153fe31b418adc25) )
	ROM_LOAD( "cd.3c", 0x0800, 0x0800, CRC(b61adb76) SHA1(9805593fc6d9b01e4a63bfc35e5442c4c547c103) )
ROM_END

} // anonymous namespace


GAME(1980?, unkpoker, 0, unkpoker, unkpoker, unkpoker_state, empty_init, ROT0, "<unknown>", "unknown 1980 poker game", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE)
