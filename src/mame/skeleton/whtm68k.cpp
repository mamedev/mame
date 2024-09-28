// license:BSD-3-Clause
// copyright-holders:

/*
Unknown WHT gambling game

The PCB is marked 'COPYRIGHT FOR WHT ELE CO,. VER 3.0'

Main components are:

M68HC000P10 CPU
24.000 MHz XTAL (near CPU)
2x MK48H64N-120 SRAM (near CPU)
HM6116L-70 SRAM (near battery)
P-80C32 audio CPU
HM6116L-70 SRAM (near P-80C32)
GM68B45S CRTC
12.000 MHz XTAL (near CRTC and P-80C32)
2x MU9C4870-80PC RAMDAC
5x HM6265L-70 SRAM (near GFX ROMs)
3x Lattice pLSI 1032-60LJ
KB89C67 (YM2413 clone)
3.579545 MHz XTAL (near KB89C67)
K-665 sound chip (Oki M6295 clone)
2x 8-DIP banks


TODO:
- correct GFX decoding;
- hook up RAMDAC devices / colors;
- audio CPU ROM banking;
- CRTC?;
- inputs;
- lamps.
*/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "video/mc6845.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_PORTS     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_PORTS)

#include "logmacro.h"

#define LOGPORTS(...)     LOGMASKED(LOG_PORTS,     __VA_ARGS__)


namespace {

class whtm68k_state : public driver_device
{
public:
	whtm68k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_bgram(*this, "bgram"),
		m_fgram(*this, "fgram")
	{ }

	void unkwht(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint16_t> m_bgram;
	required_shared_ptr<uint16_t> m_fgram;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void bgram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fgram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_program_map(address_map &map) ATTR_COLD;
	void audio_program_map(address_map &map) ATTR_COLD;
	void audio_io_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
	void ramdac2_map(address_map &map) ATTR_COLD;
};


void whtm68k_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(whtm68k_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(whtm68k_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_fg_tilemap->set_transparent_pen(0);
}

// TODO: wrong, just to show something
TILE_GET_INFO_MEMBER(whtm68k_state::get_bg_tile_info)
{
	uint16_t const tile = m_bgram[tile_index] & 0xfff;
	uint16_t const attr = m_bgram[tile_index] & 0xf000 >> 12;
	tileinfo.set(0, tile, attr, 0);
}

TILE_GET_INFO_MEMBER(whtm68k_state::get_fg_tile_info)
{
	uint16_t const tile = ((m_fgram[tile_index] & 0xfff) | 0x1000); // TODO: actually find tile bank
	uint16_t const attr = m_fgram[tile_index] & 0xf000 >> 12;
	tileinfo.set(0, tile, attr, 0);
}

uint32_t whtm68k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void whtm68k_state::bgram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bgram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

void whtm68k_state::fgram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fgram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}


void whtm68k_state::main_program_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x800100, 0x800101).nopw(); //TODO: map(0x800101, 0x800101).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x800102, 0x800103).nopw(); //TODO: map(0x800103, 0x800103).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x800104, 0x800105).nopw(); //TODO: map(0x800105, 0x800105).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x800200, 0x800201).nopw(); //TODO: map(0x800201, 0x800201).w("ramdac2", FUNC(ramdac_device::index_w));
	map(0x800202, 0x800203).nopw(); //TODO: map(0x800203, 0x800203).w("ramdac2", FUNC(ramdac_device::mask_w));
	map(0x800204, 0x800205).nopw(); //TODO: map(0x800205, 0x800205).w("ramdac2", FUNC(ramdac_device::pal_w));
	map(0x810002, 0x810003).portr("IN0");
	map(0x810100, 0x810101).portr("DSW1"); // ??
	map(0x810300, 0x810301).portr("DSW2"); // ??
	map(0x810401, 0x810401).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xd00000, 0xd03fff).ram().w(FUNC(whtm68k_state::bgram_w)).share(m_bgram);
	map(0xd10000, 0xd13fff).ram().w(FUNC(whtm68k_state::fgram_w)).share(m_fgram);
	map(0xd20000, 0xd23fff).ram(); // attribute RAM? or third bitmap?
	map(0xe00000, 0xe03fff).ram(); // work RAM?
	map(0xe10000, 0xe10fff).ram().share("nvram");
	map(0xe30000, 0xe30001).nopr(); // TODO: read continuously during gameplay
}

void whtm68k_state::audio_program_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x8000, 0xffff).rom().region("audiocpu", 0x8000); // TODO: banked somewhere here
}

void whtm68k_state::audio_io_map(address_map &map)
{
	map(0x8000, 0x803f).ram(); // ??
	map(0xa000, 0xa000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xb000, 0xb000).r("soundlatch", FUNC(generic_latch_8_device::read)).nopw(); // TODO: write? latch acknowledge?
	map(0xc000, 0xc001).w("ym", FUNC(ym2413_device::write));
}

