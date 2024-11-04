// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
/****************************************************************************

Wall Crash by Midcoin (c) 1984


Driver by Jarek Burczynski
2002.12.23




     DIPSW-8     AY-3-8912                               DIPSW-4
                                                         DIPSW-4

                        74s288


                                               WAC1   WAC2   WAC3
                                               (2532) (2532) (2532)
12.288MHz

   +------------+        2114  2114  2114  2114
   + EPOXY WITH +                                +-------+
   + LS08       +      WAC05  WAC1/52   EMPTY    + SMALL +
   +LS240, LS245+      (2764) (2764)    SOCKET   + EPOXY +
   + Z80        +                                +-------+
   +------------+

The bigger Epoxy brick contains three standard 74LSxxx chips and is used as
DATA lines decoder for all READS from addresses in range: 0..0x7fff.
The pinout (of the whole brick) is 1:1 Z80 and it can be replaced with
a plain Z80, given that decoded ROMS are put in place of WAC05 and WAC1/52.

The smaller Epoxy contains:
 5 chips (names sanded off...): 20 pins, 8 pins, 14 pins, 16 pins, 16 pins,
 1 resistor: 120 Ohm
 1 probably resistor: measured: 1000 Ohm
 1 diode: standard 1N4148 (info from HIGHWAYMAN)
 4 capacitors: 3 same: blue ones probably 10n , 1 smaller 1.3n (measured by HIGHWAYMAN)
It's mapped as ROM at 0x6000-0x7fff but is NOT accessed by the CPU.
It's also not needed for emulation.


Thanks to Dox for donating PCB.
Thanks to HIGHWAYMAN for providing info on how to get to these epoxies
(heat gun) and for info (very close one) on decoding.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/adc0804.h"
#include "sound/ay8910.h"
#include "video/resnet.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class wallc_state : public driver_device
{
public:
	wallc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram")
	{ }

	void sidampkr(machine_config &config);
	void sidampkra(machine_config &config);
	void unkitpkr(machine_config &config);
	void wallc(machine_config &config);
	void wallca(machine_config &config);

	void init_wallc();
	void init_wallca();
	void init_sidam();
	void init_unkitpkr();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void unkitpkr_map(address_map &map) ATTR_COLD;
	void wallc_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_videoram;

	tilemap_t *m_bg_tilemap = nullptr;

	bool m_bookkeeping_mode = false;

	void videoram_w(offs_t offset, uint8_t data);
	void wallc_coin_counter_w(uint8_t data);
	void unkitpkr_out0_w(uint8_t data);
	void unkitpkr_out1_w(uint8_t data);
	void unkitpkr_out2_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info_unkitpkr);
	TILE_GET_INFO_MEMBER(get_bg_tile_info_sidampkr);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void wallc_palette(palette_device &palette) const;
	void unkitpkr_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(unkitpkr);
	DECLARE_VIDEO_START(sidampkr);
};



/***************************************************************************

  Convert the color PROMs into a more usable format.

  Wall Crash has one 32 bytes palette PROM, connected to the RGB output this
  way:

  bit 6 -- 330 Ohm resistor --+-- 330 Ohm pulldown resistor -- RED
  bit 5 -- 220 Ohm resistor --/

  bit 4 -- NC

  bit 3 -- 330 Ohm resistor --+-- 330 Ohm pulldown resistor -- GREEN
  bit 2 -- 220 Ohm resistor --/

  bit 1 -- 330 Ohm resistor --+--+-- 330 Ohm pulldown resistor -- BLUE
  bit 0 -- 220 Ohm resistor --/  |
                                 |
  bit 7 -+- diode(~655 Ohm)------/
         \------220 Ohm pullup (+5V) resistor


***************************************************************************/

