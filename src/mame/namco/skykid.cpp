// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

Dragon Buster (c) 1984 Namco
Sky Kid       (c) 1985 Namco

Driver by Manuel Abadia <emumanu+mame@gmail.com>

Notes:
-----
- Sky Kid sets:
  There are 2 basic versions, one with CUS63 and ROM prefix "SK2" and
  the other which uses CUS60 and has the ROM prefix "SK1"

***************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "cpu/m6809/m6809.h"
#include "machine/watchdog.h"
#include "sound/namco.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class skykid_state : public driver_device
{
public:
	skykid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videoram(*this, "videoram")
		, m_textram(*this, "textram")
		, m_spriteram(*this, "spriteram")
		, m_rombank(*this, "rombank")
		, m_maincpu(*this, "maincpu")
		, m_mcu(*this, "mcu")
		, m_cus30(*this, "namco")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_dsw(*this, { "DSWA", "DSWB" })
		, m_pl(*this, "P%u", 1U)
		, m_button2(*this, "BUTTON2")
		, m_system(*this, "SYSTEM")
		, m_leds(*this, "led%u", 0U)
	{ }

	void skykid(machine_config &config);

	void init_skykid();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_textram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_memory_bank m_rombank;

	required_device<cpu_device> m_maincpu;
	required_device<hd63701v0_cpu_device> m_mcu;
	required_device<namco_cus30_device> m_cus30;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_ioport_array<2> m_dsw;
	required_ioport_array<2> m_pl;
	required_ioport m_button2;
	required_ioport m_system;

	output_finder<2> m_leds;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_tx_tilemap = nullptr;
	uint8_t m_priority = 0;
	uint16_t m_scroll_x = 0;
	uint16_t m_scroll_y = 0;
	uint8_t m_main_irq_mask = 0;
	uint8_t m_mcu_irq_mask = 0;
	uint8_t m_inputport_selected = 0;

	void inputport_select_w(uint8_t data);
	uint8_t inputport_r();
	void led_w(uint8_t data);
	void subreset_w(offs_t offset, uint8_t data);
	void bankswitch_w(offs_t offset, uint8_t data);
	void irq_1_ctrl_w(offs_t offset, uint8_t data);
	void irq_2_ctrl_w(offs_t offset, uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void textram_w(offs_t offset, uint8_t data);
	void scroll_x_w(offs_t offset, uint8_t data);
	void scroll_y_w(offs_t offset, uint8_t data);
	void flipscreen_priority_w(offs_t offset, uint8_t data);
	TILEMAP_MAPPER_MEMBER(tx_tilemap_scan);
	TILE_GET_INFO_MEMBER(tx_get_tile_info);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void mcu_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};



/***************************************************************************

    Convert the color PROMs.

    The palette PROMs are connected to the RGB output this way:

    bit 3   -- 220 ohm resistor  -- RED/GREEN/BLUE
            -- 470 ohm resistor  -- RED/GREEN/BLUE
            -- 1  kohm resistor  -- RED/GREEN/BLUE
    bit 0   -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

void skykid_state::palette(palette_device &palette) const
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

	// text palette
	for (int i = 0; i < 0x100; i++)
		palette.set_pen_indirect(i, i);

	// tiles/sprites
	for (int i = 0x100; i < 0x500; i++)
	{
		uint8_t const ctabentry = color_prom[i - 0x100];
		palette.set_pen_indirect(i, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

// convert from 32x32 to 36x28
TILEMAP_MAPPER_MEMBER(skykid_state::tx_tilemap_scan)
{
	row += 2;
	col -= 2;
	if (col & 0x20)
		return row + ((col & 0x1f) << 5);
	else
		return col + (row << 5);
}

TILE_GET_INFO_MEMBER(skykid_state::tx_get_tile_info)
{
	int code = m_textram[tile_index];
	int attr = m_textram[tile_index + 0x400];
	tileinfo.category = code >> 4 & 0xf;

	/* the hardware has two character sets, one normal and one flipped. When
	   screen is flipped, character flip is done by selecting the 2nd character set.
	   We reproduce this here, but since the tilemap system automatically flips
	   characters when screen is flipped, we have to flip them back. */
	tileinfo.set(0,
			code | (flip_screen() ? 0x100 : 0),
			attr & 0x3f,
			flip_screen() ? (TILE_FLIPY | TILE_FLIPX) : 0);
}


