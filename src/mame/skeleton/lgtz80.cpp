// license:BSD-3-Clause
// copyright-holders:AJR

/*
Video slots by Logic Game Tech Int. (LGT).

The main components are:
scratched off rectangular 100-pin chip, stickered ASIC 1
scratched off square 100-pin chip, stickered ASIC 2
scratched off square 100-pin chip, stickered ASIC 3
scratched off square 44-pin chip, stickered ASIC 4
12 MHz XTAL (near ASIC 2)
7.3728 MHz XTAL (near ASIC 4)
U6295 sound chip
HM86171-80 RAMDAC (near CPU ROM)
6x M5M5256DVP (1 near CPU ROM, 5 near GFX ROMs)

The two dumped games use PCBs with different layout, however the components appear
to be the same or at least same from different manufacturers.

"ASIC 1" is probably a KL5C80A12 CPU, though its on-chip peripherals are used for little
besides rudimentary ROM banking and port I/O.

TODO:
- arthurkn uploads code to NVRAM at 2B000-2BFFF if missing, fruitcat seemingly needs the
  same range pre-populated, thus currently runs off the rails when calling to NVRAM, seems
  to fortuitously recover, but never populates tile RAM
- arthurkn runs correctly and needs the following:
  - reels' scrolling implementation is weird (hacky?)
  - outputs (lamps / meters)
  - hopper (off by default)
  - visible area is probably not 100% correct
*/


#include "emu.h"

#include "cpu/z80/kl5c80a12.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class lgtz80_state : public driver_device
{
public:
	lgtz80_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_oki(*this, "oki"),
		m_tile_ram(*this, "tile_ram"),
		m_tile_attr_ram(*this, "tile_attr_ram"),
		m_reel_ram(*this, "reel_ram%u", 0U),
		m_reel_attr_ram(*this, "reel_attr_ram%u", 0U),
		m_reel_scroll_ram(*this, "reel_scroll_ram%u", 0U),
		m_control(0)
	{ }

	void fruitcat(machine_config &config) ATTR_COLD;
	void arthurkn(machine_config &config) ATTR_COLD;

	void init_arthurkn() ATTR_COLD;
	void init_fruitcat() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<kl5c80a12_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<okim6295_device> m_oki;

	required_shared_ptr<u8> m_tile_ram;
	required_shared_ptr<u8> m_tile_attr_ram;
	required_shared_ptr_array<u8, 4> m_reel_ram;
	required_shared_ptr_array<u8, 4> m_reel_attr_ram;
	required_shared_ptr_array<u8, 4> m_reel_scroll_ram;

	u8 m_control = 0;

	tilemap_t *m_tilemap = nullptr;
	tilemap_t *m_reel_tilemap[4] {};

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void vblank_nmi_w(int state);
	void oki_bank_w(u8 data);

	u8 control_r();
	void control_w(u8 data);
	u8 e0_r();

	void tile_ram_w(offs_t offset, u8 data);
	void tile_attr_ram_w(offs_t offset, u8 data);
	template <uint8_t Which> void reel_ram_w(offs_t offset, u8 data);
	template <uint8_t Which> void reel_attr_ram_w(offs_t offset, u8 data);
	template <uint8_t Which> void reel_scroll_ram_w(offs_t offset, u8 data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_reel_tile_info);

	void program_map(address_map &map) ATTR_COLD;
	void fruitcat_io_map(address_map &map) ATTR_COLD;
	void arthurkn_io_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
};


void lgtz80_state::machine_start()
{
	save_item(NAME(m_control));
}

void lgtz80_state::machine_reset()
{
	control_w(0);
}


