// license:BSD-3-Clause
// copyright-holders: Nicola Salmoria, Couriersud

/***************************************************************************

Gyruss memory map (preliminary)

Main processor memory map.
0000-5fff ROM (6000-7fff diagnostics)
8000-83ff Color RAM
8400-87ff Video RAM
9000-a7ff RAM
a000-a17f \ sprites
a200-a27f /

memory mapped ports:

read:
c080      IN0  (system inputs)
c0a0      IN1
c0c0      IN2
c0e0      DSW1
c000      DSW2
c100      DSW3

write:
a000-a1ff  Odd frame spriteram
a200-a3ff  Even frame spriteram
a700       Frame odd or even?
a701       Semaphore system:  tells 6809 to draw queued sprites
a702       Semaphore system:  tells 6809 to queue sprites
c000       watchdog reset
c080       trigger interrupt on audio CPU
c100       command for the audio CPU
c180       interrupt enable
c185       flip screen

interrupts:
standard NMI at 0x66


SOUND BOARD:
0000-3fff  Audio ROM (4000-5fff diagnostics)
6000-63ff  Audio RAM
8000       Read Sound Command

I/O:

Gyruss has 5 PSGs:
1)  Control: 0x00    Read: 0x01    Write: 0x02
2)  Control: 0x04    Read: 0x05    Write: 0x06
3)  Control: 0x08    Read: 0x09    Write: 0x0a
4)  Control: 0x0c    Read: 0x0d    Write: 0x0e
5)  Control: 0x10    Read: 0x11    Write: 0x12

and 1 SFX channel controlled by an 8039:
1)  SoundOn: 0x14    SoundData: 0x18

***************************************************************************/

#include "emu.h"

#include "konami1.h"
#include "konamipt.h"

#include "cpu/m6809/m6809.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/discrete.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_FILTER     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_FILTER)

#include "logmacro.h"

#define LOGFILTER(...)     LOGMASKED(LOG_FILTER,     __VA_ARGS__)


namespace {

class gyruss_state : public driver_device
{
public:
	gyruss_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_audiocpu_2(*this, "audio2"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram")
	{ }

	void gyruss(machine_config &config);

	void init_gyruss();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<i8039_device> m_audiocpu_2;
	required_device<discrete_sound_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t *m_tilemap = nullptr;
	uint8_t m_master_nmi_mask = 0U;
	uint8_t m_slave_irq_mask = 0U;
	bool m_flipscreen = false;

	void irq_clear_w(uint8_t data);
	void sh_irqtrigger_w(uint8_t data);
	void i8039_irq_w(uint8_t data);
	void master_nmi_mask_w(int state);
	void slave_irq_mask_w(uint8_t data);
	template <uint8_t Which> void coin_counter_w(int state);
	void spriteram_w(offs_t offset, uint8_t data);
	uint8_t scanline_r();
	void flipscreen_w(int state);
	uint8_t porta_r();
	void dac_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	template <uint8_t Which> void filter_w(uint8_t data);

	void audio_cpu1_io_map(address_map &map) ATTR_COLD;
	void audio_cpu1_map(address_map &map) ATTR_COLD;
	void audio_cpu2_io_map(address_map &map) ATTR_COLD;
	void audio_cpu2_map(address_map &map) ATTR_COLD;
	void main_cpu1_map(address_map &map) ATTR_COLD;
	void main_cpu2_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Gyruss has one 32x8 palette PROM and two 256x4 lookup table PROMs
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

void gyruss_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double weights_rg[3], weights_b[2];
	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, weights_rg, 470, 0,
			2, resistances_b,  weights_b,  470, 0,
			0, nullptr, nullptr, 0, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(weights_rg, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(weights_rg, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(weights_b, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 32;

	// sprites map to the lower 16 palette entries
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}

	// characters map to the upper 16 palette entries
	for (int i = 0x100; i < 0x140; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry | 0x10);
	}
}



void gyruss_state::spriteram_w(offs_t offset, uint8_t data)
{
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());
	m_spriteram[offset] = data;
}


TILE_GET_INFO_MEMBER(gyruss_state::get_tile_info)
{
	int const code = ((m_colorram[tile_index] & 0x20) << 3) | m_videoram[tile_index];
	int const color = m_colorram[tile_index] & 0x0f;
	int const flags = TILE_FLIPYX(m_colorram[tile_index] >> 6);

	tileinfo.group = (m_colorram[tile_index] & 0x10) ? 0 : 1;

	tileinfo.set(2, code, color, flags);
}


void gyruss_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gyruss_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap->set_transmask(0, 0x00, 0); // opaque
	m_tilemap->set_transmask(1, 0x0f, 0); // transparent

