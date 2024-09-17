// license:BSD-3-Clause
// copyright-holders:Barry Rodewald,Carl
/*

    Sharp X68000 video functions
    driver by Barry Rodewald

    X68000 video hardware (there are some minor revisions to these custom chips across various X680x0 models):
        Custom sprite controller "Cynthia"
        Custom CRT controller "Vinas / Vicon"
        Custom video controller "VSOP / VIPS"
        Custom video data selector "Cathy"

    In general terms:
        1 "Text" layer - effectively a 4bpp bitmap split into 4 planes at 1bpp each
                         512kB "text" VRAM
                         can write to multiple planes at once
                         can copy one character line to another character line
                         is 1024x1024 in size
        Up to 4 graphic layers - can be 4 layers with a 16 colour palette, 2 layers with a 256 colour palette,
                                 or 1 layer at 16-bit RGB.
                                 512k graphic VRAM
                                 all layers are 512x512, but at 16 colours, the 4 layers can be combined into 1 1024x1024 layer
                                 one or more layers can be cleared at once quickly with a simple hardware function
         2 tilemapped layers - can be 8x8 or 16x16, 16 colours per tile, max 256 colours overall
         1 sprite layer - up to 128 16x16 sprites, 16 colours per sprite, maximum 16 sprites per scanline (not yet implemented).

         Questions: What do the other bits in m_video.reg[2] do?
                    How is the intensity applied during blending if at all?
                    Black appears to be opaque only at priority 2 but not 3, is that right?
                    Are the gfx layers blended from the bottom up or all at once?
                    Special priority in 16bit color mode?

*/

#include "emu.h"
#include "x68k.h"

//#define VERBOSE 0
#include "logmacro.h"


rgb_t x68k_state::GGGGGRRRRRBBBBBI(uint32_t raw)
{
	uint8_t const i = raw & 1;
	uint8_t const r = pal6bit(((raw >> 5) & 0x3e) | i);
	uint8_t const g = pal6bit(((raw >> 10) & 0x3e) | i);
	uint8_t const b = pal6bit(((raw >> 0) & 0x3e) | i);
	return rgb_t(r, g, b);
}

inline void x68k_state::plot_pixel(bitmap_rgb32 &bitmap, int x, int y, uint32_t color)
{
	bitmap.pix(y, x) = (uint16_t)color;
}
/*
bitmap_rgb32* ::x68k_get_gfx_page(int pri,int type)
{
    if(type == GFX16)
    {
        switch(pri)
        {
        case 0:
            return x68k_gfx_0_bitmap_16;
        case 1:
            return x68k_gfx_1_bitmap_16;
        case 2:
            return x68k_gfx_2_bitmap_16;
        case 3:
            return x68k_gfx_3_bitmap_16;
        default:
            return x68k_gfx_0_bitmap_16;  // should never reach here.
        }
    }
    if(type == GFX256)
    {
        switch(pri)
        {
        case 0:
        case 1:
            return x68k_gfx_0_bitmap_256;
        case 2:
        case 3:
            return x68k_gfx_1_bitmap_256;
        default:
            return x68k_gfx_0_bitmap_256;  // should never reach here.
        }
    }
    if(type == GFX65536)
        return x68k_gfx_0_bitmap_65536;

    return nullptr;  // should never reach here either.
}
*/

uint16_t x68k_state::tvram_read(offs_t offset)
{
	return m_tvram[offset];
}

void x68k_state::tvram_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_tvram[offset]);
}

uint16_t x68k_state::gvram_read(offs_t offset)
{
	return m_gvram[offset];
}

void x68k_state::gvram_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_gvram[offset]);
}

void x68k_state::spritereg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_spritereg[offset]);
	switch(offset)
	{
	case 0x400:
		m_bg0_8->set_scrollx(0,(data - m_video.bg_hstart) & 0x3ff);
		m_bg0_16->set_scrollx(0,(data - m_video.bg_hstart) & 0x3ff);
		break;
	case 0x401:
		m_bg0_8->set_scrolly(0,(data - m_video.bg_vstart) & 0x3ff);
		m_bg0_16->set_scrolly(0,(data - m_video.bg_vstart) & 0x3ff);
		break;
	case 0x402:
		m_bg1_8->set_scrollx(0,(data - m_video.bg_hstart) & 0x3ff);
		m_bg1_16->set_scrollx(0,(data - m_video.bg_hstart) & 0x3ff);
		break;
	case 0x403:
		m_bg1_8->set_scrolly(0,(data - m_video.bg_vstart) & 0x3ff);
		m_bg1_16->set_scrolly(0,(data - m_video.bg_vstart) & 0x3ff);
		break;
	case 0x406:  // BG H-DISP (normally equals CRTC reg 2 value + 4)
		m_video.bg_hstart = (data - 4) * 8 + 1;
		break;
	case 0x407:  // BG V-DISP (like CRTC reg 6)
		m_video.bg_vstart = data;
		break;
	case 0x408:  // BG H/V-Res
		m_video.bg_hvres = data & 0x1f;
		break;
	}
}

