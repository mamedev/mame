// license:BSD-3-Clause
// copyright-holders: Nicola Salmoria

/***************************************************************************

Deniam games
driver by Nicola Salmoria

Check archive.org for http://deniam.co.kr

Title            System     Date
---------------- ---------- ----------
GO!GO!           deniam-16b 1995/10/11 UNDUMPED "Go Go Quiz Song Stop"
Logic Pro        deniam-16b 1996/10/20
Karian Cross     deniam-16b 1997/04/17
LOTTERY GAME     deniam-16c 1997/05/21 UNDUMPED "Bogori (Lottery Game)"
Logic Pro 2      deniam-16c 1997/06/20
Propose          deniam-16c 1997/06/21 UNDUMPED

They call the hardware "deniam-16", but it's actually pretty much identical to
Sega System 16.


Notes:

- The logicpr2 OKIM6295 ROM has four banks, but the game seems to only use 0 and 1.
  (the latter two banks are identical/nearly so to the first two?)
- logicpro dip switches might be wrong (using the logicpr2 ones)
- flip screen is not supported but these games don't use it (no flip screen dip
  and no cocktail mode)
- if it's like System 16, the top bit of palette ram should be an additional bit
  for Green. But is it ever not 0?
- Logic Pro 2 sound test menu is bugged on the real pcb. You can't change the sound
  sample and music selected due to an incorrect compare/branch at pc=0x3fd0/0x3fd2
  (for sfx) and pc=0x3ffa/0x3ffc (for bgm). Patching the two compare opcodes with
  the value 0xb041 lets you properly change the sound and bgm in the test mode,
  otherwise they're stuck at 19 and 03 respectively. Verified on real hardware.
- Logic Pro has an unemulated graphical effect: when you insert a coin, the screen
  becomes very slightly darker until the next 'scene change'. This is not emulated
  yet.
- Both Logic Pro and Logic Pro 2 have 4 pixels of garbage (related to fg layer?)
  at the left edge of the screen on the real pcb. Sprites can properly be displayed
  in this area. Mame displays this area as black.

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class deniamc_state : public driver_device
{
public:
	deniamc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_textram(*this, "textram"),
		m_spriteram(*this, "spriteram"),
		m_spritegfx(*this, "spritegfx")
	{ }

	void deniam16c(machine_config &config);

	void init_logicpro();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;

	// configuration
	u16 m_bg_scrollx_offs = 0;
	u8 m_bg_scrolly_offs = 0;
	u16 m_fg_scrollx_offs = 0;
	u8 m_fg_scrolly_offs = 0;
	u8 m_bg_scrollx_reg = 0;
	u8 m_bg_scrolly_reg = 0;
	u8 m_bg_page_reg = 0;
	u8 m_fg_scrollx_reg = 0;
	u8 m_fg_scrolly_reg = 0;
	u8 m_fg_page_reg = 0;

	void common_init();
	void base_main_map(address_map &map) ATTR_COLD;

private:
	// devices
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<u16> m_videoram;
	required_shared_ptr<u16> m_textram;
	required_shared_ptr<u16> m_spriteram;

	required_region_ptr<u8> m_spritegfx;

	// video-related
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_tx_tilemap = nullptr;
	u8 m_display_enable = 0;
	int m_bg_page[4]{};
	int m_fg_page[4]{};
	u16 m_coinctrl = 0;

	void irq_ack_w(u16 data);
	void videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void textram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 coinctrl_r();
	void coinctrl_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void oki_rom_bank_w(u8 data);
	TILEMAP_MAPPER_MEMBER(scan_pages);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_bg_page(int page, int value);
	void set_fg_page(int page, int value);
	void main_map(address_map &map) ATTR_COLD;
};

class deniamb_state : public deniamc_state
{
public:
	deniamb_state(const machine_config &mconfig, device_type type, const char *tag) :
		deniamc_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void deniam16b(machine_config &config);

	void init_karianx();

private:
	// devices
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;

	void oki_rom_bank_w(u8 data);

	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


void deniamc_state::common_init()
{
	m_bg_scrollx_reg = 0x00a4/2;
	m_bg_scrolly_reg = 0x00a8/2;
	m_bg_page_reg    = 0x00ac/2;
	m_fg_scrollx_reg = 0x00a2/2;
	m_fg_scrolly_reg = 0x00a6/2;
	m_fg_page_reg    = 0x00aa/2;

	m_display_enable = 0;
	m_coinctrl = 0;

	for (int i = 0; i < 4; i++)
	{
		m_bg_page[i] = 0;
		m_fg_page[i] = 0;
	}
}

void deniamc_state::init_logicpro()
{
	common_init();

	m_bg_scrollx_offs = 0x00d;
	m_bg_scrolly_offs = 0x000;
	m_fg_scrollx_offs = 0x009;
	m_fg_scrolly_offs = 0x000;
}

void deniamb_state::init_karianx()
{
	common_init();

	m_bg_scrollx_offs = 0x10d;
	m_bg_scrolly_offs = 0x080;
	m_fg_scrollx_offs = 0x109;
	m_fg_scrolly_offs = 0x080;
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(deniamc_state::scan_pages)
{
	// logical (col,row) -> memory offset
	return (col & 0x3f) + ((row & 0x1f) << 6) + ((col & 0x40) << 5) + ((row & 0x20) << 7);
}

TILE_GET_INFO_MEMBER(deniamc_state::get_bg_tile_info)
{
	const int page = tile_index >> 11;
	const u16 attr = m_videoram[m_bg_page[page] * 0x0800 + (tile_index & 0x7ff)];
	tileinfo.set(0,
			attr,
			(attr & 0x1fc0) >> 6,
			0);
}

TILE_GET_INFO_MEMBER(deniamc_state::get_fg_tile_info)
{
	const int page = tile_index >> 11;
	const u16 attr = m_videoram[m_fg_page[page] * 0x0800 + (tile_index & 0x7ff)];
	tileinfo.set(0,
			attr,
			(attr & 0x1fc0) >> 6,
			0);
}

TILE_GET_INFO_MEMBER(deniamc_state::get_tx_tile_info)
{
	const u16 attr = m_textram[tile_index];
	tileinfo.set(0,
			attr & 0xf1ff,
			(attr & 0x0e00) >> 9,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void deniamc_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(deniamc_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(deniamc_state::scan_pages)), 8, 8, 128, 64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(deniamc_state::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(deniamc_state::scan_pages)), 8, 8, 128, 64);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(deniamc_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void deniamc_state::videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);

	const int page = offset >> 11;
	for (int i = 0; i < 4; i++)
	{
		if (m_bg_page[i] == page)
			m_bg_tilemap->mark_tile_dirty(i * 0x800 + (offset & 0x7ff));
		if (m_fg_page[i] == page)
			m_fg_tilemap->mark_tile_dirty(i * 0x800 + (offset & 0x7ff));
	}
}


void deniamc_state::textram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_textram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}


u16 deniamc_state::coinctrl_r()
{
	return m_coinctrl;
}

void deniamc_state::coinctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_coinctrl);

	// bit 0 is coin counter
	machine().bookkeeping().coin_counter_w(0, m_coinctrl & 0x01);

	// bit 6 is display enable (0 freezes screen)
	m_display_enable = m_coinctrl & 0x20;

	// other bits unknown (unused?)
}



/***************************************************************************

  Display refresh

***************************************************************************/

