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
- everything. Currently stops with 'BK RAM ERROR'.
*/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
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
		m_txtram(*this, "txtram")
	{ }

	void unkwht(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint16_t> m_txtram;

	tilemap_t *m_txt_tilemap = nullptr;

	TILE_GET_INFO_MEMBER(get_txt_tile_info);
	void txtram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_program_map(address_map &map) ATTR_COLD;
	void audio_program_map(address_map &map) ATTR_COLD;
	void audio_io_map(address_map &map) ATTR_COLD;
};


void whtm68k_state::video_start()
{
	m_txt_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(whtm68k_state::get_txt_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

// TODO: wrong, just to show something
TILE_GET_INFO_MEMBER(whtm68k_state::get_txt_tile_info)
{
	int const tile = m_txtram[tile_index] & 0xfff;
	tileinfo.set(0, tile, 0, 0);
}

uint32_t whtm68k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	m_txt_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void whtm68k_state::txtram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_txtram[offset]);
	m_txt_tilemap->mark_tile_dirty(offset);
}

void whtm68k_state::main_program_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0xd00000, 0xd03fff).ram().w(FUNC(whtm68k_state::txtram_w)).share(m_txtram);
	map(0xd10000, 0xd13fff).ram();
	map(0xd20000, 0xd23fff).ram();
	map(0xe00000, 0xe03fff).ram();
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
	 // some kind of latch with the main CPU or inputs? Returning rand activates the sound chips.
	map(0xb000, 0xb000).nopr();//lr8(NAME([this] () -> uint8_t { return machine().rand(); }));
	map(0xc000, 0xc001).w("ym", FUNC(ym2413_device::write));
}


static INPUT_PORTS_START( unkwht )
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
	{ STEP4(0,1) },
	{ STEP8(28,-4) },
	{ STEP8(0,4*8) },
	8*8*4
};

static GFXDECODE_START( gfx_wht )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x4_packed_msb_r,  0, 16 )
GFXDECODE_END


void whtm68k_state::unkwht(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &whtm68k_state::main_program_map);
	m_maincpu->set_vblank_int("screen", FUNC(whtm68k_state::irq2_line_hold));

	i80c32_device &audiocpu(I80C32(config, "audiocpu", 12_MHz_XTAL));
	audiocpu.set_addrmap(AS_PROGRAM, &whtm68k_state::audio_program_map);
	audiocpu.set_addrmap(AS_IO, &whtm68k_state::audio_io_map);
	audiocpu.port_in_cb<0>().set([this] () { LOGPORTS("%s: 80C32 port 0 read\n", machine().describe_context()); return 0; });
	audiocpu.port_in_cb<1>().set([this] () { LOGPORTS("%s: 80C32 port 1 read\n", machine().describe_context()); return 0; });
	audiocpu.port_in_cb<2>().set([this] () { LOGPORTS("%s: 80C32 port 2 read\n", machine().describe_context()); return 0; });
	audiocpu.port_in_cb<3>().set([this] () { LOGPORTS("%s: 80C32 port 3 read\n", machine().describe_context()); return 0; });
	audiocpu.port_out_cb<0>().set([this] (uint8_t data) { LOGPORTS("%s: 80C32 port 0 write %02x\n", machine().describe_context(), data); });
	audiocpu.port_out_cb<1>().set([this] (uint8_t data) { LOGPORTS("%s: 80C32 port 1 write %02x\n", machine().describe_context(), data); });
	audiocpu.port_out_cb<2>().set([this] (uint8_t data) { LOGPORTS("%s: 80C32 port 2 write %02x\n", machine().describe_context(), data); });
	audiocpu.port_out_cb<3>().set([this] (uint8_t data) { LOGPORTS("%s: 80C32 port 3 write %02x\n", machine().describe_context(), data); });

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: everything
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(whtm68k_state::screen_update));

	hd6845s_device &crtc(HD6845S(config, "crtc", 12_MHz_XTAL / 12));  // divisor guessed
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);

	RAMDAC(config, "ramdac", 0, "palette"); // MU9C4870-80PC
	RAMDAC(config, "ramdac2", 0, "palette"); // MU9C4870-80PC

	GFXDECODE(config, "gfxdecode", "palette", gfx_wht);
	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x400); // TODO

	SPEAKER(config, "mono").front_center();

	YM2413(config, "ym",  3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0); // KB89C67?

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