uint16_t x68k_state::spritereg_r(offs_t offset)
{
	if(offset >= 0x400 && offset < 0x404)
		return m_spritereg[offset] & 0x3ff;
	return m_spritereg[offset];
}

void x68k_state::spriteram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(m_spriteram+offset);
	m_video.tile8_dirty[offset / 16] = 1;
	m_video.tile16_dirty[offset / 64] = 1;
	if(offset < 0x2000)
	{
		m_bg1_8->mark_all_dirty();
		m_bg1_16->mark_all_dirty();
		m_bg0_8->mark_all_dirty();
		m_bg0_16->mark_all_dirty();
	}
	if(offset >= 0x2000 && offset < 0x3000)
	{
		m_bg1_8->mark_tile_dirty(offset & 0x0fff);
		m_bg1_16->mark_tile_dirty(offset & 0x0fff);
	}
	if(offset >= 0x3000)
	{
		m_bg0_8->mark_tile_dirty(offset & 0x0fff);
		m_bg0_16->mark_tile_dirty(offset & 0x0fff);
	}
}

uint16_t x68k_state::spriteram_r(offs_t offset)
{
	return m_spriteram[offset];
}

bool x68k_state::get_text_pixel(int line, int pixel, uint16_t *pix)
{
	uint32_t loc;  // location in TVRAM
	int bit;
	int divisor = 1;
	if(m_crtc->gfx_double_scan())
		divisor = 2;
	int xscr = (m_crtc->xscr_text() & 0x3ff) + (pixel - m_crtc->hbegin());
	int yscr = (m_crtc->yscr_text() & 0x3ff);

	// adjust for scroll registers
	loc = ((((line- m_crtc->vbegin()) / divisor) + yscr) & 0x3ff) * 64;
	loc += (xscr / 16) & 0x7f;
	loc &= 0xffff;
	bit = 15 - (xscr & 0x0f);
	*pix = (((m_tvram[loc] >> bit) & 0x01) ? 1 : 0)
						+ (((m_tvram[loc+0x10000] >> bit) & 0x01) ? 2 : 0)
						+ (((m_tvram[loc+0x20000] >> bit) & 0x01) ? 4 : 0)
						+ (((m_tvram[loc+0x30000] >> bit) & 0x01) ? 8 : 0);
	// Colour 0 is displayable if the text layer is at the priority level 2
	if(*pix || ((m_video.reg[1] & 0x0c00) == 0x0800))
		return true;
	return false;
}

