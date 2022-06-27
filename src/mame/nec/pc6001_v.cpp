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
#include "pc6001.h"

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
	uint8_t val = m_video_base [(scanline / 12) * 0x20 + pos];

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
	return m_video_base +0x0200+ (scanline / 12) * 0x20;
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
	m_video_ram = make_unique_clear<uint8_t[]>(0x4000);
	m_video_base = &m_video_ram[0];
}

void pc6001mk2_state::video_start()
{
	// ...
}

void pc6001mk2sr_state::video_start()
{
//  m_video_ram = std::make_unique<uint8_t[]>(0x4000);
	m_gvram = std::make_unique<uint8_t []>(320*256*8); // TODO: size
	std::fill_n(m_gvram.get(), 320*256*8, 0);
	save_pointer(NAME(m_gvram), 320*256*8);
}

void pc6001_state::draw_gfx_mode4(bitmap_ind16 &bitmap,const rectangle &cliprect,int attr)
{
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
	int col_setting = m_io_mode4_dsw->read() & 7;

	if((attr & 0x0c) != 0x0c)
		popmessage("Mode 4 vram attr != 0x0c, contact MESSdev");

	for(int y=0;y<192;y++)
	{
		for(int x=0;x<32;x++)
		{
			int tile = m_video_base[(x+(y*32))+0x200];

			if(col_setting == 0x00) //monochrome
			{
				for(int xi=0;xi<8;xi++)
				{
					int fgcol = (attr & 2) ? 7 : 2;

					int color = ((tile)>>(7-xi) & 1) ? fgcol : 0;

					bitmap.pix((y+24), (x*8+xi)+32) = m_palette->pen(color);
				}
			}
			else
			{
				for(int xi=0;xi<4;xi++)
				{
					int fgcol = ((tile)>>(6-(xi*2)) & 3);

					int color = (attr & 2) ? (pen_wattr[col_setting-1][fgcol]) : (pen_gattr[col_setting-1][fgcol]);

					bitmap.pix((y+24), ((x*8+xi*2)+0)+32) = m_palette->pen(color);
					bitmap.pix((y+24), ((x*8+xi*2)+1)+32) = m_palette->pen(color);
				}
			}
		}
	}
}

void pc6001_state::draw_bitmap_2bpp(bitmap_ind16 &bitmap,const rectangle &cliprect, int attr)
{
	int shrink_x = 2*4;
	int shrink_y = (attr & 8) ? 1 : 2;
	int w = (shrink_x == 8) ? 32 : 16;
	int col_bank = ((attr & 2)<<1);

	if(attr & 4)
	{
		for(int y=0;y<(192/shrink_y);y++)
		{
			for(int x=0;x<w;x++)
			{
				int tile = m_video_base[(x+(y*32))+0x200];

				for(int yi=0;yi<shrink_y;yi++)
				{
					for(int xi=0;xi<shrink_x;xi++)
					{
						int i = (shrink_x == 8) ? (xi & 0x06) : (xi & 0x0c)>>1;
						int color = ((tile >> i) & 3)+8;
						color+= col_bank;

						bitmap.pix(((y*shrink_y+yi)+24), (x*shrink_x+((shrink_x-1)-xi))+32) = m_palette->pen(color);
					}
				}
			}
		}
	}
	else /* TODO: clean this up */
	{
		for(int y=0;y<(192/shrink_y);y+=3)
		{
			for(int x=0;x<w;x++)
			{
				int tile = m_video_base[(x+((y/3)*32))+0x200];

				for(int yi=0;yi<shrink_y;yi++)
				{
					for(int xi=0;xi<shrink_x;xi++)
					{
						int i = (shrink_x == 8) ? (xi & 0x06) : (xi & 0x0c)>>1;
						int color = ((tile >> i) & 3)+8;
						color+= col_bank;

						bitmap.pix((((y+0)*shrink_y+yi)+24), (x*shrink_x+((shrink_x-1)-xi))+32) = m_palette->pen(color);
						bitmap.pix((((y+1)*shrink_y+yi)+24), (x*shrink_x+((shrink_x-1)-xi))+32) = m_palette->pen(color);
						bitmap.pix((((y+2)*shrink_y+yi)+24), (x*shrink_x+((shrink_x-1)-xi))+32) = m_palette->pen(color);
					}
				}
			}
		}
	}
}

