// license:BSD-3-Clause
// copyright-holders: David Haywood

/*
PCB Layout
----------

ACE9412
DM941204
|------------------------------------------|
|UPC1241 YM3012 YM2151       3.BIN         |
|TL084 TL084 6116      30MHz 4.BIN         |
|9.BIN    1.BIN                            |
|M6295    Z80B(2)                          |
|     4MHz                                 |
|                                      6116|
|J    6116                             6116|
|A    6116                                 |
|M                                         |
|M                          2018           |
|A                                         |
|  DSW(8)     |-----|           6264       |
|      62256  |ACTEL|           5.BIN      |
|      2.BIN  |A1020|           6.BIN      |
|      Z80B(1)|-----|           7.BIN      |
|12MHz                          8.BIN      |
|------------------------------------------|
Notes:
      Z80B(1)   - clock 6.000MHz [12/2]
      Z80B(2)   - clock 4.000MHz
      YM2151    - clock 4.000MHz
      M6295     - clock 1.000MHz [4/4]
      6116/2018 - 2k x8 SRAM
      6264      - 8k x8 SRAM
      62256     - 32k x8 SRAM

The 30 MHz XTAL (silkscreened as such on PCB) has also been seen as 15MHz on a second PCB
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class gameace_state : public driver_device
{
public:
	gameace_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_membank(*this, "bank1"),
		m_fgram(*this, "fgram"),
		m_colram(*this, "colram"),
		m_gfxdecode(*this, "gfxdecode")
	{
	}

	void gameace(machine_config &config);

	void init_hotbody();

protected:
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	optional_memory_bank m_membank;
	required_shared_ptr<uint8_t> m_fgram;
	required_shared_ptr<uint8_t> m_colram;
	required_device<gfxdecode_device> m_gfxdecode;

	tilemap_t *m_fg_tilemap = nullptr;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bank_w(uint8_t data);

	void fgram_w(offs_t offset, uint8_t data);
	void colram_w(offs_t offset, uint8_t data);
	uint8_t rand_r();

	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void main_program_map(address_map &map);
	void main_port_map(address_map &map);
	void sound_program_map(address_map &map);
};

uint32_t gameace_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

TILE_GET_INFO_MEMBER(gameace_state::get_fg_tile_info)
{
	int code = m_fgram[tile_index*2] | (m_fgram[(tile_index*2)+1] << 8);
	int col = m_colram[tile_index];
	tileinfo.set(0,code,col,0);
}

void gameace_state::fgram_w(offs_t offset, uint8_t data)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset/2);
}

void gameace_state::colram_w(offs_t offset, uint8_t data)
{
	m_colram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void gameace_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gameace_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}


void gameace_state::bank_w(uint8_t data)
{
	m_membank->set_entry(data & 0xf);
}

uint8_t gameace_state::rand_r()
{
	return machine().rand();
}

void gameace_state::main_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xc7ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0xc800, 0xcfff).ram().w(FUNC(gameace_state::colram_w)).share(m_colram);
	map(0xd000, 0xdfff).ram().w(FUNC(gameace_state::fgram_w)).share(m_fgram); // banked with sprites?
	map(0xe000, 0xefff).ram();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xffff).ram();
}

void gameace_state::main_port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(gameace_state::rand_r));
	map(0x02, 0x02).r(FUNC(gameace_state::rand_r)).nopw();
	map(0x04, 0x04).r(FUNC(gameace_state::rand_r));
	map(0x05, 0x05).nopw();
	map(0x07, 0x07).r(FUNC(gameace_state::rand_r)).nopw();
	map(0x06, 0x06).ram().w(FUNC(gameace_state::bank_w));
}

void gameace_state::sound_program_map(address_map &map) // TODO: banking and everything else
{
	map(0x0000, 0xbfff).rom().region("audiocpu", 0);
	map(0xc000, 0xc000).nopw();
	map(0xc001, 0xc001).nopr().nopw();
	map(0xc002, 0xc002).nopw();
	map(0xc003, 0xc003).nopw();
	map(0xc006, 0xc006).nopr();
	map(0xd000, 0xd7ff).ram();
}


static INPUT_PORTS_START( hotbody )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END


static GFXDECODE_START( gfx )
	//GFXDECODE_ENTRY( "sprites", gfx_8x8x4_planar, , 0, 16 ) // TODO
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x4_planar, 0, 0x40 ) // just enough to see the tiles
GFXDECODE_END


void gameace_state::gameace(machine_config &config)
{
	Z80(config, m_maincpu, 12_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gameace_state::main_program_map);
	m_maincpu->set_addrmap(AS_IO, &gameace_state::main_port_map);
	m_maincpu->set_vblank_int("screen", FUNC(gameace_state::irq0_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", 4_MHz_XTAL));
	audiocpu.set_addrmap(AS_PROGRAM, &gameace_state::sound_program_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: all wrong
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(gameace_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx);
	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x400).set_endianness(ENDIANNESS_LITTLE);

	SPEAKER(config, "mono").front_center();

	YM2151(config, "ym2151", 4_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, "oki", 4_MHz_XTAL / 4, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( hotbody )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "2.14b", 0x00000, 0x40000, CRC(4eff1b0c) SHA1(d2b443b59f50fa9013f528c18b0d38da7c938d22) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "1.4b", 0x00000, 0x20000, CRC(87e15d1d) SHA1(648d29dbf35638639bbf2ffbcd594e455cecaed2) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "3.1f", 0x00000, 0x20000, CRC(680ad651) SHA1(c1e53e7ab0b39d1ab4b6769f64323759ebb976c2) )
	ROM_LOAD( "4.2f", 0x20000, 0x20000, CRC(33d7cf7b) SHA1(8ed80382e727bee8ccfa7c24aac8b3058264c398) )

	ROM_REGION( 0x100000, "tiles", ROMREGION_INVERT ) // contain both Hot Body and Same Same titles GFX
	ROM_LOAD( "6.14g", 0x00000, 0x40000, CRC(c5f744b1) SHA1(0e979f41d7e0a66b45a789384e6a6008e539798a) )
	ROM_LOAD( "8.17g", 0x40000, 0x40000, CRC(4328f371) SHA1(3a5d1c0afb671943234120a0758077f76712f624) )
	ROM_LOAD( "5.13g", 0x80000, 0x40000, CRC(70341256) SHA1(5763351b0c6cb83b4fddd93a2b6a95b96adac148) )
	ROM_LOAD( "7.16g", 0xc0000, 0x40000, CRC(bce62a37) SHA1(8f340af1dd74f2a1b7b13c903abb2806a6a5c6dc) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "5a", 0x00000, 0x20000, CRC(2404da21) SHA1(1333634112eef8664b5d72af5fc57c4c800ce00d) )

	ROM_REGION( 0x800, "plds", ROMREGION_ERASE00 ) // all read protected
	ROM_LOAD( "pal1.4d",  0x000, 0x117, NO_DUMP ) // GAL16V8B-25LP
	ROM_LOAD( "pal2.10c", 0x200, 0x157, NO_DUMP ) // PALCE20V8H-25PC/4
	ROM_LOAD( "pal3.17e", 0x400, 0x117, NO_DUMP ) // GAL16V8B-25LP
	ROM_LOAD( "pal4.1d",  0x600, 0x157, NO_DUMP ) // PALCE20V8H-25PC/4
ROM_END

ROM_START( hotbodya ) // sprites and sound section ROMs match the above, tilemap ROMs differ (maybe censored / uncensored images?)
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "2.14b", 0x00000, 0x40000, NO_DUMP ) // EPROM damaged and micro-fine wires broken

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "1.4b", 0x00000, 0x20000, CRC(87e15d1d) SHA1(648d29dbf35638639bbf2ffbcd594e455cecaed2) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "3.1f", 0x00000, 0x20000, CRC(680ad651) SHA1(c1e53e7ab0b39d1ab4b6769f64323759ebb976c2) )
	ROM_LOAD( "4.2f", 0x20000, 0x20000, CRC(33d7cf7b) SHA1(8ed80382e727bee8ccfa7c24aac8b3058264c398) )

	ROM_REGION( 0x100000, "tiles", ROMREGION_INVERT ) // seem to contain less than the other set, but still have both Hot Body and Same Same titles GFX
	ROM_LOAD( "6.14g", 0x00000, 0x40000, CRC(e922503f) SHA1(78e64af3a5dd57a96c4a74a143e4c1f4ff917036) )
	ROM_LOAD( "8.17g", 0x40000, 0x40000, CRC(909bd6c4) SHA1(14d2c8bb4c7ec8b375c353b0f55026db5c815986) )
	ROM_LOAD( "5.13g", 0x80000, 0x40000, CRC(7251a305) SHA1(4a6e2ae65d909a973178f6b817f3fcc3552b9563) )
	ROM_LOAD( "7.16g", 0xc0000, 0x40000, CRC(02ae2c99) SHA1(2852d1f825d4de9f12a1a46f6bdebf4fac9a955b) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "5a", 0x00000, 0x20000, CRC(2404da21) SHA1(1333634112eef8664b5d72af5fc57c4c800ce00d) )

	ROM_REGION( 0x800, "plds", ROMREGION_ERASE00 ) // all read protected
	ROM_LOAD( "pal1.4d",  0x000, 0x117, NO_DUMP ) // GAL16V8B-25LP
	ROM_LOAD( "pal2.10c", 0x200, 0x157, NO_DUMP ) // PALCE20V8H-25PC/4
	ROM_LOAD( "pal3.17e", 0x400, 0x117, NO_DUMP ) // GAL16V8B-25LP
	ROM_LOAD( "pal4.1d",  0x600, 0x157, NO_DUMP ) // PALCE20V8H-25PC/4
ROM_END


void gameace_state::init_hotbody()
{
	uint8_t* rom = memregion("maincpu")->base();
	std::vector<uint8_t> buffer(0x40000);
	memcpy(&buffer[0], rom, 0x40000);

	for (int i = 0x00000; i < 0x40000; i++) // TODO: simplify this
	{
		uint8_t swap_code[0x10] = { 0x0a, 0x07, 0x08, 0x05,   0x06, 0x03, 0x04, 0x01,   0x02, 0x0f, 0x00, 0x0d,   0x0e, 0x0b, 0x0c, 0x09 };
		uint8_t swap_data[0x10] = { 0x03, 0x02, 0x01, 0x00,   0x05, 0x04, 0x07, 0x06,   0x0d, 0x0c, 0x0f, 0x0e,   0x09, 0x08, 0x0b, 0x0a };

		uint8_t col = i & 0x0f;
		uint32_t addr;

		if (i < 0x10000)
		{
			addr = swap_code[col];
		}
		else
		{
			addr = swap_data[col];
		}

		addr = (i & 0x3fff0) | addr;

		rom[i] = buffer[addr];
	}

	m_membank->configure_entries(0, 0x10, &rom[0x00000], 0x4000);
	m_membank->set_entry(7);
}

} // anonymous namespace


GAME( 1995, hotbody,  0,       gameace, hotbody, gameace_state, init_hotbody, ROT0, "Gameace", "Hot Body (set 1)", MACHINE_IS_SKELETON ) // both 1994 and 1995 strings in ROM
GAME( 1995, hotbodya, hotbody, gameace, hotbody, gameace_state, init_hotbody, ROT0, "Gameace", "Hot Body (set 2)", MACHINE_IS_SKELETON )
