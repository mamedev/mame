// license:BSD-3-Clause
// copyright-holders: Nicola Salmoria, Chris Hardy, Couriersud

/***************************************************************************

        Circus Charlie   GX380 (c) 1984 Konami

         Upper board PWB (B)3000148  - OSC 14.31818MHz
         Lower board PWB (A)3000161  - OSC 18432.00KHz


Based on drivers from Juno First emulator by Chris Hardy (chrish@kcbbs.gen.nz)

To enter service mode, keep 1&2 pressed on reset


'circusc3' ingame bug :
-----------------------

  "Test mode" displays 2, 3, 4 and 7 lives instead of 3, 4, 5 and 7
  due to code at 0xcb38 :

    CB38: 9E 18 00         LDA   $1800
    CB3B: CB               COMA
    CB3C: AC 03            ANDA  #$03
    CB3E: 09 03            CMPA  #$03
    CB40: 07 02            BCS   $CB44
    CB42: 04 05            LDA   #$05
    CB44: A9 02            ADDA  #$02

  In other sets, you have the following (code from 'circusc') :

    CB38: 9E 18 00         LDA   $1800
    CB3B: CB               COMA
    CB3C: AC 03            ANDA  #$03
    CB3E: 09 03            CMPA  #$03
    CB40: 07 02            BCS   $CB44
    CB42: 04 04            LDA   #$04
    CB44: A9 03            ADDA  #$03

  Ingame lives are correct though (same code for 'circusc' and 'circusc3') :

    6B93: 14 2F            LDA   $2F
    6B95: A6 03            ANDA  #$03
    6B97: 09 03            ADDA  #$03
    6B99: A9 06            CMPA  #$06
    6B9B: AD 02            BCS   $6B9F
    6B9D: AE 07            LDA   #$07

This bug is due to 380_r02.6h, it differs from 380_q02.6h by 2 bytes, at
 offset 0x0b43 is 0x05 and 0x0b45 is 0x02 which is the code listed above.

***************************************************************************/

#include "emu.h"

#include "konami1.h"

#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/dac.h"
#include "sound/discrete.h"
#include "sound/sn76496.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class circusc_state : public driver_device
{
public:
	circusc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_scroll(*this, "scroll"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram%u", 1U),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_sn(*this, "sn%u", 1U),
		m_dac(*this, "dac"),
		m_discrete(*this, "fltdisc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void circusc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr_array<uint8_t, 2> m_spriteram;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	bool m_spritebank = false;

	// sound-related
	uint8_t m_sn_latch = 0U;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device_array<sn76496_device, 2> m_sn;
	required_device<dac_byte_interface> m_dac;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	bool m_irq_mask = false;

	uint8_t sh_timer_r();
	void sh_irqtrigger_w(uint8_t data);
	template <uint8_t Which> void coin_counter_w(int state);
	void sound_w(offs_t offset, uint8_t data);
	void irq_mask_w(int state);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void spritebank_w(int state);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Circus Charlie has one 32x8 palette PROM and two 256x4 lookup table PROMs
  (one for characters, one for sprites).
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

void circusc_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 0, 0,
			3, &resistances_rg[0], gweights, 0, 0,
			2, &resistances_b[0],  bweights, 0, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 32;

	// characters map to the upper 16 palette entries
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry + 0x10);
	}

	// sprites map to the lower 16 palette entries
	for (int i = 0x100; i < 0x200; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(circusc_state::get_tile_info)
{
	uint8_t const attr = m_colorram[tile_index];
	tileinfo.category = BIT(attr, 4);

	tileinfo.set(
			0,
			m_videoram[tile_index] + ((attr & 0x20) << 3),
			attr & 0x0f,
			TILE_FLIPYX((attr & 0xc0) >> 6));
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void circusc_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(circusc_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_scroll_cols(32);

	save_item(NAME(m_spritebank));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void circusc_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void circusc_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void circusc_state::spritebank_w(int state)
{
	m_spritebank = state;
}



/***************************************************************************

  Display refresh

***************************************************************************/

void circusc_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *sr = m_spritebank ? m_spriteram[0] : m_spriteram[1];

	for (int offs = 0; offs < m_spriteram[0].bytes(); offs += 4)
	{
		int const code = sr[offs + 0] + 8 * (sr[offs + 1] & 0x20);
		int const color = sr[offs + 1] & 0x0f;
		int sx = sr[offs + 2];
		int sy = sr[offs + 3];
		int flipx = sr[offs + 1] & 0x40;
		int flipy = sr[offs + 1] & 0x80;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}


		m_gfxdecode->gfx(1)->transmask(bitmap, cliprect,
				code, color,
				flipx, flipy,
				sx, sy,
				m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0));
	}
}

uint32_t circusc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0; i < 10; i++)
		m_bg_tilemap->set_scrolly(i, 0);
	for (int i = 10; i < 32; i++)
		m_bg_tilemap->set_scrolly(i, *m_scroll);

	bitmap.fill(0, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void circusc_state::machine_start()
{
	save_item(NAME(m_sn_latch));
	save_item(NAME(m_irq_mask));
}

void circusc_state::machine_reset()
{
	m_sn_latch = 0;
}

uint8_t circusc_state::sh_timer_r()
{
	/* This port reads the output of a counter clocked from the CPU clock.
	 * The CPU XTAL is 14.31818MHz divided by 4.  It then goes through 10
	 * /2 stages to clock a 4 bit counter.  The output of the counter goes
	 * to D1-D4.
	 *
	 * The following:
	 * clock = m_audiocpu->total_cycles() >> 10;
	 * return (clock & 0x0f) << 1;
	 * Can be shortened to:
	 */

	int const clock = m_audiocpu->total_cycles() >> 9;

	return clock & 0x1e;
}

void circusc_state::sh_irqtrigger_w(uint8_t data)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

template <uint8_t Which>
void circusc_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}

