/******************************************************************************

    Seibu SPI hardware

    Functions to emulate the video hardware

******************************************************************************/

#include "emu.h"
#include "includes/seibuspi.h"


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
	
	m_fore_layer_d13 = m_layer_bank >> 14 & 0x2000;
	m_back_layer_d14 = m_rf2_layer_bank << 14 & 0x4000;
	m_midl_layer_d14 = m_rf2_layer_bank << 13 & 0x4000;
	m_fore_layer_d14 = m_rf2_layer_bank << 12 & 0x4000;
}

READ32_MEMBER(seibuspi_state::spi_layer_bank_r)
{
	return m_layer_bank;
}

WRITE32_MEMBER(seibuspi_state::spi_layer_bank_w)
{
	// r000f000 0010100a 00000000 00000000
	// r: rowscroll enable
	// f: fore layer d13
	// a: global alpha blending enable (0 in ejanhs, 1 in all other games)
	UINT32 prev = m_layer_bank;
	COMBINE_DATA(&m_layer_bank);

	m_rowscroll_enable = m_layer_bank >> 31 & 1;
	m_alpha_enable = m_layer_bank >> 16 & 1;
	set_layer_offsets();

	if ((prev ^ m_layer_bank) & 0x08000000)
		m_fore_layer->mark_all_dirty();
}


WRITE8_MEMBER(seibuspi_state::rf2_layer_bank_w)
{
	// 00000fmb
	// f: fore layer d14
	// m: middle layer d14
	// b: back layer d14
	UINT8 prev = m_rf2_layer_bank;
	m_rf2_layer_bank = data;
	set_layer_offsets();

	if ((prev ^ m_rf2_layer_bank) & 1)
		m_back_layer->mark_all_dirty();

	if ((prev ^ m_rf2_layer_bank) & 2)
		m_midl_layer->mark_all_dirty();

	if ((prev ^ m_rf2_layer_bank) & 4)
		m_fore_layer->mark_all_dirty();
}

WRITE32_MEMBER(seibuspi_state::spi_layer_enable_w)
{
	// 00000000 00000000 00000000 000stfmb (0=on, 1=off)
	// s: sprite layer
	// t: text layer
	// f: fore layer
	// m: middle layer
	// b: back layer
	COMBINE_DATA(&m_layer_enable);
}

WRITE32_MEMBER(seibuspi_state::video_dma_length_w)
{
	COMBINE_DATA(&m_video_dma_length);
}

WRITE32_MEMBER(seibuspi_state::video_dma_address_w)
{
	COMBINE_DATA(&m_video_dma_address);
}


/*****************************************************************************/

