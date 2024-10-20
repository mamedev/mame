// license:BSD-3-Clause
// copyright-holders: Manuel Abadia

/***************************************************************************

Baraduke    (c) 1985 Namco
Metro-Cross (c) 1985 Namco

Driver by Manuel Abadia <emumanu+mame@gmail.com>

The sprite and tilemap generator ICs are the same as in Namco System 86, but
System 86 has two tilemap generators instead of one.

Custom ICs:
----------
98XX     lamp/coin output
99XX     sound volume
CUS27    ULA clock divider
CUS30    sound control
CUS31    ULA
CUS39    ULA sprite generator
CUS41    address decoder
CUS42    dual scrolling tilemap address generator
CUS43    ULA dual tilemap generator
CUS48    ULA sprite address generator
CUS60    MCU (63701) aka 60A1


Baraduke
Namco, 1985

PCB Layout
----------

 2248961100
(2248963100)
|---------------------------------------------------|
|           BD1_8.4P                6116            |
|       43                                          |
|PR1.1N     BD1_7.4N       42       BD1_12.8N  2148 |
|                                                   |
|  PR2.2M   BD1_6.4M                BD1_11.8M  2148 |
|                                                   |
|                                   BD1_10.8L  2148 |
|2  DSWA DSWB                                       |
|2  31                    27        BD1_9.8K   39   |
|W      BD1_5.3J   49.152MHz                        |
|A                        6116                      |
|Y                                                  |
|       2148                                   48   |
|       2148     30                6116             |
|                                  6116             |
|                                  6116             |
|       6116                       6116   BD1_3.9C  |
|                                                   |
|       BD1_4B.3B                         BD1_2.9B  |
|                                                   |
|LA4460     60A1         41        6809   BD1_1.9A  |
|---------------------------------------------------|
Notes:
      Clocks:
      6809 clock : 1.536MHz (49.152 / 32)
      63701 clock: 1.536MHz (49.152 / 32)
      VSync      : 60.606060Hz

      RAMs:
      6116: 2K x8 SRAM
      2148: 1K x4 SRAM

      Namco Customs:
      27   : DIP40, manufactured by Fujitsu)
      30   : (SDIP64)
      31   : (DIP48, also written on chip is '218' and '5201')
      39   : (DIP42, manufactured by Fujitsu)
      41   : (DIP40, also written on chip is '8512MAD' and 'PHILIPPINES')
      42   : (QFP80, the only chip on the PCB that is surface-mounted)
      60A1 : (DIP40, known 63701 MCU)
      43   : (SDIP64)
      48   : (SDIP64)

      ROMs:
      PR1.1N is a PROM type MB7138E
      PR2.2M is a PROM type MB7128E
      BD1_3.9C & BD1_5.3J are 2764 EPROMs. All other ROMs are 27128 EPROMs.



Notes:
-----
- in floor 6 of baraduke, there are gaps in the big boss when the player moves.
  This is the correct behaviour, verified on the real board.

TODO:
----
- The two unknown writes for the MCU are probably watchdog reset and irq acknowledge,
  but they don't seem to work as expected. During the first few frames they are
  written out of order and hooking them up in the usual way causes the MCU to
  stop receiving interrupts.

- remove the sound kludge in Baraduke. This might actually be a feature of the
  CUS30 chip.


DIP locations verified for:
--------------------------
- baraduke (manual JP)
- metrocrs (manual US, JP)

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

class baraduke_state : public driver_device
{
public:
	baraduke_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_textram(*this, "textram"),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_cus30(*this, "namco"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_in(*this, "IN%u", 0U),
		m_dsw(*this, { "DSWA", "DSWB"}),
		m_edge(*this, "EDGE"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void init_baraduke();
	void baraduke(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void inputport_select_w(uint8_t data);
	uint8_t inputport_r();
	void lamps_w(uint8_t data);
	void irq_ack_w(uint8_t data);
	uint8_t soundkludge_r();
	void videoram_w(offs_t offset, uint8_t data);
	void textram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void scroll_w(offs_t offset, uint8_t data);
	void spriteram_w(offs_t offset, uint8_t data);
	TILEMAP_MAPPER_MEMBER(tx_tilemap_scan);
	TILE_GET_INFO_MEMBER(tx_get_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_scroll(int layer);
	void main_map(address_map &map) ATTR_COLD;
	void mcu_map(address_map &map) ATTR_COLD;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_textram;

	required_device<cpu_device> m_maincpu;
	required_device<hd63701v0_cpu_device> m_mcu;
	required_device<namco_cus30_device> m_cus30;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_ioport_array<3> m_in;
	required_ioport_array<2> m_dsw;
	required_ioport m_edge;
	output_finder<2> m_lamps;

	uint8_t m_inputport_selected = 0;
	uint16_t m_counter = 0;
	tilemap_t *m_tx_tilemap = nullptr;
	tilemap_t *m_bg_tilemap[2]{};
	uint16_t m_xscroll[2]{};
	uint8_t m_yscroll[2]{};
	bool m_copy_sprites = false;
};


/***************************************************************************

    Convert the color PROMs.

    The palette PROMs are connected to the RGB output this way:

    bit 3   -- 220 ohm resistor  -- RED/GREEN/BLUE
            -- 470 ohm resistor  -- RED/GREEN/BLUE
            -- 1  kohm resistor  -- RED/GREEN/BLUE
    bit 0   -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

void baraduke_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < 2048; i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[2048], 0);
		bit1 = BIT(color_prom[2048], 1);
		bit2 = BIT(color_prom[2048], 2);
		bit3 = BIT(color_prom[2048], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// green component
		bit0 = BIT(color_prom[0], 0);
		bit1 = BIT(color_prom[0], 1);
		bit2 = BIT(color_prom[0], 2);
		bit3 = BIT(color_prom[0], 3);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// blue component
		bit0 = BIT(color_prom[0], 4);
		bit1 = BIT(color_prom[0], 5);
		bit2 = BIT(color_prom[0], 6);
		bit3 = BIT(color_prom[0], 7);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}



/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

// convert from 32x32 to 36x28
TILEMAP_MAPPER_MEMBER(baraduke_state::tx_tilemap_scan)
{
	row += 2;
	col -= 2;
	if (col & 0x20)
		return row + ((col & 0x1f) << 5);
	else
		return col + (row << 5);
}

TILE_GET_INFO_MEMBER(baraduke_state::tx_get_tile_info)
{
	tileinfo.set(0,
			m_textram[tile_index],
			(m_textram[tile_index + 0x400] << 2) & 0x1ff,
			0);
}

TILE_GET_INFO_MEMBER(baraduke_state::get_tile_info0)
{
	int const code = m_videoram[2 * tile_index];
	int const attr = m_videoram[2 * tile_index + 1];

	tileinfo.set(1,
			code + ((attr & 0x03) << 8),
			attr,
			0);
}

TILE_GET_INFO_MEMBER(baraduke_state::get_tile_info1)
{
	int const code = m_videoram[0x1000 + 2 * tile_index];
	int const attr = m_videoram[0x1000 + 2 * tile_index + 1];

	tileinfo.set(2,
			code + ((attr & 0x03) << 8),
			attr,
			0);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void baraduke_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(baraduke_state::tx_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(baraduke_state::tx_tilemap_scan)), 8, 8, 36, 28);
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(baraduke_state::get_tile_info0)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(baraduke_state::get_tile_info1)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tx_tilemap->set_transparent_pen(3);
	m_bg_tilemap[0]->set_transparent_pen(7);
	m_bg_tilemap[1]->set_transparent_pen(7);

	m_bg_tilemap[0]->set_scrolldx(-26, -227 + 26);
	m_bg_tilemap[1]->set_scrolldx(-24, -227 + 24);
	m_bg_tilemap[0]->set_scrolldy(-9, 9);
	m_bg_tilemap[1]->set_scrolldy(-9, 9);
	m_tx_tilemap->set_scrolldy(16, 16);

	save_item(NAME(m_xscroll));
	save_item(NAME(m_yscroll));
	save_item(NAME(m_copy_sprites));
}



/***************************************************************************

    Memory handlers

***************************************************************************/

