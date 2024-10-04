// license:BSD-3-Clause
// copyright-holders:Angelo Salese,Carl
/*******************************************************************
 *
 * PC98xx video related functions
 *
 * TODO:
 * - further break-down into specific sub-parts/devices for GRCG / EGC;
 *
 ******************************************************************/

#include "emu.h"
#include "pc9801.h"


void pc9801_state::video_start()
{
	m_tvram = std::make_unique<uint16_t[]>(0x2000);

	// find memory regions
	m_char_rom = memregion("chargen")->base();
	m_kanji_rom = memregion("kanji")->base();

	std::fill(std::begin(m_ex_video_ff), std::end(m_ex_video_ff), 0);
	std::fill(std::begin(m_video_ff), std::end(m_video_ff), 0);
}

uint32_t pc9801_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	/* graphics */
	if(m_video_ff[DISPLAY_REG] != 0)
		m_hgdc[1]->screen_update(screen, bitmap, cliprect);
	m_hgdc[0]->screen_update(screen, bitmap, cliprect);

	return 0;
}

/*************************************************
 *
 * UPD7220 (GDC2) bitmap layer
 *
 ************************************************/


UPD7220_DISPLAY_PIXELS_MEMBER( pc9801_state::hgdc_display_pixels )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	uint8_t colors16_mode = (m_ex_video_ff[ANALOG_16_MODE]) ? 16 : 8;

	for(int xi=0;xi<16;xi++)
	{
		int res_x = x + xi;
		int res_y = y;

		uint8_t pen;

		pen = ((m_video_ram[1][((address & 0x3fff) + (0x4000) + ((m_vram_disp*0x20000)>>1))] >> xi) & 1) ? 1 : 0;
		pen|= ((m_video_ram[1][((address & 0x3fff) + (0x8000) + ((m_vram_disp*0x20000)>>1))] >> xi) & 1) ? 2 : 0;
		pen|= ((m_video_ram[1][((address & 0x3fff) + (0xc000) + ((m_vram_disp*0x20000)>>1))] >> xi) & 1) ? 4 : 0;
		if(m_ex_video_ff[ANALOG_16_MODE])
			pen|= ((m_video_ram[1][((address & 0x3fff) + (0) + ((m_vram_disp*0x20000)>>1))] >> xi) & 1) ? 8 : 0;
		bitmap.pix(res_y, res_x) = palette[pen + colors16_mode];
	}
}

/*************************************************
 *
 * UPD7220 (GDC1) text layer
 *
 ************************************************/

UPD7220_DRAW_TEXT_LINE_MEMBER( pc9801_state::hgdc_draw_text )
{
	draw_text(bitmap, addr, y, wd, pitch, lr, cursor_on, cursor_addr, false);
}

void pc9801_state::draw_text(bitmap_rgb32 &bitmap, uint32_t addr, int y, int wd, int pitch, int lr, int cursor_on, int cursor_addr, bool lower)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	if(m_video_ff[DISPLAY_REG] == 0) //screen is off
		return;

