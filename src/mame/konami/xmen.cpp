// license:BSD-3-Clause
// copyright-holders: Nicola Salmoria

/***************************************************************************

X-Men

driver by Nicola Salmoria

notes:

The way the double screen works in xmen6p is not fully understood.

One of the screens probably has a buffer to delay it by 1 frame. If you
hardcode buffering to the right screen, the intro is synced correctly,
but in-game is not. Buffer the left screen and then in-game is correct,
but the intro is not.

The board only has one of each gfx chip, the only additional chip not found
on the 2/4p board is 053253. This chip is also on Run n Gun which is
likewise a 2 screen game.

Reverse-engineered schematics: https://github.com/jotego/jtbin/blob/master/sch/xmen.pdf

***************************************************************************/

#include "emu.h"

#include "k052109.h"
#include "k053246_k053247_k055673.h"
#include "k053251.h"
#include "konami_helper.h"
#include "konamipt.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/k054321.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/k054539.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "layout/generic.h"


// configurable logging
#define LOG_EEPROMW     (1U << 1)
#define LOG_SOUNDIRQ    (1U << 2)
#define LOG_OKI         (1U << 3)

//#define VERBOSE (LOG_GENERAL | LOG_EEPROMW | LOG_SOUNDIRQ | LOG_OKI)

#include "logmacro.h"

#define LOGEEPROMW(...)     LOGMASKED(LOG_EEPROMW,     __VA_ARGS__)
#define LOGSOUNDIRQ(...)    LOGMASKED(LOG_SOUNDIRQ,    __VA_ARGS__)
#define LOGOKI(...)         LOGMASKED(LOG_OKI,         __VA_ARGS__)


namespace {

class xmen_state : public driver_device
{
public:
	xmen_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k052109(*this, "k052109"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_screen(*this, "screen"),
		m_z80bank(*this, "z80bank"),
		m_okibank(*this, "okibank"),
		m_eeprom_out(*this, "EEPROMOUT")
	{ }

	void xmen(machine_config &config);
	void xmenabl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// video-related
	uint8_t m_layer_colorbase[3]{};
	uint8_t m_sprite_colorbase = 0;
	int m_layerpri[3]{};
	bool m_tilemap_select;

	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<k052109_device> m_k052109;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	required_device<screen_device> m_screen;

	void eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void _18fa00_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void base(machine_config &config);
	void sound_hardware(machine_config &config);

private:
	optional_memory_bank m_z80bank;
	optional_memory_bank m_okibank;

	required_ioport m_eeprom_out;

	// misc
	uint8_t m_vblank_irq_mask = 0;

	void sound_bankswitch_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	K052109_CB_MEMBER(tile_callback);
	K053246_CB_MEMBER(sprite_callback);

	void base_main_map(address_map &map) ATTR_COLD;
	void bootleg_main_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	void bootleg_sound_hardware(machine_config &config);
};

class xmen6p_state : public xmen_state
{
public:
	xmen6p_state(const machine_config &mconfig, device_type type, const char *tag) :
		xmen_state(mconfig, type, tag),
		m_spriteram(*this, "spriteram%u", 0),
		m_tilemap(*this, "tilemap%u", 0)
	{ }

	void xmen6p(machine_config &config);

	int frame_r();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	std::unique_ptr<bitmap_ind16> m_screen_bitmap[2]; // 0 left screen, 1 right screen
	required_shared_ptr_array<uint16_t, 2> m_spriteram;
	required_shared_ptr_array<uint16_t, 4> m_tilemap;
	uint16_t *m_k053247_ram;

	template <uint8_t Which> uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void main_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(xmen_state::tile_callback)
{
	// (color & 0x02) is flip y handled internally by the 052109
	if (layer == 0)
		*color = m_layer_colorbase[layer] + ((*color & 0xf0) >> 4);
	else
		*color = m_layer_colorbase[layer] + ((*color & 0x7c) >> 2);
}

/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

K053246_CB_MEMBER(xmen_state::sprite_callback)
{
	int const pri = (*color & 0x00e0) >> 4;   // ???????

	if (pri <= m_layerpri[2])
		*priority_mask = 0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;

	*color = m_sprite_colorbase + (*color & 0x001f);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void xmen6p_state::video_start()
{
	m_k053246->k053247_get_ram(&m_k053247_ram);

	m_screen_bitmap[0] = std::make_unique<bitmap_ind16>(64 * 8, 32 * 8);
	m_screen_bitmap[1] = std::make_unique<bitmap_ind16>(64 * 8, 32 * 8);

	save_item(NAME(*m_screen_bitmap[0]));
	save_item(NAME(*m_screen_bitmap[1]));
}


/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t xmen_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const bg_colorbase = m_k053251->get_palette_index(k053251_device::CI4);
	m_sprite_colorbase = m_k053251->get_palette_index(k053251_device::CI1);
	m_layer_colorbase[0] = m_k053251->get_palette_index(k053251_device::CI3);
	m_layer_colorbase[1] = m_k053251->get_palette_index(k053251_device::CI0);
	m_layer_colorbase[2] = m_k053251->get_palette_index(k053251_device::CI2);

	m_k052109->tilemap_update();

	int layer[3];

	layer[0] = 0;
	m_layerpri[0] = m_k053251->get_priority(k053251_device::CI3);
	layer[1] = 1;
	m_layerpri[1] = m_k053251->get_priority(k053251_device::CI0);
	layer[2] = 2;
	m_layerpri[2] = m_k053251->get_priority(k053251_device::CI2);

	konami_sortlayers3(layer, m_layerpri);

	screen.priority().fill(0, cliprect);
	// note the '+1' in the background color!!!
	bitmap.fill(16 * bg_colorbase + 1, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[0], 0, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	/* this isn't supported anymore and it is unsure if still needed; keeping here for reference
	pdrawgfx_shadow_lowpri = 1; fix shadows of boulders in front of feet */
	m_k053246->k053247_sprites_draw(bitmap, cliprect);
	return 0;
}


template <uint8_t Which>
uint32_t xmen6p_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 32 * 8; y++)
	{
		uint16_t *const line_dest = &bitmap.pix(y);
		uint16_t const *const line_src = &m_screen_bitmap[Which]->pix(y);

		for (int x = 12 * 8; x < 52 * 8; x++)
			line_dest[x] = line_src[x];
	}