void baraduke_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap[offset / 0x1000]->mark_tile_dirty((offset & 0xfff) / 2);
}

void baraduke_state::textram_w(offs_t offset, uint8_t data)
{
	m_textram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x3ff);
}

template <uint8_t Which>
void baraduke_state::scroll_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: // high scroll x
			m_xscroll[Which] = (m_xscroll[Which] & 0xff) | (data << 8);
			break;
		case 1: // low scroll x
			m_xscroll[Which] = (m_xscroll[Which] & 0xff00) | data;
			break;
		case 2: // scroll y
			m_yscroll[Which] = data;
			break;
	}
}

void baraduke_state::spriteram_w(offs_t offset, uint8_t data)
{
	m_spriteram[offset] = data;

	// a write to this offset tells the sprite chip to buffer the sprite list
	if (offset == 0x1ff2)
		m_copy_sprites = true;
}



/***************************************************************************

    Display Refresh

***************************************************************************/

void baraduke_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *spriteram = m_spriteram + 0x1800;
	const uint8_t *source = &spriteram[0x0800 - 32];    // the last is NOT a sprite
	const uint8_t *finish = &spriteram[0x0000];

	int const sprite_xoffs = spriteram[0x07f5] - 256 * (spriteram[0x07f4] & 1);
	int const sprite_yoffs = spriteram[0x07f7];

	static constexpr int gfx_offs[2][2] =
	{
		{ 0, 1 },
		{ 2, 3 }
	};

	while (source >= finish)
	{
/*
    source[10] S-FT ---P
    source[11] TTTT TTTT
    source[12] CCCC CCCX
    source[13] XXXX XXXX
    source[14] ---T -S-F
    source[15] YYYY YYYY
*/
		int const priority = source[10] & 0x01;
		uint32_t const pri_mask = priority ? 0 : GFX_PMASK_2;
		int const attr1 = source[10];
		int const attr2 = source[14];
		int color = source[12];
		int sx = source[13] + (color & 0x01) * 256;
		int sy = 240 - source[15];
		int flipx = BIT(attr1, 5);
		int flipy = BIT(attr2, 0);
		int const sizex = BIT(attr1, 7);
		int const sizey = BIT(attr2, 2);
		int sprite = (source[11] & 0xff) * 4;

		if (BIT(attr1, 4) && !sizex) sprite += 1;
		if (BIT(attr2, 4) && !sizey) sprite += 2;
		color = color >> 1;

		sx += sprite_xoffs;
		sy -= sprite_yoffs;

		sy -= 16 * sizey;

		if (flip_screen())
		{
			sx = 496 + 3 - 16 * sizex - sx;
			sy = 240 - 16 * sizey - sy;
			flipx ^= 1;
			flipy ^= 1;
		}

		for (int y = 0; y <= sizey; y++)
		{
			for (int x = 0; x <= sizex; x++)
			{
				m_gfxdecode->gfx(3)->prio_transpen(bitmap, cliprect,
					sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
					color,
					flipx, flipy,
					-71 + ((sx + 16 * x) & 0x1ff),
					1 + ((sy + 16 * y) & 0xff),
					screen.priority(), pri_mask, 0xf);
			}
		}

		source -= 16;
	}
}