//  uint8_t interlace_on = m_video_ff[INTERLACE_REG];
	uint8_t char_size = m_video_ff[FONTSEL_REG] ? 16 : 8;

	uint8_t x_step;
	uint8_t lastul = 0;

	int scroll_start = 33 - (m_txt_scroll_reg[4] & 0x1f);
	int scroll_end = scroll_start + m_txt_scroll_reg[5];
	int scroll = m_txt_scroll_reg[3] % 20;
	int line = y / lr;

	for(int x=0;x<pitch;x+=x_step)
	{
		uint32_t tile_addr = addr+(x*(m_video_ff[WIDTH40_REG]+1));

		uint8_t kanji_sel = 0;
		uint8_t kanji_lr = 0;
		uint8_t tile_lr = 0;

		uint16_t tile = m_video_ram[0][tile_addr & 0xfff] & 0xff;
		uint8_t knj_tile = m_video_ram[0][tile_addr & 0xfff] >> 8;
		if(knj_tile)
		{
			/* Note: bit 7 doesn't really count, if a kanji is enabled then the successive tile is always the second part of it.
			   Trusted with Alice no Yakata, Animahjong V3, Aki no Tsukasa no Fushigi no Kabe, Apros ...
			*/
			//kanji_lr = (knj_tile & 0x80) >> 7;
			//kanji_lr |= (tile & 0x80) >> 7; // Tokimeki Sports Gal 3
			tile &= 0x7f;
			tile <<= 8;
			tile |= (knj_tile & 0x7f);
			kanji_sel = 1;
			if((tile & 0xfe00) == 0x5600)
			{
				tile_lr = knj_tile & 0x80 ? 1 : 0;
				x_step = 1;
			}
			else if((tile & 0x7c00) == 0x0800)  // 8x16 charset selector
				x_step = 1;
			else
				x_step = 2;
//          kanji_lr = 0;
		}
		else
			x_step = 1;

		for(kanji_lr=0;kanji_lr<x_step;kanji_lr++)
		{
			/* Rori Rori Rolling definitely uses different colors for brake stop PCG elements,
			   assume that all attributes are recalculated on different strips */
			uint8_t attr = (m_video_ram[0][((tile_addr+kanji_lr+tile_lr) & 0xfff) | 0x1000] & 0xff);

			uint8_t secret = (attr & 1) ^ 1;
			uint8_t blink = attr & 2;
			uint8_t reverse = attr & 4;
			uint8_t u_line = attr & 8;
			uint8_t v_line = (m_video_ff[ATTRSEL_REG]) ? 0 : attr & 0x10;
			uint8_t gfx_mode = (m_video_ff[ATTRSEL_REG]) ? attr & 0x10 : 0;
			uint8_t color = (attr & 0xe0) >> 5;

			for(int yi=0;yi<lr;yi++)
			{
				int res_y = y + yi;
				if((line >= scroll_start) && (line <= scroll_end))
				{
					if(!lower)
					{
						if(!yi)
							yi = scroll;
						else
							res_y -= scroll;
					}
					else if(lower)
					{
						if(yi >= scroll)
							break;
						res_y += lr - scroll;
					}
				}

				for(int xi=0;xi<8;xi++)
				{
					int res_x = ((x+kanji_lr)*8+xi) * (m_video_ff[WIDTH40_REG]+1);

					if(!m_screen->visible_area().contains(res_x, res_y))
						continue;

					uint8_t tile_data = 0;

					if(!secret)
					{
						/* TODO: priority */
						if(gfx_mode)
						{
							tile_data = 0;

							/*
							    gfx strip mode:

							    number refers to the bit number in the tile data.
							    This mode is identical to the one seen in PC-8801
							    00004444
							    11115555
							    22226666
							    33337777
							*/

							int gfx_bit;
							gfx_bit = (xi & 4);
							gfx_bit+= (yi & (2 << (char_size == 16 ? 0x01 : 0x00)))>>(1+(char_size == 16));
							gfx_bit+= (yi & (4 << (char_size == 16 ? 0x01 : 0x00)))>>(1+(char_size == 16));

							tile_data = ((tile >> gfx_bit) & 1) ? 0xff : 0x00;
						}
						else if(kanji_sel)
							tile_data = (m_kanji_rom[tile*0x20+yi*2+kanji_lr+tile_lr]);
						else
							tile_data = (m_char_rom[tile*char_size+m_video_ff[FONTSEL_REG]*0x800+yi]);
					}

					if(yi == lr-1)
					{
						if(u_line) tile_data = 0x0f;
						if(lastul) tile_data |= 0xf0;
					}
					if(v_line)  { tile_data|=8; }

					/* TODO: proper blink rate for these two */
					if(cursor_on && cursor_addr == tile_addr && m_screen->frame_number() & 0x10)
						tile_data^=0xff;

					if(blink && m_screen->frame_number() & 0x10)
						tile_data = 0;

					if(reverse) { tile_data^=0xff; }

					int pen;
					if(yi >= char_size)
						pen = -1;
					else
						pen = (tile_data >> (7-xi) & 1) ? color : -1;

					if(pen != -1)
						bitmap.pix(res_y, res_x) = palette[pen];

					if(m_video_ff[WIDTH40_REG])
					{
						if(!m_screen->visible_area().contains(res_x+1, res_y))
							continue;

						if(pen != -1)
							bitmap.pix(res_y, res_x+1) = palette[pen];
					}
				}
			}
			lastul = u_line;
		}
	}
	if(scroll && !lower && (line >= scroll_start) && (line <= scroll_end))
		return draw_text(bitmap, addr += pitch, y, wd, pitch, lr, cursor_on, cursor_addr, true);
}