	save_item(NAME(m_flipscreen));
}



uint8_t gyruss_state::scanline_r()
{
	// reads 1V - 128V
	return m_screen->vpos();
}


void gyruss_state::flipscreen_w(int state)
{
	m_flipscreen = state;
}


void gyruss_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0xbc; offs >= 0; offs -= 4)
	{
		int const x = m_spriteram[offs];
		int const y = 241 - m_spriteram[offs + 3];

		int const gfx_bank = m_spriteram[offs + 1] & 0x01;
		int const code = ((m_spriteram[offs + 2] & 0x20) << 2) | (m_spriteram[offs + 1] >> 1);
		int const color = m_spriteram[offs + 2] & 0x0f;
		int const flip_x = ~m_spriteram[offs + 2] & 0x40;
		int const flip_y =  m_spriteram[offs + 2] & 0x80;

		m_gfxdecode->gfx(gfx_bank)->transpen(bitmap, cliprect, code, color, flip_x, flip_y, x, y, 0);
	}
}


uint32_t gyruss_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (cliprect.min_y == screen.visible_area().min_y)
	{
		machine().tilemap().mark_all_dirty();
		machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
	}

	m_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	draw_sprites(bitmap, cliprect);
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


/* The timer clock which feeds the upper 4 bits of                      */
/* AY-3-8910 port A is based on the same clock                          */
/* feeding the sound CPU Z80.  It is a divide by                        */
/* 10240, formed by a standard divide by 1024,                          */
/* followed by a divide by 10 using a 4 bit                             */
/* bi-quinary count sequence. (See LS90 data sheet                      */
/* for an example).                                                     */
/*                                                                      */
/* Bit 0 comes from the output of the divide by 1024                    */
/*       0, 1, 0, 1, 0, 1, 0, 1, 0, 1                                   */
/* Bit 1 comes from the QC output of the LS90 producing a sequence of   */
/*       0, 0, 1, 1, 0, 0, 1, 1, 1, 0                                   */
/* Bit 2 comes from the QD output of the LS90 producing a sequence of   */
/*       0, 0, 0, 0, 1, 0, 0, 0, 0, 1                                   */
/* Bit 3 comes from the QA output of the LS90 producing a sequence of   */
/*       0, 0, 0, 0, 0, 1, 1, 1, 1, 1                                   */

static const int timer[10] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x09, 0x0a, 0x0b, 0x0a, 0x0d
};

uint8_t gyruss_state::porta_r()
{
	return timer[(m_audiocpu->total_cycles() / 1024) % 10];
}


void gyruss_state::dac_w(uint8_t data)
{
	m_discrete->write(NODE(16), data);
}

void gyruss_state::irq_clear_w(uint8_t data)
{
	m_audiocpu_2->set_input_line(0, CLEAR_LINE);
}

template <uint8_t Which>
void gyruss_state::filter_w(uint8_t data)
{
	LOGFILTER("chip %d - %02x\n", Which, data);
	for (int i = 0; i < 3; i++)
	{
		// low bit: 47000pF = 0.047uF
		// high bit: 220000pF = 0.22uF
		m_discrete->write(NODE(3 * Which + i + 21), data & 3);
		data >>= 2;
	}
}

void gyruss_state::sh_irqtrigger_w(uint8_t data)
{
	// writing to this register triggers IRQ on the sound CPU
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

void gyruss_state::i8039_irq_w(uint8_t data)
{
	m_audiocpu_2->set_input_line(0, ASSERT_LINE);
}

void gyruss_state::master_nmi_mask_w(int state)
{
	m_master_nmi_mask = state;
	if (!m_master_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void gyruss_state::slave_irq_mask_w(uint8_t data)
{
	m_slave_irq_mask = data & 1;
	if (!m_slave_irq_mask)
		m_subcpu->set_input_line(0, CLEAR_LINE);
}

template <uint8_t Which>
void gyruss_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}

void gyruss_state::main_cpu1_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram().share(m_colorram);
	map(0x8400, 0x87ff).ram().share(m_videoram);
	map(0x9000, 0x9fff).ram();
	map(0xa000, 0xa7ff).ram().share("main_cpus");
	map(0xc000, 0xc000).portr("DSW2").nopw();   // watchdog reset
	map(0xc080, 0xc080).portr("SYSTEM").w(FUNC(gyruss_state::sh_irqtrigger_w));
	map(0xc0a0, 0xc0a0).portr("P1");
	map(0xc0c0, 0xc0c0).portr("P2");
	map(0xc0e0, 0xc0e0).portr("DSW1");
	map(0xc100, 0xc100).portr("DSW3").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xc180, 0xc187).w("mainlatch", FUNC(ls259_device::write_d0));
}

void gyruss_state::main_cpu2_map(address_map &map)
{
	map(0x0000, 0x0000).r(FUNC(gyruss_state::scanline_r));
	map(0x2000, 0x2000).w(FUNC(gyruss_state::slave_irq_mask_w)).nopr();
	map(0x4000, 0x403f).ram();
	map(0x4040, 0x40ff).ram().w(FUNC(gyruss_state::spriteram_w)).share(m_spriteram);
	map(0x4100, 0x47ff).ram();
	map(0x6000, 0x67ff).ram().share("main_cpus");
	map(0xe000, 0xffff).rom();
}