void baraduke_state::set_scroll(int layer)
{
	int scrollx = m_xscroll[layer];
	int scrolly = m_yscroll[layer];

	if (flip_screen())
	{
		scrollx = -scrollx;
		scrolly = -scrolly;
	}

	m_bg_tilemap[layer]->set_scrollx(0, scrollx);
	m_bg_tilemap[layer]->set_scrolly(0, scrolly);
}


uint32_t baraduke_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	// flip screen is embedded in the sprite control registers
	flip_screen_set(m_spriteram[0x1ff6] & 0x01);
	set_scroll(0);
	set_scroll(1);

	int back;

	if (((m_xscroll[0] & 0x0e00) >> 9) == 6)
		back = 1;
	else
		back = 0;

	m_bg_tilemap[back]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
	m_bg_tilemap[back ^ 1]->draw(screen, bitmap, cliprect, 0, 2);
	draw_sprites(screen, bitmap, cliprect);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void baraduke_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		if (m_copy_sprites)
		{
			for (int i = 0x1800; i < 0x2000; i += 16)
			{
				for (int j = 10; j < 16; j++)
					m_spriteram[i + j] = m_spriteram[i + j - 6];
			}

			m_copy_sprites = false;
		}

		m_maincpu->set_input_line(0, ASSERT_LINE);
		m_mcu->set_input_line(0, HOLD_LINE);
	}
}