	return 0;
}

// my lefts and rights are mixed up in several places..
void xmen6p_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		rectangle cliprect;

		// const rectangle *visarea = m_screen->visible_area();
		// cliprect = *visarea;
		cliprect.set(0, 64 * 8 - 1, 2 * 8, 30 * 8 - 1);

		int index = m_screen->frame_number() & 1;
		bitmap_ind16 *renderbitmap = m_screen_bitmap[index].get();

		// copy the desired spritelist to the chip
		memcpy(m_k053247_ram, m_spriteram[index], 0x1000);

		/* we write the entire content of the tileram to the chip to ensure
		   everything gets marked as dirty and the desired tilemap is rendered
		   this is not very efficient!
		*/
		index += m_tilemap_select ? 2 : 0;
		for (int offset = 0; offset < (0xc000 / 2); offset++)
		{
			if (index == 0 || (offset != 0x1c00 && offset != 0x1c80 && offset != 0x1e80))
				m_k052109->write(offset, m_tilemap[index][offset] & 0x00ff);
		}

		int const bg_colorbase = m_k053251->get_palette_index(k053251_device::CI4);
		m_sprite_colorbase = m_k053251->get_palette_index(k053251_device::CI1);
		m_layer_colorbase[0] = m_k053251->get_palette_index(k053251_device::CI3);
		m_layer_colorbase[1] = m_k053251->get_palette_index(k053251_device::CI0);
		m_layer_colorbase[2] = m_k053251->get_palette_index(k053251_device::CI2);

		m_k052109->tilemap_update();

		int layer[3];

		m_layerpri[0] = m_k053251->get_priority(k053251_device::CI3);
		layer[0] = 0;
		m_layerpri[1] = m_k053251->get_priority(k053251_device::CI0);
		layer[1] = 1;
		m_layerpri[2] = m_k053251->get_priority(k053251_device::CI2);
		layer[2] = 2;

		konami_sortlayers3(layer, m_layerpri);

		m_screen->priority().fill(0, cliprect);
		// note the '+1' in the background color!!!
		renderbitmap->fill(16 * bg_colorbase + 1, cliprect);
		m_k052109->tilemap_draw(*m_screen, *renderbitmap, cliprect, layer[0], 0, 1);
		m_k052109->tilemap_draw(*m_screen, *renderbitmap, cliprect, layer[1], 0, 2);
		m_k052109->tilemap_draw(*m_screen, *renderbitmap, cliprect, layer[2], 0, 4);

		/* this isn't supported anymore and it is unsure if still needed; keeping here for reference
		pdrawgfx_shadow_lowpri = 1; fix shadows of boulders in front of feet */
		m_k053246->k053247_sprites_draw(*renderbitmap, cliprect);
	}
}


/***************************************************************************

  EEPROM

***************************************************************************/

void xmen_state::eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGEEPROMW("%06x: write %04x to 108000\n", m_maincpu->pc(), data);
	if (ACCESSING_BITS_0_7)
	{
		// bit 0 = coin counter
		machine().bookkeeping().coin_counter_w(0, data & 0x01);

		// bit 2 is data
		// bit 3 is clock (active high)
		// bit 4 is cs (active low)
		m_eeprom_out->write(data, 0xff);

		// bit 5 is enabled in IRQ3, disabled in IRQ5 (sprite DMA start?)
		// bit 7 used in xmen6p to select other tilemap bank (see halfway level 5)
		m_tilemap_select = BIT(data, 7);
	}
	if (ACCESSING_BITS_8_15)
	{
		// bit 8 = enable sprite ROM reading
		m_k053246->k053246_set_objcha_line((data & 0x0100) ? ASSERT_LINE : CLEAR_LINE);
		// bit 9 = enable char ROM reading through the video RAM
		// bit 10 = sound irq, but with some kind of hold
		m_k052109->set_rmrd_line((data & 0x0200) ? ASSERT_LINE : CLEAR_LINE);
		if (data & 0x400)
		{
			LOGSOUNDIRQ("tick!\n");
			if (m_audiocpu)
				m_audiocpu->set_input_line(0, HOLD_LINE);
		}
	}
}

void xmen_state::_18fa00_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		// bit 2 is interrupt enable
		m_vblank_irq_mask = data & 0x04;
	}
}

void xmen_state::sound_bankswitch_w(uint8_t data)
{
	m_z80bank->set_entry(data & 0x07);
}


void xmen_state::base_main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x100fff).rw(m_k053246, FUNC(k053247_device::k053247_word_r), FUNC(k053247_device::k053247_word_w));
	map(0x101000, 0x101fff).ram();
	map(0x104000, 0x104fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x108000, 0x108001).w(FUNC(xmen_state::eeprom_w));
	map(0x108020, 0x108027).w(m_k053246, FUNC(k053247_device::k053246_w));
	map(0x108060, 0x10807f).w(m_k053251, FUNC(k053251_device::write)).umask16(0x00ff);
	map(0x10a000, 0x10a001).portr("P2_P4").w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x10a002, 0x10a003).portr("P1_P3");
	map(0x10a004, 0x10a005).portr("EEPROM");
	map(0x10a00c, 0x10a00d).r(m_k053246, FUNC(k053247_device::k053246_r));
	map(0x110000, 0x113fff).ram();     // main RAM
	map(0x18c000, 0x197fff).rw(m_k052109, FUNC(k052109_device::read), FUNC(k052109_device::write)).umask16(0x00ff);
	map(0x18fa00, 0x18fa01).w(FUNC(xmen_state::_18fa00_w));
}

