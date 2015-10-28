// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    BattleToads

    Video hardware emulation

**************************************************************************/

#include "emu.h"
#include "includes/btoads.h"


#define BT_DEBUG 0



/*************************************
 *
 *  Video system start
 *
 *************************************/

void btoads_state::video_start()
{
	/* initialize the swapped pointers */
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

WRITE16_MEMBER( btoads_state::misc_control_w )
{
	COMBINE_DATA(&m_misc_control);

	/* bit 3 controls sound reset line */
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (m_misc_control & 8) ? CLEAR_LINE : ASSERT_LINE);
}


WRITE16_MEMBER( btoads_state::display_control_w )
{
	if (ACCESSING_BITS_8_15)
	{
		/* allow multiple changes during display */
		int scanline = m_screen->vpos();
		if (scanline > 0)
			m_screen->update_partial(scanline - 1);

		/* bit 15 controls which page is rendered and which page is displayed */
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

		/* stash the remaining data for later */
		m_screen_control = data >> 8;
	}
}



/*************************************
 *
 *  Scroll registers
 *
 *************************************/

WRITE16_MEMBER( btoads_state::scroll0_w )
{
	/* allow multiple changes during display */
//	m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

	/* upper bits are Y scroll, lower bits are X scroll */
	if (ACCESSING_BITS_8_15)
		m_yscroll0 = data >> 8;
	if (ACCESSING_BITS_0_7)
		m_xscroll0 = data & 0xff;
}


WRITE16_MEMBER( btoads_state::scroll1_w )
{
	/* allow multiple changes during display */
//	m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

	/* upper bits are Y scroll, lower bits are X scroll */
	if (ACCESSING_BITS_8_15)
		m_yscroll1 = data >> 8;
	if (ACCESSING_BITS_0_7)
		m_xscroll1 = data & 0xff;
}



/*************************************
 *
 *  Palette RAM
 *
 *************************************/

WRITE16_MEMBER( btoads_state::paletteram_w )
{
	m_tlc34076->write(space, offset/2, data);
}


READ16_MEMBER( btoads_state::paletteram_r )
{
	return m_tlc34076->read(space, offset/2);
}



/*************************************
 *
 *  Background video RAM
 *
 *************************************/

WRITE16_MEMBER( btoads_state::vram_bg0_w )
{
	COMBINE_DATA(&m_vram_bg0[offset & 0x3fcff]);
}


WRITE16_MEMBER( btoads_state::vram_bg1_w )
{
	COMBINE_DATA(&m_vram_bg1[offset & 0x3fcff]);
}


READ16_MEMBER( btoads_state::vram_bg0_r )
{
	return m_vram_bg0[offset & 0x3fcff];
}


READ16_MEMBER( btoads_state::vram_bg1_r )
{
	return m_vram_bg1[offset & 0x3fcff];
}



/*************************************
 *
 *  Foreground video RAM
 *
 *************************************/

WRITE16_MEMBER( btoads_state::vram_fg_display_w )
{
	if (ACCESSING_BITS_0_7)
		m_vram_fg_display[offset] = data;
}


WRITE16_MEMBER( btoads_state::vram_fg_draw_w )
{
	if (ACCESSING_BITS_0_7)
		m_vram_fg_draw[offset] = data;
}


READ16_MEMBER( btoads_state::vram_fg_display_r )
{
	return m_vram_fg_display[offset];
}


READ16_MEMBER( btoads_state::vram_fg_draw_r )
{
	return m_vram_fg_draw[offset];
}



/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

