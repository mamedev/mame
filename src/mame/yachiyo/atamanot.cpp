// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Computer Quiz Atama no Taisou

Based off yachiyo/ssingles.cpp

 TODO:
 - Incredibly complex bank system;
 - Whatever is IC28 (code in l.bin?). Controls NVRAM, RNG answers, program flow vectors (at $4000);
 - vsync irq (NMI just like ssingles?)
 - Inputs, key matrix based?
 - colors (missing PROM(s) ?)

===================================================================================================

Computer Quiz Atama no Taisou
(c)1983 Yachiyo Denki / Uni Enterprise

-----------------------------------------
TZU-093
CPU: Z80(IC9) surface scratched Z80?(IC28)
Sound: AY-3-8910 x2
OSC: 14.000MHz
RAMs:
IC5 (probably 6116)
IC4 (probably 6116)
IC45 (probably 6116)
IC44 (probably 6116)

M5L5101LP-1(IC19,20,21,22)
-----------------------------------------
ROMs:
TT1.2        [da9e270d] (2764)
TT2.3        [7595ade8] /

CA.49        [28d20b52] (2764)
CC.48        [209cab0d]  |
CB.47        [8bc85c0c]  |
CD.46        [22e8d103] /

IC36.BIN     [643e3077] (surface scratched, 27C256)
IC35.BIN     [fe0302a0]  |
IC34.BIN     [06e7c7da]  |
IC33.BIN     [323a70e7] /

color PROMs:
1.52         [13f5762b] (82S129)
2.53         [4142f525]  |
3.54         [88acb21e] /


-----------------------------------------
Sub board
CQM-082-M
RAM:
HM6116LP-3
Other:
Battery
8-position DIPSW
-----------------------------------------
ROMs:
TA.BIN       [5c61edaf] (2764)
TB.BIN       [07bd2e6f]  |
TC.BIN       [1e09ac09]  |
TD.BIN       [bd514d51]  |
TE.BIN       [825ed49f]  |
TF.BIN       [d92b5eb9]  |
TG.BIN       [eb25aa72]  |
TH.BIN       [13396cfb] /

TI.BIN       [60193df3] (2764)
J.BIN        [cd16ddbf]  |
K.BIN        [c75c7a1e]  |
L.BIN        [dbb4ed60] /


/////////////////////////////////////////

DIPSW
1: off
2: off
3: coinage
    on: 1coin 2credits
   off: 1coin 1credit
4: Note
    on: show notes
   off: game
5: Test mode
    on: test
   off: game
6: Attract sound
    on: no
   off: yes
7-8: Timer setting
   off-off: quickest
    on-off: quick
   off- on: slow
    on- on: slowest
(default settings: all off)


wiring
GND(sw)   A01|B01 GND(speaker)
GND(power)A02|B02 GND(power)
+5V       A03|B03 +5V
--------  A04|B04 +5V(coin counter)
+12V      A05|B05 +12V
--------  A06|B06 speaker

    (A7-A15, B7-B15: NC)

2P genre1 A16|B16 1P genre1
2P genre2 A17|B17 1P genre2
2P genre3 A18|B18 1P genre3
2P genre4 A19|B19 1P genre4
2P push A A20|B20 1P push A
2P push B A21|B21 1P push B
2P push C A22|B22 1P push C
Flip/flop A23|B23 Flip/flop
--------  A24|B24 --------
GREEN     A25|B25 BLUE
RED       A26|B26 SYNC
GND       A27|B27 GND
GND(video)A28|B28 coin sw


/////////////////////////////////////////

--- Team Japump!!! ---
Dumped by Chack'n
04/Nov/2009

**************************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "logmacro.h"

namespace {

static constexpr uint8_t NUM_PENS = 4 * 8;

class atamanot_state : public driver_device
{
public:
	atamanot_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_videoram(*this, "videoram")
		, m_colorram(*this, "colorram")
		, m_gfx_rom(*this, "gfx")
		, m_kanji_rom(*this, "kanji")
		, m_bank(*this, "bank")
		//, m_extra(*this, "EXTRA")
	{ }

