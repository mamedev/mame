// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano

/***************************************************************************

Cop 01       (c) 1985 Nichibutsu
Mighty Guy   (c) 1986 Nichibutsu

driver by Carlos A. Lozano <calb@gsyc.inf.uc3m.es>

TODO:
- Fix priority kludge
- Inaccurate 1412M2 protection chip emulation in mightguy, used by the sound CPU.
  This is probably an extra CPU (program ROM is the ic2 one), presumably
  with data / address line scrambling
- Some sound problems remaining, not just 1412M2, but see also MT7949


Mighty Guy board layout:
-----------------------

  cpu

12MHz     SW1
          SW2                 clr.13D clr.14D clr.15D      clr.19D

      Nichibutsu
      NBB 60-06                        4     5

  1 2 3  6116 6116                    6116   6116

-------

 video

 6116   11  Nichibutsu
            NBA 60-07    13B                               20MHz
                                2148                2148
                                2148                2148

              6116             9                        8  2E
 20G          10               7                        6
 -------

 audio sub-board MT-S3
 plugs into 40 pin socket at 20G

                     10.IC2

                     Nichibutsu
                     1412M2 (Yamaha 3810?)
                               8MHz
                     YM3526

***************************************************************************/

#include "emu.h"

#include "nb1412m2.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

#define MIGHTGUY_HACK    0