void whtm68k_state::ramdac_map(address_map &map)
{
	map(0x000, 0x2ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void whtm68k_state::ramdac2_map(address_map &map)
{
	map(0x000, 0x2ff).rw("ramdac2", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}


static INPUT_PORTS_START( unkwht )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4) // Coin related? Gives 'coin jam if pressed'
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4) // Coin
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(4)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
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


 // TODO: wrong, just enough to see something in the decoder
const gfx_layout gfx_8x8x4_packed_msb_r =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(3,-1) },
	{ STEP8(28,-4) },
	{ STEP8(0,4*8) },
	8*8*4
};

static GFXDECODE_START( gfx_wht )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x4_packed_msb_r, 0, 16 )
GFXDECODE_END


void whtm68k_state::unkwht(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &whtm68k_state::main_program_map);
	m_maincpu->set_vblank_int("screen", FUNC(whtm68k_state::irq1_line_hold));

	i80c32_device &audiocpu(I80C32(config, "audiocpu", 12_MHz_XTAL));
	audiocpu.set_addrmap(AS_PROGRAM, &whtm68k_state::audio_program_map);
	audiocpu.set_addrmap(AS_IO, &whtm68k_state::audio_io_map);
	audiocpu.port_in_cb<0>().set([this] () { LOGPORTS("%s: 80C32 port 0 read\n", machine().describe_context()); return 0; });
	audiocpu.port_in_cb<1>().set([this] () { LOGPORTS("%s: 80C32 port 1 read\n", machine().describe_context()); return 0; }); // TODO: read all the time
	audiocpu.port_in_cb<2>().set([this] () { LOGPORTS("%s: 80C32 port 2 read\n", machine().describe_context()); return 0; });
	audiocpu.port_in_cb<3>().set([this] () { LOGPORTS("%s: 80C32 port 3 read\n", machine().describe_context()); return 0; });
	audiocpu.port_out_cb<0>().set([this] (uint8_t data) { LOGPORTS("%s: 80C32 port 0 write %02x\n", machine().describe_context(), data); });
	audiocpu.port_out_cb<1>().set([this] (uint8_t data) { LOGPORTS("%s: 80C32 port 1 write %02x\n", machine().describe_context(), data); });
	audiocpu.port_out_cb<2>().set([this] (uint8_t data) { LOGPORTS("%s: 80C32 port 2 write %02x\n", machine().describe_context(), data); });
	audiocpu.port_out_cb<3>().set([this] (uint8_t data) { LOGPORTS("%s: 80C32 port 3 write %02x\n", machine().describe_context(), data); });

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: everything
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 512);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(whtm68k_state::screen_update));

	hd6845s_device &crtc(HD6845S(config, "crtc", 12_MHz_XTAL / 12));  // divisor guessed
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);

	RAMDAC(config, "ramdac", 0, "palette").set_addrmap(0, &whtm68k_state::ramdac_map); // MU9C4870-80PC
	RAMDAC(config, "ramdac2", 0, "palette2").set_addrmap(0, &whtm68k_state::ramdac2_map); // MU9C4870-80PC

	GFXDECODE(config, "gfxdecode", "palette", gfx_wht);
	PALETTE(config, "palette").set_entries(0x100); // TODO
	PALETTE(config, "palette2").set_entries(0x100); // TODO

	GENERIC_LATCH_8(config, "soundlatch");

	SPEAKER(config, "mono").front_center();

	YM2413(config, "ym", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0); // KB89C67?

	OKIM6295(config, "oki", 12_MHz_XTAL / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock and pin 7 not verified
}


ROM_START( unkwht )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "chs_p1.u13", 0x00000, 0x20000, CRC(6f180b89) SHA1(cfbdd93360f6f8a8c47624c2522e6c005658a436) )
	ROM_LOAD16_BYTE( "chs_p2.u14", 0x00001, 0x20000, CRC(c8f53b59) SHA1(c8c7e0131e7cbfda59cd658da0f3d4a28deef0b1) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "chs_m.u74", 0x00000, 0x20000, CRC(b0c030df) SHA1(0cd388dc39004a41cc58ebedab32cc45e338f64b) )

	ROM_REGION( 0x60000, "tiles", 0 )
	ROM_LOAD( "chs_v1.u50", 0x00000, 0x20000, CRC(dde0d62b) SHA1(bbf0d7dadbeec9036c20a4dfd64a8276c4ff1664) )
	ROM_LOAD( "chs_v2.u54", 0x20000, 0x20000, CRC(347db59c) SHA1(532d5621ea45fe569e37612f99fed46e6b6fe377) ) // the u54 socket is between u50 and u53 on PCB
	ROM_LOAD( "chs_v3.u53", 0x40000, 0x20000, CRC(36353ce4) SHA1(427c9b946398a38afae850c199438808eee96d80) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "chs_s.u34", 0x00000, 0x20000, CRC(e8492df9) SHA1(08be7cd33b751d56ec830240840adfd841b3af93) ) // 1xxxxxxxxxxxxxxxx = 0x00, very few samples
ROM_END

} // anonymous namespace


GAME( 1996, unkwht,  0, unkwht, unkwht,  whtm68k_state, empty_init, ROT0, "WHT", "unknown WHT gambling game", MACHINE_IS_SKELETON ) // 1996 WHT copyright in GFX