/*
 * Sprite Format
 * ------------------
 *
 * Word | Bit(s)           | Use
 * -----+-fedcba9876543210-+----------------
 *   0  | --------xxxxxxxx | display y start
 *   0  | xxxxxxxx-------- | display y end
 *   2  | -------xxxxxxxxx | x position
 *   2  | ------x--------- | unknown (used in logicpr2, maybe just a bug?)
 *   2  | xxxxxx---------- | unused?
 *   4  | ---------xxxxxxx | width
 *   4  | --------x------- | is this flip y like in System 16?
 *   4  | -------x-------- | flip x
 *   4  | xxxxxxx--------- | unused?
 *   6  | xxxxxxxxxxxxxxxx | ROM address low bits
 *   8  | ----------xxxxxx | color
 *   8  | --------xx------ | priority
 *   8  | ---xxxxx-------- | ROM address high bits
 *   8  | xxx------------- | unused? (extra address bits for larger ROMs?)
 *   a  | ---------------- | zoomx like in System 16?
 *   c  | ---------------- | zoomy like in System 16?
 *   e  | ---------------- |
 */
void deniamc_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_spriteram.bytes() / 2 - 8; offs >= 0; offs -= 8)
	{
		int sx = (m_spriteram[offs + 1] & 0x01ff) + 16 * 8 - 1;
		if (sx >= 512) sx -= 512;
		const int starty = m_spriteram[offs + 0] & 0xff;
		const int endy = m_spriteram[offs + 0] >> 8;

		const int width = m_spriteram[offs + 2] & 0x007f;
		bool flipx = m_spriteram[offs + 2] & 0x0100;
		if (flipx) sx++;

		const u32 color = 0x40 + (m_spriteram[offs + 4] & 0x3f);

		int primask = 8;
		switch (m_spriteram[offs + 4] & 0xc0)
		{
			case 0x00: primask |= 4 | 2 | 1; break; // below everything
			case 0x40: primask |= 4 | 2;     break; // below fg and tx
			case 0x80: primask |= 4;         break; // below tx
			case 0xc0:                       break; // above everything
		}

		const int start = m_spriteram[offs + 3] + ((m_spriteram[offs + 4] & 0x1f00) << 8);
		const u8 *rom = &m_spritegfx[2 * start];

		for (int y = starty + 1; y <= endy; y++)
		{
			bool drawing = false;
			int i = 0;

			rom += 2 * width;   // note that the first line is skipped
			int x = 0;
			while (i < 512) // safety check
			{
				if (flipx)
				{
					if ((rom[i] & 0x0f) == 0x0f)
					{
						if (!drawing) drawing = true;
						else break;
					}
					else
					{
						if (rom[i] & 0x0f)
						{
							if (cliprect.contains(sx + x, y))
							{
								if ((screen.priority().pix(y, sx + x) & primask) == 0)
									bitmap.pix(y, sx + x) = color * 16 + (rom[i] & 0x0f);
								screen.priority().pix(y, sx + x) = 8;
							}
						}
						x++;
					}

					if ((rom[i] & 0xf0) == 0xf0)
					{
						if (!drawing) drawing = true;
						else break;
					}
					else
					{
						if (rom[i] & 0xf0)
						{
							if (cliprect.contains(sx + x, y))
							{
								if ((screen.priority().pix(y, sx + x) & primask) == 0)
									bitmap.pix(y, sx + x) = color * 16 + (rom[i] >> 4);
								screen.priority().pix(y, sx + x) = 8;
							}
						}
						x++;
					}

					i--;
				}
				else
				{
					if ((rom[i] & 0xf0) == 0xf0)
					{
						if (!drawing) drawing = true;
						else break;
					}
					else
					{
						if (rom[i] & 0xf0)
						{
							if (cliprect.contains(sx + x, y))
							{
								if ((screen.priority().pix(y, sx + x) & primask) == 0)
									bitmap.pix(y, sx + x) = color * 16 + (rom[i] >> 4);
								screen.priority().pix(y, sx + x) = 8;
							}
						}
						x++;
					}

					if ((rom[i] & 0x0f) == 0x0f)
					{
						if (!drawing) drawing = true;
						else break;
					}
					else
					{
						if (rom[i] & 0x0f)
						{
							if (cliprect.contains(sx + x, y))
							{
								if ((screen.priority().pix(y, sx + x) & primask) == 0)
									bitmap.pix(y, sx + x) = color * 16 + (rom[i] & 0x0f);
								screen.priority().pix(y, sx + x) = 8;
							}
						}
						x++;
					}

					i++;
				}
			}
		}
	}
}