TILE_GET_INFO_MEMBER(skykid_state::bg_get_tile_info)
{
	int code = m_videoram[tile_index];
	int attr = m_videoram[tile_index + 0x800];

	tileinfo.set(1,
			code + ((attr & 0x01) << 8),
			((attr & 0x7e) >> 1) | ((attr & 0x01) << 6),
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void skykid_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(skykid_state::tx_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(skykid_state::tx_tilemap_scan)), 8,8, 36,28);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(skykid_state::bg_get_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 64,32);

	m_tx_tilemap->set_transparent_pen(0);

	save_item(NAME(m_priority));
	save_item(NAME(m_scroll_x));
	save_item(NAME(m_scroll_y));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void skykid_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

void skykid_state::textram_w(offs_t offset, uint8_t data)
{
	m_textram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void skykid_state::scroll_x_w(offs_t offset, uint8_t data)
{
	m_scroll_x = offset;
}

void skykid_state::scroll_y_w(offs_t offset, uint8_t data)
{
	m_scroll_y = offset;
}

void skykid_state::flipscreen_priority_w(offs_t offset, uint8_t data)
{
	m_priority = data;
	flip_screen_set(offset & 1);
}



/***************************************************************************

  Display Refresh

***************************************************************************/

// the sprite generator IC is the same as Mappy
void skykid_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	uint8_t *spriteram = m_spriteram + 0x780;
	uint8_t *spriteram_2 = spriteram + 0x0800;
	uint8_t *spriteram_3 = spriteram_2 + 0x0800;

	for (int offs = 0; offs < 0x80; offs += 2)
	{
		static const int gfx_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};
		int sprite = spriteram[offs] + ((spriteram_3[offs] & 0x80) << 1);
		int color = (spriteram[offs + 1] & 0x3f);
		int sx = (spriteram_2[offs + 1]) + 0x100 * (spriteram_3[offs + 1] & 1) - 71;
		int sy = 256 - spriteram_2[offs] - 7;
		int flipx = (spriteram_3[offs] & 0x01);
		int flipy = (spriteram_3[offs] & 0x02) >> 1;
		int sizex = (spriteram_3[offs] & 0x04) >> 2;
		int sizey = (spriteram_3[offs] & 0x08) >> 3;

		sprite &= ~sizex;
		sprite &= ~(sizey << 1);

		if (flip_screen())
		{
			flipx ^= 1;
			flipy ^= 1;
		}

		sy -= 16 * sizey;
		sy = (sy & 0xff) - 32;  // fix wraparound

		for (int y = 0; y <= sizey; y++)
		{
			for (int x = 0; x <= sizex; x++)
			{
				m_gfxdecode->gfx(2)->transmask(bitmap, cliprect,
					sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
					color,
					flipx, flipy,
					sx + 16 * x, sy + 16 * y,
					m_palette->transpen_mask(*m_gfxdecode->gfx(2), color, 0xff));
			}
		}
	}
}


uint32_t skykid_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (flip_screen())
	{
		m_bg_tilemap->set_scrollx(0, 189 - (m_scroll_x ^ 1));
		m_bg_tilemap->set_scrolly(0, 7 - m_scroll_y);
	}
	else
	{
		m_bg_tilemap->set_scrollx(0, m_scroll_x + 35);
		m_bg_tilemap->set_scrolly(0, m_scroll_y + 25);
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	if (m_priority & 0x04)
	{
		// textlayer priority enabled?
		int pri = m_priority >> 4;

		// draw low priority tiles
		m_tx_tilemap->draw(screen, bitmap, cliprect, pri, 0);

		draw_sprites(bitmap, cliprect);

		// draw the other tiles
		for (int cat = 0; cat < 0x10; cat++)
			if (cat != pri) m_tx_tilemap->draw(screen, bitmap, cliprect, cat, 0);
	}
	else
	{
		draw_sprites(bitmap, cliprect);
		m_tx_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_ALL_CATEGORIES, 0);
	}

	return 0;
}


void skykid_state::inputport_select_w(uint8_t data)
{
	if ((data & 0xe0) == 0x60)
		m_inputport_selected = data & 0x07;
	else if ((data & 0xe0) == 0xc0)
	{
		machine().bookkeeping().coin_lockout_global_w(~data & 1);
		machine().bookkeeping().coin_counter_w(0, data & 2);
		machine().bookkeeping().coin_counter_w(1, data & 4);
	}
}

