// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    TC009xLVC device emulation

    Written by Angelo Salese, based off Taito L implementation

    TODO:
    - non-video stuff needs to be ported there as well
    - Verify difference between TC0090LVC and TC0091LVC

***************************************************************************/

#include "emu.h"
#include "tc009xlvc.h"

#include "emupal.h"
#include "screen.h"


DEFINE_DEVICE_TYPE(TC0090LVC, tc0090lvc_device, "tc0090lvc", "Taito TC0090LVC")
DEFINE_DEVICE_TYPE(TC0091LVC, tc0091lvc_device, "tc0091lvc", "Taito TC0091LVC")

void tc0090lvc_device::vram_w(offs_t offset, u8 data)
{
	// TODO: offset 0x0000 - 0x3fff is not used?

	m_vram[offset] = data;
	gfx(2)->mark_dirty(offset / 32);
	if (offset >= 0x8000 && offset < 0x9000) // 0x8000-0x9000 bg 0
	{
		offset -= 0x8000;
		m_bg_tilemap[0]->mark_tile_dirty(offset/2);
	}
	else if (offset >= 0x9000 && offset < 0xa000) // 0x9000-0xa000 bg 1
	{
		offset -= 0x9000;
		m_bg_tilemap[1]->mark_tile_dirty(offset/2);
	}
	else if (offset >= 0xa000 && offset < 0xb000) // 0xa000-0xb000 text
	{
		offset -= 0xa000;
		m_tx_tilemap->mark_tile_dirty(offset/2);
	}
	// 0xb000-0xb3ff sprites
}

void tc0090lvc_device::vregs_w(offs_t offset, u8 data)
{
	if ((offset & 0xfc) == 0)
	{
		m_bg_tilemap[0]->mark_all_dirty();
		m_bg_tilemap[1]->mark_all_dirty();
	}

	m_vregs[offset] = data;
}

static const gfx_layout layout_8x8 =
{
	8, 8,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ STEP4(3,-1), STEP4(4*4+3,-1) },
	{ STEP8(0,4*4*2) },
	8*8*4
};

static const gfx_layout layout_16x16 =
{
	16, 16,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ STEP4(3,-1), STEP4(4*4+3,-1), STEP4(8*8*4+3,-1), STEP4(8*8*4+4*4+3,-1) },
	{ STEP8(0,4*4*2), STEP8(8*8*4*2,4*4*2) },
	8*8*4*4
};

GFXDECODE_MEMBER(tc0090lvc_device::gfxinfo)
	GFXDECODE_DEVICE("gfx",      0, layout_8x8,   0, 16)
	GFXDECODE_DEVICE("gfx",      0, layout_16x16, 0, 16)
	GFXDECODE_DEVICE_RAM("vram", 0, layout_8x8,   0, 16)
GFXDECODE_END

void tc0090lvc_device::cpu_map(address_map &map)
{
	// 0x0000-0x7fff ROM (0x0000-0x5fff Fixed, 0x6000-0x7fff Bankswitched)
	map(0x0000, 0x5fff).r(FUNC(tc0090lvc_device::rom_r));
	map(0x6000, 0x7fff).lr8(NAME([this] (offs_t offset) { return rom_r((m_rom_bank << 13) | (offset & 0x1fff)); }));

	// 0x8000-0xbfff External mappable area

	// 0xc000-0xfdff RAM banks (Connected in VRAMs, 4KB boundary)
	map(0xc000, 0xfdff).rw(FUNC(tc0090lvc_device::banked_vram_r), FUNC(tc0090lvc_device::banked_vram_w));

	// 0xfe00-0xffff Internal functions
}

void tc0090lvc_device::vram_map(address_map &map)
{
	map(0x010000, 0x01ffff).readonly().share("vram");
	// NOTE: the way tiles are addressed suggests that 0x0000-0x3fff of this might be usable,
	//       but we don't map it anywhere, so the first tiles are always blank at the moment.
	map(0x014000, 0x01ffff).lw8(NAME([this] (offs_t offset, u8 data) { vram_w(offset + 0x4000, data); }));
	map(0x040000, 0x05ffff).ram().share("bitmap_ram");
	map(0x080000, 0x0801ff).ram().mirror(0x00e00).w("palette", FUNC(palette_device::write8)).share("palette");
}

