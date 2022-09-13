// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Barry Rodewald
/******************************************************************************
 *
 * Sharp X1 Video functions
 *
 * TODO:
 * - Rewrite drawing functions by taking scanline renderer into account
 *   * cfr. x1fdemo raster effect on first screen.
 * - Use mc6845 internal functions instead of breaking encapsulation
 *   * annoying due of the double height/width stuff.
 * - Improve border drawing, pinpoint what are the visible limits for a mc6845;
 * - Move X1Turbo features into specific overrides;
 *
 *****************************************************************************/

#include "emu.h"
#include "x1.h"

/*************************************
 *
 *  Video Functions
 *
 *************************************/

void x1_state::video_start()
{
	m_gfx_bitmap_ram = make_unique_clear<uint8_t[]>(0xc000*2);
	m_bitmapbank->configure_entries(0, 2, m_gfx_bitmap_ram.get(), 0xc000);
	m_bitmapbank->set_entry(0);
	// TODO: set this up only on x1turbo
	m_pal_4096 = make_unique_clear<uint8_t[]>(0x1000*3);
}

// helper for a single tile pixel taking height and width into account
// TODO: height is never used
void x1_state::x1_draw_pixel(bitmap_rgb32 &bitmap, int y, int x, uint16_t pen, uint8_t width, uint8_t height)
{
	if(!m_screen->visible_area().contains(x, y))
		return;

	if(width && height)
	{
		bitmap.pix(y+0+m_ystart, x+0+m_xstart) = m_palette->pen(pen);
		bitmap.pix(y+0+m_ystart, x+1+m_xstart) = m_palette->pen(pen);
		bitmap.pix(y+1+m_ystart, x+0+m_xstart) = m_palette->pen(pen);
		bitmap.pix(y+1+m_ystart, x+1+m_xstart) = m_palette->pen(pen);
	}
	else if(width)
	{
		bitmap.pix(y+m_ystart, x+0+m_xstart) = m_palette->pen(pen);
		bitmap.pix(y+m_ystart, x+1+m_xstart) = m_palette->pen(pen);
	}
	else if(height)
	{
		bitmap.pix(y+0+m_ystart, x+m_xstart) = m_palette->pen(pen);
		bitmap.pix(y+1+m_ystart, x+m_xstart) = m_palette->pen(pen);
	}
	else
		bitmap.pix(y+m_ystart, x+m_xstart) = m_palette->pen(pen);
}

#define mc6845_h_char_total     (m_crtc_vreg[0])
#define mc6845_h_display        (m_crtc_vreg[1])
#define mc6845_h_sync_pos       (m_crtc_vreg[2])
#define mc6845_sync_width       (m_crtc_vreg[3])
#define mc6845_v_char_total     (m_crtc_vreg[4])
#define mc6845_v_total_adj      (m_crtc_vreg[5])
#define mc6845_v_display        (m_crtc_vreg[6])
#define mc6845_v_sync_pos       (m_crtc_vreg[7])
#define mc6845_mode_ctrl        (m_crtc_vreg[8])
#define mc6845_tile_height      (m_crtc_vreg[9]+1)
#define mc6845_cursor_y_start   (m_crtc_vreg[0x0a])
#define mc6845_cursor_y_end     (m_crtc_vreg[0x0b])
#define mc6845_start_addr       (((m_crtc_vreg[0x0c]<<8) & 0x3f00) | (m_crtc_vreg[0x0d] & 0xff))
#define mc6845_cursor_addr      (((m_crtc_vreg[0x0e]<<8) & 0x3f00) | (m_crtc_vreg[0x0f] & 0xff))
#define mc6845_light_pen_addr   (((m_crtc_vreg[0x10]<<8) & 0x3f00) | (m_crtc_vreg[0x11] & 0xff))
#define mc6845_update_addr      (((m_crtc_vreg[0x12]<<8) & 0x3f00) | (m_crtc_vreg[0x13] & 0xff))