uint8_t skykid_state::inputport_r()
{
	switch (m_inputport_selected)
	{
		case 0x00:  // DSW B (bits 0-4)
			return (m_dsw[1]->read() & 0xf8) >> 3;
		case 0x01:  // DSW B (bits 5-7), DSW A (bits 0-1)
			return ((m_dsw[1]->read() & 0x07) << 2) | ((m_dsw[0]->read() & 0xc0) >> 6);
		case 0x02:  // DSW A (bits 2-6)
			return (m_dsw[0]->read() & 0x3e) >> 1;
		case 0x03:  // DSW A (bit 7), DSW C (bits 0-3)
			return ((m_dsw[0]->read() & 0x01) << 4) | (m_button2->read() & 0x0f);
		case 0x04:  // coins, start
			return m_system->read();
		case 0x05:  // 2P controls
			return m_pl[1]->read();
		case 0x06:  // 1P controls
			return m_pl[0]->read();
		default:
			return 0xff;
	}
}

void skykid_state::led_w(uint8_t data)
{
	m_leds[0] = BIT(data, 3);
	m_leds[1] = BIT(data, 4);
}

void skykid_state::subreset_w(offs_t offset, uint8_t data)
{
	int bit = !BIT(offset, 11);
	m_mcu->set_input_line(INPUT_LINE_RESET, bit ? CLEAR_LINE : ASSERT_LINE);
}

void skykid_state::bankswitch_w(offs_t offset, uint8_t data)
{
	m_rombank->set_entry(!BIT(offset, 11));
}

void skykid_state::irq_1_ctrl_w(offs_t offset, uint8_t data)
{
	int bit = !BIT(offset, 11);
	m_main_irq_mask = bit;
	if (!bit)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void skykid_state::irq_2_ctrl_w(offs_t offset, uint8_t data)
{
	int bit = !BIT(offset, 13);
	m_mcu_irq_mask = bit;
	if (!bit)
		m_mcu->set_input_line(0, CLEAR_LINE);
}

void skykid_state::machine_start()
{
	m_leds.resolve();

	// configure the banks
	m_rombank->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x2000);

	save_item(NAME(m_inputport_selected));
}



void skykid_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).bankr(m_rombank);
	map(0x2000, 0x2fff).ram().w(FUNC(skykid_state::videoram_w)).share(m_videoram); // background
	map(0x4000, 0x47ff).ram().w(FUNC(skykid_state::textram_w)).share(m_textram);
	map(0x4800, 0x5fff).ram().share(m_spriteram);
	map(0x6000, 0x60ff).w(FUNC(skykid_state::scroll_y_w));
	map(0x6200, 0x63ff).w(FUNC(skykid_state::scroll_x_w));
	map(0x6800, 0x6bff).rw(m_cus30, FUNC(namco_cus30_device::namcos1_cus30_r), FUNC(namco_cus30_device::namcos1_cus30_w)); // PSG device, shared RAM
	map(0x7000, 0x7fff).w(FUNC(skykid_state::irq_1_ctrl_w));
	map(0x7800, 0x7fff).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0x8000, 0xffff).rom();
	map(0x8000, 0x8fff).w(FUNC(skykid_state::subreset_w));
	map(0x9000, 0x9fff).w(FUNC(skykid_state::bankswitch_w));
	map(0xa000, 0xa001).w(FUNC(skykid_state::flipscreen_priority_w));
}

void skykid_state::mcu_map(address_map &map)
{
	map(0x1000, 0x13ff).rw(m_cus30, FUNC(namco_cus30_device::namcos1_cus30_r), FUNC(namco_cus30_device::namcos1_cus30_w)); // PSG device, shared RAM/
	map(0x2000, 0x3fff).w("watchdog", FUNC(watchdog_timer_device::reset_w));     // ?
	map(0x4000, 0x7fff).w(FUNC(skykid_state::irq_2_ctrl_w));
	map(0x8000, 0x9fff).rom().region("mcusub", 0);
	map(0xc000, 0xc7ff).ram();
}



