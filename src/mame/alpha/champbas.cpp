// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Jarek Parchanski, Nicola Salmoria
/***************************************************************************

Talbot                     - (c) 1982 Alpha Denshi Co.
Champion Base Ball         - (c) 1983 Alpha Denshi Co.
Champion Base Ball Part-2  - (c) 1983 Alpha Denshi Co.
Exciting Soccer            - (c) 1983 Alpha Denshi Co.
Exciting Soccer II         - (c) 1984 Alpha Denshi Co.

Note: the Champion Baseball II unofficial schematics show a 8302 instead of
the 8201, however the MCU is used like a plain 8201, 830x extra instructions
are not used.

champbbj and champbb2 has Alpha8201 mcu for protection.
champbja is a patched version of champbbj with different protection.
exctsccr has Alpha8302 MCU for protection.

main CPU

0000-5fff ROM
6000-63ff MCU shared RAM
7800-7fff ROM (Champion Baseball 2 only)
8000-83ff Video RAM
8400-87ff Color RAM
8800-8fff RAM

read:
a000      IN0
a040      IN1
a080      DSW
a0a0      ?(same as DSW)
a0c0      COIN

write:
7000      8910 write
7001      8910 control
8ff0-8fff sprites
a000      irq enable
a006      MCU HALT control
a007      NOP (MCU shared RAM switch)
a060-a06f sprites
a080      command for the sound CPU
a0c0      watchdog reset (watchdog time = 16xvblank)

sub CPU (speech DAC)

read:
0000-5fff   ROM
6000(-7fff) sound latch
e000-e3ff   RAM

write:

8000(-9fff) 4bit status for main CPU
a000(-bfff) clear sound latch
c000(-dfff) DAC
e000-e3ff   RAM


Notes:
------
- Bit 2 of the watchdog counter can be read through an input port. The games check
  it on boot and hang if it is not 0. Also, the Talbot MCU does a security check
  and crashes if the bit doesn't match bit 2 of RAM location 0x8c00.

- The Exciting Soccer bootleg runs on a modified Champion Baseball board. The
  original board has vastly improved sound hardware which is therefore missing
  from the bootleg.


TODO:
-----
- Exciting Soccer: interrupt source for sound CPU is unknown.
- Exciting Soccer: sound CPU writes to unknown ports on startup. Timer configure?
- Exciting Soccer: Unknown writes to 8910 I/O ports (filters?)

***************************************************************************/

#include "emu.h"

#include "alpha8201.h"

#include "cpu/z80/z80.h"
#include "cpu/m6805/m68705.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class champbas_state : public driver_device
{
public:
	champbas_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainlatch(*this, "mainlatch"),
		m_alpha_8201(*this, "alpha_8201"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_mainram(*this, "mainram"),
		m_vram(*this, "vram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2")
	{ }

	int watchdog_bit2();

	void init_champbas();

	void champbas(machine_config &config);
	void champbb2(machine_config &config);
	void champbb2j(machine_config &config);
	void talbot(machine_config &config);
	void tbasebal(machine_config &config);
	void champbasjb(machine_config &config);
	void champbasj(machine_config &config);
	void champbasja(machine_config &config);

protected:
	// handlers
	void irq_enable_w(int state);
	uint8_t champbja_protection_r(offs_t offset);

	void vblank_irq(int state);

	void tilemap_w(offs_t offset, uint8_t data);
	void gfxbank_w(int state);
	void palette_bank_w(int state);

	void champbas_palette(palette_device &palette) const;
	TILE_GET_INFO_MEMBER(champbas_get_bg_tile_info);

	uint32_t screen_update_champbas(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void champbas_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void champbas_map(address_map &map) ATTR_COLD;
	void champbasj_map(address_map &map) ATTR_COLD;
	void champbasja_map(address_map &map) ATTR_COLD;
	void champbasjb_map(address_map &map) ATTR_COLD;
	void champbb2_map(address_map &map) ATTR_COLD;
	void champbb2j_map(address_map &map) ATTR_COLD;
	void tbasebal_map(address_map &map) ATTR_COLD;
	void champbas_sound_map(address_map &map) ATTR_COLD;

	// devices, memory pointers
	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_mainlatch;
	optional_device<alpha_8201_device> m_alpha_8201;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_spriteram2;

	// internal state
	uint8_t m_irq_mask = 0U;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_gfx_bank = 0U;
	uint8_t m_palette_bank = 0U;
};

class exctsccr_state : public champbas_state
{
public:
	exctsccr_state(const machine_config &mconfig, device_type type, const char *tag) :
		champbas_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu")
	{ }

	void init_exctsccr();

	void exctsccr(machine_config &config);
	void exctsccrb(machine_config &config);
	void exctscc2(machine_config &config);

protected:
	TIMER_DEVICE_CALLBACK_MEMBER(exctsccr_sound_irq);

	void exctsccr_palette(palette_device &palette) const;
	TILE_GET_INFO_MEMBER(exctsccr_get_bg_tile_info);

	uint32_t screen_update_exctsccr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void exctsccr_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void video_start() override ATTR_COLD;

	void exctsccr_map(address_map &map) ATTR_COLD;
	void exctsccrb_map(address_map &map) ATTR_COLD;
	void exctsccr_sound_map(address_map &map) ATTR_COLD;
	void exctsccr_sound_io_map(address_map &map) ATTR_COLD;
	void exctscc2_sound_io_map(address_map &map) ATTR_COLD;

private:
	required_device<cpu_device> m_audiocpu;
};



/***************************************************************************

  Convert the color PROMs into a more useable format.

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

void champbas_state::champbas_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };

	/* compute the color output resistor weights */
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 0, 0,
			3, &resistances_rg[0], gweights, 0, 0,
			2, &resistances_b[0],  bweights, 0, 0);

	/* create a lookup table for the palette */
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	color_prom += 0x20;

	for (int i = 0; i < 0x200; i++)
	{
		uint8_t const ctabentry = (color_prom[i & 0xff] & 0x0f) | ((i & 0x100) >> 4);
		palette.set_pen_indirect(i, ctabentry);
	}
}


void exctsccr_state::exctsccr_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	/* create a lookup table for the palette */
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* characters / sprites (3bpp) */
	for (int i = 0; i < 0x100; i++)
	{
		int const swapped_i = bitswap<8>(i, 2, 7, 6, 5, 4, 3, 1, 0);
		uint8_t const ctabentry = (color_prom[swapped_i] & 0x0f) | ((i & 0x80) >> 3);
		palette.set_pen_indirect(i, ctabentry);
	}

	/* sprites (4bpp) */
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = (color_prom[0x100 + i] & 0x0f) | 0x10;
		palette.set_pen_indirect(i + 0x100, ctabentry);
	}
}



/*************************************
 *
 *  Callbacks for the TileMap code
 *
 *************************************/

