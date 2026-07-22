// license:BSD-3-Clause
// copyright-holders:BUT
/*
 *  Thunder Ceptor board
 *  (C) 1986 Namco
 *
 *  Hardware analyzed by nono
 *  Driver by BUT
 */


#include "emu.h"

#include "namco_c45road.h"

#include "cpu/m6502/r65c02.h"
#include "cpu/m6800/m6801.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6809/m6809.h"
#include "machine/adc0808.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "sound/namco.h"
#include "sound/ymopm.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "tceptor2.lh"

#include <algorithm>


namespace {

class tceptor_state : public driver_device
{
public:
	tceptor_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu%u", 1U),
		m_subcpu(*this, "sub"),
		m_mcu(*this, "mcu"),
		m_cus30(*this, "namco"),
		m_sprite_ram(*this, "sprite_ram"),
		m_c45_road(*this, "c45_road"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_tile_ram(*this, "tile_ram"),
		m_tile_attr(*this, "tile_attr"),
		m_bg_ram(*this, "bg_ram"),
		m_m68k_shared_ram(*this, "m68k_shared_ram"),
		m_inp(*this, "IN%u", 0U),
		m_dsw(*this, "DSW%u", 1U),
		m_shutter(*this, "shutter")
	{ }

	void tceptor(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<cpu_device, 2> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_mcu;
	required_device<namco_cus30_device> m_cus30;
	required_device<buffered_spriteram16_device> m_sprite_ram;
	required_device<namco_c45_road_device> m_c45_road;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_tile_ram;
	required_shared_ptr<uint8_t> m_tile_attr;
	required_shared_ptr<uint8_t> m_bg_ram;
	required_shared_ptr<uint8_t> m_m68k_shared_ram;

	required_ioport_array<2> m_inp;
	required_ioport_array<2> m_dsw;
	output_finder<> m_shutter;

	uint8_t m_m6809_irq_enable = 0;
	uint8_t m_m68k_irq_enable = 0;
	uint8_t m_mcu_irq_enable = 0;
	int m_sprite16 = 0;
	int m_sprite32 = 0;
	int m_bg = 0;
	tilemap_t *m_tx_tilemap = nullptr;
	tilemap_t *m_bg_tilemap[2]{};
	int32_t m_bg_scroll_x[2]{};
	int32_t m_bg_scroll_y[2]{};
	bitmap_ind16 m_temp_bitmap;
	std::unique_ptr<uint8_t[]> m_decoded_16;
	std::unique_ptr<uint8_t[]> m_decoded_32;
	bool m_is_mask_spr[1024/16]{};

	uint8_t m68k_shared_r(offs_t offset);
	void m68k_shared_w(offs_t offset, uint8_t data);
	void m6809_irq_enable_w(uint8_t data);
	void m6809_irq_disable_w(uint8_t data);
	void m68k_irq_enable_w(uint16_t data);
	void mcu_irq_enable_w(uint8_t data);
	void mcu_irq_disable_w(uint8_t data);
	uint8_t dsw0_r();
	uint8_t dsw1_r();
	uint8_t input0_r();
	uint8_t input1_r();
	void tile_ram_w(offs_t offset, uint8_t data);
	void tile_attr_w(offs_t offset, uint8_t data);
	void bg_ram_w(offs_t offset, uint8_t data);
	void bg_scroll_w(offs_t offset, uint8_t data);
	void tceptor2_shutter_w(uint8_t data);
	void tile_mark_dirty(int offset);

	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	void palette_init(palette_device &palette) ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	INTERRUPT_GEN_MEMBER(m6809_vb_interrupt);
	INTERRUPT_GEN_MEMBER(m68k_vb_interrupt);
	INTERRUPT_GEN_MEMBER(mcu_vb_interrupt);
	inline int get_tile_addr(int tile_index);
	void decode_bg(const char *region) ATTR_COLD;
	void decode_sprite(int gfx_index, const gfx_layout *layout, const void *data) ATTR_COLD;
	void decode_sprite16(const char *region) ATTR_COLD;
	void decode_sprite32(const char *region) ATTR_COLD;
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_priority);
	inline uint8_t fix_input0(uint8_t in1, uint8_t in2);
	inline uint8_t fix_input1(uint8_t in1, uint8_t in2);

	void m6502_a_map(address_map &map) ATTR_COLD;
	void m6502_b_map(address_map &map) ATTR_COLD;
	void m6809_map(address_map &map) ATTR_COLD;
	void m68k_map(address_map &map) ATTR_COLD;
	void mcu_io_map(address_map &map) ATTR_COLD;
	void mcu_map(address_map &map) ATTR_COLD;
};


static constexpr uint8_t TX_TILE_OFFSET_CENTER = 32 * 2;
static constexpr uint8_t TX_TILE_OFFSET_RIGHT = 32 * 0 + 2;
static constexpr uint16_t TX_TILE_OFFSET_LEFT = 32 * 31 + 2;

static constexpr uint16_t SPR_TRANS_COLOR = 0xff + 0x300;
static constexpr uint16_t SPR_MASK_COLOR = 0xfe + 0x300;


/*******************************************************************/

void tceptor_state::palette_init(palette_device &palette)
{
	const uint8_t *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x400; i++)
	{
		int const r = pal4bit(color_prom[i + 0x000]);
		int const g = pal4bit(color_prom[i + 0x400]);
		int const b = pal4bit(color_prom[i + 0x800]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0xc00;

	/*
	    color lookup table:
	     0-    +1024 ( 4 * 256) colors: text   (use 0-   256 colors)
	     1024- +1024 (16 *  64) colors: sprite (use 768- 256 colors)
	     2048-  +512 ( 8 *  64) colors: bg     (use 0-   512 colors)
	     3840-  +256 ( 4 *  64) colors: road   (use 512- 256 colors)
	*/

	// tiles lookup table (1024 colors)
	for (int i = 0; i < 0x0400; i++)
	{
		int const ctabentry = color_prom[i];
		palette.set_pen_indirect(i, ctabentry);
	}

	// sprites lookup table (1024 colors)
	for (int i = 0x0400; i < 0x0800; i++)
	{
		int const ctabentry = color_prom[i] | 0x300;
		palette.set_pen_indirect(i, ctabentry);
	}

	// background: no lookup PROM, use directly (512 colors)
	for (int i = 0x0a00; i < 0x0c00; i++)
	{
		int const ctabentry = i & 0x1ff;
		palette.set_pen_indirect(i, ctabentry);
	}

	// road lookup table (256 colors)
	for (int i = 0x0f00; i < 0x1000; i++)
	{
		int const ctabentry = color_prom[i - 0x700] | 0x200;
		palette.set_pen_indirect(i, ctabentry);
	}

	// setup sprite mask color map
	// tceptor2: only 0x23
	std::fill(std::begin(m_is_mask_spr), std::end(m_is_mask_spr), false);
	for (int i = 0; i < 0x400; i++)
	{
		if (palette.pen_indirect(i | 0x400) == SPR_MASK_COLOR)
			m_is_mask_spr[i >> 4] = true;
	}
}


/*******************************************************************/

inline int tceptor_state::get_tile_addr(int tile_index)
{
	int const x = tile_index / 28;
	int const y = tile_index % 28;

	switch (x)
	{
	case 0:
		return TX_TILE_OFFSET_LEFT + y;
	case 33:
		return TX_TILE_OFFSET_RIGHT + y;
	}

	return TX_TILE_OFFSET_CENTER + (x - 1) + y * 32;
}

TILE_GET_INFO_MEMBER(tceptor_state::get_tx_tile_info)
{
	int const offset = get_tile_addr(tile_index);
	int const code = m_tile_ram[offset];
	int const color = m_tile_attr[offset];

	tileinfo.group = color;

	tileinfo.set(0, code, color, 0);
}

void tceptor_state::tile_mark_dirty(int offset)
{
	int x = -1;
	int y = -1;

	if (offset >= TX_TILE_OFFSET_LEFT && offset < TX_TILE_OFFSET_LEFT + 28)
	{
		x = 0;
		y = offset - TX_TILE_OFFSET_LEFT;
	}
	else if (offset >= TX_TILE_OFFSET_RIGHT && offset < TX_TILE_OFFSET_RIGHT + 28)
	{
		x = 33;
		y = offset - TX_TILE_OFFSET_RIGHT;
	}
	else if (offset >= TX_TILE_OFFSET_CENTER && offset < TX_TILE_OFFSET_CENTER + 32 * 28)
	{
		offset -= TX_TILE_OFFSET_CENTER;
		x = (offset % 32) + 1;
		y = offset / 32;
	}

	if (x >= 0)
		m_tx_tilemap->mark_tile_dirty(x * 28 + y);
}


void tceptor_state::tile_ram_w(offs_t offset, uint8_t data)
{
	if (m_tile_ram[offset] != data)
	{
		m_tile_ram[offset] = data;
		tile_mark_dirty(offset);
	}
}

void tceptor_state::tile_attr_w(offs_t offset, uint8_t data)
{
	if (m_tile_attr[offset] != data)
	{
		m_tile_attr[offset] = data;
		tile_mark_dirty(offset);
	}
}


/*******************************************************************/

TILE_GET_INFO_MEMBER(tceptor_state::get_bg1_tile_info)
{
	uint16_t const data = m_bg_ram[tile_index * 2] | (m_bg_ram[tile_index * 2 + 1] << 8);
	int const code = (data & 0x3ff) | 0x000;
	int const color = (data & 0xfc00) >> 10;

	tileinfo.set(m_bg, code, color, 0);
}

TILE_GET_INFO_MEMBER(tceptor_state::get_bg2_tile_info)
{
	uint16_t const data = m_bg_ram[tile_index * 2 + 0x1000] | (m_bg_ram[tile_index * 2 + 1 + 0x1000] << 8);
	int const code = (data & 0x3ff) | 0x400;
	int const color = (data & 0xfc00) >> 10;

	tileinfo.set(m_bg, code, color, 0);
}

void tceptor_state::bg_ram_w(offs_t offset, uint8_t data)
{
	m_bg_ram[offset] = data;

	m_bg_tilemap[offset >> 12]->mark_tile_dirty((offset & 0xfff) >> 1);
}

void tceptor_state::bg_scroll_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		m_bg_scroll_x[0] &= 0xff;
		m_bg_scroll_x[0] |= data << 8;
		break;
	case 1:
		m_bg_scroll_x[0] &= 0xff00;
		m_bg_scroll_x[0] |= data;
		break;
	case 2:
		m_bg_scroll_y[0] = data;
		break;

	case 4:
		m_bg_scroll_x[1] &= 0xff;
		m_bg_scroll_x[1] |= data << 8;
		break;
	case 5:
		m_bg_scroll_x[1] &= 0xff00;
		m_bg_scroll_x[1] |= data;
		break;
	case 6:
		m_bg_scroll_y[1] = data;
		break;
	}
}


/*******************************************************************/

void tceptor_state::decode_bg(const char *region)
{
	static const gfx_layout bg_layout =
	{
		8, 8,
		2048,
		3,
		{ 0x40000+4, 0, 4 },
		{ 0, 1, 2, 3, 8, 9, 10, 11 },
		{ 0, 16, 32, 48, 64, 80, 96, 112 },
		128
	};

	int const gfx_index = m_bg;
	uint8_t *src = memregion(region)->base() + 0x8000;
	int const len = 0x8000;

	std::vector<uint8_t> buffer(len);

	// expand ROM tc2-19.10d
	for (int i = 0; i < len / 2; i++)
	{
		buffer[i * 2 + 1] = src[i] & 0x0f;
		buffer[i * 2] = (src[i] & 0xf0) >> 4;
	}

	memcpy(src, &buffer[0], len);

	// decode the graphics
	m_gfxdecode->set_gfx(gfx_index, std::make_unique<gfx_element>(m_palette, bg_layout, memregion(region)->base(), 0, 64, 0x0a00));
}

void tceptor_state::decode_sprite(int gfx_index, const gfx_layout *layout, const void *data)
{
	// decode the graphics
	m_gfxdecode->set_gfx(gfx_index, std::make_unique<gfx_element>(m_palette, *layout, (const uint8_t *)data, 0, 64, 1024));
}

// fix sprite order
void tceptor_state::decode_sprite16(const char *region)
{
	static const gfx_layout spr16_layout =
	{
		16, 16,
		512,
		4,
		{ 0x00000, 0x00004, 0x40000, 0x40004 },
		{
			0*8, 0*8+1, 0*8+2, 0*8+3, 1*8, 1*8+1, 1*8+2, 1*8+3,
			2*8, 2*8+1, 2*8+2, 2*8+3, 3*8, 3*8+1, 3*8+2, 3*8+3
		},
		{
			0*2*16,  1*2*16,  2*2*16,  3*2*16,  4*2*16,  5*2*16,  6*2*16,  7*2*16,
			8*2*16,  9*2*16, 10*2*16, 11*2*16, 12*2*16, 13*2*16, 14*2*16, 15*2*16
		},
		2*16*16
	};

	uint8_t *src = memregion(region)->base();
	int const len = memregion(region)->bytes();

	m_decoded_16 = std::make_unique<uint8_t[]>(len);

	for (int i = 0; i < len / (4 * 4 * 16); i++)
		for (int y = 0; y < 16; y++)
		{
			memcpy(&m_decoded_16[(i * 4 + 0) * (2 * 16 * 16 / 8) + y * (2 * 16 / 8)],
					&src[i * (2 * 32 * 32 / 8) + y * (2 * 32 / 8)],
					4);
			memcpy(&m_decoded_16[(i * 4 + 1) * (2 * 16 * 16 / 8) + y * (2 * 16 / 8)],
					&src[i * (2 * 32 * 32 / 8) + y * (2 * 32 / 8) + (4 * 8 / 8)],
					4);
			memcpy(&m_decoded_16[(i * 4 + 2) * (2 * 16 * 16 / 8) + y * (2 * 16 / 8)],
					&src[i * (2 * 32 * 32 / 8) + y * (2 * 32 / 8) + (16 * 2 * 32 / 8)],
					4);
			memcpy(&m_decoded_16[(i * 4 + 3) * (2 * 16 * 16 / 8) + y * (2 * 16 / 8)],
					&src[i * (2 * 32 * 32 / 8) + y * (2 * 32 / 8) + (4 * 8 / 8) + (16 * 2 * 32 / 8)],
					4);
		}

	decode_sprite(m_sprite16, &spr16_layout, m_decoded_16.get());
}

// fix sprite order
void tceptor_state::decode_sprite32(const char *region)
{
	static const gfx_layout spr32_layout =
	{
		32, 32,
		1024,
		4,
		{ 0x000000, 0x000004, 0x200000, 0x200004 },
		{
			0*8, 0*8+1, 0*8+2, 0*8+3, 1*8, 1*8+1, 1*8+2, 1*8+3,
			2*8, 2*8+1, 2*8+2, 2*8+3, 3*8, 3*8+1, 3*8+2, 3*8+3,
			4*8, 4*8+1, 4*8+2, 4*8+3, 5*8, 5*8+1, 5*8+2, 5*8+3,
			6*8, 6*8+1, 6*8+2, 6*8+3, 7*8, 7*8+1, 7*8+2, 7*8+3
		},
		{
			0*2*32,  1*2*32,  2*2*32,  3*2*32,  4*2*32,  5*2*32,  6*2*32,  7*2*32,
			8*2*32,  9*2*32,  10*2*32, 11*2*32, 12*2*32, 13*2*32, 14*2*32, 15*2*32,
			16*2*32, 17*2*32, 18*2*32, 19*2*32, 20*2*32, 21*2*32, 22*2*32, 23*2*32,
			24*2*32, 25*2*32, 26*2*32, 27*2*32, 28*2*32, 29*2*32, 30*2*32, 31*2*32
		},
		2*32*32
	};

	uint8_t *src = memregion(region)->base();
	int const len = memregion(region)->bytes();
	int const total = spr32_layout.total;
	int const size = spr32_layout.charincrement / 8;

	m_decoded_32 = make_unique_clear<uint8_t[]>(len);

	for (int i = 0; i < total; i++)
	{
		int code;

		code = (i & 0x07f) | ((i & 0x180) << 1) | 0x80;
		code &= ~((i & 0x200) >> 2);

		memcpy(&m_decoded_32[size * (i + 0)],     &src[size * (code + 0)],     size);
		memcpy(&m_decoded_32[size * (i + total)], &src[size * (code + total)], size);
	}

	decode_sprite(m_sprite32, &spr32_layout, m_decoded_32.get());
}

void tceptor_state::video_start()
{
	int gfx_index;

	// find first empty slot to decode gfx
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (m_gfxdecode->gfx(gfx_index) == nullptr)
			break;
	assert(gfx_index + 4 <= MAX_GFX_ELEMENTS);

	m_bg = gfx_index++;
	decode_bg("bgtiles");

	m_sprite16 = gfx_index++;
	decode_sprite16("spr16tiles");

	m_sprite32 = gfx_index++;
	decode_sprite32("spr32tiles");

	// allocate temp bitmaps
	m_screen->register_screen_bitmap(m_temp_bitmap);

	m_c45_road->set_transparent_color(m_palette->pen_indirect(0xfff));

	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tceptor_state::get_tx_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 34, 28);