WRITE32_MEMBER(seibuspi_state::tilemap_dma_start_w)
{
	if (!m_tilemap_ram)
		return;

	// safety check
	int dma_length_user = m_rowscroll_enable ? 0x4000 : 0x2800;
	int dma_length_real = (m_video_dma_length + 1) * 2; // ideally we should be using this, let's check if we have to:
	if (m_video_dma_length != 0 && dma_length_user != dma_length_real)
		popmessage("Tile LEN %X %X, contact MAMEdev", dma_length_user, dma_length_real); // shouldn't happen
	else if ((m_video_dma_address & 3) != 0 || (m_video_dma_length & 3) != 3 || (m_video_dma_address + dma_length_user) > 0x40000)
		popmessage("Tile DMA %X %X, contact MAMEdev", m_video_dma_address, m_video_dma_length); // shouldn't happen
	if (m_video_dma_address < 0x800)
		logerror("tilemap_dma_start_w in I/O area: %X\n", m_video_dma_address);

	int index = m_video_dma_address / 4;

	/* back layer */
	for (int i = 0; i < 0x800/4; i++)
	{
		UINT32 tile = m_mainram[index];
		if (m_tilemap_ram[i] != tile)
		{
			m_tilemap_ram[i] = tile;
			m_back_layer->mark_tile_dirty((i * 2));
			m_back_layer->mark_tile_dirty((i * 2) + 1);
		}
		index++;
	}

	/* back layer row scroll */
	if (m_rowscroll_enable)
	{
		memcpy(&m_tilemap_ram[0x800/4], &m_mainram[index], 0x800/4);
		index += 0x800/4;
	}

	/* fore layer */
	for (int i = 0; i < 0x800/4; i++)
	{
		UINT32 tile = m_mainram[index];
		if (m_tilemap_ram[i+m_fore_layer_offset] != tile)
		{
			m_tilemap_ram[i+m_fore_layer_offset] = tile;
			m_fore_layer->mark_tile_dirty((i * 2));
			m_fore_layer->mark_tile_dirty((i * 2) + 1);
		}
		index++;
	}

	/* fore layer row scroll */
	if (m_rowscroll_enable)
	{
		memcpy(&m_tilemap_ram[0x1800/4], &m_mainram[index], 0x800/4);
		index += 0x800/4;
	}

	/* middle layer */
	for (int i = 0; i < 0x800/4; i++)
	{
		UINT32 tile = m_mainram[index];
		if (m_tilemap_ram[i+m_midl_layer_offset] != tile)
		{
			m_tilemap_ram[i+m_midl_layer_offset] = tile;
			m_midl_layer->mark_tile_dirty((i * 2));
			m_midl_layer->mark_tile_dirty((i * 2) + 1);
		}
		index++;
	}

	/* middle layer row scroll */
	if (m_rowscroll_enable)
	{
		memcpy(&m_tilemap_ram[0x1800/4], &m_mainram[index], 0x800/4);
		index += 0x800/4;
	}

	/* text layer */
	for (int i = 0; i < 0x1000/4; i++)
	{
		UINT32 tile = m_mainram[index];
		if (m_tilemap_ram[i+m_text_layer_offset] != tile)
		{
			m_tilemap_ram[i+m_text_layer_offset] = tile;
			m_text_layer->mark_tile_dirty((i * 2));
			m_text_layer->mark_tile_dirty((i * 2) + 1);
		}
		index++;
	}
}


WRITE32_MEMBER(seibuspi_state::palette_dma_start_w)
{
	int dma_length = (m_video_dma_length + 1) * 2;

	// safety check
	if ((m_video_dma_address & 3) != 0 || (m_video_dma_length & 3) != 3 || dma_length > m_palette_ram_size || (m_video_dma_address + dma_length) > 0x40000)
		popmessage("Pal DMA %X %X, contact MAMEdev", m_video_dma_address, m_video_dma_length); // shouldn't happen
	if (m_video_dma_address < 0x800)
		logerror("palette_dma_start_w in I/O area: %X\n", m_video_dma_address);

	for (int i = 0; i < dma_length / 4; i++)
	{
		UINT32 color = m_mainram[m_video_dma_address / 4 + i];
		if (m_palette_ram[i] != color)
		{
			m_palette_ram[i] = color;
			m_palette->set_pen_color((i * 2), pal5bit(m_palette_ram[i] >> 0), pal5bit(m_palette_ram[i] >> 5), pal5bit(m_palette_ram[i] >> 10));
			m_palette->set_pen_color((i * 2) + 1, pal5bit(m_palette_ram[i] >> 16), pal5bit(m_palette_ram[i] >> 21), pal5bit(m_palette_ram[i] >> 26));
		}
	}
}


WRITE16_MEMBER(seibuspi_state::sprite_dma_start_w)
{
	// safety check
	if ((m_video_dma_address & 3) != 0 || (m_video_dma_address + m_sprite_ram_size) > 0x40000)
		popmessage("Sprite DMA %X, contact MAMEdev", m_video_dma_address); // shouldn't happen
	if (m_video_dma_address < 0x800)
		logerror("sprite_dma_start_w in I/O area: %X\n", m_video_dma_address);

	memcpy(m_sprite_ram, &m_mainram[m_video_dma_address / 4], m_sprite_ram_size);
}