void deniamc_state::set_bg_page(int page, int value)
{
	if (m_bg_page[page] != value)
	{
		m_bg_page[page] = value;
		for (int tile_index = page * 0x800; tile_index < (page + 1) * 0x800; tile_index++)
			m_bg_tilemap->mark_tile_dirty(tile_index);
	}
}

void deniamc_state::set_fg_page(int page, int value)
{
	if (m_fg_page[page] != value)
	{
		m_fg_page[page] = value;
		for (int tile_index = page * 0x800; tile_index < (page + 1) * 0x800; tile_index++)
			m_fg_tilemap->mark_tile_dirty(tile_index);
	}
}

u32 deniamc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int page;

	if (!m_display_enable)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;   /* don't update (freeze display) */
	}

	const int bg_scrollx = m_textram[m_bg_scrollx_reg] - m_bg_scrollx_offs;
	const int bg_scrolly = (m_textram[m_bg_scrolly_reg] & 0xff) - m_bg_scrolly_offs;
	page = m_textram[m_bg_page_reg];
	set_bg_page(3, (page >>12) & 0x0f);
	set_bg_page(2, (page >> 8) & 0x0f);
	set_bg_page(1, (page >> 4) & 0x0f);
	set_bg_page(0, (page >> 0) & 0x0f);

	const int fg_scrollx = m_textram[m_fg_scrollx_reg] - m_fg_scrollx_offs;
	const int fg_scrolly = (m_textram[m_fg_scrolly_reg] & 0xff) - m_fg_scrolly_offs;
	page = m_textram[m_fg_page_reg];
	set_fg_page(3, (page >>12) & 0x0f);
	set_fg_page(2, (page >> 8) & 0x0f);
	set_fg_page(1, (page >> 4) & 0x0f);
	set_fg_page(0, (page >> 0) & 0x0f);

	m_bg_tilemap->set_scrollx(0, bg_scrollx & 0x1ff);
	m_bg_tilemap->set_scrolly(0, bg_scrolly & 0x0ff);
	m_fg_tilemap->set_scrollx(0, fg_scrollx & 0x1ff);
	m_fg_tilemap->set_scrolly(0, fg_scrolly & 0x0ff);

	screen.priority().fill(0, cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 2);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 4);

	draw_sprites(screen, bitmap, cliprect);
	return 0;
}