tc0090lvc_device::tc0090lvc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tc0090lvc_device(mconfig, TC0090LVC, tag, owner, clock)
{
}

tc0090lvc_device::tc0090lvc_device(const machine_config &mconfig, device_type &type, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, type, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, gfxinfo, "palette")
	, m_program_space_config("program", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(tc0090lvc_device::cpu_map), this))
	, m_vram_space_config("vram_space", ENDIANNESS_LITTLE, 8, 20, 0, address_map_constructor(FUNC(tc0090lvc_device::vram_map), this))
	, m_io_space_config("io", ENDIANNESS_LITTLE, 8, 16, 0) // TODO: IO space exists like original Z80?
	, m_tilemap_xoffs(0)
	, m_tilemap_yoffs(0)
	, m_tilemap_flipped_xoffs(0)
	, m_tilemap_flipped_yoffs(0)
	, m_irq_enable(0)
	, m_vram(*this, "vram")
	, m_bitmap_ram(*this, "bitmap_ram")
	, m_rom(*this, DEVICE_SELF)
	, m_tile_cb(*this)
{
}

tc0091lvc_device::tc0091lvc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tc0090lvc_device(mconfig, TC0091LVC, tag, owner, clock)
{
}

device_memory_interface::space_config_vector tc0090lvc_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_space_config),
		std::make_pair(AS_DATA,    &m_vram_space_config),
		std::make_pair(AS_IO,      &m_io_space_config)
	};
}

/*
    Scroll Tilemap format (2 bytes per tiles, 64x32 tilemap)

    Offset Bits     Description
           76543210
    00     xxxxxxxx Code (bit 0-7)
    01     ------xx Code (bit 8-9)
           ----xx-- Bank select (0x400 code boundary)
           xxxx---- Palette select (16 color boundary)
*/

template<unsigned Offset>
TILE_GET_INFO_MEMBER(tc0090lvc_device::get_tile_info)
{
	const u16 attr = m_vram[Offset + (2 * tile_index) + 1];
	u32 code = m_vram[Offset + (2 * tile_index)]
			| ((attr & 0x03) << 8)
			| ((tilebank((attr & 0xc) >> 2)) << 10);

	m_tile_cb(code);

	tileinfo.set(0,
			code,
			(attr & 0xf0) >> 4,
			0);
}

/*
    Fixed Tilemap format (2 bytes per tiles, 64x32 tilemap)

    Offset Bits     Description
           76543210
    00     xxxxxxxx Code (bit 0-7)
    01     -----xxx Code (bit 8-10)
           xxxx---- Palette select (16 color boundary)
*/

TILE_GET_INFO_MEMBER(tc0090lvc_device::get_tx_tile_info)
{
	const u16 attr = m_vram[0xa000 + (2 * tile_index) + 1];
	const u16 code = m_vram[0xa000 + (2 * tile_index)]
			| ((attr & 0x07) << 8);

	tileinfo.set(2,
			code,
			(attr & 0xf0) >> 4,
			0);
}


void tc0090lvc_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, "palette", palette_device::BLACK).set_format(palette_device::xBGRBBBBGGGGRRRR_bit0, 0x100);
}


void tc0090lvc_device::device_post_load()
{
	gfx(2)->mark_all_dirty();
}