static INPUT_PORTS_START( skykid )
	PORT_START("DSWA")  // DSW A
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWA:1" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:3,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Round Skip" ) PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Freeze" ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:8,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )

	PORT_START("DSWB")  // DSW B
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:2,1")
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:4,3")
	PORT_DIPSETTING(    0x00, "20k every 80k" )
	PORT_DIPSETTING(    0x10, "20k and 80k" )
	PORT_DIPSETTING(    0x20, "30k every 90k" )
	PORT_DIPSETTING(    0x30, "30k and 90k" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SWB:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SWB:6" )
	PORT_DIPNAME( 0x02, 0x02, "Allow Buy In" ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BUTTON2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")    // IN 0
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")    // IN 1
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")    // IN 2
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( skykids )
	PORT_INCLUDE( skykid )

	PORT_MODIFY("DSWA")  // DSW A
	PORT_DIPNAME( 0x08, 0x08, "Round Select" ) PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )

	PORT_MODIFY("DSWB")  // DSW B
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:4,3")
	PORT_DIPSETTING(    0x00, "30k every 80k" )
	PORT_DIPSETTING(    0x10, "30k and 80k" )
	PORT_DIPSETTING(    0x20, "40k every 90k" )
	PORT_DIPSETTING(    0x30, "40k and 90k" )
INPUT_PORTS_END

static INPUT_PORTS_START( drgnbstr )
	PORT_START("DSWA")  // DSW A
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWA:1" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:3,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Round Skip" ) PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Freeze" ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:8,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )

	PORT_START("DSWB")  // DSW B
	PORT_DIPNAME( 0x80, 0x80, "Spurt Time" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x40, 0x40, "Level of Monster" ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x30, 0x30, "Starting Vitality" ) PORT_DIPLOCATION("SWB:4,3")
	PORT_DIPSETTING(    0x00, "160" )
	PORT_DIPSETTING(    0x30, "128" )
	PORT_DIPSETTING(    0x10, "96" )
	PORT_DIPSETTING(    0x20, "64" )
	PORT_DIPNAME( 0x0c, 0x0c, "Bonus Vitality" ) PORT_DIPLOCATION("SWB:6,5") /* Clear Mountain, Tower, ect... */
	PORT_DIPSETTING(    0x00, "64" )
	PORT_DIPSETTING(    0x08, "48/64" )
	PORT_DIPSETTING(    0x04, "32/64" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x02, 0x02, "Bonus Level" ) PORT_DIPLOCATION("SWB:7")    /* Clear Round */
	PORT_DIPSETTING(    0x02, "Full" )
	PORT_DIPSETTING(    0x00, "Partial" )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("BUTTON2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")    // IN 0
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")    // IN 1
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")    // IN 2
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static const gfx_layout text_layout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2)+4, 0, 4 },
	{ 0, 1, 2, 3, 8*8, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};

static GFXDECODE_START( gfx_skykid )
	GFXDECODE_ENTRY( "chars",   0, text_layout,   0, 64 )
	GFXDECODE_ENTRY( "tiles",   0, tile_layout,   64*4, 128 )
	GFXDECODE_ENTRY( "sprites", 0, sprite_layout, 64*4+128*4, 64 )
GFXDECODE_END


void skykid_state::vblank_irq(int state)
{
	if (state && m_main_irq_mask)
		m_maincpu->set_input_line(0, ASSERT_LINE);

	if (state && m_mcu_irq_mask)
		m_mcu->set_input_line(0, ASSERT_LINE);
}