void deniamb_state::oki_rom_bank_w(u8 data)
{
	m_oki->set_rom_bank((data >> 6) & 1);
}

void deniamc_state::oki_rom_bank_w(u8 data)
{
	if ((data & 0xfe) != 0) popmessage("OKI bank was not 0 or 1! contact MAMEDEV!");
	m_oki->set_rom_bank(data & 0x01);
}

void deniamc_state::irq_ack_w(u16 data)
{
	m_maincpu->set_input_line(4, CLEAR_LINE);
}

void deniamc_state::base_main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x400000, 0x40ffff).ram().w(FUNC(deniamb_state::videoram_w)).share(m_videoram);
	map(0x410000, 0x410fff).ram().w(FUNC(deniamb_state::textram_w)).share(m_textram);
	map(0x440000, 0x4407ff).writeonly().share(m_spriteram);
	map(0x840000, 0x840fff).w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xc40002, 0xc40003).rw(FUNC(deniamb_state::coinctrl_r), FUNC(deniamb_state::coinctrl_w));
	map(0xc40004, 0xc40005).w(FUNC(deniamb_state::irq_ack_w));
	map(0xc44000, 0xc44001).portr("SYSTEM");
	map(0xc44002, 0xc44003).portr("P1");
	map(0xc44004, 0xc44005).portr("P2").nopw();
	map(0xff0000, 0xffffff).ram();
}

void deniamb_state::main_map(address_map &map)
{
	base_main_map(map);

	map(0xc40000, 0xc40000).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xc44006, 0xc44007).nopr(); // unused?
	map(0xc4400a, 0xc4400b).portr("DSW");
}

void deniamb_state::sound_map(address_map &map)
{
	map(0x0000, 0xf7ff).rom();
	map(0xf800, 0xffff).ram();
}

void deniamb_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x02, 0x03).w("ymsnd", FUNC(ym3812_device::write));
	map(0x05, 0x05).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x07, 0x07).w(FUNC(deniamb_state::oki_rom_bank_w));
}

// identical to 16b, but handles sound directly
void deniamc_state::main_map(address_map &map)
{
	base_main_map(map);

	map(0xc40001, 0xc40001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xc40007, 0xc40007).w(FUNC(deniamc_state::oki_rom_bank_w));
	map(0xc44006, 0xc44007).nopr(); // read unused? extra input port/dipswitches?
	map(0xc40008, 0xc4000b).w("ymsnd", FUNC(ym3812_device::write)).umask16(0xff00);
	map(0xc4400a, 0xc4400b).portr("DSW"); // probably YM3812 input port
}



static INPUT_PORTS_START( karianx )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, "Demo Music" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_SERVICE( 0x80, 0x80 )
INPUT_PORTS_END

static INPUT_PORTS_START( logicpr2 )
	PORT_INCLUDE( karianx )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x18, 0x18, "Play Time" )
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END



