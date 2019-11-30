// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    TC009xLVC device emulation

    Written by Angelo Salese, based off Taito L implementation

    TODO:
    - non-video stuff needs to be ported there as well

***************************************************************************/

#include "emu.h"
#include "machine/tc009xlvc.h"

#include "emupal.h"
#include "screen.h"


DEFINE_DEVICE_TYPE(TC0091LVC, tc0091lvc_device, "tc009xlvc", "Taito TC0091LVC")

void tc0091lvc_device::vram_w(offs_t offset, u8 data)
{
	// TODO : offset 0x0000 - 0x3fff is not used?

	m_vram[offset] = data;
	gfx(2)->mark_dirty(offset / 32);
	if (offset >= 0x8000 && offset < 0x9000) // 0x8000-0x9000 bg 0
	{
		offset -= 0x8000;
		bg_tilemap[0]->mark_tile_dirty(offset/2);
	}
	else if (offset >= 0x9000 && offset < 0xa000) // 0x9000-0xa000 bg 1
	{
		offset -= 0x9000;
		bg_tilemap[1]->mark_tile_dirty(offset/2);
	}
	else if (offset >= 0xa000 && offset < 0xb000) // 0xa000-0xb000 text
	{
		offset -= 0xa000;
		tx_tilemap->mark_tile_dirty(offset/2);
	}
}

void tc0091lvc_device::vregs_w(offs_t offset, u8 data)
{
	if ((offset & 0xfc) == 0)
	{
		bg_tilemap[0]->mark_all_dirty();
		bg_tilemap[1]->mark_all_dirty();
	}

	m_vregs[offset] = data;
}

