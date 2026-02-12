// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni, Couriersud
/***************************************************************************

Mario Bros driver by Mirko Buffoni

US revision letter is from the ROM labels, but chronology does not appear
to match letter ordering according to hex compare.

F: Looks like the oldest revision of the three.

F->G: 2 bytes difference (at $30A4 and $30E9).

G->E: Adds patches to $F710 area (jumps to there from $31DA and $324C), and
an additional 2 bytes different. A Nintendo service bulletin mentions they
updated the program due to coin detection problems, the update shortens the
required duration the coin switch needs to be active. And indeed, if you try
PORT_IMPULSE(1), it works fine on revision E, but not on the other revisions.

Revision E also fixes glitches in gfx2 that look like a production error
(not a bad dump): a gap in Mario/Luigi's hat when he jumps, and a wrong eye
color in one of the walk animations that's visible on Luigi.

The Japan revision C is from around the same time as US revision E.

The sound MCU can be easily replaced with a ROMless one such as I8039
(or just force EA high), by doing a 1-byte patch to the external ROM:
offset $01: change $00 to $01 (call $100 -> call $101)

TODO:
- coin 2 doesn't work in marioe and marioj (though it works in service mode),
  is it a bug or deliberate?
- draw_sprites should adopt the scanline logic from dkong, the schematics have
  the same logic for sprite buffering
- a lot of soundlatch warnings in error.log, it's probably harmless

BTANB:
- erratic line at top when scrolling down "Mario Bros" title is confirmed
  as being present on real PCB as well

============================================================================

Memory map (preliminary):

0000-5fff ROM
6000-6fff RAM
7000-73ff Sprite RAM
7400-77ff Video RAM
f000-ffff ROM

read:
7c00      IN0
7c80      IN1
7f80      DSW

*
 * IN0 (bits NOT inverted)
 * bit 7 : TEST
 * bit 6 : START 2
 * bit 5 : START 1
 * bit 4 : JUMP player 1
 * bit 3 : ? DOWN player 1 ?
 * bit 2 : ? UP player 1 ?
 * bit 1 : LEFT player 1
 * bit 0 : RIGHT player 1
 *
*
 * IN1 (bits NOT inverted)
 * bit 7 : ?
 * bit 6 : COIN 2
 * bit 5 : COIN 1
 * bit 4 : JUMP player 2
 * bit 3 : ? DOWN player 2 ?
 * bit 2 : ? UP player 2 ?
 * bit 1 : LEFT player 2
 * bit 0 : RIGHT player 2
 *
*
 * DSW (bits NOT inverted)
 * bit 7 : \ difficulty
 * bit 6 : / 00 = easy  01 = medium  10 = hard  11 = hardest
 * bit 5 : \ bonus
 * bit 4 : / 00 = 20000  01 = 30000  10 = 40000  11 = none
 * bit 3 : \ coins per play
 * bit 2 : /
 * bit 1 : \ 00 = 3 lives  01 = 4 lives
 * bit 0 : / 10 = 5 lives  11 = 6 lives
 *

write:
7d00      vertical scroll (pow)
7d80      ?
7e00      sound
7e80-7e87 misc. triggers (see mcfg)
7f00-7f07 sound triggers

I/O ports

write:
00        Z80 DMA

Video generation is like dkong/dkongjr. However, pixel clock is 24MHZ.
7C -> 100 => 256 - 124 = 132 ==> 264 Scanlines

***************************************************************************/

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/z80dma.h"
#include "sound/ay8910.h"

#include "machine/netlist.h"
#include "netlist/devices/net_lib.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"
#include "video/resnet.h"

#include "nl_mario.h"


