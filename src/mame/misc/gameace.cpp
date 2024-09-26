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

----

basically the hardware is a cost reduced clone of mitchell.cpp with some bits moved around

TODO: is sound emulation complete? there's data in audio ROM at 0xe000, and while we map
      that as ROM space in the CPU, it never appears to read there, so it could be banked
      lower.

*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_MEMBANK     (1U << 1)
#define LOG_PALBANK     (1U << 2)
#define LOG_VIDBANK     (1U << 3)

//#define VERBOSE (LOG_GENERAL | LOG_MEMBANK | LOG_PALBANK | LOG_VIDBANK)

#include "logmacro.h"

#define LOGMEMBANK(...)     LOGMASKED(LOG_MEMBANK,     __VA_ARGS__)
#define LOGPALBANK(...)     LOGMASKED(LOG_PALBANK,     __VA_ARGS__)
#define LOGVIDBANK(...)     LOGMASKED(LOG_VIDBANK,     __VA_ARGS__)


namespace {

class gameace_state : public driver_device
{
public:
	gameace_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_membank(*this, "bank1"),
		m_fgram(*this, "fgram"),
		m_spriteram(*this, "spriteram"),
		m_videoview(*this, "videoview"),
		m_palview(*this, "palview"),
		m_colram(*this, "colram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_ymsnd(*this, "ymsnd")
	{
	}

	void gameace(machine_config &config);

	void init_hotbody();

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_memory_bank m_membank;
	required_shared_ptr<uint8_t> m_fgram;
	required_shared_ptr<uint8_t> m_spriteram;
	memory_view m_videoview;
	memory_view m_palview;
	required_shared_ptr<uint8_t> m_colram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ym2151_device> m_ymsnd;

	std::unique_ptr<uint8_t[]> m_paletteram;

	tilemap_t *m_fg_tilemap = nullptr;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bank_w(uint8_t data);
	void vidbank_w(uint8_t data);
	void palbank_w(uint8_t data);

	void fgram_w(offs_t offset, uint8_t data);
	void colram_w(offs_t offset, uint8_t data);

	uint8_t pal_low_r(offs_t offset, uint8_t data);
	uint8_t pal_high_r(offs_t offset, uint8_t data);
	void pal_low_w(offs_t offset, uint8_t data);
	void pal_high_w(offs_t offset, uint8_t data);

	uint8_t unk_sound_r();

	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void main_program_map(address_map &map) ATTR_COLD;
	void main_port_map(address_map &map) ATTR_COLD;
	void sound_program_map(address_map &map) ATTR_COLD;

	void decode_cpu();
	void decode_sprites();
};

void gameace_state::machine_start()
{
	uint8_t* rom = memregion("maincpu")->base();
	m_membank->configure_entries(0, 0x10, &rom[0x00000], 0x4000);
	m_membank->set_entry(7);
	m_videoview.select(0);
	m_palview.select(0);
}

void gameace_state::draw_sprites(bitmap_ind16& bitmap, const rectangle& cliprect)
{
	// very similar to mitchell.cpp, but with everything on a different byte
	for (int offs = 0x1000 - 0x40; offs >= 0; offs -= 0x20)
	{
		int code = m_spriteram[offs + 0x10];
		int sy = 255 - m_spriteram[offs + 0x11];
		int const attr = m_spriteram[offs + 0x13];
		int const color = attr & 0x0f;
		int sx = m_spriteram[offs + 0x12] + ((attr & 0x10) << 4);
		code += (attr & 0xe0) << 3;
		/*
		if (m_flipscreen)
		{
		    sx = 496 - sx;
		    sy = 240 - sy;
		}
		*/
		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
			code,
			color,
			0, 0,
			sx - 7, sy - 14, 15);
	}
}


uint32_t gameace_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

TILE_GET_INFO_MEMBER(gameace_state::get_fg_tile_info)
{
	int const code = m_fgram[tile_index * 2] | (m_fgram[(tile_index * 2) + 1] << 8);
	int const col = m_colram[tile_index];
	tileinfo.set(0, code, col, 0);
}

void gameace_state::fgram_w(offs_t offset, uint8_t data)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

void gameace_state::colram_w(offs_t offset, uint8_t data)
{
	m_colram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void gameace_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gameace_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_paletteram = std::make_unique<uint8_t[]>(0x1000);
	m_palette->basemem().set(m_paletteram.get(), 0x1000, 8, ENDIANNESS_LITTLE, 2);
	save_pointer(NAME(m_paletteram), 0x1000);
}


void gameace_state::bank_w(uint8_t data)
{
	if (data & 0xf0)
		LOGMEMBANK("bank_w unused bits %02x\n", data & 0xf0);

	m_membank->set_entry(data & 0xf);
}

void gameace_state::palbank_w(uint8_t data)
{
	if (data & 0xdf)
		LOGPALBANK("palbank_w unused bits %02x\n", data & 0xdf);

	m_palview.select((data & 0x20) >> 5);
}


void gameace_state::vidbank_w(uint8_t data)
{
	if (data & 0xfe)
		LOGVIDBANK("videbank_w unused bits %02x\n", data & 0xfe);

	m_videoview.select(data & 1);
}

uint8_t gameace_state::pal_low_r(offs_t offset, uint8_t data)
{
	return m_palette->read8(offset);
}

uint8_t gameace_state::pal_high_r(offs_t offset, uint8_t data)
{
	return m_palette->read8(offset + 0x800);
}

void gameace_state::pal_low_w(offs_t offset, uint8_t data)
{
	m_palette->write8(offset, data);
}

void gameace_state::pal_high_w(offs_t offset, uint8_t data)
{
	m_palette->write8(offset + 0x800, data);
}

void gameace_state::main_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0xbfff).bankr(m_membank);
	map(0xc000, 0xc7ff).view(m_palview);
	m_palview[0](0xc000, 0xc7ff).rw(FUNC(gameace_state::pal_low_r), FUNC(gameace_state::pal_low_w));
	m_palview[1](0xc000, 0xc7ff).rw(FUNC(gameace_state::pal_high_r), FUNC(gameace_state::pal_high_w));
	map(0xc800, 0xcfff).ram().w(FUNC(gameace_state::colram_w)).share(m_colram);
	map(0xd000, 0xdfff).view(m_videoview);
	m_videoview[0](0xd000, 0xdfff).ram().w(FUNC(gameace_state::fgram_w)).share(m_fgram);
	m_videoview[1](0xd000, 0xdfff).ram().share(m_spriteram);
	map(0xe000, 0xefff).ram();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xffff).ram();
}