u32 lgtz80_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int j = 0; j < 0x40; j++)
		m_reel_tilemap[0]->set_scrolly(j, m_reel_scroll_ram[0][j + 0x40]);

	for (int i = 1; i < 0x04; i++)
		for (int j = 0; j < 0x40; j++)
			m_reel_tilemap[i]->set_scrolly(j, m_reel_scroll_ram[i][j]);

	m_reel_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_reel_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_reel_tilemap[2]->draw(screen, bitmap, cliprect, 0, 0);
	m_reel_tilemap[3]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void lgtz80_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lgtz80_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_reel_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lgtz80_state::get_reel_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lgtz80_state::get_reel_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lgtz80_state::get_reel_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lgtz80_state::get_reel_tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	m_tilemap->set_transparent_pen(0);

	for (int i = 1; i < 4; i++)
		m_reel_tilemap[i]->set_transparent_pen(0x1f);
}

TILE_GET_INFO_MEMBER(lgtz80_state::get_tile_info)
{
	int const tile = m_tile_ram[tile_index] | (m_tile_attr_ram[tile_index] << 8);

	tileinfo.set(0, tile, 0, 0);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(lgtz80_state::get_reel_tile_info)
{
	int const tile = m_reel_ram[Which][tile_index] | (m_reel_attr_ram[Which][tile_index] << 8);

	tileinfo.set(1, tile, 0, 0);
}

void lgtz80_state::tile_ram_w(offs_t offset, u8 data)
{
	m_tile_ram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void lgtz80_state::tile_attr_ram_w(offs_t offset, u8 data)
{
	m_tile_attr_ram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

template <uint8_t Which>
void lgtz80_state::reel_ram_w(offs_t offset, u8 data)
{
	m_reel_ram[Which][offset] = data;
	m_reel_tilemap[Which]->mark_tile_dirty(offset);
}

template <uint8_t Which>
void lgtz80_state::reel_attr_ram_w(offs_t offset, u8 data)
{
	m_reel_attr_ram[Which][offset] = data;
	m_reel_tilemap[Which]->mark_tile_dirty(offset);
}

template <uint8_t Which>
void lgtz80_state::reel_scroll_ram_w(offs_t offset, u8 data)
{
	m_reel_scroll_ram[Which][offset] = data;
	m_reel_tilemap[Which]->mark_tile_dirty(offset);
}


void lgtz80_state::oki_bank_w(u8 data)
{
	// fruitcat only? arthurkn configures P00 as an output pin but never writes to the data register
	m_oki->set_rom_bank(BIT(data, 0));
}

u8 lgtz80_state::control_r()
{
	return m_control;
}

void lgtz80_state::control_w(u8 data)
{
	// Bit 7 = NMI mask
	m_control = data;
	if (!BIT(data, 7))
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	// Bit 6 always set?

	if (!BIT(data, 6))
		logerror("%s: control_w bit 6 unset (%02X)\n", machine().describe_context(), data);

	if (data & 0x3f)
		logerror("%s: control_w bits 0-5 (%02X)\n", machine().describe_context(), data);
}

u8 lgtz80_state::e0_r()
{
	// arthurkn: protection?
	return 0x6e;
}

void lgtz80_state::vblank_nmi_w(int state)
{
	if (state && BIT(m_control, 7))
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


void lgtz80_state::program_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x28000, 0x2bfff).ram().share("nvram");
	map(0x4c000, 0x4c1ff).ram().w(FUNC(lgtz80_state::reel_attr_ram_w<0>)).share(m_reel_attr_ram[0]);
	map(0x4c200, 0x4c3ff).ram().w(FUNC(lgtz80_state::reel_attr_ram_w<1>)).share(m_reel_attr_ram[1]);
	map(0x4c400, 0x4c5ff).ram().w(FUNC(lgtz80_state::reel_attr_ram_w<2>)).share(m_reel_attr_ram[2]);
	map(0x4c600, 0x4c7ff).ram().w(FUNC(lgtz80_state::reel_attr_ram_w<3>)).share(m_reel_attr_ram[3]);
	map(0x4c800, 0x4c9ff).ram().w(FUNC(lgtz80_state::reel_ram_w<0>)).share(m_reel_ram[0]);
	map(0x4ca00, 0x4cbff).ram().w(FUNC(lgtz80_state::reel_ram_w<1>)).share(m_reel_ram[1]);
	map(0x4cc00, 0x4cdff).ram().w(FUNC(lgtz80_state::reel_ram_w<2>)).share(m_reel_ram[2]);
	map(0x4ce00, 0x4cfff).ram().w(FUNC(lgtz80_state::reel_ram_w<3>)).share(m_reel_ram[3]);
	map(0x4d000, 0x4d7ff).ram().w(FUNC(lgtz80_state::tile_attr_ram_w)).share(m_tile_attr_ram);
	map(0x4d800, 0x4dfff).ram().w(FUNC(lgtz80_state::tile_ram_w)).share(m_tile_ram);
	map(0x4e000, 0x4e07f).ram().w(FUNC(lgtz80_state::reel_scroll_ram_w<0>)).share(m_reel_scroll_ram[0]);
	map(0x4e080, 0x4e0ff).ram().w(FUNC(lgtz80_state::reel_scroll_ram_w<1>)).share(m_reel_scroll_ram[1]);
	map(0x4e100, 0x4e17f).ram().w(FUNC(lgtz80_state::reel_scroll_ram_w<2>)).share(m_reel_scroll_ram[2]);
	map(0x4e180, 0x4e1ff).ram().w(FUNC(lgtz80_state::reel_scroll_ram_w<3>)).share(m_reel_scroll_ram[3]);
}

void lgtz80_state::fruitcat_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x80).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x81, 0x81).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x82, 0x82).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x88, 0x88).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	// map(0x98, 0x98).w(); TODO
	map(0xc0, 0xc0).rw(FUNC(lgtz80_state::control_r), FUNC(lgtz80_state::control_w));
}