bool x68k_state::draw_gfx_scanline( bitmap_ind16 &bitmap, rectangle cliprect, uint8_t priority)
{
	int pixel;
	uint32_t loc;  // location in GVRAM
	uint32_t lineoffset;
	uint16_t xscr,yscr;
	uint16_t colour = 0;
	int shift;
	bool blend, ret = false;
	uint16_t *pal = (uint16_t *)m_gfxpalette->basemem().base();
	int divisor = 1;
	if(m_crtc->gfx_double_scan())
		divisor = 2;

	for(int scanline=cliprect.min_y;scanline<=cliprect.max_y;scanline++)  // per scanline
	{
		if(m_crtc->is_1024x1024())  // 1024x1024 "real" screen size - use 1024x1024 16-colour gfx layer
		{
			// adjust for scroll registers
			if(m_video.reg[2] & 0x0010 && priority == m_video.gfxlayer_pri[0])
			{
				xscr = (m_crtc->xscr_gfx(0) & 0x3ff);
				yscr = (m_crtc->yscr_gfx(0) & 0x3ff);
				lineoffset = (((scanline - m_crtc->vbegin()) + yscr) & 0x3ff) * 1024;
				loc = xscr & 0x3ff;
				for(pixel=m_crtc->hbegin();pixel<=m_crtc->hend();pixel++)
				{
					switch(lineoffset & 0xc0000)
					{
					case 0x00000:
						colour = m_gvram[lineoffset + (loc & 0x3ff)] & 0x000f;
						break;
					case 0x40000:
						colour = (m_gvram[(lineoffset - 0x40000) + (loc & 0x3ff)] & 0x00f0) >> 4;
						break;
					case 0x80000:
						colour = (m_gvram[(lineoffset - 0x80000) + (loc & 0x3ff)] & 0x0f00) >> 8;
						break;
					case 0xc0000:
						colour = (m_gvram[(lineoffset - 0xc0000) + (loc & 0x3ff)] & 0xf000) >> 12;
						break;
					}
					if(colour || (priority == 3))
					{
						if(((m_video.reg[2] & 0x1800) == 0x1000) && (colour & 1))
							m_special.pix(scanline, pixel) = colour;
						else
						{
							bitmap.pix(scanline, pixel) = colour;
							m_special.pix(scanline, pixel) = 0;
						}
					}
					loc++;
					loc &= 0x3ff;
				}
			}
		}
		else  // else 512x512 "real" screen size
		{
			if(m_video.reg[2] & (1 << priority))
			{
				int page = m_video.gfxlayer_pri[priority];
				// adjust for scroll registers
				switch(m_video.reg[0] & 0x03)
				{
				case 0x00: // 16 colours
					xscr = m_crtc->xscr_gfx(page) & 0x1ff;
					yscr = m_crtc->yscr_gfx(page) & 0x1ff;
					lineoffset = (((scanline - m_crtc->vbegin() / divisor) + yscr) & 0x1ff) * 512;
					loc = xscr & 0x1ff;
					shift = 4;
					if((m_video.reg[2] & 0x1a00) == 0x1a00)
						ret = true;
					for(pixel=m_crtc->hbegin();pixel<=m_crtc->hend();pixel++)
					{
						colour = ((m_gvram[lineoffset + loc] >> page*shift) & 0x000f);
						if(ret && (colour & 1))
						{
							blend = true;
							colour &= 0xfe;
						}
						else
							blend = false;
						if(colour || (priority == 3))
						{
							if(ret)
							{
								if(blend && bitmap.pix(scanline, pixel))
									bitmap.pix(scanline, pixel) = ((bitmap.pix(scanline, pixel) >> 1) & 0x7bde) + ((pal[colour] >> 1) & 0x7bde) + 1;
								else
									bitmap.pix(scanline, pixel) = (pal[colour] & 0xfffe) + blend;
							}
							else if(((m_video.reg[2] & 0x1800) == 0x1000) && (colour & 1))
								m_special.pix(scanline, pixel) = colour;
							else
							{
								m_special.pix(scanline, pixel) = 0;
								bitmap.pix(scanline, pixel) = colour;
							}
						}
						else if(((m_video.reg[2] & 0x1800) == 0x1000) && m_special.pix(scanline, pixel))
						{
							bitmap.pix(scanline, pixel) = m_special.pix(scanline, pixel);
							m_special.pix(scanline, pixel) = 0;
						}
						loc++;
						loc &= 0x1ff;
					}
					break;
				case 0x01: // 256 colours
					if(page == 0 || page == 2)
					{
						// What effect do priorites and plane disable have here? Does it work in 16bit color mode?
						uint16_t xscr0, yscr0, xscr1, yscr1;
						uint32_t loc0, loc1, lineoffset0, lineoffset1;
						xscr0 = m_crtc->xscr_gfx(page) & 0x1ff;
						yscr0 = m_crtc->yscr_gfx(page) & 0x1ff;
						xscr1 = m_crtc->xscr_gfx(page + 1) & 0x1ff;
						yscr1 = m_crtc->yscr_gfx(page + 1) & 0x1ff;

						lineoffset0 = (((scanline - m_crtc->vbegin() / divisor) + yscr0) & 0x1ff) * 512;
						loc0 = xscr0 & 0x1ff;
						lineoffset1 = (((scanline - m_crtc->vbegin() / divisor) + yscr1) & 0x1ff) * 512;
						loc1 = xscr1 & 0x1ff;
						shift = 4;
						if((m_video.reg[2] & 0x1a00) == 0x1a00)
							ret = true;
						for(pixel=m_crtc->hbegin();pixel<=m_crtc->hend();pixel++)
						{
							colour = ((m_gvram[lineoffset0 + loc0] >> page*shift) & 0x000f);
							colour |= ((m_gvram[lineoffset1 + loc1] >> page*shift) & 0x00f0);
							if(ret && (colour & 1))
							{
								blend = true;
								colour &= 0xfe;
							}
							else
								blend = false;
							if(colour || (priority == 3))
							{
								if(ret)
								{
									if(blend && bitmap.pix(scanline, pixel))
										bitmap.pix(scanline, pixel) = ((bitmap.pix(scanline, pixel) >> 1) & 0x7bde) + ((pal[colour] >> 1) & 0x7bde) + 1;
									else
										bitmap.pix(scanline, pixel) = (pal[colour] & 0xfffe) + blend;
								}
								else if(((m_video.reg[2] & 0x1800) == 0x1000) && (colour & 1))
									m_special.pix(scanline, pixel) = colour;
								else
								{
									bitmap.pix(scanline, pixel) = colour;
									m_special.pix(scanline, pixel) = 0;
								}
							}
							else if(((m_video.reg[2] & 0x1800) == 0x1000) && m_special.pix(scanline, pixel))
							{
								bitmap.pix(scanline, pixel) = m_special.pix(scanline, pixel);
								m_special.pix(scanline, pixel) = 0;
							}
							loc0++;
							loc0 &= 0x1ff;
							loc1++;
							loc1 &= 0x1ff;
						}
					}
					break;
				case 0x03: // 65536 colours
					xscr = m_crtc->xscr_gfx(0) & 0x1ff;
					yscr = m_crtc->yscr_gfx(0) & 0x1ff;
					lineoffset = (((scanline - m_crtc->vbegin() / divisor) + yscr) & 0x1ff) * 512;
					loc = xscr & 0x1ff;
					for(pixel=m_crtc->hbegin();pixel<=m_crtc->hend();pixel++)
					{
						colour = m_gvram[lineoffset + loc];
						if(colour || (priority == 3))
							bitmap.pix(scanline, pixel) = colour;
						loc++;
						loc &= 0x1ff;
					}
					break;
				}
			}
		}
	}
	return ret;
}

