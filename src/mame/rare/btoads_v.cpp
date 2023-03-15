// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    BattleToads

    Video hardware emulation

**************************************************************************/

#include "emu.h"
#include "btoads.h"


#define BT_DEBUG 0



/*************************************
 *
 *  Video system start
 *
 *************************************/

void btoads_state::video_start()
{
	// initialize the swapped pointers
	m_vram_fg_draw = m_vram_fg0;
	m_vram_fg_display = m_vram_fg1;

	save_item(NAME(m_xscroll0));
	save_item(NAME(m_xscroll1));
	save_item(NAME(m_yscroll0));
	save_item(NAME(m_yscroll1));
	save_item(NAME(m_screen_control));

	save_item(NAME(m_sprite_source_offs));
	save_item(NAME(m_sprite_dest_offs));
	save_item(NAME(m_misc_control));
}



/*************************************
 *
 *  Control registers
 *
 *************************************/

void btoads_state::misc_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_misc_control);

	// bit 3 controls sound reset line
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (m_misc_control & 8) ? CLEAR_LINE : ASSERT_LINE);
}


void btoads_state::display_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		// allow multiple changes during display
		int scanline = m_screen->vpos();
		if (scanline > 0)
			m_screen->update_partial(scanline - 1);

		// bit 15 controls which page is rendered and which page is displayed
		if (data & 0x8000)
		{
			m_vram_fg_draw = m_vram_fg1;
			m_vram_fg_display = m_vram_fg0;
		}
		else
		{
			m_vram_fg_draw = m_vram_fg0;
			m_vram_fg_display = m_vram_fg1;
		}

		// stash the remaining data for later
		m_screen_control = data >> 8;
	}
}



/*************************************
 *
 *  Scroll registers
 *
 *************************************/

void btoads_state::scroll0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// allow multiple changes during display
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

	// upper bits are Y scroll, lower bits are X scroll
	if (ACCESSING_BITS_8_15)
		m_yscroll0 = data >> 8;
	if (ACCESSING_BITS_0_7)
		m_xscroll0 = data & 0xff;
}


void btoads_state::scroll1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// allow multiple changes during display
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

	// upper bits are Y scroll, lower bits are X scroll
	if (ACCESSING_BITS_8_15)
		m_yscroll1 = data >> 8;
	if (ACCESSING_BITS_0_7)
		m_xscroll1 = data & 0xff;
}



/*************************************
 *
 *  Background video RAM
 *
 *************************************/

void btoads_state::vram_bg0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram_bg0[offset & 0x3fcff]);
}


void btoads_state::vram_bg1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram_bg1[offset & 0x3fcff]);
}


uint16_t btoads_state::vram_bg0_r(offs_t offset)
{
	return m_vram_bg0[offset & 0x3fcff];
}


uint16_t btoads_state::vram_bg1_r(offs_t offset)
{
	return m_vram_bg1[offset & 0x3fcff];
}



/*************************************
 *
 *  Foreground video RAM
 *
 *************************************/

void btoads_state::vram_fg_display_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_vram_fg_display[offset] = data;
}


void btoads_state::vram_fg_draw_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_vram_fg_draw[offset] = data;
}


uint16_t btoads_state::vram_fg_display_r(offs_t offset)
{
	return m_vram_fg_display[offset];
}


uint16_t btoads_state::vram_fg_draw_r(offs_t offset)
{
	return m_vram_fg_draw[offset];
}



/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

void btoads_state::render_sprite_row(uint16_t *sprite_source, uint32_t address)
{
	int flipxor = ((*m_sprite_control >> 10) & 1) ? 0xffff : 0x0000;
	int width = (~*m_sprite_control & 0x1ff) + 2;
	int color = (~*m_sprite_control >> 8) & 0xf0;
	int srcoffs = m_sprite_source_offs << 8;
	int srcend = srcoffs + (width << 8);
	int srcstep = 0x100 - (m_sprite_scale[0] & 0xffff);
	int dststep = 0x100 - (m_sprite_scale[4] & 0xffff);
	int dstoffs = m_sprite_dest_offs << 8;

	if (!(m_misc_control & 0x10))
	{
		// non-shadow case
		for ( ; srcoffs < srcend; srcoffs += srcstep, dstoffs += dststep)
		{
			uint16_t src = sprite_source[(srcoffs >> 10) & 0x1ff];
			if (src)
			{
				src = (src >> (((srcoffs ^ flipxor) >> 6) & 0x0c)) & 0x0f;
				if (src)
					m_sprite_dest_base[(dstoffs >> 8) & 0x1ff] = src | color;
			}
		}
	}
	else
	{
		// shadow case
		for ( ; srcoffs < srcend; srcoffs += srcstep, dstoffs += dststep)
		{
			uint16_t src = sprite_source[(srcoffs >> 10) & 0x1ff];
			if (src)
			{
				src = (src >> (((srcoffs ^ flipxor) >> 6) & 0x0c)) & 0x0f;
				if (src)
					m_sprite_dest_base[(dstoffs >> 8) & 0x1ff] = color;
			}
		}
	}

	m_sprite_source_offs += width;
	m_sprite_dest_offs = dstoffs >> 8;
}