void wallc_state::wallc_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	static constexpr int resistances_rg[2] = { 330, 220 };
	static constexpr int resistances_b[3] = { 655, 330, 220 };

	double weights_r[2], weights_g[2], weights_b[3];
	compute_resistor_weights(0, 255,    -1.0,
			2,  resistances_rg, weights_r,  330,    0,
			2,  resistances_rg, weights_g,  330,    0,
			3,  resistances_b,  weights_b,  330,    655+220);

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit7;

		// red component
		bit0 = BIT(color_prom[i], 5);
		bit1 = BIT(color_prom[i], 6);
		int const r = combine_weights(weights_r, bit1, bit0);

		// green component
		bit0 = BIT(color_prom[i], 2);
		bit1 = BIT(color_prom[i], 3);
		int const g = combine_weights(weights_g, bit1, bit0);

		// blue component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit7 = BIT(color_prom[i], 7);
		int const b = combine_weights(weights_b, bit7, bit1, bit0);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void wallc_state::unkitpkr_palette(palette_device &palette) const
{
	// this pcb has 470 Ohm resistors instead of the expected 330 Ohms.
	uint8_t const *const color_prom = memregion("proms")->base();

	static constexpr int resistances_rg[2] = { 470, 220 };
	static constexpr int resistances_b[3] = { 655, 470, 220 };

	double weights_r[2], weights_g[2], weights_b[3];
	compute_resistor_weights(0, 255,    -1.0,
			2,  resistances_rg, weights_r,  470,    0,
			2,  resistances_rg, weights_g,  470,    0,
			3,  resistances_b,  weights_b,  470,    655+220);

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit7;

		// red component
		bit0 = BIT(color_prom[i], 5);
		bit1 = BIT(color_prom[i], 6);
		int const r = combine_weights(weights_r, bit1, bit0);

		// green component
		bit0 = BIT(color_prom[i], 2);
		bit1 = BIT(color_prom[i], 3);
		int const g = combine_weights(weights_g, bit1, bit0);

		// blue component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit7 = BIT(color_prom[i], 7);
		int const b = combine_weights(weights_b, bit7, bit1, bit0);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void wallc_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(wallc_state::get_bg_tile_info)
{
	tileinfo.set(0, m_videoram[tile_index] | 0x100, 1, 0);
}

TILE_GET_INFO_MEMBER(wallc_state::get_bg_tile_info_unkitpkr)
{
	int code = m_videoram[tile_index];

	// hack to display "card" graphics in middle of screen outside of bookkeeping mode
	if (m_bookkeeping_mode || (tile_index & 0x1f) < 0x08 || (tile_index & 0x1f) >= 0x10)
		code |= 0x100;

	tileinfo.set(0, code, 1, 0);
}

TILE_GET_INFO_MEMBER(wallc_state::get_bg_tile_info_sidampkr)
{
	tileinfo.set(0, m_videoram[tile_index] | 0x100, 0, 0);
}

void wallc_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wallc_state::get_bg_tile_info)), TILEMAP_SCAN_COLS_FLIP_Y, 8, 8, 32, 32);
}

VIDEO_START_MEMBER(wallc_state, unkitpkr)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wallc_state::get_bg_tile_info_unkitpkr)), TILEMAP_SCAN_COLS_FLIP_Y, 8, 8, 32, 32);
}

VIDEO_START_MEMBER(wallc_state, sidampkr)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wallc_state::get_bg_tile_info_sidampkr)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

uint32_t wallc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void wallc_state::wallc_coin_counter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 2);
}


void wallc_state::unkitpkr_out0_w(uint8_t data)
{
}

void wallc_state::unkitpkr_out1_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));
}

void wallc_state::unkitpkr_out2_w(uint8_t data)
{
	if (m_bookkeeping_mode != BIT(data, 0))
	{
		m_bookkeeping_mode = BIT(data, 0);
		m_bg_tilemap->mark_all_dirty();
	}
}

void wallc_state::wallc_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram().w(FUNC(wallc_state::videoram_w)).mirror(0xc00).share("videoram");   // 2114, 2114
	map(0xa000, 0xa3ff).ram();     // 2114, 2114

	map(0xb000, 0xb000).portr("DSW1");
	map(0xb200, 0xb200).portr("SYSTEM");
	map(0xb400, 0xb400).r("adc", FUNC(adc0804_device::read_and_write));

	map(0xb000, 0xb000).nopw();
	map(0xb100, 0xb100).w(FUNC(wallc_state::wallc_coin_counter_w));
	map(0xb200, 0xb200).nopw();
	map(0xb500, 0xb500).w("aysnd", FUNC(ay8912_device::address_w));
	map(0xb600, 0xb600).rw("aysnd", FUNC(ay8912_device::data_r), FUNC(ay8912_device::data_w));
}