// separate tile index when we are under double height condition
uint8_t x1_state::check_prev_height(int x, int y, int x_size)
{
	uint8_t prev_tile = m_tvram[(x+((y-1)*x_size)+mc6845_start_addr) & 0x7ff];
	uint8_t cur_tile = m_tvram[(x+(y*x_size)+mc6845_start_addr) & 0x7ff];
	uint8_t prev_attr = m_avram[(x+((y-1)*x_size)+mc6845_start_addr) & 0x7ff];
	uint8_t cur_attr = m_avram[(x+(y*x_size)+mc6845_start_addr) & 0x7ff];

	if(prev_tile == cur_tile && prev_attr == cur_attr)
		return 8;

	return 0;
}

// exoa2: if double height isn't enabled on the first tile of the row then it is masked out on every other tile.
// (game enables it for sprite/background collision)
uint8_t x1_state::check_line_valid_height(int y, int x_size, int height)
{
	uint8_t line_attr = m_avram[(0+(y*x_size)+mc6845_start_addr) & 0x7ff];

	if((line_attr & 0x40) == 0)
		return 0;

	return height;
}

// tilemap drawing
void x1_state::draw_fgtilemap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/*
	    attribute table:
	    x--- ---- double width
	    -x-- ---- double height
	    --x- ---- PCG select
	    ---x ---- color blinking (if 1 reverses color patterns rather than true blinking)
	    ---- x--- reverse color
	    ---- -xxx color pen

	    X1 Turbo can also access an additional Kanji VRAM area
	    x--- ---- select Kanji ROM
	    -x-- ---- Kanji side (0=left, 1=right)
	    --x- ---- Underline
	    ---x ---- Kanji ROM select (0=level 1, 1=level 2)
	    ---- xxxx Kanji upper 4 bits
	*/

	int y, x, res_x, res_y;
	uint32_t tile_offset;
	uint8_t x_size, y_size;

	x_size = mc6845_h_display;
	y_size = mc6845_v_display;

	//don't bother if screen is off
	if(x_size == 0 || y_size == 0)
		return;

	if(x_size != 80 && x_size != 40 && y_size != 25)
		popmessage("%d %d",x_size,y_size);

	for (y = 0; y < y_size; y++)
	{
		for (x = 0; x < x_size; x++)
		{
			// prepare information for this specific row/col
			int tile = m_tvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff];
			int color = m_avram[((x+y*x_size)+mc6845_start_addr) & 0x7ff] & 0x1f;
			int width = BIT(m_avram[((x+y*x_size)+mc6845_start_addr) & 0x7ff], 7);
			int height = BIT(m_avram[((x+y*x_size)+mc6845_start_addr) & 0x7ff], 6);
			int pcg_bank = BIT(m_avram[((x+y*x_size)+mc6845_start_addr) & 0x7ff], 5);
			uint8_t *gfx_data = pcg_bank ? m_pcg_ram.get() : m_cg_rom;
			int knj_enable = 0;
			int knj_side = 0;
			int knj_bank = 0;
			int knj_uline = 0;
			if(m_is_turbo)
			{
				knj_enable = BIT(m_kvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff], 7);
				knj_side = BIT(m_kvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff], 6);
				knj_uline = BIT(m_kvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff], 5);
				// TODO: verify usage and implement
				//knj_lv2 = BIT(m_kvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff], 4);
				knj_bank = m_kvram[((x+y*x_size)+mc6845_start_addr) & 0x7ff] & 0x0f;
				if(knj_enable)
				{
					gfx_data = m_kanji_rom;
					tile = ((tile + (knj_bank << 8)) << 1) + (knj_side & 1);
				}
			}

			{
				int pen[3], pen_mask, pcg_pen, xi, yi, dy;

				pen_mask = color & 7;

				dy = 0;

				height = check_line_valid_height(y,x_size,height);

				if(height && y)
					dy = check_prev_height(x,y,x_size);

				/* guess: assume that if Kanji read is enabled then no double height can occur */
				if(knj_enable) { height = 0; }

				for(yi = 0; yi < mc6845_tile_height; yi++)
				{
					for(xi = 0; xi < 8; xi++)
					{
						if(knj_enable)
						{
							// Kanji ROM read
							tile_offset  = tile * 16;
							tile_offset += (yi+dy*(m_scrn_reg.v400_mode+1)) >> (height+m_scrn_reg.v400_mode);
							pen[0] = gfx_data[tile_offset+0x0000]>>(7-xi) & (pen_mask & 1)>>0;
							pen[1] = gfx_data[tile_offset+0x0000]>>(7-xi) & (pen_mask & 2)>>1;
							pen[2] = gfx_data[tile_offset+0x0000]>>(7-xi) & (pen_mask & 4)>>2;

							// apply underline
							if(yi == mc6845_tile_height-1 && knj_uline)
							{
								pen[0] = (pen_mask & 1)>>0;
								pen[1] = (pen_mask & 2)>>1;
								pen[2] = (pen_mask & 4)>>2;
							}

							if((yi >= 16 && m_scrn_reg.v400_mode == 0) || (yi >= 32 && m_scrn_reg.v400_mode == 1))
								pen[0] = pen[1] = pen[2] = 0;
						}
						else if(pcg_bank)
						{
							// PCG read
							tile_offset  = tile * 8;
							tile_offset += (yi+dy*(m_scrn_reg.v400_mode+1)) >> (height+m_scrn_reg.v400_mode);

							pen[0] = gfx_data[tile_offset+0x0000]>>(7-xi) & (pen_mask & 1)>>0;
							pen[1] = gfx_data[tile_offset+0x0800]>>(7-xi) & (pen_mask & 2)>>1;
							pen[2] = gfx_data[tile_offset+0x1000]>>(7-xi) & (pen_mask & 4)>>2;

							if((yi >= 8 && m_scrn_reg.v400_mode == 0) || (yi >= 16 && m_scrn_reg.v400_mode == 1))
								pen[0] = pen[1] = pen[2] = 0;
						}
						else
						{
							// ANK read (-> BIOS GFX ROM)
							tile_offset  = tile * (8*(m_scrn_reg.ank_sel+1));
							tile_offset += (yi+dy*(m_scrn_reg.v400_mode+1)) >> (height+m_scrn_reg.v400_mode);

							pen[0] = gfx_data[tile_offset+m_scrn_reg.ank_sel*0x0800]>>(7-xi) & (pen_mask & 1)>>0;
							pen[1] = gfx_data[tile_offset+m_scrn_reg.ank_sel*0x0800]>>(7-xi) & (pen_mask & 2)>>1;
							pen[2] = gfx_data[tile_offset+m_scrn_reg.ank_sel*0x0800]>>(7-xi) & (pen_mask & 4)>>2;

							// disable drawing it goes beyond the available tile space
							if(m_scrn_reg.ank_sel)
							{
								if((yi >= 16 && m_scrn_reg.v400_mode == 0) || (yi >= 32 && m_scrn_reg.v400_mode == 1))
									pen[0] = pen[1] = pen[2] = 0;
							}
							else
							{
								if((yi >=  8 && m_scrn_reg.v400_mode == 0) || (yi >= 16 && m_scrn_reg.v400_mode == 1))
									pen[0] = pen[1] = pen[2] = 0;
							}
						}

						pcg_pen = (pen[2] << 2)|(pen[1] << 1)|(pen[0] << 0);

						// apply blink reverser
						// TODO: verify interval
						if(color & 0x10 && m_screen->frame_number() & 0x10)
							pcg_pen ^= 7;

						// ignore a color entry of zero (drawn at mixing time)
						if(pcg_pen == 0 && (!(color & 8)))
							continue;

						// reverse attribute
						if(color & 8)
							pcg_pen ^= 7;

						// X1Turbo only: the black clip register overrides the pen with black if it's hit
						if((m_scrn_reg.blackclip & 8) && (color == (m_scrn_reg.blackclip & 7)))
							pcg_pen = 0;

						res_x = x*8+xi*(width+1);
						res_y = y*(mc6845_tile_height)+yi;

						// apply partial update
						// TODO: not working properly, see top of file
						if(res_y < cliprect.min_y || res_y > cliprect.max_y)
							continue;

						x1_draw_pixel(bitmap, res_y, res_x, pcg_pen, width, 0);
					}
				}
			}

			if(width) //skip next char if we are under double width condition
				x++;
		}
	}
}

