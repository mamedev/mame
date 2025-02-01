// license:BSD-3-Clause
// copyright-holders:Ville Linde, hap, Nicola Salmoria
/******************************************************************************

    Seibu SPI hardware

    Functions to emulate the video hardware

******************************************************************************/

#include "emu.h"
#include "seibuspi.h"
#include "seibuspi_m.h"
#include "screen.h"

#include <algorithm>

/**************************************************************************

Tile encryption
---------------

The tile graphics encryption uses the same algorithm in all games. This is
similar to, but simpler than, that used by the SEI252, RISE10 and RISE11
custom chips.

- Take 24 bits of gfx data (used to decrypt 4 pixels at 6 bpp) and perform
  a bit permutation on them (the permutation is the same in all games).
- Take the low 12 bits of the tile code and add a 24-bit number (KEY1) to it.
- Add the two 24-bit numbers resulting from the above steps, but with a
  catch: while performing the sum, some bits generate carry as usual, other
  bits don't, depending on a 24-bit key (KEY2). Note that the carry generated
  by bit 23 (if enabled) wraps around to bit 0.
- XOR the result with a 24-bit number (KEY3).

The decryption is actually programmable; the games write the key to the
custom CRTC on startup!! (writes to 000414)

**************************************************************************/


static u32 decrypt_tile(u32 val, int tileno, u32 key1, u32 key2, u32 key3)
{
	val = bitswap<24>(val, 18,19,9,5, 10,17,16,20, 21,22,6,11, 15,14,4,23, 0,1,7,8, 13,12,3,2);

	return partial_carry_sum24( val, tileno + key1, key2 ) ^ key3;
}

static void decrypt_text(u8 *rom, u32 key1, u32 key2, u32 key3)
{
	for (int i = 0; i < 0x10000; i++)
	{
		u32 w = (rom[(i * 3) + 0] << 16) | (rom[(i * 3) + 1] << 8) | (rom[(i * 3) + 2]);

		w = decrypt_tile(w, i >> 4, key1, key2, key3);

		rom[(i * 3) + 0] = (w >> 16) & 0xff;
		rom[(i * 3) + 1] = (w >> 8) & 0xff;
		rom[(i * 3) + 2] = w & 0xff;
	}
}

static void decrypt_bg(u8 *rom, int size, u32 key1, u32 key2, u32 key3)
{
	for (int j = 0; j < size; j += 0xc0000)
	{
		for (int i = 0; i < 0x40000; i++)
		{
			u32 w = (rom[j + (i * 3) + 0] << 16) | (rom[j + (i * 3) + 1] << 8) | (rom[j + (i * 3) + 2]);

			w = decrypt_tile(w, i >> 6, key1, key2, key3);

			rom[j + (i * 3) + 0] = (w >> 16) & 0xff;
			rom[j + (i * 3) + 1] = (w >> 8) & 0xff;
			rom[j + (i * 3) + 2] = w & 0xff;
		}
	}
}

/******************************************************************************************
cpu #0 (PC=0033B2EB): unmapped program memory dword write to 00000414 = 00000000 & 0000FFFF
cpu #0 (PC=0033B2EB): unmapped program memory dword write to 00000414 = 0000DF5B & 0000FFFF
cpu #0 (PC=0033B2EB): unmapped program memory dword write to 00000414 = 000078CF & 0000FFFF
cpu #0 (PC=0033B2EB): unmapped program memory dword write to 00000414 = 00001377 & 0000FFFF
cpu #0 (PC=0033B2EB): unmapped program memory dword write to 00000414 = 00002538 & 0000FFFF
cpu #0 (PC=0033B2EB): unmapped program memory dword write to 00000414 = 00004535 & 0000FFFF
cpu #0 (PC=0033B2EB): unmapped program memory dword write to 00000414 = 06DC0000 & FFFF0000
******************************************************************************************/

void seibuspi_state::text_decrypt(u8 *rom)
{
	decrypt_text( rom, 0x5a3845, 0x77cf5b, 0x1378df);
}