void pc6001_state::draw_tile_3bpp(bitmap_ind16 &bitmap,const rectangle &cliprect,int x,int y,int tile,int attr)
{
	int pen;
	if(attr & 0x10) //2x2 squares on a single cell
		pen = (tile & 0x70)>>4;
	else //2x3
		pen = (tile & 0xc0) >> 6 | (attr & 2)<<1;

	for(int yi=0;yi<12;yi++)
	{
		for(int xi=0;xi<8;xi++)
		{
			int i = (xi & 4)>>2; //x-axis
			if(attr & 0x10) //2x2
			{
				i+= (yi >= 6) ? 2 : 0; //y-axis
			}
			else //2x3
			{
				i+= (yi & 4)>>1; //y-axis 1
				i+= (yi & 8)>>1; //y-axis 2
			}

			int color = ((tile >> i) & 1) ? pen+8 : 0;

			bitmap.pix(((y*12+(11-yi))+24), (x*8+(7-xi))+32) = m_palette->pen(color);
		}
	}
}

void pc6001_state::draw_tile_text(bitmap_ind16 &bitmap,const rectangle &cliprect,int x,int y,int tile,int attr,int has_mc6847)
{
	uint8_t const *const gfx_data = m_region_gfx1->base();

	for(int yi=0;yi<12;yi++)
	{
		for(int xi=0;xi<8;xi++)
		{
			int pen = gfx_data[(tile*0x10)+yi]>>(7-xi) & 1;

			int fgcol,color;
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

			bitmap.pix(((y*12+yi)+24), (x*8+xi)+32) = m_palette->pen(color);
		}
	}
}

void pc6001_state::draw_border(bitmap_ind16 &bitmap,const rectangle &cliprect,int attr,int has_mc6847)
{
	for(int y=0;y<240;y++)
	{
		for(int x=0;x<320;x++)
		{
			int color;
			if(!has_mc6847) //mk2 border color is always black
				color = 0;
			else if((attr & 0x90) == 0x80) //2bpp
				color = ((attr & 2)<<1) + 8;
			else if((attr & 0x90) == 0x90) //1bpp
				color = (attr & 2) ? 7 : 2;
			else
				color = 0; //FIXME: other modes not yet checked

			bitmap.pix(y, x) = m_palette->pen(color);
		}
	}
}

void pc6001_state::pc6001_screen_draw(bitmap_ind16 &bitmap,const rectangle &cliprect, int has_mc6847)
{
	int attr = m_video_base[0];

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
		for(int y=0;y<16;y++)
		{
			for(int x=0;x<32;x++)
			{
				int tile = m_video_base[(x+(y*32))+0x200];
				attr = m_video_base[(x+(y*32)) & 0x1ff];

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

uint32_t pc6001_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	pc6001_screen_draw(bitmap,cliprect,1);

	return 0;
}

uint32_t pc6001mk2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* note: bitmap mode have priority over everything else, check American Truck */
	if(m_exgfx_bitmap_mode)
	{
		int count = 0;

		for(int y=0;y<200;y++)
		{
			for(int x=0;x<160;x+=4)
			{
				for(int i=0;i<4;i++)
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

					pen[0] = m_video_base[count+0x0000] >> (6-i*2) & 3;
					pen[1] = m_video_base[count+0x2000] >> (6-i*2) & 3;

					int color = 0x10;
					color |= ((pen[0] & 1) << 2);
					color |= ((pen[0] & 2) >> 1);
					color |= ((pen[1] & 1) << 1);
					color |= ((pen[1] & 2) << 2);

					if (cliprect.contains((x+i)*2+0, y))
						bitmap.pix(y, (x+i)*2+0) = m_palette->pen(color);
					if (cliprect.contains((x+i)*2+1, y))
						bitmap.pix(y, (x+i)*2+1) = m_palette->pen(color);
				}

				count++;
			}
		}
	}
	else if(m_exgfx_2bpp_mode)
	{
		int count = 0;

		for(int y=0;y<200;y++)
		{
			for(int x=0;x<320;x+=8)
			{
				for(int i=0;i<8;i++)
				{
					int pen[2];
#if 0
					/* palette reference: */
					static const uint8_t pal_num[] = { 0x00, 0x04, 0x01, 0x05 };

					color |= pal_num[(pen[0] & 1) | ((pen[1] & 1) << 1)];
#endif

					pen[0] = m_video_base[count+0x0000] >> (7-i) & 1;
					pen[1] = m_video_base[count+0x2000] >> (7-i) & 1;

					int color;
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
						bitmap.pix(y, (x+i)) = m_palette->pen(color);
				}

				count++;
			}
		}

	}
	else if(m_exgfx_text_mode)
	{
		uint8_t const *const gfx_data = m_region_gfx1->base();

		for(int y=0;y<20;y++)
		{
			for(int x=0;x<40;x++)
			{
				/*
				exgfx attr format:
				x--- ---- rom bank select
				-xxx ---- bg color
				---- xxxx fg color
				Note that the exgfx banks a different gfx ROM
				*/
				int tile = m_video_base[(x+(y*40))+0x400] + 0x200;
				int attr = m_video_base[(x+(y*40)) & 0x3ff];
				tile += ((attr & 0x80) << 1);

				for(int yi=0;yi<12;yi++)
				{
					for(int xi=0;xi<8;xi++)
					{
						int pen = gfx_data[(tile*0x10)+yi]>>(7-xi) & 1;

						int fgcol = (attr & 0x0f) + 0x10;
						int bgcol = ((attr & 0x70) >> 4) + 0x10 + ((m_bgcol_bank & 2) << 2);

						int color = pen ? fgcol : bgcol;

						if (cliprect.contains(x*8+xi, y*12+yi))
							bitmap.pix(((y*12+yi)), (x*8+xi)) = m_palette->pen(color);
					}
				}
			}
		}
	}
	else
	{
		//attr = m_video_base[0];
		pc6001_screen_draw(bitmap,cliprect,0);
	}

	return 0;
}