/*
 * Priority Mixer Calculation (pri)
 *
 * If pri is 0xff then the bitmap entirely covers the tilemap, if it's 0x00 then
 * the tilemap priority is entirely above the bitmap. Any other value mixes the
 * bitmap and the tilemap priorities based on the pen value, bit 0 = entry 0 <-> bit 7 = entry 7
 * of the bitmap.
 *
 */
int x1_state::priority_mixer_pri(int color)
{
	int pri_i,pri_mask_calc;

	pri_i = 0;
	pri_mask_calc = 1;

	while(pri_i < 7)
	{
		if((color & 7) == pri_i)
			break;

		pri_i++;
		pri_mask_calc<<=1;
	}

	return pri_mask_calc;
}

// bitmap drawing
void x1_state::draw_gfxbitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, int plane, int pri)
{
	int xi, yi, x, y;
	int pen_r, pen_g, pen_b, color;
	int pri_mask_val;
	uint8_t x_size, y_size;
	int gfx_offset;

	x_size = mc6845_h_display;
	y_size = mc6845_v_display;

	//don't bother if screen is off
	if(x_size == 0 || y_size == 0)
		return;

	if(x_size != 80 && x_size != 40 && y_size != 25)
		popmessage("%d %d",x_size,y_size);

	//popmessage("%04x %02x",mc6845_start_addr,mc6845_tile_height);

	for (y = 0; y < y_size; y++)
	{
		for(x = 0; x < x_size; x++)
		{
			for(yi = 0; yi < (mc6845_tile_height); yi++)
			{
				for(xi = 0; xi < 8; xi++)
				{
					gfx_offset = ((x+(y*x_size)) + mc6845_start_addr) & 0x7ff;
					// X1Turbo only: apply extra offset bank
					gfx_offset+= ((yi >> m_scrn_reg.v400_mode) * 0x800) & 0x3fff;
					// X1Turbo only: plane variable, for double buffering
					pen_b = (m_gfx_bitmap_ram[gfx_offset+0x0000+plane*0xc000]>>(7-xi)) & 1;
					pen_r = (m_gfx_bitmap_ram[gfx_offset+0x4000+plane*0xc000]>>(7-xi)) & 1;
					pen_g = (m_gfx_bitmap_ram[gfx_offset+0x8000+plane*0xc000]>>(7-xi)) & 1;

					color =  (pen_g<<2 | pen_r<<1 | pen_b<<0) | 8;

					// checkout priority mixing
					// TODO: due of this we loop twice, it should be handled at mixing time instead.
					pri_mask_val = priority_mixer_pri(color);
					if(pri_mask_val & pri) continue;

					// X1Turbo only: the black clip register overrides the pen with black if it's hit
					if((color == 8 && m_scrn_reg.blackclip & 0x10) || (color == 9 && m_scrn_reg.blackclip & 0x20))
						color = 0;

					// apply partial update
					// TODO: not working properly, see top of file
					if(y*(mc6845_tile_height)+yi < cliprect.min_y || y*(mc6845_tile_height)+yi > cliprect.max_y)
						continue;

					// TODO: call a fn subset instead of looping for a width/height that is never hit
					x1_draw_pixel(bitmap, y*(mc6845_tile_height)+yi, x*8+xi, color, 0, 0);
				}
			}
		}
	}
}

uint32_t x1_state::screen_update_x1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t(0xff,0x00,0x00,0x00), cliprect);

	// TODO: correct calculation thru mc6845 regs
	m_xstart = ((mc6845_h_char_total - mc6845_h_sync_pos) * 8) / 2;
	m_ystart = ((mc6845_v_char_total - mc6845_v_sync_pos) * 8) / 2;

//  popmessage("%d %d %d %d",mc6845_h_sync_pos,mc6845_v_sync_pos,mc6845_h_char_total,mc6845_v_char_total);

	draw_gfxbitmap(bitmap, cliprect, m_scrn_reg.disp_bank, m_scrn_reg.pri);
	draw_fgtilemap(bitmap, cliprect);
	draw_gfxbitmap(bitmap, cliprect, m_scrn_reg.disp_bank, m_scrn_reg.pri ^ 0xff);

	return 0;
}