void gameace_state::main_port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW1");
	map(0x02, 0x02).portr("COINS").w(FUNC(gameace_state::vidbank_w));
	map(0x04, 0x04).portr("P2");
	map(0x05, 0x05).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x06, 0x06).portr("P1");
	map(0x06, 0x06).w(FUNC(gameace_state::bank_w));
	map(0x07, 0x07).portr("UNK").w(FUNC(gameace_state::palbank_w));
}

uint8_t gameace_state::unk_sound_r()
{
	// returning bit 1 set here also causes it to read c00e, then do writes to ROM region, is it a development leftover?
	return 0x00;
}

void gameace_state::sound_program_map(address_map &map)
{
	map(0x0000, 0xbfff).rom().region("audiocpu", 0x0000);
	map(0xc000, 0xc001).rw(m_ymsnd, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xc002, 0xc003).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // it appears to map the oki across 2 addresses
	map(0xc006, 0xc006).r("soundlatch", FUNC(generic_latch_8_device::read));

	map(0xc00f, 0xc00f).r(FUNC(gameace_state::unk_sound_r)).nopw(); // checks bit 1

	map(0xd000, 0xd7ff).ram();
	map(0xe000, 0xffff).rom().region("audiocpu", 0xe000); // maybe, there is data in ROM
}


static INPUT_PORTS_START( hotbody )

	PORT_START("COINS")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("P1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("UNK")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION( "SW1:1,2" )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SW1:8")
INPUT_PORTS_END

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2) + 4, RGN_FRAC(1,2) + 0, 4, 0},
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3, 32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{
		0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16
	},
	64*8
};

static GFXDECODE_START( gfx )
	GFXDECODE_ENTRY( "tiles",   0, gfx_8x8x4_planar, 0, 0x80 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,     0, 0x80 )
GFXDECODE_END