void seibuspi_state::bg_decrypt(u8 *rom, int size)
{
	decrypt_bg( rom, size, 0x5a3845, 0x77cf5b, 0x1378df);
}

/******************************************************************************************
cpu #0 (PC=002A097D): unmapped program memory dword write to 00000414 = 00000001 & 0000FFFF
cpu #0 (PC=002A097D): unmapped program memory dword write to 00000414 = 0000DCF8 & 0000FFFF
cpu #0 (PC=002A097D): unmapped program memory dword write to 00000414 = 00007AE2 & 0000FFFF
cpu #0 (PC=002A097D): unmapped program memory dword write to 00000414 = 0000154D & 0000FFFF
cpu #0 (PC=002A097D): unmapped program memory dword write to 00000414 = 00001731 & 0000FFFF
cpu #0 (PC=002A097D): unmapped program memory dword write to 00000414 = 0000466B & 0000FFFF
cpu #0 (PC=002A097D): unmapped program memory dword write to 00000414 = 3EDC0000 & FFFF0000
******************************************************************************************/

void seibuspi_state::rdft2_text_decrypt(u8 *rom)
{
	decrypt_text( rom, 0x823146, 0x4de2f8, 0x157adc);
}

void seibuspi_state::rdft2_bg_decrypt(u8 *rom, int size)
{
	decrypt_bg( rom, size, 0x823146, 0x4de2f8, 0x157adc);
}

/******************************************************************************************
cpu #0 (PC=002C40F9): unmapped program memory dword write to 00000414 = 00000001 & 0000FFFF
cpu #0 (PC=002C40F9): unmapped program memory dword write to 00000414 = 00006630 & 0000FFFF
cpu #0 (PC=002C40F9): unmapped program memory dword write to 00000414 = 0000B685 & 0000FFFF
cpu #0 (PC=002C40F9): unmapped program memory dword write to 00000414 = 0000CCFE & 0000FFFF
cpu #0 (PC=002C40F9): unmapped program memory dword write to 00000414 = 000032A7 & 0000FFFF
cpu #0 (PC=002C40F9): unmapped program memory dword write to 00000414 = 0000547C & 0000FFFF
cpu #0 (PC=002C40F9): unmapped program memory dword write to 00000414 = 3EDC0000 & FFFF0000
******************************************************************************************/

void seibuspi_state::rfjet_text_decrypt(u8 *rom)
{
	decrypt_text( rom, 0xaea754, 0xfe8530, 0xccb666);
}

void seibuspi_state::rfjet_bg_decrypt(u8 *rom, int size)
{
	decrypt_bg( rom, size, 0xaea754, 0xfe8530, 0xccb666);
}

void seibuspi_state::tile_decrypt_key_w(u16 data)
{
	if (data != 0 && data != 1)
		logerror("Decryption key: %04X\n", data);
}


/*****************************************************************************/

void seibuspi_state::set_layer_offsets()
{
	if (m_rowscroll_enable)
	{
		m_fore_layer_offset = 0x1000 / 4;
		m_midl_layer_offset = 0x2000 / 4;
		m_text_layer_offset = 0x3000 / 4;
	}
	else
	{
		m_fore_layer_offset = 0x1000 / 4 / 2;
		m_midl_layer_offset = 0x2000 / 4 / 2;
		m_text_layer_offset = 0x3000 / 4 / 2;
	}

	m_fore_layer_d13 = m_layer_bank << 2 & 0x2000;
	m_back_layer_d14 = m_rf2_layer_bank << 14 & 0x4000;
	m_midl_layer_d14 = m_rf2_layer_bank << 13 & 0x4000;
	m_fore_layer_d14 = m_rf2_layer_bank << 12 & 0x4000;
}