void baraduke_state::inputport_select_w(uint8_t data)
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

uint8_t baraduke_state::inputport_r()
{
	switch (m_inputport_selected)
	{
		case 0x00:  // DSW A (bits 0-4)
			return (m_dsw[0]->read() & 0xf8) >> 3;
		case 0x01:  // DSW A (bits 5-7), DSW B (bits 0-1)
			return ((m_dsw[0]->read() & 0x07) << 2) | ((m_dsw[1]->read() & 0xc0) >> 6);
		case 0x02:  // DSW B (bits 2-6)
			return (m_dsw[1]->read() & 0x3e) >> 1;
		case 0x03:  // DSW B (bit 7), DSW C (bits 0-3)
			return ((m_dsw[1]->read() & 0x01) << 4) | (m_edge->read() & 0x0f);
		case 0x04:  // coins, start
			return m_in[0]->read();
		case 0x05:  // 2P controls
			return m_in[2]->read();
		case 0x06:  // 1P controls
			return m_in[1]->read();
		default:
			return 0xff;
	}
}

void baraduke_state::lamps_w(uint8_t data)
{
	m_lamps[0] = BIT(data, 3);
	m_lamps[1] = BIT(data, 4);
}

void baraduke_state::irq_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}



void baraduke_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().w(FUNC(baraduke_state::spriteram_w)).share(m_spriteram);
	map(0x2000, 0x3fff).ram().w(FUNC(baraduke_state::videoram_w)).share(m_videoram);
	map(0x4000, 0x43ff).rw(m_cus30, FUNC(namco_cus30_device::namcos1_cus30_r), FUNC(namco_cus30_device::namcos1_cus30_w)); // PSG device, shared RAM
	map(0x4800, 0x4fff).ram().w(FUNC(baraduke_state::textram_w)).share(m_textram);
	map(0x8000, 0x8000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x8800, 0x8800).w(FUNC(baraduke_state::irq_ack_w));
	map(0xb000, 0xb002).w(FUNC(baraduke_state::scroll_w<0>));
	map(0xb004, 0xb006).w(FUNC(baraduke_state::scroll_w<1>));
	map(0x6000, 0xffff).rom();
}

uint8_t baraduke_state::soundkludge_r()
{
	uint8_t ret = (m_counter >> 4) & 0xff;
	if (!machine().side_effects_disabled())
		m_counter++;
	return ret;
}

void baraduke_state::mcu_map(address_map &map)
{
	map(0x1000, 0x13ff).rw(m_cus30, FUNC(namco_cus30_device::namcos1_cus30_r), FUNC(namco_cus30_device::namcos1_cus30_w)); // PSG device, shared RAM
	map(0x1105, 0x1105).r(FUNC(baraduke_state::soundkludge_r)); // cures speech
	map(0x8000, 0xbfff).rom().region("mcusub", 0);  // MCU external ROM
	map(0x8000, 0x8000).nopw(); // watchdog reset?
	map(0x8800, 0x8800).nopw(); // IRQ acknowledge?
	map(0xc000, 0xc7ff).ram();
}