/*************************************************
 *
 * Flip Flop registers
 *
 ************************************************/

void pc9801_state::pc9801_video_ff_w(uint8_t data)
{
	/*
	TODO: this is my best bet so far. Register 4 is annoying, the pattern seems to be:
	Write to video FF register Graphic -> 00
	Write to video FF register 200 lines -> 0x
	Write to video FF register 200 lines -> 00

	where x is the current mode.
	*/
	switch((data & 0x0e) >> 1)
	{
		case 1:
			m_gfx_ff = 1;
			if(data & 1)
				logerror("Graphic f/f actually enabled!\n");
			break;
		case 4:
			if(m_gfx_ff)
			{
				m_video_ff[(data & 0x0e) >> 1] = data &1;
				m_gfx_ff = 0;
			}
			break;
		default: m_video_ff[(data & 0x0e) >> 1] = data & 1; break;
	}

	if(0)
	{
		static const char *const video_ff_regnames[] =
		{
			"Attribute Select", // 0
			"Graphic",          // 1
			"Column",           // 2
			"Font Select",      // 3
			"200 lines",        // 4
			"KAC?",             // 5
			"Memory Switch",    // 6
			"Display ON"        // 7
		};

		logerror("Write to video FF register %s -> %02x\n",video_ff_regnames[(data & 0x0e) >> 1],data & 1);
	}
}

uint8_t pc9801_state::txt_scrl_r(offs_t offset)
{
	//logerror("Read to display register [%02x]\n",offset+0x70);
	/* TODO: ok? */
	if(offset <= 5)
		return m_txt_scroll_reg[offset];
	return 0xff;
}

void pc9801_state::txt_scrl_w(offs_t offset, uint8_t data)
{
	//logerror("Write to display register [%02x] %02x\n",offset+0x70,data);
	if(offset <= 5)
		m_txt_scroll_reg[offset] = data;

	//popmessage("%02x %02x %02x %02x",m_txt_scroll_reg[0],m_txt_scroll_reg[1],m_txt_scroll_reg[2],m_txt_scroll_reg[3]);
}

/*************************************************
 *
 * Video accessors
 *
 ************************************************/

uint8_t pc9801_state::pc9801_a0_r(offs_t offset)
{
	if((offset & 1) == 0)
	{
		switch(offset & 0xe)
		{
			case 0x00:
			case 0x02:
				return m_hgdc[1]->read((offset & 2) >> 1);
			/* TODO: double check these two */
			case 0x04:
				return m_vram_disp & 1;
			case 0x06:
				return m_vram_bank & 1;
			/* bitmap palette clut read */
			case 0x08:
			case 0x0a:
			case 0x0c:
			case 0x0e:
				return m_pal_clut[(offset & 0x6) >> 1];
		}

		return 0xff; //code unreachable
	}
	else // odd
	{
		switch((offset & 0xe) + 1)
		{
			case 0x09://cg window font read
			{
				uint32_t pcg_offset;

				pcg_offset = (m_font_addr & 0x7f7f) << 5;
				pcg_offset|= m_font_line;
				pcg_offset|= m_font_lr;

				return m_kanji_rom[pcg_offset];
			}
		}

		logerror("Read to undefined port [%02x]\n",offset+0xa0);
		return 0xff;
	}
}