void tc0090lvc_device::device_start()
{
	// rom_r assumes it can make a mask with (m_rom.length() - 1)
	assert(!(m_rom.length() & (m_rom.length() - 1)));

	z80_device::device_start();

	space(AS_DATA).specific(m_vram_space);

	m_tile_cb.resolve_safe();

	std::fill_n(&m_vram[0], m_vram.bytes(), 0);
	std::fill_n(&m_bitmap_ram[0], m_bitmap_ram.bytes(), 0);
	std::fill(std::begin(m_ram_bank), std::end(m_ram_bank), 0);

	m_vregs = make_unique_clear<u8[]>(0x100);
	m_sprram_buffer = make_unique_clear<u8[]>(0x400);

	m_tx_tilemap    = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(tc0090lvc_device::get_tx_tile_info)),      TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(tc0090lvc_device::get_tile_info<0x8000>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(tc0090lvc_device::get_tile_info<0x9000>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tx_tilemap->set_transparent_pen(0);
	m_bg_tilemap[0]->set_transparent_pen(0);
	m_bg_tilemap[1]->set_transparent_pen(0);

	m_tx_tilemap->set_scrolldx((-8) + m_tilemap_xoffs, (-8) + m_tilemap_flipped_xoffs);
	m_bg_tilemap[0]->set_scrolldx(28 + m_tilemap_xoffs, (-11) + m_tilemap_flipped_xoffs);
	m_bg_tilemap[1]->set_scrolldx(38 + m_tilemap_xoffs, (-21) + m_tilemap_flipped_xoffs);

	m_tx_tilemap->set_scrolldy(m_tilemap_yoffs, m_tilemap_flipped_yoffs);
	m_bg_tilemap[0]->set_scrolldy(m_tilemap_yoffs, m_tilemap_flipped_yoffs);
	m_bg_tilemap[1]->set_scrolldy(m_tilemap_yoffs, m_tilemap_flipped_yoffs);

	save_item(NAME(m_bg_scroll));
	save_item(NAME(m_irq_vector));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_rom_bank));
	save_pointer(NAME(m_vregs), 0x100);
	save_pointer(NAME(m_sprram_buffer), 0x400);
}

void tc0090lvc_device::device_reset()
{
	// needs for taito_l.cpp
	for (int i = 0; i < 3; i++)
		m_irq_vector[i] = 0;

	m_irq_enable = 0;

	for (int i = 0; i < 4; i++)
		m_ram_bank[i] = 0x80;

	m_rom_bank = 0;

	z80_device::device_reset();

	/* video related */
	m_vregs[0] = m_vregs[1] = m_vregs[2] = m_vregs[3] = m_vregs[4] = 0;
}

void tc0090lvc_device::mark_all_layer_dirty()
{
	m_tx_tilemap->mark_all_dirty();
	m_bg_tilemap[0]->mark_all_dirty();
	m_bg_tilemap[1]->mark_all_dirty();
}

/*
    Sprite format (8 bytes per sprites, max 125 entries (0x3e8 bytes))

    Offset Bits     Description
           76543210
    00     xxxxxxxx Code (bit 0-7)
    01     xxxxxxxx Code (bit 8-15)
    02     ----xxxx Palette select (16 color boundary)
    03     ------x- Flip Y
           -------x Flip X
    04     xxxxxxxx X position (bit 0-7)
    05     -------x X position (bit 8, unsigned)
    06     xxxxxxxx Y position (unsigned)
    07     xxxxxxxx unknown / ignored? Seems just garbage in many cases, e.g
                    plgirs2 bullets and raimais big bosses.
*/

void tc0090lvc_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int count = 0; count < 0x3e7; count += 8)
	{
		u32 code = m_sprram_buffer[count + 0] | (m_sprram_buffer[count + 1] << 8);
		int x = m_sprram_buffer[count + 4] | ((m_sprram_buffer[count + 5] & 1) << 8);
		if (x >= cliprect.max_x)
			x -= 512;
		int y = m_sprram_buffer[count + 6];
		if (y >= cliprect.max_y)
			y -= 256;
		const u32 col = m_sprram_buffer[count + 2] & 0x0f;
		int fx = m_sprram_buffer[count + 3] & 0x1;
		int fy = m_sprram_buffer[count + 3] & 0x2;

		// each sprite is 4 8x8 tile group, actually tile number for each tile is << 2 of real address
		code <<= 2;
		m_tile_cb(code);
		code >>= 2;

		if (global_flip())
		{
			x = 304 - x;
			y = 240 - y;
			fx = !fx;
			fy = !fy;
		}

		gfx(1)->prio_transpen(bitmap, cliprect, code, col, fx, fy, x, y, screen.priority(), (col & 0x08) ? 0xaa : 0x00, 0);
	}
}