void xmen_state::main_map(address_map &map)
{
	base_main_map(map);

	map(0x108040, 0x10805f).m("k054321", FUNC(k054321_device::main_map)).umask16(0x00ff);
}

void xmen_state::bootleg_main_map(address_map &map)
{
	base_main_map(map);

	// map(0x103ffe, 0x103fff) // sound related, too?
	map(0x10804d, 0x10804d).lw8(NAME([this] (uint8_t data) { m_okibank->set_entry(data & 0x0f); LOGOKI("oki bank :%02x\n", data); })); // TODO: verify once oki ROM 1 is redumped / confirmed
	map(0x10804f, 0x10804f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void xmen_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_z80bank);
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe22f).rw("k054539", FUNC(k054539_device::read), FUNC(k054539_device::write));
	map(0xe800, 0xe801).mirror(0x0400).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xf000, 0xf003).m("k054321", FUNC(k054321_device::sound_map));
	map(0xf800, 0xf800).w(FUNC(xmen_state::sound_bankswitch_w));
}

void xmen_state::oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr(m_okibank);
}

void xmen6p_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x0fffff).rom();
	map(0x100000, 0x100fff).ram().share(m_spriteram[0]); // left screen
	map(0x101000, 0x101fff).ram();
	map(0x102000, 0x102fff).ram().share(m_spriteram[1]); // right screen
	map(0x103000, 0x103fff).ram();     // 6p - a buffer?
	map(0x104000, 0x104fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x108000, 0x108001).w(FUNC(xmen6p_state::eeprom_w));
	map(0x108020, 0x108027).w(m_k053246, FUNC(k053247_device::k053246_w)); // sprites
	map(0x108040, 0x10805f).m("k054321", FUNC(k054321_device::main_map)).umask16(0x00ff);
	map(0x108060, 0x10807f).w(m_k053251, FUNC(k053251_device::write)).umask16(0x00ff);
	map(0x10a000, 0x10a001).portr("P2_P4").w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x10a002, 0x10a003).portr("P1_P3");
	map(0x10a004, 0x10a005).portr("EEPROM");
	map(0x10a006, 0x10a007).portr("P5_P6");
	map(0x10a00c, 0x10a00d).r(m_k053246, FUNC(k053247_device::k053246_r)); // sprites
	map(0x110000, 0x113fff).ram();     // main RAM
//  map(0x18c000, 0x197fff).w("k052109", FUNC(k052109_device:write)).umask16(0x00ff).share("tilemapleft");
	map(0x18c000, 0x197fff).ram().share(m_tilemap[0]); // left screen
	map(0x18fa00, 0x18fa01).w(FUNC(xmen6p_state::_18fa00_w));
/*
    map(0x1ac000, 0x1af7ff).readonly();
    map(0x1ac000, 0x1af7ff).writeonly();

    map(0x1b0000, 0x1b37ff).readonly();
    map(0x1b0000, 0x1b37ff).writeonly();

    map(0x1b4000, 0x1b77ff).readonly();
    map(0x1b4000, 0x1b77ff).writeonly();
*/
	map(0x1ac000, 0x1b7fff).ram().share(m_tilemap[1]); // right screen

	// what are the regions below buffers? (used by hw or software?)
/*
    map(0x1cc000, 0x1cf7ff).readonly();
    map(0x1cc000, 0x1cf7ff).writeonly();

    map(0x1d0000, 0x1d37ff).readonly();
    map(0x1d0000, 0x1d37ff).writeonly();
*/
	map(0x1cc000, 0x1d7fff).ram().share(m_tilemap[2]); // left screen

	// whats the stuff below, buffers?
/*
    map(0x1ec000, 0x1ef7ff).readonly();
    map(0x1ec000, 0x1ef7ff).writeonly();
    map(0x1f0000, 0x1f37ff).readonly();
    map(0x1f0000, 0x1f37ff).writeonly();
    map(0x1f4000, 0x1f77ff).readonly();
    map(0x1f4000, 0x1f77ff).writeonly();
*/
	map(0x1ec000, 0x1f7fff).ram().share(m_tilemap[3]); // right screen
}


static INPUT_PORTS_START( xmen )
	PORT_START("P1_P3")
	KONAMI16_LSB_UDLR(1, IPT_BUTTON3, IPT_COIN1 )
	KONAMI16_MSB_UDLR(3, IPT_BUTTON3, IPT_COIN3 )

	PORT_START("P2_P4")
	KONAMI16_LSB_UDLR(2, IPT_BUTTON3, IPT_COIN2 )
	KONAMI16_MSB_UDLR(4, IPT_BUTTON3, IPT_COIN4 )

	PORT_START("EEPROM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE4 )
	PORT_BIT( 0x0030, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::do_read))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::ready_read))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x3000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?
	PORT_SERVICE_NO_TOGGLE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::di_write))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::clk_write))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::cs_write))
INPUT_PORTS_END

static INPUT_PORTS_START( xmen2p )
	PORT_START("P1_P3")
	KONAMI16_LSB_UDLR(1, IPT_BUTTON3, IPT_COIN1 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_P4")
	KONAMI16_LSB_UDLR(2, IPT_BUTTON3, IPT_COIN2 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EEPROM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x003c, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::do_read))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::ready_read))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x3000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?
	PORT_SERVICE_NO_TOGGLE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::di_write))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::clk_write))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::cs_write))
INPUT_PORTS_END