void pc9801_state::pc9801_a0_w(offs_t offset, uint8_t data)
{
	if((offset & 1) == 0)
	{
		switch(offset & 0xe)
		{
			case 0x00:
			case 0x02:
				m_hgdc[1]->write((offset & 2) >> 1,data);
				return;
			case 0x04:
				m_vram_disp = data & 1;
				return;
			case 0x06:
				m_vram_bank = data & 1;
				return;
			/* bitmap palette clut write */
			case 0x08:
			case 0x0a:
			case 0x0c:
			case 0x0e:
			{
				uint8_t pal_entry;

				m_pal_clut[(offset & 0x6) >> 1] = data;

				/* can't be more twisted I presume ... :-/ */
				pal_entry = (((offset & 4) >> 1) | ((offset & 2) << 1)) >> 1;
				pal_entry ^= 3;

				m_palette->set_pen_color((pal_entry)|4|8, pal1bit((data & 0x2) >> 1), pal1bit((data & 4) >> 2), pal1bit((data & 1) >> 0));
				m_palette->set_pen_color((pal_entry)|8, pal1bit((data & 0x20) >> 5), pal1bit((data & 0x40) >> 6), pal1bit((data & 0x10) >> 4));
				return;
			}
			default:
				logerror("Write to undefined port [%02x] <- %02x\n",offset+0xa0,data);
				return;
		}
	}
	else // odd
	{
		switch((offset & 0xe) + 1)
		{
			case 0x01:
				m_font_addr = (data & 0xff) | (m_font_addr & 0xff00);
				return;
			case 0x03:
				m_font_addr = ((data & 0xff) << 8) | (m_font_addr & 0xff);
				return;
			case 0x05:
				//logerror("%02x\n",data);
				m_font_line = ((data & 0x0f) << 1);
				m_font_lr = ((data & 0x20) >> 5) ^ 1;
				return;
			case 0x09: //cg window font write
			{
				uint32_t pcg_offset;

				pcg_offset = (m_font_addr & 0x7fff) << 5;
				pcg_offset|= m_font_line;
				pcg_offset|= m_font_lr;
				//logerror("%04x %02x %02x %08x\n",m_font_addr,m_font_line,m_font_lr,pcg_offset);
				if((m_font_addr & 0xff00) == 0x5600 || (m_font_addr & 0xff00) == 0x5700)
				{
					m_kanji_rom[pcg_offset] = data;
					m_gfxdecode->gfx(2)->mark_dirty(pcg_offset >> 5);
				}
				return;
			}
		}

		//logerror("Write to undefined port [%02x) <- %02x\n",offset+0xa0,data);
	}
}


/*************************************************
 *
 * Text layer accessors
 *
 ************************************************/

/* TODO: banking? */
uint16_t pc9801_state::tvram_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t res;

	if((offset & 0x1000) && (mem_mask == 0xff00))
		return 0xffff;

	if(offset < (0x3fe2>>1))
		res = m_tvram[offset];
	else
		res = m_memsw->read(offset & 0xf);

	return res;
}

void pc9801_state::tvram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(offset < (0x3fe2>>1))
		COMBINE_DATA(&m_tvram[offset]);
	else if(m_video_ff[MEMSW_REG] && ACCESSING_BITS_0_7)
		m_memsw->write(offset & 0x0f, data & 0xff);

	COMBINE_DATA(&m_video_ram[0][offset]); //TODO: check me
}

/*************************************************
 *
 * Graphic layer accessors
 *
 ************************************************/

/* +0x8000 is trusted (bank 0 is actually used by 16 colors mode) */
uint8_t pc9801_state::gvram_r(offs_t offset)
{
	return bitswap<8>(m_video_ram[1][(offset>>1)+0x04000+m_vram_bank*0x10000] >> ((offset & 1) << 3),0,1,2,3,4,5,6,7);
}

void pc9801_state::gvram_w(offs_t offset, uint8_t data)
{
	uint16_t ram = m_video_ram[1][(offset>>1)+0x04000+m_vram_bank*0x10000];
	int mask = (offset & 1) << 3;
	data = bitswap<8>(data,0,1,2,3,4,5,6,7);
	m_video_ram[1][(offset>>1)+0x04000+m_vram_bank*0x10000] = (ram & (0xff00 >> mask)) | (data << mask);
}

/*************************************************
 *
 * GRCG (GRaphic CharGer)
 *
 ************************************************/

uint16_t pc9801vm_state::upd7220_grcg_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t res = 0;

	if(!(m_grcg.mode & 0x80) || machine().side_effects_disabled())
		res = m_video_ram[1][offset];
	else if(m_ex_video_ff[2])
		res = egc_blit_r(offset, mem_mask);
	else if(!(m_grcg.mode & 0x40))
	{
		int i;

		offset &= 0x13fff;
		res = 0;
		for(i=0;i<4;i++)
		{
			if((m_grcg.mode & (1 << i)) == 0)
			{
				res |= m_video_ram[1][offset | (((i + 1) & 3) * 0x4000)] ^ (m_grcg.tile[i] | m_grcg.tile[i] << 8);
			}
		}

		res ^= 0xffff;
	}

	return res;
}