static INPUT_PORTS_START( baraduke )
	PORT_START("DSWA")
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SWA:1" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:2,3")
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x60, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x80, "Every 10k" )             // "B"
	PORT_DIPSETTING(    0xc0, "10k And Every 20k" )     // "A" (default)
	PORT_DIPSETTING(    0x40, "Every 20k" )             // "C"
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )         // "D"
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )         // "B"
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )       // "A" (default)
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )         // "C"
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )    // "D"
	/*  To advance rounds: set SWB:5 to ON, coin up game and push 1P start.
	    Use joystick to select round 1 - 48. Set SWB:5 to OFF to play selected round. */
	PORT_DIPNAME( 0x08, 0x08, "Round Select" )          PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Freeze" )                PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Allow Continue From Last Level" ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SWB:8" )        // Listed as "Unused"

	PORT_START("EDGE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )        // service switch from the edge connector
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("EDGE21:1") // edge connector Pin 21
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( metrocrs )
	PORT_INCLUDE( baraduke )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWA:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )         // "B"
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )       // "A" (default)
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )         // "C"
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )    // "D"
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWA:6") // metrocrs: after round 8, metrocrsa: after round 4
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	/*  To advance rounds: set SWB:2 to ON, coin up game and push 1P start.
	    Use joystick to select round 1 - 32. Set SWB:2 to OFF to play selected round. */
	PORT_DIPNAME( 0x40, 0x40, "Round Select" )          PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" )                PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SWB:4" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SWB:5" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SWB:6" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SWB:7" )        // Listed as "Unused"
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
	1024,
	3,
	{ 0x8000*8, 0, 4 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};

static GFXDECODE_START( gfx_baraduke )
	GFXDECODE_ENTRY( "chars",   0,      text_layout,            0, 512 )
	GFXDECODE_ENTRY( "tiles",   0x0000, tile_layout,            0, 256 )
	GFXDECODE_ENTRY( "tiles",   0x4000, tile_layout,            0, 256 )
	GFXDECODE_ENTRY( "sprites", 0,      gfx_16x16x4_packed_msb, 0, 128 )
GFXDECODE_END


void baraduke_state::machine_start()
{
	m_lamps.resolve();

	save_item(NAME(m_inputport_selected));
	save_item(NAME(m_counter));
}


void baraduke_state::baraduke(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, XTAL(49'152'000) / 32); // 68A09E
	m_maincpu->set_addrmap(AS_PROGRAM, &baraduke_state::main_map);

	HD63701V0(config, m_mcu, XTAL(49'152'000) / 8);
	m_mcu->set_addrmap(AS_PROGRAM, &baraduke_state::mcu_map);
	m_mcu->in_p1_cb().set(FUNC(baraduke_state::inputport_r));
	m_mcu->out_p1_cb().set(FUNC(baraduke_state::inputport_select_w));
	m_mcu->in_p2_cb().set_constant(0xff);                             // LEDs won't work otherwise
	m_mcu->out_p2_cb().set(FUNC(baraduke_state::lamps_w));

	config.set_maximum_quantum(attotime::from_hz(6000));      // we need heavy synch

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(49'152'000) / 8, 384, 0, 36 * 8, 264, 2 * 8, 30 * 8);
	screen.set_screen_update(FUNC(baraduke_state::screen_update));
	screen.screen_vblank().set(FUNC(baraduke_state::screen_vblank));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_baraduke);
	PALETTE(config, m_palette, FUNC(baraduke_state::palette), 2048);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	NAMCO_CUS30(config, m_cus30, XTAL(49'152'000) / 2048);
	m_cus30->set_voices(8);
	m_cus30->add_route(ALL_OUTPUTS, "mono", 1.0);
}