static GFXDECODE_START( gfx_deniam )
	GFXDECODE_ENTRY( "chars", 0, gfx_8x8x3_planar, 0, 128 )    // colors 0-1023
														// sprites use colors 1024-2047
GFXDECODE_END


void deniamc_state::machine_start()
{
	save_item(NAME(m_display_enable));
	save_item(NAME(m_coinctrl));

	save_item(NAME(m_bg_page));
	save_item(NAME(m_fg_page));
}


void deniamc_state::machine_reset()
{
	/* logicpr2 does not reset the bank base on startup, though it probably
	doesn't matter since the coinup sfx (sample borrowed from 'tyrian' on PC)
	exists in both banks; it properly sets the bank as soon as the ufo sfx
	plays or a player character is selected on the character select screen */
	m_oki->set_rom_bank(0);
}

void deniamc_state::deniam16c(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(25'000'000) / 2);  // 12.5Mhz verified
	m_maincpu->set_addrmap(AS_PROGRAM, &deniamc_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(deniamb_state::irq4_line_assert));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	//screen.set_visarea(24*8, 64*8-1, 0*8, 28*8-1); // looks better but doesn't match hardware
	screen.set_visarea(24*8-4, 64*8-5, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(deniamb_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_deniam);
	PALETTE(config, m_palette).set_format(palette_device::xBGRBBBBGGGGRRRR_bit0, 2048); // bit 15 is toggle shadow / highlight?

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(25'000'000) / 6)); // "SM64" ym3812 clone; 4.166470 measured, = 4.166666Mhz verified
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.60);

	OKIM6295(config, m_oki, XTAL(25'000'000) / 24, okim6295_device::PIN7_HIGH); // 1.041620 measured, = 1.0416666Mhz verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void deniamb_state::deniam16b(machine_config &config)
{
	deniam16c(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &deniamb_state::main_map);

	Z80(config, m_audiocpu, XTAL(25'000'000) / 4);    // 6.25Mhz verified
	m_audiocpu->set_addrmap(AS_PROGRAM, &deniamb_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &deniamb_state::sound_io_map);

	// sound hardware
	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	subdevice<ym3812_device>("ymsnd")->irq_handler().set_inputline(m_audiocpu, 0);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( croquis )
	ROM_REGION( 0x100000, "maincpu", 0 ) // ROM names are the PCB locations silkscreened under chip socket
	ROM_LOAD16_BYTE( "sys_e0", 0x00000, 0x40000, CRC(b74cd4a9) SHA1(350557377bc5815d0e7f48a96898eba6374f65dd) ) // 27C020
	ROM_LOAD16_BYTE( "sys_o0", 0x00001, 0x40000, CRC(346e18f4) SHA1(0107ef7b5843a654f393d6f22ef6d52b5bf45fc0) ) // 27C020

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "logicpro.r2", 0x0000, 0x10000, CRC(000d624b) SHA1(c0da218ee81d01b3dcef2159bbaaff5d3ddb7619) ) // 27C512 - silkscreened: DSS

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "logicpro.r5", 0x000000, 0x080000, CRC(dedf18c9) SHA1(9725e096427f03ed5fd81584c0aa85a53f9681c9) ) // 27C040 - silkscreened: BG(00)
	ROM_LOAD( "logicpro.r6", 0x080000, 0x080000, CRC(3ecbd1c2) SHA1(dd6afacd58eaaa2562e007a92b6667ecc968377d) ) // 27C040 - silkscreened: BG(01)
	ROM_LOAD( "logicpro.r7", 0x100000, 0x080000, CRC(47135521) SHA1(ee6a93332190fc966f8e820430d652942f030b00) ) // 27C040 - silkscreened: BG(02)

	ROM_REGION( 0x400000, "spritegfx", 0 )   // used at run time
	ROM_LOAD16_BYTE( "logicpro.r9", 0x000000, 0x080000, CRC(a98bc1d2) SHA1(f4aed07cccca892f3d3a91546b3a98fbe3e66d9c) ) // 27C040 - silkscreened: OBJ(E0)
	ROM_LOAD16_BYTE( "logicpro.r8", 0x000001, 0x080000, CRC(1de46298) SHA1(3385a2956d9a427c85554f39c8d85922bbeb1ce1) ) // 27C040 - silkscreened: OBJ(O0)
	// OBJ(E1), OBJ(E2), OBJ(O1) & OBJ(O2) not populated

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "logicpro.r1", 0x0000, 0x080000, CRC(a1fec4d4) SHA1(4390cd18b4a7de2d8cb68270180ea3de42fd2282) ) // 27C040 - silkscreened: V_ROM0
	// V_ROM1 not populated
