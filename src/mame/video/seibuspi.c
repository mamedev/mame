#include "emu.h"
#include "includes/seibuspi.h"


READ32_MEMBER(seibuspi_state::spi_layer_bank_r)
{
	return m_layer_bank;
}

WRITE32_MEMBER(seibuspi_state::spi_layer_bank_w)
{
	COMBINE_DATA( &m_layer_bank );

	if (m_layer_bank & 0x80000000) {
		m_fore_layer_offset = 0x1000 / 4;
		m_mid_layer_offset = 0x2000 / 4;
		m_text_layer_offset = 0x3000 / 4;
	}
	else {
		m_fore_layer_offset = 0x800 / 4;
		m_mid_layer_offset = 0x1000 / 4;
		m_text_layer_offset = 0x1800 / 4;
	}
}

void rf2_set_layer_banks(running_machine &machine, int banks)
{
	seibuspi_state *state = machine.driver_data<seibuspi_state>();
	if (state->m_rf2_layer_bank[0] != BIT(banks,0))
	{
		state->m_rf2_layer_bank[0] = BIT(banks,0);
		state->m_back_layer->mark_all_dirty();
	}

	if (state->m_rf2_layer_bank[1] != BIT(banks,1))
	{
		state->m_rf2_layer_bank[1] = BIT(banks,1);
		state->m_mid_layer->mark_all_dirty();
	}

	if (state->m_rf2_layer_bank[2] != BIT(banks,2))
	{
		state->m_rf2_layer_bank[2] = BIT(banks,2);
		state->m_fore_layer->mark_all_dirty();
	}
}

#ifdef UNUSED_FUNCTION
READ32_MEMBER(seibuspi_state::spi_layer_enable_r)
{
	return m_layer_enable;
}
#endif

WRITE32_MEMBER(seibuspi_state::spi_layer_enable_w)
{
	COMBINE_DATA( &m_layer_enable );
	m_back_layer->enable((m_layer_enable & 0x1) ^ 0x1);
	m_mid_layer->enable(((m_layer_enable >> 1) & 0x1) ^ 0x1);
	m_fore_layer->enable(((m_layer_enable >> 2) & 0x1) ^ 0x1);
}

WRITE32_MEMBER(seibuspi_state::tilemap_dma_start_w)
{
	if (m_video_dma_address != 0)
	{
		int i;
		int index = (m_video_dma_address / 4) - 0x200;

		if (m_layer_bank & 0x80000000)
		{
			/* back layer */
			for (i=0; i < 0x800/4; i++) {
				UINT32 tile = m_spimainram[index];
				if (m_tilemap_ram[i] != tile) {
					m_tilemap_ram[i] = tile;
					m_back_layer->mark_tile_dirty((i * 2) );
					m_back_layer->mark_tile_dirty((i * 2) + 1 );
				}
				index++;
			}

			/* back layer row scroll */
			memcpy(&m_tilemap_ram[0x800/4], &m_spimainram[index], 0x800/4);
			index += 0x800/4;

			/* fore layer */
			for (i=0; i < 0x800/4; i++) {
				UINT32 tile = m_spimainram[index];
				if (m_tilemap_ram[i+m_fore_layer_offset] != tile) {
					m_tilemap_ram[i+m_fore_layer_offset] = tile;
					m_fore_layer->mark_tile_dirty((i * 2) );
					m_fore_layer->mark_tile_dirty((i * 2) + 1 );
				}
				index++;
			}

			/* fore layer row scroll */
			memcpy(&m_tilemap_ram[0x1800/4], &m_spimainram[index], 0x800/4);
			index += 0x800/4;

			/* mid layer */
			for (i=0; i < 0x800/4; i++) {
				UINT32 tile = m_spimainram[index];
				if (m_tilemap_ram[i+m_mid_layer_offset] != tile) {
					m_tilemap_ram[i+m_mid_layer_offset] = tile;
					m_mid_layer->mark_tile_dirty((i * 2) );
					m_mid_layer->mark_tile_dirty((i * 2) + 1 );
				}
				index++;
			}

			/* mid layer row scroll */
			memcpy(&m_tilemap_ram[0x1800/4], &m_spimainram[index], 0x800/4);
			index += 0x800/4;

			/* text layer */
			for (i=0; i < 0x1000/4; i++) {
				UINT32 tile = m_spimainram[index];
				if (m_tilemap_ram[i+m_text_layer_offset] != tile) {
					m_tilemap_ram[i+m_text_layer_offset] = tile;
					m_text_layer->mark_tile_dirty((i * 2) );
					m_text_layer->mark_tile_dirty((i * 2) + 1 );
				}
				index++;
			}
		}
		else
		{
			/* back layer */
			for (i=0; i < 0x800/4; i++) {
				UINT32 tile = m_spimainram[index];
				if (m_tilemap_ram[i] != tile) {
					m_tilemap_ram[i] = tile;
					m_back_layer->mark_tile_dirty((i * 2) );
					m_back_layer->mark_tile_dirty((i * 2) + 1 );
				}
				index++;
			}

			/* fore layer */
			for (i=0; i < 0x800/4; i++) {
				UINT32 tile = m_spimainram[index];
				if (m_tilemap_ram[i+m_fore_layer_offset] != tile) {
					m_tilemap_ram[i+m_fore_layer_offset] = tile;
					m_fore_layer->mark_tile_dirty((i * 2) );
					m_fore_layer->mark_tile_dirty((i * 2) + 1 );
				}
				index++;
			}

			/* mid layer */
			for (i=0; i < 0x800/4; i++) {
				UINT32 tile = m_spimainram[index];
				if (m_tilemap_ram[i+m_mid_layer_offset] != tile) {
					m_tilemap_ram[i+m_mid_layer_offset] = tile;
					m_mid_layer->mark_tile_dirty((i * 2) );
					m_mid_layer->mark_tile_dirty((i * 2) + 1 );
				}
				index++;
			}

			/* text layer */
			for (i=0; i < 0x1000/4; i++) {
				UINT32 tile = m_spimainram[index];
				if (m_tilemap_ram[i+m_text_layer_offset] != tile) {
					m_tilemap_ram[i+m_text_layer_offset] = tile;
					m_text_layer->mark_tile_dirty((i * 2) );
					m_text_layer->mark_tile_dirty((i * 2) + 1 );
				}
				index++;
			}
		}
	}
}

