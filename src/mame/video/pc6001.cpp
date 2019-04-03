// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*******************************************************************
 *
 * PC-6xxx video related functions
 *
 *
 *
 ******************************************************************/

#include "emu.h"
#include "includes/pc6001.h"

/*****************************************
 *
 * Palette Inits
 *
 ****************************************/

static constexpr rgb_t defcolors[] =
{
	rgb_t(0x07, 0xff, 0x00), // GREEN
	rgb_t(0xff, 0xff, 0x00), // YELLOW
	rgb_t(0x3b, 0x08, 0xff), // BLUE
	rgb_t(0xcc, 0x00, 0x3b), // RED
	rgb_t(0xff, 0xff, 0xff), // BUFF
	rgb_t(0x07, 0xe3, 0x99), // CYAN
	rgb_t(0xff, 0x1c, 0xff), // MAGENTA
	rgb_t(0xff, 0x81, 0x00), // ORANGE

	// MC6847 specific
	rgb_t(0x00, 0x7c, 0x00), // ALPHANUMERIC DARK GREEN
	rgb_t(0x07, 0xff, 0x00), // ALPHANUMERIC BRIGHT GREEN
	rgb_t(0x91, 0x00, 0x00), // ALPHANUMERIC DARK ORANGE
	rgb_t(0xff, 0x81, 0x00)  // ALPHANUMERIC BRIGHT ORANGE
};

static constexpr rgb_t mk2_defcolors[] =
{
	rgb_t(0x00, 0x00, 0x00), // BLACK
	rgb_t(0xff, 0xaf, 0x00), // ORANGE
	rgb_t(0x00, 0xff, 0xaf), // tone of GREEN
	rgb_t(0xaf, 0xff, 0x00), // tone of GREEN
	rgb_t(0xaf, 0x00, 0xff), // VIOLET
	rgb_t(0xff, 0x00, 0xaf), // SCARLET
	rgb_t(0x00, 0xaf, 0xff), // LIGHT BLUE
	rgb_t(0xaf, 0xaf, 0xaf), // GRAY
	rgb_t(0x00, 0x00, 0x00), // BLACK
	rgb_t(0xff, 0x00, 0x00), // RED
	rgb_t(0x00, 0xff, 0x00), // GREEN
	rgb_t(0xff, 0xff, 0x00), // YELLOW
	rgb_t(0x00, 0x00, 0xff), // BLUE
	rgb_t(0xff, 0x00, 0xff), // PINK
	rgb_t(0x00, 0xff, 0xff), // CYAN
	rgb_t(0xff, 0xff, 0xff)  // WHITE
};

void pc6001_state::pc6001_palette(palette_device &palette) const
{
	for(int i=0;i<8+4;i++)
		palette.set_pen_color(i+8,defcolors[i]);
}

void pc6001mk2_state::pc6001mk2_palette(palette_device &palette) const
{
	for(int i=0;i<8;i++)
		palette.set_pen_color(i+8,defcolors[i]);

	for(int i=0x10;i<0x20;i++)
		palette.set_pen_color(i,mk2_defcolors[i-0x10]);
}

/*****************************************
 *
 * Video functions
 *
 ****************************************/

 // MC6847 old interfacing code
#ifdef UNUSED_FUNCTION
ATTR_CONST pc6001_state::uint8_t pc6001_get_attributes(uint8_t c,int scanline, int pos)
{
	uint8_t result = 0x00;
	uint8_t val = m_video_ram [(scanline / 12) * 0x20 + pos];

	if (val & 0x01) {
		result |= M6847_INV;
	}
	if (val & 0x40)
		result |= M6847_AG | M6847_GM1; //TODO

	result |= M6847_INTEXT; // always use external ROM
	return result;
}

const pc6001_state::uint8_t *pc6001_get_video_ram(int scanline)
{
	return m_video_ram +0x0200+ (scanline / 12) * 0x20;
}