bool x68k_state::draw_gfx(bitmap_rgb32 &bitmap,rectangle cliprect)
{
	int priority;
	bool gfxblend=false;
	rectangle gfxrect = cliprect;
	if(m_crtc->gfx_double_scan())
	{
		gfxrect.max_y >>= 1;
		gfxrect.min_y >>= 1;
	}

	if(m_crtc->gfx_layer_buffer())  // if graphic layers are set to buffer, then they aren't visible
		return false;

	m_gfxbitmap.fill(0, gfxrect);
	if((m_video.reg[2] & 0x1800) == 0x1000)
		m_special.fill(0, gfxrect);

	for(priority=3;priority>=0;priority--)
	{
		gfxblend = draw_gfx_scanline(m_gfxbitmap,gfxrect,priority);
	}
	return gfxblend;
}

template <bool Blend> rgb_t x68k_state::get_gfx_pixel(int scanline, int pixel, bool gfxblend, rgb_t blendpix)
{
	uint16_t colour;
	int divisor = 1;
	bool blend = false;
	if(m_crtc->gfx_double_scan())
		divisor = 2;
	if((m_video.reg[0] & 0x03) == 3)
	{
		colour = m_gfxbitmap.pix(scanline / divisor, pixel);
		if(colour || (m_video.gfx_pri == 2))
			return GGGGGRRRRRBBBBBI(colour);
	}
	else if(gfxblend)
	{
		colour = m_gfxbitmap.pix(scanline / divisor, pixel);
		if(Blend && (colour & 1))
			blend = true;
		else
			blend = false;
		if(colour || (m_video.gfx_pri == 2))
		{
			if(blend)
				return ((blendpix >> 1) & 0xff7f7f7f) + ((pal555(colour, 6, 11, 1) >> 2) & 0x3f3f3f);
			else
				return pal555(colour, 6, 11, 1);
		}
	}
	else
	{
		colour = m_gfxbitmap.pix(scanline / divisor, pixel) & 0xff;
		if(Blend && (colour & 1))
		{
			blend = true;
			colour &= 0xfe;
		}
		else
			blend = false;
		if((m_gfxpalette->pen(colour) & 0xffffff) || (m_video.gfx_pri == 2))
		{
			if(blend)
				return ((blendpix >> 1) & 0xff7f7f7f) + ((m_gfxpalette->pen(colour) >> 2) & 0x3f3f3f);
			else
				return m_gfxpalette->pen(colour);
		}
	}
	return 0;
}