int xmen6p_state::frame_r()
{
	return m_screen->frame_number() & 1;
}

static INPUT_PORTS_START( xmen6p )
	PORT_START("P1_P3")
	KONAMI16_LSB_UDLR(1, IPT_BUTTON3, IPT_COIN1 )
	KONAMI16_MSB_UDLR(3, IPT_BUTTON3, IPT_COIN3 )

	PORT_START("P2_P4")
	KONAMI16_LSB_UDLR(2, IPT_BUTTON3, IPT_COIN2 )
	KONAMI16_MSB_UDLR(4, IPT_BUTTON3, IPT_COIN4 )

	PORT_START("P5_P6")
	KONAMI16_LSB_UDLR(5, IPT_BUTTON3, IPT_COIN5 )
	KONAMI16_MSB_UDLR(6, IPT_BUTTON3, IPT_COIN6 )

	PORT_START("EEPROM")
	PORT_BIT( 0x003f, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::do_read))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::ready_read))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START5 ) // not verified
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_START6 ) // not verified
	PORT_SERVICE_NO_TOGGLE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(xmen6p_state::frame_r))  // screen indicator?

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::di_write))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::clk_write))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::cs_write))
INPUT_PORTS_END


void xmen_state::machine_start()
{
	if (m_z80bank)
	{
		m_z80bank->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);
		m_z80bank->set_entry(0);
	}

	if (m_okibank)
	{
		m_okibank->configure_entries(0, 16, memregion("oki")->base(), 0x10000);
		m_okibank->set_entry(0);
	}

	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_layerpri));
	save_item(NAME(m_vblank_irq_mask));
	save_item(NAME(m_tilemap_select));
}