	void atamanot(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_region_ptr<uint8_t> m_gfx_rom;
	required_region_ptr<uint8_t> m_kanji_rom;
	required_device<address_map_bank_device> m_bank;

//	required_ioport m_extra;

	bool m_nmi_enable = false;
	pen_t m_pens[NUM_PENS];

	void palette(palette_device &palette) const;

	uint8_t atamanot_prot_r(offs_t offset);
	void atamanot_prot_w(uint8_t data);

	void atamanot_irq(int state);
	MC6845_UPDATE_ROW(atamanot_update_row);

	void main_io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void bank_map(address_map &map) ATTR_COLD;
};

// fake palette
// TODO: copied from ssingles, most likely wrong (cfr. service mode color bars)
static constexpr rgb_t atamanot_colors[NUM_PENS] =
{
	{ 0x00,0x00,0x00 }, { 0xff,0xff,0xff }, { 0xff,0x00,0x00 }, { 0xaa,0x00,0x00 },
	{ 0x00,0x00,0x00 }, { 0xff,0xff,0xff }, { 0xff,0xaa,0x00 }, { 0xaa,0x55,0x00 },
	{ 0x00,0x00,0x00 }, { 0xff,0xff,0xff }, { 0xff,0x00,0x00 }, { 0xff,0xaa,0x00 },
	{ 0x00,0x00,0x00 }, { 0xff,0xaa,0x00 }, { 0xff,0x00,0x00 }, { 0xff,0x55,0x00 },
	{ 0x00,0x00,0x00 }, { 0xff,0xff,0x00 }, { 0xff,0xaa,0x00 }, { 0xaa,0x55,0x00 },
	{ 0x00,0x00,0x00 }, { 0xff,0x00,0x00 }, { 0x00,0xff,0x00 }, { 0xff,0xff,0x00 },
	{ 0x00,0x00,0x00 }, { 0x00,0x00,0xff }, { 0xaa,0xff,0x00 }, { 0xff,0xaa,0x00 },
	{ 0x00,0x00,0x00 }, { 0xff,0x00,0xff }, { 0xaa,0x00,0xaa }, { 0x55,0x00,0x55 }
};


void atamanot_state::palette(palette_device &palette) const
{
	for (int i = 0; i < NUM_PENS; ++i)
		palette.set_pen_color(i, atamanot_colors[i]);
}

void atamanot_state::video_start()
{
	for (int i = 0; i < NUM_PENS; ++i)
		m_pens[i] = atamanot_colors[i];
}


MC6845_UPDATE_ROW(atamanot_state::atamanot_update_row)
{
	for (int cx = 0; cx < x_count; ++cx)
	{
		int const address = ((ma >> 1) + (cx >> 1)) & 0xff;

		uint16_t const cell = m_videoram[address] + (m_colorram[address] << 8);

		// attr bit 7 is kanji ROM enable
		if (BIT(cell, 15))
		{
			// TODO: bank bits not understood, unknown 8x16 LR enable bit/semantics
			uint32_t const tile_address = ((cell & 0x7ff) << 4) + (BIT(cell, 8 + 3) << 3) + (ra & 7);
			uint16_t const palette = 0;

			uint16_t const cxo = (cx & 1) ? 0x8000 : 0;
			uint8_t b0 = m_kanji_rom[tile_address + (BIT(ra, 3) ? 0x10000 : 0) + cxo];

			for (int x = 7; x >= 0; --x)
			{
				bitmap.pix(y, (cx << 3) | x) = m_pens[palette + (b0 & 1)];
				b0 >>= 1;
			}
		}
		else
		{
			// TODO: c&p from yachiyo/ssingles.cpp, entirely unverified so far
			uint32_t const tile_address = ((cell & 0x1ff) << 4) + ra;
			uint16_t const palette = (cell >> 10) & 0x1c;

			uint16_t const cxo = (cx & 1) ? 0x2000 : 0;
			uint8_t b0 = m_gfx_rom[tile_address + 0x0000 + cxo];
			uint8_t b1 = m_gfx_rom[tile_address + 0x4000 + cxo];

			for (int x = 7; x >= 0; --x)
			{
				bitmap.pix(y, (cx << 3) | x) = m_pens[palette + ((b0 & 1) | ((b1 & 1) << 1))];
				b0 >>= 1;
				b1 >>= 1;
			}
		}
	}
}

// NVRAM, controlled by the unknown device?
uint8_t atamanot_state::atamanot_prot_r(offs_t offset)
{
	static const char prot_id[] = { "PROGRAM BY KOYAMA" };

	return prot_id[offset % 0x11];
}

void atamanot_state::atamanot_prot_w(uint8_t data)
{
	// TODO: maybe not all of it
	// 0x60 in particular looks some kind of sync with the other device
	// cfr. PC=13C8 loop where 13D7~8 are nop-ed
	m_bank->set_bank(data);
}

void atamanot_state::main_map(address_map &map)
{
	map(0x0000, 0x00ff).writeonly().share(m_videoram);
	map(0x0800, 0x08ff).writeonly().share(m_colorram);
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x6000, 0x60ff).ram(); // ?
//  map(0x6000, 0x7fff).rom();
	// NOTE: granularity guessed
	map(0x8000, 0x9fff).rw(m_bank, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
}

void atamanot_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("ay1", FUNC(ay8910_device::address_w));
	map(0x04, 0x04).w("ay1", FUNC(ay8910_device::data_w));
	map(0x06, 0x06).w("ay2", FUNC(ay8910_device::address_w));
	map(0x08, 0x08).nopr();
	map(0x0a, 0x0a).w("ay2", FUNC(ay8910_device::data_w));
//	map(0x16, 0x16).portr("DSW0");
	map(0x18, 0x18).w(FUNC(atamanot_state::atamanot_prot_w));
	map(0x1c, 0x1c).nopw(); // noisy, key matrix select for $1e?
	map(0x1a, 0x1a).lw8(NAME([this] (offs_t offset, u8 data) {
		// unconfirmed
		m_nmi_enable = !!BIT(data, 0);
		// bit 1 looks always high, video enable?
		if (data & 0xfc)
			logerror("I/O $1e: %02x\n", data);
	}));
//	map(0x1e, 0x1e) unknown, read a lot with mask & 7
	map(0x1e, 0x1e).portr("INPUTS");
	map(0xfe, 0xfe).w("crtc", FUNC(mc6845_device::address_w));
	map(0xff, 0xff).w("crtc", FUNC(mc6845_device::register_w));
}

void atamanot_state::bank_map(address_map &map)
{
//	map(0x000000, 0x001fff).rom().region("question", 0x0e000);
	// border in example question
	map(0x002000, 0x003fff).rom().region("question", 0x14000);
	// example question
	map(0x004000, 0x005fff).rom().region("question", 0x0c000);
//	map(0x006000, 0x007fff).rom().region("question", 0x0c000);
//	map(0x008000, 0x009fff).rom().region("question", 0x06000);
//	map(0x00a000, 0x00bfff).rom().region("question", 0x0e000);
//	map(0x00c000, 0x00dfff).rom().region("question", 0x02000);
//	map(0x00e000, 0x00ffff).rom().region("question", 0x00000);

	// lower GFXs of example question
	map(0x016000, 0x017fff).rom().region("question", 0x04000);

//	map(0x07e000, 0x07ffff).rom().region("question", 0x0e000);

	// Note counters and RNG answers maps here
//	map(0x040000, 0x04ffff).lr8(NAME([this] (offs_t offset) { return machine().rand(); }));
	map(0x040000, 0x04000f).r(FUNC(atamanot_state::atamanot_prot_r));
	map(0x042110, 0x04211f).r(FUNC(atamanot_state::atamanot_prot_r));
	map(0x044220, 0x04422f).r(FUNC(atamanot_state::atamanot_prot_r));
	map(0x046330, 0x04633f).r(FUNC(atamanot_state::atamanot_prot_r));

	map(0x0c0000, 0x0c1fff).rom().region("question", 0x16000);

	// 2 goes to what it seems an analyzer, with "NOTE 2" as header
	// 1 draws a "Sound" NOTE 1
	// 0 draws a "System check, please wait"
	// an example question can be checked with a '1' then fiddling with inputs and:
	// bp 4000,1,{pc=c85;g}
	map(0x180000, 0x180000).lr8(NAME([] () { return 2; }));
}

// cabinet (and non-JAMMA pinout) shows 4 + 3 buttons, arranged as:
// genre   ~ buttons
// 1-2-3-4 ~ A-B-C
// layout looks matrix-ed
static INPUT_PORTS_START( atamanot )
	PORT_START("INPUTS")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

/*
atamanot kanji gfx decoding:

It looks "stolen" from an unknown Japanese computer?
*/

static const gfx_layout layout_16x16 =
{
	16,16,
	RGN_FRAC(1,4),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		RGN_FRAC(1,4)+0, RGN_FRAC(1,4)+1, RGN_FRAC(1,4)+2, RGN_FRAC(1,4)+3, RGN_FRAC(1,4)+4, RGN_FRAC(1,4)+5, RGN_FRAC(1,4)+6, RGN_FRAC(1,4)+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		RGN_FRAC(2,4)+0*8, RGN_FRAC(2,4)+1*8, RGN_FRAC(2,4)+2*8, RGN_FRAC(2,4)+3*8, RGN_FRAC(2,4)+4*8, RGN_FRAC(2,4)+5*8, RGN_FRAC(2,4)+6*8, RGN_FRAC(2,4)+7*8 },
	8*8
};