void skykid_state::skykid(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, XTAL(49'152'000) / 32);
	m_maincpu->set_addrmap(AS_PROGRAM, &skykid_state::main_map);

	HD63701V0(config, m_mcu, XTAL(49'152'000) / 8); // or compatible 6808 with extra instructions
	m_mcu->set_addrmap(AS_PROGRAM, &skykid_state::mcu_map);
	m_mcu->in_p1_cb().set(FUNC(skykid_state::inputport_r));
	m_mcu->out_p1_cb().set(FUNC(skykid_state::inputport_select_w));
	m_mcu->in_p2_cb().set_constant(0xff);                           // leds won't work otherwise
	m_mcu->out_p2_cb().set(FUNC(skykid_state::led_w));

	config.set_maximum_quantum(attotime::from_hz(6000));  // we need heavy synch

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60.606060);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(36*8, 28*8);
	screen.set_visarea(0*8, 36*8-1, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(skykid_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(skykid_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_skykid);
	PALETTE(config, m_palette, FUNC(skykid_state::palette), 64*4 + 128*4 + 64*8, 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	NAMCO_CUS30(config, m_cus30, 49152000 / 2048);
	m_cus30->set_voices(8);
	m_cus30->add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( skykid ) // a PCB was found with ROM 4 and 6 labeled sk1, but hashes match the sk2 listed here and in other sets, while they differ from the sk1 ROMs in set skykidd?
	ROM_REGION( 0x14000, "maincpu", 0 ) // 6809 code
	ROM_LOAD( "sk2_2.6c",     0x08000, 0x4000, CRC(ea8a5822) SHA1(5b13133410bcb7d647e662b476dbfd2edab8aac0) )
	ROM_LOAD( "sk1-1c.6b",    0x0c000, 0x4000, CRC(7abe6c6c) SHA1(7d2631cc6149fa3e02b1355cb899de5474ff5d0a) )
	ROM_LOAD( "sk1_3.6d",     0x10000, 0x4000, CRC(314b8765) SHA1(d90a8a853ce672fe5ee190f07bcb33262c73df3b) )   // banked ROM

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "cus63-63a1.mcu", 0x0000, 0x1000, CRC(6ef08fb3) SHA1(4842590d60035a0059b0899eb2d5f58ae72c2529) )  // MCU internal code

	ROM_REGION( 0x2000, "mcusub", 0 )
	ROM_LOAD( "sk2_4.3c",     0x0000, 0x2000, CRC(a460d0e0) SHA1(7124ffeb3b84b282940dcbf9421ae4934bcce1c8) )  // subprogram for the MCU

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "sk1_6.6l",     0x0000, 0x2000, CRC(58b731b9) SHA1(40f7be85914833ce02a734c20d68c0db8b77911d) )

	ROM_REGION( 0x02000, "tiles", 0 )
	ROM_LOAD( "sk1_5.7e",     0x0000, 0x2000, CRC(c33a498e) SHA1(9f89a514888418a9bebbca341a8cc66e41b58acb) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "sk1_8.10n",    0x0000, 0x4000, CRC(44bb7375) SHA1(5b2fa6782671150bab5f3c3ac46b47bc23f3d7e0) )
	ROM_LOAD( "sk1_7.10m",    0x4000, 0x4000, CRC(3454671d) SHA1(723b26a0f208addc2a22736457cb4be6ab6c69cc) )
	// 0x8000-0xbfff  will be unpacked from 0x4000-0x5fff
	ROM_FILL(                 0xc000, 0x4000, 0x00 )    // part of the gfx is 2bpp decoded as 3bpp

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "sk1-1.2n",     0x0000, 0x0100, CRC(0218e726) SHA1(8b766162a4783c058d9a1ecf8741673d7ef955fb) )    // red component
	ROM_LOAD( "sk1-2.2p",     0x0100, 0x0100, CRC(fc0d5b85) SHA1(d1b13e42e735b24594cf0b840dee8110de23369e) )    // green component
	ROM_LOAD( "sk1-3.2r",     0x0200, 0x0100, CRC(d06b620b) SHA1(968a2d62c65e201d521e9efa8fcf6ad15898e4b3) )    // blue component
	ROM_LOAD( "sk1-4.5n",     0x0300, 0x0200, CRC(c697ac72) SHA1(3b79755e6cbb22c14fc4affdbd3f4521da1d90e8) )    // tiles lookup table
	ROM_LOAD( "sk1-5.6n",     0x0500, 0x0200, CRC(161514a4) SHA1(4488ce60d12be6586e4a1ddbbfd06bf4e7dfaceb) )    // sprites lookup table
ROM_END