namespace {

class mario_state : public driver_device
{
public:
	mario_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_z80dma(*this, "z80dma"),
		m_soundlatch(*this, "soundlatch%u", 0),
		m_audio_snd0(*this, "snd_nl:snd0"),
		m_audio_snd1(*this, "snd_nl:snd1"),
		m_audio_snd7(*this, "snd_nl:snd7"),
		m_audio_dac(*this, "snd_nl:dac"),
		m_soundrom(*this, "soundrom"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram")
	{ }

	void mario_base(machine_config &config);
	void masao(machine_config &config);
	void mario(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(adjust_palette) { set_palette(newval); }

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void sound_start() override ATTR_COLD;
	virtual void sound_reset() override ATTR_COLD;

private:
	// devices
	required_device<z80_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<z80dma_device> m_z80dma;
	optional_device_array<generic_latch_8_device, 4> m_soundlatch;
	optional_device<netlist_mame_logic_input_device> m_audio_snd0;
	optional_device<netlist_mame_logic_input_device> m_audio_snd1;
	optional_device<netlist_mame_logic_input_device> m_audio_snd7;
	optional_device<netlist_mame_int_input_device> m_audio_dac;

	// memory pointers
	optional_region_ptr<uint8_t> m_soundrom;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;

	// video state
	uint8_t m_gfx_bank = 0;
	uint8_t m_palette_bank = 0;
	tilemap_t *m_bg_tilemap = nullptr;

	// misc
	uint8_t m_irq_clock = 0;
	bool m_nmi_mask = false;

	// handlers
	uint8_t mario_sh_tune_r(offs_t offset);
	void mario_sh_sound_w(uint8_t data);
	void masao_sh_irqtrigger_w(uint8_t data);
	void mario_sh1_w(uint8_t data) { m_audio_snd0->write(data); }
	void mario_sh2_w(uint8_t data) { m_audio_snd1->write(data); }
	void mario_sh3_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void set_palette(int monitor);
	void palette(palette_device &palette);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void videoram_w(offs_t offset, uint8_t data);
	void gfx_bank_w(int state);
	void palette_bank_w(int state);
	void scroll_w(uint8_t data);
	void nmi_mask_w(int state);
	void coin_counter_1_w(int state);
	void coin_counter_2_w(int state);
	void vblank_irq(int state);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);

	void base_map(address_map &map) ATTR_COLD;
	void mario_map(address_map &map) ATTR_COLD;
	void masao_map(address_map &map) ATTR_COLD;
	void mario_io_map(address_map &map) ATTR_COLD;

	void mario_sound_map(address_map &map) ATTR_COLD;
	void mario_sound_io_map(address_map &map) ATTR_COLD;
	void masao_sound_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Sound initialization
 *
 *************************************/

void mario_state::sound_start()
{
	if (m_audiocpu->type() != Z80)
	{
		uint8_t *SND = memregion("audiocpu")->base();

		// Hack to bootstrap MCU program into external MB1
		SND[0x0000] = 0xf5;
		SND[0x0001] = 0x04;
		SND[0x0002] = 0x00;
	}

	save_item(NAME(m_irq_clock));
}

void mario_state::sound_reset()
{
	m_soundlatch[0]->clear_w();
	if (m_soundlatch[1]) m_soundlatch[1]->clear_w();
	if (m_soundlatch[2]) m_soundlatch[2]->clear_w();
	if (m_soundlatch[3]) m_soundlatch[3]->clear_w();

	m_irq_clock = 0;
}


/*************************************
 *
 *  Sound I/O handlers
 *
 *************************************/

uint8_t mario_state::mario_sh_tune_r(offs_t offset)
{
	uint8_t p2 = m_soundlatch[2]->read();

	if (BIT(p2, 7))
		return m_soundlatch[0]->read();
	else
		return m_soundrom[(p2 & 0x0f) << 8 | offset];
}

void mario_state::mario_sh_sound_w(uint8_t data)
{
	m_audio_dac->write(data);
}

void mario_state::masao_sh_irqtrigger_w(uint8_t data)
{
	data &= 1;

	// setting bit 0 high then low triggers IRQ on the sound CPU
	if (m_irq_clock && !data)
		m_audiocpu->set_input_line(0, HOLD_LINE);

	m_irq_clock = data;
}


void mario_state::mario_sh3_w(offs_t offset, uint8_t data)
{
	data &= 1;

	switch (offset)
	{
		// death
		case 0:
			m_audiocpu->set_input_line(0, data ? ASSERT_LINE : CLEAR_LINE);
			break;

		// get coin, ice
		case 1: case 2:
		{
			const uint8_t mask = 1 << (offset - 1);
			m_soundlatch[3]->write((m_soundlatch[3]->read() & ~mask) | (data ? mask : 0));
			break;
		}

		// crab, turtle, fly, coin
		case 3: case 4: case 5: case 6:
		{
			const uint8_t mask = 1 << (offset - 3);
			m_soundlatch[1]->write((m_soundlatch[1]->read() & ~mask) | (data ? mask : 0));
			break;
		}

		// skid
		case 7:
			m_audio_snd7->write(data ^ 1);
			break;
	}
}


/*************************************
 *
 *  Video initialization
 *
 *************************************/

TILE_GET_INFO_MEMBER(mario_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index] + 256 * m_gfx_bank;
	int color = 8 + (m_videoram[tile_index] >> 5) + 16 * m_palette_bank;

	tileinfo.set(0, code, color, 0);
}

void mario_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mario_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);

	m_gfxdecode->gfx(0)->set_granularity(8);

	save_item(NAME(m_gfx_bank));
	save_item(NAME(m_palette_bank));
}

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mario Bros. has a 512x8 palette PROM; interstingly, bytes 0-255 contain an
  inverted palette, as other Nintendo games like Donkey Kong, while bytes
  256-511 contain a non inverted palette. This was probably done to allow
  connection to both the special Nintendo and a standard monitor.
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor -- inverter  -- RED
        -- 470 ohm resistor -- inverter  -- RED
        -- 1  kohm resistor -- inverter  -- RED
        -- 220 ohm resistor -- inverter  -- GREEN
        -- 470 ohm resistor -- inverter  -- GREEN
        -- 1  kohm resistor -- inverter  -- GREEN
        -- 220 ohm resistor -- inverter  -- BLUE
  bit 0 -- 470 ohm resistor -- inverter  -- BLUE

