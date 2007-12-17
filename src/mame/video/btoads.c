/*************************************************************************

    BattleToads

    Video hardware emulation

**************************************************************************/

#include "driver.h"
#include "cpu/tms34010/tms34010.h"
#include "video/tlc34076.h"
#include "btoads.h"


#define BT_DEBUG 0



/*************************************
 *
 *  Global variables
 *
 *************************************/

UINT16 *btoads_vram_fg0, *btoads_vram_fg1, *btoads_vram_fg_data;
UINT16 *btoads_vram_bg0, *btoads_vram_bg1;
UINT16 *btoads_sprite_scale;
UINT16 *btoads_sprite_control;

static UINT8 *vram_fg_draw, *vram_fg_display;

static int xscroll0, yscroll0;
static int xscroll1, yscroll1;
static int screen_control;

static UINT16 sprite_source_offs;
static UINT8 *sprite_dest_base;
static UINT16 sprite_dest_offs;

static UINT16 misc_control;



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( btoads )
{
	/* initialize the swapped pointers */
	vram_fg_draw = (UINT8 *)btoads_vram_fg0;
	vram_fg_display = (UINT8 *)btoads_vram_fg1;
}



/*************************************
 *
 *  Control registers
 *
 *************************************/

WRITE16_HANDLER( btoads_misc_control_w )
{
	COMBINE_DATA(&misc_control);

	/* bit 3 controls sound reset line */
	cpunum_set_input_line(1, INPUT_LINE_RESET, (misc_control & 8) ? CLEAR_LINE : ASSERT_LINE);
}


WRITE16_HANDLER( btoads_display_control_w )
{
	if (ACCESSING_MSB)
	{
		/* allow multiple changes during display */
		video_screen_update_partial(0, video_screen_get_vpos(0) - 1);

		/* bit 15 controls which page is rendered and which page is displayed */
		if (data & 0x8000)
		{
			vram_fg_draw = (UINT8 *)btoads_vram_fg1;
			vram_fg_display = (UINT8 *)btoads_vram_fg0;
		}
		else
		{
			vram_fg_draw = (UINT8 *)btoads_vram_fg0;
			vram_fg_display = (UINT8 *)btoads_vram_fg1;
		}

		/* stash the remaining data for later */
		screen_control = data >> 8;
	}
}



/*************************************
 *
 *  Scroll registers
 *
 *************************************/

WRITE16_HANDLER( btoads_scroll0_w )
{
	/* allow multiple changes during display */
	video_screen_update_partial(0, video_screen_get_vpos(0) - 1);

	/* upper bits are Y scroll, lower bits are X scroll */
	if (ACCESSING_MSB)
		yscroll0 = data >> 8;
	if (ACCESSING_LSB)
		xscroll0 = data & 0xff;
}


WRITE16_HANDLER( btoads_scroll1_w )
{
	/* allow multiple changes during display */
	video_screen_update_partial(0, video_screen_get_vpos(0) - 1);

	/* upper bits are Y scroll, lower bits are X scroll */
	if (ACCESSING_MSB)
		yscroll1 = data >> 8;
	if (ACCESSING_LSB)
		xscroll1 = data & 0xff;
}



/*************************************
 *
 *  Palette RAM
 *
 *************************************/

WRITE16_HANDLER( btoads_paletteram_w )
{
	tlc34076_lsb_w(offset/2, data, mem_mask);
}


READ16_HANDLER( btoads_paletteram_r )
{
	return tlc34076_lsb_r(offset/2, mem_mask);
}



/*************************************
 *
 *  Background video RAM
 *
 *************************************/

WRITE16_HANDLER( btoads_vram_bg0_w )
{
	COMBINE_DATA(&btoads_vram_bg0[offset & 0x3fcff]);
}


WRITE16_HANDLER( btoads_vram_bg1_w )
{
	COMBINE_DATA(&btoads_vram_bg1[offset & 0x3fcff]);
}


READ16_HANDLER( btoads_vram_bg0_r )
{
	return btoads_vram_bg0[offset & 0x3fcff];
}


READ16_HANDLER( btoads_vram_bg1_r )
{
	return btoads_vram_bg1[offset & 0x3fcff];
}



/*************************************
 *
 *  Foreground video RAM
 *
 *************************************/

WRITE16_HANDLER( btoads_vram_fg_display_w )
{
	if (ACCESSING_LSB)
		vram_fg_display[offset] = data;
}