void circusc_state::sound_w(offs_t offset, uint8_t data)
{
	switch (offset & 7)
	{
		// CS2
		case 0:
			m_sn_latch = data;
			break;

		// CS3
		case 1:
			m_sn[0]->write(m_sn_latch);
			break;

		// CS4
		case 2:
			m_sn[1]->write(m_sn_latch);
			break;

		// CS5
		case 3:
			m_dac->write(data);
			break;

		// CS6
		case 4:
			m_discrete->write(NODE_05, (offset & 0x20) >> 5);
			m_discrete->write(NODE_06, (offset & 0x18) >> 3);
			m_discrete->write(NODE_07, (offset & 0x40) >> 6);
			break;
	}
}

void circusc_state::irq_mask_w(int state)
{
	m_irq_mask = state;
	if (!m_irq_mask)
		m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}

void circusc_state::main_map(address_map &map)
{
	map(0x0000, 0x0007).mirror(0x03f8).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x0400, 0x0400).mirror(0x03ff).w("watchdog", FUNC(watchdog_timer_device::reset_w)); // WDOG
	map(0x0800, 0x0800).mirror(0x03ff).w("soundlatch", FUNC(generic_latch_8_device::write)); // SOUND DATA
	map(0x0c00, 0x0c00).mirror(0x03ff).w(FUNC(circusc_state::sh_irqtrigger_w));    // SOUND-ON causes interrupt on audio CPU
	map(0x1000, 0x1000).mirror(0x03fc).portr("SYSTEM");
	map(0x1001, 0x1001).mirror(0x03fc).portr("P1");
	map(0x1002, 0x1002).mirror(0x03fc).portr("P2");
	map(0x1003, 0x1003).mirror(0x03fc).nopr();              // unpopulated DIPSW
	map(0x1400, 0x1400).mirror(0x03ff).portr("DSW1");
	map(0x1800, 0x1800).mirror(0x03ff).portr("DSW2");
	map(0x1c00, 0x1c00).mirror(0x03ff).writeonly().share(m_scroll); // VGAP
	map(0x2000, 0x2fff).ram();
	map(0x3000, 0x33ff).ram().w(FUNC(circusc_state::colorram_w)).share(m_colorram);
	map(0x3400, 0x37ff).ram().w(FUNC(circusc_state::videoram_w)).share(m_videoram);
	map(0x3800, 0x38ff).ram().share(m_spriteram[1]);
	map(0x3900, 0x39ff).ram().share(m_spriteram[0]);
	map(0x3a00, 0x3fff).ram();
	map(0x6000, 0xffff).rom();
}

void circusc_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).mirror(0x1c00).ram();
	map(0x6000, 0x6000).mirror(0x1fff).r("soundlatch", FUNC(generic_latch_8_device::read)); // CS0
	map(0x8000, 0x8000).mirror(0x1fff).r(FUNC(circusc_state::sh_timer_r));                  // CS1
	map(0xa000, 0xa07f).mirror(0x1f80).w(FUNC(circusc_state::sound_w));                     // CS2 - CS6
}