void gyruss_state::audio_cpu1_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x63ff).ram();
	map(0x8000, 0x8000).r("soundlatch", FUNC(generic_latch_8_device::read));
}

void gyruss_state::audio_cpu1_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("ay1", FUNC(ay8910_device::address_w));
	map(0x01, 0x01).r("ay1", FUNC(ay8910_device::data_r));
	map(0x02, 0x02).w("ay1", FUNC(ay8910_device::data_w));
	map(0x04, 0x04).w("ay2", FUNC(ay8910_device::address_w));
	map(0x05, 0x05).r("ay2", FUNC(ay8910_device::data_r));
	map(0x06, 0x06).w("ay2", FUNC(ay8910_device::data_w));
	map(0x08, 0x08).w("ay3", FUNC(ay8910_device::address_w));
	map(0x09, 0x09).r("ay3", FUNC(ay8910_device::data_r));
	map(0x0a, 0x0a).w("ay3", FUNC(ay8910_device::data_w));
	map(0x0c, 0x0c).w("ay4", FUNC(ay8910_device::address_w));
	map(0x0d, 0x0d).r("ay4", FUNC(ay8910_device::data_r));
	map(0x0e, 0x0e).w("ay4", FUNC(ay8910_device::data_w));
	map(0x10, 0x10).w("ay5", FUNC(ay8910_device::address_w));
	map(0x11, 0x11).r("ay5", FUNC(ay8910_device::data_r));
	map(0x12, 0x12).w("ay5", FUNC(ay8910_device::data_w));
	map(0x14, 0x14).w(FUNC(gyruss_state::i8039_irq_w));
	map(0x18, 0x18).w("soundlatch2", FUNC(generic_latch_8_device::write));
}

void gyruss_state::audio_cpu2_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}

void gyruss_state::audio_cpu2_io_map(address_map &map)
{
	map(0x00, 0xff).r("soundlatch2", FUNC(generic_latch_8_device::read));
}