/*****************************************************************************/

void seibuspi_state::drawgfx_blend(bitmap_rgb32 &bitmap, const rectangle &cliprect, gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, int sx, int sy)
{
	const pen_t *pens = &m_palette->pen(gfx->colorbase());
	const UINT8 *dp;
	int i, j;
	int x1, x2;
	int y1, y2;
	int px, py;
	int xd = 1, yd = 1;

	int width = gfx->width();
	int height = gfx->height();

	x1 = sx;
	x2 = sx + width - 1;
	y1 = sy;
	y2 = sy + height - 1;

	if (x1 > cliprect.max_x || x2 < cliprect.min_x)
	{
		return;
	}
	if (y1 > cliprect.max_y || y2 < cliprect.min_y)
	{
		return;
	}

	px = 0;
	py = 0;

	if (flipx)
	{
		xd = -xd;
		px = width - 1;
	}
	if (flipy)
	{
		yd = -yd;
		py = height - 1;
	}

	// clip x
	if (x1 < cliprect.min_x)
	{
		if (flipx)
		{
			px = width - (cliprect.min_x - x1) - 1;
		}
		else
		{
			px = (cliprect.min_x - x1);
		}
		x1 = cliprect.min_x;
	}
	if (x2 > cliprect.max_x)
	{
		x2 = cliprect.max_x;
	}

	// clip y
	if (y1 < cliprect.min_y)
	{
		if (flipy)
		{
			py = height - (cliprect.min_y - y1) - 1;
		}
		else
		{
			py = (cliprect.min_y - y1);
		}
		y1 = cliprect.min_y;
	}
	if (y2 > cliprect.max_y)
	{
		y2 = cliprect.max_y;
	}

	dp = gfx->get_data(code);

	// draw
	for (j = y1; j <= y2; j++)
	{
		UINT32 *p = &bitmap.pix32(j);
		UINT8 trans_pen = (1 << m_sprite_bpp) - 1;
		int dp_i = (py * width) + px;
		py += yd;

		for (i = x1; i <= x2; i++)
		{
			UINT8 pen = dp[dp_i];
			if (pen != trans_pen)
			{
				int global_pen = pen + (color << m_sprite_bpp);
				if (m_alpha_enable && m_alpha_table[global_pen])
				{
					p[i] = alpha_blend_r32(p[i], pens[global_pen], 0x7f);
				}
				else
				{
					p[i] = pens[global_pen];
				}
			}
			dp_i += xd;
		}
	}
}