TILE_GET_INFO_MEMBER(champbas_state::champbas_get_bg_tile_info)
{
	int const code = m_vram[tile_index] | (m_gfx_bank << 8);
	int const color = (m_vram[tile_index + 0x400] & 0x1f) | 0x20;

	tileinfo.set(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(exctsccr_state::exctsccr_get_bg_tile_info)
{
	int const code = m_vram[tile_index] | (m_gfx_bank << 8);
	int const color = m_vram[tile_index + 0x400] & 0x0f;

	tileinfo.set(0, code, color, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

void champbas_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(champbas_state::champbas_get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void exctsccr_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(exctsccr_state::exctsccr_get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

void champbas_state::champbas_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element* const gfx = m_gfxdecode->gfx(1);
	assert (m_spriteram.bytes() == 0x10);

	for (int offs = 0x10-2; offs >= 0; offs -= 2)
	{
		// spriteram holds x/y data
		int sx = m_spriteram[offs + 1] - 16;
		int sy = 255 - m_spriteram[offs];

		// attribute data is from last section of mainram
		uint8_t *attr = &(m_mainram[0x7f0]);
		int code = (attr[offs] >> 2 & 0x3f) | (m_gfx_bank << 6);
		int color = (attr[offs + 1] & 0x1f) | (m_palette_bank << 6);
		int flipx = ~attr[offs] & 1;
		int flipy = ~attr[offs] & 2;

		gfx->transmask(bitmap, cliprect,
			code, color,
			flipx, flipy,
			sx, sy,
			m_palette->transpen_mask(*gfx, color, 0));

		// wraparound
		gfx->transmask(bitmap, cliprect,
			code, color,
			flipx, flipy,
			sx + 256, sy,
			m_palette->transpen_mask(*gfx, color, 0));
	}
}

void exctsccr_state::exctsccr_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element* const gfx1 = m_gfxdecode->gfx(1);
	gfx_element* const gfx2 = m_gfxdecode->gfx(2);
	assert (m_spriteram.bytes() == 0x10);

	for (int offs = 0x10-2; offs >= 0; offs -= 2)
	{
		// spriteram holds x/y data
		int sx = m_spriteram[offs + 1] - 16;
		int sy = 255 - m_spriteram[offs];

		// attribute data is from videoram
		int code = (m_vram[offs] >> 2 & 0x3f) | (m_vram[offs + 1] << 2 & 0x40);
		int flipx = ~m_vram[offs] & 1;
		int flipy = ~m_vram[offs] & 2;
		int color = m_vram[offs + 1] & 0xf;

		gfx1->transpen(bitmap, cliprect,
			code, color,
			flipx, flipy,
			sx, sy, 0);
	}

	for (int offs = 0x10-2; offs >= 0; offs -= 2)
	{
		// spriteram2 holds x/y data
		int sx = m_spriteram2[offs + 1] - 16;
		int sy = 255 - m_spriteram2[offs];

		// attribute data is from mainram
		int code = m_mainram[offs] >> 2 & 0x3f;
		int flipx = ~m_mainram[offs] & 1;
		int flipy = ~m_mainram[offs] & 2;
		int color = m_mainram[offs + 1] & 0xf;

		gfx2->transmask(bitmap, cliprect,
			code, color,
			flipx, flipy,
			sx, sy,
			m_palette->transpen_mask(*gfx2, color, 0x10));
	}
}


uint32_t champbas_state::screen_update_champbas(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	champbas_draw_sprites(bitmap, cliprect);
	return 0;
}

uint32_t exctsccr_state::screen_update_exctsccr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	exctsccr_draw_sprites(bitmap, cliprect);
	return 0;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

int champbas_state::watchdog_bit2()
{
	return (0x10 - m_watchdog->get_vblank_counter()) >> 2 & 1;
}

void champbas_state::irq_enable_w(int state)
{
	m_irq_mask = state;

	if (!m_irq_mask)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(exctsccr_state::exctsccr_sound_irq)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

void champbas_state::tilemap_w(offs_t offset, uint8_t data)
{
	m_vram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void champbas_state::gfxbank_w(int state)
{
	m_gfx_bank = state;
	m_bg_tilemap->mark_all_dirty();
}

void champbas_state::palette_bank_w(int state)
{
	m_palette_bank = state;
	m_bg_tilemap->set_palette_offset(m_palette_bank << 8);
}



/*************************************
 *
 *  Protection handling
 *
 *************************************/

/* champbja another protection */
uint8_t champbas_state::champbja_protection_r(offs_t offset)
{
	uint8_t data = 0;
	/*
	(68BA) & 0x99 == 0x00
	(6867) & 0x99 == 0x99
	(68AB) & 0x80 == 0x80
	(6854) & 0x99 == 0x19

	BA 1011_1010
	00 0--0_0--0

	54 0101_0100
	19 0--1_1--1

	67 0110_0111
	99 1--1_1--1

	AB 1010_1011
	80 1--0_0--0
	*/

	/* bit7 =  bit0 */
	if ((offset & 0x01))
		data |= 0x80;

	/* bit4,3,0 =  bit6 */
	if ((offset & 0x40))
		data |= 0x19;

	return data;
}



/*************************************
 *
 *  Address maps
 *
 *************************************/

// maincpu

// base map
void champbas_state::champbas_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x7000, 0x7001).mirror(0x0ffe).w("ay1", FUNC(ay8910_device::data_address_w));
	map(0x8000, 0x87ff).ram().w(FUNC(champbas_state::tilemap_w)).share("vram");
	map(0x8800, 0x8fff).ram().share("mainram");

	map(0xa000, 0xa000).portr("P1");
	map(0xa040, 0xa040).portr("P2");
	map(0xa080, 0xa080).mirror(0x0020).portr("DSW");
	map(0xa0c0, 0xa0c0).portr("SYSTEM");

	map(0xa000, 0xa007).w(m_mainlatch, FUNC(ls259_device::write_d0));

	map(0xa060, 0xa06f).writeonly().share("spriteram");
	map(0xa080, 0xa080).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xa0c0, 0xa0c0).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
}

// base map + ALPHA-8x0x protection
void champbas_state::champbasj_map(address_map &map)
{
	champbas_map(map);
	map(0x6000, 0x63ff).rw(m_alpha_8201, FUNC(alpha_8201_device::ext_ram_r), FUNC(alpha_8201_device::ext_ram_w));
}

// different protection for champbasja
void champbas_state::champbasja_map(address_map &map)
{
	champbas_map(map);
	map(0x6000, 0x63ff).ram();
	map(0x6800, 0x68ff).r(FUNC(champbas_state::champbja_protection_r));
}

// champbasjb appears to have no protection
void champbas_state::champbasjb_map(address_map &map)
{
	champbas_map(map);
	map(0x6000, 0x63ff).ram();
}

// champbb2
void champbas_state::champbb2_map(address_map &map)
{
	champbasj_map(map);
	map(0x7800, 0x7fff).rom();
}

// champbb2j appears to have AY select inverted
void champbas_state::champbb2j_map(address_map &map)
{
	champbb2_map(map);
	map(0x7000, 0x7001).mirror(0x0ffe).w("ay1", FUNC(ay8910_device::address_data_w));
}

void champbas_state::tbasebal_map(address_map &map)
{
	champbas_map(map);
	map(0x6000, 0x63ff).ram();
	map(0x6800, 0x6fff).rom();
	map(0x7800, 0x7fff).rom();
}

// more sprites in exctsccr
void exctsccr_state::exctsccr_map(address_map &map)
{
	champbasj_map(map);
	map(0x7000, 0x7001).unmaprw(); // aysnd is controlled by audiocpu
	map(0x7c00, 0x7fff).ram();
	map(0xa040, 0xa04f).writeonly().share("spriteram2");
}

// exctsccrb
void exctsccr_state::exctsccrb_map(address_map &map)
{
	champbasj_map(map);
	map(0xa040, 0xa04f).writeonly().share("spriteram2");
}


// audiocpu

// champbas/champbb2 (note: talbot doesn't have audiocpu)
void champbas_state::champbas_sound_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x6000).mirror(0x1fff).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x8000, 0x8000).mirror(0x1fff).nopw(); // 4-bit return code to main CPU (not used)
	map(0xa000, 0xa000).mirror(0x1fff).w("soundlatch", FUNC(generic_latch_8_device::clear_w));
	map(0xc000, 0xc000).mirror(0x1fff).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xe000, 0xe3ff).mirror(0x1c00).ram();
}

// exctsccr
void exctsccr_state::exctsccr_sound_map(address_map &map)
{
	map(0x0000, 0x8fff).rom();
	map(0xa000, 0xa7ff).ram();
	map(0xc008, 0xc008).w("dac1", FUNC(dac_byte_interface::data_w));
	map(0xc009, 0xc009).w("dac2", FUNC(dac_byte_interface::data_w));
	map(0xc00c, 0xc00c).w("soundlatch", FUNC(generic_latch_8_device::clear_w));
	map(0xc00d, 0xc00d).r("soundlatch", FUNC(generic_latch_8_device::read));
//  map(0xc00f, 0xc00f).nopw(); // ?
}

void exctsccr_state::exctsccr_sound_io_map(address_map &map)
{
	map.global_mask(0x00ff);
	map(0x82, 0x83).w("ay1", FUNC(ay8910_device::data_address_w));
	map(0x86, 0x87).w("ay2", FUNC(ay8910_device::data_address_w));
	map(0x8a, 0x8b).w("ay3", FUNC(ay8910_device::data_address_w));
	map(0x8e, 0x8f).w("ay4", FUNC(ay8910_device::data_address_w));
}

void exctsccr_state::exctscc2_sound_io_map(address_map &map)
{
	map.global_mask(0x00ff);
	map(0x8a, 0x8b).w("ay1", FUNC(ay8910_device::data_address_w));
	map(0x8e, 0x8f).w("ay2", FUNC(ay8910_device::data_address_w));
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( talbot )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPNAME( 0x30, 0x30, "Rabbits to Capture" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x30, "8" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(champbas_state, watchdog_bit2) // bit 2 of the watchdog counter

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( champbas )
	PORT_INCLUDE( talbot )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // throw (red)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) // changes (blue)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) // steal (yellow)

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL // steal (yellow)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL // changes (blue)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL // throw (red)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, "A 2/1 B 3/2" )
	PORT_DIPSETTING(    0x02, "A 1/1 B 2/1")
	PORT_DIPSETTING(    0x01, "A 1/2 B 1/6" )
	PORT_DIPSETTING(    0x00, "A 1/3 B 1/6")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ))
	PORT_DIPUNKNOWN( 0x40, 0x00 )
INPUT_PORTS_END

static INPUT_PORTS_START( exctsccr )
	PORT_INCLUDE( talbot )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, "A 1C/1C B 3C/1C" )
	PORT_DIPSETTING(    0x01, "A 1C/2C B 1C/4C" )
	PORT_DIPSETTING(    0x00, "A 1C/3C B 1C/6C" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Game_Time ) )
	PORT_DIPSETTING(    0x20, "1 Min." )
	PORT_DIPSETTING(    0x00, "2 Min." )
	PORT_DIPSETTING(    0x60, "3 Min." )
	PORT_DIPSETTING(    0x40, "4 Min." )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8,1), STEP4(0,1) },
	{ STEP8(0,8) },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1), STEP4(0,1) },
	{ STEP8(0,8), STEP8(32*8,8) },
	64*8
};

static GFXDECODE_START( gfx_talbot )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x100, 0x100>>2 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0x000, 0x100>>2 )
GFXDECODE_END

static GFXDECODE_START( gfx_champbas )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 0x200>>2 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 0x200>>2 )
GFXDECODE_END


static const gfx_layout charlayout_3bpp =
{
	8,8,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2)+4, 0, 4 },
	{ STEP4(8*8,1), STEP4(0,1) },
	{ STEP8(0,8) },
	16*8
};