// Sprite controller "Cynthia" at 0xeb0000
void x68k_state::draw_sprites(bitmap_ind16 &bitmap, screen_device &screen, rectangle cliprect)
{
	/*
	   0xeb0000 - 0xeb07ff - Sprite registers (up to 128)
	       + 00 : b9-0,  Sprite X position
	       + 02 : b9-0,  Sprite Y position
	       + 04 : b15,   Vertical Reversing (flipping?)
	              b14,   Horizontal Reversing
	              b11-8, Sprite colour
	              b7-0,  Sprite tile code (in PCG)
	       + 06 : b1-0,  Priority
	                     00 = Sprite not displayed

	   0xeb0800 - BG0 X Scroll  (10-bit)
	   0xeb0802 - BG0 Y Scroll
	   0xeb0804 - BG1 X Scroll
	   0xeb0806 - BG1 Y Scroll
	   0xeb0808 - BG control
	              b9,    BG/Sprite display (RAM and register access is faster if 1)
	              b4,    PCG area 1 available
	              b3,    BG1 display enable
	              b1,    PCG area 0 available
	              b0,    BG0 display enable
	   0xeb080a - Horizontal total (like CRTC reg 0 - is 0xff if in 256x256?)
	   0xeb080c - Horizontal display position (like CRTC reg 2 - +4)
	   0xeb080e - Vertical display position (like CRTC reg 6)
	   0xeb0810 - Resolution setting
	              b4,    "L/H" (apparently 15kHz/31kHz switch for sprites/BG?)
	              b3-2,  V-Res
	              b1-0,  H-Res (0 = 8x8 tilemaps, 1 = 16x16 tilemaps, 2 or 3 = unknown)
	*/
	int ptr,pri;
	int divisor = 1;
	if((!(m_video.bg_hvres & 0x0c) && m_crtc->gfx_double_scan()) || ((m_video.bg_hvres & 0x1c) == 0x10 && m_crtc->vfactor() == 1))
		divisor = 2;

	for(ptr=0;ptr<=508;ptr+=4)  // stepping through sprites
	{
		pri = m_spritereg[ptr+3] & 0x03;
		if(!pri) continue;
#ifdef MAME_DEBUG
		if(machine().input().code_pressed(KEYCODE_I)) continue;
#endif
		rectangle rect;
		int code = m_spritereg[ptr+2] & 0x00ff;
		int colour = (m_spritereg[ptr+2] & 0x0f00) >> 8;
		int xflip = m_spritereg[ptr+2] & 0x4000;
		int yflip = m_spritereg[ptr+2] & 0x8000;
		int sx = (m_spritereg[ptr+0] & 0x3ff) - 16;
		int sy = (m_spritereg[ptr+1] & 0x3ff) - 16;

		rect.min_x=m_video.bg_hstart;
		rect.min_y=m_video.bg_vstart;
		rect.max_x=rect.min_x + m_crtc->visible_width()-1;
		rect.max_y=rect.min_y + m_crtc->visible_height()-1;
		int pmask = (pri == 1) ? 0xfffe : (pri == 2) ? 0xfffc : 0xfff0;

		m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,code,colour,xflip,yflip,m_video.bg_hstart+sx,(m_video.bg_vstart / divisor)+sy,screen.priority(),pmask,0x00);
	}
}

void x68k_state::draw_bg(bitmap_ind16 &bitmap, screen_device &screen, int layer, bool opaque, rectangle rect)
{
	int sclx = layer ? m_spritereg[0x402] : m_spritereg[0x400];
	int scly = layer ? m_spritereg[0x403] : m_spritereg[0x401];
	tilemap_t* x68k_bg0;
	tilemap_t* x68k_bg1;
	tilemap_t* map;
	int divisor = 1;
	if((!(m_video.bg_hvres & 0x0c) && m_crtc->gfx_double_scan()) || ((m_video.bg_hvres & 0x1c) == 0x10 && m_crtc->vfactor() == 1))
		divisor = 2;

	if((m_spritereg[0x408] & 0x03) == 0x00)  // Sprite/BG H-Res 0=8x8, 1=16x16, 2 or 3 = undefined.
	{
		x68k_bg0 = m_bg0_8;
		x68k_bg1 = m_bg1_8;
	}
	else
	{
		x68k_bg0 = m_bg0_16;
		x68k_bg1 = m_bg1_16;
	}

	if(layer)
		map = (m_spritereg[0x404] & 0x0030) == 0x10 ? x68k_bg0 : x68k_bg1;
	else
		map = (m_spritereg[0x404] & 0x0006) == 0x02 ? x68k_bg0 : x68k_bg1;

	map->set_scrollx(0,(sclx - m_video.bg_hstart) & 0x3ff);
	map->set_scrolly(0,(scly - (m_video.bg_vstart / divisor)) & 0x3ff);
	map->draw(screen, bitmap, rect, opaque ? TILEMAP_DRAW_OPAQUE : 0, !layer ? 2 : 1);
}