static INPUT_PORTS_START( circusc )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // SW7 of 8 on unpopulated DIPSW 3
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) // 1P UP - unused
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) // 1P DOWN - unused
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) // 1P SHOOT2 - unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) // 2P UP - unused
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) // 2P DOWN - unused
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) // 2P SHOOT2 - unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "20k 90k 70k+" )
	PORT_DIPSETTING(    0x00, "30k 110k 80k+" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW2:5" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static GFXDECODE_START( gfx_circusc )
	GFXDECODE_ENTRY( "tiles",   0, gfx_8x8x4_packed_msb,       0, 16 )
	GFXDECODE_ENTRY( "sprites", 0, gfx_16x16x4_packed_msb, 16*16, 16 )
GFXDECODE_END


static const discrete_mixer_desc circusc_mixer_desc =
	{DISC_MIXER_IS_RESISTOR,
		{RES_K(2.2), RES_K(2.2), RES_K(10)},
		{0,0,0},    // no variable resistors
		{0,0,0},  // no node capacitors
		0, RES_K(1),
		CAP_U(0.1),
		CAP_U(0.47),
		0, 1};

static DISCRETE_SOUND_START( circusc_discrete )

	DISCRETE_INPUTX_STREAM(NODE_01, 0, 1.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_02, 1, 1.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_03, 2, 2.0, 0) // DAC 0..32767, multiply by 2

	DISCRETE_INPUT_DATA(NODE_05)
	DISCRETE_INPUT_DATA(NODE_06)
	DISCRETE_INPUT_DATA(NODE_07)

	DISCRETE_RCFILTER_SW(NODE_10, 1, NODE_01, NODE_05, 1000, CAP_U(0.47), 0, 0, 0)
	DISCRETE_RCFILTER_SW(NODE_11, 1, NODE_02, NODE_06, 1000, CAP_U(0.047), CAP_U(0.47), 0, 0)
	DISCRETE_RCFILTER_SW(NODE_12, 1, NODE_03, NODE_07, 1000, CAP_U(0.47), 0, 0, 0)

	DISCRETE_MIXER3(NODE_20, 1, NODE_10, NODE_11, NODE_12, &circusc_mixer_desc)

	DISCRETE_OUTPUT(NODE_20, 10.0 )

DISCRETE_SOUND_END

void circusc_state::vblank_irq(int state)
{
	if (state && m_irq_mask)
		m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

void circusc_state::circusc(machine_config &config)
{
	// basic machine hardware
	KONAMI1(config, m_maincpu, 2'048'000);        // 2 MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &circusc_state::main_map);

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 2C
	mainlatch.q_out_cb<0>().set(FUNC(circusc_state::flip_screen_set)); // FLIP
	mainlatch.q_out_cb<1>().set(FUNC(circusc_state::irq_mask_w)); // INTST
	mainlatch.q_out_cb<2>().set_nop(); // MUT - not used
	mainlatch.q_out_cb<3>().set(FUNC(circusc_state::coin_counter_w<0>)); // COIN1
	mainlatch.q_out_cb<4>().set(FUNC(circusc_state::coin_counter_w<1>)); // COIN2
	mainlatch.q_out_cb<5>().set(FUNC(circusc_state::spritebank_w)); // OBJ CHENG

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 8);

	Z80(config, m_audiocpu, XTAL(14'318'181) / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &circusc_state::sound_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(circusc_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(circusc_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_circusc);
	PALETTE(config, m_palette, FUNC(circusc_state::palette), 16*16 + 16*16, 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	SN76496(config, m_sn[0], XTAL(14'318'181) / 8).add_route(0, "fltdisc", 1.0, 0);

	SN76496(config, m_sn[1], XTAL(14'318'181) / 8).add_route(0, "fltdisc", 1.0, 1);

	DAC_8BIT_R2R(config, "dac", 0).set_output_range(0, 1).add_route(0, "fltdisc", 1.0, 2); // ls374.7g + r44+r45+r47+r48+r50+r56+r57+r58+r59 (20k) + r46+r49+r51+r52+r53+r54+r55 (10k) + upc324.3h

	DISCRETE(config, m_discrete, circusc_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( circusc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "380_s05.3h",   0x6000, 0x2000, CRC(48feafcf) SHA1(0e5bd350fa5fee42569eb0c4accf7512d645b792) )
	ROM_LOAD( "380_q04.4h",   0x8000, 0x2000, CRC(c283b887) SHA1(458c398911453d558003f49c298b0d593c941c11) ) // Could also be labeled 380 R04
	ROM_LOAD( "380_q03.5h",   0xa000, 0x2000, CRC(e90c0e86) SHA1(03211f0cc90b6e356989c5e2a41b70f4ff2ead83) ) // Could also be labeled 380 R03
	ROM_LOAD( "380_q02.6h",   0xc000, 0x2000, CRC(4d847dc6) SHA1(a1f65e73c4e5abff1b0970bad32a128173245561) )
	ROM_LOAD( "380_q01.7h",   0xe000, 0x2000, CRC(18c20adf) SHA1(2f40e1a109d129bb127a8b98e27817988cd08c8b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "380_l14.5c",   0x0000, 0x2000, CRC(607df0fb) SHA1(67103d61994fd3a1e2de7cf9487e4f763234b18e) )
	ROM_LOAD( "380_l15.7c",   0x2000, 0x2000, CRC(a6ad30e1) SHA1(14f305717edcc2471e763b262960a0b96eef3530) )

	ROM_REGION( 0x04000, "tiles", 0 )
	ROM_LOAD( "380_j12.4a",   0x0000, 0x2000, CRC(56e5b408) SHA1(73b9e3d46dfe9e39b390c634df153648a0906876) )
	ROM_LOAD( "380_j13.5a",   0x2000, 0x2000, CRC(5aca0193) SHA1(4d0b0a773c385b7f1dcf024760d0437f47e78fbe) )

	ROM_REGION( 0x0c000, "sprites", 0 )
	ROM_LOAD( "380_j06.11e",  0x0000, 0x2000, CRC(df0405c6) SHA1(70a50dcc86dfbdaa9c2af613105aae7f90747804) )
	ROM_LOAD( "380_j07.12e",  0x2000, 0x2000, CRC(23dfe3a6) SHA1(2ad7cbcbdbb434dc43e9c94cd00df9e57ac097f5) )
	ROM_LOAD( "380_j08.13e",  0x4000, 0x2000, CRC(3ba95390) SHA1(b22ad7cfda392894208eb4b39505f38bfe4c4342) )
	ROM_LOAD( "380_j09.14e",  0x6000, 0x2000, CRC(a9fba85a) SHA1(1a649ec667d377ffab26b4694be790b3a2742f30) )
	ROM_LOAD( "380_j10.15e",  0x8000, 0x2000, CRC(0532347e) SHA1(4c02b75a62993cce60d2cb87b81c7738abbc9a0d) )
	ROM_LOAD( "380_j11.16e",  0xa000, 0x2000, CRC(e1725d24) SHA1(d315588e6cc2f4263be621d2d8603c8215a90046) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "380_j18.2a",   0x0000, 0x020, CRC(10dd4eaa) SHA1(599acd25f36445221c553510a5de23ddba5ecc15) ) // palette
	ROM_LOAD( "380_j17.7b",   0x0020, 0x100, CRC(13989357) SHA1(0d61d468f6d3e1570fd18d236ec8cab92db4ed5c) ) // character lookup table
	ROM_LOAD( "380_j16.10c",  0x0120, 0x100, CRC(c244f2aa) SHA1(86df21c8e0b1ed51a0a4bd33dbb33f6efdea7d39) ) // sprite lookup table
ROM_END

ROM_START( circusc2 ) // This set verified to come with Q revision ROMs for 01 through 04
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "380_w05.3h",   0x6000, 0x2000, CRC(87df9f5e) SHA1(d759ff6200546c562aecee749dc9941bbbdb9918) )
	ROM_LOAD( "380_q04.4h",   0x8000, 0x2000, CRC(c283b887) SHA1(458c398911453d558003f49c298b0d593c941c11) ) // Could also be labeled 380 R04
	ROM_LOAD( "380_q03.5h",   0xa000, 0x2000, CRC(e90c0e86) SHA1(03211f0cc90b6e356989c5e2a41b70f4ff2ead83) ) // Could also be labeled 380 R03
	ROM_LOAD( "380_q02.6h",   0xc000, 0x2000, CRC(4d847dc6) SHA1(a1f65e73c4e5abff1b0970bad32a128173245561) )
	ROM_LOAD( "380_q01.7h",   0xe000, 0x2000, CRC(18c20adf) SHA1(2f40e1a109d129bb127a8b98e27817988cd08c8b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "380_l14.5c",   0x0000, 0x2000, CRC(607df0fb) SHA1(67103d61994fd3a1e2de7cf9487e4f763234b18e) )
	ROM_LOAD( "380_l15.7c",   0x2000, 0x2000, CRC(a6ad30e1) SHA1(14f305717edcc2471e763b262960a0b96eef3530) )

	ROM_REGION( 0x04000, "tiles", 0 )
	ROM_LOAD( "380_j12.4a",   0x0000, 0x2000, CRC(56e5b408) SHA1(73b9e3d46dfe9e39b390c634df153648a0906876) )
	ROM_LOAD( "380_k13.5a",   0x2000, 0x2000, CRC(5aca0193) SHA1(4d0b0a773c385b7f1dcf024760d0437f47e78fbe) ) // == 380_j13.5a

	ROM_REGION( 0x0c000, "sprites", 0 )
	ROM_LOAD( "380_j06.11e",  0x0000, 0x2000, CRC(df0405c6) SHA1(70a50dcc86dfbdaa9c2af613105aae7f90747804) )
	ROM_LOAD( "380_j07.12e",  0x2000, 0x2000, CRC(23dfe3a6) SHA1(2ad7cbcbdbb434dc43e9c94cd00df9e57ac097f5) )
	ROM_LOAD( "380_j08.13e",  0x4000, 0x2000, CRC(3ba95390) SHA1(b22ad7cfda392894208eb4b39505f38bfe4c4342) )
	ROM_LOAD( "380_j09.14e",  0x6000, 0x2000, CRC(a9fba85a) SHA1(1a649ec667d377ffab26b4694be790b3a2742f30) )
	ROM_LOAD( "380_j10.15e",  0x8000, 0x2000, CRC(0532347e) SHA1(4c02b75a62993cce60d2cb87b81c7738abbc9a0d) )
	ROM_LOAD( "380_j11.16e",  0xa000, 0x2000, CRC(e1725d24) SHA1(d315588e6cc2f4263be621d2d8603c8215a90046) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "380_j18.2a",   0x0000, 0x020, CRC(10dd4eaa) SHA1(599acd25f36445221c553510a5de23ddba5ecc15) ) // palette
	ROM_LOAD( "380_j17.7b",   0x0020, 0x100, CRC(13989357) SHA1(0d61d468f6d3e1570fd18d236ec8cab92db4ed5c) ) // character lookup table
	ROM_LOAD( "380_j16.10c",  0x0120, 0x100, CRC(c244f2aa) SHA1(86df21c8e0b1ed51a0a4bd33dbb33f6efdea7d39) ) // sprite lookup table
ROM_END

ROM_START( circusc3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "380_w05.3h",   0x6000, 0x2000, CRC(87df9f5e) SHA1(d759ff6200546c562aecee749dc9941bbbdb9918) )
	ROM_LOAD( "380_r04.4h",   0x8000, 0x2000, CRC(c283b887) SHA1(458c398911453d558003f49c298b0d593c941c11) ) // == 380_q04.4h
	ROM_LOAD( "380_r03.5h",   0xa000, 0x2000, CRC(e90c0e86) SHA1(03211f0cc90b6e356989c5e2a41b70f4ff2ead83) ) // == 380_q03.5h
	ROM_LOAD( "380_r02.6h",   0xc000, 0x2000, CRC(2d434c6f) SHA1(2c794f24422db7671d1bc85cef308ab4a62d523d) ) // Cause of incorrect Lives bug in service mode
	ROM_LOAD( "380_q01.7h",   0xe000, 0x2000, CRC(18c20adf) SHA1(2f40e1a109d129bb127a8b98e27817988cd08c8b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "380_l14.5c",   0x0000, 0x2000, CRC(607df0fb) SHA1(67103d61994fd3a1e2de7cf9487e4f763234b18e) )
	ROM_LOAD( "380_l15.7c",   0x2000, 0x2000, CRC(a6ad30e1) SHA1(14f305717edcc2471e763b262960a0b96eef3530) )

	ROM_REGION( 0x04000, "tiles", 0 )
	ROM_LOAD( "380_j12.4a",   0x0000, 0x2000, CRC(56e5b408) SHA1(73b9e3d46dfe9e39b390c634df153648a0906876) )
	ROM_LOAD( "380_j13.5a",   0x2000, 0x2000, CRC(5aca0193) SHA1(4d0b0a773c385b7f1dcf024760d0437f47e78fbe) )

	ROM_REGION( 0x0c000, "sprites", 0 )
	ROM_LOAD( "380_j06.11e",  0x0000, 0x2000, CRC(df0405c6) SHA1(70a50dcc86dfbdaa9c2af613105aae7f90747804) )
	ROM_LOAD( "380_j07.12e",  0x2000, 0x2000, CRC(23dfe3a6) SHA1(2ad7cbcbdbb434dc43e9c94cd00df9e57ac097f5) )
	ROM_LOAD( "380_j08.13e",  0x4000, 0x2000, CRC(3ba95390) SHA1(b22ad7cfda392894208eb4b39505f38bfe4c4342) )
	ROM_LOAD( "380_j09.14e",  0x6000, 0x2000, CRC(a9fba85a) SHA1(1a649ec667d377ffab26b4694be790b3a2742f30) )
	ROM_LOAD( "380_j10.15e",  0x8000, 0x2000, CRC(0532347e) SHA1(4c02b75a62993cce60d2cb87b81c7738abbc9a0d) )
	ROM_LOAD( "380_j11.16e",  0xa000, 0x2000, CRC(e1725d24) SHA1(d315588e6cc2f4263be621d2d8603c8215a90046) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "380_j18.2a",   0x0000, 0x020, CRC(10dd4eaa) SHA1(599acd25f36445221c553510a5de23ddba5ecc15) ) // palette
	ROM_LOAD( "380_j17.7b",   0x0020, 0x100, CRC(13989357) SHA1(0d61d468f6d3e1570fd18d236ec8cab92db4ed5c) ) // character lookup table
	ROM_LOAD( "380_j16.10c",  0x0120, 0x100, CRC(c244f2aa) SHA1(86df21c8e0b1ed51a0a4bd33dbb33f6efdea7d39) ) // sprite lookup table
ROM_END

ROM_START( circusc4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "380_r05.3h",   0x6000, 0x2000, CRC(ed52c60f) SHA1(aa9dc6a57e29895be521ac6a146de56a7beef957) )
	ROM_LOAD( "380_n04.4h",   0x8000, 0x2000, CRC(fcc99e33) SHA1(da140a849ac22419e8890414b8984aa264f7e3c7) )
	ROM_LOAD( "380_n03.5h",   0xa000, 0x2000, CRC(5ef5b3b5) SHA1(b058600c915a0d6653eaa5fc87ecee44a38eed00) )
	ROM_LOAD( "380_n02.6h",   0xc000, 0x2000, CRC(a5a5e796) SHA1(a41700b272ff4198447ed75138d65ec3a759d221) )
	ROM_LOAD( "380_n01.7h",   0xe000, 0x2000, CRC(70d26721) SHA1(eb71cb0da26991a3628150f45f1389c2f2ef90fc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "380_l14.5c",   0x0000, 0x2000, CRC(607df0fb) SHA1(67103d61994fd3a1e2de7cf9487e4f763234b18e) )
	ROM_LOAD( "380_l15.7c",   0x2000, 0x2000, CRC(a6ad30e1) SHA1(14f305717edcc2471e763b262960a0b96eef3530) )

	ROM_REGION( 0x04000, "tiles", 0 )
	ROM_LOAD( "380_j12.4a",   0x0000, 0x2000, CRC(56e5b408) SHA1(73b9e3d46dfe9e39b390c634df153648a0906876) )
	ROM_LOAD( "380_j13.5a",   0x2000, 0x2000, CRC(5aca0193) SHA1(4d0b0a773c385b7f1dcf024760d0437f47e78fbe) )

	ROM_REGION( 0x0c000, "sprites", 0 )
	ROM_LOAD( "380_j06.11e",  0x0000, 0x2000, CRC(df0405c6) SHA1(70a50dcc86dfbdaa9c2af613105aae7f90747804) )
	ROM_LOAD( "380_j07.12e",  0x2000, 0x2000, CRC(23dfe3a6) SHA1(2ad7cbcbdbb434dc43e9c94cd00df9e57ac097f5) )
	ROM_LOAD( "380_j08.13e",  0x4000, 0x2000, CRC(3ba95390) SHA1(b22ad7cfda392894208eb4b39505f38bfe4c4342) )
	ROM_LOAD( "380_j09.14e",  0x6000, 0x2000, CRC(a9fba85a) SHA1(1a649ec667d377ffab26b4694be790b3a2742f30) )
	ROM_LOAD( "380_j10.15e",  0x8000, 0x2000, CRC(0532347e) SHA1(4c02b75a62993cce60d2cb87b81c7738abbc9a0d) )
	ROM_LOAD( "380_j11.16e",  0xa000, 0x2000, CRC(e1725d24) SHA1(d315588e6cc2f4263be621d2d8603c8215a90046) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "380_j18.2a",   0x0000, 0x020, CRC(10dd4eaa) SHA1(599acd25f36445221c553510a5de23ddba5ecc15) ) // palette
	ROM_LOAD( "380_j17.7b",   0x0020, 0x100, CRC(13989357) SHA1(0d61d468f6d3e1570fd18d236ec8cab92db4ed5c) ) // character lookup table
	ROM_LOAD( "380_j16.10c",  0x0120, 0x100, CRC(c244f2aa) SHA1(86df21c8e0b1ed51a0a4bd33dbb33f6efdea7d39) ) // sprite lookup table
ROM_END

ROM_START( circuscc ) // Version U
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "380_u05.3h",   0x6000, 0x2000, CRC(964c035a) SHA1(bd69bb755be327d04fc95cd33115663b33b33ed3) )
	ROM_LOAD( "380_p04.4h",   0x8000, 0x2000, CRC(dd0c0ee7) SHA1(e56e48f6f251430b7ce0e2cc59cfd00b5c760b9c) )
	ROM_LOAD( "380_p03.5h",   0xa000, 0x2000, CRC(190247af) SHA1(f2128fb5e6c16791493af1c77628b610b86d4677) )
	ROM_LOAD( "380_p02.6h",   0xc000, 0x2000, CRC(7e63725e) SHA1(f731f15956c6e7a0a4e8225513f8b9e6017c7a17) )
	ROM_LOAD( "380_p01.7h",   0xe000, 0x2000, CRC(eedaa5b2) SHA1(0c606ca4d092c3dc290c30b1a73f94e3b348e8fd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "380_l14.5c",   0x0000, 0x2000, CRC(607df0fb) SHA1(67103d61994fd3a1e2de7cf9487e4f763234b18e) )
	ROM_LOAD( "380_l15.7c",   0x2000, 0x2000, CRC(a6ad30e1) SHA1(14f305717edcc2471e763b262960a0b96eef3530) )

	ROM_REGION( 0x04000, "tiles", 0 )
	ROM_LOAD( "380_j12.4a",   0x0000, 0x2000, CRC(56e5b408) SHA1(73b9e3d46dfe9e39b390c634df153648a0906876) )
	ROM_LOAD( "380_j13.5a",   0x2000, 0x2000, CRC(5aca0193) SHA1(4d0b0a773c385b7f1dcf024760d0437f47e78fbe) )

	ROM_REGION( 0x0c000, "sprites", 0 )
	ROM_LOAD( "380_j06.11e",  0x0000, 0x2000, CRC(df0405c6) SHA1(70a50dcc86dfbdaa9c2af613105aae7f90747804) )
	ROM_LOAD( "380_j07.12e",  0x2000, 0x2000, CRC(23dfe3a6) SHA1(2ad7cbcbdbb434dc43e9c94cd00df9e57ac097f5) )
	ROM_LOAD( "380_j08.13e",  0x4000, 0x2000, CRC(3ba95390) SHA1(b22ad7cfda392894208eb4b39505f38bfe4c4342) )
	ROM_LOAD( "380_j09.14e",  0x6000, 0x2000, CRC(a9fba85a) SHA1(1a649ec667d377ffab26b4694be790b3a2742f30) )
	ROM_LOAD( "380_j10.15e",  0x8000, 0x2000, CRC(0532347e) SHA1(4c02b75a62993cce60d2cb87b81c7738abbc9a0d) )
	ROM_LOAD( "380_j11.16e",  0xa000, 0x2000, CRC(e1725d24) SHA1(d315588e6cc2f4263be621d2d8603c8215a90046) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "380_j18.2a",   0x0000, 0x020, CRC(10dd4eaa) SHA1(599acd25f36445221c553510a5de23ddba5ecc15) ) // palette
	ROM_LOAD( "380_j17.7b",   0x0020, 0x100, CRC(13989357) SHA1(0d61d468f6d3e1570fd18d236ec8cab92db4ed5c) ) // character lookup table
	ROM_LOAD( "380_j16.10c",  0x0120, 0x100, CRC(c244f2aa) SHA1(86df21c8e0b1ed51a0a4bd33dbb33f6efdea7d39) ) // sprite lookup table
ROM_END

ROM_START( circusce ) // Version P
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "380_p05.3h",   0x6000, 0x2000, CRC(7ca74494) SHA1(326e081490e413b0638ec77de184b128fb2afd14) )
	ROM_LOAD( "380_p04.4h",   0x8000, 0x2000, CRC(dd0c0ee7) SHA1(e56e48f6f251430b7ce0e2cc59cfd00b5c760b9c) )
	ROM_LOAD( "380_p03.5h",   0xa000, 0x2000, CRC(190247af) SHA1(f2128fb5e6c16791493af1c77628b610b86d4677) )
	ROM_LOAD( "380_p02.6h",   0xc000, 0x2000, CRC(7e63725e) SHA1(f731f15956c6e7a0a4e8225513f8b9e6017c7a17) )
	ROM_LOAD( "380_p01.7h",   0xe000, 0x2000, CRC(eedaa5b2) SHA1(0c606ca4d092c3dc290c30b1a73f94e3b348e8fd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "380_l14.5c",   0x0000, 0x2000, CRC(607df0fb) SHA1(67103d61994fd3a1e2de7cf9487e4f763234b18e) )
	ROM_LOAD( "380_l15.7c",   0x2000, 0x2000, CRC(a6ad30e1) SHA1(14f305717edcc2471e763b262960a0b96eef3530) )

	ROM_REGION( 0x04000, "tiles", 0 )
	ROM_LOAD( "380_j12.4a",   0x0000, 0x2000, CRC(56e5b408) SHA1(73b9e3d46dfe9e39b390c634df153648a0906876) )
	ROM_LOAD( "380_j13.5a",   0x2000, 0x2000, CRC(5aca0193) SHA1(4d0b0a773c385b7f1dcf024760d0437f47e78fbe) )

	ROM_REGION( 0x0c000, "sprites", 0 )
	ROM_LOAD( "380_j06.11e",  0x0000, 0x2000, CRC(df0405c6) SHA1(70a50dcc86dfbdaa9c2af613105aae7f90747804) )
	ROM_LOAD( "380_j07.12e",  0x2000, 0x2000, CRC(23dfe3a6) SHA1(2ad7cbcbdbb434dc43e9c94cd00df9e57ac097f5) )
	ROM_LOAD( "380_j08.13e",  0x4000, 0x2000, CRC(3ba95390) SHA1(b22ad7cfda392894208eb4b39505f38bfe4c4342) )
	ROM_LOAD( "380_j09.14e",  0x6000, 0x2000, CRC(a9fba85a) SHA1(1a649ec667d377ffab26b4694be790b3a2742f30) )
	ROM_LOAD( "380_j10.15e",  0x8000, 0x2000, CRC(0532347e) SHA1(4c02b75a62993cce60d2cb87b81c7738abbc9a0d) )
	ROM_LOAD( "380_j11.16e",  0xa000, 0x2000, CRC(e1725d24) SHA1(d315588e6cc2f4263be621d2d8603c8215a90046) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "380_j18.2a",   0x0000, 0x020, CRC(10dd4eaa) SHA1(599acd25f36445221c553510a5de23ddba5ecc15) ) // palette
	ROM_LOAD( "380_j17.7b",   0x0020, 0x100, CRC(13989357) SHA1(0d61d468f6d3e1570fd18d236ec8cab92db4ed5c) ) // character lookup table
	ROM_LOAD( "380_j16.10c",  0x0120, 0x100, CRC(c244f2aa) SHA1(86df21c8e0b1ed51a0a4bd33dbb33f6efdea7d39) ) // sprite lookup table
ROM_END

} // anonymous namespace


GAME( 1984, circusc,  0,       circusc, circusc, circusc_state, empty_init, ROT90, "Konami",                   "Circus Charlie (level select, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, circusc2, circusc, circusc, circusc, circusc_state, empty_init, ROT90, "Konami",                   "Circus Charlie (level select, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, circusc3, circusc, circusc, circusc, circusc_state, empty_init, ROT90, "Konami",                   "Circus Charlie (level select, set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, circusc4, circusc, circusc, circusc, circusc_state, empty_init, ROT90, "Konami",                   "Circus Charlie (no level select)",     MACHINE_SUPPORTS_SAVE )
GAME( 1984, circuscc, circusc, circusc, circusc, circusc_state, empty_init, ROT90, "Konami (Centuri license)", "Circus Charlie (Centuri)",             MACHINE_SUPPORTS_SAVE )
GAME( 1984, circusce, circusc, circusc, circusc, circusc_state, empty_init, ROT90, "Konami (Centuri license)", "Circus Charlie (Centuri, earlier)",    MACHINE_SUPPORTS_SAVE )