void lgtz80_state::arthurkn_io_map(address_map &map)
{
	map.global_mask(0xff);
	// map(0x80, 0x80).w(); TODO: bits 1-4 are meters, tested in key test screen
	// map(0x88, 0x88).w(); TODO: probably lamps
	map(0xa0, 0xa0).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xb0, 0xb0).w("ramdac", FUNC(ramdac_device::index_w));
	map(0xb1, 0xb1).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0xb2, 0xb2).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0xc0, 0xc0).rw(FUNC(lgtz80_state::control_r), FUNC(lgtz80_state::control_w));
	map(0xe0, 0xe0).r(FUNC(lgtz80_state::e0_r));
}

void lgtz80_state::ramdac_map(address_map &map)
{
	map(0x000, 0x2ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}


static INPUT_PORTS_START( arthurkn )
	PORT_START("IN1")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) //  also works as down in system configuration
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) // also works for exiting system configuration
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) // TODO: "Get" in test mode, is it take? also works as up in system configuration
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Low / Show Odds") // "Small" in test mode. Also works as change setting down in system configuration
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // "Coin out" in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) // "Stop A" in test mode

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) // "Stop B" in test mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) // "Stop C" in test mode
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) // "Check" in test mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) // "Big" in test mode. Also works as change setting up in system configuration
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) // TODO: hopper

	PORT_START("IN3") // not shown in key test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) // ??

// no DSW on PCB
INPUT_PORTS_END


GFXLAYOUT_RAW(gfx_8x32x8_raw, 8, 32, 8 * 8, 8 * 32 * 8);

static GFXDECODE_START( gfx )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x8_raw, 0, 1 )
	GFXDECODE_ENTRY( "reels", 0, gfx_8x32x8_raw, 0, 1 )
GFXDECODE_END


void lgtz80_state::fruitcat(machine_config &config)
{
	KL5C80A12(config, m_maincpu, 12_MHz_XTAL); // exact CPU model and divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &lgtz80_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &lgtz80_state::fruitcat_io_map);
	m_maincpu->out_p0_callback().set(FUNC(lgtz80_state::oki_bank_w));
	m_maincpu->in_p1_callback().set_ioport("IN1");
	m_maincpu->in_p2_callback().set_ioport("IN2");
	m_maincpu->in_p3_callback().set_ioport("IN3");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(1, 64*8-1, 8-1, 31*8-1);
	screen.set_screen_update(FUNC(lgtz80_state::screen_update));
	screen.screen_vblank().set(FUNC(lgtz80_state::vblank_nmi_w));

	GFXDECODE(config, "gfxdecode", "palette", gfx);

	PALETTE(config, "palette").set_entries(0x100);

	RAMDAC(config, "ramdac", 0, "palette").set_addrmap(0, &lgtz80_state::ramdac_map);

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 12_MHz_XTAL / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // pin 7 and clock not verified
}