ROM_START( aliensec )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 6809 code
	ROM_LOAD( "bd1_3.9c", 0x6000, 0x02000, CRC(ea2ea790) SHA1(ab6f523803b2b0ea04b78f2f252de6c2d344a26c) )
	ROM_LOAD( "bd2_1.9a", 0x8000, 0x04000, CRC(9a0a9a87) SHA1(6d88fb5b443c822ede4884d4452e333834b16aca) )
	ROM_LOAD( "bd2_2.9b", 0xc000, 0x04000, CRC(383e5458) SHA1(091f25e287f0a81649c9a4fa196ebe4112a82295) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "cus60-60a1.mcu", 0x0000, 0x1000, CRC(076ea82a) SHA1(22b5e62e26390d7d5cacc0503c7aa5ed524204df) )  // MCU internal code

	ROM_REGION( 0x4000, "mcusub", 0 )
	ROM_LOAD( "bd1_4.3b", 0x0000, 0x4000, CRC(abda0fe7) SHA1(f7edcb5f9fa47bb38a8207af5678cf4ccc243547) )  // subprogram for the MCU

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "bd1_5.3j", 0x00000, 0x2000, CRC(706b7fee) SHA1(e5694289bd4346c1a3a004feaa940710cea755c6) )

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "bd2_8.4p", 0x00000, 0x4000, CRC(432bd7d9) SHA1(876e071b514864d434ab49002c5432f9c88665c1) )
	ROM_LOAD( "bd1_7.4n", 0x04000, 0x4000, CRC(0d7ebec9) SHA1(6b86b476db61f5760bc8610b51adc1115cfdad96) )
	ROM_LOAD( "bd2_6.4m", 0x08000, 0x4000, CRC(f4c1df60) SHA1(8a3a6682884b227fe4293adb09624a4389ee660d) )
	// 0xc000-0xffff  will be unpacked from 0x8000-0xbfff

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "bd1_9.8k",  0x00000, 0x4000, CRC(87a29acc) SHA1(3aa00efc95d1da50f6e4637d101640328287dea1) )
	ROM_LOAD( "bd1_10.8l", 0x04000, 0x4000, CRC(72b6d20c) SHA1(e40b48dacefce4fd62ab28d3e6ff3932d4ff005b) )
	ROM_LOAD( "bd1_11.8m", 0x08000, 0x4000, CRC(3076af9c) SHA1(57ce09b298fd0bae94e4d8c817a34ce812c3ddfc) )
	ROM_LOAD( "bd1_12.8n", 0x0c000, 0x4000, CRC(8b4c09a3) SHA1(46e0ef39cb313c6780f6137769153dc4a054c77f) )

	ROM_REGION( 0x1000, "proms", 0 )
	ROM_LOAD( "bd1-1.1n", 0x0000, 0x0800, CRC(0d78ebc6) SHA1(0a0c1e23eb4d1748c4e6c448284d785286c77911) )    // Blue + Green palette (PROM type mb7138e)
	ROM_LOAD( "bd1-2.2m", 0x0800, 0x0800, CRC(03f7241f) SHA1(16ae059f084ba0ac4ddaa95dbeed113295f106ea) )    // Red palette (PROM type mb7128e)
ROM_END

ROM_START( baraduke )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 6809 code
	ROM_LOAD( "bd1_3.9c", 0x6000, 0x02000, CRC(ea2ea790) SHA1(ab6f523803b2b0ea04b78f2f252de6c2d344a26c) )
	ROM_LOAD( "bd1_1.9a", 0x8000, 0x04000, CRC(4e9f2bdc) SHA1(bc6e71d4d3b2064e662a105c1a77d2731070d58e) )
	ROM_LOAD( "bd1_2.9b", 0xc000, 0x04000, CRC(40617fcd) SHA1(51d17f3a2fc96e13c8ef5952efece526e0fb33f4) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "cus60-60a1.mcu", 0x0000, 0x1000, CRC(076ea82a) SHA1(22b5e62e26390d7d5cacc0503c7aa5ed524204df) )  // MCU internal code

	ROM_REGION( 0x4000, "mcusub", 0 )
	ROM_LOAD( "bd1_4b.3b", 0x0000, 0x4000, CRC(a47ecd32) SHA1(a2a75e65deb28224a5729ed134ee4d5ea8c50706) )  // subprogram for the MCU

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "bd1_5.3j", 0x00000, 0x2000, CRC(706b7fee) SHA1(e5694289bd4346c1a3a004feaa940710cea755c6) )

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "bd1_8.4p", 0x00000, 0x4000, CRC(b0bb0710) SHA1(797832aea59bf80342fd2a3505645f2766bde65b) )
	ROM_LOAD( "bd1_7.4n", 0x04000, 0x4000, CRC(0d7ebec9) SHA1(6b86b476db61f5760bc8610b51adc1115cfdad96) )
	ROM_LOAD( "bd1_6.4m", 0x08000, 0x4000, CRC(e5da0896) SHA1(abb8bf7e9dc1c60bc0a20a691109fb145bb1d8e0) )
	// 0xc000-0xffff  will be unpacked from 0x8000-0xbfff

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "bd1_9.8k",  0x00000, 0x4000, CRC(87a29acc) SHA1(3aa00efc95d1da50f6e4637d101640328287dea1) )
	ROM_LOAD( "bd1_10.8l", 0x04000, 0x4000, CRC(72b6d20c) SHA1(e40b48dacefce4fd62ab28d3e6ff3932d4ff005b) )
	ROM_LOAD( "bd1_11.8m", 0x08000, 0x4000, CRC(3076af9c) SHA1(57ce09b298fd0bae94e4d8c817a34ce812c3ddfc) )
	ROM_LOAD( "bd1_12.8n", 0x0c000, 0x4000, CRC(8b4c09a3) SHA1(46e0ef39cb313c6780f6137769153dc4a054c77f) )

	ROM_REGION( 0x1000, "proms", 0 )
	ROM_LOAD( "bd1-1.1n", 0x0000, 0x0800, CRC(0d78ebc6) SHA1(0a0c1e23eb4d1748c4e6c448284d785286c77911) )    // Blue + Green palette (PROM type mb7138e)
	ROM_LOAD( "bd1-2.2m", 0x0800, 0x0800, CRC(03f7241f) SHA1(16ae059f084ba0ac4ddaa95dbeed113295f106ea) )    // Red palette (PROM type mb7128e)