static const gfx_layout x68k_pcg_8 =
{
	8,8,
	256,
	4,
	{ 0,1,2,3 },
	{ 8,12,0,4,24,28,16,20 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout x68k_pcg_16 =
{
	16,16,
	256,
	4,
	{ 0,1,2,3 },
	{ 8,12,0,4,24,28,16,20,8+64*8,12+64*8,64*8,4+64*8,24+64*8,28+64*8,16+64*8,20+64*8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
	8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8
};

TILE_GET_INFO_MEMBER(x68k_state::get_bg0_tile)
{
	int code = m_spriteram[0x3000+tile_index] & 0x00ff;
	int colour = (m_spriteram[0x3000+tile_index] & 0x0f00) >> 8;
	int flags = (m_spriteram[0x3000+tile_index] & 0xc000) >> 14;
	tileinfo.set(0,code,colour,flags);
}

TILE_GET_INFO_MEMBER(x68k_state::get_bg1_tile)
{
	int code = m_spriteram[0x2000+tile_index] & 0x00ff;
	int colour = (m_spriteram[0x2000+tile_index] & 0x0f00) >> 8;
	int flags = (m_spriteram[0x2000+tile_index] & 0xc000) >> 14;
	tileinfo.set(0,code,colour,flags);
}

TILE_GET_INFO_MEMBER(x68k_state::get_bg0_tile_16)
{
	int code = m_spriteram[0x3000+tile_index] & 0x00ff;
	int colour = (m_spriteram[0x3000+tile_index] & 0x0f00) >> 8;
	int flags = (m_spriteram[0x3000+tile_index] & 0xc000) >> 14;
	tileinfo.set(1,code,colour,flags);
}

TILE_GET_INFO_MEMBER(x68k_state::get_bg1_tile_16)
{
	int code = m_spriteram[0x2000+tile_index] & 0x00ff;
	int colour = (m_spriteram[0x2000+tile_index] & 0x0f00) >> 8;
	int flags = (m_spriteram[0x2000+tile_index] & 0xc000) >> 14;
	tileinfo.set(1,code,colour,flags);
}

void x68k_state::video_start()
{
	int gfx_index;

	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (m_gfxdecode->gfx(gfx_index) == nullptr)
			break;

	/* create the char set (gfx will then be updated dynamically from RAM) */
	m_gfxdecode->set_gfx(gfx_index, std::make_unique<gfx_element>(m_pcgpalette, x68k_pcg_8, memregion("user1")->base(), 0, 32, 0));

	gfx_index++;

	m_gfxdecode->set_gfx(gfx_index, std::make_unique<gfx_element>(m_pcgpalette, x68k_pcg_16, memregion("user1")->base(), 0, 32, 0));
	m_gfxdecode->gfx(gfx_index)->set_colors(32);

	/* Tilemaps */
	m_bg0_8 =  &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(x68k_state::get_bg0_tile)),    TILEMAP_SCAN_ROWS,  8,  8, 64, 64);
	m_bg1_8 =  &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(x68k_state::get_bg1_tile)),    TILEMAP_SCAN_ROWS,  8,  8, 64, 64);
	m_bg0_16 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(x68k_state::get_bg0_tile_16)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_bg1_16 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(x68k_state::get_bg1_tile_16)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);

	m_bg0_8->set_transparent_pen(0);
	m_bg1_8->set_transparent_pen(0);
	m_bg0_16->set_transparent_pen(0);
	m_bg1_16->set_transparent_pen(0);

	m_screen->register_screen_bitmap(m_pcgbitmap);
	m_screen->register_screen_bitmap(m_gfxbitmap);
	m_screen->register_screen_bitmap(m_special);

//  m_scanline_timer->adjust(attotime::zero, 0, attotime::from_hz(55.45)/568);
}