static INPUT_PORTS_START( gyruss )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // 1p shoot 2 - unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    // 2p shoot 2 - unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), DEF_STR( Free_Play ), SW1)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:4")     // tables at 0x1653 (15 bytes) or 0x4bf3 (13 bytes)
	PORT_DIPSETTING(    0x08, "30k 90k 60k+" )              // last bonus life at 810k : max. 14 bonus lives
	PORT_DIPSETTING(    0x00, "40k 110k 70k+" )             // last bonus life at 810k : max. 12 bonus lives
	PORT_DIPNAME( 0x70, 0x30, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:5,6,7")
	PORT_DIPSETTING(    0x70, "1 (Easiest)" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x50, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x30, "5 (Average)" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x10, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "Demo Music" )                PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC(0xfe, IP_ACTIVE_LOW, "SW3:2,3,4,5,6,7,8")
INPUT_PORTS_END

static INPUT_PORTS_START( gyrussce )
	PORT_INCLUDE( gyruss )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:4")     // tables at 0x1653 (15 bytes) or 0x4bf3 (13 bytes)
	PORT_DIPSETTING(    0x08, "50k 120k 70k+" )             // last bonus life at 960k : max. 14 bonus lives
	PORT_DIPSETTING(    0x00, "60k 140k 80k+" )             // last bonus life at 940k : max. 12 bonus lives
	PORT_DIPNAME( 0x70, 0x20, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:5,6,7") // "Difficult" default setting according to Centuri manual
	PORT_DIPSETTING(    0x70, "1 (Easiest)" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x50, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x30, "5 (Average)" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x10, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	512,    // 512 characters
	2,  // 2 bits per pixel
	{ 4, 0 },
	{ 0, 1, 2, 3, 8*8+0,8*8+1,8*8+2,8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8    // every char takes 16 consecutive bytes
};

static const gfx_layout spritelayout =
{
	8,16,   // 8*16 sprites
	256,    // 256 sprites
	4,  // 4 bits per pixel
	{ 0x4000*8+4, 0x4000*8+0, 4, 0  },
	{ 0, 1, 2, 3,  8*8, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8    // every sprite takes 64 consecutive bytes
};


static GFXDECODE_START( gfx_gyruss )
	GFXDECODE_ENTRY( "sprites", 0x0000, spritelayout, 0, 16 )  // upper half
	GFXDECODE_ENTRY( "sprites", 0x0010, spritelayout, 0, 16 )  // lower half
	GFXDECODE_ENTRY( "tiles",   0x0000, charlayout,   16*16, 16 )
GFXDECODE_END

static const discrete_mixer_desc konami_right_mixer_desc =
	{DISC_MIXER_IS_RESISTOR,
	{RES_K(2.2), RES_K(2.2), RES_K(2.2), RES_K(3.3)/3, RES_K(3.3)/3 },
	{0,0,0,0,0,0},  // no variable resistors
	{0,0,0,0,0,0},  // no node capacitors
	0, 200,
	CAP_U(0.1),
	CAP_U(1),       // DC - Removal, not in schematics
	0, 1};

static const discrete_mixer_desc konami_left_mixer_desc =
	{DISC_MIXER_IS_RESISTOR,
	{RES_K(2.2), RES_K(2.2), RES_K(2.2), RES_K(3.3)/3, RES_K(4.7) },
	{0,0,0,0,0,0},  // no variable resistors
	{0,0,0,0,0,0},  // no node capacitors
	0, 200,
	CAP_U(0.1),
	CAP_U(1),       // DC - Removal, not in schematics
	0, 1};

static DISCRETE_SOUND_START( sound_discrete )

	// Chip 1 right
	DISCRETE_INPUTX_STREAM(NODE_01, 0, 1.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_02, 1, 1.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_03, 2, 1.0, 0)

	// Chip 2 left
	DISCRETE_INPUTX_STREAM(NODE_04, 3, 1.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_05, 4, 1.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_06, 5, 1.0, 0)

	// Chip 3 right
	/* Outputs are tied together after 3.3k resistor on each channel.
	 * A/R + B/R + C/R = (A + B + C) / 3 * (1/(R/3))
	 */
	DISCRETE_INPUTX_STREAM(NODE_07, 6, 0.33, 0)
	DISCRETE_INPUTX_STREAM(NODE_08, 7, 0.33, 0)
	DISCRETE_INPUTX_STREAM(NODE_09, 8, 0.33, 0)

	// Chip 4 right
	DISCRETE_INPUTX_STREAM(NODE_10, 9, 0.33, 0)
	DISCRETE_INPUTX_STREAM(NODE_11,10, 0.33, 0)
	DISCRETE_INPUTX_STREAM(NODE_12,11, 0.33, 0)

	// Chip 5 left
	DISCRETE_INPUTX_STREAM(NODE_13,12, 0.33, 0)
	DISCRETE_INPUTX_STREAM(NODE_14,13, 0.33, 0)
	DISCRETE_INPUTX_STREAM(NODE_15,14, 0.33, 0)

	// DAC left
	/* Output voltage depends on load. Datasheet gives 2.4 as minimum.
	 * This is in line with TTL, so 4V with no load seems adequate */
	DISCRETE_INPUTX_DATA(NODE_16, 256.0 * 4.0 / 5.0, 0.0, 0.0)

	// Chip 1 Filter enable
	DISCRETE_INPUT_DATA(NODE_21)
	DISCRETE_INPUT_DATA(NODE_22)
	DISCRETE_INPUT_DATA(NODE_23)

	// Chip 2 Filter enable
	DISCRETE_INPUT_DATA(NODE_24)
	DISCRETE_INPUT_DATA(NODE_25)
	DISCRETE_INPUT_DATA(NODE_26)

	// Chip 1 Filter
	DISCRETE_RCFILTER_SW(NODE_31, 1, NODE_01, NODE_21, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.047), CAP_U(0.22), 0, 0)
	DISCRETE_RCFILTER_SW(NODE_32, 1, NODE_02, NODE_22, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.047), CAP_U(0.22), 0, 0)
	DISCRETE_RCFILTER_SW(NODE_33, 1, NODE_03, NODE_23, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.047), CAP_U(0.22), 0, 0)

	// Chip 2 Filter
	DISCRETE_RCFILTER_SW(NODE_34, 1, NODE_04, NODE_24, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.047), CAP_U(0.22), 0, 0)
	DISCRETE_RCFILTER_SW(NODE_35, 1, NODE_05, NODE_25, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.047), CAP_U(0.22), 0, 0)
	DISCRETE_RCFILTER_SW(NODE_36, 1, NODE_06, NODE_26, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.047), CAP_U(0.22), 0, 0)

	// Chip 3
	DISCRETE_ADDER3(NODE_40, 1, NODE_07, NODE_08, NODE_09)
	// Chip 4
	DISCRETE_ADDER3(NODE_41, 1, NODE_10, NODE_11, NODE_12)
	// Chip 5
	DISCRETE_ADDER3(NODE_42, 1, NODE_13, NODE_14, NODE_15)

	// right channel
	DISCRETE_MIXER5(NODE_50, 1, NODE_31, NODE_32, NODE_33, NODE_40, NODE_41, &konami_right_mixer_desc)
	// left channel
	DISCRETE_MIXER5(NODE_51, 1, NODE_34, NODE_35, NODE_36, NODE_42, NODE_16, &konami_left_mixer_desc)

	DISCRETE_OUTPUT(NODE_50, 11.0)
	DISCRETE_OUTPUT(NODE_51, 11.0)

DISCRETE_SOUND_END


void gyruss_state::machine_start()
{
	save_item(NAME(m_master_nmi_mask));
	save_item(NAME(m_slave_irq_mask));
}