void seibuspi_state::spi_layer_bank_w(offs_t offset, u16 data, u16 mem_mask)
{
	//logerror("Writing %04X to layer register\n", data);

	// r000f000 0010100a
	// r: rowscroll enable
	// f: fore layer d13
	// a: ? (0 in ejanhs and rdft22kc, 1 in all other games)
	const u16 prev = m_layer_bank;
	COMBINE_DATA(&m_layer_bank);

	m_rowscroll_enable = BIT(m_layer_bank, 15);
	set_layer_offsets();

	if ((prev ^ m_layer_bank) & 0x0800)
		m_fore_layer->mark_all_dirty();
}


void seibuspi_state::rf2_layer_bank_w(u8 data)
{
	// 00000fmb
	// f: fore layer d14
	// m: middle layer d14
	// b: back layer d14
	const u8 prev = m_rf2_layer_bank;
	m_rf2_layer_bank = data;
	set_layer_offsets();

	if ((prev ^ m_rf2_layer_bank) & 1)
		m_back_layer->mark_all_dirty();

	if ((prev ^ m_rf2_layer_bank) & 2)
		m_midl_layer->mark_all_dirty();

	if ((prev ^ m_rf2_layer_bank) & 4)
		m_fore_layer->mark_all_dirty();
}

void seibuspi_state::spi_layer_enable_w(offs_t offset, u16 data, u16 mem_mask)
{
	// 00000000 000stfmb (0=on, 1=off)
	// s: sprite layer
	// t: text layer
	// f: fore layer
	// m: middle layer
	// b: back layer
	COMBINE_DATA(&m_layer_enable);
}

void seibuspi_state::scroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_scrollram[offset]);
}

void seibuspi_state::video_dma_length_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_video_dma_length);
}

void seibuspi_state::video_dma_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_video_dma_address);
}


/*****************************************************************************/

void seibuspi_state::tilemap_dma_start_w(u32 data)
{
	if (!m_tilemap_ram)
		return;

	// safety check
	const int dma_length_user = m_rowscroll_enable ? 0x4000 : 0x2800;
	const int dma_length_real = (m_video_dma_length + 1) * 2; // ideally we should be using this, let's check if we have to:
	if (m_video_dma_length != 0 && dma_length_user != dma_length_real)
		popmessage("Tile LEN %X %X, contact MAMEdev", dma_length_user, dma_length_real); // shouldn't happen
	else if (!DWORD_ALIGNED(m_video_dma_address) || (m_video_dma_length & 3) != 3 || (m_video_dma_address + dma_length_user) > 0x40000)
		popmessage("Tile DMA %X %X, contact MAMEdev", m_video_dma_address, m_video_dma_length); // shouldn't happen
	if (m_video_dma_address < 0x800)
		logerror("tilemap_dma_start_w in I/O area: %X\n", m_video_dma_address);

	int index = m_video_dma_address / 4;

	// back layer
	for (int i = 0; i < 0x800/4; i++)
	{
		const u32 tile = m_mainram[index];
		if (m_tilemap_ram[i] != tile)
		{
			m_tilemap_ram[i] = tile;
			m_back_layer->mark_tile_dirty((i * 2));
			m_back_layer->mark_tile_dirty((i * 2) + 1);
		}
		index++;
	}

	// back layer row scroll
	if (m_rowscroll_enable)
	{
		std::copy_n(&m_mainram[index], 0x800/4, &m_tilemap_ram[0x800/4]);
		index += 0x800/4;
	}

	// fore layer
	for (int i = 0; i < 0x800/4; i++)
	{
		const u32 tile = m_mainram[index];
		if (m_tilemap_ram[i+m_fore_layer_offset] != tile)
		{
			m_tilemap_ram[i+m_fore_layer_offset] = tile;
			m_fore_layer->mark_tile_dirty((i * 2));
			m_fore_layer->mark_tile_dirty((i * 2) + 1);
		}
		index++;
	}

	// fore layer row scroll
	if (m_rowscroll_enable)
	{
		std::copy_n(&m_mainram[index], 0x800/4, &m_tilemap_ram[0x2800/4]);
		index += 0x800/4;
	}

	// middle layer
	for (int i = 0; i < 0x800/4; i++)
	{
		const u32 tile = m_mainram[index];
		if (m_tilemap_ram[i+m_midl_layer_offset] != tile)
		{
			m_tilemap_ram[i+m_midl_layer_offset] = tile;
			m_midl_layer->mark_tile_dirty((i * 2));
			m_midl_layer->mark_tile_dirty((i * 2) + 1);
		}
		index++;
	}

	// middle layer row scroll
	if (m_rowscroll_enable)
	{
		std::copy_n(&m_mainram[index], 0x800/4, &m_tilemap_ram[0x1800/4]);
		index += 0x800/4;
	}

	// text layer
	for (int i = 0; i < 0x1000/4; i++)
	{
		const u32 tile = m_mainram[index];
		if (m_tilemap_ram[i+m_text_layer_offset] != tile)
		{
			m_tilemap_ram[i+m_text_layer_offset] = tile;
			m_text_layer->mark_tile_dirty((i * 2));
			m_text_layer->mark_tile_dirty((i * 2) + 1);
		}
		index++;
	}
}