static const gfx_layout spritelayout_3bpp =
{
	16,16,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2)+4, 0, 4 },
	{ STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1), STEP4(0,1) },
	{ STEP8(0,8), STEP8(32*8,8) },
	64*8
};

static const gfx_layout spritelayout_4bpp =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1), STEP4(0,1) },
	{ STEP8(0,8), STEP8(32*8,8) },
	64*8
};

static GFXDECODE_START( gfx_exctsccr )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_3bpp,   0x000, 0x080>>3 ) /* chars */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout_3bpp, 0x080, 0x080>>3 ) /* sprites */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout_4bpp, 0x100, 0x100>>4 ) /* sprites */
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void champbas_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_gfx_bank));
}

void champbas_state::vblank_irq(int state)
{
	if (state && m_irq_mask)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}


void champbas_state::talbot(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(18'432'000)/6);
	m_maincpu->set_addrmap(AS_PROGRAM, &champbas_state::champbasj_map);

	LS259(config, m_mainlatch);
	m_mainlatch->q_out_cb<0>().set(FUNC(champbas_state::irq_enable_w));
	m_mainlatch->q_out_cb<1>().set_nop(); // !WORK board output (no use?)
	m_mainlatch->q_out_cb<2>().set_nop(); // no gfxbank
	m_mainlatch->q_out_cb<3>().set(FUNC(champbas_state::flip_screen_set)).invert();
	m_mainlatch->q_out_cb<4>().set_nop(); // no palettebank
	m_mainlatch->q_out_cb<5>().set_nop(); // n.c.
	m_mainlatch->q_out_cb<6>().set(m_alpha_8201, FUNC(alpha_8201_device::mcu_start_w));
	m_mainlatch->q_out_cb<7>().set(m_alpha_8201, FUNC(alpha_8201_device::bus_dir_w));

	ALPHA_8201(config, m_alpha_8201, XTAL(18'432'000)/6/8);
	config.set_perfect_quantum("alpha_8201:mcu");

	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count("screen", 0x10);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(champbas_state::screen_update_champbas));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(champbas_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_talbot);
	PALETTE(config, m_palette, FUNC(champbas_state::champbas_palette), 512, 32);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	AY8910(config, "ay1", XTAL(18'432'000)/12).add_route(ALL_OUTPUTS, "speaker", 0.5);
}


void champbas_state::champbas(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(18'432'000)/6);
	m_maincpu->set_addrmap(AS_PROGRAM, &champbas_state::champbas_map);

	LS259(config, m_mainlatch); // 9D; 8G on Champion Baseball II Double Board Configuration
	m_mainlatch->q_out_cb<0>().set(FUNC(champbas_state::irq_enable_w));
	m_mainlatch->q_out_cb<1>().set_nop(); // !WORK board output (no use?)
	m_mainlatch->q_out_cb<2>().set(FUNC(champbas_state::gfxbank_w));
	m_mainlatch->q_out_cb<3>().set(FUNC(champbas_state::flip_screen_set)).invert();
	m_mainlatch->q_out_cb<4>().set(FUNC(champbas_state::palette_bank_w));
	m_mainlatch->q_out_cb<5>().set_nop(); // n.c.
	m_mainlatch->q_out_cb<6>().set_nop(); // no MCU
	m_mainlatch->q_out_cb<7>().set_nop(); // no MCU

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(18'432'000)/6));
	audiocpu.set_addrmap(AS_PROGRAM, &champbas_state::champbas_sound_map);

	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count("screen", 0x10);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(champbas_state::screen_update_champbas));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(champbas_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_champbas);
	PALETTE(config, m_palette, FUNC(champbas_state::champbas_palette), 512, 32);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	AY8910(config, "ay1", XTAL(18'432'000)/12).add_route(ALL_OUTPUTS, "speaker", 0.3);

	DAC_6BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.7); // unknown DAC
}

void champbas_state::champbasj(machine_config &config)
{
	champbas(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &champbas_state::champbasj_map);

	m_mainlatch->q_out_cb<6>().set(m_alpha_8201, FUNC(alpha_8201_device::mcu_start_w));
	m_mainlatch->q_out_cb<7>().set(m_alpha_8201, FUNC(alpha_8201_device::bus_dir_w));

	ALPHA_8201(config, m_alpha_8201, XTAL(18'432'000)/6/8); // note: 8302 rom on champbb2 (same device!)
	config.set_perfect_quantum("alpha_8201:mcu");
}


void champbas_state::champbasja(machine_config &config)
{
	champbas(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &champbas_state::champbasja_map);
}

void champbas_state::champbasjb(machine_config &config)
{
	champbas(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &champbas_state::champbasjb_map);
}

void champbas_state::champbb2(machine_config &config)
{
	champbasj(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &champbas_state::champbb2_map);
}

void champbas_state::champbb2j(machine_config &config)
{
	champbb2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &champbas_state::champbb2j_map);
}


void champbas_state::tbasebal(machine_config &config)
{
	champbas(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &champbas_state::tbasebal_map);

	m_mainlatch->q_out_cb<6>().set([this](int state) { logerror("%s latch bit 6 w: %d\n", machine().describe_context(), state); }); // to M68705? the code here seems the same as champbb2
	m_mainlatch->q_out_cb<7>().set([this](int state) { logerror("%s latch bit 7 w: %d\n", machine().describe_context(), state); }); // "

	m68705p_device &mcu(M68705P3(config, "mcu", XTAL(18'432'000) / 6)); // ?Mhz
	mcu.porta_r().set([this]() { logerror("%s MCU port A r\n", machine().describe_context()); return 0xff; });
	mcu.porta_w().set([this](uint8_t data) { logerror("%s MCU port A w: %02X\n", machine().describe_context(), data); });
	mcu.portb_r().set([this]() { logerror("%s MCU port B r\n", machine().describe_context()); return 0xff; });
	mcu.portb_w().set([this](uint8_t data) { logerror("%s MCU port B w: %02X\n", machine().describe_context(), data); });
	mcu.portc_r().set([this]() { logerror("%s MCU port C r\n", machine().describe_context()); return 0xff; });
	mcu.portc_w().set([this](uint8_t data) { logerror("%s MCU port C w: %02X\n", machine().describe_context(), data); });
}



void exctsccr_state::exctsccr(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(18'432'000)/6);
	m_maincpu->set_addrmap(AS_PROGRAM, &exctsccr_state::exctsccr_map);

	LS259(config, m_mainlatch);
	m_mainlatch->q_out_cb<0>().set(FUNC(exctsccr_state::irq_enable_w));
	m_mainlatch->q_out_cb<1>().set_nop(); // !WORK board output (no use?)
	m_mainlatch->q_out_cb<2>().set(FUNC(exctsccr_state::gfxbank_w));
	m_mainlatch->q_out_cb<3>().set(FUNC(exctsccr_state::flip_screen_set)).invert();
	m_mainlatch->q_out_cb<4>().set_nop(); // no palettebank
	m_mainlatch->q_out_cb<5>().set_nop(); // n.c.
	m_mainlatch->q_out_cb<6>().set(m_alpha_8201, FUNC(alpha_8201_device::mcu_start_w));
	m_mainlatch->q_out_cb<7>().set(m_alpha_8201, FUNC(alpha_8201_device::bus_dir_w));

	Z80(config, m_audiocpu, XTAL(14'318'181)/4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &exctsccr_state::exctsccr_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &exctsccr_state::exctsccr_sound_io_map);
	m_audiocpu->set_periodic_int(FUNC(exctsccr_state::nmi_line_pulse), attotime::from_hz(4000)); // 4 kHz, updates the dac

	timer_device &exc_snd_irq(TIMER(config, "exc_snd_irq"));
	exc_snd_irq.configure_periodic(FUNC(exctsccr_state::exctsccr_sound_irq), attotime::from_hz(75)); // irq source unknown, determines music tempo
	exc_snd_irq.set_start_delay(attotime::from_hz(75));

	ALPHA_8201(config, m_alpha_8201, XTAL(18'432'000)/6/8); // note: 8302 rom, or 8303 on exctscc2 (same device!)
	config.set_perfect_quantum("alpha_8201:mcu");

	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count("screen", 0x10);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60.54);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(exctsccr_state::screen_update_exctsccr));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(exctsccr_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_exctsccr);
	PALETTE(config, m_palette, FUNC(exctsccr_state::exctsccr_palette), 512, 32);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	/* AY (melody) clock is specified by a VR (0.9 - 3.9 MHz) */
	AY8910(config, "ay1", 1940000).add_route(ALL_OUTPUTS, "speaker", 0.08); /* VR has a factory mark and this is the value read */

	AY8910(config, "ay2", XTAL(14'318'181)/8).add_route(ALL_OUTPUTS, "speaker", 0.08);

	AY8910(config, "ay3", XTAL(14'318'181)/8).add_route(ALL_OUTPUTS, "speaker", 0.08);

	AY8910(config, "ay4", XTAL(14'318'181)/8).add_route(ALL_OUTPUTS, "speaker", 0.08);

	DAC_6BIT_R2R(config, "dac1", 0).add_route(ALL_OUTPUTS, "speaker", 0.3); // unknown DAC
	DAC_6BIT_R2R(config, "dac2", 0).add_route(ALL_OUTPUTS, "speaker", 0.3); // unknown DAC
}