void seibuspi_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri_mask)
{
	INT16 xpos, ypos;
	int tile_num, color;
	int width, height;
	int flip_x, flip_y;
	int priority;
	int x1, y1;
	gfx_element *gfx = m_gfxdecode->gfx(2);
	const int has_tile_high = (gfx->elements() > 0x10000) ? 1 : 0;
	const int colormask = (m_sprite_bpp == 6) ? 0x3f : 0x1f;

	static const int sprite_xtable[2][8] =
	{
		{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
		{ 7*16, 6*16, 5*16, 4*16, 3*16, 2*16, 1*16, 0*16 }
	};
	static const int sprite_ytable[2][8] =
	{
		{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
		{ 7*16, 6*16, 5*16, 4*16, 3*16, 2*16, 1*16, 0*16 }
	};

	if (m_layer_enable & 0x10)
		return;

	for (int a = m_sprite_ram_size / 4 - 2; a >= 0; a -= 2)
	{
		/*
		    Word 0
		    xxxxxxxx xxxxxxxx -------- --------  tile_num low
		    -------- -------- x------- --------  flip_y
		    -------- -------- -xxx---- --------  height
		    -------- -------- ----x--- --------  flip_x
		    -------- -------- -----xxx --------  width
		    -------- -------- -------- xx------  priority
		    -------- -------- -------- --xxxxxx  color (highest bit not used on SYS386F)

		    Word 1, unmarked bits have no function
		    -------x xxxxxxxx -------- --------  ypos
		    -------- -------- ---x---- --------  tile_num high (only on RISE10/11 chip)
		    -------- -------- ------xx xxxxxxxx  xpos
		*/
		tile_num = m_sprite_ram[a + 0] >> 16 & 0xffff;
		if (tile_num == 0)
			continue;

		if (has_tile_high)
			tile_num |= m_sprite_ram[a + 1] << 4 & 0x10000;

		priority = m_sprite_ram[a + 0] >> 6 & 0x3;
		if (pri_mask != priority)
			continue;

		xpos = m_sprite_ram[a + 1] & 0x3ff;
		if (xpos & 0x200)
			xpos |= 0xfc00;
		ypos = m_sprite_ram[a + 1] >> 16 & 0x1ff;
		if (ypos & 0x100)
			ypos |= 0xfe00;
		color = m_sprite_ram[a + 0] & colormask;

		width = (m_sprite_ram[a + 0] >> 8 & 0x7) + 1;
		height = (m_sprite_ram[a + 0] >> 12 & 0x7) + 1;
		flip_x = m_sprite_ram[a + 0] >> 11 & 0x1;
		flip_y = m_sprite_ram[a + 0] >> 15 & 0x1;
		x1 = 0;
		y1 = 0;

		if (flip_x)
		{
			x1 = 8 - width;
			width = width + x1;
		}
		if (flip_y)
		{
			y1 = 8 - height;
			height = height + y1;
		}

		for (int x = x1; x < width; x++)
		{
			for (int y = y1; y < height; y++)
			{
				drawgfx_blend(bitmap, cliprect, gfx, tile_num, color, flip_x, flip_y, xpos + sprite_xtable[flip_x][x], ypos + sprite_ytable[flip_y][y]);

				/* xpos seems to wrap-around to 0 at 512 */
				if ((xpos + (16 * x) + 16) >= 512)
				{
					drawgfx_blend(bitmap, cliprect, gfx, tile_num, color, flip_x, flip_y, xpos - 512 + sprite_xtable[flip_x][x], ypos + sprite_ytable[flip_y][y]);
				}

				tile_num++;
			}
		}
	}
}

void seibuspi_state::combine_tilemap(bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *tile, int x, int y, int opaque, INT16 *rowscroll)
{
	UINT16 *s;
	UINT32 *d;
	UINT8 *t;
	UINT32 xscroll_mask, yscroll_mask;

	bitmap_ind16 &pen_bitmap = tile->pixmap();
	bitmap_ind8 &flags_bitmap = tile->flagsmap();
	xscroll_mask = pen_bitmap.width() - 1;
	yscroll_mask = pen_bitmap.height() - 1;

	for (int j = cliprect.min_y; j <= cliprect.max_y; j++)
	{
		int rx = x;
		if (rowscroll)
		{
			rx += rowscroll[(j+y) & yscroll_mask];
		}

		d = &bitmap.pix32(j);
		s = &pen_bitmap.pix16((j+y) & yscroll_mask);
		t = &flags_bitmap.pix8((j+y) & yscroll_mask);
		for (int i = cliprect.min_x+rx; i <= cliprect.max_x+rx; i++)
		{
			if (opaque || (t[i & xscroll_mask] & (TILEMAP_PIXEL_LAYER0 | TILEMAP_PIXEL_LAYER1)))
			{
				UINT16 pen = s[i & xscroll_mask];
				if (m_alpha_enable && m_alpha_table[pen])
				{
					*d = alpha_blend_r32(*d, m_palette->pen(pen), 0x7f);
				}
				else
				{
					*d = m_palette->pen(pen);
				}
			}
			++d;
		}
	}
}


UINT32 seibuspi_state::screen_update_spi(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	INT16 *back_rowscroll, *midl_rowscroll, *fore_rowscroll;
	if (m_rowscroll_enable)
	{
		back_rowscroll = (INT16*)&m_tilemap_ram[0x200];
		midl_rowscroll = (INT16*)&m_tilemap_ram[0x600];
		fore_rowscroll = (INT16*)&m_tilemap_ram[0xa00];
	}
	else
	{
		back_rowscroll = NULL;
		midl_rowscroll = NULL;
		fore_rowscroll = NULL;
	}

	if (m_layer_enable & 1)
		bitmap.fill(0, cliprect);

	if (~m_layer_enable & 1)
		combine_tilemap(bitmap, cliprect, m_back_layer, m_scrollram[0] & 0xffff, (m_scrollram[0] >> 16) & 0xffff, 1, back_rowscroll);

	draw_sprites(bitmap, cliprect, 0);

	// if fore layer is enabled, draw priority 0 sprites behind back layer
	if ((m_layer_enable & 0x15) == 0)
		combine_tilemap(bitmap, cliprect, m_back_layer, m_scrollram[0] & 0xffff, (m_scrollram[0] >> 16) & 0xffff, 0, back_rowscroll);

	// if fore layer is enabled, draw priority 1 sprites behind middle layer
	if (~m_layer_enable & 4)
		draw_sprites(bitmap, cliprect, 1);

	if (~m_layer_enable & 2)
		combine_tilemap(bitmap, cliprect, m_midl_layer, m_scrollram[1] & 0xffff, (m_scrollram[1] >> 16) & 0xffff, 0, midl_rowscroll);

	// if fore layer is disabled, draw priority 1 sprites above middle layer
	if (m_layer_enable & 4)
		draw_sprites(bitmap, cliprect, 1);

	draw_sprites(bitmap, cliprect, 2);

	if (~m_layer_enable & 4)
		combine_tilemap(bitmap, cliprect, m_fore_layer, m_scrollram[2] & 0xffff, (m_scrollram[2] >> 16) & 0xffff, 0, fore_rowscroll);

	draw_sprites(bitmap, cliprect, 3);

	if (~m_layer_enable & 8)
		combine_tilemap(bitmap, cliprect, m_text_layer, 0, 0, 0, NULL);

	return 0;
}

UINT32 seibuspi_state::screen_update_sys386f(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	draw_sprites(bitmap, cliprect, 0);
	draw_sprites(bitmap, cliprect, 1);
	draw_sprites(bitmap, cliprect, 2);
	draw_sprites(bitmap, cliprect, 3);

	return 0;
}


/*****************************************************************************/

TILE_GET_INFO_MEMBER(seibuspi_state::get_text_tile_info)
{
	int offs = tile_index / 2;
	int tile = (m_tilemap_ram[offs + m_text_layer_offset] >> ((tile_index & 0x1) ? 16 : 0)) & 0xffff;
	int color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	SET_TILE_INFO_MEMBER(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(seibuspi_state::get_back_tile_info)
{
	int offs = tile_index / 2;
	int tile = (m_tilemap_ram[offs] >> ((tile_index & 0x1) ? 16 : 0)) & 0xffff;
	int color = (tile >> 13) & 0x7;

	tile &= 0x1fff;
	tile |= m_back_layer_d14;

	SET_TILE_INFO_MEMBER(1, tile, color, 0);
}

TILE_GET_INFO_MEMBER(seibuspi_state::get_midl_tile_info)
{
	int offs = tile_index / 2;
	int tile = (m_tilemap_ram[offs + m_midl_layer_offset] >> ((tile_index & 0x1) ? 16 : 0)) & 0xffff;
	int color = (tile >> 13) & 0x7;

	tile &= 0x1fff;
	tile |= 0x2000;
	tile |= m_midl_layer_d14;

	SET_TILE_INFO_MEMBER(1, tile, color + 16, 0);
}

TILE_GET_INFO_MEMBER(seibuspi_state::get_fore_tile_info)
{
	int offs = tile_index / 2;
	int tile = (m_tilemap_ram[offs + m_fore_layer_offset] >> ((tile_index & 0x1) ? 16 : 0)) & 0xffff;
	int color = (tile >> 13) & 0x7;

	tile &= 0x1fff;
	tile |= m_bg_fore_layer_position;
	tile |= m_fore_layer_d13;
	tile |= m_fore_layer_d14;

	SET_TILE_INFO_MEMBER(1, tile, color + 8, 0);
}


void seibuspi_state::video_start()
{
	m_video_dma_length = 0;
	m_video_dma_address = 0;
	m_layer_enable = 0;
	m_layer_bank = 0;
	m_rf2_layer_bank = 0;
	m_alpha_enable = 0;
	m_rowscroll_enable = 0;
	set_layer_offsets();

	UINT32 region_length = memregion("gfx2")->bytes();

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

	m_tilemap_ram = auto_alloc_array_clear(machine(), UINT32, m_tilemap_ram_size/4);
	m_palette_ram = auto_alloc_array_clear(machine(), UINT32, m_palette_ram_size/4);
	m_sprite_ram = auto_alloc_array_clear(machine(), UINT32, m_sprite_ram_size/4);

	m_text_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(seibuspi_state::get_text_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64,32);
	m_back_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(seibuspi_state::get_back_tile_info),this), TILEMAP_SCAN_COLS, 16,16,32,32);
	m_midl_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(seibuspi_state::get_midl_tile_info),this), TILEMAP_SCAN_COLS, 16,16,32,32);
	m_fore_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(seibuspi_state::get_fore_tile_info),this), TILEMAP_SCAN_COLS, 16,16,32,32);

	m_text_layer->set_transparent_pen(31);
	m_back_layer->set_transparent_pen(63);
	m_midl_layer->set_transparent_pen(63);
	m_fore_layer->set_transparent_pen(63);

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