WRITE16_HANDLER( btoads_vram_fg_draw_w )
{
	if (ACCESSING_LSB)
		vram_fg_draw[offset] = data;
}


READ16_HANDLER( btoads_vram_fg_display_r )
{
	return vram_fg_display[offset];
}


READ16_HANDLER( btoads_vram_fg_draw_r )
{
	return vram_fg_draw[offset];
}



/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

static void render_sprite_row(UINT16 *sprite_source, UINT32 address)
{
	int flipxor = ((*btoads_sprite_control >> 10) & 1) ? 0xffff : 0x0000;
	int width = (~*btoads_sprite_control & 0x1ff) + 2;
	int color = (~*btoads_sprite_control >> 8) & 0xf0;
	int srcoffs = sprite_source_offs << 8;
	int srcend = srcoffs + (width << 8);
	int srcstep = 0x100 - btoads_sprite_scale[0];
	int dststep = 0x100 - btoads_sprite_scale[8];
	int dstoffs = sprite_dest_offs << 8;

	/* non-shadow case */
	if (!(misc_control & 0x10))
	{
		for ( ; srcoffs < srcend; srcoffs += srcstep, dstoffs += dststep)
		{
			UINT16 src = sprite_source[(srcoffs >> 10) & 0x1ff];
			if (src)
			{
				src = (src >> (((srcoffs ^ flipxor) >> 6) & 0x0c)) & 0x0f;
				if (src)
					sprite_dest_base[(dstoffs >> 8) & 0x1ff] = src | color;
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
					sprite_dest_base[(dstoffs >> 8) & 0x1ff] = color;
			}
		}
	}

	sprite_source_offs += width;
	sprite_dest_offs = dstoffs >> 8;
}



/*************************************
 *
 *  Shift register read/write
 *
 *************************************/

void btoads_to_shiftreg(UINT32 address, UINT16 *shiftreg)
{
	address &= ~0x40000000;

	/* reads from this first region are usual shift register reads */
	if (address >= 0xa0000000 && address <= 0xa3ffffff)
		memcpy(shiftreg, &vram_fg_display[TOWORD(address & 0x3fffff)], TOBYTE(0x1000));

	/* reads from this region set the sprite destination address */
	else if (address >= 0xa4000000 && address <= 0xa7ffffff)
	{
		sprite_dest_base = &vram_fg_draw[TOWORD(address & 0x3fc000)];
		sprite_dest_offs = (INT16)((address & 0x3ff) << 2) >> 2;
	}

	/* reads from this region set the sprite source address */
	else if (address >= 0xa8000000 && address <= 0xabffffff)
	{
		memcpy(shiftreg, &btoads_vram_fg_data[TOWORD(address & 0x7fc000)], TOBYTE(0x2000));
		sprite_source_offs = (address & 0x003fff) >> 3;
	}

	else
		logerror("%08X:btoads_to_shiftreg(%08X)\n", activecpu_get_pc(), address);
}