void gyruss_state::vblank_irq(int state)
{
	if (state && m_master_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	if (state && m_slave_irq_mask)
		m_subcpu->set_input_line(0, ASSERT_LINE);
}

void gyruss_state::gyruss(machine_config &config)
{
	static constexpr XTAL MASTER_CLOCK = XTAL(18'432'000);
	static constexpr XTAL SOUND_CLOCK = XTAL(14'318'181);

// Video timing
// PCB measured: H = 15.50khz V = 60.56hz, +/- 0.01hz
// --> VTOTAL should be OK, HTOTAL not 100% certain
	static constexpr XTAL PIXEL_CLOCK = MASTER_CLOCK / 3;

	static constexpr int HTOTAL  = 396;
	static constexpr int HBEND   = 0;
	static constexpr int HBSTART = 256;

	static constexpr int VTOTAL  = 256;
	static constexpr int VBEND   = 0+2*8;
	static constexpr int VBSTART = 224+2*8;

	// basic machine hardware
	Z80(config, m_maincpu, MASTER_CLOCK / 6);    // 3.072 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &gyruss_state::main_cpu1_map);

	KONAMI1(config, m_subcpu, MASTER_CLOCK / 12);     // 1.536 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &gyruss_state::main_cpu2_map);

	Z80(config, m_audiocpu, SOUND_CLOCK / 4);    // 3.579545 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &gyruss_state::audio_cpu1_map);
	m_audiocpu->set_addrmap(AS_IO, &gyruss_state::audio_cpu1_io_map);

	I8039(config, m_audiocpu_2, XTAL(8'000'000));
	m_audiocpu_2->set_addrmap(AS_PROGRAM, &gyruss_state::audio_cpu2_map);
	m_audiocpu_2->set_addrmap(AS_IO, &gyruss_state::audio_cpu2_io_map);
	m_audiocpu_2->p1_out_cb().set(FUNC(gyruss_state::dac_w));
	m_audiocpu_2->p2_out_cb().set(FUNC(gyruss_state::irq_clear_w));

	config.set_maximum_quantum(attotime::from_hz(6000));

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 3C
	mainlatch.q_out_cb<0>().set(FUNC(gyruss_state::master_nmi_mask_w));
	mainlatch.q_out_cb<2>().set(FUNC(gyruss_state::coin_counter_w<0>));
	mainlatch.q_out_cb<3>().set(FUNC(gyruss_state::coin_counter_w<1>));
	mainlatch.q_out_cb<5>().set(FUNC(gyruss_state::flipscreen_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(gyruss_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(gyruss_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gyruss);
	PALETTE(config, m_palette, FUNC(gyruss_state::palette), 16*4+16*16, 32);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, "soundlatch");
	GENERIC_LATCH_8(config, "soundlatch2");

	ay8910_device &ay1(AY8910(config, "ay1", SOUND_CLOCK / 8));
	ay1.set_flags(AY8910_DISCRETE_OUTPUT);
	ay1.set_resistors_load(RES_K(3.3), RES_K(3.3), RES_K(3.3));
	ay1.port_b_write_callback().set(FUNC(gyruss_state::filter_w<0>));
	ay1.add_route(0, "discrete", 1.0, 0);
	ay1.add_route(1, "discrete", 1.0, 1);
	ay1.add_route(2, "discrete", 1.0, 2);

	ay8910_device &ay2(AY8910(config, "ay2", SOUND_CLOCK / 8));
	ay2.set_flags(AY8910_DISCRETE_OUTPUT);
	ay2.set_resistors_load(RES_K(3.3), RES_K(3.3), RES_K(3.3));
	ay2.port_b_write_callback().set(FUNC(gyruss_state::filter_w<1>));
	ay2.add_route(0, "discrete", 1.0, 3);
	ay2.add_route(1, "discrete", 1.0, 4);
	ay2.add_route(2, "discrete", 1.0, 5);

	ay8910_device &ay3(AY8910(config, "ay3", SOUND_CLOCK / 8));
	ay3.set_flags(AY8910_DISCRETE_OUTPUT);
	ay3.set_resistors_load(RES_K(3.3), RES_K(3.3), RES_K(3.3));
	ay3.port_a_read_callback().set(FUNC(gyruss_state::porta_r));
	ay3.add_route(0, "discrete", 1.0, 6);
	ay3.add_route(1, "discrete", 1.0, 7);
	ay3.add_route(2, "discrete", 1.0, 8);

	ay8910_device &ay4(AY8910(config, "ay4", SOUND_CLOCK / 8));
	ay4.set_flags(AY8910_DISCRETE_OUTPUT);
	ay4.set_resistors_load(RES_K(3.3), RES_K(3.3), RES_K(3.3));
	ay4.add_route(0, "discrete", 1.0, 9);
	ay4.add_route(1, "discrete", 1.0, 10);
	ay4.add_route(2, "discrete", 1.0, 11);

	ay8910_device &ay5(AY8910(config, "ay5", SOUND_CLOCK / 8));
	ay5.set_flags(AY8910_DISCRETE_OUTPUT);
	ay5.set_resistors_load(RES_K(3.3), RES_K(3.3), RES_K(3.3));
	ay5.add_route(0, "discrete", 1.0, 12);
	ay5.add_route(1, "discrete", 1.0, 13);
	ay5.add_route(2, "discrete", 1.0, 14);

	DISCRETE(config, m_discrete, sound_discrete);
	m_discrete->add_route(0, "rspeaker", 1.0);
	m_discrete->add_route(1, "lspeaker", 1.0);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( gyruss )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "gyrussk.1",    0x0000, 0x2000, CRC(c673b43d) SHA1(7c464fb154bac35dd6e2f547e157addeb8798194) )
	ROM_LOAD( "gyrussk.2",    0x2000, 0x2000, CRC(a4ec03e4) SHA1(08c33ad7fcc2ad5e5787a1050284e3f8164f4618) )
	ROM_LOAD( "gyrussk.3",    0x4000, 0x2000, CRC(27454a98) SHA1(030c7df225652ee20d5ef64d005eb011dc89a27d) )
	// Diagnostic ROM, not populated. Checksums are from Shoestring's unofficial version.
	// The game jumps to this location at startup if the first byte is 0x55.
#if 0
	ROM_LOAD( "gyrussk.4",    0x6000, 0x2000, CRC(7f28f9e4) SHA1(10f2de94f17ace513410cb358f67632b5b201896) )
#endif

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "gyrussk.9",    0xe000, 0x2000, CRC(822bf27e) SHA1(36d5bea2392a7d3476dd797dc05602705cfa23ef) )

	ROM_REGION( 0x6000, "audiocpu", 0 )
	ROM_LOAD( "gyrussk.1a",   0x0000, 0x2000, CRC(f4ae1c17) SHA1(ae568c96a31d910afe30d2b7eeb9ed1ed07290e3) )
	ROM_LOAD( "gyrussk.2a",   0x2000, 0x2000, CRC(ba498115) SHA1(9cd1f42898cc590f39ba7cb3c975b0b3d3062eba) )
	// 4000-5fff: Empty socket, not populated

	ROM_REGION( 0x1000, "audio2", 0 )   // 8039
	ROM_LOAD( "gyrussk.3a",   0x0000, 0x1000, CRC(3f9b5dea) SHA1(6e807da02c2885b18e8cc2199f12f6be9040bf75) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "gyrussk.6",    0x0000, 0x2000, CRC(c949db10) SHA1(fcb8bcbd2bdd751fecb322a33c8a92fb6f07a7ab) )
	ROM_LOAD( "gyrussk.5",    0x2000, 0x2000, CRC(4f22411a) SHA1(763bcd039f8c1838a0d7da7d4dadc14a26e25596) )
	ROM_LOAD( "gyrussk.8",    0x4000, 0x2000, CRC(47cd1fbc) SHA1(8203c4ff0b1cd7b4dbc708e300bfeac1e7366e09) )
	ROM_LOAD( "gyrussk.7",    0x6000, 0x2000, CRC(8e8d388c) SHA1(8f2928d71c02aba977d67575d6e34d69bda2b9d4) )

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "gyrussk.4",    0x0000, 0x2000, CRC(27d8329b) SHA1(564ff945465a23d93a93137ad277298770dfa06a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "gyrussk.pr3",  0x0000, 0x0020, CRC(98782db3) SHA1(b891e43b25187faca8002919ccb44d744daa3594) )    // palette
	ROM_LOAD( "gyrussk.pr1",  0x0020, 0x0100, CRC(7ed057de) SHA1(c04069ae1e2c62f9b3048844cd8cf5e1b03b7d3c) )    // sprite lookup table
	ROM_LOAD( "gyrussk.pr2",  0x0120, 0x0100, CRC(de823a81) SHA1(1af94b2a6a319a89b238a5076a2867f1cfd279b0) )    // character lookup table
ROM_END

ROM_START( gyrussce )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "gya-1.11j",    0x0000, 0x2000, CRC(85f8b7c2) SHA1(5dde696b53efedee671d500feae1d314e95b1c96) )
	ROM_LOAD( "gya-2.12j",    0x2000, 0x2000, CRC(1e1a970f) SHA1(5a2e391489608f7571bbb4f85549a79795e2177e) )
	ROM_LOAD( "gya-3.13j",    0x4000, 0x2000, CRC(f6dbb33b) SHA1(19cab8e7f2f2358b6271ab402f132654e8be95d4) )

	// Diagnostic ROM, not populated. Checksums are from Shoestring's unofficial version.
	// The game jumps to this location at startup if the first byte is 0x55.
#if 0
	ROM_LOAD( "gya-4.14j",    0x6000, 0x2000, CRC(7f28f9e4) SHA1(10f2de94f17ace513410cb358f67632b5b201896) )
#endif

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "gy-5.19e",    0xe000, 0x2000, CRC(822bf27e) SHA1(36d5bea2392a7d3476dd797dc05602705cfa23ef) )

	ROM_REGION( 0x6000, "audiocpu", 0 )
	ROM_LOAD( "gy-11.7a",   0x0000, 0x2000, CRC(f4ae1c17) SHA1(ae568c96a31d910afe30d2b7eeb9ed1ed07290e3) )
	ROM_LOAD( "gy-12.8a",   0x2000, 0x2000, CRC(ba498115) SHA1(9cd1f42898cc590f39ba7cb3c975b0b3d3062eba) )
	// 4000-5fff: Empty socket, not populated

	ROM_REGION( 0x1000, "audio2", 0 )   // 8039
	ROM_LOAD( "gy-13.11h",   0x0000, 0x1000, CRC(3f9b5dea) SHA1(6e807da02c2885b18e8cc2199f12f6be9040bf75) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "gy-10.9d",    0x0000, 0x2000, CRC(c949db10) SHA1(fcb8bcbd2bdd751fecb322a33c8a92fb6f07a7ab) )
	ROM_LOAD( "gy-9.8d",    0x2000, 0x2000, CRC(4f22411a) SHA1(763bcd039f8c1838a0d7da7d4dadc14a26e25596) )
	ROM_LOAD( "gy-8.7d",    0x4000, 0x2000, CRC(47cd1fbc) SHA1(8203c4ff0b1cd7b4dbc708e300bfeac1e7366e09) )
	ROM_LOAD( "gy-7.6d",    0x6000, 0x2000, CRC(8e8d388c) SHA1(8f2928d71c02aba977d67575d6e34d69bda2b9d4) )

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "gy-6.1g",    0x0000, 0x2000, CRC(27d8329b) SHA1(564ff945465a23d93a93137ad277298770dfa06a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "gyrussk.pr3",  0x0000, 0x0020, CRC(98782db3) SHA1(b891e43b25187faca8002919ccb44d744daa3594) )    // palette
	ROM_LOAD( "gyrussk.pr1",  0x0020, 0x0100, CRC(7ed057de) SHA1(c04069ae1e2c62f9b3048844cd8cf5e1b03b7d3c) )    // sprite lookup table
	ROM_LOAD( "gyrussk.pr2",  0x0120, 0x0100, CRC(de823a81) SHA1(1af94b2a6a319a89b238a5076a2867f1cfd279b0) )    // character lookup table