static const gfx_layout layout_8x16 =
{
	8,16,
	RGN_FRAC(1,2),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		RGN_FRAC(2,4)+0*8, RGN_FRAC(2,4)+1*8, RGN_FRAC(2,4)+2*8, RGN_FRAC(2,4)+3*8, RGN_FRAC(2,4)+4*8, RGN_FRAC(2,4)+5*8, RGN_FRAC(2,4)+6*8, RGN_FRAC(2,4)+7*8 },
	8*8
};


static GFXDECODE_START( gfx_atamanot )
	GFXDECODE_ENTRY( "gfx",      0, gfx_8x8x2_planar,   0, 8 )
	GFXDECODE_ENTRY( "kanji",    0, layout_16x16,       0, 8 )
	GFXDECODE_ENTRY( "kanji_uc", 0, layout_8x16,        0, 8 )
	GFXDECODE_ENTRY( "kanji_lc", 0, layout_8x16,        0, 8 )
GFXDECODE_END

void atamanot_state::machine_start()
{
	save_item(NAME(m_nmi_enable));
}

void atamanot_state::machine_reset()
{
	m_nmi_enable = false;
}


void atamanot_state::atamanot_irq(int state)
{
	if (m_nmi_enable)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, state);
	}
}

void atamanot_state::atamanot(machine_config &config)
{
	Z80(config, m_maincpu, 14_MHz_XTAL / 4); // 3.5 MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &atamanot_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &atamanot_state::main_io_map);

	ADDRESS_MAP_BANK(config, m_bank);
	m_bank->set_options(ENDIANNESS_LITTLE, 8, 21, 0x2000);
	m_bank->set_map(&atamanot_state::bank_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(14_MHz_XTAL / 2, 400, 0, 288, 293, 0, 192); // from CRTC
	m_screen->set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, "palette", FUNC(atamanot_state::palette), NUM_PENS);

	GFXDECODE(config, "gfxdecode", "palette", gfx_atamanot);

	mc6845_device &crtc(MC6845(config, "crtc", 14_MHz_XTAL / 16));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(atamanot_state::atamanot_update_row));
	crtc.out_vsync_callback().set(FUNC(atamanot_state::atamanot_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay1", 14_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.5);
	AY8910(config, "ay2", 14_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.5);
}


ROM_START( atamanot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tt1.2",    0x0000, 0x2000, CRC(da9e270d) SHA1(b7408be913dad8abf022c6153f2493204dd74952) )
	ROM_LOAD( "tt2.3",    0x2000, 0x2000, CRC(7595ade8) SHA1(71f9d6d987407f88cdd3b28bd1e35e00ac17e1f5) )

	ROM_REGION( 0x18000, "question", 0 ) // question ROMs?
	// kanji GFXs
	ROM_LOAD( "ta.bin",   0x0e000, 0x2000, CRC(5c61edaf) SHA1(ea56df6b320aa7e52828aaccbb5838cd0c756f24) )
	ROM_LOAD( "tb.bin",   0x0c000, 0x2000, CRC(07bd2e6f) SHA1(bf245d8208db447572e484057b9daa6276f03683) )
	ROM_LOAD( "tc.bin",   0x0a000, 0x2000, CRC(1e09ac09) SHA1(91ec1b2c5767b5bad8915f7c9984f423fcb399c9) )
	ROM_LOAD( "td.bin",   0x08000, 0x2000, CRC(bd514d51) SHA1(1a1e95558b2608f0103ca1b42fe9e59ccb90487f) )
	ROM_LOAD( "te.bin",   0x06000, 0x2000, CRC(825ed49f) SHA1(775044f6d53ecbfa0ab604947a21e368bad85ce0) )
	ROM_LOAD( "tf.bin",   0x04000, 0x2000, CRC(d92b5eb9) SHA1(311fdefdc1f1026cb7f7cc1e1adaffbdbe7a70d9) )
	ROM_LOAD( "tg.bin",   0x02000, 0x2000, CRC(eb25aa72) SHA1(de3a3d87a2eb540b96947f776c321dc9a7c21e78) )
	ROM_LOAD( "th.bin",   0x00000, 0x2000, CRC(13396cfb) SHA1(d98ea4ff2e2175aa7003e37001664b3fa898c071) )
	// regular GFXs
	ROM_LOAD( "ti.bin",   0x10000, 0x2000, CRC(60193df3) SHA1(58840bde303a760a0458224983af0c0bbe939a2f) )
	ROM_LOAD( "j.bin",    0x12000, 0x2000, CRC(cd16ddbf) SHA1(b418b5d73d3699697ebd42a6f4df598dcdcaf264) )
	ROM_LOAD( "k.bin",    0x14000, 0x2000, CRC(c75c7a1e) SHA1(59b136626267fa3ba5a2e1709acb632142e1560e) )
	// code?
	ROM_LOAD( "l.bin",    0x16000, 0x2000, CRC(dbb4ed60) SHA1(b5054ba3ccd268594d22e1e67f70bb227095ca4c) )

	ROM_REGION( 0x8000, "gfx", 0 )
	ROM_LOAD( "ca.49",    0x0000, 0x2000, CRC(28d20b52) SHA1(a104ef1cd103f31803b88bd2d4804eab5a26e7fa) )
	ROM_LOAD( "cc.48",    0x2000, 0x2000, CRC(209cab0d) SHA1(9a89af1f7186e4845e43f9cdafd273e69d280bfb) )
	ROM_LOAD( "cb.47",    0x4000, 0x2000, CRC(8bc85c0c) SHA1(64701bc910c28666d15ee22f59f32888cc2302ae) )
	ROM_LOAD( "cd.46",    0x6000, 0x2000, CRC(22e8d103) SHA1(f0146f7e192eef8d03404a9c5b8a9f9c9577d936) )

	ROM_REGION( 0x20000, "kanji", 0 )
	ROM_LOAD( "ic36.bin", 0x18000, 0x8000, CRC(643e3077) SHA1(fa81a3a3eebd59c6dc9c9b7eeb4a480bb1440c17) )
	ROM_LOAD( "ic35.bin", 0x10000, 0x8000, CRC(fe0302a0) SHA1(f8d3a58da4e8dd09db240039f5216e7ebe9cc384) )
	ROM_LOAD( "ic34.bin", 0x08000, 0x8000, CRC(06e7c7da) SHA1(a222c0b0eccfda613f916320e6afbb33385921ba) )
	ROM_LOAD( "ic33.bin", 0x00000, 0x8000, CRC(323a70e7) SHA1(55e570f039c97d15b06bfcb1ebf03562cbcf8324) )

	ROM_REGION( 0x10000, "kanji_uc", 0 ) // upper case
	ROM_COPY( "kanji",    0x10000, 0x08000, 0x08000 )
	ROM_COPY( "kanji",    0x00000, 0x00000, 0x08000 )

	ROM_REGION( 0x10000, "kanji_lc", 0 ) // lower case
	ROM_COPY( "kanji",    0x18000, 0x08000, 0x08000 )
	ROM_COPY( "kanji",    0x08000, 0x00000, 0x08000 )

	ROM_REGION( 0x0300,  "proms", 0 ) // NOT color proms
	ROM_LOAD( "1.52",     0x0000, 0x0100, CRC(13f5762b) SHA1(da9cc51eda0681b0d3c17b212d23ab89af2813ff) )
	ROM_LOAD( "2.53",     0x0100, 0x0100, CRC(4142f525) SHA1(2e2e896ba7b49df9cf3fddff6becc07a3d50d926) )
	ROM_LOAD( "3.54",     0x0200, 0x0100, CRC(88acb21e) SHA1(18fe5280dad6687daf6bf42d37dde45157fab5e3) )
ROM_END


} // anonymous namespace

GAME( 1983, atamanot, 0, atamanot, atamanot, atamanot_state, empty_init, ROT0,  "Yachiyo Denki / Uni Enterprise", "Computer Quiz Atama no Taisou (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_WRONG_COLORS )