WRITE32_MEMBER(seibuspi_state::palette_dma_start_w)
{
	if (m_video_dma_address != 0)
	{
		int i;
		for (i=0; i < ((m_video_dma_length+1) * 2) / 4; i++)
		{
			UINT32 color = m_spimainram[(m_video_dma_address / 4) + i - 0x200];
			if (m_palette_ram[i] != color) {
				m_palette_ram[i] = color;
				palette_set_color_rgb( machine(), (i * 2), pal5bit(m_palette_ram[i] >> 0), pal5bit(m_palette_ram[i] >> 5), pal5bit(m_palette_ram[i] >> 10) );
				palette_set_color_rgb( machine(), (i * 2) + 1, pal5bit(m_palette_ram[i] >> 16), pal5bit(m_palette_ram[i] >> 21), pal5bit(m_palette_ram[i] >> 26) );
			}
		}
	}
}

WRITE32_MEMBER(seibuspi_state::sprite_dma_start_w)
{
	if (m_video_dma_address != 0)
	{
		memcpy( m_sprite_ram, &m_spimainram[(m_video_dma_address / 4) - 0x200], m_sprite_dma_length);
	}
}

WRITE32_MEMBER(seibuspi_state::video_dma_length_w)
{
	COMBINE_DATA( &m_video_dma_length );
}

WRITE32_MEMBER(seibuspi_state::video_dma_address_w)
{
	COMBINE_DATA( &m_video_dma_address );
}

