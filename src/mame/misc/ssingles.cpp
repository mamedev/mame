// license:BSD-3-Clause
// copyright-holders: Tomasz Slanina

/*
 'Swinging Singles' US distribution by Ent. Ent. Ltd
 Original Japan release is 'Utamaro' by 'Yachiyo' (undumped!)
 driver by Tomasz Slanina


 Crap XXX game.
 Three ROMs contain text "BY YACHIYO"

 Upper half of 7.bin = upper half of 8.bin = intentional or bad dump ?

 TODO:
 - atamanot: needs a trojan, in order to understand how the protection really works.
 - colors (missing PROM(s) ?)
 - samples (at least two of unused ROMs contains samples (unkn. format , ADPCM ?)
 - dips (one is tested in game (difficulty related?), another 2 are tested at start)

 Unknown reads/writes:
 - AY i/o ports (writes)
 - mem $c000, $c001 = protection device ? if tests fails, game crashes (problems with stack - skipped code with "pop af")
 - i/o port $8 = data read used for  $e command arg for one of AY chips (volume? - could be a sample player (based on volume changes?)
 - i/o port $1a = 1 or 0, rarely accessed, related to crt  writes

==================================================================

Computer Quiz Atama no Taisou
(c)1983 Yachiyo Denki / Uni Enterprize

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


*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


// configurable logging
#define LOG_ATAMANOTPROT     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_ATAMANOTPROT)

#include "logmacro.h"

#define LOGATAMANOTPROT(...)     LOGMASKED(LOG_ATAMANOTPROT,     __VA_ARGS__)


namespace {

static constexpr uint8_t NUM_PENS = 4 * 8;

class ssingles_state : public driver_device
{
public:
	ssingles_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_videoram(*this, "videoram")
		, m_colorram(*this, "colorram")
		, m_gfx_rom(*this, "gfx")
		, m_extra(*this, "EXTRA")
	{ }

	void ssingles(machine_config &config);
	void atamanot(machine_config &config);

	ioport_value controls_r();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_region_ptr<uint8_t> m_gfx_rom;

	required_ioport m_extra;

	pen_t m_pens[NUM_PENS];

	// ssingles
	uint8_t m_prot_data = 0;

	// atamanot
	uint8_t m_atamanot_prot_state = 0;

	// ssingles
	uint8_t c000_r();
	uint8_t c001_r();
	void c001_w(uint8_t data);

	// atamanot
	uint8_t atamanot_prot_r(offs_t offset);
	void atamanot_prot_w(uint8_t data);

	void atamanot_irq(int state);
	MC6845_UPDATE_ROW(ssingles_update_row);
	MC6845_UPDATE_ROW(atamanot_update_row);

	void atamanot_io_map(address_map &map) ATTR_COLD;
	void atamanot_map(address_map &map) ATTR_COLD;
	void ssingles_io_map(address_map &map) ATTR_COLD;
	void ssingles_map(address_map &map) ATTR_COLD;
};

//fake palette
static constexpr rgb_t ssingles_colors[NUM_PENS] =
{
	{ 0x00,0x00,0x00 }, { 0xff,0xff,0xff }, { 0xff,0x00,0x00 }, { 0x80,0x00,0x00 },
	{ 0x00,0x00,0x00 }, { 0xf0,0xf0,0xf0 }, { 0xff,0xff,0x00 }, { 0x40,0x40,0x40 },
	{ 0x00,0x00,0x00 }, { 0xff,0xff,0xff }, { 0xff,0x00,0x00 }, { 0xff,0xff,0x00 },
	{ 0x00,0x00,0x00 }, { 0xff,0xff,0x00 }, { 0xd0,0x00,0x00 }, { 0x80,0x00,0x00 },
	{ 0x00,0x00,0x00 }, { 0xff,0x00,0x00 }, { 0xff,0xff,0x00 }, { 0x80,0x80,0x00 },
	{ 0x00,0x00,0x00 }, { 0xff,0x00,0x00 }, { 0x40,0x40,0x40 }, { 0xd0,0xd0,0xd0 },
	{ 0x00,0x00,0x00 }, { 0x00,0x00,0xff }, { 0x60,0x40,0x30 }, { 0xff,0xff,0x00 },
	{ 0x00,0x00,0x00 }, { 0xff,0x00,0xff }, { 0x80,0x00,0x80 }, { 0x40,0x00,0x40 }
};

MC6845_UPDATE_ROW(ssingles_state::ssingles_update_row)
{
	for (int cx = 0; cx < x_count; ++cx)
	{
		int const address = ((ma >> 1) + (cx >> 1)) & 0xff;

		uint16_t const cell = m_videoram[address] + (m_colorram[address] << 8);

		uint32_t const tile_address = ((cell & 0x3ff) << 4) + ra;
		uint16_t const palette = (cell >> 10) & 0x1c;

		uint8_t b0, b1;
		if (cx & 1)
		{
			b0 = m_gfx_rom[tile_address + 0x0000]; //  9.bin
			b1 = m_gfx_rom[tile_address + 0x8000]; // 11.bin
		}
		else
		{
			b0 = m_gfx_rom[tile_address + 0x4000]; // 10.bin
			b1 = m_gfx_rom[tile_address + 0xc000]; // 12.bin
		}

		for (int x = 7; x >= 0; --x)
		{
			bitmap.pix(y, (cx << 3) | x) = m_pens[palette + ((b1 & 1) | ((b0 & 1) << 1))];
			b0 >>= 1;
			b1 >>= 1;
		}
	}
}

MC6845_UPDATE_ROW(ssingles_state::atamanot_update_row)
{
	for (int cx = 0; cx < x_count; ++cx)
	{
		int const address = ((ma >> 1) + (cx >> 1)) & 0xff;

		uint16_t const cell = m_videoram[address] + (m_colorram[address] << 8);

		uint32_t const tile_address = ((cell & 0x1ff) << 4) + ra;
		uint16_t const palette = (cell >> 10) & 0x1c;

		uint8_t b0, b1;
		if (cx & 1)
		{
			b0 = m_gfx_rom[tile_address + 0x0000]; //  9.bin
			b1 = m_gfx_rom[tile_address + 0x4000]; // 11.bin
		}
		else
		{
			b0 = m_gfx_rom[tile_address + 0x2000]; // 10.bin
			b1 = m_gfx_rom[tile_address + 0x6000]; // 12.bin
		}

		for (int x = 7; x >= 0; --x)
		{
			bitmap.pix(y, (cx << 3) | x) = m_pens[palette + ((b1 & 1) | ((b0 & 1) << 1))];
			b0 >>= 1;
			b1 >>= 1;
		}
	}
}


void ssingles_state::video_start()
{
	for (int i = 0; i < NUM_PENS; ++i)
		m_pens[i] = ssingles_colors[i];
}


uint8_t ssingles_state::c000_r()
{
	return m_prot_data;
}

uint8_t ssingles_state::c001_r()
{
	m_prot_data = 0xc4;
	return 0;
}

void ssingles_state::c001_w(uint8_t data)
{
	m_prot_data ^= data ^ 0x11;
}

ioport_value ssingles_state::controls_r()
{
	int data = 7;
	switch (m_extra->read())     //multiplexed
	{
		case 0x01: data = 1; break;
		case 0x02: data = 2; break;
		case 0x04: data = 3; break;
		case 0x08: data = 4; break;
		case 0x10: data = 5; break;
		case 0x20: data = 6; break;
		case 0x40: data = 0; break;
	}

	return data;
}

void ssingles_state::ssingles_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().share(m_videoram);
	map(0x0800, 0x08ff).ram().share(m_colorram);
	map(0x0000, 0x1fff).rom();
	map(0xc000, 0xc000).r(FUNC(ssingles_state::c000_r));
	map(0xc001, 0xc001).rw(FUNC(ssingles_state::c001_r), FUNC(ssingles_state::c001_w));
	map(0x6000, 0xbfff).rom();
	map(0xf800, 0xffff).ram();
}


uint8_t ssingles_state::atamanot_prot_r(offs_t offset)
{
	static const char prot_id[] = { "PROGRAM BY KOYAMA" };

	LOGATAMANOTPROT("%04x %02x\n", offset, m_atamanot_prot_state);

	switch (m_atamanot_prot_state)
	{
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
			return prot_id[offset % 0x11];

		case 0xc0:
			return 2; // 1 goes to service mode?
	}

	return 0;
}

void ssingles_state::atamanot_prot_w(uint8_t data)
{
	m_atamanot_prot_state = data;
}


void ssingles_state::atamanot_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().share(m_videoram);
	map(0x0800, 0x08ff).ram().share(m_colorram);
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x6000, 0x60ff).ram(); // kanji tilemap?
//  map(0x6000, 0x7fff).rom();
	map(0x8000, 0x83ff).r(FUNC(ssingles_state::atamanot_prot_r));
//  map(0x8000, 0x9fff).rom().region("question", 0x10000);
//  map(0xc000, 0xc000).r(FUNC(ssingles_state::c000_r));
//  map(0xc001, 0xc001).rw(FUNC(ssingles_state::c001_r), FUNC(ssingles_state::c001_w));
}

void ssingles_state::ssingles_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("ay1", FUNC(ay8910_device::address_w));
	map(0x04, 0x04).w("ay1", FUNC(ay8910_device::data_w));
	map(0x06, 0x06).w("ay2", FUNC(ay8910_device::address_w));
	map(0x08, 0x08).nopr();
	map(0x0a, 0x0a).w("ay2", FUNC(ay8910_device::data_w));
	map(0x16, 0x16).portr("DSW0");
	map(0x18, 0x18).portr("DSW1");
	map(0x1c, 0x1c).portr("INPUTS");
//  map(0x1a, 0x1a).nopw(); // video/crt related
	map(0xfe, 0xfe).w("crtc", FUNC(mc6845_device::address_w));
	map(0xff, 0xff).w("crtc", FUNC(mc6845_device::register_w));
}

void ssingles_state::atamanot_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("ay1", FUNC(ay8910_device::address_w));
	map(0x04, 0x04).w("ay1", FUNC(ay8910_device::data_w));
	map(0x06, 0x06).w("ay2", FUNC(ay8910_device::address_w));
	map(0x08, 0x08).nopr();
	map(0x0a, 0x0a).w("ay2", FUNC(ay8910_device::data_w));
	map(0x16, 0x16).portr("DSW0");
	map(0x18, 0x18).portr("DSW1").w(FUNC(ssingles_state::atamanot_prot_w));
	map(0x1c, 0x1c).portr("INPUTS");
//  map(0x1a, 0x1a).nopw(); // video/crt related
	map(0xfe, 0xfe).w("crtc", FUNC(mc6845_device::address_w));
	map(0xff, 0xff).w("crtc", FUNC(mc6845_device::register_w));
}

static INPUT_PORTS_START( ssingles )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // must be LOW
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(ssingles_state, controls_r)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 )

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x04, 0x04, "Unk1" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Unk2" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Unk3" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Unk4" ) // tested in game, every frame, could be difficulty related
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Unk5" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Unk 6" )
	PORT_DIPSETTING(    0x01, "Pos 1" )
	PORT_DIPSETTING(    0x03, "Pos 2" )
	PORT_DIPSETTING(    0x00, "Pos 3" )
	PORT_DIPSETTING(    0x02, "Pos 4" )
	PORT_DIPNAME( 0x04, 0x04, "Unk7" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Unk8" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Unk9" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "UnkA" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "UnkB" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "UnkC" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

/*
atamanot kanji gfx decoding:

It looks "stolen" from an unknown Japanese computer?
*/