uint8_t pc6001_state::pc6001_get_char_rom(uint8_t ch, int line)
{
	uint8_t *gfx = m_region_gfx1->base();
	return gfx[ch*16+line];
}
#endif

void pc6001_state::video_start()
{
	#if 0
	m6847_config cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.type = M6847_VERSION_M6847T1_NTSC;
	cfg.get_attributes = pc6001_get_attributes;
	cfg.get_video_ram = pc6001_get_video_ram;
	cfg.get_char_rom = pc6001_get_char_rom;
	m6847_init(machine(), &cfg);
	#endif
	m_video_ram = auto_alloc_array_clear(machine(), uint8_t, 0x4000);
}

void pc6001mk2_state::video_start()
{
	// ...
}

void pc6001sr_state::video_start()
{
//  m_video_ram = auto_alloc_array_clear(machine(), uint8_t, 0x4000);
	m_gvram = auto_alloc_array_clear(machine(), uint8_t, 320*256*8); // TODO: size
	save_pointer(NAME(m_gvram), 320*256*8);
}

/* this is known as gfx mode 4 */
void pc6001_state::draw_gfx_mode4(bitmap_ind16 &bitmap,const rectangle &cliprect,int attr)
{
	int x,y,xi;
	int fgcol,color;
	int col_setting;
	static const uint8_t pen_gattr[4][4] = {
		{ 0, 1, 6, 2 }, //Red / Blue
		{ 0, 6, 1, 2 }, //Blue / Red
		{ 0, 5, 2, 2 }, //Pink / Green
		{ 0, 2, 5, 2 }, //Green / Pink
	};
	static const uint8_t pen_wattr[4][4] = {
		{ 0, 1, 6, 7 }, //Red / Blue
		{ 0, 6, 1, 7 }, //Blue / Red
		{ 0, 5, 2, 7 }, //Pink / Green
		{ 0, 2, 5, 7 }, //Green / Pink
	};
	col_setting = m_io_mode4_dsw->read() & 7;

	if((attr & 0x0c) != 0x0c)
		popmessage("Mode 4 vram attr != 0x0c, contact MESSdev");

	for(y=0;y<192;y++)
	{
		for(x=0;x<32;x++)
		{
			int tile = m_video_ram[(x+(y*32))+0x200];

			if(col_setting == 0x00) //monochrome
			{
				for(xi=0;xi<8;xi++)
				{
					fgcol = (attr & 2) ? 7 : 2;

					color = ((tile)>>(7-xi) & 1) ? fgcol : 0;

					bitmap.pix16((y+24), (x*8+xi)+32) = m_palette->pen(color);
				}
			}
			else
			{
				for(xi=0;xi<4;xi++)
				{
					fgcol = ((tile)>>(6-(xi*2)) & 3);

					color = (attr & 2) ? (pen_wattr[col_setting-1][fgcol]) : (pen_gattr[col_setting-1][fgcol]);

					bitmap.pix16((y+24), ((x*8+xi*2)+0)+32) = m_palette->pen(color);
					bitmap.pix16((y+24), ((x*8+xi*2)+1)+32) = m_palette->pen(color);
				}
			}
		}
	}
}

