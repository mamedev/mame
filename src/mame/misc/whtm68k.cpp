// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

益智象棋 (Yìzhì Xiàngqí)

TODO:
- Dip-switches (needs sheet);
- Lamps;
- Convert video to use 6845 semantics;
- is there a way to access key test? There are strings in program ROM and GFX in the gfx2 region.

===================================================================================================

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

**************************************************************************************************/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
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
#define LOG_OUTPUTS   (1U << 2)

#define VERBOSE (LOG_GENERAL | LOG_PORTS | LOG_OUTPUTS)
//#define LOG_OUTPUT_FUNC osd_printf_error

#include "logmacro.h"

#define LOGPORTS(...)     LOGMASKED(LOG_PORTS,     __VA_ARGS__)
#define LOGOUTPUTS(...)   LOGMASKED(LOG_OUTPUTS,   __VA_ARGS__)


namespace {

class whtm68k_state : public driver_device
{
public:
	whtm68k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_crtc(*this, "crtc"),
		m_ramdac(*this, "ramdac%u", 0U),
		m_palette(*this, "palette"),
		m_hopper(*this, "hopper"),
		m_bgram(*this, "bgram"),
		m_fgram(*this, "fgram"),
		m_bg_attr(*this, "bg_attr"),
		m_audiobank(*this, "audiobank")
	{ }

	void yizhix(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<hd6845s_device> m_crtc;
	required_device_array<ramdac_device, 2> m_ramdac;
	required_device<palette_device> m_palette;
	required_device<hopper_device> m_hopper;

	required_shared_ptr<uint16_t> m_bgram;
	required_shared_ptr<uint16_t> m_fgram;
	// TODO: uint8?
	required_shared_ptr<uint16_t> m_bg_attr;
	required_memory_bank m_audiobank;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);

	void outputs_w(uint16_t data);

	TILE_GET_INFO_MEMBER(get_4bpp_tile_info);
	TILE_GET_INFO_MEMBER(get_7bpp_tile_info);

	void bgram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fgram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bg_attr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_program_map(address_map &map) ATTR_COLD;
	void audio_program_map(address_map &map) ATTR_COLD;
	void audio_io_map(address_map &map) ATTR_COLD;
	template <uint8_t Which> void ramdac_map(address_map &map) ATTR_COLD;
};

void whtm68k_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(whtm68k_state::get_7bpp_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 128, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(whtm68k_state::get_4bpp_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 128, 32);

	m_bg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);
}

TILE_GET_INFO_MEMBER(whtm68k_state::get_4bpp_tile_info)
{
	uint16_t const tile = m_fgram[tile_index] & 0xfff;
	uint16_t const color = (m_fgram[tile_index] & 0xf000) >> 12;

	tileinfo.set(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(whtm68k_state::get_7bpp_tile_info)
{
	uint16_t const tile = ((m_bgram[tile_index] & 0xfff));
	uint16_t const attr = m_bg_attr[tile_index];
	uint16_t const color = (attr >> 4) & 1;

	tileinfo.set(1, tile, color, 0);
}

uint32_t whtm68k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->pen(0), cliprect);

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

void whtm68k_state::bg_attr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_attr[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

// TODO: unverified sources
TIMER_DEVICE_CALLBACK_MEMBER(whtm68k_state::scanline_cb)
{
	int const scanline = param;

	if (scanline == 256)
		m_maincpu->set_input_line(3, HOLD_LINE);

	if (scanline == 0)
		m_maincpu->set_input_line(1, HOLD_LINE);
}

/*
 * x--- ---- display enable?
 * ---x ---- hopper motor
 * ---- -x-- coin counter (shared)
 */
void whtm68k_state::outputs_w(uint16_t data)
{
	m_hopper->motor_w(BIT(data, 4));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));

	if (data & 0xff6b)
		LOGOUTPUTS("%s unknown outputs_w bits set: %4x\n", machine().describe_context(), data);
}