void xmen_state::machine_reset()
{
	for (int i = 0; i < 3; i++)
	{
		m_layerpri[i] = 0;
		m_layer_colorbase[i] = 0;
	}

	m_sprite_colorbase = 0;
	m_vblank_irq_mask = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(xmen_state::scanline)
{
	int const scanline = param;

	if (scanline == 240 && m_vblank_irq_mask) // vblank-out irq
		m_maincpu->set_input_line(3, HOLD_LINE);

	if (scanline == 0) // sprite DMA irq?
		m_maincpu->set_input_line(5, HOLD_LINE);
}

void xmen_state::sound_hardware(machine_config &config)
{
	Z80(config, m_audiocpu, XTAL(16'000'000) / 2); // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &xmen_state::sound_map);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	K054321(config, "k054321", "lspeaker", "rspeaker");

	YM2151(config, "ymsnd", XTAL(16'000'000) / 4).add_route(0, "lspeaker", 0.20).add_route(1, "rspeaker", 0.20);  // verified on PCB

	k054539_device &k054539(K054539(config, "k054539", XTAL(18'432'000)));
	k054539.add_route(0, "rspeaker", 1.00);
	k054539.add_route(1, "lspeaker", 1.00);
}

void xmen_state::bootleg_sound_hardware(machine_config &config)
{
	// sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", 1'000'000, okim6295_device::PIN7_HIGH)); // clock and pin7 not verified
	oki.set_addrmap(0, &xmen_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void xmen_state::base(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(16'000'000)); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &xmen_state::main_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(xmen_state::scanline), "screen", 0, 1);

	EEPROM_ER5911_8BIT(config, "eeprom");

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(59.17);   // verified on PCB
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(13*8, (64-13)*8-1, 2*8, 30*8-1 );   // correct, same issue of tmnt2
	m_screen->set_screen_update(FUNC(xmen_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 2048).enable_shadows();

	K052109(config, m_k052109, 0);
	m_k052109->set_palette("palette");
	m_k052109->set_screen(nullptr);
	m_k052109->set_tile_callback(FUNC(xmen_state::tile_callback));

	K053246(config, m_k053246, 0);
	m_k053246->set_sprite_callback(FUNC(xmen_state::sprite_callback));
	m_k053246->set_config(NORMAL_PLANE_ORDER, 53, -2);
	m_k053246->set_palette("palette");

	K053251(config, m_k053251, 0);
}

void xmen_state::xmen(machine_config &config)
{
	base(config);

	sound_hardware(config);
}

void xmen_state::xmenabl(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &xmen_state::bootleg_main_map);

	bootleg_sound_hardware(config);
}

void xmen6p_state::xmen6p(machine_config &config)
{
	xmen(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &xmen6p_state::main_map);

	// video hardware
	config.set_default_layout(layout_dualhsxs);

	config.device_remove("screen");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(12*8, 48*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(xmen6p_state::screen_update<0>));
	m_screen->set_palette("palette");

	screen_device &screen2(SCREEN(config, "screen2", SCREEN_TYPE_RASTER));
	screen2.set_refresh_hz(60);
	screen2.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen2.set_size(64*8, 32*8);
	screen2.set_visarea(16*8, 52*8-1, 2*8, 30*8-1);
	screen2.set_screen_update(FUNC(xmen6p_state::screen_update<1>));
	screen2.screen_vblank().set(FUNC(xmen6p_state::screen_vblank));
	screen2.set_palette("palette");

	m_k053246->set_screen(m_screen);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

/*

    KONAMI - X-MEN 2P/4P - GX065 - PWB353018A

    +--------------------------------------------------------------------------------------+
    |                                                                                      |
    |                                  [  065A06.1f  ]     [  065A12.1h  ][  065A11.1l  ]  |
    |                                                                                      |
    | sound                                                [  065A09.1h  ][  065A10.1l  ]  |
    |  out                                                                                 |
    |                                                                                      |
    |        [  054544  ] [  054539  ]                                                     |
    |                                                                                      |
    |                                                                                      |
    +-+     [    Z80    ] [ YM2151 ] [  065*01.6f  ]                                       |
      |                                                                                    |
    +-+                                                                     [  053246  ]   |
    |                                                                                      |
    |                   [  065A02.9d  ]  [  065A03.9f  ]                                   |
    +-+                                                                                    |
    +-+                 [  065*04.10d ]  [  065*05.10f ]                                   |
    |   J                                                                                  |
    |   A                                                                                  |
    |   M                                                                   [  053247  ]   |
    |   M                                                                                  |
    |   A              [     68000 - 16Mhz     ]  [qz 24Mhz]                               |
    |                                                                                      |
    |                                                                                      |
    |                                                                                      |
    |                    [qz 32Mhz/18.432Mhz]   [  052109  ] [  051962  ]   [  053251  ]   |
    |                                                                                      |
    +-+                                                                                    |
      |                                                                    [  065A08.15l ] |
    +-+                                                                                    |
    |                                                                      [  065A07.16l ] |
    |     [cn7 (4P)]                                                                       |
    |test                                                                                  |
    | sw  [cn6 (3P)]                                                                       |
    |                                                                                      |
    +--------------------------------------------------------------------------------------+

    054544
    054539
    053246
    053247
    052109
    051962
    053251
*/

ROM_START( xmen )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-eba04.10d",  0x00000, 0x20000, CRC(3588c5ec) SHA1(7966e7259038468845dafd19e5f7fc576c2901fa) ) // Europe 4 Player version
	ROM_LOAD16_BYTE( "065-eba05.10f",  0x00001, 0x20000, CRC(79ce32f8) SHA1(1a21b38d4a82103d78e246aca68ed3e4afaf60f3) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "065-a08.15l", 0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )
	ROM_LOAD32_WORD( "065-a07.16l", 0x000002, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "k053246", 0 )   // graphics (addressable by the main CPU)
	ROM_LOAD64_WORD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) ) // sprites
	ROM_LOAD64_WORD( "065-a10.2l",  0x000002, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD64_WORD( "065-a12.1h",  0x000004, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD64_WORD( "065-a11.1l",  0x000006, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )    // samples
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_eba.nv", 0x0000, 0x0080, CRC(37f8e77a) SHA1(0b92caba33486c6fd104806aa96f735743bb2221) )
ROM_END

ROM_START( xmenu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-ubb04.10d",  0x00000, 0x20000, CRC(f896c93b) SHA1(0bee89fe4d36a9b2ded864770198eb2df6903580) ) // US 4 Player version
	ROM_LOAD16_BYTE( "065-ubb05.10f",  0x00001, 0x20000, CRC(e02e5d64) SHA1(9838c1cf9862db3ca70a23ef5f3c5883729c4e0c) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "065-a08.15l", 0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )
	ROM_LOAD32_WORD( "065-a07.16l", 0x000002, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "k053246", 0 )   // graphics (addressable by the main CPU)
	ROM_LOAD64_WORD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) ) // sprites
	ROM_LOAD64_WORD( "065-a10.2l",  0x000002, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD64_WORD( "065-a12.1h",  0x000004, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD64_WORD( "065-a11.1l",  0x000006, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )    // samples
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_ubb.nv", 0x0000, 0x0080, CRC(52f334ba) SHA1(171c22b5ac41bcbbcfc31528cf49c096f6829a72) )
ROM_END

ROM_START( xmenua )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-ueb04.10d",  0x00000, 0x20000, CRC(eee4e7ef) SHA1(72fe588bc58c692e7f9891f3e89c7d6fcc28c480) ) // US 4 Player version
	ROM_LOAD16_BYTE( "065-ueb05.10f",  0x00001, 0x20000, CRC(c3b2ffde) SHA1(27b32429c8c35cf15d1e5437535c4c335eee2118) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "065-a08.15l", 0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )
	ROM_LOAD32_WORD( "065-a07.16l", 0x000002, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "k053246", 0 )   // graphics (addressable by the main CPU)
	ROM_LOAD64_WORD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) ) // sprites
	ROM_LOAD64_WORD( "065-a10.2l",  0x000002, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD64_WORD( "065-a12.1h",  0x000004, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD64_WORD( "065-a11.1l",  0x000006, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )    // samples
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_ueb.nv", 0x0000, 0x0080, CRC(db85fef4) SHA1(9387d2f4dbb3cb5cdf59d2304393ac50b3c12ebe) )
ROM_END

ROM_START( xmenj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-jba04.10d",  0x00000, 0x20000, CRC(d86cf5eb) SHA1(8bf67eb6cdb7187142557c27b058282886984a61) ) // Japan 4 Player version
	ROM_LOAD16_BYTE( "065-jba05.10f",  0x00001, 0x20000, CRC(abbc8126) SHA1(482a3c9be45b9d77460bd3df94e3c6cf285e63a2) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "065-a08.15l", 0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )
	ROM_LOAD32_WORD( "065-a07.16l", 0x000002, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "k053246", 0 )   // graphics (addressable by the main CPU)
	ROM_LOAD64_WORD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) ) // sprites
	ROM_LOAD64_WORD( "065-a10.2l",  0x000002, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD64_WORD( "065-a12.1h",  0x000004, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD64_WORD( "065-a11.1l",  0x000006, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )    // samples
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_jba.nv", 0x0000, 0x0080, CRC(7439cea7) SHA1(d34b8ed0549b0457362159098e5c86b1356e35d0) )
ROM_END

ROM_START( xmenja )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-jea04.10d",  0x00000, 0x20000, CRC(655a61d6) SHA1(30a50b0a13205252d1cec6b6a6300e25c2f376c6) ) // Japan 4 Player version
	ROM_LOAD16_BYTE( "065-jea05.10f",  0x00001, 0x20000, CRC(7ea9fc84) SHA1(767f957c0c2f6a2d938d9e37388a17a6bce01dd8) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "065-a08.15l", 0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )
	ROM_LOAD32_WORD( "065-a07.16l", 0x000002, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "k053246", 0 )   // graphics (addressable by the main CPU)
	ROM_LOAD64_WORD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) ) // sprites
	ROM_LOAD64_WORD( "065-a10.2l",  0x000002, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD64_WORD( "065-a12.1h",  0x000004, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD64_WORD( "065-a11.1l",  0x000006, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )    // samples
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_jea.nv", 0x0000, 0x0080, CRC(df5b6bc6) SHA1(42fff0793bb1488bcdd69c39a8c5f58cdf39e1ff) )
ROM_END

ROM_START( xmena )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-aea04.10d",  0x00000, 0x20000, CRC(0e8d2e98) SHA1(f58613bd8719566ae04d4b5f03864524a7c86a65) ) // Asia 4 Player version
	ROM_LOAD16_BYTE( "065-aea05.10f",  0x00001, 0x20000, CRC(0b742a4e) SHA1(ed9c986261e72af7a80b44f9c2c576c265807e90) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "065-a08.15l", 0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )
	ROM_LOAD32_WORD( "065-a07.16l", 0x000002, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "k053246", 0 )   // graphics (addressable by the main CPU)
	ROM_LOAD64_WORD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) ) // sprites
	ROM_LOAD64_WORD( "065-a10.2l",  0x000002, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD64_WORD( "065-a12.1h",  0x000004, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD64_WORD( "065-a11.1l",  0x000006, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )    // samples
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_aea.nv", 0x0000, 0x0080, CRC(d73d4f20) SHA1(b39906eb59ecf8f1e8141b467021e0a581186d47) )
ROM_END

ROM_START( xmenaa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-ada04.10d",  0x00000, 0x20000, CRC(b8276624) SHA1(5b601393faa4bf578d84b590eb2360ad400368a5) ) // Asia 4 Player version
	ROM_LOAD16_BYTE( "065-ada05.10f",  0x00001, 0x20000, CRC(c68582ad) SHA1(d2ca23cc0ad08e7f3d5c533f6fe43d4c215c114e) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "065-a08.15l", 0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )
	ROM_LOAD32_WORD( "065-a07.16l", 0x000002, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "k053246", 0 )   // graphics (addressable by the main CPU)
	ROM_LOAD64_WORD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) ) // sprites
	ROM_LOAD64_WORD( "065-a10.2l",  0x000002, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD64_WORD( "065-a12.1h",  0x000004, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD64_WORD( "065-a11.1l",  0x000006, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )    // samples
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_ada.nv", 0x0000, 0x0080, CRC(a77a3891) SHA1(84ec257790d5c1859ffcbc9371a72ea99d7f8928) )
ROM_END

ROM_START( xmen2pe )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-eaa04.10d",  0x00000, 0x20000, CRC(502861e7) SHA1(f96aab2d2006703065de5bd7e341f929d04f5f60) ) // Europe 2 Player version
	ROM_LOAD16_BYTE( "065-eaa05.10f",  0x00001, 0x20000, CRC(ca6071bf) SHA1(454ddc3b598389e960e87e577a01a7de71d1f591) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "065-a08.15l", 0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )
	ROM_LOAD32_WORD( "065-a07.16l", 0x000002, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "k053246", 0 )   // graphics (addressable by the main CPU)
	ROM_LOAD64_WORD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) ) // sprites
	ROM_LOAD64_WORD( "065-a10.2l",  0x000002, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD64_WORD( "065-a12.1h",  0x000004, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD64_WORD( "065-a11.1l",  0x000006, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )    // samples
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_eaa.nv", 0x0000, 0x0080, CRC(1cbcb653) SHA1(a86b4ad34ccbd868662ff8c61eb21ec07e8bf8b1) )
ROM_END

ROM_START( xmen2pu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-uab04.10d",  0x00000, 0x20000, CRC(ff003db1) SHA1(b8aeb98dc0a38f0bda152d893fb60679bbdbadb3) ) // US 2 Player version
	ROM_LOAD16_BYTE( "065-uab05.10f",  0x00001, 0x20000, CRC(4e99a943) SHA1(66b47de497a116b2002cc5496c1a3f4173a55bc5) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "065-a08.15l", 0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )
	ROM_LOAD32_WORD( "065-a07.16l", 0x000002, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "k053246", 0 )   // graphics (addressable by the main CPU)
	ROM_LOAD64_WORD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) ) // sprites
	ROM_LOAD64_WORD( "065-a10.2l",  0x000002, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD64_WORD( "065-a12.1h",  0x000004, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD64_WORD( "065-a11.1l",  0x000006, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )    // samples
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_uab.nv", 0x0000, 0x0080, CRC(79b76593) SHA1(f9921a2963f249fa341bfb57cc9e213e2efed9b9) )
ROM_END

ROM_START( xmen2pj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-jaa04.10d",  0x00000, 0x20000, CRC(66746339) SHA1(8cc5f5deb4178b0444ffc5974940a30cb003114e) ) // Japan 2 Player version
	ROM_LOAD16_BYTE( "065-jaa05.10f",  0x00001, 0x20000, CRC(1215b706) SHA1(b746dedab9c509b5cd941f0f4ddd3709e8a58cce) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "065-a08.15l", 0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )
	ROM_LOAD32_WORD( "065-a07.16l", 0x000002, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "k053246", 0 )   // graphics (addressable by the main CPU)
	ROM_LOAD64_WORD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) ) // sprites
	ROM_LOAD64_WORD( "065-a10.2l",  0x000002, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD64_WORD( "065-a12.1h",  0x000004, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD64_WORD( "065-a11.1l",  0x000006, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )    // samples
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_jaa.nv", 0x0000, 0x0080, CRC(849a9e19) SHA1(bd335a2d33bf4433de4fd57b8108b216eb3a2cf1) )
ROM_END

ROM_START( xmen2pa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-aaa04.10d",  0x00000, 0x20000, CRC(7f8b27c2) SHA1(052db1f47671564a440544a41fc397a19d1aff3a) ) // Asia 2 Player version
	ROM_LOAD16_BYTE( "065-aaa05.10f",  0x00001, 0x20000, CRC(841ed636) SHA1(33f96022ce3dae9b49eb51fd4e8f7387a1777002) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "065-a08.15l", 0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )
	ROM_LOAD32_WORD( "065-a07.16l", 0x000002, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "k053246", 0 )   // graphics (addressable by the main CPU)
	ROM_LOAD64_WORD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) ) // sprites
	ROM_LOAD64_WORD( "065-a10.2l",  0x000002, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD64_WORD( "065-a12.1h",  0x000004, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD64_WORD( "065-a11.1l",  0x000006, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )    // samples
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_aaa.nv", 0x0000, 0x0080, CRC(750fd447) SHA1(27884c1ceb0b5174f7d06e1e06bbbd6d6c5b47e7) )
ROM_END

/*

 KONAMI - X-MEN 2P/4P/6P - GX065 - PWB352532B

    +--------------------------------------------------------------------------------------+
    |                                                                                      |
    |                      [  065A06.1d  ]                 [  065A07.1h  ][  065A08.1l  ]  |
    |                                                                                      |
    |       [  054544  ]                                                                   |
    |                      [  054539  ]     [  053251  ]   [  051962  ] [  052109  ]       |
    |                                                                                      |
    |         [ YM2151 ]                                                                   |
    | sound                     [qz 24Mhz]                                                 |
    |  out                                                                                 |
    +-+  [  065*01.7b  ]                                                                   |
      |                                                                                    |
    +-+                                                                                    |
    |      [    Z80    ]                                                                   |
    |                                                                                      |
    +-+                                                                                    |
    +-+                                                                                    |
    |   J                                                                                  |
    |   A                                                                           [0     |
    |   M                                                                            6     |
    |   M                                                                            5     |
    |   A                                                                            A     |
    |                                                                                0     |
    |                                      [  053253  ]   [  053247  ] [  053246  ]  9.12l]|
    |                                                                                      |
    |                                                                               [0     |
    |                                                                                6     |
    +-+                                                                              5     |
      |                                   [  065A02.17g ]  [  065A03.17j ]           A     |
    +-+                                                                              1     |
    | test                                [  065*04.18g ]  [  065*05.18j ]           0.17l]|
    |  sw                                                                                  |
    | [cn9 (6P)]                                 [     68000 - 16Mhz     ]                 |
    | [cn8 (5P)]                                                                           |
    | [cn7 (4P)]                      [qz 32Mhz/18.432Mhz] [  065A12.22h ][  065A11.22l ]  |
    | [cn6 (3P)]                                                                           |
    |    rgb out                                                                           |
    +--------------------------------------------------------------------------------------+

    054544 *
    054539 *
    053251 *
    051962 *
    052109 *
    053253 - not on other version?
    053247 *
    053246 *

*/

ROM_START( xmen6p )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-ecb04.18g", 0x00000, 0x20000, CRC(258eb21f) SHA1(f1a22a880245f28195e5b6519822c0aa3b166541) ) // Euro 6 Player version
	ROM_LOAD16_BYTE( "065-ecb05.18j", 0x00001, 0x20000, CRC(25997bcd) SHA1(86fb1c64e133b7ca59ffb3910b62b61ee372c71a) )
	ROM_LOAD16_BYTE( "065-a02.17g",   0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.17j",   0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "065-a01.7b",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "065-a08.1l",  0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )
	ROM_LOAD32_WORD( "065-a07.1h",  0x000002, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "k053246", 0 )   // graphics (addressable by the main CPU)
	ROM_LOAD64_WORD( "065-a09.12l", 0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) )    // sprites
	ROM_LOAD64_WORD( "065-a10.17l", 0x000002, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD64_WORD( "065-a12.22h", 0x000004, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD64_WORD( "065-a11.22l", 0x000006, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )    // samples
	ROM_LOAD( "065-a06.1d",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_ecb.nv", 0x0000, 0x0080, CRC(462c6e1a) SHA1(a57087163d7a760d5922c70842cfae20e6a2f5b5) )
ROM_END

ROM_START( xmen6pu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-ucb04.18g", 0x00000, 0x20000, CRC(0f09b8e0) SHA1(79f4d86d8ec45b39e34ddf45860bea0c74dae183) ) // US 6 Player version
	ROM_LOAD16_BYTE( "065-ucb05.18j", 0x00001, 0x20000, CRC(867becbf) SHA1(3f81f4dbd289f98b78d7821a8925598c771f01ef) )
	ROM_LOAD16_BYTE( "065-a02.17g",   0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.17j",   0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "065-a01.7b",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "065-a08.1l",  0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )
	ROM_LOAD32_WORD( "065-a07.1h",  0x000002, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "k053246", 0 )   // graphics (addressable by the main CPU)
	ROM_LOAD64_WORD( "065-a09.12l", 0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) )    // sprites
	ROM_LOAD64_WORD( "065-a10.17l", 0x000002, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD64_WORD( "065-a12.22h", 0x000004, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD64_WORD( "065-a11.22l", 0x000006, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )    // samples
	ROM_LOAD( "065-a06.1d",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_ucb.nv", 0x0000, 0x0080, CRC(f3d0f682) SHA1(b0d4655c651238ae028ffb59a704acba798f93f8) )
ROM_END

// PCB with no Konami custom chips but 4x TPC1020AFN instead. Has M6295 instead of Z80 + YM2151 + K054539. All labels peeled off / unreadable. GFX ROMs are identical to the original.
ROM_START( xmenabl )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom1",  0x00000, 0x80000, CRC(c895fdc1) SHA1(e7e6908374310ef0e71d9541402a18818ea10134) )
	ROM_LOAD16_BYTE( "rom2",  0x00001, 0x80000, CRC(50d56da7) SHA1(7ae0748f65d9de62be8c0cd1b41dceb1ea91ba3e) )

	ROM_REGION( 0x200000, "k052109", 0 )    // tiles, k052109 not really present
	ROM_LOAD32_WORD( "gfx15l", 0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )
	ROM_LOAD32_WORD( "gfx16l", 0x000002, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "k053246", 0 )   // graphics (addressable by the main CPU), k053246 not really present
	ROM_LOAD64_WORD( "gfx2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) ) // sprites
	ROM_LOAD64_WORD( "gfx2l",  0x000002, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD64_WORD( "gfx1h",  0x000004, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD64_WORD( "gfx1l",  0x000006, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "oki1", 0x00000, 0x80000, CRC(2967436b) SHA1(44d4bf4c38e340169577098d958a09877067e119) )
	ROM_LOAD( "oki2", 0x80000, 0x80000, CRC(74a12d1e) SHA1(b5e23e8efea2cf19385c081cfe8088bead132c21) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_aea.nv", 0x0000, 0x0080, CRC(d73d4f20) SHA1(b39906eb59ecf8f1e8141b467021e0a581186d47) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "gal16v8b", 0x000, 0x117, CRC(af9802f9) SHA1(173466314d8672232fa6ed7ad7ec0a3b6aeae85a) ) // near the sprite hw and ROMs
	ROM_LOAD( "gal16v8d", 0x200, 0x117, CRC(5f0f86a3) SHA1(bcc78bf37ed8f11f76759a50e7639bb9b063bc29) ) // near the M6295
ROM_END

} // anonymous namespace


/*

Second "version" letter denotes cabinet type:

A = 2 players, 2 coin slots, can set coin/credit by coin slot, COINs common/independent (when independent, premium start & continue values can be set), FREE PLAY option, requires start buttons
B = 4 players, 4 coin slots, can set premium start & continue value, with or without start buttons
C = 6 players, 6 coin slots, can set premium start & continue value, no support for START buttons, 2 monitors
D = 4 players, 4 coin slots, 4 separate service coins, can set premium start & continue value, with or without start buttons
E = 4 players, 2 coin slots, can set coin/credit by coin slot, FREE PLAY option, requires start buttons

*/

GAME( 1992, xmen,    0,    xmen,    xmen,   xmen_state,   empty_init, ROT0, "Konami",  "X-Men (4 Players ver EBA)",          MACHINE_SUPPORTS_SAVE )
GAME( 1992, xmenu,   xmen, xmen,    xmen,   xmen_state,   empty_init, ROT0, "Konami",  "X-Men (4 Players ver UBB)",          MACHINE_SUPPORTS_SAVE )
GAME( 1992, xmenua,  xmen, xmen,    xmen,   xmen_state,   empty_init, ROT0, "Konami",  "X-Men (4 Players ver UEB)",          MACHINE_SUPPORTS_SAVE )
GAME( 1992, xmenj,   xmen, xmen,    xmen,   xmen_state,   empty_init, ROT0, "Konami",  "X-Men (4 Players ver JBA)",          MACHINE_SUPPORTS_SAVE )
GAME( 1992, xmenja,  xmen, xmen,    xmen,   xmen_state,   empty_init, ROT0, "Konami",  "X-Men (4 Players ver JEA)",          MACHINE_SUPPORTS_SAVE )
GAME( 1992, xmena,   xmen, xmen,    xmen,   xmen_state,   empty_init, ROT0, "Konami",  "X-Men (4 Players ver AEA)",          MACHINE_SUPPORTS_SAVE )
GAME( 1992, xmenaa,  xmen, xmen,    xmen,   xmen_state,   empty_init, ROT0, "Konami",  "X-Men (4 Players ver ADA)",          MACHINE_SUPPORTS_SAVE )
GAME( 1992, xmenabl, xmen, xmenabl, xmen,   xmen_state,   empty_init, ROT0, "bootleg", "X-Men (4 Players ver AEA, bootleg)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // sprites are wrong, Oki banking not correct (doubtful it can be this bad, even being a bootleg)

GAME( 1992, xmen2pe, xmen, xmen,    xmen2p, xmen_state,   empty_init, ROT0, "Konami",  "X-Men (2 Players ver EAA)",          MACHINE_SUPPORTS_SAVE )
GAME( 1992, xmen2pu, xmen, xmen,    xmen2p, xmen_state,   empty_init, ROT0, "Konami",  "X-Men (2 Players ver UAB)",          MACHINE_SUPPORTS_SAVE )
GAME( 1992, xmen2pj, xmen, xmen,    xmen2p, xmen_state,   empty_init, ROT0, "Konami",  "X-Men (2 Players ver JAA)",          MACHINE_SUPPORTS_SAVE )
GAME( 1992, xmen2pa, xmen, xmen,    xmen2p, xmen_state,   empty_init, ROT0, "Konami",  "X-Men (2 Players ver AAA)",          MACHINE_SUPPORTS_SAVE )

GAME( 1992, xmen6p,  xmen, xmen6p,  xmen6p, xmen6p_state, empty_init, ROT0, "Konami",  "X-Men (6 Players ver ECB)",          MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1992, xmen6pu, xmen, xmen6p,  xmen6p, xmen6p_state, empty_init, ROT0, "Konami",  "X-Men (6 Players ver UCB)",          MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