void pc6001_state::draw_bitmap_2bpp(bitmap_ind16 &bitmap,const rectangle &cliprect, int attr)
{
	int color,x,y,xi,yi;

	int shrink_x = 2*4;
	int shrink_y = (attr & 8) ? 1 : 2;
	int w = (shrink_x == 8) ? 32 : 16;
	int col_bank = ((attr & 2)<<1);

	if(attr & 4)
	{
		for(y=0;y<(192/shrink_y);y++)
		{
			for(x=0;x<w;x++)
			{
				int tile = m_video_ram[(x+(y*32))+0x200];

				for(yi=0;yi<shrink_y;yi++)
				{
					for(xi=0;xi<shrink_x;xi++)
					{
						int i;
						i = (shrink_x == 8) ? (xi & 0x06) : (xi & 0x0c)>>1;
						color = ((tile >> i) & 3)+8;
						color+= col_bank;

						bitmap.pix16(((y*shrink_y+yi)+24), (x*shrink_x+((shrink_x-1)-xi))+32) = m_palette->pen(color);
					}
				}
			}
		}
	}
	else /* TODO: clean this up */
	{
		for(y=0;y<(192/shrink_y);y+=3)
		{
			for(x=0;x<w;x++)
			{
				int tile = m_video_ram[(x+((y/3)*32))+0x200];

				for(yi=0;yi<shrink_y;yi++)
				{
					for(xi=0;xi<shrink_x;xi++)
					{
						int i;
						i = (shrink_x == 8) ? (xi & 0x06) : (xi & 0x0c)>>1;
						color = ((tile >> i) & 3)+8;
						color+= col_bank;

						bitmap.pix16((((y+0)*shrink_y+yi)+24), (x*shrink_x+((shrink_x-1)-xi))+32) = m_palette->pen(color);
						bitmap.pix16((((y+1)*shrink_y+yi)+24), (x*shrink_x+((shrink_x-1)-xi))+32) = m_palette->pen(color);
						bitmap.pix16((((y+2)*shrink_y+yi)+24), (x*shrink_x+((shrink_x-1)-xi))+32) = m_palette->pen(color);
					}
				}
			}
		}
	}
}

void pc6001_state::draw_tile_3bpp(bitmap_ind16 &bitmap,const rectangle &cliprect,int x,int y,int tile,int attr)
{
	int color,pen,xi,yi;

	if(attr & 0x10) //2x2 squares on a single cell
		pen = (tile & 0x70)>>4;
	else //2x3
		pen = (tile & 0xc0) >> 6 | (attr & 2)<<1;

	for(yi=0;yi<12;yi++)
	{
		for(xi=0;xi<8;xi++)
		{
			int i;
			i = (xi & 4)>>2; //x-axis
			if(attr & 0x10) //2x2
			{
				i+= (yi >= 6) ? 2 : 0; //y-axis
			}
			else //2x3
			{
				i+= (yi & 4)>>1; //y-axis 1
				i+= (yi & 8)>>1; //y-axis 2
			}

			color = ((tile >> i) & 1) ? pen+8 : 0;

			bitmap.pix16(((y*12+(11-yi))+24), (x*8+(7-xi))+32) = m_palette->pen(color);
		}
	}
}

void pc6001_state::draw_tile_text(bitmap_ind16 &bitmap,const rectangle &cliprect,int x,int y,int tile,int attr,int has_mc6847)
{
	int xi,yi,pen,fgcol,color;
	uint8_t *gfx_data = m_region_gfx1->base();

	for(yi=0;yi<12;yi++)
	{
		for(xi=0;xi<8;xi++)
		{
			pen = gfx_data[(tile*0x10)+yi]>>(7-xi) & 1;

			if(has_mc6847)
			{
				fgcol = (attr & 2) ? 0x12 : 0x10;

				if(attr & 1)
					color = pen ? (fgcol+0) : (fgcol+1);
				else
					color = pen ? (fgcol+1) : (fgcol+0);

			}
			else
			{
				fgcol = (attr & 2) ? 2 : 7;

				if(attr & 1)
					color = pen ? 0 : fgcol;
				else
					color = pen ? fgcol : 0;
			}

			bitmap.pix16(((y*12+yi)+24), (x*8+xi)+32) = m_palette->pen(color);
		}
	}
}

void pc6001_state::draw_border(bitmap_ind16 &bitmap,const rectangle &cliprect,int attr,int has_mc6847)
{
	int x,y,color;

	for(y=0;y<240;y++)
	{
		for(x=0;x<320;x++)
		{
			if(!has_mc6847) //mk2 border color is always black
				color = 0;
			else if((attr & 0x90) == 0x80) //2bpp
				color = ((attr & 2)<<1) + 8;
			else if((attr & 0x90) == 0x90) //1bpp
				color = (attr & 2) ? 7 : 2;
			else
				color = 0; //FIXME: other modes not yet checked

			bitmap.pix16(y, x) = m_palette->pen(color);
		}
	}
}