static void drawgfx_blend(bitmap_rgb32 &bitmap, const rectangle &cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, int sx, int sy)
{
	seibuspi_state *state = gfx->machine().driver_data<seibuspi_state>();
	const pen_t *pens = &gfx->machine().pens[gfx->color_base];
	const UINT8 *dp;
	int i, j;
	int x1, x2;
	int y1, y2;
	int px, py;
	int xd = 1, yd = 1;

	int width = gfx->width;
	int height = gfx->height;

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

	if (gfx->total_elements <= 0x10000)
	{
		code &= 0xffff;
	}

	dp = gfx_element_get_data(gfx, code);

	// draw
	for (j=y1; j <= y2; j++)
	{
		UINT32 *p = &bitmap.pix32(j);
		UINT8 trans_pen = (1 << state->m_sprite_bpp) - 1;
		int dp_i = (py * width) + px;
		py += yd;

		for (i=x1; i <= x2; i++)
		{
			UINT8 pen = dp[dp_i];
			if (pen != trans_pen)
			{
				int global_pen = pen + (color << state->m_sprite_bpp);
				UINT8 alpha = state->m_alpha_table[global_pen];
				if (alpha)
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

static void draw_sprites(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri_mask)
{
	seibuspi_state *state = machine.driver_data<seibuspi_state>();
	INT16 xpos, ypos;
	int tile_num, color;
	int width, height;
	int flip_x = 0, flip_y = 0;
	int a;
	int priority;
	int x,y, x1, y1;
	const gfx_element *gfx = machine.gfx[2];

	if( state->m_layer_enable & 0x10 )
		return;

	for( a = (state->m_sprite_dma_length / 4) - 2; a >= 0; a -= 2 ) {
		tile_num = (state->m_sprite_ram[a + 0] >> 16) & 0xffff;
		if( state->m_sprite_ram[a + 1] & 0x1000 )
			tile_num |= 0x10000;

		if( !tile_num )
			continue;

		priority = (state->m_sprite_ram[a + 0] >> 6) & 0x3;
		if( pri_mask != priority )
			continue;

		xpos = state->m_sprite_ram[a + 1] & 0x3ff;
		if( xpos & 0x200 )
			xpos |= 0xfc00;
		ypos = (state->m_sprite_ram[a + 1] >> 16) & 0x1ff;
		if( ypos & 0x100 )
			ypos |= 0xfe00;
		color = (state->m_sprite_ram[a + 0] & 0x3f);

		width = ((state->m_sprite_ram[a + 0] >> 8) & 0x7) + 1;
		height = ((state->m_sprite_ram[a + 0] >> 12) & 0x7) + 1;
		flip_x = (state->m_sprite_ram[a + 0] >> 11) & 0x1;
		flip_y = (state->m_sprite_ram[a + 0] >> 15) & 0x1;
		x1 = 0;
		y1 = 0;

		if( flip_x ) {
			x1 = 8 - width;
			width = width + x1;
		}
		if( flip_y ) {
			y1 = 8 - height;
			height = height + y1;
		}

		for( x=x1; x < width; x++ ) {
			for( y=y1; y < height; y++ ) {
				drawgfx_blend(bitmap, cliprect, gfx, tile_num, color, flip_x, flip_y, xpos + sprite_xtable[flip_x][x], ypos + sprite_ytable[flip_y][y]);

				/* xpos seems to wrap-around to 0 at 512 */
				if( (xpos + (16 * x) + 16) >= 512 ) {
					drawgfx_blend(bitmap, cliprect, gfx, tile_num, color, flip_x, flip_y, xpos - 512 + sprite_xtable[flip_x][x], ypos + sprite_ytable[flip_y][y]);
				}

				tile_num++;
			}
		}
	}
}

static TILE_GET_INFO( get_text_tile_info )
{
	seibuspi_state *state = machine.driver_data<seibuspi_state>();
	int offs = tile_index / 2;
	int tile = (state->m_tilemap_ram[offs + state->m_text_layer_offset] >> ((tile_index & 0x1) ? 16 : 0)) & 0xffff;
	int color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	SET_TILE_INFO(0, tile, color, 0);
}

static TILE_GET_INFO( get_back_tile_info )
{
	seibuspi_state *state = machine.driver_data<seibuspi_state>();
	int offs = tile_index / 2;
	int tile = (state->m_tilemap_ram[offs] >> ((tile_index & 0x1) ? 16 : 0)) & 0xffff;
	int color = (tile >> 13) & 0x7;

	tile &= 0x1fff;

	if( state->m_rf2_layer_bank[0] )
		tile |= 0x4000;

	SET_TILE_INFO(1, tile, color, 0);
}

static TILE_GET_INFO( get_mid_tile_info )
{
	seibuspi_state *state = machine.driver_data<seibuspi_state>();
	int offs = tile_index / 2;
	int tile = (state->m_tilemap_ram[offs + state->m_mid_layer_offset] >> ((tile_index & 0x1) ? 16 : 0)) & 0xffff;
	int color = (tile >> 13) & 0x7;

	tile &= 0x1fff;
	tile |= 0x2000;

	if( state->m_rf2_layer_bank[1] )
		tile |= 0x4000;

	SET_TILE_INFO(1, tile, color + 16, 0);
}

static TILE_GET_INFO( get_fore_tile_info )
{
	seibuspi_state *state = machine.driver_data<seibuspi_state>();
	int offs = tile_index / 2;
	int tile = (state->m_tilemap_ram[offs + state->m_fore_layer_offset] >> ((tile_index & 0x1) ? 16 : 0)) & 0xffff;
	int color = (tile >> 13) & 0x7;

	tile &= 0x1fff;
	tile |= state->m_bg_fore_layer_position;

	if( state->m_rf2_layer_bank[2] )
		tile |= 0x4000;

	tile |= ((state->m_layer_bank >> 27) & 0x1) << 13;

	SET_TILE_INFO(1, tile, color + 8, 0);
}

VIDEO_START( spi )
{
	seibuspi_state *state = machine.driver_data<seibuspi_state>();
	int i;
	int region_length;

	state->m_text_layer	= tilemap_create( machine, get_text_tile_info, tilemap_scan_rows,  8,8,64,32 );
	state->m_back_layer	= tilemap_create( machine, get_back_tile_info, tilemap_scan_cols,  16,16,32,32 );
	state->m_mid_layer	= tilemap_create( machine, get_mid_tile_info, tilemap_scan_cols,  16,16,32,32 );
	state->m_fore_layer	= tilemap_create( machine, get_fore_tile_info, tilemap_scan_cols,  16,16,32,32 );

	state->m_text_layer->set_transparent_pen(31);
	state->m_mid_layer->set_transparent_pen(63);
	state->m_fore_layer->set_transparent_pen(63);

	state->m_tilemap_ram = auto_alloc_array_clear(machine, UINT32, 0x4000/4);
	state->m_palette_ram = auto_alloc_array_clear(machine, UINT32, 0x3000/4);
	state->m_sprite_ram = auto_alloc_array_clear(machine, UINT32, 0x1000/4);

	state->m_sprite_bpp = 6;
	state->m_sprite_dma_length = 0x1000;

	for (i=0; i < 6144; i++) {
		palette_set_color(machine, i, MAKE_RGB(0, 0, 0));
	}

	memset(state->m_alpha_table, 0, 6144 * sizeof(UINT8));

	// sprites
	//for (i = 1792; i < 1808; i++) { state->m_alpha_table[i] = 1; } // breaks rdft
	for (i = 1840; i < 1856; i++) { state->m_alpha_table[i] = 1; }
	for (i = 1920; i < 1952; i++) { state->m_alpha_table[i] = 1; }
	//for (i = 1984; i < 2048; i++) { state->m_alpha_table[i] = 1; } // breaks batlball
	//for (i = 3840; i < 3904; i++) { state->m_alpha_table[i] = 1; } // breaks rdft
	for (i = 4032; i < 4096; i++) { state->m_alpha_table[i] = 1; }

	// mid layer
	for (i = 4960; i < 4992; i++) { state->m_alpha_table[i] = 1; }	// breaks ejanhs
	for (i = 5040; i < 5056; i++) { state->m_alpha_table[i] = 1; }	// breaks ejanhs
	for (i = 5104; i < 5120; i++) { state->m_alpha_table[i] = 1; }
	// fore layer
	for (i = 5552; i < 5568; i++) { state->m_alpha_table[i] = 1; }	// breaks ejanhs
	for (i = 5616; i < 5632; i++) { state->m_alpha_table[i] = 1; }	// breaks ejanhs
	// text layer
	for (i = 6000; i < 6016; i++) { state->m_alpha_table[i] = 1; }
	for (i = 6128; i < 6144; i++) { state->m_alpha_table[i] = 1; }

	region_length = machine.region("gfx2")->bytes();

	if (region_length <= 0x300000)
	{
		state->m_bg_fore_layer_position = 0x2000;
	}
	else if (region_length <= 0x600000)
	{
		state->m_bg_fore_layer_position = 0x4000;
	}
	else
	{
		state->m_bg_fore_layer_position = 0x8000;
	}
}

#ifdef UNUSED_FUNCTION
static void set_rowscroll(tilemap_t *layer, int scroll, INT16* rows)
{
	seibuspi_state *state = machine.driver_data<seibuspi_state>();
	int i;
	int x = state->m_spi_scrollram[scroll] & 0xffff;
	int y = (state->m_spi_scrollram[scroll] >> 16) & 0xffff;
	layer->set_scroll_rows(512);
	for( i=0; i < 512; i++ ) {
		layer->set_scrollx(i, x + rows[i]);
	}
	layer->set_scrolly(0, y);
}

static void set_scroll(tilemap_t *layer, int scroll)
{
	seibuspi_state *state = machine.driver_data<seibuspi_state>();
	int x = state->m_spi_scrollram[scroll] & 0xffff;
	int y = (state->m_spi_scrollram[scroll] >> 16) & 0xffff;
	layer->set_scrollx(0, x);
	layer->set_scrolly(0, y);
}
#endif


static void combine_tilemap(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *tile, int x, int y, int opaque, INT16 *rowscroll)
{
	seibuspi_state *state = machine.driver_data<seibuspi_state>();
	int i,j;
	UINT16 *s;
	UINT32 *d;
	UINT8 *t;
	UINT32 xscroll_mask, yscroll_mask;

	bitmap_ind16 &pen_bitmap = tile->pixmap();
	bitmap_ind8 &flags_bitmap = tile->flagsmap();
	xscroll_mask = pen_bitmap.width() - 1;
	yscroll_mask = pen_bitmap.height() - 1;

	for (j=cliprect.min_y; j <= cliprect.max_y; j++)
	{
		int rx = x;
		if (rowscroll)
		{
			rx += rowscroll[(j+y) & yscroll_mask];
		}

		d = &bitmap.pix32(j);
		s = &pen_bitmap.pix16((j+y) & yscroll_mask);
		t = &flags_bitmap.pix8((j+y) & yscroll_mask);
		for (i=cliprect.min_x+rx; i <= cliprect.max_x+rx; i++)
		{
			if (opaque || (t[i & xscroll_mask] & (TILEMAP_PIXEL_LAYER0 | TILEMAP_PIXEL_LAYER1)))
			{
				UINT16 pen = s[i & xscroll_mask];
				UINT8 alpha = state->m_alpha_table[pen];
				if (alpha)
				{
					*d = alpha_blend_r32(*d, machine.pens[pen], 0x7f);
				}
				else
				{
					*d = machine.pens[pen];
				}
			}
			++d;
		}
	}
}



SCREEN_UPDATE_RGB32( spi )
{
	seibuspi_state *state = screen.machine().driver_data<seibuspi_state>();
	INT16 *back_rowscroll, *mid_rowscroll, *fore_rowscroll;
	if( state->m_layer_bank & 0x80000000 ) {
		back_rowscroll	= (INT16*)&state->m_tilemap_ram[0x200];
		mid_rowscroll	= (INT16*)&state->m_tilemap_ram[0x600];
		fore_rowscroll	= (INT16*)&state->m_tilemap_ram[0xa00];
	} else {
		back_rowscroll	= NULL;
		mid_rowscroll	= NULL;
		fore_rowscroll	= NULL;
	}

	if( state->m_layer_enable & 0x1 )
		bitmap.fill(0, cliprect);

	if (!(state->m_layer_enable & 0x1))
		combine_tilemap(screen.machine(), bitmap, cliprect, state->m_back_layer, state->m_spi_scrollram[0] & 0xffff, (state->m_spi_scrollram[0] >> 16) & 0xffff, 1, back_rowscroll);

	draw_sprites(screen.machine(), bitmap, cliprect, 0);

	// if fore layer is enabled, draw priority 1 sprites behind mid layer
	if (!(state->m_layer_enable & 0x4))
		draw_sprites(screen.machine(), bitmap, cliprect, 1);

	if (!(state->m_layer_enable & 0x2))
		combine_tilemap(screen.machine(), bitmap, cliprect, state->m_mid_layer, state->m_spi_scrollram[1] & 0xffff, (state->m_spi_scrollram[1] >> 16) & 0xffff, 0, mid_rowscroll);

	// if fore layer is disabled, draw priority 1 sprites above mid layer
	if ((state->m_layer_enable & 0x4))
		draw_sprites(screen.machine(), bitmap, cliprect, 1);

	draw_sprites(screen.machine(), bitmap, cliprect, 2);

	if (!(state->m_layer_enable & 0x4))
		combine_tilemap(screen.machine(), bitmap, cliprect, state->m_fore_layer, state->m_spi_scrollram[2] & 0xffff, (state->m_spi_scrollram[2] >> 16) & 0xffff, 0, fore_rowscroll);

	draw_sprites(screen.machine(), bitmap, cliprect, 3);

	combine_tilemap(screen.machine(), bitmap, cliprect, state->m_text_layer, 0, 0, 0, NULL);
	return 0;
}

VIDEO_START( sys386f2 )
{
	seibuspi_state *state = machine.driver_data<seibuspi_state>();
	int i;

	state->m_palette_ram = auto_alloc_array_clear(machine, UINT32, 0x4000/4);
	state->m_sprite_ram = auto_alloc_array_clear(machine, UINT32, 0x2000/4);

	state->m_sprite_bpp = 8;
	state->m_sprite_dma_length = 0x2000;
	state->m_layer_enable = 0;

	for (i=0; i < 8192; i++) {
		palette_set_color(machine, i, MAKE_RGB(0, 0, 0));
	}

	memset(state->m_alpha_table, 0, 8192 * sizeof(UINT8));
}

SCREEN_UPDATE_RGB32( sys386f2 )
{
	bitmap.fill(0, cliprect);
	draw_sprites(screen.machine(), bitmap, cliprect, 0);
	draw_sprites(screen.machine(), bitmap, cliprect, 1);
	draw_sprites(screen.machine(), bitmap, cliprect, 2);
	draw_sprites(screen.machine(), bitmap, cliprect, 3);
	return 0;
}