void btoads_state::render_sprite_row(UINT16 *sprite_source, UINT32 address)
{
	int flipxor = ((*m_sprite_control >> 10) & 1) ? 0xffff : 0x0000;
	int width = (~*m_sprite_control & 0x1ff) + 2;
	int color = (~*m_sprite_control >> 8) & 0xf0;
	int srcoffs = m_sprite_source_offs << 8;
	int srcend = srcoffs + (width << 8);
	int srcstep = 0x100 - m_sprite_scale[0];
	int dststep = 0x100 - m_sprite_scale[8];
	int dstoffs = m_sprite_dest_offs << 8;

	/* non-shadow case */
	if (!(m_misc_control & 0x10))
	{
		for ( ; srcoffs < srcend; srcoffs += srcstep, dstoffs += dststep)
		{
			UINT16 src = sprite_source[(srcoffs >> 10) & 0x1ff];
			if (src)
			{
				src = (src >> (((srcoffs ^ flipxor) >> 6) & 0x0c)) & 0x0f;
				if (src)
					m_sprite_dest_base[(dstoffs >> 8) & 0x1ff] = src | color;
			}
		}
	}

	/* shadow case */
	else
	{
		for ( ; srcoffs < srcend; srcoffs += srcstep, dstoffs += dststep)
		{
			UINT16 src = sprite_source[(srcoffs >> 10) & 0x1ff];
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

	/* reads from this first region are usual shift register reads */
	if (address >= 0xa0000000 && address <= 0xa3ffffff)
		memcpy(shiftreg, &m_vram_fg_display[TOWORD(address & 0x3fffff)], TOBYTE(0x1000));

	/* reads from this region set the sprite destination address */
	else if (address >= 0xa4000000 && address <= 0xa7ffffff)
	{
		m_sprite_dest_base = &m_vram_fg_draw[TOWORD(address & 0x3fc000)];
		m_sprite_dest_offs = (address & 0x003fff) >> 5;
	}

	/* reads from this region set the sprite source address */
	else if (address >= 0xa8000000 && address <= 0xabffffff)
	{
		memcpy(shiftreg, &m_vram_fg_data[TOWORD(address & 0x7fc000)], TOBYTE(0x2000));
		m_sprite_source_offs = (address & 0x003fff) >> 3;
	}

	else
		logerror("%s:btoads_to_shiftreg(%08X)\n", machine().describe_context(), address);
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(btoads_state::from_shiftreg)
{
	address &= ~0x40000000;

	/* writes to this first region are usual shift register writes */
	if (address >= 0xa0000000 && address <= 0xa3ffffff)
		memcpy(&m_vram_fg_display[TOWORD(address & 0x3fc000)], shiftreg, TOBYTE(0x1000));

	/* writes to this region are ignored for our purposes */
	else if (address >= 0xa4000000 && address <= 0xa7ffffff)
		;

	/* writes to this region copy standard data */
	else if (address >= 0xa8000000 && address <= 0xabffffff)
		memcpy(&m_vram_fg_data[TOWORD(address & 0x7fc000)], shiftreg, TOBYTE(0x2000));

	/* writes to this region render the current sprite data */
	else if (address >= 0xac000000 && address <= 0xafffffff)
		render_sprite_row(shiftreg, address);

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
	UINT32 fulladdr = ((params->rowaddr << 16) | params->coladdr) >> 4;
	UINT16 *bg0_base = &m_vram_bg0[(fulladdr + (m_yscroll0 << 10)) & 0x3fc00];
	UINT16 *bg1_base = &m_vram_bg1[(fulladdr + (m_yscroll1 << 10)) & 0x3fc00];
	UINT8 *spr_base = &m_vram_fg_display[fulladdr & 0x3fc00];
	UINT32 *dst = &bitmap.pix32(scanline);
	const rgb_t *pens = m_tlc34076->get_pens();
	int coladdr = fulladdr & 0x3ff;
	int x;

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
			for (x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
			{
				UINT8 sprpix = spr_base[coladdr & 0xff];

				if (sprpix && !(sprpix & 0x80))
				{
					dst[x + 0] = pens[sprpix];
					dst[x + 1] = pens[sprpix];
				}
				else
				{
					UINT16 bg0pix = bg0_base[(coladdr + m_xscroll0) & 0xff];
					UINT16 bg1pix = bg1_base[(coladdr + m_xscroll1) & 0xff];
					UINT8 sprpix = spr_base[coladdr & 0xff];

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
			for (x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
			{
				UINT8 sprpix = spr_base[coladdr & 0xff];

				if (sprpix && !(sprpix & 0x80))
				{
					dst[x + 0] = pens[sprpix];
					dst[x + 1] = pens[sprpix];
				}
				else
				{
					UINT16 bg0pix = bg0_base[(coladdr + m_xscroll0) & 0xff];
					UINT16 bg1pix = bg1_base[(coladdr + m_xscroll1) & 0xff];

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
			for (x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
			{
				UINT8 sprpix = spr_base[coladdr & 0xff];

				if (sprpix)
				{
					dst[x + 0] = pens[sprpix];
					dst[x + 1] = pens[sprpix];
				}
				else
				{
					UINT16 bg0pix = bg0_base[(coladdr + m_xscroll0) & 0xff];
					UINT16 bg1pix = bg1_base[(coladdr + m_xscroll1) & 0xff];

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
			for (x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
			{
				UINT16 bg0pix = bg0_base[(coladdr + m_xscroll0) & 0xff];
				UINT16 bg1pix = bg1_base[(coladdr + m_xscroll1) & 0xff];
				UINT8 sprpix = spr_base[coladdr & 0xff];

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
		int i;

		while (machine().input().code_pressed(KEYCODE_X)) ;

		sprintf(name, "disp%d.log", m_xcount++);
		f = fopen(name, "w");
		fprintf(f, "screen_control = %04X\n\n", m_screen_control << 8);

		for (i = 0; i < 3; i++)
		{
			UINT16 *base = (i == 0) ? (UINT16 *)m_vram_fg_display : (i == 1) ? m_vram_bg0 : m_vram_bg1;
			int xscr = (i == 0) ? 0 : (i == 1) ? m_xscroll0 : m_xscroll1;
			int yscr = (i == 0) ? 0 : (i == 1) ? m_yscroll0 : m_yscroll1;
			int y;

			for (y = 0; y < 224; y++)
			{
				UINT32 offs = ((y + yscr) & 0xff) * TOWORD(0x4000);
				for (x = 0; x < 256; x++)
				{
					UINT16 pix = base[offs + ((x + xscr) & 0xff)];
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