void pc9801vm_state::upd7220_grcg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(!(m_grcg.mode & 0x80))
		COMBINE_DATA(&m_video_ram[1][offset]);
	else if(m_ex_video_ff[2])
		egc_blit_w(offset, data, mem_mask);
	else
	{
		int i;
		uint8_t *vram = (uint8_t *)m_video_ram[1].target();
		offset = (offset << 1) & 0x27fff;

		if(m_grcg.mode & 0x40) // RMW
		{
			for(i=0;i<4;i++)
			{
				if((m_grcg.mode & (1 << i)) == 0)
				{
					if(mem_mask & 0xff)
					{
						vram[offset | (((i + 1) & 3) * 0x8000)] &= ~(data >> 0);
						vram[offset | (((i + 1) & 3) * 0x8000)] |= m_grcg.tile[i] & (data >> 0);
					}
					if(mem_mask & 0xff00)
					{
						vram[offset | (((i + 1) & 3) * 0x8000) | 1] &= ~(data >> 8);
						vram[offset | (((i + 1) & 3) * 0x8000) | 1] |= m_grcg.tile[i] & (data >> 8);
					}
				}
			}
		}
		else // TDW
		{
			for(i=0;i<4;i++)
			{
				if((m_grcg.mode & (1 << i)) == 0)
				{
					if(mem_mask & 0xff)
						vram[offset | (((i + 1) & 3) * 0x8000)] = m_grcg.tile[i];
					if(mem_mask & 0xff00)
						vram[offset | (((i + 1) & 3) * 0x8000) | 1] = m_grcg.tile[i];
				}
			}
		}
	}
}


/*************************************************
 *
 * EGC (Enhanced Graphics Charger)
 *
 ************************************************/

uint16_t pc9801vm_state::egc_color_pat(int plane) const
{
	uint8_t color = 0;
	switch((m_egc.regs[1] >> 13) & 3)
	{
		case 1:
			//back color
			color = m_egc.regs[5];
			break;
		case 2:
			//fore color
			color = m_egc.regs[3];
			break;
		default:
			return m_egc.pat[plane];
	}
	return BIT(color, plane) ? 0xffff : 0;
}

uint16_t pc9801vm_state::egc_shift(int plane, uint16_t val)
{
	int src_off = m_egc.regs[6] & 0xf, dst_off = (m_egc.regs[6] >> 4) & 0xf;
	int left = src_off - dst_off, right = dst_off - src_off;
	uint16_t out;
	if(m_egc.regs[6] & 0x1000)
	{
		if(right >= 0)
		{
			out = (val >> right) | m_egc.leftover[plane];
			m_egc.leftover[plane] = val << (16 - right);
		}
		else
		{
			out = (val >> (16 - left)) | m_egc.leftover[plane];
			m_egc.leftover[plane] = val << left;
		}
	}
	else
	{
		if(right >= 0)
		{
			out = (val << right) | m_egc.leftover[plane];
			m_egc.leftover[plane] = val >> (16 - right);
		}
		else
		{
			out = (val << (16 - left)) | m_egc.leftover[plane];
			m_egc.leftover[plane] = val >> left;
		}
	}
	return out;
}

uint16_t pc9801vm_state::egc_do_partial_op(int plane, uint16_t src, uint16_t pat, uint16_t dst) const
{
	uint16_t out = 0;

	for(int i = 7; i >= 0; i--)
	{
		if(BIT(m_egc.regs[2], i))
			out |= src & pat & dst;
		pat = ~pat;
		dst = (!(i & 1)) ? ~dst : dst;
		src = (i == 4) ? ~src : src;
	}
	return out;
}