class cop01_state : public driver_device
{
public:
	cop01_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void cop01(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	uint8_t sound_command_r();

	void cop01_base(machine_config &config);

	required_device<cpu_device> m_audiocpu;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fgvideoram;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	uint8_t m_vreg[4]{};

	// sound-related
	uint8_t m_pulse = 0;
	static constexpr int TIMER_RATE = 11475;  // unknown, hand-tuned to match audio reference

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void irq_ack_w(uint8_t data);
	uint8_t sound_irq_ack_w();
	void background_w(offs_t offset, uint8_t data);
	void foreground_w(offs_t offset, uint8_t data);
	void vreg_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void audio_io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

class mightguy_state : public cop01_state
{
public:
	mightguy_state(const machine_config &mconfig, device_type type, const char *tag) :
		cop01_state(mconfig, type, tag),
		m_prot(*this, "prot_chip"),
		m_fake(*this, "FAKE")
	{ }

	void mightguy(machine_config &config);

	template <int Mask> int area_r();

	void init_mightguy();

private:
	void audio_io_map(address_map &map) ATTR_COLD;

	required_device<nb1412m2_device> m_prot;

	required_ioport m_fake;
};


void cop01_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x100; i++)
	{
		int const r = pal4bit(color_prom[i + 0x000]);
		int const g = pal4bit(color_prom[i + 0x100]);
		int const b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x300;

	// characters use colors 0x00-0x0f (or 0x00-0x7f, but the eight rows are identical)
	for (int i = 0; i < 0x10; i++)
		palette.set_pen_indirect(i, i);

	// background tiles use colors 0xc0-0xff
	// I don't know how much of the lookup table PROM is hooked up,
	// I'm only using the first 32 bytes because the rest is empty.
	for (int i = 0; i < 0x80; i++)
	{
		uint8_t const ctabentry = 0xc0 | (i & 0x30) | (color_prom[((i & 0x40) >> 2) | (i & 0x0f)] & 0x0f);
		palette.set_pen_indirect(i + 0x10, ctabentry);
	}

	// sprites use colors 0x80-0x8f (or 0x80-0xbf, but the four rows are identical)
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t ctabentry = 0x80 | (color_prom[i + 0x100] & 0x0f);
		palette.set_pen_indirect(i + 0x90, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(cop01_state::get_bg_tile_info)
{
	int const tile = m_bgvideoram[tile_index];
	int const attr = m_bgvideoram[tile_index + 0x800];
	int pri = (attr & 0x80) >> 7;

	/* kludge: priority is not actually pen based, but color based. Since the
	 * game uses a lookup table, the two are not the same thing.
	 * Palette entries with bits 2&3 set have priority over sprites.
	 * tilemap.cpp can't handle that yet, so I'm cheating, because I know that
	 * color codes using the second row of the lookup table don't use palette
	 * entries 12-15.
	 * The only place where this has any effect is the beach at the bottom of
	 * the screen right at the beginning of mightguy. cop01 doesn't seem to
	 * use priority at all.
	 */
	if (attr & 0x10)
		pri = 0;

	tileinfo.set(1, tile + ((attr & 0x03) << 8), (attr & 0x1c) >> 2, 0);
	tileinfo.group = pri;
}

TILE_GET_INFO_MEMBER(cop01_state::get_fg_tile_info)
{
	int const tile = m_fgvideoram[tile_index];
	tileinfo.set(0, tile, 0, 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void cop01_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cop01_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cop01_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(15);

	// priority doesn't exactly work this way, see above
	m_bg_tilemap->set_transmask(0, 0xffff, 0x0000); // split type 0 is totally transparent in front half
	m_bg_tilemap->set_transmask(1, 0x0fff, 0xf000); // split type 1 has pens 0-11 transparent in front half
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void cop01_state::background_w(offs_t offset, uint8_t data)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

void cop01_state::foreground_w(offs_t offset, uint8_t data)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void cop01_state::vreg_w(offs_t offset, uint8_t data)
{
	/*  0x40: --xx---- sprite bank, coin counters, flip screen
	 *        -----x-- flip screen
	 *        ------xx coin counters
	 *  0x41: xxxxxxxx xscroll
	 *  0x42: ---xx--- ? matches the bg tile color most of the time, but not
	 *                 during level transitions. Maybe sprite palette bank?
	 *                 (the four banks in the PROM are identical)
	 *        ------x- unused (xscroll overflow)
	 *        -------x msb xscroll
	 *  0x43: xxxxxxxx yscroll
	 */
	m_vreg[offset] = data;

	if (offset == 0)
	{
		machine().bookkeeping().coin_counter_w(0, data & 1);
		machine().bookkeeping().coin_counter_w(1, data & 2);
		flip_screen_set(data & 4);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

void cop01_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int code = m_spriteram[offs + 1];
		int const attr = m_spriteram[offs + 2];
		/* xxxx---- color
		 * ----xx-- flipy,flipx
		 * -------x msbx
		 */
		int const color = attr >> 4;
		int flipx = attr & 0x04;
		int flipy = attr & 0x08;

		int sx = (m_spriteram[offs + 3] - 0x80) + 256 * (attr & 0x01);
		int sy = 240 - m_spriteram[offs];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (code & 0x80)
			code += (m_vreg[0] & 0x30) << 3;

		m_gfxdecode->gfx(2)->transmask(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				sx, sy,
				m_palette->transpen_mask(*m_gfxdecode->gfx(2), color, 0x8f));
	}
}


uint32_t cop01_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_vreg[1] + 256 * (m_vreg[2] & 1));
	m_bg_tilemap->set_scrolly(0, m_vreg[3]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

uint8_t cop01_state::sound_command_r()
{
	int res = (m_soundlatch->read() & 0x7f) << 1;

	// bit 0 seems to be a timer
	if ((m_audiocpu->total_cycles() / TIMER_RATE) & 1)
	{
		if (m_pulse == 0)
			res |= 1;

		m_pulse = 1;
	}
	else
		m_pulse = 0;

	return res;
}


template <int Mask>
int mightguy_state::area_r()
{
	return (m_fake->read() & Mask) ? 1 : 0;
}

void cop01_state::irq_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

uint8_t cop01_state::sound_irq_ack_w()
{
	m_audiocpu->set_input_line(0, CLEAR_LINE);
	return 0;
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void cop01_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram(); // c000-c7ff in cop01
	map(0xd000, 0xdfff).ram().w(FUNC(cop01_state::background_w)).share(m_bgvideoram);
	map(0xe000, 0xe0ff).writeonly().share(m_spriteram);
	map(0xf000, 0xf3ff).w(FUNC(cop01_state::foreground_w)).share(m_fgvideoram);
}

void cop01_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("P1");
	map(0x01, 0x01).portr("P2");
	map(0x02, 0x02).portr("SYSTEM");
	map(0x03, 0x03).portr("DSW1");
	map(0x04, 0x04).portr("DSW2");
	map(0x40, 0x43).w(FUNC(cop01_state::vreg_w));
	map(0x44, 0x44).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x45, 0x45).w(FUNC(cop01_state::irq_ack_w)); // ?
}

void cop01_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8000).r(FUNC(cop01_state::sound_irq_ack_w));
	map(0xc000, 0xc7ff).ram();
}

void cop01_state::audio_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x02, 0x03).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x04, 0x05).w("ay3", FUNC(ay8910_device::address_data_w));
	map(0x06, 0x06).r(FUNC(cop01_state::sound_command_r));
}

