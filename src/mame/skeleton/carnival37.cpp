// license:BSD-3-Clause
// copyright-holders:

/*
Carnival 37 by Able (1992)

NE910130 REV.A PCB

TMPZ84C011AF-6 CPU
12.000 MHz XTAL
HD6445P4 CRTC
6116ALSP-15 RAM 8 (near CRTC)
HD153129P RAMDAC
YM2149F PSG
2x bank of 8 switches
reset button

TODO:
- hopper
- player 2 inputs
- verify if we aren't missing any inputs from the pinout (game has no service mode)
- colors are very dark but seem to be dark on original snaps, too. BTANB?
*/

#include "emu.h"

#include "cpu/z80/tmpz84c011.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class carnival37_state : public driver_device
{
public:
	carnival37_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_ramdac(*this, "ramdac"),
		m_tileram(*this, "tileram"),
		m_attrram(*this, "attrram")
	{ }

	void carniv37(machine_config &config) ATTR_COLD;

	void init_gfx() ATTR_COLD;
protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<tmpz84c011_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<ramdac_device> m_ramdac;
	required_shared_ptr<uint8_t> m_tileram;
	required_shared_ptr<uint8_t> m_attrram;

	tilemap_t *m_tilemap = nullptr;

	TILE_GET_INFO_MEMBER(get_tile_info);
	void tileram_w(offs_t offset, uint8_t data);
	void attrram_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
};


void carnival37_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(carnival37_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 36, 28);
}

TILE_GET_INFO_MEMBER(carnival37_state::get_tile_info)
{
	uint16_t const tile = m_tileram[tile_index] | (m_attrram[tile_index] << 8);

	tileinfo.set(0, tile, 0, 0);
}

void carnival37_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void carnival37_state::attrram_w(offs_t offset, uint8_t data)
{
	m_attrram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

uint32_t carnival37_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void carnival37_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xa000, 0xa7ff).ram();
	map(0xe000, 0xe3ff).ram().w(FUNC(carnival37_state::tileram_w)).share(m_tileram);
	map(0xe400, 0xe7ff).ram().w(FUNC(carnival37_state::attrram_w)).share(m_attrram);
}

void carnival37_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x80, 0x80).rw("crtc", FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));
	map(0x81, 0x81).rw("crtc", FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0x90, 0x91).rw("ym", FUNC(ym2149_device::data_r), FUNC(ym2149_device::address_data_w));
	map(0xa0, 0xa0).w("ramdac", FUNC(ramdac_device::index_w));
	map(0xa1, 0xa1).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0xa2, 0xa2).w("ramdac", FUNC(ramdac_device::mask_w));
}

// writes 4 bit data over a regular RGB666?
void carnival37_state::ramdac_map(address_map &map)
{
	map(0x000, 0x2ff).lrw8(
		NAME([this] (offs_t offset)  {
			return m_ramdac->ramdac_pal_r(offset) << 2;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_ramdac->ramdac_rgb666_w(offset, data << 2);
		})
	);
}


static INPUT_PORTS_START( carniv37 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	// TODO: IPT_GAMBLE has no support for PORT_COCKTAIL/PORT_PLAYER(2) (same mapping as above)
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) // has no coin counter write
	// all coin chutes are very sensible (prone to "coinjam"s)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) // PORT_NAME("Analyzer")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // hopper status? (shows hopper empty message)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END


static GFXDECODE_START( gfx )
	GFXDECODE_ENTRY( "decoded_tiles", 0, gfx_8x8x4_packed_msb, 0, 1 )
GFXDECODE_END


static const z80_daisy_config daisy_chain[] =
{
	TMPZ84C011_DAISY_INTERNAL,
	{ nullptr }
};


void carnival37_state::carniv37(machine_config &config)
{
	// basic machine hardware
	TMPZ84C011(config, m_maincpu, 12_MHz_XTAL / 2);
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &carnival37_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &carnival37_state::io_map);
	m_maincpu->in_pa_callback().set_ioport("IN0");
	m_maincpu->in_pb_callback().set_ioport("IN1");
	m_maincpu->in_pc_callback().set_ioport("IN2");
	m_maincpu->out_pd_callback().set([this] (uint8_t data) {
		machine().bookkeeping().coin_counter_w(0, BIT(~data, 0));
		machine().bookkeeping().coin_counter_w(1, BIT(~data, 1));
		machine().bookkeeping().coin_counter_w(2, BIT(~data, 2));
		// bit 3 hopper motor, active low
		// bit 4 unknown
		// bit 5-6 high at payout
		flip_screen_set(BIT(data, 7));
		logerror("%s CPU port D write: %02x\n", machine().describe_context(), data);
	});

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 36*8-1, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(carnival37_state::screen_update));

	HD6845S(config, "crtc", 12_MHz_XTAL / 3); // actually HD6445P4 CRTC-II

	GFXDECODE(config, m_gfxdecode, "palette", gfx);

	PALETTE(config, "palette").set_entries(0x100);

	RAMDAC(config, m_ramdac, "palette").set_addrmap(0, &carnival37_state::ramdac_map);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2149_device &ym(YM2149(config, "ym", 12_MHz_XTAL / 8)); // divider not verified
	ym.port_a_read_callback().set_ioport("DSW1");
	ym.port_b_read_callback().set_ioport("DSW2");
	ym.add_route(ALL_OUTPUTS, "mono", 1.00);
}

void carnival37_state::init_gfx()
{
	u8 *tiles = memregion("tiles")->base();
	u8 *decoded = memregion("decoded_tiles")->base();

	for (int base_addr = 0; base_addr < 0x10000; base_addr += 0x4000)
	{
		for (int i = 0; i < 0x2000; i ++)
		{
			decoded[base_addr | (i << 1) | 0] = tiles[base_addr | i | 0x0000];
			decoded[base_addr | (i << 1) | 1] = tiles[base_addr | i | 0x2000];
		}
	}
}

ROM_START( carniv37 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "256.ic28", 0x0000, 0x8000, CRC(7d7d3f2a) SHA1(76223d42995f3925379d45ba0524541380c4f43b) ) // handwritten label

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "512.ic40", 0x0000, 0x10000, CRC(789a2336) SHA1(3f441f063c113b787aff919259ae974b3b4e4a62) ) // handwritten label

	ROM_REGION( 0x10000, "decoded_tiles", ROMREGION_ERASEFF)

	// ic41 not populated
ROM_END

} // anonymous namespace


GAME( 1992, carniv37, 0, carniv37, carniv37, carnival37_state, init_gfx, ROT0, "Able", "Carnival 37", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_NO_COCKTAIL )