void btoads_from_shiftreg(UINT32 address, UINT16 *shiftreg)
{
	address &= ~0x40000000;

	/* writes to this first region are usual shift register writes */
	if (address >= 0xa0000000 && address <= 0xa3ffffff)
		memcpy(&vram_fg_display[TOWORD(address & 0x3fc000)], shiftreg, TOBYTE(0x1000));

	/* writes to this region are ignored for our purposes */
	else if (address >= 0xa4000000 && address <= 0xa7ffffff)
		;

	/* writes to this region copy standard data */
	else if (address >= 0xa8000000 && address <= 0xabffffff)
		memcpy(&btoads_vram_fg_data[TOWORD(address & 0x7fc000)], shiftreg, TOBYTE(0x2000));

	/* writes to this region render the current sprite data */
	else if (address >= 0xac000000 && address <= 0xafffffff)
		render_sprite_row(shiftreg, address);

	else
		logerror("%08X:btoads_from_shiftreg(%08X)\n", activecpu_get_pc(), address);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

void btoads_scanline_update(running_machine *machine, int screen, mame_bitmap *bitmap, int scanline, const tms34010_display_params *params)
{
	UINT32 fulladdr = ((params->rowaddr << 16) | params->coladdr) >> 4;
	UINT16 *bg0_base = &btoads_vram_bg0[(fulladdr + (yscroll0 << 10)) & 0x3fc00];
	UINT16 *bg1_base = &btoads_vram_bg1[(fulladdr + (yscroll1 << 10)) & 0x3fc00];
	UINT8 *spr_base = &vram_fg_display[fulladdr & 0x3fc00];
	UINT16 *dst = BITMAP_ADDR16(bitmap, scanline, 0);
	int coladdr = fulladdr & 0x3ff;
	int x;

	/* for each scanline, switch off the render mode */
	switch (screen_control & 3)
	{
		/* mode 0: used in ship level, snake boss, title screen (free play) */
		case 0:

		/* mode 2: used in EOA screen, jetpack level, first level, high score screen */
		case 2:
			for (x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
			{
				UINT8 sprpix = spr_base[coladdr & 0xff];

				if (sprpix)
				{
					dst[x + 0] = sprpix;
					dst[x + 1] = sprpix;
				}
				else
				{
					UINT16 bg0pix = bg0_base[(coladdr + xscroll0) & 0xff];
					UINT16 bg1pix = bg1_base[(coladdr + xscroll1) & 0xff];

					if (bg1pix & 0xff)
						dst[x + 0] = bg1pix & 0xff;
					else
						dst[x + 0] = bg0pix & 0xff;

					if (bg1pix >> 8)
						dst[x + 1] = bg1pix >> 8;
					else
						dst[x + 1] = bg0pix >> 8;
				}
			}
			break;

		/* mode 1: used in snow level, title screen (free play), top part of rolling ball level */
		case 1:
			for (x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
			{
				UINT8 sprpix = spr_base[coladdr & 0xff];

				if (sprpix && !(sprpix & 0x80))
				{
					dst[x + 0] = sprpix;
					dst[x + 1] = sprpix;
				}
				else
				{
					UINT16 bg0pix = bg0_base[(coladdr + xscroll0) & 0xff];
					UINT16 bg1pix = bg1_base[(coladdr + xscroll1) & 0xff];

					if (bg0pix & 0xff)
						dst[x + 0] = bg0pix & 0xff;
					else if (bg1pix & 0x80)
						dst[x + 0] = bg1pix & 0xff;
					else if (sprpix)
						dst[x + 0] = sprpix;
					else
						dst[x + 0] = bg1pix & 0xff;

					if (bg0pix >> 8)
						dst[x + 1] = bg0pix >> 8;
					else if (bg1pix & 0x8000)
						dst[x + 1] = bg1pix >> 8;
					else if (sprpix)
						dst[x + 1] = sprpix;
					else
						dst[x + 1] = bg1pix >> 8;
				}
			}
			break;

		/* mode 3: used in toilet level, toad intros, bottom of rolling ball level */
		case 3:
			for (x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
			{
				UINT16 bg0pix = bg0_base[(coladdr + xscroll0) & 0xff];
				UINT16 bg1pix = bg1_base[(coladdr + xscroll1) & 0xff];
				UINT8 sprpix = spr_base[coladdr & 0xff];

				if (bg1pix & 0x80)
					dst[x + 0] = bg1pix & 0xff;
				else if (sprpix & 0x80)
					dst[x + 0] = sprpix;
				else if (bg1pix & 0xff)
					dst[x + 0] = bg1pix & 0xff;
				else if (sprpix)
					dst[x + 0] = sprpix;
				else
					dst[x + 0] = bg0pix & 0xff;

				if (bg1pix & 0x8000)
					dst[x + 1] = bg1pix >> 8;
				else if (sprpix & 0x80)
					dst[x + 1] = sprpix;
				else if (bg1pix >> 8)
					dst[x + 1] = bg1pix >> 8;
				else if (sprpix)
					dst[x + 1] = sprpix;
				else
					dst[x + 1] = bg0pix >> 8;
			}
			break;
	}

	/* debugging - dump the screen contents to a file */
#if BT_DEBUG
	if (input_code_pressed(KEYCODE_X))
	{
		static int count = 0;
		char name[10];
		FILE *f;
		int i;

		sprintf(name, "disp%d.log", count++);
		f = fopen(name, "w");
		fprintf(f, "screen_control = %04X\n\n", screen_control << 8);

		for (i = 0; i < 3; i++)
		{
			UINT16 *base = (i == 0) ? (UINT16 *)vram_fg_display : (i == 1) ? btoads_vram_bg0 : btoads_vram_bg1;
			int xscr = (i == 0) ? 0 : (i == 1) ? xscroll0 : xscroll1;
			int yscr = (i == 0) ? 0 : (i == 1) ? yscroll0 : yscroll1;
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