void mightguy_state::audio_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ymsnd", FUNC(ym3526_device::write));
	map(0x02, 0x02).w("prot_chip", FUNC(nb1412m2_device::command_w));
	map(0x03, 0x03).rw("prot_chip", FUNC(nb1412m2_device::data_r), FUNC(nb1412m2_device::data_w));
	map(0x06, 0x06).r(FUNC(mightguy_state::sound_command_r));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( cop01 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    // TEST, COIN, START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x10, 0x10, "Invulnerability (Cheat, 1/2)" ) // Undocumented invulnerability cheat (both DIP switches need to be ON)
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	/* DP2:3,4,5 defined in manual/test-mode as:
	PORT_DIPNAME( 0x10, 0x10, "1st Bonus Life" )
	PORT_DIPSETTING(    0x10, "20000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPNAME( 0x60, 0x60, "2nd Bonus Life" )
	PORT_DIPSETTING(    0x60, "30000" )
	PORT_DIPSETTING(    0x20, "50000" )
	PORT_DIPSETTING(    0x40, "100000" )
	PORT_DIPSETTING(    0x00, "150000" ) */
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x70, "20k 50k 30k+" )
	PORT_DIPSETTING(    0x30, "20k 70k 50k+" )
	PORT_DIPSETTING(    0x50, "20k 120k 100k+" )
	PORT_DIPSETTING(    0x10, "20k 170k 150k+" )
	PORT_DIPSETTING(    0x60, "30k 60k 30k+" )
	PORT_DIPSETTING(    0x20, "30k 80k 50k+" )
	PORT_DIPSETTING(    0x40, "30k 130k 100k+" )
	PORT_DIPSETTING(    0x00, "30k 180k 150k+" )
	PORT_DIPNAME( 0x80, 0x80, "Invulnerability (Cheat, 2/2)" ) // Undocumented invulnerability cheat (both DIP switches need to be ON)
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* There is an ingame bug at 0x00e4 to 0x00e6 that performs 3 times 'rrca' instead of 'rlca'
   so DSW1-8 has no effect and you can NOT start a game at areas 5 to 8. */
static INPUT_PORTS_START( mightguy )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )            // same as the service dip switch
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x04, "Every 200k" )
	PORT_DIPSETTING(    0x00, "500k only" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) ) // actually reversed compared to service mode
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(mightguy_state, area_r<0x04>)    // "Start Area" - see fake Dip Switch

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, "Invincibility")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(mightguy_state, area_r<0x01>)    // "Start Area" - see fake Dip Switch
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(mightguy_state, area_r<0x02>)    // "Start Area" - see fake Dip Switch

	PORT_START("FAKE")  // FAKE Dip Switch
	PORT_DIPNAME( 0x07, 0x07, "Starting Area" )
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	// Not working due to ingame bug (see above)
#if MIGHTGUY_HACK
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8" )
#endif
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{
		RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0,   4, 0,
		RGN_FRAC(1,2)+12, RGN_FRAC(1,2)+8,  12, 8,
		RGN_FRAC(1,2)+20, RGN_FRAC(1,2)+16, 20, 16,
		RGN_FRAC(1,2)+28, RGN_FRAC(1,2)+24, 28, 24,
	},
	{
		0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32
	},
	64*8
};