void seibuspi_state::palette_dma_start_w(u32 data)
{
	const int dma_length = (m_video_dma_length + 1) * 2;

	// safety check
	if (!DWORD_ALIGNED(m_video_dma_address) || (m_video_dma_length & 3) != 3 || dma_length > m_palette_ram_size || (m_video_dma_address + dma_length) > 0x40000)
		popmessage("Pal DMA %X %X, contact MAMEdev", m_video_dma_address, m_video_dma_length); // shouldn't happen
	if (m_video_dma_address < 0x800)
		logerror("palette_dma_start_w in I/O area: %X\n", m_video_dma_address);

	for (int i = 0; i < dma_length / 4; i++)
	{
		const u32 color = m_mainram[m_video_dma_address / 4 + i];
		if (m_palette_ram[i] != color)
		{
			m_palette_ram[i] = color;
			m_palette->set_pen_color((i * 2), pal5bit(m_palette_ram[i] >> 0), pal5bit(m_palette_ram[i] >> 5), pal5bit(m_palette_ram[i] >> 10));
			m_palette->set_pen_color((i * 2) + 1, pal5bit(m_palette_ram[i] >> 16), pal5bit(m_palette_ram[i] >> 21), pal5bit(m_palette_ram[i] >> 26));
		}
	}
}


void seibuspi_state::sprite_dma_start_w(u16 data)
{
	// safety check
	if (!DWORD_ALIGNED(m_video_dma_address) || (m_video_dma_address + m_sprite_ram_size) > 0x40000)
		popmessage("Sprite DMA %X, contact MAMEdev", m_video_dma_address); // shouldn't happen
	if (m_video_dma_address < 0x800)
		logerror("sprite_dma_start_w in I/O area: %X\n", m_video_dma_address);

	for (int i = 0; i < m_sprite_ram_size / 4; i++)
	{
		m_sprite_ram[i << 1] = m_mainram[(m_video_dma_address / 4) + i] & 0xffff;
		m_sprite_ram[(i << 1) | 1] = (m_mainram[(m_video_dma_address / 4) + i] >> 16) & 0xffff;
	}
}


/*****************************************************************************/

u32 seibuspi_state::gfxbank_callback(u32 code, u8 ext)
{
	return code | (((m_spritegen->gfx(0)->elements() > 0x10000) && ext) ? 0x10000 : 0);
}

void seibuspi_state::blend_sprite(bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri)
{
	if (BIT(m_layer_enable, 4))
		return;

	bitmap_ind16 &sprite_bitmap = m_spritegen->get_sprite_temp_bitmap();
	pri <<= 14;
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const u16 *src = &sprite_bitmap.pix(y, cliprect.min_x);
		u32 *dest = &bitmap.pix(y, cliprect.min_x);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			u16 pen = *src++;
			if (((pen & 0xc000) == pri) && (pen != 0xffff))
			{
				pen &= ~0xc000;

				if (m_alpha_table[pen])
					*dest = alpha_blend_r32(*dest, m_palette->pen(pen), 0x7f);
				else
					*dest = m_palette->pen(pen);
			}
			dest++;
		}
	}
}