ROM_END

ROM_START( gyrussb ) // PCB has stickers stating "TAITO (NEW ZEALAND) LTD"
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "1.bin", 0x0000, 0x2000, CRC(6bc21c10) SHA1(9d44f766398b9994f90edb2ffb272b4f22564854) ) // minor code patch / redirection
	ROM_LOAD( "2.bin", 0x2000, 0x2000, CRC(a4ec03e4) SHA1(08c33ad7fcc2ad5e5787a1050284e3f8164f4618) )
	ROM_LOAD( "3.bin", 0x4000, 0x2000, CRC(27454a98) SHA1(030c7df225652ee20d5ef64d005eb011dc89a27d) )
	// 6000-7fff: Empty socket, space for diagnostics ROM

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "9.bin", 0xe000, 0x2000, CRC(822bf27e) SHA1(36d5bea2392a7d3476dd797dc05602705cfa23ef) )

	ROM_REGION( 0x6000, "audiocpu", 0 )
	ROM_LOAD( "11.bin",   0x0000, 0x2000, CRC(f4ae1c17) SHA1(ae568c96a31d910afe30d2b7eeb9ed1ed07290e3) )
	ROM_LOAD( "12.bin",   0x2000, 0x2000, CRC(ba498115) SHA1(9cd1f42898cc590f39ba7cb3c975b0b3d3062eba) )
	// 4000-5fff: Empty socket, not populated

	ROM_REGION( 0x1000, "audio2", 0 )   // 8039
	ROM_LOAD( "13.bin",   0x0000, 0x1000, CRC(3f9b5dea) SHA1(6e807da02c2885b18e8cc2199f12f6be9040bf75) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "6.bin", 0x0000, 0x2000, CRC(c949db10) SHA1(fcb8bcbd2bdd751fecb322a33c8a92fb6f07a7ab) )
	ROM_LOAD( "5.bin", 0x2000, 0x2000, CRC(4f22411a) SHA1(763bcd039f8c1838a0d7da7d4dadc14a26e25596) )
	ROM_LOAD( "8.bin", 0x4000, 0x2000, CRC(47cd1fbc) SHA1(8203c4ff0b1cd7b4dbc708e300bfeac1e7366e09) )
	ROM_LOAD( "7.bin", 0x6000, 0x2000, CRC(8e8d388c) SHA1(8f2928d71c02aba977d67575d6e34d69bda2b9d4) )

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "4.bin", 0x0000, 0x2000, CRC(27d8329b) SHA1(564ff945465a23d93a93137ad277298770dfa06a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "gyrussk.pr3",  0x0000, 0x0020, CRC(98782db3) SHA1(b891e43b25187faca8002919ccb44d744daa3594) )    // palette
	ROM_LOAD( "gyrussk.pr1",  0x0020, 0x0100, CRC(7ed057de) SHA1(c04069ae1e2c62f9b3048844cd8cf5e1b03b7d3c) )    // sprite lookup table
	ROM_LOAD( "gyrussk.pr2",  0x0120, 0x0100, CRC(de823a81) SHA1(1af94b2a6a319a89b238a5076a2867f1cfd279b0) )    // character lookup table
