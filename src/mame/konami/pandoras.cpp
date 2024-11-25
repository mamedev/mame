// license:BSD-3-Clause
// copyright-holders: Manuel Abadia

/***************************************************************************

Pandora's Palace(GX328) (c) 1984 Konami/Interlogic

Driver by Manuel Abadia <emumanu+mame@gmail.com>

Notes:
- Press 1P and 2P together to enter test mode.

TODO:
- CPU B continuously reads from 1e00. It seems to be important, could be a
  scanline counter or something like that.

2009-03:
Added dsw locations and verified factory setting based on Guru's notes
(DSW3 not mentioned)

Boards:
- CPU/Video board labeled PWB(A)2000109B
- Sound board labeled PWB(B)3000154A

***************************************************************************/

#include "emu.h"

#include "konamipt.h"

#include "cpu/m6809/m6809.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class pandoras_state : public driver_device
{
public:
	pandoras_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void pandoras(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<i8039_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// video-related
	tilemap_t *m_layer0 = nullptr;

	uint8_t m_irq_enable_a = 0;
	uint8_t m_irq_enable_b = 0;
	uint8_t m_firq_old_data_a = 0;
	uint8_t m_firq_old_data_b = 0;
	uint8_t m_i8039_status = 0;

	void cpua_irq_enable_w(int state);
	void cpub_irq_enable_w(int state);
	void cpua_irqtrigger_w(uint8_t data);
	void cpub_irqtrigger_w(uint8_t data);
	void i8039_irqtrigger_w(uint8_t data);
	void i8039_irqen_and_status_w(uint8_t data);
	void z80_irqtrigger_w(uint8_t data);
	template <uint8_t Which> void coin_counter_w(int state);
	void vram_w(offs_t offset, uint8_t data);
	void cram_w(offs_t offset, uint8_t data);
	void scrolly_w(uint8_t data);
	uint8_t porta_r();
	uint8_t portb_r();
	TILE_GET_INFO_MEMBER(get_tile_info0);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* sr);

	void i8039_io_map(address_map &map) ATTR_COLD;
	void i8039_map(address_map &map) ATTR_COLD;
	void master_map(address_map &map) ATTR_COLD;
	void slave_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***********************************************************************

  Convert the color PROMs into a more useable format.

  Pandora's Palace has one 32x8 palette PROM and two 256x4 lookup table
  PROMs (one for characters, one for sprites).
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

void pandoras_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 1000, 0,
			3, &resistances_rg[0], gweights, 1000, 0,
			2, &resistances_b[0],  bweights, 1000, 0);

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
	color_prom += 0x20;

	// sprites
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}

	// characters
	for (int i = 0x100; i < 0x200; i++)
	{
		uint8_t const ctabentry = (color_prom[i] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(pandoras_state::get_tile_info0)
{
	uint8_t const attr = m_colorram[tile_index];
	tileinfo.set(1,
			m_videoram[tile_index] + ((attr & 0x10) << 4),
			attr & 0x0f,
			TILE_FLIPYX((attr & 0xc0) >> 6));
	tileinfo.category = (attr & 0x20) >> 5;
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void pandoras_state::video_start()
{
	m_layer0 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pandoras_state::get_tile_info0)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

/***************************************************************************

  Memory Handlers

***************************************************************************/

void pandoras_state::vram_w(offs_t offset, uint8_t data)
{
	m_layer0->mark_tile_dirty(offset);
	m_videoram[offset] = data;
}

void pandoras_state::cram_w(offs_t offset, uint8_t data)
{
	m_layer0->mark_tile_dirty(offset);
	m_colorram[offset] = data;
}

void pandoras_state::scrolly_w(uint8_t data)
{
	m_layer0->set_scrolly(0, data);
}


/***************************************************************************

  Screen Refresh

***************************************************************************/

void pandoras_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* sr)
{
	for (int offs = 0; offs < 0x100; offs += 4)
	{
		int const sx = sr[offs + 1];
		int const sy = 240 - sr[offs];
		int const color = sr[offs + 3] & 0x0f;
		int const nflipx = sr[offs + 3] & 0x40;
		int const nflipy = sr[offs + 3] & 0x80;

		m_gfxdecode->gfx(0)->transmask(bitmap, cliprect,
			sr[offs + 2],
			color,
			!nflipx, !nflipy,
			sx, sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(0), color, 0));
	}
}

uint32_t pandoras_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_layer0->draw(screen, bitmap, cliprect, 1 ,0);
	draw_sprites(bitmap, cliprect, &m_spriteram[0x800]);
	m_layer0->draw(screen, bitmap, cliprect, 0 ,0);
	return 0;
}