	m_tx_tilemap->set_scrollx(0, -2*8);
	m_tx_tilemap->set_scrolly(0, 0);
	m_tx_tilemap->configure_groups(*m_gfxdecode->gfx(0), 7);

	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tceptor_state::get_bg1_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tceptor_state::get_bg2_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	save_item(NAME(m_bg_scroll_x));
	save_item(NAME(m_bg_scroll_y));
}


/*******************************************************************/

/*
    Sprite data format

    000: zzzzzzBB BTTTTTTT
    002: ZZZZZZPP PPCCCCCC
    100: fFL---YY YYYYYYYY
    102: ------XX XXXXXXXX

    B: bank
    T: number
    P: priority
    C: color
    X: x
    Y: y
    L: large sprite
    F: flip x
    f: flip y
    Z: zoom x
    z: zoom y
*/

void tceptor_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_priority)
{
	const uint16_t *const mem1 = &m_sprite_ram->buffer()[0x000/2];
	const uint16_t *const mem2 = &m_sprite_ram->buffer()[0x100/2];
	bool need_mask = false;

	for (int i = 0; i < 0x100; i += 2)
	{
		int scalex = (mem1[1 + i] & 0xfc00) << 1;
		int scaley = (mem1[0 + i] & 0xfc00) << 1;
		int const pri = 7 - ((mem1[1 + i] & 0x3c0) >> 6);

		if (pri == sprite_priority && scalex && scaley)
		{
			int x = mem2[1 + i] & 0x3ff;
			int y = 512 - (mem2[0 + i] & 0x3ff);
			int const flipx = BIT(mem2[0 + i], 14);
			int const flipy = BIT(mem2[0 + i], 15);
			int const color = mem1[1 + i] & 0x3f;
			int gfx;
			int code;

			if (mem2[0 + i] & 0x2000)
			{
				gfx = m_sprite32;
				code = mem1[0 + i] & 0x3ff;

			}
			else
			{
				gfx = m_sprite16;
				code = mem1[0 + i] & 0x1ff;
				scaley *= 2;
			}

			if (m_is_mask_spr[color])
			{
				if (!need_mask)
					// backup previous bitmap
					copybitmap(m_temp_bitmap, bitmap, 0, 0, 0, 0, cliprect);

				need_mask = true;
			}

			// round off
			scalex += 0x800;
			scaley += 0x800;

			x -= 64;
			y -= 78;

			m_gfxdecode->gfx(gfx)->zoom_transmask(bitmap,
					cliprect,
					code,
					color,
					flipx, flipy,
					x, y,
					scalex,
					scaley,
					m_palette->transpen_mask(*m_gfxdecode->gfx(gfx), color, SPR_TRANS_COLOR));
		}
	}

	// if SPR_MASK_COLOR pen is used, restore pixels from previous bitmap
	if (need_mask)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
				if (m_palette->pen_indirect(bitmap.pix(y, x)) == SPR_MASK_COLOR)
					// restore pixel
					bitmap.pix(y, x) = m_temp_bitmap.pix(y, x);
	}
}