ROM_END

ROM_START( venus )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "r1",           0x0000, 0x2000, CRC(d030abb1) SHA1(14a70e15f5df9ef957779771d8915203d3828532) )
	ROM_LOAD( "r2",           0x2000, 0x2000, CRC(dbf65d4d) SHA1(a0ad0dc3420442f06691bda2115fadd961ce86a7) )
	ROM_LOAD( "r3",           0x4000, 0x2000, CRC(db246fcd) SHA1(c0228b35591c9e1c778370a2abd3739c441f14aa) )
	// 6000-7fff: Empty socket, space for diagnostics ROM

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "gyrussk.9",    0xe000, 0x2000, CRC(822bf27e) SHA1(36d5bea2392a7d3476dd797dc05602705cfa23ef) )

	ROM_REGION( 0x6000, "audiocpu", 0 )
	ROM_LOAD( "gyrussk.1a",   0x0000, 0x2000, CRC(f4ae1c17) SHA1(ae568c96a31d910afe30d2b7eeb9ed1ed07290e3) )
	ROM_LOAD( "gyrussk.2a",   0x2000, 0x2000, CRC(ba498115) SHA1(9cd1f42898cc590f39ba7cb3c975b0b3d3062eba) )
	// 4000-5fff: Empty socket, not populated

	ROM_REGION( 0x1000, "audio2", 0 )   // 8039
	ROM_LOAD( "gyrussk.3a",   0x0000, 0x1000, CRC(3f9b5dea) SHA1(6e807da02c2885b18e8cc2199f12f6be9040bf75) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "gyrussk.6",    0x0000, 0x2000, CRC(c949db10) SHA1(fcb8bcbd2bdd751fecb322a33c8a92fb6f07a7ab) )
	ROM_LOAD( "gyrussk.5",    0x2000, 0x2000, CRC(4f22411a) SHA1(763bcd039f8c1838a0d7da7d4dadc14a26e25596) )
	ROM_LOAD( "gyrussk.8",    0x4000, 0x2000, CRC(47cd1fbc) SHA1(8203c4ff0b1cd7b4dbc708e300bfeac1e7366e09) )
	ROM_LOAD( "gyrussk.7",    0x6000, 0x2000, CRC(8e8d388c) SHA1(8f2928d71c02aba977d67575d6e34d69bda2b9d4) )

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "gyrussk.4",    0x0000, 0x2000, CRC(27d8329b) SHA1(564ff945465a23d93a93137ad277298770dfa06a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "gyrussk.pr3",  0x0000, 0x0020, CRC(98782db3) SHA1(b891e43b25187faca8002919ccb44d744daa3594) )    // palette
	ROM_LOAD( "gyrussk.pr1",  0x0020, 0x0100, CRC(7ed057de) SHA1(c04069ae1e2c62f9b3048844cd8cf5e1b03b7d3c) )    // sprite lookup table
	ROM_LOAD( "gyrussk.pr2",  0x0120, 0x0100, CRC(de823a81) SHA1(1af94b2a6a319a89b238a5076a2867f1cfd279b0) )    // character lookup table
ROM_END

} // anonymous namespace


GAME( 1983, gyruss,   0,      gyruss, gyruss,   gyruss_state, empty_init, ROT90, "Konami",                   "Gyruss",                    MACHINE_SUPPORTS_SAVE )
GAME( 1983, gyrussce, gyruss, gyruss, gyrussce, gyruss_state, empty_init, ROT90, "Konami (Centuri license)", "Gyruss (Centuri)",          MACHINE_SUPPORTS_SAVE )
GAME( 1983, gyrussb,  gyruss, gyruss, gyruss,   gyruss_state, empty_init, ROT90, "bootleg?",                 "Gyruss (bootleg?)",         MACHINE_SUPPORTS_SAVE ) // Supposed Taito NZ license, but (c) Konami
GAME( 1983, venus,    gyruss, gyruss, gyruss,   gyruss_state, empty_init, ROT90, "bootleg",                  "Venus (bootleg of Gyruss)", MACHINE_SUPPORTS_SAVE )