ROM_START( skykido )
	ROM_REGION( 0x14000, "maincpu", 0 ) // 6809 code
	ROM_LOAD( "sk2_2.6c",     0x08000, 0x4000, CRC(ea8a5822) SHA1(5b13133410bcb7d647e662b476dbfd2edab8aac0) )
	ROM_LOAD( "sk1_1.6b",     0x0c000, 0x4000, CRC(070a49d4) SHA1(4b994bde3e34b574bd927843804d2fb1a08d1bdf) )
	ROM_LOAD( "sk1_3.6d",     0x10000, 0x4000, CRC(314b8765) SHA1(d90a8a853ce672fe5ee190f07bcb33262c73df3b) )   // banked ROM

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "cus63-63a1.mcu", 0x0000, 0x1000, CRC(6ef08fb3) SHA1(4842590d60035a0059b0899eb2d5f58ae72c2529) )  // MCU internal code

	ROM_REGION( 0x2000, "mcusub", 0 )
	ROM_LOAD( "sk2_4.3c",     0x0000, 0x2000, CRC(a460d0e0) SHA1(7124ffeb3b84b282940dcbf9421ae4934bcce1c8) )  // subprogram for the MCU

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "sk1_6.6l",     0x0000, 0x2000, CRC(58b731b9) SHA1(40f7be85914833ce02a734c20d68c0db8b77911d) )

	ROM_REGION( 0x02000, "tiles", 0 )
	ROM_LOAD( "sk1_5.7e",     0x0000, 0x2000, CRC(c33a498e) SHA1(9f89a514888418a9bebbca341a8cc66e41b58acb) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "sk1_8.10n",    0x0000, 0x4000, CRC(44bb7375) SHA1(5b2fa6782671150bab5f3c3ac46b47bc23f3d7e0) )
	ROM_LOAD( "sk1_7.10m",    0x4000, 0x4000, CRC(3454671d) SHA1(723b26a0f208addc2a22736457cb4be6ab6c69cc) )
	// 0x8000-0xbfff  will be unpacked from 0x4000-0x5fff
	ROM_FILL(                 0xc000, 0x4000, 0x00 )    // part of the gfx is 2bpp decoded as 3bpp

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "sk1-1.2n",     0x0000, 0x0100, CRC(0218e726) SHA1(8b766162a4783c058d9a1ecf8741673d7ef955fb) )    // red component
	ROM_LOAD( "sk1-2.2p",     0x0100, 0x0100, CRC(fc0d5b85) SHA1(d1b13e42e735b24594cf0b840dee8110de23369e) )    // green component
	ROM_LOAD( "sk1-3.2r",     0x0200, 0x0100, CRC(d06b620b) SHA1(968a2d62c65e201d521e9efa8fcf6ad15898e4b3) )    // blue component
	ROM_LOAD( "sk1-4.5n",     0x0300, 0x0200, CRC(c697ac72) SHA1(3b79755e6cbb22c14fc4affdbd3f4521da1d90e8) )    // tiles lookup table
	ROM_LOAD( "sk1-5.6n",     0x0500, 0x0200, CRC(161514a4) SHA1(4488ce60d12be6586e4a1ddbbfd06bf4e7dfaceb) )    // sprites lookup table
ROM_END

ROM_START( skykidd )
	ROM_REGION( 0x14000, "maincpu", 0 ) // 6809 code
	ROM_LOAD( "sk1_2.6c",     0x08000, 0x4000, CRC(8370671a) SHA1(7038f952ebfc4482440b73ee4027fa908561d122) )
	ROM_LOAD( "sk1_1.6b",     0x0c000, 0x4000, CRC(070a49d4) SHA1(4b994bde3e34b574bd927843804d2fb1a08d1bdf) )
	ROM_LOAD( "sk1_3.6d",     0x10000, 0x4000, CRC(314b8765) SHA1(d90a8a853ce672fe5ee190f07bcb33262c73df3b) )   // banked ROM

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "cus60-60a1.mcu", 0x0000, 0x1000, CRC(076ea82a) SHA1(22b5e62e26390d7d5cacc0503c7aa5ed524204df) )  // MCU internal code

	ROM_REGION( 0x2000, "mcusub", 0 )
	ROM_LOAD( "sk1_4.3c",     0x0000, 0x2000, CRC(887137cc) SHA1(dd0f66afb78833c4da73539b692854346f448c0d) )  // subprogram for the MCU

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "sk1_6.6l",     0x0000, 0x2000, CRC(58b731b9) SHA1(40f7be85914833ce02a734c20d68c0db8b77911d) )

	ROM_REGION( 0x02000, "tiles", 0 )
	ROM_LOAD( "sk1_5.7e",     0x0000, 0x2000, CRC(c33a498e) SHA1(9f89a514888418a9bebbca341a8cc66e41b58acb) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "sk1_8.10n",    0x0000, 0x4000, CRC(44bb7375) SHA1(5b2fa6782671150bab5f3c3ac46b47bc23f3d7e0) )
	ROM_LOAD( "sk1_7.10m",    0x4000, 0x4000, CRC(3454671d) SHA1(723b26a0f208addc2a22736457cb4be6ab6c69cc) )
	// 0x8000-0xbfff  will be unpacked from 0x4000-0x5fff
	ROM_FILL(                 0xc000, 0x4000, 0x00 )    // part of the gfx is 2bpp decoded as 3bpp

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "sk1-1.2n",     0x0000, 0x0100, CRC(0218e726) SHA1(8b766162a4783c058d9a1ecf8741673d7ef955fb) )    // red component
	ROM_LOAD( "sk1-2.2p",     0x0100, 0x0100, CRC(fc0d5b85) SHA1(d1b13e42e735b24594cf0b840dee8110de23369e) )    // green component
	ROM_LOAD( "sk1-3.2r",     0x0200, 0x0100, CRC(d06b620b) SHA1(968a2d62c65e201d521e9efa8fcf6ad15898e4b3) )    // blue component
	ROM_LOAD( "sk1-4.5n",     0x0300, 0x0200, CRC(c697ac72) SHA1(3b79755e6cbb22c14fc4affdbd3f4521da1d90e8) )    // tiles lookup table
	ROM_LOAD( "sk1-5.6n",     0x0500, 0x0200, CRC(161514a4) SHA1(4488ce60d12be6586e4a1ddbbfd06bf4e7dfaceb) )    // sprites lookup table