uint32_t tceptor_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const bg_center = 144 - ((((m_bg_scroll_x[0] + m_bg_scroll_x[1] ) & 0x1ff) - 288) / 2);

	// left background
	rectangle rect = cliprect;
	rect.max_x = bg_center;
	m_bg_tilemap[0]->set_scrollx(0, m_bg_scroll_x[0] + 12);
	m_bg_tilemap[0]->set_scrolly(0, m_bg_scroll_y[0] + 20); //32?
	m_bg_tilemap[0]->draw(screen, bitmap, rect, 0, 0);

	// right background
	rect.min_x = bg_center;
	rect.max_x = cliprect.max_x;
	m_bg_tilemap[1]->set_scrollx(0, m_bg_scroll_x[1] + 20);
	m_bg_tilemap[1]->set_scrolly(0, m_bg_scroll_y[1] + 20); // 32?
	m_bg_tilemap[1]->draw(screen, bitmap, rect, 0, 0);

	for (int pri = 0; pri < 8; pri++)
	{
		m_c45_road->draw(screen, bitmap, cliprect, pri * 2);
		m_c45_road->draw(screen, bitmap, cliprect, pri * 2 + 1);
		draw_sprites(bitmap, cliprect, pri);
	}

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void tceptor_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		m_sprite_ram->copy();

		if (m_m6809_irq_enable)
			m_maincpu->set_input_line(0, HOLD_LINE);
		else
			m_m6809_irq_enable = 1;

		if (m_m68k_irq_enable)
			m_subcpu->set_input_line(1, HOLD_LINE);

		if (m_mcu_irq_enable)
			m_mcu->set_input_line(0, HOLD_LINE);
		else
			m_mcu_irq_enable = 1;
	}
}