/*************************************
 *
 *  Shift register read/write
 *
 *************************************/

TMS340X0_TO_SHIFTREG_CB_MEMBER(btoads_state::to_shiftreg)
{
	address &= ~0x40000000;

	if (address >= 0xa0000000 && address <= 0xa3ffffff)
	{
		// reads from this first region are usual shift register reads
		memcpy(shiftreg, &m_vram_fg_display[(address & 0x3fffff) >> 4], 0x200);
	}
	else if (address >= 0xa4000000 && address <= 0xa7ffffff)
	{
		// reads from this region set the sprite destination address
		m_sprite_dest_base = &m_vram_fg_draw[(address & 0x3fc000) >> 4];
		m_sprite_dest_offs = (address & 0x003fff) >> 5;
	}
	else if (address >= 0xa8000000 && address <= 0xabffffff)
	{
		// reads from this region set the sprite source address
		const u32 *src = &m_vram_fg_data[(address & 0x7fc000) >> 5];
		u16 *dest = shiftreg;
		for (unsigned int i = 0; i != 0x100; i++)
		{
			*dest++ = *src;
			*dest++ = *src >> 16;
			src++;
		}
		m_sprite_source_offs = (address & 0x003fff) >> 3;
	}
	else
		logerror("%s:btoads_to_shiftreg(%08X)\n", machine().describe_context(), address);
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(btoads_state::from_shiftreg)
{
	address &= ~0x40000000;

	if (address >= 0xa0000000 && address <= 0xa3ffffff)
	{
		// writes to this first region are usual shift register writes
		memcpy(&m_vram_fg_display[(address & 0x3fc000) >> 4], shiftreg, 0x200);
	}
	else if (address >= 0xa4000000 && address <= 0xa7ffffff)
	{
		// writes to this region are ignored for our purposes
	}
	else if (address >= 0xa8000000 && address <= 0xabffffff)
	{
		// writes to this region copy standard data
		const u16 *src = shiftreg;
		u32 *dest = &m_vram_fg_data[(address & 0x7fc000) >> 5];
		for (unsigned int i = 0; i != 0x100; i++)
		{
			*dest++ = src[0] | (src[1] << 16);
			src += 2;
		}
	}
	else if (address >= 0xac000000 && address <= 0xafffffff)
	{
		// writes to this region render the current sprite data
		render_sprite_row(shiftreg, address);
	}
	else
		logerror("%s:btoads_from_shiftreg(%08X)\n", machine().describe_context(), address);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

TMS340X0_SCANLINE_RGB32_CB_MEMBER(btoads_state::scanline_update)
{
	uint32_t fulladdr = ((params->rowaddr << 16) | params->coladdr) >> 4;
	uint16_t *bg0_base = &m_vram_bg0[(fulladdr + (m_yscroll0 << 10)) & 0x3fc00];
	uint16_t *bg1_base = &m_vram_bg1[(fulladdr + (m_yscroll1 << 10)) & 0x3fc00];
	uint8_t *spr_base = &m_vram_fg_display[fulladdr & 0x3fc00];
	uint32_t *const dst = &bitmap.pix(scanline);
	const pen_t *pens = m_tlc34076->pens();
	int coladdr = fulladdr & 0x3ff;

	/* for each scanline, switch off the render mode */
	switch (m_screen_control & 3)
	{
		/* mode 0: used in ship level, snake boss, title screen (free play) */
		/* priority is:
		    1. Sprite pixels with high bit clear
		    2. BG1 pixels with the high bit set
		    3. Sprites
		    4. BG1
		    5. BG0
		*/
		case 0:
			for (int x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
			{
				uint8_t sprpix = spr_base[coladdr & 0xff];

				if (sprpix && !(sprpix & 0x80))
				{
					dst[x + 0] = pens[sprpix];
					dst[x + 1] = pens[sprpix];
				}
				else
				{
					uint16_t bg0pix = bg0_base[(coladdr + m_xscroll0) & 0xff];
					uint16_t bg1pix = bg1_base[(coladdr + m_xscroll1) & 0xff];
					uint8_t sprpix = spr_base[coladdr & 0xff];

					if (bg1pix & 0x80)
						dst[x + 0] = pens[bg1pix & 0xff];
					else if (sprpix)
						dst[x + 0] = pens[sprpix];
					else if (bg1pix & 0xff)
						dst[x + 0] = pens[bg1pix & 0xff];
					else
						dst[x + 0] = pens[bg0pix & 0xff];

					if (bg1pix & 0x8000)
						dst[x + 1] = pens[bg1pix >> 8];
					else if (sprpix)
						dst[x + 1] = pens[sprpix];
					else if (bg1pix >> 8)
						dst[x + 1] = pens[bg1pix >> 8];
					else
						dst[x + 1] = pens[bg0pix >> 8];
				}
			}
			break;

		/* mode 1: used in snow level, title screen (free play), top part of rolling ball level */
		/* priority is:
		    1. Sprite pixels with high bit clear
		    2. BG0
		    3. BG1 pixels with high bit set
		    4. Sprites
		    5. BG1
		*/
		case 1:
			for (int x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
			{
				uint8_t sprpix = spr_base[coladdr & 0xff];

				if (sprpix && !(sprpix & 0x80))
				{
					dst[x + 0] = pens[sprpix];
					dst[x + 1] = pens[sprpix];
				}
				else
				{
					uint16_t bg0pix = bg0_base[(coladdr + m_xscroll0) & 0xff];
					uint16_t bg1pix = bg1_base[(coladdr + m_xscroll1) & 0xff];

					if (bg0pix & 0xff)
						dst[x + 0] = pens[bg0pix & 0xff];
					else if (bg1pix & 0x80)
						dst[x + 0] = pens[bg1pix & 0xff];
					else if (sprpix)
						dst[x + 0] = pens[sprpix];
					else
						dst[x + 0] = pens[bg1pix & 0xff];

					if (bg0pix >> 8)
						dst[x + 1] = pens[bg0pix >> 8];
					else if (bg1pix & 0x8000)
						dst[x + 1] = pens[bg1pix >> 8];
					else if (sprpix)
						dst[x + 1] = pens[sprpix];
					else
						dst[x + 1] = pens[bg1pix >> 8];
				}
			}
			break;

		/* mode 2: used in EOA screen, jetpack level, first level, high score screen */
		/* priority is:
		    1. Sprites
		    2. BG1
		    3. BG0
		*/
		case 2:
			for (int x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
			{
				uint8_t sprpix = spr_base[coladdr & 0xff];

				if (sprpix)
				{
					dst[x + 0] = pens[sprpix];
					dst[x + 1] = pens[sprpix];
				}
				else
				{
					uint16_t bg0pix = bg0_base[(coladdr + m_xscroll0) & 0xff];
					uint16_t bg1pix = bg1_base[(coladdr + m_xscroll1) & 0xff];

					if (bg1pix & 0xff)
						dst[x + 0] = pens[bg1pix & 0xff];
					else
						dst[x + 0] = pens[bg0pix & 0xff];

					if (bg1pix >> 8)
						dst[x + 1] = pens[bg1pix >> 8];
					else
						dst[x + 1] = pens[bg0pix >> 8];
				}
			}
			break;

		/* mode 3: used in toilet level, toad intros, bottom of rolling ball level */
		/* priority is:
		    1. BG1 pixels with the high bit set
		    2. Sprite pixels with the high bit set
		    3. BG1
		    4. Sprites
		    5. BG0
		*/
		case 3:
			for (int x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
			{
				uint16_t bg0pix = bg0_base[(coladdr + m_xscroll0) & 0xff];
				uint16_t bg1pix = bg1_base[(coladdr + m_xscroll1) & 0xff];
				uint8_t sprpix = spr_base[coladdr & 0xff];

				if (bg1pix & 0x80)
					dst[x + 0] = pens[bg1pix & 0xff];
				else if (sprpix & 0x80)
					dst[x + 0] = pens[sprpix];
				else if (bg1pix & 0xff)
					dst[x + 0] = pens[bg1pix & 0xff];
				else if (sprpix)
					dst[x + 0] = pens[sprpix];
				else
					dst[x + 0] = pens[bg0pix & 0xff];

				if (bg1pix & 0x8000)
					dst[x + 1] = pens[bg1pix >> 8];
				else if (sprpix & 0x80)
					dst[x + 1] = pens[sprpix];
				else if (bg1pix >> 8)
					dst[x + 1] = pens[bg1pix >> 8];
				else if (sprpix)
					dst[x + 1] = pens[sprpix];
				else
					dst[x + 1] = pens[bg0pix >> 8];
			}
			break;
	}

	/* debugging - dump the screen contents to a file */
#if BT_DEBUG
	popmessage("screen_control = %02X", m_screen_control & 0x7f);

	if (machine().input().code_pressed(KEYCODE_X))
	{
		char name[10];
		FILE *f;

		while (machine().input().code_pressed(KEYCODE_X)) { }

		sprintf(name, "disp%d.log", m_xcount++);
		f = fopen(name, "w");
		fprintf(f, "screen_control = %04X\n\n", m_screen_control << 8);

		for (int i = 0; i < 3; i++)
		{
			uint16_t *base = (i == 0) ? (uint16_t *)m_vram_fg_display : (i == 1) ? m_vram_bg0 : m_vram_bg1;
			int xscr = (i == 0) ? 0 : (i == 1) ? m_xscroll0 : m_xscroll1;
			int yscr = (i == 0) ? 0 : (i == 1) ? m_yscroll0 : m_yscroll1;

			for (int y = 0; y < 224; y++)
			{
				uint32_t offs = ((y + yscr) & 0xff) * TOWORD(0x4000);
				for (int x = 0; x < 256; x++)
				{
					uint16_t pix = base[offs + ((x + xscr) & 0xff)];
					fprintf(f, "%02X%02X", pix & 0xff, pix >> 8);
					if (x % 16 == 15) fprintf(f, " ");
				}
				fprintf(f, "\n");
			}
			fprintf(f, "\n\n");
		}
		fclose(f);
	}

	logerror("---VBLANK---\n");
#endif
}