static const gfx_layout layout_8x8 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};


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

static GFXDECODE_START( gfx_ssingles )
	GFXDECODE_ENTRY( "gfx", 0, layout_8x8, 0, 8 )
GFXDECODE_END

static GFXDECODE_START( gfx_atamanot )
	GFXDECODE_ENTRY( "gfx", 0, layout_8x8, 0, 8 )
	GFXDECODE_ENTRY( "kanji", 0, layout_16x16,     0, 8 )
	GFXDECODE_ENTRY( "kanji_uc", 0, layout_8x16,     0, 8 )
	GFXDECODE_ENTRY( "kanji_lc", 0, layout_8x16,     0, 8 )
GFXDECODE_END

void ssingles_state::ssingles(machine_config &config)
{
	Z80(config, m_maincpu, 4'000'000);         // ? MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &ssingles_state::ssingles_map);
	m_maincpu->set_addrmap(AS_IO, &ssingles_state::ssingles_io_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(4'000'000, 256, 0, 256, 256, 0, 256);   // temporary, CRTC will configure screen
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, "palette").set_entries(4); // guess

	GFXDECODE(config, "gfxdecode", "palette", gfx_ssingles);

	mc6845_device &crtc(MC6845(config, "crtc", 1'000'000)); // ? MHz
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(ssingles_state::ssingles_update_row));
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay1", 1'500'000).add_route(ALL_OUTPUTS, "mono", 0.5); // ? MHz

	AY8910(config, "ay2", 1'500'000).add_route(ALL_OUTPUTS, "mono", 0.5); // ? MHz
}