void tceptor_state::tceptor2_shutter_w(uint8_t data)
{
	// 3D scope shutter control
	m_shutter = BIT(data, 0);
}


/*******************************************************************/

uint8_t tceptor_state::m68k_shared_r(offs_t offset)
{
	return m_m68k_shared_ram[offset];
}

void tceptor_state::m68k_shared_w(offs_t offset, uint8_t data)
{
	m_m68k_shared_ram[offset] = data;
}


/*******************************************************************/

void tceptor_state::m6809_irq_enable_w(uint8_t data)
{
	m_m6809_irq_enable = 1;
}

void tceptor_state::m6809_irq_disable_w(uint8_t data)
{
	m_m6809_irq_enable = 0;
}


void tceptor_state::m68k_irq_enable_w(uint16_t data)
{
	m_m68k_irq_enable = data;
}


void tceptor_state::mcu_irq_enable_w(uint8_t data)
{
	m_mcu_irq_enable = 1;
}

void tceptor_state::mcu_irq_disable_w(uint8_t data)
{
	m_mcu_irq_enable = 0;
}


// fix dsw/input data to memory mapped data
uint8_t tceptor_state::fix_input0(uint8_t in1, uint8_t in2)
{
	return bitswap<4>(in1, 1, 3, 5, 7) | bitswap<4>(in2, 1, 3, 5, 7) << 4;
}