void whtm68k_state::main_program_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x800001, 0x800001).rw(m_crtc, FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));
	map(0x800003, 0x800003).rw(m_crtc, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0x800101, 0x800101).lw8(
		NAME([this] (offs_t offset, u8 data) {
			// assumed, no access beyond auto-increments
			m_ramdac[0]->index_w(bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7));
		})
	);
	map(0x800103, 0x800103).w(m_ramdac[0], FUNC(ramdac_device::mask_w));
	map(0x800105, 0x800105).w(m_ramdac[0], FUNC(ramdac_device::pal_w));
	map(0x800201, 0x800201).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_ramdac[1]->index_w(bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7));
		})
	);
	map(0x800203, 0x800203).w(m_ramdac[1], FUNC(ramdac_device::mask_w));
	map(0x800205, 0x800205).w(m_ramdac[1], FUNC(ramdac_device::pal_w));
	map(0x810002, 0x810003).portr("IN0");
	map(0x810100, 0x810101).portr("DSW");
	map(0x810200, 0x810201).w(FUNC(whtm68k_state::outputs_w));
	map(0x810300, 0x810301).portr("IN1");
	map(0x810401, 0x810401).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xd00000, 0xd03fff).ram().w(FUNC(whtm68k_state::fgram_w)).share(m_fgram);
	map(0xd10000, 0xd13fff).ram().w(FUNC(whtm68k_state::bgram_w)).share(m_bgram);
	map(0xd20000, 0xd23fff).ram().w(FUNC(whtm68k_state::bg_attr_w)).share(m_bg_attr);
	map(0xd24000, 0xd24001).ram(); // unknown, set once during POST with 0x42
	map(0xe00000, 0xe03fff).ram();
	map(0xe10000, 0xe10fff).ram().share("nvram");
	 // TODO: read continuously during gameplay
	 // branches with 0xaa, 0xbb, 0xcc and 0xee at PC=824c, once per frame
	map(0xe30001, 0xe30001).lr8(NAME([this] (offs_t offset) { return m_screen->frame_number(); }));
}

void whtm68k_state::audio_program_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("audiorom", 0);
	map(0x8000, 0xffff).bankr(m_audiobank);
}

void whtm68k_state::audio_io_map(address_map &map)
{
	map(0x8000, 0x803f).ram(); // ??
	map(0xa000, 0xa000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xb000, 0xb000).r("soundlatch", FUNC(generic_latch_8_device::read)).nopw(); // TODO: write? latch acknowledge?
	map(0xc000, 0xc001).w("ym", FUNC(ym2413_device::write));
}

template <uint8_t Which>
void whtm68k_state::ramdac_map(address_map &map)
{
	map(0x000, 0x2ff).lrw8(
		NAME([this] (offs_t offset)  {
			return bitswap<6>(m_ramdac[Which]->ramdac_pal_r(offset), 2, 3, 4, 5, 6, 7) << 2;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_ramdac[Which]->ramdac_rgb666_w(offset, bitswap<6>(data, 2, 3, 4, 5, 6, 7));
		})
	);
}


static INPUT_PORTS_START( yizhix ) // TODO: possibly some missing inputs
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) // very susceptible. Gives 'coin jam' if pressed for too long
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) // doesn't lock on long presses
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) // called A in game
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // called C in game
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Red") // in roulette double up
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Black") // in roulette double up
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// TODO: these are present on PCB, but do they have any effect?
	// Settings are done via test mode
	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC(0x0001, 0x0001, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0002, 0x0002, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0004, 0x0004, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0008, 0x0008, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x0010, 0x0010, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x0020, 0x0020, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x0040, 0x0040, "SW1:7") // "off line" if set
	PORT_DIPUNKNOWN_DIPLOC(0x0080, 0x0080, "SW1:8")
	PORT_DIPUNKNOWN_DIPLOC(0x0100, 0x0100, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0200, 0x0200, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0400, 0x0400, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0800, 0x0800, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x1000, 0x1000, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x2000, 0x2000, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x4000, 0x4000, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x8000, 0x8000, "SW2:8")
INPUT_PORTS_END


const gfx_layout gfx_8x8x4_packed_msb_r =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 28, 24, 20, 16, 12, 8 },
	{ STEP8(0,4*8) },
	8*8*4
};

// 0 is unused, matters for faces around $880 block
const gfx_layout gfx_8x8x7_packed_msb_r =
{
	8,8,
	RGN_FRAC(1,2),
	7,
	{ 1, 2, 3, RGN_FRAC(1, 2)+0, RGN_FRAC(1, 2)+1, RGN_FRAC(1, 2)+2, RGN_FRAC(1, 2)+3 },
	{ 4, 0, 28, 24, 20, 16, 12, 8 },
	{ STEP8(0,4*8) },
	8*8*4
};

static GFXDECODE_START( gfx_wht )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_msb_r, 0x000, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x7_packed_msb_r, 0x100, 2 )
GFXDECODE_END

void whtm68k_state::machine_start()
{
	m_audiobank->configure_entries(0, 4, memregion("audiobanks")->base() + 0x00000, 0x8000);
}