VIDEO_START_MEMBER(seibuspi_state,sys386f)
{
	m_video_dma_length = 0;
	m_video_dma_address = 0;
	m_layer_enable = 0;
	m_layer_bank = 0;
	m_rf2_layer_bank = 0;
	m_alpha_enable = 0;
	m_rowscroll_enable = 0;
	set_layer_offsets();
	
	m_tilemap_ram_size = 0;
	m_palette_ram_size = 0x4000;
	m_sprite_ram_size = 0x2000;
	m_sprite_bpp = 8;

	m_tilemap_ram = NULL;
	m_palette_ram = auto_alloc_array_clear(machine(), UINT32, m_palette_ram_size/4);
	m_sprite_ram = auto_alloc_array_clear(machine(), UINT32, m_sprite_ram_size/4);

	memset(m_alpha_table, 0, 0x2000);

	register_video_state();
}

void seibuspi_state::register_video_state()
{
	save_item(NAME(m_video_dma_length));
	save_item(NAME(m_video_dma_address));
	save_item(NAME(m_layer_enable));
	save_item(NAME(m_layer_bank));
	save_item(NAME(m_rf2_layer_bank));
	save_item(NAME(m_alpha_enable));
	save_item(NAME(m_rowscroll_enable));

	save_item(NAME(m_midl_layer_offset));
	save_item(NAME(m_fore_layer_offset));
	save_item(NAME(m_text_layer_offset));
	save_item(NAME(m_fore_layer_d13));
	save_item(NAME(m_back_layer_d14));
	save_item(NAME(m_midl_layer_d14));
	save_item(NAME(m_fore_layer_d14));

	if (m_tilemap_ram != NULL) save_pointer(NAME(m_tilemap_ram), m_tilemap_ram_size/4);
	save_pointer(NAME(m_palette_ram), m_palette_ram_size/4);
	save_pointer(NAME(m_sprite_ram), m_sprite_ram_size/4);
}