u32 tc0090lvc_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!screen_enable())
	{
		bitmap.fill(palette().black_pen(), cliprect);
		return 0;
	}

	if (bitmap_mode()) // 8bpp bitmap enabled
	{
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			const int res_y = (global_flip()) ? 256 - y : y;
			u32 count = (res_y & 0xff) * 512;

			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				const int res_x = (global_flip()) ? 320 - x : x;

				bitmap.pix(y, x) = palette().pen(m_bitmap_ram[count + (res_x & 0x1ff)]);
			}
		}
	}
	else
	{
		update_scroll(&m_vram[0xb000]); // TODO: not buffered?

		machine().tilemap().set_flip_all(global_flip() ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

		int dx = m_bg_scroll[0][0] | (m_bg_scroll[0][1] << 8);
		if (global_flip()) { dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf; }
		int dy = m_bg_scroll[0][2];

		m_bg_tilemap[0]->set_scrollx(0, -dx);
		m_bg_tilemap[0]->set_scrolly(0, -dy);

		dx = m_bg_scroll[1][0] | (m_bg_scroll[1][1] << 8);
		if (global_flip()) { dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf; }
		dy = m_bg_scroll[1][2];

		m_bg_tilemap[1]->set_scrollx(0, -dx);
		m_bg_tilemap[1]->set_scrolly(0, -dy);

		screen.priority().fill(0, cliprect);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0); // TODO: opaque flag?
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, (bg0_pri()) ? 0 : 1);
		draw_sprites(screen, bitmap, cliprect);
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	return 0;
}

u32 tc0091lvc_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!screen_enable())
	{
		bitmap.fill(palette().black_pen(), cliprect);
		return 0;
	}

	if (bitmap_mode()) // 8bpp bitmap enabled
	{
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			const int res_y = (global_flip()) ? 256 - y : y;
			u32 count = (res_y & 0xff) * 512;

			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				const int res_x = (global_flip()) ? 320 - x : x;

				bitmap.pix(y, x) = palette().pen(m_bitmap_ram[count + (res_x & 0x1ff)]);
			}
		}
	}
	else
	{
		machine().tilemap().set_flip_all(global_flip() ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

		int dx = m_bg_scroll[0][0] | (m_bg_scroll[0][1] << 8);
		if (global_flip()) { dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf; }
		int dy = m_bg_scroll[0][2];

		m_bg_tilemap[0]->set_scrollx(0, -dx);
		m_bg_tilemap[0]->set_scrolly(0, -dy);

		dx = m_bg_scroll[1][0] | (m_bg_scroll[1][1] << 8);
		if (global_flip()) { dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf; }
		dy = m_bg_scroll[1][2];

		m_bg_tilemap[1]->set_scrollx(0, -dx);
		m_bg_tilemap[1]->set_scrolly(0, -dy);

		screen.priority().fill(0, cliprect);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0); // TODO: opaque flag?
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, (bg0_pri()) ? 0 : 1);
		draw_sprites(screen, bitmap, cliprect);
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	return 0;
}

void tc0090lvc_device::screen_eof()
{
	std::copy_n(&m_vram[0xb000], 0x400, &m_sprram_buffer[0]);
}

void tc0091lvc_device::screen_eof()
{
	tc0090lvc_device::screen_eof();
	update_scroll(m_sprram_buffer.get()); // buffered in TC0091LVC?
}

void tc0090lvc_device::update_scroll(u8 *ram)
{
	m_bg_scroll[0][0] = ram[0x3f4];
	m_bg_scroll[0][1] = ram[0x3f5];
	m_bg_scroll[0][2] = ram[0x3f6];

	m_bg_scroll[1][0] = ram[0x3fc];
	m_bg_scroll[1][1] = ram[0x3fd];
	m_bg_scroll[1][2] = ram[0x3fe];
}