void seibuspi_state::combine_tilemap(bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *tile, int sx, int sy, int opaque, s16 *rowscroll)
{
	bitmap_ind16 &pen_bitmap = tile->pixmap();
	bitmap_ind8 &flags_bitmap = tile->flagsmap();
	const u32 xscroll_mask = pen_bitmap.width() - 1;
	const u32 yscroll_mask = pen_bitmap.height() - 1;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int rx = sx;
		if (rowscroll)
			rx += rowscroll[(y + 19) & yscroll_mask]; // adder value probably not hardcoded but came from CRTC

		u32 *dest = &bitmap.pix(y, cliprect.min_x);
		const u16 *src = &pen_bitmap.pix((y + sy) & yscroll_mask);
		const u8 *flags = &flags_bitmap.pix((y + sy) & yscroll_mask);
		for (int x = cliprect.min_x + rx; x <= cliprect.max_x + rx; x++)
		{
			if (opaque || (flags[x & xscroll_mask] & (TILEMAP_PIXEL_LAYER0 | TILEMAP_PIXEL_LAYER1)))
			{
				const u16 pen = src[x & xscroll_mask];
				if (m_alpha_table[pen])
					*dest = alpha_blend_r32(*dest, m_palette->pen(pen), 0x7f);
				else
					*dest = m_palette->pen(pen);
			}
			dest++;
		}
	}
}


u32 seibuspi_state::screen_update_spi(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	s16 *back_rowscroll, *midl_rowscroll, *fore_rowscroll;
	if (m_rowscroll_enable)
	{
		back_rowscroll = (s16*)&m_tilemap_ram[0x200];
		midl_rowscroll = (s16*)&m_tilemap_ram[0x600];
		fore_rowscroll = (s16*)&m_tilemap_ram[0xa00];
	}
	else
	{
		back_rowscroll = nullptr;
		midl_rowscroll = nullptr;
		fore_rowscroll = nullptr;
	}

	if (BIT(~m_layer_enable, 4))
		m_spritegen->draw_sprites(screen, bitmap, cliprect, m_sprite_ram.get(), m_sprite_ram_size);

	if (BIT(m_layer_enable, 0))
		bitmap.fill(0, cliprect);
	else
		combine_tilemap(bitmap, cliprect, m_back_layer, m_scrollram[0], m_scrollram[1], 1, back_rowscroll);

	blend_sprite(bitmap, cliprect, 0);

	// if fore layer is enabled, draw priority 0 sprites behind back layer
	if ((m_layer_enable & 0x15) == 0)
		combine_tilemap(bitmap, cliprect, m_back_layer, m_scrollram[0], m_scrollram[1], 0, back_rowscroll);

	// if fore layer is enabled, draw priority 1 sprites behind middle layer
	if (BIT(~m_layer_enable, 2))
		blend_sprite(bitmap, cliprect, 1);

	if (BIT(~m_layer_enable, 1))
		combine_tilemap(bitmap, cliprect, m_midl_layer, m_scrollram[2], m_scrollram[3], 0, midl_rowscroll);

	// if fore layer is disabled, draw priority 1 sprites above middle layer
	if (BIT(m_layer_enable, 2))
		blend_sprite(bitmap, cliprect, 1);

	blend_sprite(bitmap, cliprect, 2);

	if (BIT(~m_layer_enable, 2))
		combine_tilemap(bitmap, cliprect, m_fore_layer, m_scrollram[4], m_scrollram[5], 0, fore_rowscroll);

	blend_sprite(bitmap, cliprect, 3);

	if (BIT(~m_layer_enable, 3))
		combine_tilemap(bitmap, cliprect, m_text_layer, 0, 0, 0, nullptr);

	return 0;
}