void ssingles_state::atamanot_irq(int state)
{
	// ...
}

void ssingles_state::atamanot(machine_config &config)
{
	ssingles(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &ssingles_state::atamanot_map);
	m_maincpu->set_addrmap(AS_IO, &ssingles_state::atamanot_io_map);

	mc6845_device &crtc(*subdevice<mc6845_device>("crtc"));
	crtc.set_update_row_callback(FUNC(ssingles_state::atamanot_update_row));
	crtc.out_vsync_callback().set(FUNC(ssingles_state::atamanot_irq));

	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_atamanot);
}

ROM_START( ssingles )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80
	ROM_LOAD( "1.bin", 0x00000, 0x2000, CRC(43f02215) SHA1(9f04a7d4671ff39fd2bd8ec7afced4981ee7be05) )
	ROM_LOAD( "2.bin", 0x06000, 0x2000, CRC(281f27e4) SHA1(cef28717ab2ed991a5709464c01490f0ab1dc17c) )
	ROM_LOAD( "3.bin", 0x08000, 0x2000, CRC(14fdcb65) SHA1(70f7fcb46e74937de0e4037c9fe79349a30d0d07) )
	ROM_LOAD( "4.bin", 0x0a000, 0x2000, CRC(acb44685) SHA1(d68aab8b7e68d842a350d3fb76985ac857b1d972) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "9.bin",  0x0000, 0x4000, CRC(57fac6f9) SHA1(12f6695c9831399e599a95008ebf9db943725437) )
	ROM_LOAD( "10.bin", 0x4000, 0x4000, CRC(cd3ba260) SHA1(2499ad9982cc6356e2eb3a0f10d77886872a0c9f) )
	ROM_LOAD( "11.bin", 0x8000, 0x4000, CRC(f7107b29) SHA1(a405926fd3cb4b3d2a1c705dcde25d961dba5884) )
	ROM_LOAD( "12.bin", 0xc000, 0x4000, CRC(e5585a93) SHA1(04d55699b56d869066f2be2c6ac48042aa6c3108) )

	ROM_REGION( 0x08000, "user1", 0) // samples ? data ?
	ROM_LOAD( "5.bin", 0x00000, 0x2000, CRC(242a8dda) SHA1(e140893cc05fb8cee75904d98b02626f2565ed1b) )
	ROM_LOAD( "6.bin", 0x02000, 0x2000, CRC(85ab8aab) SHA1(566f034e1ba23382442f27457447133a0e0f1cfc) )
	ROM_LOAD( "7.bin", 0x04000, 0x2000, CRC(57cc112d) SHA1(fc861c58ae39503497f04d302a9f16fca19b37fb) )
	ROM_LOAD( "8.bin", 0x06000, 0x2000, CRC(52de717a) SHA1(e60399355165fb46fac862fb7fcdff16ff351631) )