uint32_t pc6001mk2sr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const gfx_data = m_region_gfx1->base();

	bitmap.fill(0,cliprect);

	if(m_sr_text_mode == true) // text mode
	{
		const u8 text_cols = (m_width80 + 1) * 40;

		for(int y = 0; y < m_sr_text_rows; y++)
		{
			for(int x = 0; x < text_cols; x++)
			{
				int tile = m_video_base[(x + (y * text_cols)) * 2 + 0];
				int attr = m_video_base[(x + (y * text_cols)) * 2 + 1];
				tile += ((attr & 0x80) << 1);

				for(int yi = 0; yi < 12; yi++)
				{
					int res_y = y * 12 + yi;

					for(int xi = 0; xi < 8; xi++)
					{
						int res_x = x * 8 + xi;

						int pen = gfx_data[(tile * 0x10) + yi] >> (7 - xi) & 1;

						int fgcol = (attr & 0x0f) + 0x10;
						// TODO: definitely wants bright colors for N66SR BASIC, but quite won't work for "PC-6*01 World" screens
						// (can't pinpoint banking on this HW, or maybe it's side effect of CLUT?)
						int bgcol = ((attr & 0x70) >> 4) + 0x18; //+ m_bgcol_bank;

						int color = pen ? fgcol : bgcol;

						if (cliprect.contains(res_x, res_y))
							bitmap.pix(res_y, res_x) = m_palette->pen(color);
					}
				}
			}
		}
	}
	else
	{
		//4bpp bitmap mode
		const u32 scroll_x = (m_sr_scrollx[0]) + (m_sr_scrollx[1] << 8);
		const u32 scroll_y = m_sr_scrolly[0];
		const int x_pitch = 320;
		const int y_pitch = 204;

		//popmessage("%04x %02x", scroll_x, scroll_y);

		for(int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			for(int x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				uint32_t vram_addr;

				// The Jp emulators maps this for the rightmost X > 256, but it doesn't seem to be the case?
//              vram_addr = 0x1a00 + (x-256)+y*64;

				// TODO: scrolling is preliminary, based off how Pakuridius sets VRAM and scroll regs
				// It seems to wraparound at 320x204
				// Title screen scrolling usage is quite jerky, but it sorta makes sense on gameplay ...
				vram_addr = ((x + scroll_x) % x_pitch) + ((y + scroll_y) % y_pitch) * x_pitch;

				// wants RGB -> BRG rotation
				// (is it using a different palette bank?)
				u8 color = bitswap<4>(m_gvram[vram_addr] & 0x0f, 3, 0, 2, 1) + 0x10;
				if (cliprect.contains(x, y))
					bitmap.pix(y, x) = m_palette->pen(color);
			}
		}
	}

	return 0;
}