u32 seibuspi_state::screen_update_sys386f(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_sprite_ram.get(), m_sprite_ram_size);

	return 0;
}


/*****************************************************************************/

TILE_GET_INFO_MEMBER(seibuspi_state::get_text_tile_info)
{
	const int offs = tile_index / 2;
	u32 tile = (m_tilemap_ram[offs + m_text_layer_offset] >> ((tile_index & 0x1) ? 16 : 0)) & 0xffff;
	const u32 color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	tileinfo.set(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(seibuspi_state::get_back_tile_info)
{
	const int offs = tile_index / 2;
	u32 tile = (m_tilemap_ram[offs] >> ((tile_index & 0x1) ? 16 : 0)) & 0xffff;
	const u32 color = (tile >> 13) & 0x7;

	tile &= 0x1fff;
	tile |= m_back_layer_d14;

	tileinfo.set(1, tile, color, 0);
}

TILE_GET_INFO_MEMBER(seibuspi_state::get_midl_tile_info)
{
	const int offs = tile_index / 2;
	u32 tile = (m_tilemap_ram[offs + m_midl_layer_offset] >> ((tile_index & 0x1) ? 16 : 0)) & 0xffff;
	const u32 color = (tile >> 13) & 0x7;

	tile &= 0x1fff;
	tile |= 0x2000;
	tile |= m_midl_layer_d14;

	tileinfo.set(1, tile, color + 16, 0);
}

TILE_GET_INFO_MEMBER(seibuspi_state::get_fore_tile_info)
{
	const int offs = tile_index / 2;
	u32 tile = (m_tilemap_ram[offs + m_fore_layer_offset] >> ((tile_index & 0x1) ? 16 : 0)) & 0xffff;
	const u32 color = (tile >> 13) & 0x7;

	tile &= 0x1fff;
	tile |= m_bg_fore_layer_position;
	tile |= m_fore_layer_d13;
	tile |= m_fore_layer_d14;

	tileinfo.set(1, tile, color + 8, 0);
}


void seibuspi_state::video_start()
{
	m_spritegen->alloc_sprite_bitmap();
	m_video_dma_length = 0;
	m_video_dma_address = 0;
	m_layer_enable = 0;
	m_layer_bank = 0;
	m_rf2_layer_bank = 0;
	m_rowscroll_enable = false;
	m_scrollram[0] = 0;
	m_scrollram[1] = 0;
	m_scrollram[2] = 0;
	m_scrollram[3] = 0;
	m_scrollram[4] = 0;
	m_scrollram[5] = 0;
	set_layer_offsets();

	const u32 region_length = memregion("tiles")->bytes();

	if (region_length <= 0x300000)
		m_bg_fore_layer_position = 0x2000;
	else if (region_length <= 0x600000)
		m_bg_fore_layer_position = 0x4000;
	else
		m_bg_fore_layer_position = 0x8000;

	m_tilemap_ram_size = 0x4000;
	m_palette_ram_size = 0x3000;
	m_sprite_ram_size = 0x1000;
	m_sprite_bpp = 6;

	m_tilemap_ram = make_unique_clear<u32[]>(m_tilemap_ram_size/4);
	m_palette_ram = make_unique_clear<u32[]>(m_palette_ram_size/4);
	m_sprite_ram = make_unique_clear<u16[]>(m_sprite_ram_size/2);

	m_palette->basemem().set(&m_palette_ram[0], m_palette_ram_size, 32, ENDIANNESS_LITTLE, 2);

	m_text_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(seibuspi_state::get_text_tile_info)), TILEMAP_SCAN_ROWS,  8, 8, 64,32);
	m_back_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(seibuspi_state::get_back_tile_info)), TILEMAP_SCAN_COLS, 16,16, 32,32);
	m_midl_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(seibuspi_state::get_midl_tile_info)), TILEMAP_SCAN_COLS, 16,16, 32,32);
	m_fore_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(seibuspi_state::get_fore_tile_info)), TILEMAP_SCAN_COLS, 16,16, 32,32);

	m_text_layer->set_transparent_pen(31);
	m_back_layer->set_transparent_pen(63);
	m_midl_layer->set_transparent_pen(63);
	m_fore_layer->set_transparent_pen(63);

	// TODO : Differs per games?
	// alpha blending (preliminary)
	memset(m_alpha_table, 0, 0x2000);

	// sprites(0000-0fff):
	//memset(m_alpha_table + 0x700, 1, 0x10); // breaks rdft
	memset(m_alpha_table + 0x730, 1, 0x10);
	memset(m_alpha_table + 0x780, 1, 0x20);
	//memset(m_alpha_table + 0x7c0, 1, 0x40); // breaks batlball
	//memset(m_alpha_table + 0xf00, 1, 0x40); // breaks rdft
	memset(m_alpha_table + 0xfc0, 1, 0x40);

	// back layer(1000-11ff): nope
	// fore layer(1200-13ff):
	memset(m_alpha_table + 0x1200 + 0x160, 1, 0x20);
	memset(m_alpha_table + 0x1200 + 0x1b0, 1, 0x10);
	memset(m_alpha_table + 0x1200 + 0x1f0, 1, 0x10);
	// midl layer(1400-15ff)
	memset(m_alpha_table + 0x1400 + 0x1b0, 1, 0x10);
	memset(m_alpha_table + 0x1400 + 0x1f0, 1, 0x10);
	// text layer(1600-17ff)
	memset(m_alpha_table + 0x1600 + 0x170, 1, 0x10);
	memset(m_alpha_table + 0x1600 + 0x1f0, 1, 0x10);

	register_video_state();
}