void lgtz80_state::arthurkn(machine_config &config)
{
	fruitcat(config);
	m_maincpu->set_addrmap(AS_IO, &lgtz80_state::arthurkn_io_map);
}


ROM_START( fruitcat )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "fruit_cat_v2.00.u8", 0x00000, 0x20000, CRC(83d71147) SHA1(4253f5d3273bce22262d34a08f492fa72f776aa5) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_WORD_SWAP( "fruit_cat_rom2.u22", 0x000000, 0x200000, CRC(c78150e8) SHA1(eb276b9b2c4e45b8caf81f17831f6201a6d7392c) ) // actual label has ROM2 between brackets

	ROM_REGION( 0x200000, "reels", 0 )
	ROM_LOAD16_WORD_SWAP( "fruit_cat_rom3.u29", 0x000000, 0x200000, CRC(71afea49) SHA1(89c814302fb58705a479310edb433594d151dfb5) ) // actual label has ROM3 between brackets

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "am29f040.u2", 0x00000, 0x80000, CRC(efd1209e) SHA1(5cd76c9d3073b2e689aa7903e2d65b8ce5b091ca) )

	ROM_REGION( 0x200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf16v8b-15pc.u21", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( arthurkn ) // no stickers on ROMs
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "w29ee011.u21", 0x00000, 0x20000, CRC(d8e2b9f4) SHA1(e8c55c42d7b57fde3168e07fa51f307b83803967) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_WORD_SWAP( "m27c160.u42", 0x000000, 0x200000, CRC(f03a9b0d) SHA1(1e8d9efe7d50871ffc6a0c4c7f08047dd5aac294) )

	ROM_REGION( 0x200000, "reels", 0 )
	ROM_LOAD16_WORD_SWAP( "m27c160.u43", 0x000000, 0x200000, CRC(31d2caab) SHA1(0ee7f35dadb1d5159a487701d059bfd2f54f8c02) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "m29f040b.u18", 0x00000, 0x80000, CRC(2b9ab706) SHA1(92154126c7db227acaa4966f71d28475c622e1e6) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf16v8b-15pc.u3", 0x000, 0x117, NO_DUMP )
ROM_END


void lgtz80_state::init_fruitcat()
{
	// Encryption involves a permutation of odd-numbered data lines, conditional on address lines
	u8 *rom = memregion("maincpu")->base();
	for (int i = 0; i < 0x20000; i++)
	{
		switch (i & 0x7c0)
		{
		case 0x000:
			rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 1, 2, 3, 0);
			break;

		case 0x040:
		case 0x440:
			rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 1, 2, 5, 0);
			break;

		case 0x080:
		case 0x480:
			rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 5, 2, 3, 0);
			break;

		case 0x0c0:
			rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 3, 2, 7, 0);
			break;

		case 0x100:
		case 0x400:
			rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 5, 2, 1, 0);
			break;

		case 0x140:
			rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 7, 2, 5, 0);
			break;

		case 0x180:
		case 0x2c0:
			rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 1, 2, 7, 0);
			break;

		case 0x1c0:
			rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 1, 2, 3, 0);
			break;

		case 0x200:
			rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 1, 2, 7, 0);
			break;

		case 0x240:
		case 0x5c0:
			rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 7, 2, 3, 0);
			break;

		case 0x280:
			rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 7, 2, 5, 0);
			break;

		case 0x300:
			//rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 3, 2, 1, 0);
			break;

		case 0x340:
			rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 3, 2, 5, 0);
			break;

		case 0x380:
			rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 7, 2, 1, 0);
			break;

		case 0x3c0:
		case 0x540:
			rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 3, 2, 7, 0);
			break;

		case 0x4c0:
			rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 5, 2, 1, 0);
			break;

		case 0x500:
		case 0x780:
			rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 5, 2, 3, 0);
			break;

		case 0x580:
			rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 7, 2, 3, 0);
			break;

		case 0x600:
		case 0x680:
			rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 3, 2, 5, 0);
			break;

		case 0x640:
			rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 7, 2, 1, 0);
			break;

		case 0x6c0:
			rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 3, 2, 1, 0);
			break;

		case 0x700:
			rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 1, 2, 5, 0);
			break;

		case 0x740:
			rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 5, 2, 7, 0);
			break;

		case 0x7c0:
			rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 5, 2, 7, 0);
			break;
		}
	}
}