ROM_END

ROM_START( croquisg )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "r4.bin", 0x00000, 0x40000, CRC(03c9055e) SHA1(b1fa8e7a272887decca30eefe73ac782f296f0dd) )
	ROM_LOAD16_BYTE( "r3.bin", 0x00001, 0x40000, CRC(a98ae4f6) SHA1(80fcedb4ee0f35eb2d0b4a248c15f872af2e08f2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "logicpro.r2", 0x0000, 0x10000, CRC(000d624b) SHA1(c0da218ee81d01b3dcef2159bbaaff5d3ddb7619) )

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "logicpro.r5", 0x000000, 0x080000, CRC(dedf18c9) SHA1(9725e096427f03ed5fd81584c0aa85a53f9681c9) )
	ROM_LOAD( "logicpro.r6", 0x080000, 0x080000, CRC(3ecbd1c2) SHA1(dd6afacd58eaaa2562e007a92b6667ecc968377d) )
	ROM_LOAD( "logicpro.r7", 0x100000, 0x080000, CRC(47135521) SHA1(ee6a93332190fc966f8e820430d652942f030b00) )

	ROM_REGION( 0x400000, "spritegfx", 0 )   // used at run time
	ROM_LOAD16_BYTE( "logicpro.r9", 0x000000, 0x080000, CRC(a98bc1d2) SHA1(f4aed07cccca892f3d3a91546b3a98fbe3e66d9c) )
	ROM_LOAD16_BYTE( "logicpro.r8", 0x000001, 0x080000, CRC(1de46298) SHA1(3385a2956d9a427c85554f39c8d85922bbeb1ce1) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "logicpro.r1", 0x0000, 0x080000, CRC(a1fec4d4) SHA1(4390cd18b4a7de2d8cb68270180ea3de42fd2282) )
ROM_END

ROM_START( logicpro )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "logicpro.r4", 0x00000, 0x40000, CRC(c506d484) SHA1(5d662b109e1d2e09556bc4ecbc11bbf5ccb639d3) )
	ROM_LOAD16_BYTE( "logicpro.r3", 0x00001, 0x40000, CRC(d5a4cf62) SHA1(138ea4f1629e453c1a00410eda7086d3633240e3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "logicpro.r2", 0x0000, 0x10000, CRC(000d624b) SHA1(c0da218ee81d01b3dcef2159bbaaff5d3ddb7619) )

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "logicpro.r5", 0x000000, 0x080000, CRC(dedf18c9) SHA1(9725e096427f03ed5fd81584c0aa85a53f9681c9) )
	ROM_LOAD( "logicpro.r6", 0x080000, 0x080000, CRC(3ecbd1c2) SHA1(dd6afacd58eaaa2562e007a92b6667ecc968377d) )
	ROM_LOAD( "logicpro.r7", 0x100000, 0x080000, CRC(47135521) SHA1(ee6a93332190fc966f8e820430d652942f030b00) )

	ROM_REGION( 0x400000, "spritegfx", 0 )   // used at run time
	ROM_LOAD16_BYTE( "logicpro.r9", 0x000000, 0x080000, CRC(a98bc1d2) SHA1(f4aed07cccca892f3d3a91546b3a98fbe3e66d9c) )
	ROM_LOAD16_BYTE( "logicpro.r8", 0x000001, 0x080000, CRC(1de46298) SHA1(3385a2956d9a427c85554f39c8d85922bbeb1ce1) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "logicpro.r1", 0x0000, 0x080000, CRC(a1fec4d4) SHA1(4390cd18b4a7de2d8cb68270180ea3de42fd2282) )
ROM_END