VIDEO_START_MEMBER(seibuspi_state,ejanhs)
{
	video_start();

	memset(m_alpha_table, 0, 0x2000); // no alpha blending
}

VIDEO_START_MEMBER(seibuspi_state,sys386f)
{
	m_video_dma_length = 0;
	m_video_dma_address = 0;
	m_layer_enable = 0;
	m_layer_bank = 0;
	m_rf2_layer_bank = 0;
	m_rowscroll_enable = false;
	set_layer_offsets();

	m_tilemap_ram_size = 0;
	m_palette_ram_size = 0x4000;
	m_sprite_ram_size = 0x2000;
	m_sprite_bpp = 8;

	m_tilemap_ram = nullptr;
	m_palette_ram = make_unique_clear<u32[]>(m_palette_ram_size/4);
	m_sprite_ram = make_unique_clear<u16[]>(m_sprite_ram_size/2);

	m_palette->basemem().set(&m_palette_ram[0], m_palette_ram_size, 32, ENDIANNESS_LITTLE, 2);

	memset(m_alpha_table, 0, 0x2000); // no alpha blending

	register_video_state();
}

void seibuspi_state::register_video_state()
{
	m_spritegen->gfx(0)->set_granularity(1 << m_sprite_bpp);
	save_item(NAME(m_video_dma_length));
	save_item(NAME(m_video_dma_address));
	save_item(NAME(m_layer_enable));
	save_item(NAME(m_layer_bank));
	save_item(NAME(m_rf2_layer_bank));
	save_item(NAME(m_rowscroll_enable));
	save_item(NAME(m_scrollram));

	save_item(NAME(m_midl_layer_offset));
	save_item(NAME(m_fore_layer_offset));
	save_item(NAME(m_text_layer_offset));
	save_item(NAME(m_fore_layer_d13));
	save_item(NAME(m_back_layer_d14));
	save_item(NAME(m_midl_layer_d14));
	save_item(NAME(m_fore_layer_d14));

	if (m_tilemap_ram != nullptr) save_pointer(NAME(m_tilemap_ram), m_tilemap_ram_size/4);
	save_pointer(NAME(m_palette_ram), m_palette_ram_size/4);
	save_pointer(NAME(m_sprite_ram), m_sprite_ram_size/2);
}