void wallc_state::unkitpkr_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram().w(FUNC(wallc_state::videoram_w)).mirror(0xc00).share("videoram");   // 2114, 2114
	map(0xa000, 0xa3ff).ram();     // 2114, 2114

	map(0xb000, 0xb000).portr("DSW1");
	map(0xb100, 0xb100).portr("IN1");
	map(0xb200, 0xb200).portr("IN2");
	map(0xb300, 0xb300).portr("IN3");
	map(0xb500, 0xb5ff).nopr(); // read by memory test routine. left over from some other game

	map(0xb000, 0xb000).w(FUNC(wallc_state::unkitpkr_out0_w));
	map(0xb100, 0xb100).w(FUNC(wallc_state::unkitpkr_out1_w));
	map(0xb200, 0xb200).w(FUNC(wallc_state::unkitpkr_out2_w));
	map(0xb500, 0xb500).w("aysnd", FUNC(ay8912_device::address_w));
	map(0xb600, 0xb600).rw("aysnd", FUNC(ay8912_device::data_r), FUNC(ay8910_device::data_w));  // Port A = DSW
}


static INPUT_PORTS_START( wallc )
	PORT_START("SYSTEM")    // b200
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )    //Right curve button; select current playfield in test mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    //not used ?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )   //service?? plays loud,high-pitched sound
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )    //Left curve button; browse playfields in test mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )  //ok
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )  //ok
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )  //ok
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) //ok

	PORT_START("DIAL")      // b400 - player position 8 bit analog input - value read is used as position of the player directly - what type of input is that ? DIAL ?
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(3) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("DSW1")      // b000
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life) )        PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "100K/200K/400K/800K" )
	PORT_DIPSETTING(    0x08, "80K/160K/320K/640K" )
	PORT_DIPSETTING(    0x04, "60K/120K/240K/480K" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x00, "Curve Effect" )          PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "More" )
	PORT_DIPNAME( 0x60, 0x60, "Timer Speed" )           PORT_DIPLOCATION("SW3:2,3")
	PORT_DIPSETTING(    0x60, "Slow" )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, "Fast" )
	PORT_DIPSETTING(    0x00, "Super Fast" )
	PORT_DIPNAME( 0x80, 0x00, "Service" )               PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x80, "Free Play With Level Select" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )

	PORT_START("DSW2")      // b600
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x30, 0x00, "Coin C" )                PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_5C ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" ) // Shown as "Unused" in the manual
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" ) // Shown as "Unused" in the manual
INPUT_PORTS_END