void lgtz80_state::init_arthurkn()
{
	// Encryption involves a permutation of odd-numbered data lines, conditional on address lines
	u8 *rom = memregion("maincpu")->base();
	for (int i = 0; i < 0x20000; i++)
	{
		switch (i & 0x7c0)
		{
		case 0x000:
		case 0x1c0:
			rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 1, 2, 7, 0);
			break;

		case 0x040:
		case 0x0c0:
			rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 1, 2, 5, 0);
			break;

		case 0x080:
		case 0x2c0:
			rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 7, 2, 3, 0);
			break;

		case 0x100:
			rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 3, 2, 5, 0);
			break;

		case 0x140:
		case 0x340:
			rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 5, 2, 1, 0);
			break;

		case 0x180:
			//rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 3, 2, 1, 0);
			break;

		case 0x200:
		case 0x380:
			rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 3, 2, 5, 0);
			break;

		case 0x240:
			rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 1, 2, 7, 0);
			break;

		case 0x280:
		case 0x300:
			rom[i] = bitswap<8>(rom[i], 7, 6, 1, 4, 5, 2, 3, 0);
			break;

		case 0x3c0:
		case 0x440:
			rom[i] = bitswap<8>(rom[i], 5, 6, 1, 4, 3, 2, 7, 0);
			break;

		case 0x400:
		case 0x480:
			rom[i] = bitswap<8>(rom[i], 1, 6, 7, 4, 5, 2, 3, 0);
			break;

		case 0x4c0:
			rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 3, 2, 1, 0);
			break;

		case 0x500:
			rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 7, 2, 5, 0);
			break;

		case 0x540:
			rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 7, 2, 3, 0);
			break;

		case 0x580:
			rom[i] = bitswap<8>(rom[i], 3, 6, 5, 4, 7, 2, 1, 0);
			break;

		case 0x5c0:
			rom[i] = bitswap<8>(rom[i], 3, 6, 1, 4, 5, 2, 7, 0);
			break;

		case 0x600:
			rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 1, 2, 3, 0);
			break;

		case 0x640:
			rom[i] = bitswap<8>(rom[i], 5, 6, 7, 4, 1, 2, 3, 0);
			break;

		case 0x680:
			rom[i] = bitswap<8>(rom[i], 1, 6, 5, 4, 3, 2, 7, 0);
			break;

		case 0x6c0:
			rom[i] = bitswap<8>(rom[i], 7, 6, 3, 4, 5, 2, 1, 0);
			break;

		case 0x700:
			rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 5, 2, 7, 0);
			break;

		case 0x740:
			rom[i] = bitswap<8>(rom[i], 3, 6, 7, 4, 1, 2, 5, 0);
			break;

		case 0x780:
			rom[i] = bitswap<8>(rom[i], 1, 6, 3, 4, 7, 2, 5, 0);
			break;

		case 0x7c0:
			rom[i] = bitswap<8>(rom[i], 5, 6, 3, 4, 7, 2, 1, 0);
			break;
		}
	}
}

} // anonymous namespace


GAME( 2003?, fruitcat, 0, fruitcat, arthurkn, lgtz80_state, init_fruitcat, ROT0, "LGT", "Fruit Cat (v2.00)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 200?,  arthurkn, 0, arthurkn, arthurkn, lgtz80_state, init_arthurkn, ROT0, "LGT", "Arthur's Knights",  MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