uint8_t tceptor_state::fix_input1(uint8_t in1, uint8_t in2)
{
	return bitswap<4>(in1, 0, 2, 4, 6) | bitswap<4>(in2, 0, 2, 4, 6) << 4;
}

uint8_t tceptor_state::dsw0_r()
{
	return fix_input0(m_dsw[0]->read(), m_dsw[1]->read());
}

uint8_t tceptor_state::dsw1_r()
{
	return fix_input1(m_dsw[0]->read(), m_dsw[1]->read());
}

uint8_t tceptor_state::input0_r()
{
	return fix_input0(m_inp[0]->read(), m_inp[1]->read());
}

uint8_t tceptor_state::input1_r()
{
	return fix_input1(m_inp[0]->read(), m_inp[1]->read());
}

/*******************************************************************/

void tceptor_state::m6809_map(address_map &map)
{
	map(0x0000, 0x17ff).ram();
	map(0x1800, 0x1bff).ram().w(FUNC(tceptor_state::tile_ram_w)).share(m_tile_ram);
	map(0x1c00, 0x1fff).ram().w(FUNC(tceptor_state::tile_attr_w)).share(m_tile_attr);
	map(0x2000, 0x3fff).ram().w(FUNC(tceptor_state::bg_ram_w)).share(m_bg_ram); // background (VIEW RAM)
	map(0x4000, 0x43ff).m(m_cus30, FUNC(namco_cus30_device::amap));
	map(0x4800, 0x4800).w(FUNC(tceptor_state::tceptor2_shutter_w));
	map(0x4f00, 0x4f07).rw("adc", FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w));
	map(0x5000, 0x5006).w(FUNC(tceptor_state::bg_scroll_w));
	map(0x6000, 0x7fff).ram().share(m_m68k_shared_ram); // COM RAM
	map(0x8000, 0x8000).w(FUNC(tceptor_state::m6809_irq_disable_w));
	map(0x8800, 0x8800).w(FUNC(tceptor_state::m6809_irq_enable_w));
	map(0x8000, 0xffff).rom();
}


void tceptor_state::m6502_a_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().share("soundshared");
	map(0x0100, 0x01ff).ram();
	map(0x0200, 0x02ff).ram();
	map(0x0300, 0x030f).ram();
	map(0x2000, 0x2001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x3000, 0x30ff).ram().share("mcushared");
	map(0x3c01, 0x3c01).nopw();
	map(0x8000, 0xffff).rom();
}


void tceptor_state::m6502_b_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().share("soundshared");
	map(0x0100, 0x01ff).ram();
	map(0x4000, 0x4000).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x5000, 0x5000).nopw(); // voice ctrl??
	map(0x8000, 0xffff).rom();
}


void tceptor_state::m68k_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom(); // M68K ERROR 1
	map(0x100000, 0x10ffff).rom(); // not sure
	map(0x200000, 0x203fff).ram(); // M68K ERROR 0
	map(0x300000, 0x300001).nopw();
	map(0x400000, 0x4001ff).writeonly().share("sprite_ram");
	map(0x500000, 0x51ffff).m(m_c45_road, FUNC(namco_c45_road_device::writeonly_map));
	map(0x600000, 0x600001).w(FUNC(tceptor_state::m68k_irq_enable_w)); // not sure
	map(0x700000, 0x703fff).rw(FUNC(tceptor_state::m68k_shared_r), FUNC(tceptor_state::m68k_shared_w)).umask16(0x00ff);
}


void tceptor_state::mcu_map(address_map &map)
{
	map(0x1000, 0x13ff).m(m_cus30, FUNC(namco_cus30_device::amap));
	map(0x1400, 0x154d).ram();
	map(0x17c0, 0x17ff).ram();
	map(0x2000, 0x20ff).ram().share("mcushared");
	map(0x2100, 0x2100).r(FUNC(tceptor_state::dsw0_r));
	map(0x2101, 0x2101).r(FUNC(tceptor_state::dsw1_r));
	map(0x2200, 0x2200).r(FUNC(tceptor_state::input0_r));
	map(0x2201, 0x2201).r(FUNC(tceptor_state::input1_r));
	map(0x8000, 0x8000).w(FUNC(tceptor_state::mcu_irq_disable_w));
	map(0x8800, 0x8800).w(FUNC(tceptor_state::mcu_irq_enable_w));
	map(0x8000, 0xbfff).rom().region("mcusub", 0);
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xdfff).ram().share("nvram"); // Battery Backup
}



/*******************************************************************/

static INPUT_PORTS_START( tceptor )
	PORT_START("IN0") // Memory Mapped Port
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // shot
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // bomb
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) // shot
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) // bomb
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1") // Memory Mapped Port
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW ) // TEST SW
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1") // DSW 1
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Freeze" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2") // DSW 2
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, "A" )
	PORT_DIPSETTING(    0x03, "B" )
	PORT_DIPSETTING(    0x01, "C" )
	PORT_DIPSETTING(    0x00, "D" )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PEDAL") // ADC0809 - 8 CHANNEL ANALOG - CHANNEL 1
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xd6) PORT_SENSITIVITY(100) PORT_KEYDELTA(16) PORT_CODE_INC(KEYCODE_Z)

	PORT_START("STICKX") // ADC0809 - 8 CHANNEL ANALOG - CHANNEL 2
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xfe) PORT_SENSITIVITY(100) PORT_KEYDELTA(16)

	PORT_START("STICKY") // ADC08090 - 8 CHANNEL ANALOG - CHANNEL 3
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xfe) PORT_SENSITIVITY(100) PORT_KEYDELTA(16)
INPUT_PORTS_END

static INPUT_PORTS_START( tceptor2 )
	PORT_INCLUDE( tceptor )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x04, 0x00, "Mode" )
	PORT_DIPSETTING(    0x00, "2D" )
	PORT_DIPSETTING(    0x04, "3D" )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/*******************************************************************/