void whtm68k_state::yizhix(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &whtm68k_state::main_program_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(whtm68k_state::scanline_cb), "screen", 0, 1);

	HOPPER(config, m_hopper, attotime::from_msec(50)); // period is just a guess

	i80c32_device &audiocpu(I80C32(config, "audiocpu", 12_MHz_XTAL));
	audiocpu.set_addrmap(AS_PROGRAM, &whtm68k_state::audio_program_map);
	audiocpu.set_addrmap(AS_IO, &whtm68k_state::audio_io_map);
	audiocpu.port_in_cb<0>().set([this] () { LOGPORTS("%s: 80C32 port 0 read\n", machine().describe_context()); return 0; });
	audiocpu.port_in_cb<1>().set([this] () {
		// TODO: read all the time
		(void)this; /*LOGPORTS("%s: 80C32 port 1 read\n", machine().describe_context());*/ return 0;
	});
	audiocpu.port_in_cb<2>().set([this] () { LOGPORTS("%s: 80C32 port 2 read\n", machine().describe_context()); return 0; });
	audiocpu.port_in_cb<3>().set([this] () { LOGPORTS("%s: 80C32 port 3 read\n", machine().describe_context()); return 0; });
	audiocpu.port_out_cb<0>().set([this] (uint8_t data) { LOGPORTS("%s: 80C32 port 0 write %02x\n", machine().describe_context(), data); });
	audiocpu.port_out_cb<1>().set([this] (uint8_t data) {
		// bit 2 used on coin insertions (soundlatch clear?)
		m_audiobank->set_entry((data & 0x30) >> 4);
		LOGPORTS("%s: 80C32 port 1 write %02x\n", machine().describe_context(), data);
	});
	audiocpu.port_out_cb<2>().set([this] (uint8_t data) { LOGPORTS("%s: 80C32 port 2 write %02x\n", machine().describe_context(), data); });
	audiocpu.port_out_cb<3>().set([this] (uint8_t data) { LOGPORTS("%s: 80C32 port 3 write %02x\n", machine().describe_context(), data); });

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: everything
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 262);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(whtm68k_state::screen_update));

	HD6845S(config, m_crtc, 12_MHz_XTAL / 8);  // clock/divisor guessed
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);

	RAMDAC(config, m_ramdac[0], 0, "palette"); // MU9C4870-80PC
	m_ramdac[0]->set_addrmap(0, &whtm68k_state::ramdac_map<0>);
	m_ramdac[0]->set_color_base(0);

	RAMDAC(config, m_ramdac[1], 0, "palette"); // MU9C4870-80PC
	m_ramdac[1]->set_addrmap(0, &whtm68k_state::ramdac_map<1>);
	m_ramdac[1]->set_color_base(0x100);

	GFXDECODE(config, "gfxdecode", "palette", gfx_wht);
	PALETTE(config, m_palette).set_entries(0x200);

	GENERIC_LATCH_8(config, "soundlatch");

	SPEAKER(config, "mono").front_center();

	YM2413(config, "ym", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0); // KB89C67?

	OKIM6295(config, "oki", 12_MHz_XTAL / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock and pin 7 not verified
}


ROM_START( yizhix )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "chs_p1.u13", 0x00000, 0x20000, CRC(6f180b89) SHA1(cfbdd93360f6f8a8c47624c2522e6c005658a436) )
	ROM_LOAD16_BYTE( "chs_p2.u14", 0x00001, 0x20000, CRC(c8f53b59) SHA1(c8c7e0131e7cbfda59cd658da0f3d4a28deef0b1) )

	ROM_REGION( 0x20000, "audiorom", 0 )
	ROM_LOAD( "chs_m.u74", 0x00000, 0x20000, CRC(b0c030df) SHA1(0cd388dc39004a41cc58ebedab32cc45e338f64b) )

	ROM_REGION( 0x20000, "audiobanks", ROMREGION_ERASEFF )
	// no clue what should map at 3 (unused by the game)
	ROM_COPY( "audiorom", 0x08000, 0x00000, 0x18000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "chs_v1.u50", 0x00000, 0x20000, CRC(dde0d62b) SHA1(bbf0d7dadbeec9036c20a4dfd64a8276c4ff1664) )

	ROM_REGION( 0x40000, "gfx2", 0)
	ROM_LOAD( "chs_v2.u54", 0x00000, 0x20000, CRC(347db59c) SHA1(532d5621ea45fe569e37612f99fed46e6b6fe377) ) // the u54 socket is between u50 and u53 on PCB
	ROM_LOAD( "chs_v3.u53", 0x20000, 0x20000, CRC(36353ce4) SHA1(427c9b946398a38afae850c199438808eee96d80) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "chs_s.u34", 0x00000, 0x20000, CRC(e8492df9) SHA1(08be7cd33b751d56ec830240840adfd841b3af93) ) // 1xxxxxxxxxxxxxxxx = 0x00, very few samples

	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "nvram.bin", 0x0000, 0x1000, CRC(99d772e2) SHA1(59d68e7b03d4005758e01e813fc137e8319fa2c5) ) // default settings
ROM_END

} // anonymous namespace


GAME( 1996, yizhix,  0, yizhix, yizhix,  whtm68k_state, empty_init, ROT0, "WHT", "Yizhi Xiangqi",  MACHINE_NOT_WORKING )