void pc9801vm_state::egc_blit_w(uint32_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t mask = m_egc.regs[4] & mem_mask, out = 0;
	bool dir = !(m_egc.regs[6] & 0x1000);
	int dst_off = (m_egc.regs[6] >> 4) & 0xf, src_off = m_egc.regs[6] & 0xf;
	offset &= 0x13fff;

	if(!m_egc.init && (src_off > dst_off))
	{
		if(BIT(m_egc.regs[2], 10))
		{
			m_egc.leftover[0] = 0;
			egc_shift(0, data);
			// leftover[0] is inited above, set others to same
			m_egc.leftover[1] = m_egc.leftover[2] = m_egc.leftover[3] = m_egc.leftover[0];
		}
		m_egc.init = true;
		return;
	}

	// mask off the bits before the start
	if(m_egc.first)
	{
		mask &= dir ? ~((1 << dst_off) - 1) : ((1 << (16 - dst_off)) - 1);
		if(BIT(m_egc.regs[2], 10) && !m_egc.init)
			m_egc.leftover[0] = m_egc.leftover[1] = m_egc.leftover[2] = m_egc.leftover[3] = 0;
		m_egc.init = true;
	}

	// mask off the bits past the end of the blit
	if(((m_egc.count < 8) && (mem_mask != 0xffff)) || ((m_egc.count < 16) && (mem_mask == 0xffff)))
	{
		uint16_t end_mask = dir ? ((1 << m_egc.count) - 1) : ~((1 << (16 - m_egc.count)) - 1);
		// if the blit is less than the write size, adjust the masks
		if(m_egc.first)
		{
			if(dir)
				end_mask <<= dst_off;
			else
				end_mask >>= dst_off;
		}
		mask &= end_mask;
	}

	for(int i = 0; i < 4; i++)
	{
		if(!BIT(m_egc.regs[0], i))
		{
			uint16_t src = m_egc.src[i], pat = egc_color_pat(i);
			if(BIT(m_egc.regs[2], 10))
				src = egc_shift(i, data);

			if((m_egc.regs[2] & 0x300) == 0x200)
				pat = m_video_ram[1][offset + (((i + 1) & 3) * 0x4000)];

			switch((m_egc.regs[2] >> 11) & 3)
			{
				case 0:
					out = data;
					break;
				case 1:
					out = egc_do_partial_op(i, src, pat, m_video_ram[1][offset + (((i + 1) & 3) * 0x4000)]);
					break;
				case 2:
					out = pat;
					break;
				case 3:
					logerror("Invalid EGC blit operation\n");
					return;
			}

			m_video_ram[1][offset + (((i + 1) & 3) * 0x4000)] &= ~mask;
			m_video_ram[1][offset + (((i + 1) & 3) * 0x4000)] |= out & mask;
		}
	}
	if(mem_mask != 0xffff)
	{
		if(m_egc.first)
			m_egc.count -= 8 - (dst_off & 7);
		else
			m_egc.count -= 8;
	}
	else
	{
		if(m_egc.first)
			m_egc.count -= 16 - dst_off;
		else
			m_egc.count -= 16;
	}

	m_egc.first = false;

	if(m_egc.count <= 0)
	{
		m_egc.first = true;
		m_egc.init = false;
		m_egc.count = (m_egc.regs[7] & 0xfff) + 1;
	}
}

uint16_t pc9801vm_state::egc_blit_r(uint32_t offset, uint16_t mem_mask)
{
	uint32_t plane_off = offset & 0x13fff;
	if((m_egc.regs[2] & 0x300) == 0x100)
	{
		m_egc.pat[0] = m_video_ram[1][plane_off + 0x4000];
		m_egc.pat[1] = m_video_ram[1][plane_off + (0x4000 * 2)];
		m_egc.pat[2] = m_video_ram[1][plane_off + (0x4000 * 3)];
		m_egc.pat[3] = m_video_ram[1][plane_off];
	}
	//TODO: this needs another look
	/*if(m_egc.first && !m_egc.init)
	{
	    m_egc.leftover[0] = m_egc.leftover[1] = m_egc.leftover[2] = m_egc.leftover[3] = 0;
	    if(((m_egc.regs[6] >> 4) & 0xf) >= (m_egc.regs[6] & 0xf)) // check if we have enough bits
	        m_egc.init = true;
	}*/
	m_egc.init = true;
	for(int i = 0; i < 4; i++)
		m_egc.src[i] = egc_shift(i, m_video_ram[1][plane_off + (((i + 1) & 3) * 0x4000)]);

	if(BIT(m_egc.regs[2], 13))
	{
		uint16_t ret;
		// docs say vram is compared to the foreground color register but 4a2 13-14 must be 2
		// guess that the other values probably work too
		ret =  ~(egc_color_pat(0) ^ m_video_ram[1][plane_off + 0x4000]);
		ret &= ~(egc_color_pat(1) ^ m_video_ram[1][plane_off + (0x4000 * 2)]);
		ret &= ~(egc_color_pat(2) ^ m_video_ram[1][plane_off + (0x4000 * 3)]);
		ret &= ~(egc_color_pat(3) ^ m_video_ram[1][plane_off]);
		return ret;
	}
	else
		return m_egc.src[(m_egc.regs[1] >> 8) & 3];
}