ROM_END


ROM_START( atamanot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tt1.2",   0x0000, 0x2000, CRC(da9e270d) SHA1(b7408be913dad8abf022c6153f2493204dd74952) )
	ROM_LOAD( "tt2.3",   0x2000, 0x2000, CRC(7595ade8) SHA1(71f9d6d987407f88cdd3b28bd1e35e00ac17e1f5) )

	ROM_REGION( 0x18000, "question", 0 ) // question ROMs?
	ROM_LOAD( "ta.bin",  0x00000, 0x2000, CRC(5c61edaf) SHA1(ea56df6b320aa7e52828aaccbb5838cd0c756f24) )
	ROM_LOAD( "tb.bin",  0x02000, 0x2000, CRC(07bd2e6f) SHA1(bf245d8208db447572e484057b9daa6276f03683) )
	ROM_LOAD( "tc.bin",  0x04000, 0x2000, CRC(1e09ac09) SHA1(91ec1b2c5767b5bad8915f7c9984f423fcb399c9) )
	ROM_LOAD( "td.bin",  0x06000, 0x2000, CRC(bd514d51) SHA1(1a1e95558b2608f0103ca1b42fe9e59ccb90487f) )
	ROM_LOAD( "te.bin",  0x08000, 0x2000, CRC(825ed49f) SHA1(775044f6d53ecbfa0ab604947a21e368bad85ce0) )
	ROM_LOAD( "tf.bin",  0x0a000, 0x2000, CRC(d92b5eb9) SHA1(311fdefdc1f1026cb7f7cc1e1adaffbdbe7a70d9) )
	ROM_LOAD( "tg.bin",  0x0c000, 0x2000, CRC(eb25aa72) SHA1(de3a3d87a2eb540b96947f776c321dc9a7c21e78) )
	ROM_LOAD( "th.bin",  0x0e000, 0x2000, CRC(13396cfb) SHA1(d98ea4ff2e2175aa7003e37001664b3fa898c071) )
	ROM_LOAD( "ti.bin",  0x10000, 0x2000, CRC(60193df3) SHA1(58840bde303a760a0458224983af0c0bbe939a2f) )
	ROM_LOAD( "j.bin",   0x12000, 0x2000, CRC(cd16ddbf) SHA1(b418b5d73d3699697ebd42a6f4df598dcdcaf264) )
	ROM_LOAD( "k.bin",   0x14000, 0x2000, CRC(c75c7a1e) SHA1(59b136626267fa3ba5a2e1709acb632142e1560e) )
	ROM_LOAD( "l.bin",   0x16000, 0x2000, CRC(dbb4ed60) SHA1(b5054ba3ccd268594d22e1e67f70bb227095ca4c) )

	ROM_REGION( 0x8000, "gfx", 0 )
	ROM_LOAD( "ca.49",   0x0000, 0x2000, CRC(28d20b52) SHA1(a104ef1cd103f31803b88bd2d4804eab5a26e7fa) )
	ROM_LOAD( "cc.48",   0x2000, 0x2000, CRC(209cab0d) SHA1(9a89af1f7186e4845e43f9cdafd273e69d280bfb) )
	ROM_LOAD( "cb.47",   0x4000, 0x2000, CRC(8bc85c0c) SHA1(64701bc910c28666d15ee22f59f32888cc2302ae) )
	ROM_LOAD( "cd.46",   0x6000, 0x2000, CRC(22e8d103) SHA1(f0146f7e192eef8d03404a9c5b8a9f9c9577d936) )

	ROM_REGION( 0x20000, "kanji", 0 )
	ROM_LOAD( "ic36.bin",   0x18000, 0x8000, CRC(643e3077) SHA1(fa81a3a3eebd59c6dc9c9b7eeb4a480bb1440c17) )
	ROM_LOAD( "ic35.bin",   0x10000, 0x8000, CRC(fe0302a0) SHA1(f8d3a58da4e8dd09db240039f5216e7ebe9cc384) )
	ROM_LOAD( "ic34.bin",   0x08000, 0x8000, CRC(06e7c7da) SHA1(a222c0b0eccfda613f916320e6afbb33385921ba) )
	ROM_LOAD( "ic33.bin",   0x00000, 0x8000, CRC(323a70e7) SHA1(55e570f039c97d15b06bfcb1ebf03562cbcf8324) )

	ROM_REGION( 0x10000, "kanji_uc", 0 ) // upper case
	ROM_COPY( "kanji", 0x10000, 0x08000, 0x08000 )
	ROM_COPY( "kanji", 0x00000, 0x00000, 0x08000 )

	ROM_REGION( 0x10000, "kanji_lc", 0 ) // lower case
	ROM_COPY( "kanji", 0x18000, 0x08000, 0x08000 )
	ROM_COPY( "kanji", 0x08000, 0x00000, 0x08000 )

	ROM_REGION( 0x0300,  "proms", 0 ) // NOT color proms
	ROM_LOAD( "1.52",   0x00000, 0x0100, CRC(13f5762b) SHA1(da9cc51eda0681b0d3c17b212d23ab89af2813ff) )
	ROM_LOAD( "2.53",   0x00100, 0x0100, CRC(4142f525) SHA1(2e2e896ba7b49df9cf3fddff6becc07a3d50d926) )
	ROM_LOAD( "3.54",   0x00200, 0x0100, CRC(88acb21e) SHA1(18fe5280dad6687daf6bf42d37dde45157fab5e3) )
ROM_END

} // anonymous namespace


GAME( 1983, ssingles, 0, ssingles, ssingles, ssingles_state, empty_init, ROT90, "Yachiyo Denki (Entertainment Enterprises, Ltd. license)", "Swinging Singles (US)",                 MACHINE_SUPPORTS_SAVE | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_SOUND )
GAME( 1983, atamanot, 0, atamanot, ssingles, ssingles_state, empty_init, ROT90, "Yachiyo Denki / Uni Enterprize",                          "Computer Quiz Atama no Taisou (Japan)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