void pandoras_state::vblank_irq(int state)
{
	if (state && m_irq_enable_a)
		m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
	if (state && m_irq_enable_b)
		m_subcpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

void pandoras_state::cpua_irq_enable_w(int state)
{
	if (!state)
		m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
	m_irq_enable_a = state;
}

void pandoras_state::cpub_irq_enable_w(int state)
{
	if (!state)
		m_subcpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
	m_irq_enable_b = state;
}

void pandoras_state::cpua_irqtrigger_w(uint8_t data)
{
	if (!m_firq_old_data_a && data)
		m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);

	m_firq_old_data_a = data;
}

void pandoras_state::cpub_irqtrigger_w(uint8_t data)
{
	if (!m_firq_old_data_b && data)
		m_subcpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);

	m_firq_old_data_b = data;
}

void pandoras_state::i8039_irqtrigger_w(uint8_t data)
{
	m_mcu->set_input_line(0, ASSERT_LINE);
}

void pandoras_state::i8039_irqen_and_status_w(uint8_t data)
{
	// bit 7 enables IRQ
	if ((data & 0x80) == 0)
		m_mcu->set_input_line(0, CLEAR_LINE);

	// bit 5 goes to 8910 port A
	m_i8039_status = (data & 0x20) >> 5;
}

void pandoras_state::z80_irqtrigger_w(uint8_t data)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

template <uint8_t Which>
void pandoras_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}


void pandoras_state::master_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share(m_spriteram);                                // shared with CPU B
	map(0x1000, 0x13ff).ram().w(FUNC(pandoras_state::cram_w)).share(m_colorram); // shared with CPU B
	map(0x1400, 0x17ff).ram().w(FUNC(pandoras_state::vram_w)).share(m_videoram); // shared with CPU B
	map(0x1800, 0x1807).w("mainlatch", FUNC(ls259_device::write_d0));            // INT control
	map(0x1a00, 0x1a00).w(FUNC(pandoras_state::scrolly_w));
	map(0x1c00, 0x1c00).w(FUNC(pandoras_state::z80_irqtrigger_w));
	map(0x1e00, 0x1e00).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x2000, 0x2000).w(FUNC(pandoras_state::cpub_irqtrigger_w));
	map(0x2001, 0x2001).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x4000, 0x5fff).rom();                                                   // space for diagnostic ROM
	map(0x6000, 0x67ff).ram().share("cpua_cpub");
	map(0x8000, 0xffff).rom();
}

void pandoras_state::slave_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share(m_spriteram);                                // shared with CPU A
	map(0x1000, 0x13ff).ram().w(FUNC(pandoras_state::cram_w)).share(m_colorram); // shared with CPU A
	map(0x1400, 0x17ff).ram().w(FUNC(pandoras_state::vram_w)).share(m_videoram); // shared with CPU A
	map(0x1800, 0x1800).portr("DSW1");
	map(0x1800, 0x1807).w("mainlatch", FUNC(ls259_device::write_d0));            // INT control
	map(0x1a00, 0x1a00).portr("SYSTEM");
	map(0x1a01, 0x1a01).portr("P1");
	map(0x1a02, 0x1a02).portr("P2");
	map(0x1a03, 0x1a03).portr("DSW3");
	map(0x1c00, 0x1c00).portr("DSW2");
//  map(0x1e00, 0x1e00).nopr();                                                     // ??? seems to be important
	map(0x8000, 0x8000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xa000, 0xa000).w(FUNC(pandoras_state::cpua_irqtrigger_w));
	map(0xc000, 0xc7ff).ram().share("cpua_cpub");
	map(0xe000, 0xffff).rom();
}

void pandoras_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).ram();
	map(0x4000, 0x4000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x6000, 0x6000).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x6001, 0x6001).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x6002, 0x6002).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x8000, 0x8000).w(FUNC(pandoras_state::i8039_irqtrigger_w));
	map(0xa000, 0xa000).w("soundlatch2", FUNC(generic_latch_8_device::write));
}

void pandoras_state::i8039_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}

