// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Tomasz Slanina, Olivier Galibert
/*
****************************************************
Mirax (C)1985 Current Technology, Inc.

driver by
Tomasz Slanina analog[AT]op[DOT]pl
Angelo Salese
Olivier Galibert

TODO:
- sound ports are a mystery (PC=0x02e0)
- video offsets?
- score / credits display should stay above the sprites?

====================================================
Current CT8052 PCB

CPU: Ceramic potted module, Z80C
Sound: AY-3-8912 (x2)
RAM: 74S201, 74S201 (x6), 2148 (x6), 2114 (x2), 58725 (x2), 6116
PROMS: 82S123 (x2)
XTAL: 12 MHz

Here comes a high energy space game of its own kind.

Mirax(TM)
* First person perspective
* Fully integrated game play.
* Continually changing 3-D graphics plus powerful sound effects.

HOW TO PLAY
1. Your goal is to terminate Mirax City - a giant enemy central station shown at stage 10.
2. Destroy all enemy objects (air or ground) as many as you can. Avoid indestructible building blocks.
3. Shooting a group of ground targets causes satellites to rise.
4. Enemy flagship appears after a wave of far ground vessels completely destroyed.
5. Hitting flagship puts you into power shooting mode - no fire button required.
6. Bonus flag is given for each flagship hit. 5 or 8 flags award you an extra fighter.
7. Use joystick to take sight before firing.


Pinouts

Parts          Solder
1 Gnd          Gnd
2 Gnd          Gnd
3 Gnd          Video gnd
4 Gnd          Gnd
5
6
7
8
9 Coin 1       Video sync
10 Up
11 Fire
12 Down
13 start1      start2
14 left
15 right
16 Video green
17 Video blue
18 Video red
19 Speaker-    Speaker+
20 +5          +5
21 +5          +5
22 +12         +12


Stephh's notes (based on the games Z80 code and some tests) :

1) 'mirax'

  - Screen flipping settings (DSW1 bit 3) are only tested AFTER the ROM/RAM checks;
    so the first texts may be upside down related to screen orientation (ingame bug).
  - Coin A and Coin B use the same coinage based on DSW1 bits 0 and 1.
    Furthermore, they share the same coin counter at 0xf500.
  - When reaching 10th "boss" (level 100), the game will consider that you're on level 1 :
    it will display "STAGE 1 - MIRAX CITY: 90000 MILES" on "presentation" screen and
    "STAGE 1" at the bottom right when fighting the "boss".
    Once you have defeated it, you'll go back to normal stage 2 : it will display
    "STAGE 2 - MIRAX CITY: 80000 MILES" on "presentation" screen and "STAGE 2"
    at the bottom right (ingame bug).

2) 'miraxa'

  - Screen flipping settings (DSW1 bit 3) are only BEFORE the ROM/RAM checks
    due to additional code at 0x0200, so the first texts always fit screen orientation.
  - Coin A and Coin B use different coinages even if both are based on DSW1 bits 0 and 1.
    There are 2 coin counters : Coin A at 0xf500 and Coin B at 0xf502.
  - Other noticeable differences with 'mirax' :
      * different lives settings (DSW1 bits 4 and 5)
      * different bonus lives settings (DSW2 bits 0 and 1)
      * different stages names :
          . stages  1 to 10 : "LUXORI" instead of "MIRAX"
          . stages 71 to 80 : "DESCOM" instead of "DESBOM"
        furthermore, for all stages, it's written "UNIT" instead of "CITY"
  - Same ingame bug as in 'mirax' when you reach level 100 (of course, it will display
    "LUXORI UNIT" instead of "MIRAX CITY" on "presentation" screen).


Roberto Fresca notes about set 'miraxb'

  - The game starts without a self test.

  Stages:

  stages 01 to 10 : "MIRAX"
  stages 11 to 20 : "RUTHIN"
  stages 21 to 30 : "GORGAN"
  stages 31 to 40 : "PEMBAY"
  stages 41 to 50 : "URMIA"
  stages 51 to 60 : "VENLO"
  stages 61 to 70 : "OHRE"
  stages 71 to 80 : "DESBOM"
  stages 81 to 90 : "XELUN"
  stages 91 to 99 : "MURBO"

  After the stage 99, the game jumps to stage 1 (take as 100), where you reach the city.
  even displaying the wrong text. Cities appear at stage 1, 11, 21, etc...
  Once you reach again the stage 99, you'll get the stage 1 again,
  but the city will displace to the stage 2, 12, 22, etc... Always with wrong text on screen.

  Indeed it's a bug of the game.


************************************************
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class mirax_state : public driver_device
{
public:
	mirax_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ay(*this, "ay%u", 1),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram")
	{ }

	void mirax(machine_config &config);

	void init_mirax();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device_array<ay8912_device, 2> m_ay;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;

	uint8_t m_ay_addr_latch = 0;
	bool m_nmi_mask = false;
	bool m_flipscreen_x = false;
	bool m_flipscreen_y = false;

	void ay_addr_latch_w(offs_t offset, uint8_t data);
	void nmi_mask_w(int state);
	void sound_cmd_w(uint8_t data);
	template<unsigned Which> void coin_counter_w(int state);
	void flip_screen_x_w(int state);
	void flip_screen_y_w(int state);
	template<unsigned Which> void ay_data_w(uint8_t data);

	void mirax_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_tilemap(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t draw_flag);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void vblank_irq(int state);
	void mirax_main_map(address_map &map) ATTR_COLD;
	void mirax_sound_map(address_map &map) ATTR_COLD;
};


void mirax_state::mirax_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = 0x52 * bit0 + 0xad * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


void mirax_state::draw_tilemap(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t draw_flag)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);

	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			uint32_t tile = m_videoram[32 * y + x];
			uint32_t color = (m_colorram[x * 2] << 8) | (m_colorram[(x * 2) + 1]);
			int const x_scroll = (color & 0xff00) >> 8;
			tile |= ((color & 0xe0) << 3);

			int const res_x = m_flipscreen_x ? (248 - x * 8) : (x * 8);
			int const res_y = m_flipscreen_y ? (248 - y * 8 + x_scroll) : (y * 8 - x_scroll);
			int const wrapy = m_flipscreen_y ? -256 : 256;

			if ((x <= 1 || x >= 30) ^ draw_flag)
			{
				gfx->opaque(bitmap, cliprect,
					tile, color & 7,
					m_flipscreen_x, m_flipscreen_y,
					res_x, res_y);
				// wrap-around
				gfx->opaque(bitmap, cliprect,
					tile, color & 7,
					m_flipscreen_x, m_flipscreen_y,
					res_x, res_y + wrapy);
			}
		}
	}
}

void mirax_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int count = 0; count < 0x200; count += 4)
	{
		if (!m_spriteram[count] || !m_spriteram[count + 3])
			continue;

		uint32_t spr_offs = m_spriteram[count + 1] & 0x3f;
		spr_offs += (m_spriteram[count + 2] & 0xe0) << 1;
		spr_offs += (m_spriteram[count + 2] & 0x10) << 5;

		uint32_t const color = m_spriteram[count + 2] & 0x7;
		bool const fx = m_flipscreen_x ^ BIT(m_spriteram[count + 1], 6); // guess
		bool const fy = m_flipscreen_y ^ BIT(m_spriteram[count + 1], 7);

		int const y = m_flipscreen_y ? m_spriteram[count] : 0x100 - m_spriteram[count] - 16;
		int const x = m_flipscreen_x ? 240 - m_spriteram[count + 3] : m_spriteram[count + 3];

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
			spr_offs, color,
			fx, fy,
			x, y, 0);
	}
}

uint32_t mirax_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_tilemap(bitmap, cliprect, 1);
	draw_sprites(bitmap, cliprect);
	draw_tilemap(bitmap, cliprect, 0);
	return 0;
}


void mirax_state::machine_start()
{
	save_item(NAME(m_ay_addr_latch));
	save_item(NAME(m_nmi_mask));
	save_item(NAME(m_flipscreen_x));
	save_item(NAME(m_flipscreen_y));
}

void mirax_state::ay_addr_latch_w(offs_t offset, uint8_t data)
{
	m_ay_addr_latch = offset;
}

template<unsigned Which>
void mirax_state::ay_data_w(uint8_t data)
{
	m_ay[Which]->address_w(m_ay_addr_latch);
	m_ay[Which]->data_w(data);
}

void mirax_state::nmi_mask_w(int state)
{
	m_nmi_mask = state;
	if (!m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void mirax_state::sound_cmd_w(uint8_t data)
{
	m_soundlatch->write(data & 0xff);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

template<unsigned Which>
void mirax_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}

void mirax_state::flip_screen_x_w(int state)
{
	m_flipscreen_x = state;
}

void mirax_state::flip_screen_y_w(int state)
{
	m_flipscreen_y = state;
}

void mirax_state::mirax_main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc800, 0xd7ff).ram();
	map(0xe000, 0xe3ff).ram().share(m_videoram);
	map(0xe800, 0xe9ff).ram().share(m_spriteram);
	map(0xea00, 0xea3f).ram().share(m_colorram); // per-column color + bank bits for the videoram
	map(0xf000, 0xf000).portr("P1");
	map(0xf100, 0xf100).portr("P2");
	map(0xf200, 0xf200).portr("DSW1");
	map(0xf300, 0xf300).nopr(); // watchdog? value is always read then discarded
	map(0xf400, 0xf400).portr("DSW2");
	map(0xf500, 0xf507).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0xf800, 0xf800).w(FUNC(mirax_state::sound_cmd_w));
//  map(0xf900, 0xf900) // sound cmd mirror? ack?
}

void mirax_state::mirax_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x8000, 0x8fff).ram();
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));

	map(0xe000, 0xe000).nopw();
	map(0xe001, 0xe001).nopw();
	map(0xe003, 0xe003).w(FUNC(mirax_state::ay_data_w<0>)); // 1st ay ?

	map(0xe400, 0xe400).nopw();
	map(0xe401, 0xe401).nopw();
	map(0xe403, 0xe403).w(FUNC(mirax_state::ay_data_w<1>)); // 2nd ay ?

	map(0xf900, 0xf9ff).w(FUNC(mirax_state::ay_addr_latch_w));
}


// verified from Z80 code
static INPUT_PORTS_START( mirax )
	// up/down directions are trusted according of the continue screen
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )       // table at 0x11b5 (2 * 3 * 2 bytes)
	PORT_DIPSETTING(    0x00, "30k 80k 150k" )
	PORT_DIPSETTING(    0x01, "900k 950k 990k" )
	PORT_DIPNAME( 0x02, 0x00, "Flags for Extra Life" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x02, "8" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	// this dip makes the game to behave like attract mode, even if you insert a coin
	PORT_DIPNAME( 0x10, 0x00, "Auto-Play Mode (Debug)" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH )
INPUT_PORTS_END

// verified from Z80 code
static INPUT_PORTS_START( miraxa )
	PORT_INCLUDE( mirax )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, "A: 2C/1C - B: 12C/1C" )
	PORT_DIPSETTING(    0x00, "A: 1C/1C - B: 6C/1C" )
	PORT_DIPSETTING(    0x01, "A: 1C/2C - B: 3C/1C" )
	PORT_DIPSETTING(    0x02, "A: 1C/3C - B: 2C/1C" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )       // table at 0x1276 (2 * 3 * 2 bytes)
	PORT_DIPSETTING(    0x00, "30k 80k 150k" )
	PORT_DIPSETTING(    0x01, "50k 100k 900k" )
INPUT_PORTS_END


static const gfx_layout layout16 =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3),RGN_FRAC(1,3),RGN_FRAC(0,3)},
	{ STEP8(0,1), STEP8(8*8,1) },
	{ STEP8(0,8), STEP8(8*8*2,8) },
	16*16
};

static GFXDECODE_START( gfx_mirax )
	GFXDECODE_ENTRY( "tiles",   0, gfx_8x8x3_planar, 0, 8 )
	GFXDECODE_ENTRY( "sprites", 0, layout16,         0, 8 )
GFXDECODE_END


void mirax_state::vblank_irq(int state)
{
	if (state && m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void mirax_state::mirax(machine_config &config)
{
	Z80(config, m_maincpu, 12000000/4); // ceramic potted module, encrypted z80
	m_maincpu->set_addrmap(AS_PROGRAM, &mirax_state::mirax_main_map);

	Z80(config, m_audiocpu, 12000000/4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &mirax_state::mirax_sound_map);
	m_audiocpu->set_periodic_int(FUNC(mirax_state::irq0_line_hold), attotime::from_hz(4*60));

	ls259_device &mainlatch(LS259(config, "mainlatch")); // R10
	mainlatch.q_out_cb<0>().set(FUNC(mirax_state::coin_counter_w<0>));
	mainlatch.q_out_cb<1>().set(FUNC(mirax_state::nmi_mask_w));
	mainlatch.q_out_cb<2>().set(FUNC(mirax_state::coin_counter_w<1>)); // only used in 'miraxa' - see notes
	// One address flips X, the other flips Y, but I can't tell which is which
	// Since the value is the same for the 2 addresses, it doesn't really matter
	mainlatch.q_out_cb<6>().set(FUNC(mirax_state::flip_screen_x_w));
	mainlatch.q_out_cb<7>().set(FUNC(mirax_state::flip_screen_y_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(256, 256);
	screen.set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(mirax_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(mirax_state::vblank_irq));

	PALETTE(config, m_palette, FUNC(mirax_state::mirax_palette), 0x40);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mirax);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8912(config, m_ay[0], 12000000/4).add_route(ALL_OUTPUTS, "mono", 0.80);
	AY8912(config, m_ay[1], 12000000/4).add_route(ALL_OUTPUTS, "mono", 0.80);
}


ROM_START( mirax )
	ROM_REGION( 0xc000, "maincpu", ROMREGION_ERASE00 ) // put decrypted code there

	ROM_REGION( 0xc000, "data_code", 0 ) // encrypted code for the main cpu
	ROM_LOAD( "mxp5-42.rom",   0x0000, 0x4000, CRC(716410a0) SHA1(55171376e1e164b1d5e728789da6e04a3a33c172) )
	ROM_LOAD( "mxr5-4v.rom",   0x4000, 0x4000, CRC(c9484fc3) SHA1(101c5e4b9d49d2424ad80970eb3bdb87949a9966) )
	ROM_LOAD( "mxs5-4v.rom",   0x8000, 0x4000, CRC(e0085f91) SHA1(cf143b94048e1ebb5c899b94b500e193dfd42e18) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mxr2-4v.rom",   0x0000, 0x2000, CRC(cd2d52dc) SHA1(0d4181dc68beac338f47a2065c7b755008877896) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "mxe3-4v.rom",   0x0000, 0x4000, CRC(0cede01f) SHA1(c723dd8ee9dc06c94a7fe5d5b5bccc42e2181af1) )
	ROM_LOAD( "mxh3-4v.rom",   0x4000, 0x4000, CRC(58221502) SHA1(daf5c508939b44616ca76308fc33f94d364ed587) )
	ROM_LOAD( "mxk3-4v.rom",   0x8000, 0x4000, CRC(6dbc2961) SHA1(5880c28f1ef704fee2d625a42682c7d65613acc8) )

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "mxe2-4v.rom",   0x04000, 0x4000, CRC(2cf5d8b7) SHA1(f66bce4d413a48f6ae07974870dc0f31eefa68e9) )
	ROM_LOAD( "mxf2-4v.rom",   0x0c000, 0x4000, CRC(1f42c7fa) SHA1(33e56c6ddf7676a12f57de87ec740c6b6eb1cc8c) )
	ROM_LOAD( "mxh2-4v.rom",   0x14000, 0x4000, CRC(cbaff4c6) SHA1(2dc4a1f51b28e98be0cfb5ab7576047c748b6728) )
	ROM_LOAD( "mxf3-4v.rom",   0x00000, 0x4000, CRC(14b1ca85) SHA1(775a4c81a81b78490d45095af31e24c16886f0a2) )
	ROM_LOAD( "mxi3-4v.rom",   0x08000, 0x4000, CRC(20fb2099) SHA1(da6bbd5d2218ba49b8ef98e7affdcab912f84ade) )
	ROM_LOAD( "mxl3-4v.rom",   0x10000, 0x4000, CRC(918487aa) SHA1(47ba6914722a253f65c733b5edff4d15e73ea6c2) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "mra3.prm",   0x0000, 0x0020, CRC(ae7e1a63) SHA1(f5596db77c1e352ef7845465db3e54e19cd5df9e) )
	ROM_LOAD( "mrb3.prm",   0x0020, 0x0020, CRC(e3f3d0f5) SHA1(182b06c9db5bec1e3030f705247763bd2380ba83) )
	ROM_LOAD( "mirax.prm",  0x0040, 0x0020, NO_DUMP ) // data ? encrypted roms for cpu1 ?
ROM_END

ROM_START( miraxa )
	ROM_REGION( 0xc000, "maincpu", ROMREGION_ERASE00 ) // put decrypted code there

	ROM_REGION( 0xc000, "data_code", 0 ) // encrypted code for the main cpu
	ROM_LOAD( "mx_p5_43v.p5",   0x0000, 0x4000, CRC(87664903) SHA1(ccc11ecf0658e7af0db3229f60a16b1a44bd12bc) )
	ROM_LOAD( "mx_r5_43v.r5",   0x4000, 0x4000, CRC(1ba4cd8e) SHA1(8fd22d3a4bca7c989382aaf7b08ac381a3566493) )
	ROM_LOAD( "mx_s5_43v.s5",   0x8000, 0x4000, CRC(c58cc151) SHA1(0846e22f4da6d85c6dc29ff1472bc84b419b2289) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mxr2-4v.rom",   0x0000, 0x2000, CRC(cd2d52dc) SHA1(0d4181dc68beac338f47a2065c7b755008877896) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "mxe3-4v.rom",   0x0000, 0x4000, CRC(0cede01f) SHA1(c723dd8ee9dc06c94a7fe5d5b5bccc42e2181af1) )
	ROM_LOAD( "mxh3-4v.rom",   0x4000, 0x4000, CRC(58221502) SHA1(daf5c508939b44616ca76308fc33f94d364ed587) )
	ROM_LOAD( "mxk3-4v.rom",   0x8000, 0x4000, CRC(6dbc2961) SHA1(5880c28f1ef704fee2d625a42682c7d65613acc8) )

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "mxe2-4v.rom",   0x04000, 0x4000, CRC(2cf5d8b7) SHA1(f66bce4d413a48f6ae07974870dc0f31eefa68e9) )
	ROM_LOAD( "mxf2-4v.rom",   0x0c000, 0x4000, CRC(1f42c7fa) SHA1(33e56c6ddf7676a12f57de87ec740c6b6eb1cc8c) )
	ROM_LOAD( "mxh2-4v.rom",   0x14000, 0x4000, CRC(cbaff4c6) SHA1(2dc4a1f51b28e98be0cfb5ab7576047c748b6728) )
	ROM_LOAD( "mxf3-4v.rom",   0x00000, 0x4000, CRC(14b1ca85) SHA1(775a4c81a81b78490d45095af31e24c16886f0a2) )
	ROM_LOAD( "mxi3-4v.rom",   0x08000, 0x4000, CRC(20fb2099) SHA1(da6bbd5d2218ba49b8ef98e7affdcab912f84ade) )
	ROM_LOAD( "mxl3-4v.rom",   0x10000, 0x4000, CRC(918487aa) SHA1(47ba6914722a253f65c733b5edff4d15e73ea6c2) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "mra3.prm",   0x0000, 0x0020, CRC(ae7e1a63) SHA1(f5596db77c1e352ef7845465db3e54e19cd5df9e) )
	ROM_LOAD( "mrb3.prm",   0x0020, 0x0020, CRC(e3f3d0f5) SHA1(182b06c9db5bec1e3030f705247763bd2380ba83) )
ROM_END

ROM_START( miraxb )
	ROM_REGION( 0xc000, "maincpu", ROMREGION_ERASE00 ) // put decrypted code there

	ROM_REGION( 0xc000, "data_code", 0 ) // encrypted code for the main cpu
	ROM_LOAD( "10.p5",   0x0000, 0x4000, CRC(680cd519) SHA1(1cf4ef5a3e6907524b1fd874dc5412f95e4b5856) )
	ROM_LOAD( "11.r5",   0x4000, 0x4000, CRC(a518c8b0) SHA1(e974c5eaba7d8135b5c3d4606ce81f88550eb657) )
	ROM_LOAD( "12.s5",   0x8000, 0x4000, CRC(ed1f6c30) SHA1(360a46c412c93274a763f35493257f746f79bb43) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "13.r5",   0x0000, 0x2000, CRC(cd2d52dc) SHA1(0d4181dc68beac338f47a2065c7b755008877896) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "4.e3",   0x0000, 0x4000, CRC(0cede01f) SHA1(c723dd8ee9dc06c94a7fe5d5b5bccc42e2181af1) )
	ROM_LOAD( "6.h3",   0x4000, 0x4000, CRC(58221502) SHA1(daf5c508939b44616ca76308fc33f94d364ed587) )
	ROM_LOAD( "8.k3",   0x8000, 0x4000, CRC(6dbc2961) SHA1(5880c28f1ef704fee2d625a42682c7d65613acc8) )

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "1.e2",   0x04000, 0x4000, CRC(2cf5d8b7) SHA1(f66bce4d413a48f6ae07974870dc0f31eefa68e9) )
	ROM_LOAD( "2.f2",   0x0c000, 0x4000, CRC(1f42c7fa) SHA1(33e56c6ddf7676a12f57de87ec740c6b6eb1cc8c) )
	ROM_LOAD( "3.h2",   0x14000, 0x4000, CRC(cbaff4c6) SHA1(2dc4a1f51b28e98be0cfb5ab7576047c748b6728) )
	ROM_LOAD( "5.f3",   0x00000, 0x4000, CRC(14b1ca85) SHA1(775a4c81a81b78490d45095af31e24c16886f0a2) )
	ROM_LOAD( "7.i3",   0x08000, 0x4000, CRC(20fb2099) SHA1(da6bbd5d2218ba49b8ef98e7affdcab912f84ade) )
	ROM_LOAD( "9.l3",   0x10000, 0x4000, CRC(918487aa) SHA1(47ba6914722a253f65c733b5edff4d15e73ea6c2) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "mra3.prm",   0x0000, 0x0020, CRC(ae7e1a63) SHA1(f5596db77c1e352ef7845465db3e54e19cd5df9e) )
	ROM_LOAD( "mrb3.prm",   0x0020, 0x0020, CRC(e3f3d0f5) SHA1(182b06c9db5bec1e3030f705247763bd2380ba83) )
ROM_END


void mirax_state::init_mirax()
{
	uint8_t *DATA = memregion("data_code")->base();
	uint8_t *ROM = memregion("maincpu")->base();

	for (int i = 0x0000; i < 0x4000; i++)
		ROM[bitswap<16>(i, 15,14,13,12,11,10,9, 5,7,6,8, 4,3,2,1,0)] = (bitswap<8>(DATA[i], 1, 3, 7, 0, 5, 6, 4, 2) ^ 0xff);

	for (int i = 0x4000; i < 0x8000; i++)
		ROM[bitswap<16>(i, 15,14,13,12,11,10,9, 5,7,6,8, 4,3,2,1,0)] = (bitswap<8>(DATA[i], 2, 1, 0, 6, 7, 5, 3, 4) ^ 0xff);

	for (int i = 0x8000; i < 0xc000; i++)
		ROM[bitswap<16>(i, 15,14,13,12,11,10,9, 5,7,6,8, 4,3,2,1,0)] = (bitswap<8>(DATA[i], 1, 3, 7, 0, 5, 6, 4, 2) ^ 0xff);
}

} // anonymous namespace


GAME( 1985, mirax,    0,        mirax,    mirax,  mirax_state, init_mirax, ROT90, "Current Technology, Inc.", "Mirax (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, miraxa,   mirax,    mirax,    miraxa, mirax_state, init_mirax, ROT90, "Current Technology, Inc.", "Mirax (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, miraxb,   mirax,    mirax,    miraxa, mirax_state, init_mirax, ROT90, "Current Technology, Inc.", "Mirax (set 3)", MACHINE_SUPPORTS_SAVE )