void exctsccr_state::exctscc2(machine_config &config)
{
	exctsccr(config);

	m_audiocpu->set_addrmap(AS_IO, &exctsccr_state::exctscc2_sound_io_map);

	subdevice<ay8910_device>("ay1")->set_clock(XTAL(14'318'181)/8); // measured on PCB

	// Exciting Soccer II only has two AYs
	config.device_remove("ay3");
	config.device_remove("ay4");
}

/* Bootleg running on a modified Champion Baseball board */
void exctsccr_state::exctsccrb(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(18'432'000)/6);
	m_maincpu->set_addrmap(AS_PROGRAM, &exctsccr_state::exctsccrb_map);

	LS259(config, m_mainlatch);
	m_mainlatch->q_out_cb<0>().set(FUNC(exctsccr_state::irq_enable_w));
	m_mainlatch->q_out_cb<1>().set_nop(); // !WORK board output (no use?)
	m_mainlatch->q_out_cb<2>().set(FUNC(exctsccr_state::gfxbank_w));
	m_mainlatch->q_out_cb<3>().set(FUNC(exctsccr_state::flip_screen_set)).invert();
	m_mainlatch->q_out_cb<4>().set_nop(); // no palettebank
	m_mainlatch->q_out_cb<5>().set_nop(); // n.c.
	m_mainlatch->q_out_cb<6>().set(m_alpha_8201, FUNC(alpha_8201_device::mcu_start_w));
	m_mainlatch->q_out_cb<7>().set(m_alpha_8201, FUNC(alpha_8201_device::bus_dir_w));

	Z80(config, m_audiocpu, XTAL(18'432'000)/6);
	m_audiocpu->set_addrmap(AS_PROGRAM, &exctsccr_state::champbas_sound_map);

	ALPHA_8201(config, m_alpha_8201, XTAL(18'432'000)/6/8); // champbasj 8201 on pcb, though unused
	config.set_perfect_quantum("alpha_8201:mcu");

	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count("screen", 0x10);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(exctsccr_state::screen_update_exctsccr));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(exctsccr_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_exctsccr);
	PALETTE(config, m_palette, FUNC(exctsccr_state::exctsccr_palette), 512, 32);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	AY8910(config, "ay1", XTAL(18'432'000)/12).add_route(ALL_OUTPUTS, "speaker", 0.3);

	DAC_6BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.7); // unknown DAC
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( talbot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11.10g", 0x0000, 0x1000, CRC(0368607d) SHA1(275a29fb018bd327e64cf4fcc04590099c90290a) )
	ROM_LOAD( "12.11g", 0x1000, 0x1000, CRC(400e633b) SHA1(8d76df34174286e2b0c9341bbc141c9e77533f06) )
	ROM_LOAD( "13.10h", 0x2000, 0x1000, CRC(be575d9e) SHA1(17d3bbdc755920b5a6e1e81cbb7d51be20257ff1) )
	ROM_LOAD( "14.11h", 0x3000, 0x1000, CRC(56464614) SHA1(21cfcf3212e0a74c695ce1d6412d630a7141b2c9) )
	ROM_LOAD( "15.10i", 0x4000, 0x1000, CRC(0225b7ef) SHA1(9adee4831eb633b0a31580596205a655df94c2b2) )
	ROM_LOAD( "16.11i", 0x5000, 0x1000, CRC(1612adf5) SHA1(9adeb21d5d1692f6e31460062f03f2008076b307) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8201_44801a75_2f25.bin", 0x0000, 0x2000, CRC(b77931ac) SHA1(405b02585e80d95a2821455538c5c2c31ce262d1) )

	ROM_REGION( 0x1000, "gfx1", 0 ) // chars
	ROM_LOAD( "7.6a", 0x0000, 0x1000, CRC(bde14194) SHA1(f8f569342a3094eb5450a30b8ab87901b98e6061) )

	ROM_REGION( 0x1000, "gfx2", 0 ) // sprites
	ROM_LOAD( "8.6b", 0x0000, 0x1000, CRC(ddcd227a) SHA1(c44de36311cd173afb3eebf8487305b06e069c0f) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "mb7051.7h", 0x0000, 0x0020, CRC(7a153c60) SHA1(4b147c63e467cca7359acb5f3652ed9db9a36cc8) )
	ROM_LOAD( "mb7052.5e", 0x0020, 0x0100, CRC(a3189986) SHA1(f113c1253ba2f8f213c600e93a39c0957a933306) )
ROM_END

ROM_START( champbas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "champbb.1", 0x0000, 0x2000, CRC(218de21e) SHA1(7577fd04bdda4666c017f3b36e81ec23bcddd845) )
	ROM_LOAD( "champbb.2", 0x2000, 0x2000, CRC(5ddd872e) SHA1(68e21572e27707c991180b1bd0a6b31f7b64abf6) )
	ROM_LOAD( "champbb.3", 0x4000, 0x2000, CRC(f39a7046) SHA1(3097bffe84ac74ce9e6481028a0ebbe8b1d6eaf9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "champbb.6", 0x0000, 0x2000, CRC(26ab3e16) SHA1(019b9d34233a6b7a53e204154b782ceb42915d2b) )
	ROM_LOAD( "champbb.7", 0x2000, 0x2000, CRC(7c01715f) SHA1(b15b2001b8c110f2599eee3aeed79f67686ebd7e) )
	ROM_LOAD( "champbb.8", 0x4000, 0x2000, CRC(3c911786) SHA1(eea0c467e213d237b5bb9d04b19a418d6090c2dc) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "champbb.4", 0x0000, 0x2000, CRC(1930fb52) SHA1(cae0b2701c2b53b79e9df3a7496442ba3472e996) )

	ROM_REGION( 0x2000, "gfx2", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "champbb.5", 0x0000, 0x2000, CRC(a4cef5a1) SHA1(fa00ed0d075e00992a1ddce3c1327ed74770a735) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "champbb.pr2", 0x0000, 0x020, CRC(2585ffb0) SHA1(ce7f62f37955c2bbb4f82b139cc716978b084767) ) /* palette */
	ROM_LOAD( "champbb.pr1", 0x0020, 0x100, CRC(872dd450) SHA1(6c1e2c4a2fc072f4bf4996c731adb0b01b347506) ) /* look-up table */
ROM_END

ROM_START( champbasj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11.2e", 0x0000, 0x2000, CRC(e2dfc166) SHA1(482e084d7d21b1cf2d17431699e6bab4c4b6ac15) )
	ROM_LOAD( "12.2g", 0x2000, 0x2000, CRC(7b4e5faa) SHA1(b7201816a819ef313ddc81f312d26982b83ef1c7) )
	ROM_LOAD( "13.2h", 0x4000, 0x2000, CRC(b201e31f) SHA1(bba3b611ff60ad8d5dd8484df4cfc2026f4fd344) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "16.2k", 0x0000, 0x2000, CRC(24c482ee) SHA1(c25bdf77014e095fc11a9a6b17f16858f19db451) )
	ROM_LOAD( "17.2l", 0x2000, 0x2000, CRC(f10b148b) SHA1(d66516d509f6f16e51ee59d27c4867e276064c3f) )
	ROM_LOAD( "18.2n", 0x4000, 0x2000, CRC(2dc484dd) SHA1(28bd68c787d7e6989849ca52009948dbd5cdcc79) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8201_44801a75_2f25.bin", 0x0000, 0x2000, CRC(b77931ac) SHA1(405b02585e80d95a2821455538c5c2c31ce262d1) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "14.5e", 0x0000, 0x2000, CRC(1b8202b3) SHA1(889b77fc3d0cb029baf8c47be260f513f3ed59bd) )

	ROM_REGION( 0x2000, "gfx2", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "15.5g", 0x0000, 0x2000, CRC(a67c0c40) SHA1(3845839eff8c1624d26937f28ffde67a5fcb4805) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "1e.bpr", 0x0000, 0x0020, CRC(f5ce825e) SHA1(956f580840f1a7d24bfbd72b2929d14e9ee1b660) ) /* palette */
	ROM_LOAD( "5k.bpr", 0x0020, 0x0100, CRC(2e481ffa) SHA1(bc8979efd43bee8be0ce96ebdacc873a5821e06e) ) /* look-up table */
ROM_END

ROM_START( champbasja )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10", 0x0000, 0x2000, CRC(f7cdaf8e) SHA1(d4c840f2107394fadbcf822d64aaa381ac900367) )
	ROM_LOAD( "09", 0x2000, 0x2000, CRC(9d39e5b3) SHA1(11c1a1d2296c0bf16d7610eaa79b034bfd813740) )
	ROM_LOAD( "08", 0x4000, 0x2000, CRC(53468a0f) SHA1(d4b5ea48b27754eebe593c8b4fcf5bf117f27ae4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "16.2k", 0x0000, 0x2000, CRC(24c482ee) SHA1(c25bdf77014e095fc11a9a6b17f16858f19db451) )
	ROM_LOAD( "17.2l", 0x2000, 0x2000, CRC(f10b148b) SHA1(d66516d509f6f16e51ee59d27c4867e276064c3f) )
	ROM_LOAD( "18.2n", 0x4000, 0x2000, CRC(2dc484dd) SHA1(28bd68c787d7e6989849ca52009948dbd5cdcc79) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "14.5e", 0x0000, 0x2000, CRC(1b8202b3) SHA1(889b77fc3d0cb029baf8c47be260f513f3ed59bd) )

	ROM_REGION( 0x2000, "gfx2", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "15.5g", 0x0000, 0x2000, CRC(a67c0c40) SHA1(3845839eff8c1624d26937f28ffde67a5fcb4805) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "clr",    0x0000, 0x0020, CRC(8f989357) SHA1(d0916fb5ef4b43bdf84663cd403418ffc5e98c17) ) /* palette */
	ROM_LOAD( "5k.bpr", 0x0020, 0x0100, CRC(2e481ffa) SHA1(bc8979efd43bee8be0ce96ebdacc873a5821e06e) ) /* look-up table */
ROM_END

ROM_START( champbasjb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.2e",  0x0000, 0x2000, CRC(4dcf2e03) SHA1(2cdae2cc560d316bb651f8a92e4d6af6eaac8785) )
	ROM_LOAD( "2.2g",  0x2000, 0x2000, CRC(ccbd0eff) SHA1(5437e571b417fb162b36376fd26cab753ca178ff) )
	ROM_LOAD( "3.2h",  0x4000, 0x2000, CRC(4c7f1de4) SHA1(c5b4ad5f3e3f606e372fb5316ee875f8a299129c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "6.2k",  0x0000, 0x2000, CRC(24c482ee) SHA1(c25bdf77014e095fc11a9a6b17f16858f19db451) )
	ROM_LOAD( "7.2l",  0x2000, 0x2000, CRC(f10b148b) SHA1(d66516d509f6f16e51ee59d27c4867e276064c3f) )
	ROM_LOAD( "8.2n",  0x4000, 0x2000, CRC(2dc484dd) SHA1(28bd68c787d7e6989849ca52009948dbd5cdcc79) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "4.5e",  0x0000, 0x2000, CRC(1930fb52) SHA1(cae0b2701c2b53b79e9df3a7496442ba3472e996) )

	ROM_REGION( 0x2000, "gfx2", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "5.5g",  0x0000, 0x2000, CRC(a67c0c40) SHA1(3845839eff8c1624d26937f28ffde67a5fcb4805) )

	ROM_REGION( 0x0120, "proms", 0 ) // palette + table missing in set, taken from champbasj
	ROM_LOAD( "1e.bpr", 0x0000, 0x0020, BAD_DUMP CRC(f5ce825e) SHA1(956f580840f1a7d24bfbd72b2929d14e9ee1b660) ) /* palette */
	ROM_LOAD( "5k.bpr", 0x0020, 0x0100, BAD_DUMP CRC(2e481ffa) SHA1(bc8979efd43bee8be0ce96ebdacc873a5821e06e) ) /* look-up table */
ROM_END

ROM_START( champbb2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr5932", 0x0000, 0x2000, CRC(528e3c78) SHA1(ee300201580c1bace783f1340bd4f1ea2a00dffa) )
	ROM_LOAD( "epr5929", 0x2000, 0x2000, CRC(17b6057e) SHA1(67c5aed950acf4d045edf39019066af2896265e1) )
	ROM_LOAD( "epr5930", 0x4000, 0x2000, CRC(b6570a90) SHA1(5a2651aeac986000913b5854792b2d81df6b2fc6) )
	ROM_LOAD( "epr5931", 0x7800, 0x0800, CRC(0592434d) SHA1(a7f61546c39ffdbff46c4db485c9b3f6eefcf1ac) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr5933", 0x0000, 0x2000, CRC(26ab3e16) SHA1(019b9d34233a6b7a53e204154b782ceb42915d2b) )
	ROM_LOAD( "epr5934", 0x2000, 0x2000, CRC(7c01715f) SHA1(b15b2001b8c110f2599eee3aeed79f67686ebd7e) )
	ROM_LOAD( "epr5935", 0x4000, 0x2000, CRC(3c911786) SHA1(eea0c467e213d237b5bb9d04b19a418d6090c2dc) )

	// the pcb has a 8302 on it, though only the 8201 instructions are used
	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8302_44801b35.bin", 0x0000, 0x2000, CRC(edabac6c) SHA1(eaf1c51b63023256df526b0d3fd53cffc919c901) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "epr5936", 0x0000, 0x2000, CRC(c4a4df75) SHA1(7b85dbf405697b0b8881f910c08f6db6c828b19a) )

	ROM_REGION( 0x2000, "gfx2", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "epr5937", 0x0000, 0x2000, CRC(5c80ec42) SHA1(9b79737577e48a6b2ec20ce145252545955e82c3) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "pr5957", 0x0000, 0x020, CRC(f5ce825e) SHA1(956f580840f1a7d24bfbd72b2929d14e9ee1b660) ) /* palette */
	ROM_LOAD( "pr5956", 0x0020, 0x100, CRC(872dd450) SHA1(6c1e2c4a2fc072f4bf4996c731adb0b01b347506) ) /* look-up table */
ROM_END

ROM_START( tbasebal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.2e.2764",     0x0000, 0x2000, CRC(9b75b44d) SHA1(35b67638a5e48cbe999907e3c9c3a33da9d76bba) )
	ROM_LOAD( "2_p9.2g.2764",  0x2000, 0x2000, CRC(736a1b62) SHA1(24c2d57506754ca789b378a595c03b7591eb5b5c) )
	ROM_LOAD( "3.2h.2764",     0x4000, 0x2000, CRC(cf5f28cb) SHA1(d553f2085c9c8c77b241b4239cc1ad1764b490d0) )
	ROM_LOAD( "ic2.2764",      0x6000, 0x2000, CRC(aacb9647) SHA1(4f3830ffc18f8578064babbc638efb5a59ef1a3d) ) // on sub-board with MCU

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "6.2k.2764", 0x0000, 0x2000, CRC(24c482ee) SHA1(c25bdf77014e095fc11a9a6b17f16858f19db451) )
	ROM_LOAD( "7.2l.2764", 0x2000, 0x2000, CRC(f10b148b) SHA1(d66516d509f6f16e51ee59d27c4867e276064c3f) )
	ROM_LOAD( "8.2n.2764", 0x4000, 0x2000, CRC(2dc484dd) SHA1(28bd68c787d7e6989849ca52009948dbd5cdcc79) )

	ROM_REGION( 0x0800, "mcu", 0 ) /* MC68705P5 */
	ROM_LOAD( "ic3_mc68705p3_rom.bin",    0x0000, 0x0800, CRC(6b477f5f) SHA1(c773a4bed22106346fb2347b6d5a32958be8213c) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "4.5e.2764", 0x0000, 0x2000, CRC(c4a4df75) SHA1(7b85dbf405697b0b8881f910c08f6db6c828b19a) )

	ROM_REGION( 0x2000, "gfx2", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "5.5g.2764", 0x0000, 0x2000, CRC(5c80ec42) SHA1(9b79737577e48a6b2ec20ce145252545955e82c3) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "1e.bpr",    0x0000, 0x0020, CRC(f5ce825e) SHA1(956f580840f1a7d24bfbd72b2929d14e9ee1b660) ) /* palette - wasn't dumped from this set, could be wrong */
	ROM_LOAD( "5k.82s129", 0x0020, 0x0100, CRC(2e481ffa) SHA1(bc8979efd43bee8be0ce96ebdacc873a5821e06e) ) /* look-up table */
ROM_END

ROM_START( champbb2j )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.10g", 0x0000, 0x2000, CRC(c76056d5) SHA1(2aaec9ede0f85bf67f6cf04fabdba66f7e0d6004) )
	ROM_LOAD( "2.10h", 0x2000, 0x2000, CRC(7a1ea3ea) SHA1(ce2f61be4cc7cd7b739d89a4838408912c2e2784) )
	ROM_LOAD( "3.10i", 0x4000, 0x2000, CRC(4b2f6ac4) SHA1(367b25665fc37140dc38be8bb525859c20bd2529) )
	ROM_LOAD( "0.11g", 0x7800, 0x0800, CRC(be0e180d) SHA1(7a8915e00920faa08344d752404a6f98d8fb303b) )

	/* not in this set, but probably the same */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "6.15c", 0x0000, 0x2000, CRC(24c482ee) SHA1(c25bdf77014e095fc11a9a6b17f16858f19db451) )
	ROM_LOAD( "7.15d", 0x2000, 0x2000, CRC(f10b148b) SHA1(d66516d509f6f16e51ee59d27c4867e276064c3f) )
	ROM_LOAD( "8.15e", 0x4000, 0x2000, CRC(2dc484dd) SHA1(28bd68c787d7e6989849ca52009948dbd5cdcc79) )

	// the pcb has a 8302 on it, though only the 8201 instructions are used
	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8302_44801b35.bin", 0x0000, 0x2000, CRC(edabac6c) SHA1(eaf1c51b63023256df526b0d3fd53cffc919c901) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "4.6a", 0x0000, 0x2000, CRC(c4a4df75) SHA1(7b85dbf405697b0b8881f910c08f6db6c828b19a) )

	ROM_REGION( 0x2000, "gfx2", 0 ) // chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "5.6b", 0x0000, 0x2000, CRC(5c80ec42) SHA1(9b79737577e48a6b2ec20ce145252545955e82c3) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "bpr.7h", 0x0000, 0x020, CRC(500db3d9) SHA1(7317ba2c6ffef3561acb3b2adb811def846756cf) ) /* palette */
	ROM_LOAD( "bpr.5e", 0x0020, 0x100, CRC(2e481ffa) SHA1(bc8979efd43bee8be0ce96ebdacc873a5821e06e) ) /* look-up table */
ROM_END

ROM_START( exctsccr ) /* Teams: ITA AUS GBR FRA FRG BRA */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1_g10.bin",    0x0000, 0x2000, CRC(aa68df66) SHA1(f10cac5a4c5aad1e1eb8835174dc8d517bb2921a) )
	ROM_LOAD( "2_h10.bin",    0x2000, 0x2000, CRC(2d8f8326) SHA1(8809e7b081fa2a1966cb51ac969fd7b468d35be0) )
	ROM_LOAD( "3_j10.bin",    0x4000, 0x2000, CRC(dce4a04d) SHA1(9c015e4597ec8921bea213d9841fc69c776a4e6d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "0_h6.bin",     0x0000, 0x2000, CRC(3babbd6b) SHA1(b81bd47c4449f4f21f2d55d01eb9cb6db10664c7) )
	ROM_LOAD( "9_f6.bin",     0x2000, 0x2000, CRC(639998f5) SHA1(c4ff5e5e75d53dea38449f323186d08d5b57bf90) )
	ROM_LOAD( "8_d6.bin",     0x4000, 0x2000, CRC(88651ee1) SHA1(2052e1b3f9784439369f464e31f4a2b0d1bb0565) )
	ROM_LOAD( "7_c6.bin",     0x6000, 0x2000, CRC(6d51521e) SHA1(2809bd2e61f40dcd31d43c62520982bdcfb0a865) )
	ROM_LOAD( "1_a6.bin",     0x8000, 0x1000, CRC(20f2207e) SHA1(b1ed2237d0bd50ddbe593fd2fbff9f1d67c1eb11) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8302_44801b35.bin", 0x0000, 0x2000, CRC(edabac6c) SHA1(eaf1c51b63023256df526b0d3fd53cffc919c901) )

	ROM_REGION( 0x04000, "gfx1", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "4_a5.bin",     0x0000, 0x2000, CRC(c342229b) SHA1(a989d6c12521c77882a7e17d4d80afe7eae05906) ) /* planes 0,1 */
	ROM_LOAD( "6_c5.bin",     0x2000, 0x2000, CRC(eda40e32) SHA1(6c08fd4f4fb35fd354d02e04548e960c545f6a88) ) /* plane 3 */

	ROM_REGION( 0x04000, "gfx2", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "5_b5.bin",     0x0000, 0x2000, CRC(35f4f8c9) SHA1(cdf5bbfea9abdd338938e5f4499d2d71ce3c6237) ) /* planes 0,1 */

	ROM_REGION( 0x02000, "gfx3", 0 )    // 4bpp sprites
	ROM_LOAD( "2.5k",     0x0000, 0x1000, CRC(7f9cace2) SHA1(bf05a31716f3ca1c2fd1034cd1f39e2d21cdaed3) )
	ROM_LOAD( "3.5l",     0x1000, 0x1000, CRC(db2d9e0d) SHA1(6ec09a47f7aea6bf31eb0ee78f44012f4d92de8a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom3.k5",     0x0020, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
	ROM_LOAD( "prom2.8r",     0x0120, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
ROM_END

// CPU BOARD NO. 58AS50-1, DISPLAY BOARD NO. 58AS51-1, MUSIC & VOICE BOARD NO. 59MC02
ROM_START( exctsccra ) /* Teams: ITA AUS GBR FRA FRG BRA */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1_g10.bin",    0x0000, 0x2000, CRC(aa68df66) SHA1(f10cac5a4c5aad1e1eb8835174dc8d517bb2921a) )
	ROM_LOAD( "2_h10.bin",    0x2000, 0x2000, CRC(2d8f8326) SHA1(8809e7b081fa2a1966cb51ac969fd7b468d35be0) )
	ROM_LOAD( "3_j10.bin",    0x4000, 0x2000, CRC(dce4a04d) SHA1(9c015e4597ec8921bea213d9841fc69c776a4e6d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "exctsccc.000", 0x0000, 0x2000, CRC(642fc42f) SHA1(cfc849d18e347e3e23fc31c1ce7f2580d5d9b2b0) )
	ROM_LOAD( "exctsccc.009", 0x2000, 0x2000, CRC(d88b3236) SHA1(80f083fb15243e9e68978677caed8aee8e3109a0) )
	ROM_LOAD( "8_d6.bin",     0x4000, 0x2000, CRC(88651ee1) SHA1(2052e1b3f9784439369f464e31f4a2b0d1bb0565) )
	ROM_LOAD( "7_c6.bin",     0x6000, 0x2000, CRC(6d51521e) SHA1(2809bd2e61f40dcd31d43c62520982bdcfb0a865) )
	ROM_LOAD( "1_a6.bin",     0x8000, 0x1000, CRC(20f2207e) SHA1(b1ed2237d0bd50ddbe593fd2fbff9f1d67c1eb11) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8302_44801b35.bin", 0x0000, 0x2000, CRC(edabac6c) SHA1(eaf1c51b63023256df526b0d3fd53cffc919c901) )

	ROM_REGION( 0x04000, "gfx1", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "4_a5.bin",     0x0000, 0x2000, CRC(c342229b) SHA1(a989d6c12521c77882a7e17d4d80afe7eae05906) ) /* planes 0,1 */
	ROM_LOAD( "6_c5.bin",     0x2000, 0x2000, CRC(eda40e32) SHA1(6c08fd4f4fb35fd354d02e04548e960c545f6a88) ) /* plane 3 */

	ROM_REGION( 0x04000, "gfx2", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "5_b5.bin",     0x0000, 0x2000, CRC(35f4f8c9) SHA1(cdf5bbfea9abdd338938e5f4499d2d71ce3c6237) ) /* planes 0,1 */

	ROM_REGION( 0x02000, "gfx3", 0 )    // 4bpp sprites
	ROM_LOAD( "2.5k",     0x0000, 0x1000, CRC(7f9cace2) SHA1(bf05a31716f3ca1c2fd1034cd1f39e2d21cdaed3) )
	ROM_LOAD( "3.5l",     0x1000, 0x1000, CRC(db2d9e0d) SHA1(6ec09a47f7aea6bf31eb0ee78f44012f4d92de8a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom3.k5",     0x0020, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
	ROM_LOAD( "prom2.8r",     0x0120, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
ROM_END

ROM_START( exctsccru ) /* Teams: ITA USA GBR FRA FRG BRA */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vr1u.g10",    0x0000, 0x2000, CRC(ef39676d) SHA1(f7728d86b2c68bb93a0fcf02931cda8cb65e6d48) ) /* Team USA in place of Austria */
	ROM_LOAD( "vr2u.h10",    0x2000, 0x2000, CRC(37994b86) SHA1(681a27a009909cc8d26f8046c54532ec56145f97) )
	ROM_LOAD( "vr3u.j10",    0x4000, 0x2000, CRC(2ed3c6bb) SHA1(d3bad24cbbb34eb6c43cb603cbf66ab35be2c845) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "vr0u.6h",      0x0000, 0x2000, CRC(cbb035c6) SHA1(e89e69678335edf1cadc7b7949f8cfe47dfabc46) )
	ROM_LOAD( "9_f6.bin",     0x2000, 0x2000, CRC(639998f5) SHA1(c4ff5e5e75d53dea38449f323186d08d5b57bf90) )
	ROM_LOAD( "8_d6.bin",     0x4000, 0x2000, CRC(88651ee1) SHA1(2052e1b3f9784439369f464e31f4a2b0d1bb0565) )
	ROM_LOAD( "7_c6.bin",     0x6000, 0x2000, CRC(6d51521e) SHA1(2809bd2e61f40dcd31d43c62520982bdcfb0a865) )
	ROM_LOAD( "1_a6.bin",     0x8000, 0x1000, CRC(20f2207e) SHA1(b1ed2237d0bd50ddbe593fd2fbff9f1d67c1eb11) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8302_44801b35.bin", 0x0000, 0x2000, CRC(edabac6c) SHA1(eaf1c51b63023256df526b0d3fd53cffc919c901) )

	ROM_REGION( 0x04000, "gfx1", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "vr4u.a5",     0x0000, 0x2000, CRC(103bb739) SHA1(335d89b3a374daa3fd1bd3fd66a82e7310303051) ) /* planes 0,1 */
	ROM_LOAD( "vr6u.c5",     0x2000, 0x2000, CRC(a5b2b303) SHA1(0dd1912baa8236cba2baa4bc3d2955fd19617be9) ) /* plane 3 */

	ROM_REGION( 0x04000, "gfx2", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "5_b5.bin",     0x0000, 0x2000, CRC(35f4f8c9) SHA1(cdf5bbfea9abdd338938e5f4499d2d71ce3c6237) ) /* planes 0,1 */

	ROM_REGION( 0x02000, "gfx3", 0 )    // 4bpp sprites
	ROM_LOAD( "2.5k",     0x0000, 0x1000, CRC(7f9cace2) SHA1(bf05a31716f3ca1c2fd1034cd1f39e2d21cdaed3) )
	ROM_LOAD( "3.5l",     0x1000, 0x1000, CRC(db2d9e0d) SHA1(6ec09a47f7aea6bf31eb0ee78f44012f4d92de8a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom3.k5",     0x0020, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
	ROM_LOAD( "prom2.8r",     0x0120, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
ROM_END

ROM_START( exctsccrj ) /* Teams: JPN USA GBR FRA FRG BRA */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1_10g.bin",    0x0000, 0x2000, CRC(310298a2) SHA1(b05a697ec2ed1bf4947fcc5f823ed9cb8daeee15) ) /* Corrects "ENG" to GBR & "GFR" to FGR */
	ROM_LOAD( "2_10h.bin",    0x2000, 0x2000, CRC(030fd0b7) SHA1(a4c57c5eb1c76dc7e5d9be48036f21331f9529d9) )
	ROM_LOAD( "3_10j.bin",    0x4000, 0x2000, CRC(1a51ff1f) SHA1(2a657f95807bfbf172f7d22e20b9ce75f453d028) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "0_h6.bin",     0x0000, 0x2000, CRC(3babbd6b) SHA1(b81bd47c4449f4f21f2d55d01eb9cb6db10664c7) )
	ROM_LOAD( "9_f6.bin",     0x2000, 0x2000, CRC(639998f5) SHA1(c4ff5e5e75d53dea38449f323186d08d5b57bf90) )
	ROM_LOAD( "8_d6.bin",     0x4000, 0x2000, CRC(88651ee1) SHA1(2052e1b3f9784439369f464e31f4a2b0d1bb0565) )
	ROM_LOAD( "7_c6.bin",     0x6000, 0x2000, CRC(6d51521e) SHA1(2809bd2e61f40dcd31d43c62520982bdcfb0a865) )
	ROM_LOAD( "1_a6.bin",     0x8000, 0x1000, CRC(20f2207e) SHA1(b1ed2237d0bd50ddbe593fd2fbff9f1d67c1eb11) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8302_44801b35.bin", 0x0000, 0x2000, CRC(edabac6c) SHA1(eaf1c51b63023256df526b0d3fd53cffc919c901) )

	ROM_REGION( 0x04000, "gfx1", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "4_5a.bin",     0x0000, 0x2000, CRC(74cc71d6) SHA1(ff3d59845bc66ec3335eadf81d799a684182c66f) ) /* planes 0,1 */
	ROM_LOAD( "6_5c.bin",     0x2000, 0x2000, CRC(7c4cd1b6) SHA1(141e67fec9b6d6b4380cb941b4d79341787680e3) ) /* plane 3 */

	ROM_REGION( 0x04000, "gfx2", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "5_5b.bin",     0x0000, 0x2000, CRC(35f4f8c9) SHA1(cdf5bbfea9abdd338938e5f4499d2d71ce3c6237) ) /* planes 0,1 */

	ROM_REGION( 0x02000, "gfx3", 0 )    // 4bpp sprites
	ROM_LOAD( "2.5k",         0x0000, 0x1000, CRC(7f9cace2) SHA1(bf05a31716f3ca1c2fd1034cd1f39e2d21cdaed3) )
	ROM_LOAD( "3.5l",         0x1000, 0x1000, CRC(db2d9e0d) SHA1(6ec09a47f7aea6bf31eb0ee78f44012f4d92de8a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom3.k5",     0x0020, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
	ROM_LOAD( "prom2.8r",     0x0120, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
ROM_END

ROM_START( exctsccrjo ) /* Teams: JPN USA ENG FRA GFR BRA */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.10g",    0x0000, 0x2000, CRC(d1bfdf75) SHA1(a4a9bb340712401b1d24705c26d996a798776d4f) )
	ROM_LOAD( "2.10h",    0x2000, 0x2000, CRC(5c61f0fe) SHA1(8c6751b80f89d8744d3eaa2a6da2cafdde968ed2) )
	ROM_LOAD( "3.10j",    0x4000, 0x2000, CRC(8f213b10) SHA1(5bffaee2725fe34b0614fcf1b4dc1c9a2f2df36c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "0.6h",     0x0000, 0x2000, CRC(548b08a2) SHA1(4cdcc67e34e56cbac5d07e9603650073de0bb5d1) )
	ROM_LOAD( "9.f6",     0x2000, 0x2000, CRC(639998f5) SHA1(c4ff5e5e75d53dea38449f323186d08d5b57bf90) )
	ROM_LOAD( "8.6d",     0x4000, 0x2000, CRC(b6b209a5) SHA1(e49a0db65b29337ac6b919237067b1990f2233ab) )
	ROM_LOAD( "7.6c",     0x6000, 0x2000, CRC(8856452a) SHA1(4494c225c9df97da09c180caadb4dda49d0d5392) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8302_44801b35.bin", 0x0000, 0x2000, CRC(edabac6c) SHA1(eaf1c51b63023256df526b0d3fd53cffc919c901) )

	ROM_REGION( 0x04000, "gfx1", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "4.5a",     0x0000, 0x2000, CRC(c4259307) SHA1(7bd4e229a5e1a5136826a57aa61810fcdf9c5027) ) /* planes 0,1 */
	ROM_LOAD( "6.5c",     0x2000, 0x2000, CRC(cca53367) SHA1(f06ebf2ab8f8f10cfe118af490017972990e3073) ) /* plane 3 */

	ROM_REGION( 0x04000, "gfx2", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "5.5b",     0x0000, 0x2000, CRC(851d1a18) SHA1(2cfad530c8f9d95094fd0aacd2e0965b0300898c) ) /* planes 0,1 */

	ROM_REGION( 0x02000, "gfx3", 0 )    // 4bpp sprites
	ROM_LOAD( "2.5k",     0x0000, 0x1000, CRC(7f9cace2) SHA1(bf05a31716f3ca1c2fd1034cd1f39e2d21cdaed3) )
	ROM_LOAD( "3.5l",     0x1000, 0x1000, CRC(db2d9e0d) SHA1(6ec09a47f7aea6bf31eb0ee78f44012f4d92de8a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom3.k5",     0x0020, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
	ROM_LOAD( "prom2.8r",     0x0120, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
ROM_END

/*
The Kazutomi bootleg board is a conversion from Champion Baseball:
Alpha denshi co. LTD made in Japan
cpu board 58AS1
Display board 58AS2
Voice board 58AS3
KAZUTOMI board
1x Alpha8201 (CPU board)
1x AY-3-8910 (CPU board)
1x Sharp Z80ACPU (CPU board)
1x Sharp Z80ACPU (VOICE board)
3x 2764 (CPU board) (1-2-3)
3x 2764 (VOICE board) (a-b-c)
2x 2764 (DISPLAY board) (4-5)
1x 2764 (daughter board kazutomi) (6)
2x 2732 (daughter board kazutomi) (7-8)
*/
ROM_START( exctsccrb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "es-1.e2",      0x0000, 0x2000, CRC(997c6a82) SHA1(60fe27a12eedd22c775b7e65c5ba692cfcf5ac74) )
	ROM_LOAD( "es-2.g2",      0x2000, 0x2000, CRC(5c66e792) SHA1(f7a7f32806965fa926261217cee3159ccd198d49) )
	ROM_LOAD( "es-3.h2",      0x4000, 0x2000, CRC(e0d504c0) SHA1(d9a9f37b3a44a05a3f3389aa9617c419a2cee661) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* sound */
	ROM_LOAD( "es-a.k2",      0x0000, 0x2000, CRC(99e87b78) SHA1(f12006ff3f6f3c706e06288c97a1446141373432) )
	ROM_LOAD( "es-b.l2",      0x2000, 0x2000, CRC(8b3db794) SHA1(dbfed2357c7631bfca6bbd63a23617bc3abf6ca3) )
	ROM_LOAD( "es-c.m2",      0x4000, 0x2000, CRC(7bed2f81) SHA1(cbbb0480519cc04a99e8983228b18c9e49a9985d) )

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 )
	ROM_LOAD( "alpha-8201_44801a75_2f25.bin", 0x0000, 0x2000, CRC(b77931ac) SHA1(405b02585e80d95a2821455538c5c2c31ce262d1) )

	/* the national flags are wrong. This happens on the real board */
	ROM_REGION( 0x04000, "gfx1", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only chars
	ROM_LOAD( "4_a5.bin",     0x0000, 0x2000, CRC(c342229b) SHA1(a989d6c12521c77882a7e17d4d80afe7eae05906) ) /* planes 0,1 */
	ROM_LOAD( "6_c5.bin",     0x2000, 0x2000, CRC(eda40e32) SHA1(6c08fd4f4fb35fd354d02e04548e960c545f6a88) ) /* plane 3 */

	ROM_REGION( 0x04000, "gfx2", 0 )    // 3bpp chars + sprites: rearranged by DRIVER_INIT to leave only sprites
	ROM_LOAD( "5_b5.bin",     0x0000, 0x2000, CRC(35f4f8c9) SHA1(cdf5bbfea9abdd338938e5f4499d2d71ce3c6237) ) /* planes 0,1 */

	ROM_REGION( 0x02000, "gfx3", 0 )    // 4bpp sprites
	ROM_LOAD( "2_k5.bin",     0x0000, 0x1000, CRC(7f9cace2) SHA1(bf05a31716f3ca1c2fd1034cd1f39e2d21cdaed3) )
	ROM_LOAD( "3_l5.bin",     0x1000, 0x1000, CRC(db2d9e0d) SHA1(6ec09a47f7aea6bf31eb0ee78f44012f4d92de8a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom3.k5",     0x0020, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
	ROM_LOAD( "prom2.8r",     0x0120, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
ROM_END

ROM_START( exctscc2 ) // 2-PCB stack: CPU & SOUND BOARD + DISPLAY BOARD N. 58AS51-1
	ROM_REGION( 0x10000, "maincpu", 0 ) // on CPU & sound board
	ROM_LOAD( "eprom_1_vr_b_alpha_denshi.3j",      0x0000, 0x2000, CRC(c6115362) SHA1(6a258631abd72ef6b8d7968bb4b2bc88e89e597d) ) // B handwritten
	ROM_LOAD( "eprom_2_vr_alpha_denshi.3k",        0x2000, 0x2000, CRC(de36ba00) SHA1(0a0d92e710b8c749f145571bc8a204609456d19d) )
	ROM_LOAD( "eprom_3_vr_v_alpha_denshi.3l",      0x4000, 0x2000, CRC(1ddfdf65) SHA1(313d0a7f13fc2de15aa32492c38a59fbafad9f01) ) // V handwritten

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on CPU & sound board
	ROM_LOAD( "eprom_0_vr_alpha_denshi.7d",     0x0000, 0x2000, CRC(2c675a43) SHA1(aa0a8dbcae955e3da92c435202f2a1ed238c377e) ) // yes 0, not 10
	ROM_LOAD( "eprom_9_vr_alpha_denshi.7e",     0x2000, 0x2000, CRC(e571873d) SHA1(2dfff24f5dac86e92612f40cf3642005c7f36ad3) )
	ROM_LOAD( "eprom_8_vr_alpha_denshi.7f",     0x4000, 0x2000, CRC(88651ee1) SHA1(2052e1b3f9784439369f464e31f4a2b0d1bb0565) )
	ROM_LOAD( "eprom_7_vr_alpha_denshi.7h",     0x6000, 0x2000, CRC(6d51521e) SHA1(2809bd2e61f40dcd31d43c62520982bdcfb0a865) )
	ROM_LOAD( "eprom_1_vr_alpha_denshi.7k",     0x8000, 0x1000, CRC(20f2207e) SHA1(b1ed2237d0bd50ddbe593fd2fbff9f1d67c1eb11) ) // marked for a 2764 but populated with a 2732

	ROM_REGION( 0x2000, "alpha_8201:mcu", 0 ) // on CPU & sound board
	ROM_LOAD( "alpha-8303_44801b42.1d", 0x0000, 0x2000, CRC(66adcb37) SHA1(e1c72ecb161129dcbddc0b16dd90e716d0c79311) )

	ROM_REGION( 0x04000, "gfx1", 0 )    // 3bpp chars + sprites: rearranged by init_exctsccr() to leave only chars, on display board
	ROM_LOAD( "eprom_4_vr_alpha_denshi.5a",        0x0000, 0x2000, CRC(4ff1783d) SHA1(c45074864c3a4bcbf3a87d164027ae16dca53d9c) ) // planes 0,1
	ROM_LOAD( "eprom_6_vr_alpha_denshi.5c",        0x2000, 0x2000, CRC(1fb84ee6) SHA1(56ceb86c509be783f806403ac21e7c9684760d5f) ) // plane 3

	ROM_REGION( 0x04000, "gfx2", 0 )    // 3bpp chars + sprites: rearranged by init_exctsccr() to leave only sprites, on display board
	ROM_LOAD( "eprom_5_vr_alpha_denshi.5b",        0x0000, 0x2000, CRC(5605b60b) SHA1(19d5909896ae4a3d7552225c369d30475c56793b) ) // planes 0,1

	ROM_REGION( 0x02000, "gfx3", 0 )    // 4bpp sprites, on display board
	ROM_LOAD( "eprom_7_vr_alpha_denshi.5k",        0x0000, 0x1000, CRC(1d37edfa) SHA1(184fa6dd7b1b3fff4c5fc19b42301ccb7979ac84) )
	ROM_LOAD( "eprom_8_vr_alpha_denshi.5l",        0x1000, 0x1000, CRC(b97f396c) SHA1(4ffe512acf047230bd593911a615fc0ef66b481d) )

	ROM_REGION( 0x0220, "proms", 0 ) // colors match video from PCB (even the field one)
	ROM_LOAD( "tbp18s030.5j",     0x0000, 0x0020, CRC(899d153d) SHA1(669f1a2de387ae7cdce16c2714a384c9586ed255) ) // palette, marked as 7051 on CPU & sound board
	ROM_LOAD( "tbp24s10.61d",     0x0020, 0x0100, CRC(75613784) SHA1(38dc1c1d2d0f33d58f035942e71665c9810fdab1) ) // lookup table, marked as 7052 on display board
	ROM_LOAD( "tbp24s10.60h",     0x0120, 0x0100, CRC(1a52d6eb) SHA1(cd0c8cbaf5d8df14df34103cde2ec595039a6d51) ) // lookup table, marked as 7052 on display board
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void champbas_state::init_champbas()
{
	// chars and sprites are mixed in the same ROMs, so rearrange them for easier decoding
	uint8_t *rom1 = memregion("gfx1")->base();
	uint8_t *rom2 = memregion("gfx2")->base();
	int len = memregion("gfx1")->bytes();

	for (int i = 0; i < len/2; i++)
	{
		uint8_t t = rom1[i + len/2];
		rom1[i + len/2] = rom2[i];
		rom2[i] = t;
	}
}


void exctsccr_state::init_exctsccr()
{
	// chars and sprites are mixed in the same ROMs, so rearrange them for easier decoding
	uint8_t *rom1 = memregion("gfx1")->base();
	uint8_t *rom2 = memregion("gfx2")->base();

	// planes 0,1
	for (int i = 0; i < 0x1000; i++)
	{
		uint8_t t = rom1[i + 0x1000];
		rom1[i + 0x1000] = rom2[i];
		rom2[i] = t;
	}

	// plane 3
	for (int i = 0; i < 0x1000; i++)
	{
		rom2[i + 0x3000] = rom1[i + 0x3000] >> 4;
		rom2[i + 0x2000] = rom1[i + 0x3000] & 0x0f;
	}
	for (int i = 0; i < 0x1000; i++)
	{
		rom1[i + 0x3000] = rom1[i + 0x2000] >> 4;
		rom1[i + 0x2000] &= 0x0f;
	}
}

} // anonymous namespace



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

/*    YEAR  NAME        PARENT    MACHINE     INPUT     INIT                           MONITOR COMPANY, FULLNAME, FLAGS */
GAME( 1982, talbot,     0,        talbot,     talbot,   champbas_state, empty_init,    ROT270, "Alpha Denshi Co. (Volt Electronics license)", "Talbot", MACHINE_SUPPORTS_SAVE )

GAME( 1983, champbas,   0,        champbas,   champbas, champbas_state, init_champbas, ROT0,   "Alpha Denshi Co. (Sega license)", "Champion Base Ball", MACHINE_SUPPORTS_SAVE ) // no protection
GAME( 1983, champbasj,  champbas, champbasj,  champbas, champbas_state, init_champbas, ROT0,   "Alpha Denshi Co.", "Champion Base Ball (Japan set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, champbasja, champbas, champbasja, champbas, champbas_state, init_champbas, ROT0,   "Alpha Denshi Co.", "Champion Base Ball (Japan set 2)", MACHINE_SUPPORTS_SAVE ) // simplified protection, no mcu
GAME( 1983, champbasjb, champbas, champbasjb, champbas, champbas_state, init_champbas, ROT0,   "Alpha Denshi Co.", "Champion Base Ball (Japan set 3)", MACHINE_SUPPORTS_SAVE ) // no protection
GAME( 1983, champbb2,   0,        champbb2,   champbas, champbas_state, init_champbas, ROT0,   "Alpha Denshi Co. (Sega license)", "Champion Base Ball Part-2 (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, champbb2j,  champbb2, champbb2j,  champbas, champbas_state, init_champbas, ROT0,   "Alpha Denshi Co.", "Champion Base Ball Part-2 (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, tbasebal,   champbb2, tbasebal,   champbas, champbas_state, init_champbas, ROT0,   "Alpha Denshi Co.", "Taikyoku Base Ball", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // 68705 protection instead

GAME( 1983, exctsccr,   0,        exctsccr,   exctsccr, exctsccr_state, init_exctsccr, ROT270, "Alpha Denshi Co.", "Exciting Soccer", MACHINE_SUPPORTS_SAVE )
GAME( 1983, exctsccru,  exctsccr, exctsccr,   exctsccr, exctsccr_state, init_exctsccr, ROT270, "Alpha Denshi Co.", "Exciting Soccer (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, exctsccra,  exctsccr, exctsccr,   exctsccr, exctsccr_state, init_exctsccr, ROT270, "Alpha Denshi Co.", "Exciting Soccer (alternate music)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, exctsccrj,  exctsccr, exctsccr,   exctsccr, exctsccr_state, init_exctsccr, ROT270, "Alpha Denshi Co.", "Exciting Soccer (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, exctsccrjo, exctsccr, exctsccr,   exctsccr, exctsccr_state, init_exctsccr, ROT270, "Alpha Denshi Co.", "Exciting Soccer (Japan, older)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, exctsccrb,  exctsccr, exctsccrb,  exctsccr, exctsccr_state, init_exctsccr, ROT270, "bootleg (Kazutomi)", "Exciting Soccer (bootleg)", MACHINE_SUPPORTS_SAVE ) // on champbasj hardware
GAME( 1984, exctscc2,   0,        exctscc2,   exctsccr, exctsccr_state, init_exctsccr, ROT270, "Alpha Denshi Co.", "Exciting Soccer II", MACHINE_SUPPORTS_SAVE )