ROM_END

ROM_START( skykids )
	ROM_REGION( 0x14000, "maincpu", 0 ) // 6809 code
	ROM_LOAD( "sk2a.6c",     0x08000, 0x4000, CRC(68492672) SHA1(3dbe5ec930de5c526d3ef65513993c10f2153a36) )
	ROM_LOAD( "sk1a.6b",     0x0c000, 0x4000, CRC(e16abe25) SHA1(78e0d30b15fb62c4399d847784ddc61f6819feba) )
	ROM_LOAD( "sk1_3.6d",     0x10000, 0x4000, CRC(314b8765) SHA1(d90a8a853ce672fe5ee190f07bcb33262c73df3b) )   // banked ROM

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "cus63-63a1.mcu", 0x0000, 0x1000, CRC(6ef08fb3) SHA1(4842590d60035a0059b0899eb2d5f58ae72c2529) )  // MCU internal code

	ROM_REGION( 0x2000, "mcusub", 0 )
	ROM_LOAD( "sk2_4.3c",     0x0000, 0x2000, CRC(a460d0e0) SHA1(7124ffeb3b84b282940dcbf9421ae4934bcce1c8) )  // subprogram for the MCU

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "sk1_6.6l",     0x0000, 0x2000, CRC(58b731b9) SHA1(40f7be85914833ce02a734c20d68c0db8b77911d) )

	ROM_REGION( 0x02000, "tiles", 0 )
	ROM_LOAD( "sk1_5.7e",     0x0000, 0x2000, CRC(c33a498e) SHA1(9f89a514888418a9bebbca341a8cc66e41b58acb) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "sk1_8.10n",    0x0000, 0x4000, CRC(44bb7375) SHA1(5b2fa6782671150bab5f3c3ac46b47bc23f3d7e0) )
	ROM_LOAD( "sk1_7.10m",    0x4000, 0x4000, CRC(3454671d) SHA1(723b26a0f208addc2a22736457cb4be6ab6c69cc) )
	// 0x8000-0xbfff  will be unpacked from 0x4000-0x5fff
	ROM_FILL(                 0xc000, 0x4000, 0x00 )    // part of the gfx is 2bpp decoded as 3bpp

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "sk1-1.2n",     0x0000, 0x0100, CRC(0218e726) SHA1(8b766162a4783c058d9a1ecf8741673d7ef955fb) )    // red component
	ROM_LOAD( "sk1-2.2p",     0x0100, 0x0100, CRC(fc0d5b85) SHA1(d1b13e42e735b24594cf0b840dee8110de23369e) )    // green component
	ROM_LOAD( "sk1-3.2r",     0x0200, 0x0100, CRC(d06b620b) SHA1(968a2d62c65e201d521e9efa8fcf6ad15898e4b3) )    // blue component
	ROM_LOAD( "sk1-4.5n",     0x0300, 0x0200, CRC(c697ac72) SHA1(3b79755e6cbb22c14fc4affdbd3f4521da1d90e8) )    // tiles lookup table
	ROM_LOAD( "sk1-5.6n",     0x0500, 0x0200, CRC(161514a4) SHA1(4488ce60d12be6586e4a1ddbbfd06bf4e7dfaceb) )    // sprites lookup table
ROM_END