void gameace_state::gameace(machine_config &config)
{
	Z80(config, m_maincpu, 12_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gameace_state::main_program_map);
	m_maincpu->set_addrmap(AS_IO, &gameace_state::main_port_map);
	m_maincpu->set_vblank_int("screen", FUNC(gameace_state::irq0_line_hold));

	Z80(config, m_audiocpu, 4_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &gameace_state::sound_program_map);

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(8*8, 56*8-1, 8, 31*8-1);

	screen.set_screen_update(FUNC(gameace_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx);
	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x800).set_endianness(ENDIANNESS_LITTLE);

	SPEAKER(config, "mono").front_center();

	YM2151(config, m_ymsnd, 4_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0);
	m_ymsnd->irq_handler().set_inputline(m_audiocpu, 0);

	OKIM6295(config, "oki", 4_MHz_XTAL / 4, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( hotbody )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "2.14b", 0x00000, 0x40000, CRC(4eff1b0c) SHA1(d2b443b59f50fa9013f528c18b0d38da7c938d22) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "1.4b", 0x00000, 0x20000, CRC(87e15d1d) SHA1(648d29dbf35638639bbf2ffbcd594e455cecaed2) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
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

ROM_START( hotbody2 ) // sprites and sound section ROMs match the above, tilemap ROMs differ (maybe censored / uncensored images?)
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "2.14b", 0x00000, 0x40000, NO_DUMP ) // EPROM damaged and micro-fine wires broken

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "1.4b", 0x00000, 0x20000, CRC(87e15d1d) SHA1(648d29dbf35638639bbf2ffbcd594e455cecaed2) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "3.1f", 0x00000, 0x20000, CRC(680ad651) SHA1(c1e53e7ab0b39d1ab4b6769f64323759ebb976c2) )
	ROM_LOAD( "4.2f", 0x20000, 0x20000, CRC(33d7cf7b) SHA1(8ed80382e727bee8ccfa7c24aac8b3058264c398) )

	ROM_REGION( 0x100000, "tiles", ROMREGION_INVERT ) // seem to contain less than the other set, but still have both Hot Body and Same Same titles GFX, the Hot Body title set has a 'II' graphic
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

void gameace_state::decode_cpu()
{
	uint8_t* rom = memregion("maincpu")->base();
	std::vector<uint8_t> buffer(0x40000);
	memcpy(&buffer[0], rom, 0x40000);

	constexpr uint8_t swap_code[0x10] = { 0x0a, 0x07, 0x08, 0x05,   0x06, 0x03, 0x04, 0x01,   0x02, 0x0f, 0x00, 0x0d,   0x0e, 0x0b, 0x0c, 0x09 };
	constexpr uint8_t swap_data[0x10] = { 0x03, 0x02, 0x01, 0x00,   0x05, 0x04, 0x07, 0x06,   0x0d, 0x0c, 0x0f, 0x0e,   0x09, 0x08, 0x0b, 0x0a };

	for (int i = 0x00000; i < 0x40000; i++)
	{

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
}

void gameace_state::decode_sprites()
{
	uint8_t* rom = memregion("sprites")->base();
	std::vector<uint8_t> buffer(0x40000);
	memcpy(&buffer[0], rom, 0x40000);

	constexpr uint8_t decode_table[0x20] = {
								   (2<<1)+1, (2<<1),
								   (7<<1),   (7<<1)+1,
								   (0<<1)+1, (0<<1),
								   (5<<1),   (5<<1)+1,
								   (6<<1),   (6<<1)+1,
								   (3<<1),   (3<<1)+1,
								   (4<<1),   (4<<1)+1,
								   (1<<1),   (1<<1)+1,

								   (10<<1)+1,(10<<1),
								   (15<<1),  (15<<1)+1,
								   (8<<1)+1, (8<<1),
								   (13<<1),  (13<<1)+1,
								   (14<<1),  (14<<1)+1,
								   (11<<1),  (11<<1)+1,
								   (12<<1),  (12<<1)+1,
								   (9<<1),   (9<<1)+1
								 };



	for (int i = 0x00000; i < 0x40000; i++)
	{
		uint8_t col = i & 0x1f;
		uint32_t addr = decode_table[col];
		addr = (i & 0x3ffe0) | addr;
		rom[i] = buffer[addr];
	}
}


void gameace_state::init_hotbody()
{
	decode_cpu();
	decode_sprites();
}

} // anonymous namespace


GAME( 1995, hotbody,  0, gameace, hotbody, gameace_state, init_hotbody, ROT0, "Gameace", "Hot Body I",  MACHINE_SUPPORTS_SAVE ) // both 1994 and 1995 strings in ROM
GAME( 1995, hotbody2, 0, gameace, hotbody, gameace_state, init_hotbody, ROT0, "Gameace", "Hot Body II", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // bad dump, no program ROM