uint32_t x68k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle rect(0,0,0,0);
	int priority;
	int x;
	int pixel = 0, scanline = 0;
	//uint8_t *rom;

	if(m_sysport.contrast == 0)  // if monitor contrast is 0, then don't bother displaying anything
		return 0;

	rect.min_x=m_crtc->hbegin();
	rect.min_y=m_crtc->vbegin();
	rect.max_x=m_crtc->hend();
	rect.max_y=m_crtc->vend();

	if(rect.min_y < cliprect.min_y)
		rect.min_y = cliprect.min_y;
	if(rect.max_y > cliprect.max_y)
		rect.max_y = cliprect.max_y;

	// update tiles
	for(x=0;x<256;x++)
	{
		if(m_video.tile16_dirty[x] != 0)
		{
			m_gfxdecode->gfx(1)->mark_dirty(x);
			m_video.tile16_dirty[x] = 0;
		}
		if(m_video.tile8_dirty[x] != 0)
		{
			m_gfxdecode->gfx(0)->mark_dirty(x);
			m_video.tile8_dirty[x] = 0;
		}
	}

	bool clear = false;
	if(m_video.reg[2] & 0x0040)
	{
		rectangle pcgrect = rect;
		if(!(m_video.bg_hvres & 0x0c) && m_crtc->gfx_double_scan())
		{
			pcgrect.max_y >>= 1;
			pcgrect.min_y >>= 1;
		}
		if(m_spritereg[0x404] & 0x0008)
		{
			clear = true;
			draw_bg(m_pcgbitmap, screen, 1, true, pcgrect);
		}
		else if(m_spritereg[0x404] & 0x0001)
		{
			clear = true;
			draw_bg(m_pcgbitmap, screen, 0, true, pcgrect);
		}
	}
	if(clear)
	{
		int divisor = 1;
		if(!(m_video.bg_hvres & 0x0c) && m_crtc->gfx_double_scan())
			divisor = 2;
		for(scanline=rect.min_y;scanline<=rect.max_y;scanline++)
		{
			for(pixel=m_crtc->hbegin();pixel<=m_crtc->hend();pixel++)
			{
				uint8_t colour = m_pcgbitmap.pix(scanline / divisor, pixel) & 0xff;
				bitmap.pix(scanline, pixel) = m_pcgpalette->pen(colour);
			}
		}
	}
	else
		bitmap.fill(m_pcgpalette->pen(0), rect);

	bool gfx16bcol = false; // if the gfx layer was blended so gfxbitmap is 16bit color
	int divisor = 1;
	// Graphics screen(s)
	gfx16bcol = draw_gfx(bitmap,rect);

	// Sprite / BG Tiles
	if(/*(m_spritereg[0x404] & 0x0200) &&*/ (m_video.reg[2] & 0x0040))
	{
		rectangle pcgrect = rect;
		if((!(m_video.bg_hvres & 0x0c) && m_crtc->gfx_double_scan()) || ((m_video.bg_hvres & 0x1c) == 0x10 && m_crtc->vfactor() == 1) || (m_video.bg_hvres == 1 && m_crtc->vfactor() == 2))
		{
			pcgrect.max_y >>= 1;
			pcgrect.min_y >>= 1;
			divisor = 2;
		}
		m_pcgbitmap.fill(0, pcgrect);
		screen.priority().fill(0, pcgrect);
		if(m_spritereg[0x404] & 0x0008)
			draw_bg(m_pcgbitmap, screen, 1, false, pcgrect);

		if(m_spritereg[0x404] & 0x0001)
			draw_bg(m_pcgbitmap, screen, 0, false, pcgrect);

		draw_sprites(m_pcgbitmap, screen, pcgrect);
	}

	int gfxprio = -1, textprio = -1, pcgprio = -1, currprio = 0;

	for(int priority = 0; priority < 3; priority++)
	{
		if((m_video.text_pri == priority) && (m_video.reg[2] & 0x0020) && !m_crtc->text_layer_buffer())
		{
			textprio = currprio;
			currprio++;
		}
		if((m_video.sprite_pri == priority) /*&& (m_spritereg[0x404] & 0x0200)*/ && (m_video.reg[2] & 0x0040))
		{
			pcgprio = currprio;
			currprio++;
		}
		if((m_video.gfx_pri == priority) && !m_crtc->gfx_layer_buffer())
		{
			gfxprio = currprio;
			currprio++;
		}
	}

	bool blend = (((m_video.reg[2] & 0x1900) == 0x1900) && (m_video.gfx_pri != 2));
	for(scanline=rect.min_y;scanline<=rect.max_y;scanline++)
	{
		for(pixel=m_crtc->hbegin();pixel<=m_crtc->hend();pixel++)
		{
			if((m_video.reg[2] & 0x1800) == 0x1000) // special priority
			{
				uint16_t colour = m_special.pix(scanline / divisor, pixel) & 0xff;
				// XXX: this might check the pen color not the palette index
				if(colour & ~1)
				{
					bitmap.pix(scanline, pixel) = m_gfxpalette->pen(colour & ~1);
					continue;
				}
			}
			rgb_t outpix = bitmap.pix(scanline, pixel), pix = 0;
			bool pcgpix = false, gfxpix = false;
			for(priority = 0; priority < 3; priority++)
			{
				if(blend)
				{
					if((priority == textprio) && !pcgpix)
					{
						uint16_t colour;
						if(get_text_pixel(scanline, pixel, &colour))
						{
							pix = m_pcgpalette->pen(colour);
							pcgpix = true;
						}
					}
					else if((priority == pcgprio) && !pcgpix)
					{
						uint16_t colour = m_pcgbitmap.pix(scanline / divisor, pixel) & 0xff;
						if(colour)
						{
							pix = m_pcgpalette->pen(colour);
							pcgpix = true;
						}
						else if(outpix && (pcgprio > textprio) && (textprio != -1))
							pix = outpix;
					}
					else if(priority == gfxprio)
						gfxpix = true;
					if((pix & 0xffffff) || ((priority == 2) && gfxpix))
					{
						if(!(pix & 0xffffff))
							pix = outpix;
						rgb_t blendpix = get_gfx_pixel<true>(scanline, pixel, gfx16bcol, pix);
						outpix = blendpix & 0xffffff ? blendpix : pix;
						break;
					}
				}
				else
				{
					if((priority == textprio) && !pcgpix)
					{
						uint16_t colour;
						if(get_text_pixel(scanline, pixel, &colour))
						{
							pix = m_pcgpalette->pen(colour);
							pcgpix = true;
						}
					}
					else if((priority == pcgprio) && !pcgpix)
					{
						uint16_t colour = m_pcgbitmap.pix(scanline / divisor, pixel) & 0xff;
						if(colour)
						{
							pix = m_pcgpalette->pen(colour);
							pcgpix = true;
						}
						else if(outpix && (pcgprio > textprio) && (textprio != -1))
							pix = outpix;
					}
					else if(priority == gfxprio)
						pix = get_gfx_pixel<false>(scanline, pixel, gfx16bcol, 0);
					if(pix & 0xffffff)
					{
						outpix = pix;
						break;
					}
				}
			}
			bitmap.pix(scanline, pixel) = outpix;
		}
	}