ROM_START( karianx )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "even",        0x00000, 0x80000, CRC(fd0ce238) SHA1(4b727366c942c62187d8700666b42a85c059c060) )
	ROM_LOAD16_BYTE( "odd",         0x00001, 0x80000, CRC(be173cdc) SHA1(13230b6129fd1910257624a69a3a4b74696e982e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "snd",         0x0000, 0x10000, CRC(fedd3375) SHA1(09fb2d5fc91704120f757acf9fa00d149f891a28) )

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "bkg1",        0x000000, 0x080000, CRC(5cb8558a) SHA1(9c6024c70a0f0cd529a0e2e853e467ec8d8ab446) )
	ROM_LOAD( "bkg2",        0x080000, 0x080000, CRC(95ff297c) SHA1(28f6c005e73e1680bd8be7ce355fa0d404827105) )
	ROM_LOAD( "bkg3",        0x100000, 0x080000, CRC(6c81f1b2) SHA1(14ef907a9c381b7ef45441d480bb4ccb015e474b) )

	ROM_REGION( 0x400000, "spritegfx", 0 )   // used at run time
	ROM_LOAD16_BYTE( "obj4",        0x000000, 0x080000, CRC(5f8d75a9) SHA1(0552d046742aeb2fee176887156e73480c75a1bd) )
	ROM_LOAD16_BYTE( "obj1",        0x000001, 0x080000, CRC(967ee97d) SHA1(689f2da67eab86653b846fada39139792cd4aee2) )
	ROM_LOAD16_BYTE( "obj5",        0x100000, 0x080000, CRC(e9fc22f9) SHA1(a1f7f779520346406949500e3224c0c42cbbe026) )
	ROM_LOAD16_BYTE( "obj2",        0x100001, 0x080000, CRC(d39eb04e) SHA1(c59c3e14a506cb04d09cc7eec9962daa242a0b0c) )
	ROM_LOAD16_BYTE( "obj6",        0x200000, 0x080000, CRC(c1ec35a5) SHA1(bf59f4c3de081c8cc398c825fc1f3e8577641f10) )
	ROM_LOAD16_BYTE( "obj3",        0x200001, 0x080000, CRC(6ac1ac87) SHA1(1954e25ac5489a8eca137b86c89c415f1fed360c) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "voi",         0x0000, 0x080000, CRC(c6506a80) SHA1(121229c501bd5678e55c7342619743c773a01a7e) )
ROM_END

ROM_START( logicpr2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lp2-2",       0x00000, 0x80000, CRC(cc1880bf) SHA1(5ea542b63947a570aaf924f7ab739e060e359af8) )
	ROM_LOAD16_BYTE( "lp2-1",       0x00001, 0x80000, CRC(46d5e954) SHA1(7bf5ae19caeecd2123754698276bbc78d68984d9) )

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "log2-b01",    0x000000, 0x080000, CRC(fe789e07) SHA1(c3d542564519fd807bc605029f5a2cca571eec9f) )
	ROM_LOAD( "log2-b02",    0x080000, 0x080000, CRC(1e0c51cd) SHA1(c25b3259a173e77785dcee1407ddf191c3efad79) )
	ROM_LOAD( "log2-b03",    0x100000, 0x080000, CRC(916f2928) SHA1(8c73408664dcd3de42cb27fac0d22b87b540bf52) )

	ROM_REGION( 0x400000, "spritegfx", 0 )   // used at run time
	ROM_LOAD16_WORD_SWAP( "obj",         0x000000, 0x400000, CRC(f221f305) SHA1(aa1d3d86d13e009bfb44cbc6ff4401b811b19f97) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "log2-s01",    0x0000, 0x100000, CRC(2875c435) SHA1(633538d9ac53228ea344605482ac387852c29193) )
ROM_END

} // anonymous namespace


GAME( 1996, croquis,  0,        deniam16b, logicpr2, deniamb_state, init_logicpro, ROT0, "Deniam", "Croquis (Korea)",         MACHINE_SUPPORTS_SAVE )
GAME( 1996, croquisg, croquis,  deniam16b, logicpr2, deniamb_state, init_logicpro, ROT0, "Deniam", "Croquis (Germany)",       MACHINE_SUPPORTS_SAVE )
GAME( 1996, logicpro, croquis,  deniam16b, logicpr2, deniamb_state, init_logicpro, ROT0, "Deniam", "Logic Pro (Japan)",       MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1996, karianx,  0,        deniam16b, karianx,  deniamb_state, init_karianx,  ROT0, "Deniam", "Karian Cross (Rev. 1.0)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, logicpr2, 0,        deniam16c, logicpr2, deniamc_state, init_logicpro, ROT0, "Deniam", "Logic Pro 2 (Japan)",     MACHINE_SUPPORTS_SAVE )