static const gfx_layout tile_layout =
{
	8, 8,
	512,
	2,
	{ 0x0000, 0x0004 }, //,  0x8000, 0x8004 },
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 0*8+0, 0*8+1, 0*8+2, 0*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	2*8*8
};

static GFXDECODE_START( gfx_tceptor )
	GFXDECODE_ENTRY( "txtiles", 0, tile_layout,     0,  256 )

	/* decode in video_start */
	//GFXDECODE_ENTRY( "bgtiles", 0, bg_layout,    2048,   64 )
	//GFXDECODE_ENTRY( "spr16tiles", 0, spr16_layout, 1024,   64 )
	//GFXDECODE_ENTRY( "spr32tiles", 0, spr32_layout, 1024,   64 )
GFXDECODE_END


/*******************************************************************/

void tceptor_state::machine_start()
{
	save_item(NAME(m_m6809_irq_enable));
	save_item(NAME(m_m68k_irq_enable));
	save_item(NAME(m_mcu_irq_enable));
}


/*******************************************************************/

void tceptor_state::machine_reset()
{
	m_m6809_irq_enable = 0;
	m_m68k_irq_enable = 0;
	m_mcu_irq_enable = 0;
}

/*******************************************************************/

void tceptor_state::tceptor(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 49.152_MHz_XTAL / 32);
	m_maincpu->set_addrmap(AS_PROGRAM, &tceptor_state::m6809_map);

	R65C02(config, m_audiocpu[0], 49.152_MHz_XTAL / 24);
	m_audiocpu[0]->set_addrmap(AS_PROGRAM, &tceptor_state::m6502_a_map);

	R65C02(config, m_audiocpu[1], 49.152_MHz_XTAL / 24);
	m_audiocpu[1]->set_addrmap(AS_PROGRAM, &tceptor_state::m6502_b_map);

	M68000(config, m_subcpu, 49.152_MHz_XTAL / 4);
	m_subcpu->set_addrmap(AS_PROGRAM, &tceptor_state::m68k_map);

	HD63701V0(config, m_mcu, 49.152_MHz_XTAL / 8); // or compatible 6808 with extra instructions
	m_mcu->set_addrmap(AS_PROGRAM, &tceptor_state::mcu_map);

	config.set_maximum_quantum(attotime::from_hz(6000));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	adc0809_device &adc(ADC0809(config, "adc", 768'000)); // unknown clock
	adc.in_callback<0>().set_constant(0); // unknown
	adc.in_callback<1>().set_ioport("PEDAL");
	adc.in_callback<2>().set_ioport("STICKX");
	adc.in_callback<3>().set_ioport("STICKY");

	// video hardware
	BUFFERED_SPRITERAM16(config, m_sprite_ram);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tceptor);
	PALETTE(config, m_palette, FUNC(tceptor_state::palette_init), 4096, 1024);

	NAMCO_C45_ROAD(config, m_c45_road);
	m_c45_road->set_palette(m_palette);
	m_c45_road->set_xoffset(-64);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.606060);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(38*8, 32*8);
	m_screen->set_visarea(2*8, 34*8-1 + 2*8, 0*8, 28*8-1 + 0);
	m_screen->set_screen_update(FUNC(tceptor_state::screen_update));
	m_screen->screen_vblank().set(FUNC(tceptor_state::screen_vblank));
	m_screen->set_palette(m_palette);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	ym2151_device &ym(YM2151(config, "ymsnd", 14.318181_MHz_XTAL / 4));
	ym.add_route(0, "speaker", 1.0, 0);
	ym.add_route(1, "speaker", 1.0, 1);

	NAMCO_CUS30(config, m_cus30, 49.152_MHz_XTAL / 2048);
	m_cus30->set_stereo(true);
	m_cus30->add_route(0, "speaker", 0.40, 0);
	m_cus30->add_route(1, "speaker", 0.40, 1);

	dac_8bit_r2r_device &dac(DAC_8BIT_R2R(config, "dac", 0)); // unknown DAC
	dac.add_route(ALL_OUTPUTS, "speaker", 0.4, 0);
	dac.add_route(ALL_OUTPUTS, "speaker", 0.4, 1);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tceptor )
	ROM_REGION( 0x10000, "maincpu", 0 )         // 68A09EP
	ROM_LOAD( "tc1-1.10f",  0x08000, 0x08000, CRC(4c6b063e) SHA1(d9701657186f8051391084f51a720037f9f418b1) )

	ROM_REGION( 0x10000, "audiocpu1", 0 )            // RP65C02
	ROM_LOAD( "tc1-21.1m",  0x08000, 0x08000, CRC(2d0b2fa8) SHA1(16ecd70954e52a8661642b15a5cf1db51783e444) )

	ROM_REGION( 0x10000, "audiocpu2", 0 )          // RP65C02
	ROM_LOAD( "tc1-22.3m",  0x08000, 0x08000, CRC(9f5a3e98) SHA1(2b2ffe39fe647a3039b92721817bddc9e9a92d82) )

	ROM_REGION( 0x110000, "sub", 0 )            // MC68000-12
	ROM_LOAD16_BYTE( "tc1-4.8c",     0x000000, 0x08000, CRC(ae98b673) SHA1(5da1c69dd40db9bad2e3d4dc2af3a949172af940) )
	ROM_LOAD16_BYTE( "tc1-3.10c",    0x000001, 0x08000, CRC(779a4b25) SHA1(8563213a1f1caee0eb88aa4bbd37c6004f16b309) )
	// socket 8d and 10d are emtpy

	ROM_REGION( 0x1000, "mcu", 0 )         // Custom 60A1
	ROM_LOAD( "cus60-60a1.mcu", 0x0000, 0x1000, CRC(076ea82a) SHA1(22b5e62e26390d7d5cacc0503c7aa5ed524204df) ) // MCU internal code

	ROM_REGION( 0x4000, "mcusub", 0 )
	ROM_LOAD( "tc1-2.3a",       0x0000, 0x4000, CRC(b6def610) SHA1(d0eada92a25d0243206fb8239374f5757caaea47) ) // subprogram for the MCU

	ROM_REGION( 0x02000, "txtiles", 0 )
	ROM_LOAD( "tc1-18.6b",  0x00000, 0x02000, CRC(662b5650) SHA1(ba82fe5efd1011854a6d0d7d87075475b65c0601) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "tc1-20.10e", 0x00000, 0x08000, CRC(3e5054b7) SHA1(ed359f8659a4a46d5ff7299d0da10550b1496db8) )
	ROM_LOAD( "tc1-19.10d", 0x08000, 0x04000, CRC(7406e6e7) SHA1(61ad77667e94fd7e11037da2721f7bbe0130286a) )

	ROM_REGION( 0x10000, "spr16tiles", 0 )
	ROM_LOAD( "tc1-16.8t",  0x00000, 0x08000, CRC(7c72be33) SHA1(397e11727b86688d550c28fbdcb864bb9335d891) )
	ROM_LOAD( "tc1-15.10t", 0x08000, 0x08000, CRC(51268075) SHA1(75b6b935c6721adbc984795b9bf0a791fb8b209e) )

	ROM_REGION( 0x80000, "spr32tiles", 0 )
	ROM_LOAD( "tc1-8.8m",   0x00000, 0x10000, CRC(192a1f1f) SHA1(8424a6a19c080da0a83e173e33915f4d9326f379) )
	ROM_LOAD( "tc1-10.8p",  0x10000, 0x08000, CRC(7876bcef) SHA1(09180b26d0eab51de18a13723f46d763541979fb) )
	ROM_RELOAD(             0x18000, 0x08000 )
	ROM_LOAD( "tc1-12.8r",  0x20000, 0x08000, CRC(e8f55842) SHA1(7397c8f279b9ddb7d9daf16f307669257a3fd9df) )
	ROM_RELOAD(             0x28000, 0x08000 )
	ROM_LOAD( "tc1-14.8s",  0x30000, 0x08000, CRC(723acf62) SHA1(fa62ffa2a641629803537d0ef1ad30688b04f9ca) )
	ROM_RELOAD(             0x38000, 0x08000 )
	ROM_LOAD( "tc1-7.10m",  0x40000, 0x10000, CRC(828c80d5) SHA1(6d441cbb333aee21f9c3d9608aec951130f9b0c5) )
	ROM_LOAD( "tc1-9.10p",  0x50000, 0x08000, CRC(145cf59b) SHA1(0639a36030823ccd7a476561a8fe61724c8be9d3) )
	ROM_RELOAD(             0x58000, 0x08000 )
	ROM_LOAD( "tc1-11.10r", 0x60000, 0x08000, CRC(ad7c6c7e) SHA1(2ae889c135c6ee924dc336895f7b9b8a98b715d0) )
	ROM_RELOAD(             0x68000, 0x08000 )
	ROM_LOAD( "tc1-13.10s", 0x70000, 0x08000, CRC(e67cef29) SHA1(ba8559caf498bbc1d9278d74da03ee2d910f76d8) )
	ROM_RELOAD(             0x78000, 0x08000 )

	ROM_REGION( 0x3500, "proms", 0 )
	ROM_LOAD( "tc1-3.1k",   0x00000, 0x00400, CRC(fd2fcb57) SHA1(97d5b7527714acfd729b26ac56f0a9210982c551) )    // red components
	ROM_LOAD( "tc1-1.1h",   0x00400, 0x00400, CRC(0241cf67) SHA1(9b2b579425b72a5b1f2c632f53d1c1d172b4ed1e) )    // green components
	ROM_LOAD( "tc1-2.1j",   0x00800, 0x00400, CRC(ea9eb3da) SHA1(0d7cfceac57afc53a063d7fe67cfc9bda0a8dbc8) )    // blue components
	ROM_LOAD( "tc1-5.6a",   0x00c00, 0x00400, CRC(afa8eda8) SHA1(783efbcbf0bb7e4cf2e2618ddd0ef3b52a4518cc) )    // tiles color table
	ROM_LOAD( "tc1-6.7s",   0x01000, 0x00400, CRC(72707677) SHA1(122c1b619c9efa3b7055908dda3102ee28230504) )    // sprite color table
	ROM_LOAD( "tc1-4.2e",   0x01400, 0x00100, CRC(a4e73d53) SHA1(df8231720e9b57cf2751f86ac3ed7433804f51ca) )    // road color table
	ROM_LOAD( "tc1-17.7k",  0x01500, 0x02000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )    // sprite related