static GFXDECODE_START( gfx_cop01 )
	GFXDECODE_ENTRY( "chars",   0, gfx_8x8x4_packed_lsb,  0,  1 )
	GFXDECODE_ENTRY( "tiles",   0, gfx_8x8x4_packed_lsb, 16,  8 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,    16+8*16, 16 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void cop01_state::machine_start()
{
	save_item(NAME(m_pulse));
	save_item(NAME(m_vreg));
}

void cop01_state::machine_reset()
{
	m_pulse = 0;
	m_vreg[0] = 0;
	m_vreg[1] = 0;
	m_vreg[2] = 0;
	m_vreg[3] = 0;
}


void cop01_state::cop01_base(machine_config &config)
{
	constexpr XTAL MAINCPU_CLOCK = XTAL(12'000'000);

	// basic machine hardware
	Z80(config, m_maincpu, MAINCPU_CLOCK / 2); // unknown clock / divider
	m_maincpu->set_addrmap(AS_PROGRAM, &cop01_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &cop01_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(cop01_state::irq0_line_assert));

	Z80(config, m_audiocpu, XTAL(3'000'000)); // unknown clock / divider, hand-tuned to match audio reference
	m_audiocpu->set_addrmap(AS_PROGRAM, &cop01_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &cop01_state::audio_io_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(cop01_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cop01);
	PALETTE(config, m_palette, FUNC(cop01_state::palette), 16+8*16+16*16, 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch).data_pending_callback().set_inputline(m_audiocpu, 0);
}

void cop01_state::cop01(machine_config &config)
{
	cop01_base(config);

	AY8910(config, "ay1", 1250000).add_route(ALL_OUTPUTS, "mono", 0.50); // unknown clock / divider, hand-tuned to match audio reference
	AY8910(config, "ay2", 1250000).add_route(ALL_OUTPUTS, "mono", 0.25); // "
	AY8910(config, "ay3", 1250000).add_route(ALL_OUTPUTS, "mono", 0.25); // "
}

void mightguy_state::mightguy(machine_config &config)
{
	cop01_base(config);

	constexpr XTAL AUDIOCPU_CLOCK = XTAL(8'000'000);

	m_audiocpu->set_clock(AUDIOCPU_CLOCK / 2); // unknown divider
	m_audiocpu->set_addrmap(AS_IO, &mightguy_state::audio_io_map);

	NB1412M2(config, m_prot, XTAL(8'000'000) / 2); // divided by 2 maybe
	m_prot->dac_callback().set("dac", FUNC(dac_byte_interface::data_w));

	YM3526(config, "ymsnd", AUDIOCPU_CLOCK / 2).add_route(ALL_OUTPUTS, "mono", 1.0); // unknown divider

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "mono", 0.5); // unknown DAC
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( cop01 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cop01.2b",     0x0000, 0x4000, CRC(5c2734ab) SHA1(dd6724dfb1c58e6ce3c1c99cad8732a0f5c9b773) )
	ROM_LOAD( "cop02.4b",     0x4000, 0x4000, CRC(9c7336ef) SHA1(2aa58aea19dafb53190d9bef7b3aa9c3004522f0) )
	ROM_LOAD( "cop03.5b",     0x8000, 0x4000, CRC(2566c8bf) SHA1(c9d98afd1f02a208b1af1d418e69e88f8703afa5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cop15.17b",    0x0000, 0x4000, CRC(6a5f08fa) SHA1(8838549502b1a6ac72dd5efd58e0968f8abe338a) )
	ROM_LOAD( "cop16.18b",    0x4000, 0x4000, CRC(56bf6946) SHA1(5414d00c6de96cfb5a3e68c35376333df6c525ee) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "cop14.15g",    0x00000, 0x2000, CRC(066d1c55) SHA1(017a0576799d39b919e6ca9b4a7f106ed04c0f94) )

	ROM_REGION( 0x08000, "tiles", 0 )
	ROM_LOAD( "cop04.15c",    0x00000, 0x4000, CRC(622d32e6) SHA1(982b585e9a1115bce25c1788999d34423ccb83ab) )
	ROM_LOAD( "cop05.16c",    0x04000, 0x4000, CRC(c6ac5a35) SHA1(dab3500981663ee19abac5bfeaaf6a07a3953d75) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "cop06.3g",     0x00000, 0x2000, CRC(f1c1f4a5) SHA1(139aa23416e71361fe62ce336e3f0529a21acb81) )
	ROM_LOAD( "cop07.5g",     0x02000, 0x2000, CRC(11db7b72) SHA1(47a1223ed3e7b294d7e59c05d119488ef6b3dc7a) )
	ROM_LOAD( "cop08.6g",     0x04000, 0x2000, CRC(a63ddda6) SHA1(59aaa1fe0c023c4f0d4cfbdb9ca922182201c145) )
	ROM_LOAD( "cop09.8g",     0x06000, 0x2000, CRC(855a2ec3) SHA1(8a54c0ceedeeafd7c1a6a35b4efab28046967951) )
	ROM_LOAD( "cop10.3e",     0x08000, 0x2000, CRC(444cb19d) SHA1(e74118b027db65ba06291bc0fe0ff50bcacc32c2) )
	ROM_LOAD( "cop11.5e",     0x0a000, 0x2000, CRC(9078bc04) SHA1(3d8614415027f5db9ddb77b89656e4c7fc9d28de) )
	ROM_LOAD( "cop12.6e",     0x0c000, 0x2000, CRC(257a6706) SHA1(9e7ef1f40630b94849bdc3fd13ee6e7311fffd45) )
	ROM_LOAD( "cop13.8e",     0x0e000, 0x2000, CRC(07c4ea66) SHA1(12665c0fb648fd208805e81d056ab377d65b267a) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "copproma.13d", 0x0000, 0x0100, CRC(97f68a7a) SHA1(010eaca95eeb5caec083bd184ec31e0f433fff8c) ) // red
	ROM_LOAD( "coppromb.14d", 0x0100, 0x0100, CRC(39a40b4c) SHA1(456b7f97fbd1cb4beb756033ec76a89ffe8de168) ) // green
	ROM_LOAD( "coppromc.15d", 0x0200, 0x0100, CRC(8181748b) SHA1(0098ae250095b4ac8af1811b4e41d86e3f587c7b) ) // blue
	ROM_LOAD( "coppromd.19d", 0x0300, 0x0100, CRC(6a63dbb8) SHA1(50f971f173147203cd24dc4fa7f0a27d2179f1cc) ) // tile lookup table
	ROM_LOAD( "copprome.2e",  0x0400, 0x0100, CRC(214392fa) SHA1(59d235c3e584e7fd484edf5c78c43d2597c1c3a8) ) // sprite lookup table
	ROM_LOAD( "13b",          0x0500, 0x0100, NO_DUMP ) // state machine data used for video signals generation (not used in emulation)
ROM_END

ROM_START( cop01a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cop01alt.001", 0x0000, 0x4000, CRC(a13ee0d3) SHA1(2f28f901bdc041c79f785821d0052823654983a2) )
	ROM_LOAD( "cop01alt.002", 0x4000, 0x4000, CRC(20bad28e) SHA1(79155880ae1c9e8d19390c163cac31093ee11604) )
	ROM_LOAD( "cop01alt.003", 0x8000, 0x4000, CRC(a7e10b79) SHA1(ec7e4153a211d070c2dc09ab98a59f61ab10ea78) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cop01alt.015", 0x0000, 0x4000, CRC(95be9270) SHA1(ffb4786e354c4c6ddce134ae3362da660199fd44) )
	ROM_LOAD( "cop01alt.016", 0x4000, 0x4000, CRC(c20bf649) SHA1(a719ad6bf35811957ad32e6f07494bb00f256965) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "cop01alt.014", 0x00000, 0x2000, CRC(edd8a474) SHA1(42f0655535f1e10840da383129da69627d67ff8a) )

	ROM_REGION( 0x08000, "tiles", 0 )
	ROM_LOAD( "cop04.15c",    0x00000, 0x4000, CRC(622d32e6) SHA1(982b585e9a1115bce25c1788999d34423ccb83ab) )
	ROM_LOAD( "cop05.16c",    0x04000, 0x4000, CRC(c6ac5a35) SHA1(dab3500981663ee19abac5bfeaaf6a07a3953d75) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "cop01alt.006", 0x00000, 0x2000, CRC(cac7dac8) SHA1(25990ac4614de2ae61d663323bd67acc137bbc4a) )
	ROM_LOAD( "cop07.5g",     0x02000, 0x2000, CRC(11db7b72) SHA1(47a1223ed3e7b294d7e59c05d119488ef6b3dc7a) )
	ROM_LOAD( "cop08.6g",     0x04000, 0x2000, CRC(a63ddda6) SHA1(59aaa1fe0c023c4f0d4cfbdb9ca922182201c145) )
	ROM_LOAD( "cop09.8g",     0x06000, 0x2000, CRC(855a2ec3) SHA1(8a54c0ceedeeafd7c1a6a35b4efab28046967951) )
	ROM_LOAD( "cop01alt.010", 0x08000, 0x2000, CRC(94aee9d6) SHA1(dd6f27dcee761c84447b8326bfa0532b7d708721) )
	ROM_LOAD( "cop11.5e",     0x0a000, 0x2000, CRC(9078bc04) SHA1(3d8614415027f5db9ddb77b89656e4c7fc9d28de) )
	ROM_LOAD( "cop12.6e",     0x0c000, 0x2000, CRC(257a6706) SHA1(9e7ef1f40630b94849bdc3fd13ee6e7311fffd45) )
	ROM_LOAD( "cop13.8e",     0x0e000, 0x2000, CRC(07c4ea66) SHA1(12665c0fb648fd208805e81d056ab377d65b267a) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "copproma.13d", 0x0000, 0x0100, CRC(97f68a7a) SHA1(010eaca95eeb5caec083bd184ec31e0f433fff8c) ) // red
	ROM_LOAD( "coppromb.14d", 0x0100, 0x0100, CRC(39a40b4c) SHA1(456b7f97fbd1cb4beb756033ec76a89ffe8de168) ) // green
	ROM_LOAD( "coppromc.15d", 0x0200, 0x0100, CRC(8181748b) SHA1(0098ae250095b4ac8af1811b4e41d86e3f587c7b) ) // blue
	ROM_LOAD( "coppromd.19d", 0x0300, 0x0100, CRC(6a63dbb8) SHA1(50f971f173147203cd24dc4fa7f0a27d2179f1cc) ) // tile lookup table
	ROM_LOAD( "copprome.2e",  0x0400, 0x0100, CRC(214392fa) SHA1(59d235c3e584e7fd484edf5c78c43d2597c1c3a8) ) // sprite lookup table
	ROM_LOAD( "13b",          0x0500, 0x0100, NO_DUMP ) // state machine data used for video signals generation (not used in emulation)
ROM_END

ROM_START( mightguy )
	ROM_REGION( 0x60000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "1.2b",       0x0000, 0x4000,CRC(bc8e4557) SHA1(4304ac1a0e11bad254ad937195f0be6e7186577d) )
	ROM_LOAD( "2.4b",       0x4000, 0x4000,CRC(fb73d684) SHA1(d8a4b6fb93b2c3710fc66f92df05c1459e4171c3) )
	ROM_LOAD( "3.5b",       0x8000, 0x4000,CRC(b14b6ab8) SHA1(a60059dd54c8cc974334fd879ff0cfd436a7a981) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "11.15b",     0x0000, 0x4000, CRC(576183ea) SHA1(e3f28e8e8c34ab396d158da122584ed226729c99) )

	ROM_REGION( 0x8000, "prot_chip", 0 ) // 1412M2 protection data, Z80 encrypted code presumably
	ROM_LOAD( "10.ic2",     0x0000, 0x8000, CRC(1a5d2bb1) SHA1(0fd4636133a980ba9ffa076f9010474586d37635) )

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "10.15g",     0x0000, 0x2000, CRC(17874300) SHA1(f97bee0ab491b04fe4950ebe9587031db6c815a3) )

	ROM_REGION( 0x08000, "tiles", 0 )
	ROM_LOAD( "4.15c",      0x0000, 0x4000,CRC(84d29e76) SHA1(98e6c6e4a95471c5bef9fcb85a663b86eeda6b6d) )
	ROM_LOAD( "5.16c",      0x4000, 0x4000,CRC(f7bb8d82) SHA1(6ab6585827482fd23c3be129977a4443623d831c) )

	ROM_REGION( 0x14000, "sprites", 0 )
	ROM_LOAD( "6.3g",       0x00000, 0x2000, CRC(6ff88615) SHA1(8bfeab97bd1a14861e3a7538c0dd3c073adf29aa) )
	ROM_LOAD( "7.8g",       0x02000, 0x8000, CRC(e79ea66f) SHA1(2db80eef5375294bf9c7819f4090ec71f7f2be25) )
	ROM_LOAD( "8.3e",       0x0a000, 0x2000, CRC(29f6eb44) SHA1(d7957c8579d7d32c52c19d2fe7b90d1c890f29ea) )
	ROM_LOAD( "9.8e",       0x0c000, 0x8000, CRC(b9f64c6f) SHA1(82ec6ba689f16fed1141cd32640a8b1f1ab14bdd) )

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "clr.13d",    0x000, 0x100, CRC(c4cf0bdd) SHA1(350842c46a71fb5db43c8823662378f178bbda4f) ) // red
	ROM_LOAD( "clr.14d",    0x100, 0x100, CRC(5b3b8a9b) SHA1(6b660f5f7b0efdc20a79a9fd5a1eb30c85b27324) ) // green
	ROM_LOAD( "clr.15d",    0x200, 0x100, CRC(6c072a64) SHA1(5ce5306af478330eb3e94aa7c8bff08f34ba6ea5) ) // blue
	ROM_LOAD( "clr.19d",    0x300, 0x100, CRC(19b66ac6) SHA1(5e7de11f40685effa077377e7a55d7fecf752508) ) // tile lookup table
	ROM_LOAD( "2e",         0x400, 0x100, CRC(d9c45126) SHA1(aafebe424afa400ed320f17afc2b910eaada29f5) ) // sprite lookup table
	ROM_LOAD( "13b",        0x500, 0x100, CRC(4a6f9a6d) SHA1(65f1e0bfacd1f354ece1b18598a551044c27c4d1) ) // state machine data used for video signals generation (not used in emulation)
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void mightguy_state::init_mightguy()
{
#if MIGHTGUY_HACK
	// This is a hack to fix the game code to get a fully working "Starting Area" fake DIP switch
	uint8_t *rom = (uint8_t *)memregion("maincpu")->base();
	rom[0x00e4] = 0x07; // rlca
	rom[0x00e5] = 0x07; // rlca
	rom[0x00e6] = 0x07; // rlca
	// To avoid checksum error
	rom[0x027f] = 0x00;
	rom[0x0280] = 0x00;
#endif
}

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1985, cop01,    0,     cop01,    cop01,    cop01_state,    empty_init,    ROT0,   "Nichibutsu", "Cop 01 (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1985, cop01a,   cop01, cop01,    cop01,    cop01_state,    empty_init,    ROT0,   "Nichibutsu", "Cop 01 (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, mightguy, 0,     mightguy, mightguy, mightguy_state, init_mightguy, ROT270, "Nichibutsu", "Mighty Guy",     MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