void pc6001_state::pc6001_screen_draw(bitmap_ind16 &bitmap,const rectangle &cliprect, int has_mc6847)
{
	int x,y;
	int tile,attr;

	attr = m_video_ram[0];

	draw_border(bitmap,cliprect,attr,has_mc6847);

	if(attr & 0x80) // gfx mode
	{
		if(attr & 0x10) // 256x192x1 mode (FIXME: might be a different trigger)
		{
			draw_gfx_mode4(bitmap,cliprect,attr);
		}
		else // 128x192x2 mode
		{
			draw_bitmap_2bpp(bitmap,cliprect,attr);
		}
	}
	else // text mode
	{
		for(y=0;y<16;y++)
		{
			for(x=0;x<32;x++)
			{
				tile = m_video_ram[(x+(y*32))+0x200];
				attr = m_video_ram[(x+(y*32)) & 0x1ff];

				if(attr & 0x40)
				{
					draw_tile_3bpp(bitmap,cliprect,x,y,tile,attr);
				}
				else
				{
					draw_tile_text(bitmap,cliprect,x,y,tile,attr,has_mc6847);
				}
			}
		}
	}
}

uint32_t pc6001_state::screen_update_pc6001(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	pc6001_screen_draw(bitmap,cliprect,1);

	return 0;
}

uint32_t pc6001mk2_state::screen_update_pc6001mk2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,tile,attr;

	/* note: bitmap mode have priority over everything else, check American Truck */
	if(m_exgfx_bitmap_mode)
	{
		int count,color,i;

		count = 0;

		for(y=0;y<200;y++)
		{
			for(x=0;x<160;x+=4)
			{
				for(i=0;i<4;i++)
				{
					int pen[2];
#if 0
					/* palette reference: */
					static const uint8_t pal_num[] = { 0x00, 0x04, 0x01, 0x05,
						0x02, 0x06, 0x03, 0x07,
						0x08, 0x0c, 0x09, 0x0d,
						0x0a, 0x0e, 0x0b, 0x0f };

					color |= pal_num[(pen[0] & 3) | ((pen[1] & 3) << 2)];
#endif

					pen[0] = m_video_ram[count+0x0000] >> (6-i*2) & 3;
					pen[1] = m_video_ram[count+0x2000] >> (6-i*2) & 3;

					color = 0x10;
					color |= ((pen[0] & 1) << 2);
					color |= ((pen[0] & 2) >> 1);
					color |= ((pen[1] & 1) << 1);
					color |= ((pen[1] & 2) << 2);

					if (cliprect.contains((x+i)*2+0, y))
						bitmap.pix16(y, (x+i)*2+0) = m_palette->pen(color);
					if (cliprect.contains((x+i)*2+1, y))
						bitmap.pix16(y, (x+i)*2+1) = m_palette->pen(color);
				}

				count++;
			}
		}
	}
	else if(m_exgfx_2bpp_mode)
	{
		int count,color,i;

		count = 0;

		for(y=0;y<200;y++)
		{
			for(x=0;x<320;x+=8)
			{
				for(i=0;i<8;i++)
				{
					int pen[2];
#if 0
					/* palette reference: */
					static const uint8_t pal_num[] = { 0x00, 0x04, 0x01, 0x05 };

					color |= pal_num[(pen[0] & 1) | ((pen[1] & 1) << 1)];
#endif

					pen[0] = m_video_ram[count+0x0000] >> (7-i) & 1;
					pen[1] = m_video_ram[count+0x2000] >> (7-i) & 1;

					if(m_bgcol_bank & 4) //PC-6001 emulation mode
					{
						color = 0x08;
						color |= (pen[0]) | (pen[1]<<1);
						color |= (m_bgcol_bank & 1) << 2;
					}
					else //Mk-2 mode
					{
						color = 0x10;
						color |= ((pen[0] & 1) << 2);
						color |= ((pen[1] & 1) >> 0);
						color |= ((m_bgcol_bank & 1) << 1);
						color |= ((m_bgcol_bank & 2) << 2);
					}

					if (cliprect.contains(x+i, y))
						bitmap.pix16(y, (x+i)) = m_palette->pen(color);
				}

				count++;
			}
		}

	}
	else if(m_exgfx_text_mode)
	{
		int xi,yi,pen,fgcol,bgcol,color;
		uint8_t *gfx_data = m_region_gfx1->base();

		for(y=0;y<20;y++)
		{
			for(x=0;x<40;x++)
			{
				/*
				exgfx attr format:
				x--- ---- rom bank select
				-xxx ---- bg color
				---- xxxx fg color
				Note that the exgfx banks a different gfx ROM
				*/
				tile = m_video_ram[(x+(y*40))+0x400] + 0x200;
				attr = m_video_ram[(x+(y*40)) & 0x3ff];
				tile+= ((attr & 0x80) << 1);

				for(yi=0;yi<12;yi++)
				{
					for(xi=0;xi<8;xi++)
					{
						pen = gfx_data[(tile*0x10)+yi]>>(7-xi) & 1;

						fgcol = (attr & 0x0f) + 0x10;
						bgcol = ((attr & 0x70) >> 4) + 0x10 + ((m_bgcol_bank & 2) << 2);

						color = pen ? fgcol : bgcol;

						if (cliprect.contains(x*8+xi, y*12+yi))
							bitmap.pix16(((y*12+yi)), (x*8+xi)) = m_palette->pen(color);
					}
				}
			}
		}
	}
	else
	{
		//attr = m_video_ram[0];
		pc6001_screen_draw(bitmap,cliprect,0);
	}

	return 0;
}