ROM_END

ROM_START( metrocrs )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 6809 code
	ROM_LOAD( "mc1-3.9c",   0x6000, 0x02000, CRC(3390b33c) SHA1(0733aece368acc913e2ff32e8817194cb4b630fb) )
	ROM_LOAD( "mc1-1.9a",   0x8000, 0x04000, CRC(10b0977e) SHA1(6266d173b55075da1f252092bf38185880bc4969) )
	ROM_LOAD( "mc1-2.9b",   0xc000, 0x04000, CRC(5c846f35) SHA1(3c98a0f1131f2e2477fc75a588123c57ff5350ad) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "cus60-60a1.mcu", 0x0000, 0x1000, CRC(076ea82a) SHA1(22b5e62e26390d7d5cacc0503c7aa5ed524204df) )  // MCU internal code

	ROM_REGION( 0x4000, "mcusub", 0 )
	ROM_LOAD( "mc1-4.3b", 0x0000, 0x2000, CRC(9c88f898) SHA1(d6d0345002b70c5aca41c664f34181715cd87669) )  // subprogram for the MCU

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "mc1-5.3j",   0x00000, 0x2000, CRC(9b5ea33a) SHA1(a8108e71e3440b645ebdb5cdbd87712151299789) )

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "mc1-7.4p",   0x00000, 0x4000, CRC(c9dfa003) SHA1(86e8f9fc25de67691ce5385d93b723e7eb836b2b) )
	ROM_LOAD( "mc1-6.4n",   0x04000, 0x4000, CRC(9686dc3c) SHA1(1caf712eedb1f70559169685e5421e11866e518c) )
	ROM_FILL(               0x08000, 0x4000, 0xff )
	// 0xc000-0xffff  will be unpacked from 0x8000-0xbfff

	ROM_REGION( 0x08000, "sprites", 0 )
	ROM_LOAD( "mc1-8.8k",   0x00000, 0x4000, CRC(265b31fa) SHA1(d46e6db5d6f325954d2b6159157b11e10fe5838d) )
	ROM_LOAD( "mc1-9.8l",   0x04000, 0x4000, CRC(541ec029) SHA1(a3096d8405b6bbc862b03773889f6cbd43739f5b) )

	ROM_REGION( 0x1000, "proms", 0 )
	ROM_LOAD( "mc1-1.1n",   0x0000, 0x0800, CRC(32a78a8b) SHA1(545a59bc3c5868ac1749d2947210110205fb3da2) )  // Blue + Green palette
	ROM_LOAD( "mc1-2.2m",   0x0800, 0x0800, CRC(6f4dca7b) SHA1(781134c02853aded2cba63719c0e4c78b227da1c) )  // Red palette