void pandoras_state::i8039_io_map(address_map &map)
{
	map(0x00, 0xff).r("soundlatch2", FUNC(generic_latch_8_device::read));
}


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( pandoras )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	// "No Coin B" = coins produce sound, but no effect on coin counter

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20k and every 60k" )
	PORT_DIPSETTING(    0x10, "30k and every 70k" )
	PORT_DIPSETTING(    0x08, "20k" )
	PORT_DIPSETTING(    0x00, "30k" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Freeze" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_10
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	KONAMI8_MONO_B1_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_B1_UNK
INPUT_PORTS_END



static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 15*4, 14*4, 13*4, 12*4, 11*4, 10*4, 9*4, 8*4,
			7*4, 6*4, 5*4, 4*4, 3*4, 2*4, 1*4, 0*4 },
	{ 15*4*16, 14*4*16, 13*4*16, 12*4*16, 11*4*16, 10*4*16, 9*4*16, 8*4*16,
			7*4*16, 6*4*16, 5*4*16, 4*4*16, 3*4*16, 2*4*16, 1*4*16, 0*4*16 },
	32*4*8
};

static GFXDECODE_START( gfx_pandoras )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,             0, 16 )
	GFXDECODE_ENTRY( "tiles",   0, gfx_8x8x4_packed_msb, 16*16, 16 )
GFXDECODE_END

/***************************************************************************

    Machine Driver

***************************************************************************/

void pandoras_state::machine_start()
{
	save_item(NAME(m_firq_old_data_a));
	save_item(NAME(m_firq_old_data_b));
	save_item(NAME(m_irq_enable_a));
	save_item(NAME(m_irq_enable_b));
	save_item(NAME(m_i8039_status));
}

void pandoras_state::machine_reset()
{
	m_firq_old_data_a = 0;
	m_firq_old_data_b = 0;
	m_i8039_status = 0;
}

uint8_t pandoras_state::porta_r()
{
	return m_i8039_status;
}

uint8_t pandoras_state::portb_r()
{
	return (m_audiocpu->total_cycles() / 512) & 0x0f;
}

void pandoras_state::pandoras(machine_config &config)
{
	static constexpr XTAL MASTER_CLOCK = XTAL(18'432'000);
	static constexpr XTAL SOUND_CLOCK = XTAL(14'318'181);

	// basic machine hardware
	MC6809E(config, m_maincpu, MASTER_CLOCK / 6);  // CPU A
	m_maincpu->set_addrmap(AS_PROGRAM, &pandoras_state::master_map);

	MC6809E(config, m_subcpu, MASTER_CLOCK / 6);      // CPU B
	m_subcpu->set_addrmap(AS_PROGRAM, &pandoras_state::slave_map);

	Z80(config, m_audiocpu, SOUND_CLOCK / 8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &pandoras_state::sound_map);

	I8039(config, m_mcu, SOUND_CLOCK / 2);
	m_mcu->set_addrmap(AS_PROGRAM, &pandoras_state::i8039_map);
	m_mcu->set_addrmap(AS_IO, &pandoras_state::i8039_io_map);
	m_mcu->p1_out_cb().set("dac", FUNC(dac_byte_interface::data_w));
	m_mcu->p2_out_cb().set(FUNC(pandoras_state::i8039_irqen_and_status_w));

	config.set_maximum_quantum(attotime::from_hz(6000));  // 100 CPU slices per frame - needed for correct synchronization of the sound CPUs

	ls259_device &mainlatch(LS259(config, "mainlatch")); // C3
	mainlatch.q_out_cb<0>().set(FUNC(pandoras_state::cpua_irq_enable_w)); // ENA
	mainlatch.q_out_cb<1>().set_nop(); // OFSET - unknown
	mainlatch.q_out_cb<2>().set(FUNC(pandoras_state::coin_counter_w<0>));
	mainlatch.q_out_cb<3>().set(FUNC(pandoras_state::coin_counter_w<1>));
	mainlatch.q_out_cb<5>().set(FUNC(pandoras_state::flip_screen_set)); // FLIP
	mainlatch.q_out_cb<6>().set(FUNC(pandoras_state::cpub_irq_enable_w)); // ENB
	mainlatch.q_out_cb<7>().set_inputline(m_subcpu, INPUT_LINE_RESET).invert(); // RESETB

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(pandoras_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(pandoras_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pandoras);
	PALETTE(config, m_palette, FUNC(pandoras_state::palette), 16*16+16*16, 32);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, "soundlatch");
	GENERIC_LATCH_8(config, "soundlatch2");

	ay8910_device &aysnd(AY8910(config, "aysnd", SOUND_CLOCK / 8));
	aysnd.port_a_read_callback().set(FUNC(pandoras_state::porta_r));   // not used
	aysnd.port_b_read_callback().set(FUNC(pandoras_state::portb_r));
	aysnd.add_route(ALL_OUTPUTS, "speaker", 0.4);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.12); // unknown DAC
}