***************************************************************************/

static const res_net_decode_info mario_decode_info =
{
	1,      // there may be two proms needed to construct color
	0,      // start at 0
	255,    // end at 255
	//  R,   G,   B
	{   0,   0,   0},       // offsets
	{   5,   2,   0},       // shifts
	{0x07,0x07,0x03}        // masks
};

static const res_net_info mario_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_MB7052 | RES_NET_MONITOR_SANYO_EZV20,
	{
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_EMITTER,    680, 0, 2, {  470, 220,   0 } }  // dkong
	}
};

static const res_net_info mario_net_info_std =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_MB7052,
	{
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_DARLINGTON, 470, 0, 3, { 1000, 470, 220 } },
		{ RES_NET_AMP_EMITTER,    680, 0, 2, {  470, 220,   0 } }  // dkong
	}
};


void mario_state::set_palette(int monitor)
{
	uint8_t const *const color_prom = memregion("proms")->base();

	std::vector<rgb_t> rgb;
	if (monitor == 0)
		compute_res_net_all(rgb, color_prom, mario_decode_info, mario_net_info);
	else
		compute_res_net_all(rgb, color_prom + 256, mario_decode_info, mario_net_info_std);

	m_palette->set_pen_colors(0, rgb);
	m_palette->palette()->normalize_range(0, 255);
}

void mario_state::palette(palette_device &palette)
{
	set_palette(0);
}


/*************************************
 *
 *  Video I/O handlers
 *
 *************************************/

void mario_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void mario_state::gfx_bank_w(int state)
{
	m_gfx_bank = state;
	machine().tilemap().mark_all_dirty();
}

void mario_state::palette_bank_w(int state)
{
	m_palette_bank = state;
	machine().tilemap().mark_all_dirty();
}

void mario_state::scroll_w(uint8_t data)
{
	m_bg_tilemap->set_scrolly(0, data + 17);
}


/*************************************
 *
 *  Screen update
 *
 *************************************/

void mario_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const bool flip = flip_screen();
	int offs = 0;

	while (offs != m_spriteram.bytes())
	{
		if (m_spriteram[offs])
		{
			// from schematics ....
			int y = (m_spriteram[offs + 0] + (flip ? 0xf7 : 0xf9) + 1) & 0xff;
			int x = m_spriteram[offs + 3];
			// sprite will be drawn if (y + scanline) & 0xF0 = 0xF0
			y = 240 - y; // logical screen position

			y = y ^ (flip ? 0xff : 0x00); // physical screen location
			x = x ^ (flip ? 0xff : 0x00); // physical screen location

			int code = m_spriteram[offs + 2];
			int color = (m_spriteram[offs + 1] & 0x0f) + 16 * m_palette_bank;
			int flipx = (m_spriteram[offs + 1] & 0x80);
			int flipy = (m_spriteram[offs + 1] & 0x40);

			if (flip)
			{
				y -= 14;
				x -= 7;
			}
			else
			{
				y += 1;
				x -= 8;
			}

			if (flip)
			{
				m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
					code,
					color,
					!flipx, !flipy,
					x, y, 0);
			}
			else
			{
				m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
					code,
					color,
					flipx, flipy,
					x, y, 0);
			}
		}

		offs += 4;
	}
}

uint32_t mario_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);

	return 0;
}


/*************************************
 *
 *  Misc I/O handlers
 *
 *************************************/

uint8_t mario_state::memory_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

void mario_state::memory_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}

void mario_state::nmi_mask_w(int state)
{
	m_nmi_mask = state;
	if (!state)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void mario_state::coin_counter_1_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

void mario_state::coin_counter_2_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

void mario_state::vblank_irq(int state)
{
	if (state && m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void mario_state::base_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x67ff).ram();
	map(0x6800, 0x6fff).ram().share("nvram");
	map(0x7000, 0x73ff).ram().share("spriteram");
	map(0x7400, 0x77ff).ram().w(FUNC(mario_state::videoram_w)).share("videoram");
	map(0x7c00, 0x7c00).portr("IN0");
	map(0x7c80, 0x7c80).portr("IN1");
	map(0x7d00, 0x7d00).w(FUNC(mario_state::scroll_w));
	map(0x7e00, 0x7e00).w(m_soundlatch[0], FUNC(generic_latch_8_device::write));
	map(0x7e80, 0x7e87).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x7f80, 0x7f80).portr("DSW");
	map(0xf000, 0xffff).rom();
}