void tc0091lvc_device::ram_bank_w(offs_t offset, u8 data)
{
	m_ram_bank[offset] = data;
	m_bankdev[offset]->set_bank(m_ram_bank[offset]);
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

GFXDECODE_MEMBER(tc0091lvc_device::gfxinfo)
	GFXDECODE_DEVICE("gfx",      0, layout_8x8,   0, 16)
	GFXDECODE_DEVICE("gfx",      0, layout_16x16, 0, 16)
	GFXDECODE_DEVICE_RAM("vram", 0, layout_8x8,   0, 16)
GFXDECODE_END

void tc0091lvc_device::cpu_map(address_map &map)
{
	// 0x0000-0x7fff ROM (0x0000-0x5fff Fixed, 0x6000-0x7fff Bankswitched)
	map(0x0000, 0x5fff).r(FUNC(tc0091lvc_device::rom_r));
	map(0x6000, 0x7fff).lr8(NAME([this] (offs_t offset) { return rom_r((m_rom_bank << 13) | (offset & 0x1fff)); }));

	// 0x8000-0xbfff External mappable area

	// 0xc000-0xfdff RAM banks (Connected in VRAMs, 4KB boundary)
	map(0xc000, 0xcfff).m(m_bankdev[0], FUNC(address_map_bank_device::amap8));
	map(0xd000, 0xdfff).m(m_bankdev[1], FUNC(address_map_bank_device::amap8));
	map(0xe000, 0xefff).m(m_bankdev[2], FUNC(address_map_bank_device::amap8));
	map(0xf000, 0xfdff).m(m_bankdev[3], FUNC(address_map_bank_device::amap8));

	// 0xfe00-0xffff Internal functions
}

void tc0091lvc_device::banked_map(address_map &map)
{
	map(0x010000, 0x01ffff).readonly().share("vram");
	// note, the way tiles are addressed suggests that 0x0000-0x3fff of this might be usable,
	//       but we don't map it anywhere, so the first tiles are always blank at the moment.
	map(0x014000, 0x01ffff).lw8(NAME([this] (offs_t offset, u8 data) { vram_w(offset + 0x4000, data); }));
	map(0x040000, 0x05ffff).ram().share("bitmap_ram");
	map(0x080000, 0x0801ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
}

tc0091lvc_device::tc0091lvc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TC0091LVC, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, gfxinfo, "palette")
	, m_bankdev(*this, "bankdev_%u", 0U)
	, m_vram(*this, "vram")
	, m_bitmap_ram(*this, "bitmap_ram")
	, m_rom(*this, DEVICE_SELF)
{
}

template<unsigned Offset>
TILE_GET_INFO_MEMBER(tc0091lvc_device::get_tile_info)
{
	const u16 attr = m_vram[Offset + (2 * tile_index) + 1];
	const u16 code = m_vram[Offset + (2 * tile_index)]
			| ((attr & 0x03) << 8)
			| ((m_vregs[(attr & 0xc) >> 2]) << 10);
//          | (state->m_horshoes_gfxbank << 12);

	SET_TILE_INFO_MEMBER(0,
			code,
			(attr & 0xf0) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(tc0091lvc_device::get_tx_tile_info)
{
	const u16 attr = m_vram[0xa000 + (2 * tile_index) + 1];
	const u16 code = m_vram[0xa000 + (2 * tile_index)]
			| ((attr & 0x07) << 8);

	SET_TILE_INFO_MEMBER(2,
			code,
			(attr & 0xf0) >> 4,
			0);
}


void tc0091lvc_device::device_add_mconfig(machine_config &config)
{
	for (int i = 0; i < 4; i++)
	{
		ADDRESS_MAP_BANK(config, m_bankdev[i]).set_map(&tc0091lvc_device::banked_map).set_options(ENDIANNESS_LITTLE, 8, 20, 0x1000);
	}
	PALETTE(config, "palette", palette_device::BLACK).set_format(palette_device::xBGRBBBBGGGGRRRR_bit0, 0x100);
}


void tc0091lvc_device::device_post_load()
{
	for (int i = 0; i < 4; i++)
		m_bankdev[i]->set_bank(m_ram_bank[i]);
}


void tc0091lvc_device::device_start()
{
	std::fill_n(&m_vram[0], m_vram.bytes(), 0);
	std::fill_n(&m_bitmap_ram[0], m_bitmap_ram.bytes(), 0);
	std::fill(std::begin(m_ram_bank), std::end(m_ram_bank), 0);
	for (int i = 0; i < 4; i++)
		m_bankdev[i]->set_bank(m_ram_bank[i]);

	m_rom_bank = 0;

	m_vregs = make_unique_clear<u8[]>(0x100);
	m_sprram_buffer = make_unique_clear<u8[]>(0x400);

	tx_tilemap    = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(tc0091lvc_device::get_tx_tile_info)),      TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	bg_tilemap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(tc0091lvc_device::get_tile_info<0x8000>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	bg_tilemap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(tc0091lvc_device::get_tile_info<0x9000>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	tx_tilemap->set_transparent_pen(0);
	bg_tilemap[0]->set_transparent_pen(0);
	bg_tilemap[1]->set_transparent_pen(0);

	tx_tilemap->set_scrolldx(-8, -8);
	bg_tilemap[0]->set_scrolldx(28, -11);
	bg_tilemap[1]->set_scrolldx(38, -21);

	save_item(NAME(m_irq_vector));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_rom_bank));
	save_pointer(NAME(m_vregs), 0x100);
	save_pointer(NAME(m_sprram_buffer), 0x400);
}


void tc0091lvc_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u8 global_flip)
{
	for (int count = 0; count < 0x3e7; count += 8)
	{
		const u32 code = m_sprram_buffer[count + 0] | (m_sprram_buffer[count + 1] << 8);
		int x = m_sprram_buffer[count + 4] | (m_sprram_buffer[count + 5] << 8);
		if (x >= 320)
			x -= 512;
		int y = m_sprram_buffer[count + 6];
		const u32 col = (m_sprram_buffer[count + 2]) & 0x0f;
		int fx = m_sprram_buffer[count + 3] & 0x1;
		int fy = m_sprram_buffer[count + 3] & 0x2;

		if (global_flip)
		{
			x = 304 - x;
			y = 240 - y;
			fx = !fx;
			fy = !fy;
		}

		gfx(1)->prio_transpen(bitmap, cliprect, code, col, fx, fy, x, y, screen.priority(), (col & 0x08) ? 0xaa : 0x00, 0);
	}
}

u32 tc0091lvc_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(palette().black_pen(), cliprect);

	if ((m_vregs[4] & 0x20) == 0)
		return 0;

	const u8 global_flip = m_vregs[4] & 0x10;

	if ((m_vregs[4] & 0x7) == 7) // 8bpp bitmap enabled
	{
		u32 count = 0;

		for (int y = 0; y < 256; y++)
		{
			for (int x = 0; x < 512; x++)
			{
				const int res_x = (global_flip) ? 320 - x : x;
				const int res_y = (global_flip) ? 256 - y : y;

				if (cliprect.contains(res_x, res_y))
					bitmap.pix16(res_y, res_x) = palette().pen(m_bitmap_ram[count]);

				count++;
			}
		}
	}
	else
	{
		machine().tilemap().set_flip_all(global_flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

		int dx = m_bg_scroll[0][0] | (m_bg_scroll[0][1] << 8);
		if (global_flip) { dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf; dx += 192; }
		int dy = m_bg_scroll[0][2];

		bg_tilemap[0]->set_scrollx(0, -dx);
		bg_tilemap[0]->set_scrolly(0, -dy);

		dx = m_bg_scroll[1][0] | (m_bg_scroll[1][1] << 8);
		if (global_flip) { dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf; dx += 192; }
		dy = m_bg_scroll[1][2];

		bg_tilemap[1]->set_scrollx(0, -dx);
		bg_tilemap[1]->set_scrolly(0, -dy);

		tx_tilemap->set_scrollx(0, (global_flip) ? -192 : 0);

		screen.priority().fill(0, cliprect);
		bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
		bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, (m_vregs[4] & 0x8) ? 0 : 1);
		draw_sprites(screen, bitmap, cliprect, global_flip);
		tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	return 0;
}

void tc0091lvc_device::screen_eof(void)
{
	std::copy_n(&m_vram[0xb000], 0x400, &m_sprram_buffer[0]);
	m_bg_scroll[0][0] = m_sprram_buffer[0x3f4];
	m_bg_scroll[0][1] = m_sprram_buffer[0x3f5];
	m_bg_scroll[0][2] = m_sprram_buffer[0x3f6];

	m_bg_scroll[1][0] = m_sprram_buffer[0x3fc];
	m_bg_scroll[1][1] = m_sprram_buffer[0x3fd];
	m_bg_scroll[1][2] = m_sprram_buffer[0x3fe];
}