uint32_t pc6001sr_state::screen_update_pc6001sr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,tile,attr;
	int xi,yi,pen,fgcol,bgcol,color;
	uint8_t *gfx_data = m_region_gfx1->base();

	bitmap.fill(0,cliprect);

	if(m_sr_text_mode == true) // text mode
	{
		for(y=0;y<m_sr_text_rows;y++)
		{
			for(x=0;x<40;x++)
			{
				tile = m_video_ram[(x+(y*40))*2+0];
				attr = m_video_ram[(x+(y*40))*2+1];
				tile+= ((attr & 0x80) << 1);

				for(yi=0;yi<12;yi++)
				{
					for(xi=0;xi<8;xi++)
					{
						pen = gfx_data[(tile*0x10)+yi]>>(7-xi) & 1;

						fgcol = (attr & 0x0f) + 0x10;
						bgcol = ((attr & 0x70) >> 4) + 0x10 + ((m_bgcol_bank & 2) << 2);

						color = pen ? fgcol : bgcol;

						if (cliprect.contains(x*8+xi, y*12+yi))
							bitmap.pix16(((y*12+yi)), (x*8+xi)) = m_palette->pen(color);
					}
				}
			}
		}
	}
	else //4bpp bitmap mode (TODO)
	{
		for(y=0;y<200;y++)
		{
			for(x=0;x<320;x+=2)
			{
				uint32_t vram_addr;

				if(x >= 256)
					vram_addr = 0x1a00 + (x-256)+y*64;
				else
					vram_addr = x+y*256;

				color = (m_gvram[vram_addr] & 0xf0) >> 4;

				if (cliprect.contains(x, y+0))
					bitmap.pix16((y+0), (x+0)) = m_palette->pen(color+0x10);

				color = (m_gvram[vram_addr] & 0x0f);

				if (cliprect.contains(x+1, y+0))
					bitmap.pix16((y+0), (x+1)) = m_palette->pen(color+0x10);
			}
		}
	}

	return 0;
}