ROM_END

ROM_START( tceptor2 )
	ROM_REGION( 0x10000, "maincpu", 0 )         // 68A09EP
	ROM_LOAD( "tc2-1.10f",  0x08000, 0x08000, CRC(f953f153) SHA1(f4cd0a133d23b4bf3c24c70c28c4ecf8ad4daf6f) )

	ROM_REGION( 0x10000, "audiocpu1", 0 )            // RP65C02
	ROM_LOAD( "tc1-21.1m",  0x08000, 0x08000, CRC(2d0b2fa8) SHA1(16ecd70954e52a8661642b15a5cf1db51783e444) )

	ROM_REGION( 0x10000, "audiocpu2", 0 )          // RP65C02
	ROM_LOAD( "tc1-22.3m",  0x08000, 0x08000, CRC(9f5a3e98) SHA1(2b2ffe39fe647a3039b92721817bddc9e9a92d82) )

	ROM_REGION( 0x110000, "sub", 0 )            // MC68000-12
	ROM_LOAD16_BYTE( "tc2-4.8c",     0x000000, 0x08000, CRC(6c2efc04) SHA1(3a91f5b8bbf7040083e2da2bd0fb2ab3c51ec45c) )
	ROM_LOAD16_BYTE( "tc2-3.10c",    0x000001, 0x08000, CRC(312b781a) SHA1(37bf3ced16b765d78bf8de7a4916c2b518b702ed) )
	ROM_LOAD16_BYTE( "tc2-6.8d",     0x100000, 0x08000, CRC(20711f14) SHA1(39623592bb4be3b3be2bff4b3219ac16ba612761) )
	ROM_LOAD16_BYTE( "tc2-5.10d",    0x100001, 0x08000, CRC(925f2560) SHA1(81fcef6a9c7e9dfb6884043cf2266854bc87cd69) )

	ROM_REGION( 0x1000, "mcu", 0 )         // Custom 60A1
	ROM_LOAD( "cus60-60a1.mcu", 0x0000, 0x1000, CRC(076ea82a) SHA1(22b5e62e26390d7d5cacc0503c7aa5ed524204df) ) // MCU internal code

	ROM_REGION( 0x4000, "mcusub", 0 )
	ROM_LOAD( "tc1-2.3a",       0x0000, 0x4000, CRC(b6def610) SHA1(d0eada92a25d0243206fb8239374f5757caaea47) ) // subprogram for the MCU

	ROM_REGION( 0x02000, "txtiles", 0 )
	ROM_LOAD( "tc1-18.6b",  0x00000, 0x02000, CRC(662b5650) SHA1(ba82fe5efd1011854a6d0d7d87075475b65c0601) )

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "tc2-20.10e", 0x00000, 0x08000, CRC(e72738fc) SHA1(53664400f343acdc1d8cf7e00e261ae42b857a5f) )
	ROM_LOAD( "tc2-19.10d", 0x08000, 0x04000, CRC(9c221e21) SHA1(58bcbb998dcf2190cf46dd3d22b116ac673285a6) )

	ROM_REGION( 0x10000, "spr16tiles", 0 )
	ROM_LOAD( "tc2-16.8t",  0x00000, 0x08000, CRC(dcf4da96) SHA1(e953cb46d60171271128b3e0ef4e958d1fab1d04) )
	ROM_LOAD( "tc2-15.10t", 0x08000, 0x08000, CRC(fb0a9f89) SHA1(cc9be6ff542b5d5e6ad3baca7a355b9bd31b3dd1) )

	ROM_REGION( 0x80000, "spr32tiles", 0 )
	ROM_LOAD( "tc2-8.8m",   0x00000, 0x10000, CRC(03528d79) SHA1(237810fa55c36b6d87c7e02e02f19feb64e5a11f) )
	ROM_LOAD( "tc2-10.8p",  0x10000, 0x10000, CRC(561105eb) SHA1(101a0e48a740ce4acc34a7d1a50191bb857e7371) )
	ROM_LOAD( "tc2-12.8r",  0x20000, 0x10000, CRC(626ca8fb) SHA1(0b51ced00b3de1f672f6f8c7cc5dd9e2ea2e4f8d) )
	ROM_LOAD( "tc2-14.8s",  0x30000, 0x10000, CRC(b9eec79d) SHA1(ae69033d6f80be0be883f919544c167e8f91db27) )
	ROM_LOAD( "tc2-7.10m",  0x40000, 0x10000, CRC(0e3523e0) SHA1(eb4670333ad383099fafda1c930f42e48e82f5c5) )
	ROM_LOAD( "tc2-9.10p",  0x50000, 0x10000, CRC(ccfd9ff6) SHA1(2934e098aa5231af18dbfb888fe05faab9576a7d) )
	ROM_LOAD( "tc2-11.10r", 0x60000, 0x10000, CRC(40724380) SHA1(57549094fc8403f1f528e57fe3fa64844bf89e22) )
	ROM_LOAD( "tc2-13.10s", 0x70000, 0x10000, CRC(519ec7c1) SHA1(c4abe279d7cf6f626dcbb6f6c4dc2a138b818f51) )

	ROM_REGION( 0x3500, "proms", 0 )
	ROM_LOAD( "tc2-3.1k",   0x00000, 0x00400, CRC(e3504f1a) SHA1(1ac3968e993030a6b2f4719702ff870267ab6918) )    // red components
	ROM_LOAD( "tc2-1.1h",   0x00400, 0x00400, CRC(e8a96fda) SHA1(42e5d2b351000ac0705b01ab484c5fe8e294a08b) )    // green components
	ROM_LOAD( "tc2-2.1j",   0x00800, 0x00400, CRC(c65eda61) SHA1(c316b748daa6be68eebbb480557637efc9f44781) )    // blue components
	ROM_LOAD( "tc1-5.6a",   0x00c00, 0x00400, CRC(afa8eda8) SHA1(783efbcbf0bb7e4cf2e2618ddd0ef3b52a4518cc) )    // tiles color table
	ROM_LOAD( "tc2-6.7s",   0x01000, 0x00400, CRC(badcda76) SHA1(726e0019241d31716f3af9ebe900089bce771477) )    // sprite color table
	ROM_LOAD( "tc2-4.2e",   0x01400, 0x00100, CRC(6b49fc30) SHA1(66ca39cd7985643acd71905111ae2d931c082465) )    // road color table
	ROM_LOAD( "tc1-17.7k",  0x01500, 0x02000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )    // sprite related
ROM_END

} // anonymous namespace


//     YEAR  NAME      PARENT   MACHINE  INPUT     CLASS          INIT        MONITOR  COMPANY  FULLNAME                 FLAGS )
GAME(  1986, tceptor,  0,       tceptor, tceptor,  tceptor_state, empty_init, ROT0,    "Namco", "Thunder Ceptor",        0)
GAMEL( 1986, tceptor2, tceptor, tceptor, tceptor2, tceptor_state, empty_init, ROT0,    "Namco", "3-D Thunder Ceptor II", 0, layout_tceptor2)