static INPUT_PORTS_START( unkitpkr )
	PORT_START("DSW1")    // b000
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("IN1")    // b100
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) // coin out
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("IN2")    // b200
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )

	PORT_START("IN3")    // b300
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) // ok?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("DSW2")      // b600
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:1,2") // ok
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x03, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:3,4") // ok
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x00, "Coin C" )            PORT_DIPLOCATION("SW2:5,6") // ok
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x30, "1 Coin/10 Credits" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( sidampkr )
	PORT_INCLUDE(unkitpkr)

	PORT_MODIFY("DSW1")    // b000
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( French ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPNAME( 0x1c, 0x00, "Min/Max Bet" )  PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(   0x00, "Min:1; Max:2" )
	PORT_DIPSETTING(   0x04, "Min:1; Max:5" )
	PORT_DIPSETTING(   0x08, "Min:1; Max:8" )
	PORT_DIPSETTING(   0x0c, "Min:2; Max:10" )
	PORT_DIPSETTING(   0x10, "Min:5; Max:15" )
	PORT_DIPSETTING(   0x14, "Min:10; Max:20" )
	PORT_DIPSETTING(   0x18, "Min:15; Max:30" )
	PORT_DIPSETTING(   0x1c, "Min:20; Max:40" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( sidampkra )
	PORT_INCLUDE(unkitpkr)

	PORT_MODIFY("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPNAME( 0x60, 0x60, "Min/Max Bet" )  PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x00, "Min:1; Max:5" )
	PORT_DIPSETTING(    0x20, "Min:1; Max:10" )
	PORT_DIPSETTING(    0x40, "Min:1; Max:20" )
	PORT_DIPSETTING(    0x60, "Min:1; Max:40" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x80, DEF_STR( French ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	RGN_FRAC(1,3),
	3,  // 3 bits per pixel
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) }, // the bitplanes are separated
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every char takes 8 consecutive bytes
};

static GFXDECODE_START( gfx_wallc )
	GFXDECODE_ENTRY( "gfx1", 0     , charlayout, 0, 4 )
GFXDECODE_END


void wallc_state::init_wallc()
{
	uint8_t *ROM = memregion("maincpu")->base();

	for (uint32_t i = 0; i < 0x2000 * 2; i++)
	{
		uint8_t c = ROM[ i ] ^ 0x55 ^ 0xff; // NOTE: this can be shortened but now it fully reflects what the bigger module really does
		c = bitswap<8>(c, 4,2,6,0,7,1,3,5); // also swapped inside of the bigger module
		ROM[ i ] = c;
	}
}

void wallc_state::init_wallca()
{
	uint8_t *ROM = memregion("maincpu")->base();

	for (uint32_t i = 0; i < 0x4000; i++)
	{
		uint8_t c;
		if (i & 0x100)
		{
			c = ROM[i] ^ 0x4a;
			c = bitswap<8>(c, 4,7,1,3,2,0,5,6);
		}
		else
		{
			c = ROM[i] ^ 0xa5;
			c = bitswap<8>(c, 0,2,3,6,1,5,7,4);
		}

		ROM[ i ] = c;
	}
}


void wallc_state::wallc(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 12.288_MHz_XTAL / 4);  // 3.072 MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &wallc_state::wallc_map);
	m_maincpu->set_vblank_int("screen", FUNC(wallc_state::irq0_line_hold));

	ADC0804(config, "adc", 640000).vin_callback().set_ioport("DIAL"); // clock not verified

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(wallc_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_wallc);
	PALETTE(config, "palette", FUNC(wallc_state::wallc_palette), 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	ay8912_device &aysnd(AY8912(config, "aysnd", 12288000 / 8));
	aysnd.port_a_read_callback().set_ioport("DSW2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.30);
}

void wallc_state::wallca(machine_config &config)
{
	wallc(config);
	m_maincpu->set_clock(12_MHz_XTAL / 4);
}

void wallc_state::sidampkra(machine_config &config)
{
	wallc(config);
	config.device_remove("adc");

	m_maincpu->set_addrmap(AS_PROGRAM, &wallc_state::unkitpkr_map);

	subdevice<palette_device>("palette")->set_init(FUNC(wallc_state::unkitpkr_palette));

	// sound hardware
	subdevice<ay8912_device>("aysnd")->reset_routes().add_route(ALL_OUTPUTS, "mono", 0.50);
}

void wallc_state::unkitpkr(machine_config &config)
{
	sidampkra(config);

	MCFG_VIDEO_START_OVERRIDE(wallc_state, unkitpkr)
}

void wallc_state::sidampkr(machine_config &config)
{
	sidampkra(config);

	MCFG_VIDEO_START_OVERRIDE(wallc_state, sidampkr)
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( wallc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wac05.h7",   0x0000, 0x2000, CRC(ab6e472e) SHA1(a387fec24fb899df349a35d1d3a91e897b074712) )
	ROM_LOAD( "wac1-52.h6", 0x2000, 0x2000, CRC(988eaa6d) SHA1(d5e5dbee6e7e0488fdecfb864198c686cbd5d59c) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "wc1.e3",     0x0000, 0x1000, CRC(ca5c4b53) SHA1(5d2e14fe81cca4ec7dbe0c98eaa26890fca28e58) )
	ROM_LOAD( "wc2.e2",     0x1000, 0x1000, CRC(b7f52a59) SHA1(737e7616d7295762057fbdb69d65c8c1edc773dc) )
	ROM_LOAD( "wc3.e1",     0x2000, 0x1000, CRC(f6854b3a) SHA1(bc1e7f785c338c1afa4ab61c07c61397b3de0b01) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.c2",  0x0000, 0x0020, CRC(83e3e293) SHA1(a98c5e63b688de8d175adb6539e0cdc668f313fd) )
ROM_END

// this set uses a different encryption, but the decrypted code is the same
ROM_START( wallca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom4.rom",     0x0000, 0x2000, CRC(ce43af1b) SHA1(c05419cb4aa57c6187b469573a3787d9123c4a05) )
	ROM_LOAD( "rom5.rom",     0x2000, 0x2000, CRC(b789a705) SHA1(2b62b14d1a3ad5eff5b8d502d7891e58379ee820) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "rom3.rom",     0x0800, 0x0800, CRC(6634db73) SHA1(fe6104f974495a250e0cd14c0745eec8e44b8d3a) )
	ROM_LOAD( "rom2.rom",     0x1800, 0x0800, CRC(79f49c2c) SHA1(485fdba5ebdb4c01306f3ef26c992a513aa6b5dc) )
	ROM_LOAD( "rom1.rom",     0x2800, 0x0800, CRC(3884fd4f) SHA1(47254c8828128ac48fc15f05b52fe4d42d4919e7) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.c2",  0x0000, 0x0020, CRC(83e3e293) SHA1(a98c5e63b688de8d175adb6539e0cdc668f313fd) )
ROM_END

ROM_START( brkblast )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fadesa-r0.6m", 0x0000, 0x4000, CRC(4e96ca15) SHA1(87f1a3538712aa3d6c3713b845679dd42a4ba5a4) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "rom3.rom",     0x0800, 0x0800, CRC(6634db73) SHA1(fe6104f974495a250e0cd14c0745eec8e44b8d3a) )
	ROM_LOAD( "rom2.rom",     0x1800, 0x0800, CRC(79f49c2c) SHA1(485fdba5ebdb4c01306f3ef26c992a513aa6b5dc) )
	ROM_LOAD( "rom1.rom",     0x2800, 0x0800, CRC(3884fd4f) SHA1(47254c8828128ac48fc15f05b52fe4d42d4919e7) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.c2",    0x0000, 0x0020, CRC(83e3e293) SHA1(a98c5e63b688de8d175adb6539e0cdc668f313fd) )

	ROM_REGION( 0x00eb, "plds", 0 )
	ROM_LOAD( "82s153.h8",    0x0000, 0x00eb, CRC(a6db7c28) SHA1(023f393c45ae35c0008f61af6c3a1b21f7fe7c79) ) // On the CPU subboard
ROM_END


/*

It uses an epoxy brick like wallc
Inside the brick there are:
- 74245
- 74368
- Pal16r4

74368 is a tristate not, it's used to:
-negate D0 that goes to the CPU if A15 is low
-negate D1 that goes to the CPU if A15 is low
-negate D2 that goes to the CPU if A15 is low
-negate D3 that goes to the CPU if A15 is low

-negate cpu clk to feed the pal clk ALWAYS
-negate A15 to feed 74245 /EN ALWAYS


The 74245 let pass the data unmodified if A15 is high (like wallc)

If A15 is low a Pal16r4 kick in
this chip can modify D2,D3,D4,D5,D6,D7

D0 and D1 are negated from outside to real Z80
D2 and D3 are negated after begin modified by the Pal

Pal input
A1
A3
A6
A15 (Output enable, not in equation)

D2
D3
D4
D5  (2 times)
D7  (2 times)

Pal output
D2 (via not to cpu)
D3 (via not to cpu)
D4 to cpu
D5 to cpu
D6 to cpu
D7 to cpu

*/

ROM_START( sidampkr ) // 11600 PCB + small riser PCB with Z80 and PAL (epoxy cover removed but etched 11610 on PCB)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11600.0.h6",     0x0000, 0x1000, CRC(88cac4d2) SHA1(a369da3dc80671eeff549077cf2ce860d5f4ea35) )
	ROM_LOAD( "11600.1.h5",     0x1000, 0x1000, CRC(96cca320) SHA1(85326f7126c8250a22f35f6eed138051a9ab35cb) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "11605.b.e1",     0x0800, 0x0800, CRC(a7800f8a) SHA1(3955e0f71ced6fd759f52d12c0b39ab6aab31ca4) )
	ROM_LOAD( "11605.g.e2",     0x1800, 0x0800, CRC(b7bebf1e) SHA1(764536989ba4c4c143a61d4453c3bba547bc630a) )
	ROM_LOAD( "11605.r.e3",     0x2800, 0x0800, CRC(4d645b8d) SHA1(d4f8d11c4ef796cf66ebf2e6b8a11247d630951a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "11607-74.288.c3",  0x0000, 0x0020, CRC(e14bf545) SHA1(5e8c5a9ea6e4842f27a47c1d7224ed294bbaa40b) )

	ROM_REGION( 0x0104, "plds", 0 ) // probably encryption related
	ROM_LOAD( "pal16r4anc.sub",  0x0000, 0x0104, NO_DUMP )
ROM_END

ROM_START( sidampkra ) // 11700B PCB + small riser PCB with Z80 and PAL (with 11610 epoxy cover)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11704.0.h6",     0x0000, 0x2000, CRC(62a7ff4f) SHA1(66ed12e5ec9da5e97fabf37e971342c0cfbce372) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "11705.b.e1",     0x0800, 0x0800, CRC(50a9a468) SHA1(182a753748296df859f938655e9fa27889d5cd3f) )
	ROM_LOAD( "11705.g.e2",     0x1800, 0x0800, CRC(d3366b36) SHA1(e1eb148b07a302f2ba66ad196959f019a81145f6) )
	ROM_LOAD( "11705.r.e3",     0x2800, 0x0800, CRC(d51df283) SHA1(46bd52e156be99db731e6d56ab8241d1c396ab99) )

	ROM_REGION( 0x0040, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "11707.c3",         0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "11708.c2",         0x0020, 0x0020, NO_DUMP )
	ROM_LOAD( "74s288.c2",        0x0000, 0x0020, CRC(83e3e293) SHA1(a98c5e63b688de8d175adb6539e0cdc668f313fd) ) //  using the one from unkitpkr for now


	ROM_REGION( 0x0104, "plds", 0 ) // probably encryption related
	ROM_LOAD( "pal16r4anc.sub",  0x0000, 0x0104, NO_DUMP )
ROM_END

void wallc_state::init_sidam()
{
	uint8_t *ROM = memregion("maincpu")->base();

	for (int i = 0; i < 0x2000; i++)
	{
		uint8_t x = ROM[i];
		switch(i & 0x4a) // seems correct. Plaintext available in the 0x1150-0x1550 range. First 0x50 of code are very similar if not identical to unkitpkr.
		{
			case 0x00: x = bitswap<8>(x ^ (BIT(x, 6) ? 0xaf : 0x03), 7, 3, 5, 2, 6, 4, 1, 0); break;
			case 0x02: x = bitswap<8>(x ^ 0x77, 4, 6, 2, 5, 3, 7, 1, 0); break;
			case 0x08: x = bitswap<8>(x ^ 0x5f, 2, 4, 6, 3, 7, 5, 1, 0); break;
			case 0x0a: x = bitswap<8>(x ^ 0xd7, 6, 2, 4, 7, 5, 3, 1, 0); break;
			case 0x40: x = bitswap<8>(x ^ (BIT(x, 6) ? 0xaf : 0x03), 7, 3, 5, 2, 6, 4, 1, 0); break;
			case 0x42: x = bitswap<8>(x ^ 0xeb, 5, 7, 3, 6, 4, 2, 1, 0); break;
			case 0x48: x = bitswap<8>(x ^ (BIT(x, 6) ? 0xbb : 0x03), 3, 5, 7, 4, 2, 6, 1, 0); break;
			case 0x4a: x = bitswap<8>(x ^ 0xd7, 6, 2, 4, 7, 5, 3, 1, 0); break;
		}

		ROM[i] = x;
	}
}

/*
  Unknown Italian Poker
  Seems a brute hack of an unknown game.

  The "conforme alla legge n." string is overwriting the hands table:

  "CONFORME"   = Royal Flush
  (blank line) = Straight Flush
  "ALLA LEGGE" = Four of a Kind
  (blank line) = Full House
  "N.904 DEL"  = Flush
  (blank line) = Straight
  "17.12.1986" = Three of a Kind
  (blank line) = Double Pair
  "........."  = Simple Pair

  Also the code is hacked/patched to avoid some jumps:

  00cb: ld a,(hl)
  00cc: cp c
  00cd: nop
  00ce: nop
  00cf: nop

  00d2: ld a,(hl)
  00d3: cp c
  00d4: nop
  00d5: nop
  00d6: nop

  1866: pop af
  1867: cp $46
  1869: nop
  186a: nop
  186b: nop

  Main PCB has a Microchip AY-3-8912A PSG, a 3.6V battery and no ROM numbered 4.
  A daughterboard contains the Z80 and some RAM.
*/
ROM_START( unkitpkr )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "1", 0x0000, 0x2000, CRC(82dacf83) SHA1(d2bd4664737aeb968e9e34da74c2654e556c8567) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "2", 0x0000, 0x1000, CRC(a359b7aa) SHA1(832a0dfd0689f76381f34d2d8419a7f09a6c403a) )
	ROM_CONTINUE(  0x0000, 0x1000 ) // first half is empty
	ROM_LOAD( "3", 0x1000, 0x1000, CRC(f7d7d48b) SHA1(d9787dcbbfdb5f8f8434d8e688c1ee1e0566969d) )
	ROM_CONTINUE(  0x1000, 0x1000 ) // first half is empty
	ROM_LOAD( "5", 0x2000, 0x1000, CRC(b3084b49) SHA1(21b2fa41492faf95e66c5765acfdae1685ee8784) )
	ROM_CONTINUE(  0x2000, 0x1000 ) // first half is empty

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.c2",  0x0000, 0x0020, CRC(83e3e293) SHA1(a98c5e63b688de8d175adb6539e0cdc668f313fd) ) // dumped; matches the wallc bp
ROM_END

void wallc_state::init_unkitpkr()
{
	// line swapping is too annoying to handle with ROM_LOAD macros
	uint8_t buffer[0x400];
	for (int b = 0; b < 0x3000; b += 0x400)
	{
		uint8_t *gfxrom = memregion("gfx1")->base() + b;
		for (int a = 0; a < 0x400; a++)
			buffer[a] = gfxrom[(a & 0x03f) | (a & 0x280) >> 1 | (a & 0x140) << 1];
		memcpy(gfxrom, &buffer[0], 0x400);
	}

	m_bookkeeping_mode = false;
	save_item(NAME(m_bookkeeping_mode));
}

} // anonymous namespace


//    YEAR  NAME       PARENT    MACHINE    INPUT      STATE        INIT           ROT     COMPANY             FULLNAME                               FLAGS
GAME( 1984, wallc,     0,        wallc,     wallc,     wallc_state, init_wallc,    ROT0,   "Midcoin",          "Wall Crash (set 1)",                  MACHINE_SUPPORTS_SAVE )
GAME( 1984, wallca,    wallc,    wallca,    wallc,     wallc_state, init_wallca,   ROT0,   "Midcoin",          "Wall Crash (set 2)",                  MACHINE_SUPPORTS_SAVE )
GAME( 1984, brkblast,  wallc,    wallc,     wallc,     wallc_state, init_wallca,   ROT0,   "bootleg (Fadesa)", "Brick Blast (bootleg of Wall Crash)", MACHINE_SUPPORTS_SAVE ) // Spanish bootleg board, Fadesa stickers / text on various components

GAME( 1984, sidampkr,  0,        sidampkr,  sidampkr,  wallc_state, init_sidam,    ROT270, "Sidam",            "unknown Sidam poker (vertical)",      MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE ) // colors should be verified
GAME( 1984, sidampkra, sidampkr, sidampkra, sidampkra, wallc_state, init_sidam,    ROT0,   "Sidam",            "unknown Sidam poker (horizontal)",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE ) // colors should be verified, cards GFX look wrong
GAME( 198?, unkitpkr,  0,        unkitpkr,  unkitpkr,  wallc_state, init_unkitpkr, ROT0,   "<unknown>",        "unknown Italian poker game",          MACHINE_SUPPORTS_SAVE )