void mario_state::mario_map(address_map &map)
{
	base_map(map);
	map(0x7c00, 0x7c00).w(FUNC(mario_state::mario_sh1_w)); // Mario run sample
	map(0x7c80, 0x7c80).w(FUNC(mario_state::mario_sh2_w)); // Luigi run sample
	map(0x7f00, 0x7f07).w(FUNC(mario_state::mario_sh3_w)); // misc samples
}

void mario_state::masao_map(address_map &map)
{
	base_map(map);
	map(0x7f00, 0x7f00).w(FUNC(mario_state::masao_sh_irqtrigger_w));
}

void mario_state::mario_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(m_z80dma, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
}

/************************************/

void mario_state::mario_sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region(m_soundrom, 0);
}

void mario_state::mario_sound_io_map(address_map &map)
{
	map(0x00, 0xff).r(FUNC(mario_state::mario_sh_tune_r)).w(FUNC(mario_state::mario_sh_sound_w));
}

void mario_state::masao_sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x2000, 0x23ff).ram();
	map(0x4000, 0x4000).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x6000, 0x6000).w("aysnd", FUNC(ay8910_device::address_w));
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( mario )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "20k only" )
	PORT_DIPSETTING(    0x10, "30k only" )
	PORT_DIPSETTING(    0x20, "40k only" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )

	PORT_START("MONITOR")
	PORT_CONFNAME( 0x01, 0x00, "Monitor" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(mario_state::adjust_palette), 0)
	PORT_CONFSETTING(    0x00, "Nintendo" )
	PORT_CONFSETTING(    0x01, "Std 15.72Khz" )
INPUT_PORTS_END

static INPUT_PORTS_START( mariof )
	PORT_INCLUDE( mario )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "20k 40k 20k+" )
	PORT_DIPSETTING(    0x10, "30k 50k 20k+" )
	PORT_DIPSETTING(    0x20, "40k 60k 20k+" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( marioj )
	PORT_INCLUDE( mario )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!3,!4,!5")
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x20, "2 Players Game" )        PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPSETTING(    0x20, "2 Credits" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, "20k 50k 30k+" )
	PORT_DIPSETTING(    0x40, "30k 60k 30k+" )
	PORT_DIPSETTING(    0x80, "40k 70k 30k+" )
	PORT_DIPSETTING(    0xc0, DEF_STR( None ) )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	2,  /* 2 bits per pixel */
	{ 512*8*8, 0 }, /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, /* pretty straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};


static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	256,    /* 256 sprites */
	3,  /* 3 bits per pixel */
	{ 2*256*16*16, 256*16*16, 0 },  /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,       /* the two halves of the sprite are separated */
			256*16*8+0, 256*16*8+1, 256*16*8+2, 256*16*8+3, 256*16*8+4, 256*16*8+5, 256*16*8+6, 256*16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8    /* every sprite takes 16 consecutive bytes */
};

static GFXDECODE_START( gfx_mario )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 32 )
GFXDECODE_END


/*************************************
 *
 *  Machine config
 *
 *************************************/