ROM_END

ROM_START( metrocrsa )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 6809 code
	ROM_LOAD( "mc2-3.9b",   0x6000, 0x02000, CRC(ffe08075) SHA1(4e1341d5a9a58f171e1e6f9aa18092d5557a6947) )
	ROM_LOAD( "mc2-1.9a",   0x8000, 0x04000, CRC(05a239ea) SHA1(3e7c7d305d0f48e2431d60b176a0cb451ddc4637) )
	ROM_LOAD( "mc2-2.9a",   0xc000, 0x04000, CRC(db9b0e6d) SHA1(2772b59fe7dc0e78ee29dd001a6bba75b94e0334) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "cus60-60a1.mcu", 0x0000, 0x1000, CRC(076ea82a) SHA1(22b5e62e26390d7d5cacc0503c7aa5ed524204df) )  // MCU internal code

	ROM_REGION( 0x4000, "mcusub", 0 )
	ROM_LOAD( "mc1-4.3b",   0x0000, 0x2000, CRC(9c88f898) SHA1(d6d0345002b70c5aca41c664f34181715cd87669) )  // subprogram for the MCU

	ROM_REGION( 0x02000, "chars", 0 )
	ROM_LOAD( "mc1-5.3j",   0x00000, 0x2000, CRC(9b5ea33a) SHA1(a8108e71e3440b645ebdb5cdbd87712151299789) )

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD( "mc1-7.4p",   0x00000, 0x4000, CRC(c9dfa003) SHA1(86e8f9fc25de67691ce5385d93b723e7eb836b2b) )
	ROM_LOAD( "mc1-6.4n",   0x04000, 0x4000, CRC(9686dc3c) SHA1(1caf712eedb1f70559169685e5421e11866e518c) )
	ROM_FILL(               0x08000, 0x4000, 0xff )
	// 0xc000-0xffff  will be unpacked from 0x8000-0xbfff

	ROM_REGION( 0x08000, "sprites", 0 )
	ROM_LOAD( "mc1-8.8k",   0x00000, 0x4000, CRC(265b31fa) SHA1(d46e6db5d6f325954d2b6159157b11e10fe5838d) )
	ROM_LOAD( "mc1-9.8l",   0x04000, 0x4000, CRC(541ec029) SHA1(a3096d8405b6bbc862b03773889f6cbd43739f5b) )

	ROM_REGION( 0x1000, "proms", 0 )
	ROM_LOAD( "mc1-1.1n",   0x0000, 0x0800, CRC(32a78a8b) SHA1(545a59bc3c5868ac1749d2947210110205fb3da2) )  // Blue + Green palette
	ROM_LOAD( "mc1-2.2m",   0x0800, 0x0800, CRC(6f4dca7b) SHA1(781134c02853aded2cba63719c0e4c78b227da1c) )  // Red palette
ROM_END


void baraduke_state::init_baraduke()
{
	// unpack the third tile ROM
	uint8_t *rom = memregion("tiles")->base() + 0x8000;
	for (int i = 0x2000; i < 0x4000; i++)
	{
		rom[i + 0x2000] = rom[i];
		rom[i + 0x4000] = rom[i] << 4;
	}
	for (int i = 0; i < 0x2000; i++)
	{
		rom[i + 0x2000] = rom[i] << 4;
	}
}

} // anonymous namespace


GAME( 1985, metrocrs,  0,        baraduke, metrocrs, baraduke_state, init_baraduke, ROT0, "Namco", "Metro-Cross (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, metrocrsa, metrocrs, baraduke, metrocrs, baraduke_state, init_baraduke, ROT0, "Namco", "Metro-Cross (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, aliensec,  0,        baraduke, baraduke, baraduke_state, init_baraduke, ROT0, "Namco", "Alien Sector",        MACHINE_SUPPORTS_SAVE )
GAME( 1985, baraduke,  aliensec, baraduke, baraduke, baraduke_state, init_baraduke, ROT0, "Namco", "Baraduke",            MACHINE_SUPPORTS_SAVE )