/***************************************************************************

  Game ROMs

***************************************************************************/


/*
A PCB picture shows the following label format:

PANDORA'S PAL
A1   J13

Visible ROM labels on the GX328 main board PWB(A)20001109B PCB:
PANDORA'S PAL   A1   13J
PANDORA'S PAL   A1   12J
PANDORA'S PAL   A1   10J
PANDORA'S PAL   A1   9J
  -- J14 is an empty socket --
PANDORA'S PAL   A1   17J
PANDORA'S PAL   A1   18J
PANDORA'S PAL   A1   19J

PANDORA'S PAL   A1   18A
PANDORA'S PAL   A1   19A

PANDORA'S PAL   A1   5J

BPROM at 16A stamped 328F14
BPROM at 17G stamped 328F15
BPROM at A2 not visible in picture

ROMs labels on the GX328 sound board PWB(B)3000154A PCB:
PANDORA'S PAL   A1     6C
PANDORA'S PAL   A1     7E


PCB stickered:

Pandora's Palace
KOSUKA
(c) Konami 1984

*/
ROM_START( pandoras )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pand_j13.cpu",   0x08000, 0x02000, CRC(7a0fe9c5) SHA1(e68c8d76d1abb69ac72b0e2cd8c1dfc540064ee3) )
	ROM_LOAD( "pand_j12.cpu",   0x0a000, 0x02000, CRC(7dc4bfe1) SHA1(359c3051e5d7a34d0e49578e4c168fd19c73e202) )
	ROM_LOAD( "pand_j10.cpu",   0x0c000, 0x02000, CRC(be3af3b7) SHA1(91321b53e17e58b674104cb95b1c35ee8fecae22) )
	ROM_LOAD( "pand_j9.cpu",    0x0e000, 0x02000, CRC(e674a17a) SHA1(a4b096dc455425dd60298acf2203659ef6f8d857) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "pand_j5.cpu",    0x0e000, 0x02000, CRC(4aab190b) SHA1(d2204953d6b6b34cea851bfc9c2b31426e75f90b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "pand_6c.snd",    0x00000, 0x02000, CRC(0c1f109d) SHA1(4e6cdee99261764bd2fea5abbd49d800baba0dc5) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "pand_7e.snd",    0x00000, 0x02000, CRC(1071c1ba) SHA1(3693be69f4b32fb3031bcdee8cac0d46ec8c2804) )

	ROM_REGION( 0x6000, "sprites", 0 )
	ROM_LOAD( "pand_j18.cpu",   0x00000, 0x02000, CRC(99a696c5) SHA1(35a27cd5ecc51a9a1acf01eb8078a1028f03be32) )
	ROM_LOAD( "pand_j17.cpu",   0x02000, 0x02000, CRC(38a03c21) SHA1(b0c8f642787bab3cd1d76657e56f07f4f6f9073c) )
	ROM_LOAD( "pand_j16.cpu",   0x04000, 0x02000, CRC(e0708a78) SHA1(9dbd08b6ca8a66a61e128d1806888696273de848) )

	ROM_REGION( 0x4000, "tiles", 0 )
	ROM_LOAD( "pand_a18.cpu",   0x00000, 0x02000, CRC(23706d4a) SHA1(cca92e6ff90e3006a79a214f1211fd659771de53) )
	ROM_LOAD( "pand_a19.cpu",   0x02000, 0x02000, CRC(a463b3f9) SHA1(549b7ee6e47325b80186441da11879fb8b1b47be) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pandora.2a",     0x0000, 0x020, CRC(4d56f939) SHA1(a8dac604bfdaf4b153b75dbf165de113152b6daa) ) // palette
	ROM_LOAD( "pandora.17g",    0x0020, 0x100, CRC(c1a90cfc) SHA1(c6581f2d543e38f1de399774183cf0698e61dab5) ) // sprite lookup table
	ROM_LOAD( "pandora.16b",    0x0120, 0x100, CRC(c89af0c3) SHA1(4072c8d61521b34ce4dbce1d48f546402e9539cd) ) // character lookup table
ROM_END

} // anonymous namespace


GAME( 1984, pandoras, 0, pandoras, pandoras, pandoras_state, empty_init, ROT90, "Konami / Interlogic", "Pandora's Palace", MACHINE_SUPPORTS_SAVE )