void mario_state::mario_base(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8_MHz_XTAL / 2); // verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &mario_state::mario_map);
	m_maincpu->set_addrmap(AS_IO, &mario_state::mario_io_map);
	m_maincpu->busack_cb().set(m_z80dma, FUNC(z80dma_device::bai_w));

	// devices
	Z80DMA(config, m_z80dma, 8_MHz_XTAL / 2);
	m_z80dma->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSREQ);
	m_z80dma->in_mreq_callback().set(FUNC(mario_state::memory_read_byte));
	m_z80dma->out_mreq_callback().set(FUNC(mario_state::memory_write_byte));

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 2L (7E80H)
	mainlatch.q_out_cb<0>().set(FUNC(mario_state::gfx_bank_w));         // ~T ROM
	mainlatch.q_out_cb<1>().set_nop();                                  // 2 PSL
	mainlatch.q_out_cb<2>().set(FUNC(mario_state::flip_screen_set));    // FLIP
	mainlatch.q_out_cb<3>().set(FUNC(mario_state::palette_bank_w));     // CREF 0
	mainlatch.q_out_cb<4>().set(FUNC(mario_state::nmi_mask_w));         // NMI EI
	mainlatch.q_out_cb<5>().set("z80dma", FUNC(z80dma_device::rdy_w));  // DMA SET
	mainlatch.q_out_cb<6>().set(FUNC(mario_state::coin_counter_2_w));   // COUNTER 2
	mainlatch.q_out_cb<7>().set(FUNC(mario_state::coin_counter_1_w));   // COUNTER 1

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(24_MHz_XTAL / 4, 384, 0, 256, 264, 16, 240);
	screen.set_screen_update(FUNC(mario_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(mario_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mario);
	PALETTE(config, m_palette, FUNC(mario_state::palette), 256);
}

void mario_state::mario(machine_config &config)
{
	mario_base(config);

	// sound hardware
	m58715_device &audiocpu(M58715(config, m_audiocpu, 11_MHz_XTAL));
	audiocpu.set_addrmap(AS_PROGRAM, &mario_state::mario_sound_map);
	audiocpu.set_addrmap(AS_IO, &mario_state::mario_sound_io_map);
	audiocpu.p1_in_cb().set(m_soundlatch[1], FUNC(generic_latch_8_device::read));
	audiocpu.p1_out_cb().set(m_soundlatch[1], FUNC(generic_latch_8_device::write));
	audiocpu.p2_in_cb().set(m_soundlatch[2], FUNC(generic_latch_8_device::read)).mask(0xef); // bit 4 is GND!
	audiocpu.p2_out_cb().set(m_soundlatch[2], FUNC(generic_latch_8_device::write));
	audiocpu.p2_out_cb().append_inputline(m_audiocpu, MCS48_INPUT_EA).bit(5).invert();
	audiocpu.t0_in_cb().set(m_soundlatch[3], FUNC(generic_latch_8_device::read)).bit(0);
	audiocpu.t1_in_cb().set(m_soundlatch[3], FUNC(generic_latch_8_device::read)).bit(1);

	SPEAKER(config, "mono").front_center();

	for (int i = 0; i < 4; i++)
		GENERIC_LATCH_8(config, m_soundlatch[i]);

	NETLIST_SOUND(config, "snd_nl", 48000)
		.set_source(netlist_mario)
		.add_route(ALL_OUTPUTS, "mono", 0.5);

	NETLIST_LOGIC_INPUT(config, m_audio_snd0, "SOUND0.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_audio_snd1, "SOUND1.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_audio_snd7, "SOUND7.IN", 0);
	NETLIST_INT_INPUT(config, m_audio_dac, "DAC.VAL", 0, 255);

	NETLIST_STREAM_OUTPUT(config, "snd_nl:cout0", 0, "ROUT.1").set_mult_offset(150000.0 / 32768.0, 0.0);
}

void mario_state::masao(machine_config &config)
{
	mario_base(config);

	m_maincpu->set_clock(4'000'000); // 4MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &mario_state::masao_map);

	// sound hardware
	Z80(config, m_audiocpu, 14'318'181 / 8); // 1.79MHz?
	m_audiocpu->set_addrmap(AS_PROGRAM, &mario_state::masao_sound_map);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);

	ay8910_device &aysnd(AY8910(config, "aysnd", 14'318'181 / 8)); // 1.79MHz?
	aysnd.port_a_read_callback().set(m_soundlatch[0], FUNC(generic_latch_8_device::read));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( mario )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tma1-c-7f_g.7f", 0x0000, 0x2000, CRC(c0c6e014) SHA1(36a04f9ca1c2a583477cb8a6f2ef94e044e08296) )
	ROM_LOAD( "tma1-c-7e_g.7e", 0x2000, 0x2000, CRC(116b3856) SHA1(e372f846d0e5a2b9b47ebd0330293fcc8a12363f) )
	ROM_LOAD( "tma1-c-7d_g.7d", 0x4000, 0x2000, CRC(dcceb6c1) SHA1(b19804e69ce2c98cf276c6055c3a250316b96b45) )
	ROM_LOAD( "tma1-c-7c_g.7c", 0xf000, 0x1000, CRC(4a63d96b) SHA1(b09060b2c84ab77cc540a27b8f932cb60ec8d442) )

	ROM_REGION( 0x0800, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "m58715-051p.5l", 0x0000, 0x0800, NO_DUMP ) // internal rom

	ROM_REGION( 0x1000, "soundrom", 0 )
	ROM_LOAD( "tma1-c-6k_e.6k", 0x0000, 0x1000, CRC(06b9ff85) SHA1(111a29bcb9cda0d935675fa26eca6b099a88427f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tma1-v-3f.3f",   0x0000, 0x1000, CRC(28b0c42c) SHA1(46749568aff88a28c3b6a1ac423abd1b90742a4d) )
	ROM_LOAD( "tma1-v-3j.3j",   0x1000, 0x1000, CRC(0c8cc04d) SHA1(15fae47d701dc1ef15c943cee6aa991776ecffdf) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "tma1-v-7m.7m",   0x0000, 0x1000, CRC(22b7372e) SHA1(4a1c1e239cb6d483e76f50d7a3b941025963c6a3) )
	ROM_LOAD( "tma1-v-7n.7n",   0x1000, 0x1000, CRC(4f3a1f47) SHA1(0747d693b9482f6dd28b0bc484fd1d3e29d35654) )
	ROM_LOAD( "tma1-v-7p.7p",   0x2000, 0x1000, CRC(56be6ccd) SHA1(15a6e16c189d45f72761ebcbe9db5001bdecd659) )
	ROM_LOAD( "tma1-v-7s.7s",   0x3000, 0x1000, CRC(56f1d613) SHA1(9af6844dbaa3615433d0595e9e85e72493e31a54) )
	ROM_LOAD( "tma1-v-7t.7t",   0x4000, 0x1000, CRC(641f0008) SHA1(589fe108c7c11278fd897f2ded8f0498bc149cfd) )
	ROM_LOAD( "tma1-v-7u.7u",   0x5000, 0x1000, CRC(7baf5309) SHA1(d9194ff7b89a18273d37b47228fc7fb7e2a0ed1f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tma1-c-4p.4p",   0x0000, 0x0200, CRC(afc9bd41) SHA1(90b739c4c7f24a88b6ac5ca29b06c032906a2801) )

	ROM_REGION( 0x0020, "decoder_prom", 0 ) // main cpu memory map decoding prom
	ROM_LOAD( "tma1-c-5b.5b",   0x0000, 0x0020, CRC(58d86098) SHA1(d654995004b9052b12d3b682a2b39530e70030fc) )
ROM_END

ROM_START( mariof )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tma1-c-7f_f.7f", 0x0000, 0x2000, CRC(c0c6e014) SHA1(36a04f9ca1c2a583477cb8a6f2ef94e044e08296) )
	ROM_LOAD( "tma1-c-7e_f.7e", 0x2000, 0x2000, CRC(94fb60d6) SHA1(e74d74aa27f87a164bdd453ab0076efeeb7d4ea3) )
	ROM_LOAD( "tma1-c-7d_f.7d", 0x4000, 0x2000, CRC(dcceb6c1) SHA1(b19804e69ce2c98cf276c6055c3a250316b96b45) )
	ROM_LOAD( "tma1-c-7c_f.7c", 0xf000, 0x1000, CRC(4a63d96b) SHA1(b09060b2c84ab77cc540a27b8f932cb60ec8d442) )

	ROM_REGION( 0x0800, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "m58715-051p.5l", 0x0000, 0x0800, NO_DUMP ) // internal rom

	ROM_REGION( 0x1000, "soundrom", 0 )
	ROM_LOAD( "tma1-c-6k_e.6k", 0x0000, 0x1000, CRC(06b9ff85) SHA1(111a29bcb9cda0d935675fa26eca6b099a88427f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tma1-v-3f.3f",   0x0000, 0x1000, CRC(28b0c42c) SHA1(46749568aff88a28c3b6a1ac423abd1b90742a4d) )
	ROM_LOAD( "tma1-v-3j.3j",   0x1000, 0x1000, CRC(0c8cc04d) SHA1(15fae47d701dc1ef15c943cee6aa991776ecffdf) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "tma1-v-7m.7m",   0x0000, 0x1000, CRC(22b7372e) SHA1(4a1c1e239cb6d483e76f50d7a3b941025963c6a3) )
	ROM_LOAD( "tma1-v-7n.7n",   0x1000, 0x1000, CRC(4f3a1f47) SHA1(0747d693b9482f6dd28b0bc484fd1d3e29d35654) )
	ROM_LOAD( "tma1-v-7p.7p",   0x2000, 0x1000, CRC(56be6ccd) SHA1(15a6e16c189d45f72761ebcbe9db5001bdecd659) )
	ROM_LOAD( "tma1-v-7s.7s",   0x3000, 0x1000, CRC(56f1d613) SHA1(9af6844dbaa3615433d0595e9e85e72493e31a54) )
	ROM_LOAD( "tma1-v-7t.7t",   0x4000, 0x1000, CRC(641f0008) SHA1(589fe108c7c11278fd897f2ded8f0498bc149cfd) )
	ROM_LOAD( "tma1-v-7u.7u",   0x5000, 0x1000, CRC(7baf5309) SHA1(d9194ff7b89a18273d37b47228fc7fb7e2a0ed1f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tma1-c-4p_1.4p", 0x0000, 0x0200, CRC(8187d286) SHA1(8a6d8e622599f1aacaeb10f7b1a39a23c8a840a0) ) // BPROM was a MB7124E read as 82S147

	ROM_REGION( 0x0020, "decoder_prom", 0 ) // main cpu memory map decoding prom
	ROM_LOAD( "tma1-c-5b.5b",   0x0000, 0x0020, CRC(58d86098) SHA1(d654995004b9052b12d3b682a2b39530e70030fc) ) // BPROM was a TBP18S030N read as 82S123
ROM_END

ROM_START( marioe ) // TMA1-05 CPU, TMA1-03-VIDEO
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tma1-c-7f_e-1.7f", 0x0000, 0x2000, CRC(c0c6e014) SHA1(36a04f9ca1c2a583477cb8a6f2ef94e044e08296) )
	ROM_LOAD( "tma1-c-7e_e-3.7e", 0x2000, 0x2000, CRC(b09ab857) SHA1(35b91cd1c4c3dd2d543a1ea8ff7b951715727792) )
	ROM_LOAD( "tma1-c-7d_e-1.7d", 0x4000, 0x2000, CRC(dcceb6c1) SHA1(b19804e69ce2c98cf276c6055c3a250316b96b45) )
	ROM_LOAD( "tma1-c-7c_e-3.7c", 0xf000, 0x1000, CRC(0d31bd1c) SHA1(a2e238470ba2ea3c81225fec687f61f047c68c59) )

	ROM_REGION( 0x0800, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "m58715-051p.5l",   0x0000, 0x0800, NO_DUMP ) // internal rom

	ROM_REGION( 0x1000, "soundrom", 0 )
	ROM_LOAD( "tma1-c-6k_e.6k",   0x0000, 0x1000, CRC(06b9ff85) SHA1(111a29bcb9cda0d935675fa26eca6b099a88427f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tma1-v-3f.3f",     0x0000, 0x1000, CRC(28b0c42c) SHA1(46749568aff88a28c3b6a1ac423abd1b90742a4d) )
	ROM_LOAD( "tma1-v-3j.3j",     0x1000, 0x1000, CRC(0c8cc04d) SHA1(15fae47d701dc1ef15c943cee6aa991776ecffdf) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "tma1-v.7m",        0x0000, 0x1000, CRC(d01c0e2c) SHA1(cd6cc9d69c36db15543601f5da2abf109cde43c9) )
	ROM_LOAD( "tma1-v-7n.7n",     0x1000, 0x1000, CRC(4f3a1f47) SHA1(0747d693b9482f6dd28b0bc484fd1d3e29d35654) )
	ROM_LOAD( "tma1-v-7p.7p",     0x2000, 0x1000, CRC(56be6ccd) SHA1(15a6e16c189d45f72761ebcbe9db5001bdecd659) )
	ROM_LOAD( "tma1-v.7s",        0x3000, 0x1000, CRC(ff856e6f) SHA1(2bc9ff18bb1842e8de2bc61ac828f1b175290bed) )
	ROM_LOAD( "tma1-v-7t.7t",     0x4000, 0x1000, CRC(641f0008) SHA1(589fe108c7c11278fd897f2ded8f0498bc149cfd) )
	ROM_LOAD( "tma1-v.7u",        0x5000, 0x1000, CRC(d2dbeb75) SHA1(676cf3e15252cd0d9e926ca15c3aa0caa39be269) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tma1-c-4p_1.4p",   0x0000, 0x0200, CRC(8187d286) SHA1(8a6d8e622599f1aacaeb10f7b1a39a23c8a840a0) )

	ROM_REGION( 0x0020, "decoder_prom", 0 ) // main cpu memory map decoding prom
	ROM_LOAD( "tma1-c-5b.5b",     0x0000, 0x0020, CRC(58d86098) SHA1(d654995004b9052b12d3b682a2b39530e70030fc) )
ROM_END

ROM_START( marioj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tma1-c-a1.7f",   0x0000, 0x2000, CRC(b64b6330) SHA1(f7084251ac325bbfa3fb804da16a50622e1fd213) )
	ROM_LOAD( "tma1-c-a2.7e",   0x2000, 0x2000, CRC(290c4977) SHA1(5af266be0ddc883c6548c90e4a9084024a1e91a0) )
	ROM_LOAD( "tma1-c-a1.7d",   0x4000, 0x2000, CRC(f8575f31) SHA1(710d0e72fcfce700ed2a22fb9c7c392cc76b250b) )
	ROM_LOAD( "tma1-c-a2.7c",   0xf000, 0x1000, CRC(a3c11e9e) SHA1(d0612b0f8c2ea4e798f551922a04a324f4ed5f3d) )

	ROM_REGION( 0x0800, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "m58715-051p.5l", 0x0000, 0x0800, NO_DUMP ) // internal rom

	ROM_REGION( 0x1000, "soundrom", 0 )
	ROM_LOAD( "tma1c-a.6k",     0x0000, 0x1000, CRC(06b9ff85) SHA1(111a29bcb9cda0d935675fa26eca6b099a88427f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tma1-v-a.3f",    0x0000, 0x1000, CRC(adf49ee0) SHA1(11fc2cd197bfe3ecb6af55c3c7a326c94988d2bd) )
	ROM_LOAD( "tma1-v-a.3j",    0x1000, 0x1000, CRC(a5318f2d) SHA1(e42f5e51804195c64a56addb18b7ad12c57bb09a) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "tma1-v-a.7m",    0x0000, 0x1000, CRC(186762f8) SHA1(711fdd37392656bdd5027e020d51d083ccd7c407) )
	ROM_LOAD( "tma1-v-a.7n",    0x1000, 0x1000, CRC(e0e08bba) SHA1(315eba2c10d426c9c0bb4e36987bf8ebed7df9a0) )
	ROM_LOAD( "tma1-v-a.7p",    0x2000, 0x1000, CRC(7b27c8c1) SHA1(3fb2613ce19e353fbcc77b6817927794fb35810f) )
	ROM_LOAD( "tma1-v-a.7s",    0x3000, 0x1000, CRC(912ba80a) SHA1(351fb5b160216eb10e281815d05a7165ca0e5909) )
	ROM_LOAD( "tma1-v-a.7t",    0x4000, 0x1000, CRC(5cbb92a5) SHA1(a78a378e6d3060143dc456e9c33a5068da648331) )
	ROM_LOAD( "tma1-v-a.7u",    0x5000, 0x1000, CRC(13afb9ed) SHA1(b29dcd91cf5e639ee50b734afc7a3afce79634df) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tma1-c-4p.4p",   0x0000, 0x0200, CRC(afc9bd41) SHA1(90b739c4c7f24a88b6ac5ca29b06c032906a2801) )

	ROM_REGION( 0x0020, "decoder_prom", 0 ) // main cpu memory map decoding prom
	ROM_LOAD( "tma1-c-5b.5b",   0x0000, 0x0020, CRC(58d86098) SHA1(d654995004b9052b12d3b682a2b39530e70030fc) )
ROM_END

ROM_START( masao )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "masao-4.rom",  0x0000, 0x2000, CRC(07a75745) SHA1(acc760242a8862d177e3cff90aa32c4f3dac4e65) )
	ROM_LOAD( "masao-3.rom",  0x2000, 0x2000, CRC(55c629b6) SHA1(1f5b5699821871aadacc511663cb4bd4e357e215) )
	ROM_LOAD( "masao-2.rom",  0x4000, 0x2000, CRC(42e85240) SHA1(bc8cdf867b743c5ee58fcacb63a44f826c8f8c1a) )
	ROM_LOAD( "masao-1.rom",  0xf000, 0x1000, CRC(b2817af9) SHA1(95e83752e544671a68df2107fae1010b187f04a6) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 64k for sound
	ROM_LOAD( "masao-5.rom",  0x0000, 0x1000, CRC(bd437198) SHA1(ebae88461984afc97bbc103fc6d95bc3c1865eec) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "masao-6.rom",  0x0000, 0x1000, CRC(1c9e0be2) SHA1(b4a650412dad90c6f6d79e93cde49055703b7f3e) )
	ROM_LOAD( "masao-7.rom",  0x1000, 0x1000, CRC(747c1349) SHA1(54674f78edf86953b7d500b66393483d1a5ce8ab) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "masao-8.rom",  0x0000, 0x1000, CRC(186762f8) SHA1(711fdd37392656bdd5027e020d51d083ccd7c407) )
	ROM_LOAD( "masao-9.rom",  0x1000, 0x1000, CRC(50be3918) SHA1(73e22eee67a03732ff57e523f900f20c6aee0491) )
	ROM_LOAD( "masao-10.rom", 0x2000, 0x1000, CRC(56be6ccd) SHA1(15a6e16c189d45f72761ebcbe9db5001bdecd659) )
	ROM_LOAD( "masao-11.rom", 0x3000, 0x1000, CRC(912ba80a) SHA1(351fb5b160216eb10e281815d05a7165ca0e5909) )
	ROM_LOAD( "masao-12.rom", 0x4000, 0x1000, CRC(5cbb92a5) SHA1(a78a378e6d3060143dc456e9c33a5068da648331) )
	ROM_LOAD( "masao-13.rom", 0x5000, 0x1000, CRC(13afb9ed) SHA1(b29dcd91cf5e639ee50b734afc7a3afce79634df) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tma1-c-4p.4p", 0x0000, 0x0200, CRC(afc9bd41) SHA1(90b739c4c7f24a88b6ac5ca29b06c032906a2801) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, mario,  0,     mario, mario,  mario_state, empty_init, ROT0, "Nintendo of America", "Mario Bros. (US, Revision G)",    MACHINE_SUPPORTS_SAVE )
GAME( 1983, mariof, mario, mario, mariof, mario_state, empty_init, ROT0, "Nintendo of America", "Mario Bros. (US, Revision F)",    MACHINE_SUPPORTS_SAVE )
GAME( 1983, marioe, mario, mario, mario,  mario_state, empty_init, ROT0, "Nintendo of America", "Mario Bros. (US, Revision E)",    MACHINE_SUPPORTS_SAVE )
GAME( 1983, marioj, mario, mario, marioj, mario_state, empty_init, ROT0, "Nintendo",            "Mario Bros. (Japan, Revision C)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, masao,  mario, masao, mario,  mario_state, empty_init, ROT0, "bootleg",             "Masao",                           MACHINE_SUPPORTS_SAVE )