#ifdef MAME_DEBUG
	if(machine().input().code_pressed(KEYCODE_9))
	{
		m_sprite_shift--;
		popmessage("Sprite shift = %i",m_sprite_shift);
	}
	if(machine().input().code_pressed(KEYCODE_0))
	{
		m_sprite_shift++;
		popmessage("Sprite shift = %i",m_sprite_shift);
	}

#endif

#ifdef MAME_DEBUG
//  popmessage("Layer priorities [%04x] - Txt: %i  Spr: %i  Gfx: %i  Layer Pri0-3: %i %i %i %i",m_video.reg[1],m_video.text_pri,m_video.sprite_pri,
//      m_video.gfx_pri,m_video.gfxlayer_pri[0],m_video.gfxlayer_pri[1],m_video.gfxlayer_pri[2],m_video.gfxlayer_pri[3]);
//  popmessage("CRTC regs - %i %i %i %i  - %i %i %i %i - %i - %i",m_crtc.reg[0],m_crtc.reg[1],m_crtc.reg[2],m_crtc.reg[3],
//      m_crtc.reg[4],m_crtc.reg[5],m_crtc.reg[6],m_crtc.reg[7],m_crtc.reg[8],m_crtc.reg[9]);
//  popmessage("Visible resolution = %ix%i (%s) Screen size = %ix%i",m_crtc.visible_width,m_crtc.visible_height,m_crtc.interlace ? "Interlaced" : "Non-interlaced",m_crtc.video_width,m_crtc.video_height);
//  popmessage("VBlank : scanline = %i",m_scanline);
//  popmessage("CRTC/BG compare H-TOTAL %i/%i H-DISP %i/%i V-DISP %i/%i BG Res %02x",m_crtc.reg[0],m_spritereg[0x405],m_crtc.reg[2],m_spritereg[0x406],
//      m_crtc.reg[6],m_spritereg[0x407],m_spritereg[0x408]);
//  popmessage("BG Scroll - BG0 X %i Y %i  BG1 X %i Y %i",m_spriteram[0x400],m_spriteram[0x401],m_spriteram[0x402],m_spriteram[0x403]);
//  popmessage("uPD72065 status = %02x",upd765_status_r(machine(), space, 0));
//  popmessage("Layer enable - 0x%02x",m_video.reg[2] & 0xff);
//  popmessage("Graphic layer scroll - %i, %i - %i, %i - %i, %i - %i, %i",
//      m_crtc.reg[12],m_crtc.reg[13],m_crtc.reg[14],m_crtc.reg[15],m_crtc.reg[16],m_crtc.reg[17],m_crtc.reg[18],m_crtc.reg[19]);
//  popmessage("IOC IRQ status - %02x",m_ioc.irqstatus);
//  popmessage("RAM: mouse data - %02x %02x %02x %02x",m_ram->pointer()[0x931],m_ram->pointer()[0x930],m_ram->pointer()[0x933],m_ram->pointer()[0x932]);
#endif
	return 0;
}