ROM_START( drgnbstr )
	ROM_REGION( 0x14000, "maincpu", 0 ) // 6809 code
	ROM_LOAD( "db1_2b.6c",    0x08000, 0x04000, CRC(0f11cd17) SHA1(691d853f4f08898ecf4bccfb70a568de309329f1) )
	ROM_LOAD( "db1_1.6b",     0x0c000, 0x04000, CRC(1c7c1821) SHA1(8b6111afc42e2996bdc2fc276be0c40556cd431e) )
	ROM_LOAD( "db1_3.6d",     0x10000, 0x04000, CRC(6da169ae) SHA1(235211c26562fef0660e3fde1e87f2e52626d119) )  // banked ROM

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "cus60-60a1.mcu", 0x0000, 0x1000, CRC(076ea82a) SHA1(22b5e62e26390d7d5cacc0503c7aa5ed524204df) ) // MCU internal code

	ROM_REGION( 0x2000, "mcusub", 0 )
	ROM_LOAD( "db1_4.3c",     0x0000, 0x2000, CRC(8a0b1fc1) SHA1(c2861d0da63e2d17f2d1ad46dccf753ecd902ce3) ) // subprogram for the MCU

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "db1_6.6l",     0x0000, 0x2000, CRC(c080b66c) SHA1(05dcd45274d0bd12ef8ae7fd10c8719e679b3e7b) )

	ROM_REGION( 0x02000, "tiles", 0 )
	ROM_LOAD( "db1_5.7e",     0x0000, 0x2000, CRC(28129aed) SHA1(d7f52e871d97179ec88c142a1c70eb6ad09e534a) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "db1_8.10n",    0x0000, 0x4000, CRC(11942c61) SHA1(0f065cb82cf83967e90b3c7326b36956f4fa9a52) )
	ROM_LOAD( "db1_7.10m",    0x4000, 0x4000, CRC(cc130fe2) SHA1(4f5d4f21152b3b4e523a6d17dd5ff5cef52447f2) )
	// 0x8000-0xbfff  will be unpacked from 0x4000-0x5fff
	ROM_FILL(                 0xc000, 0x4000, 0x00 )    // part of the gfx is 2bpp decoded as 3bpp

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "db1-1.2n",     0x0000, 0x0100, CRC(3f8cce97) SHA1(027b3fb0f322a9d68b434b207a40b31799a8a8d6) )    // red component
	ROM_LOAD( "db1-2.2p",     0x0100, 0x0100, CRC(afe32436) SHA1(e405787f7f2aa992edd63078e3944334d8acddb1) )    // green component
	ROM_LOAD( "db1-3.2r",     0x0200, 0x0100, CRC(c95ff576) SHA1(861a7340d29e6a6a0d5ead93abd3f73cc3df0cc7) )    // blue component
	ROM_LOAD( "db1-4.5n",     0x0300, 0x0200, CRC(b2180c21) SHA1(a5d14c31d54f04494ea99c3d94bd1b5e072b612e) )    // tiles lookup table
	ROM_LOAD( "db1-5.6n",     0x0500, 0x0200, CRC(5e2b3f74) SHA1(ef58661fa12a52bc358e81179254d37de7551b38) )    // sprites lookup table
ROM_END



void skykid_state::init_skykid()
{
	// unpack the third sprite ROM
	uint8_t *rom = memregion("sprites")->base() + 0x4000;
	for (int i = 0; i < 0x2000; i++)
	{
		rom[i + 0x4000] = rom[i];       // sprite set #1, plane 3
		rom[i + 0x6000] = rom[i] >> 4;  // sprite set #2, plane 3

		rom[i] = rom[i + 0x2000];       // sprite set #3, planes 1&2 (plane 3 is empty)
	}
}

} // anonymous namespace


GAME( 1984, drgnbstr, 0,      skykid, drgnbstr, skykid_state, init_skykid, ROT0,   "Namco", "Dragon Buster", MACHINE_SUPPORTS_SAVE )
GAME( 1985, skykid,   0,      skykid, skykid,   skykid_state, init_skykid, ROT180, "Namco", "Sky Kid (new version)", MACHINE_SUPPORTS_SAVE ) // Uses CUS63 aka 63a1
GAME( 1985, skykido,  skykid, skykid, skykid,   skykid_state, init_skykid, ROT180, "Namco", "Sky Kid (old version)", MACHINE_SUPPORTS_SAVE ) // Uses CUS63 aka 63a1
GAME( 1985, skykidd,  skykid, skykid, skykid,   skykid_state, init_skykid, ROT180, "Namco", "Sky Kid (CUS60 version)", MACHINE_SUPPORTS_SAVE ) // Uses CUS60 aka 60a1

// no license text is displayed but the PCB was licensed by Namco for production by Sipem (formerly Sidam) with Namco supplying the Custom chips (MCU etc.)
// the level select is handled in a much more user-friendly way in this set and the dip for it is inverted (although this is displayed incorrectly in the test mode)
GAME( 1985, skykids,  skykid, skykid, skykids,  skykid_state, init_skykid, ROT180, "Namco (Sipem license)", "Sky Kid (Sipem)", MACHINE_SUPPORTS_SAVE ) // Uses CUS63 aka 63a1
