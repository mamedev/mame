/*************************************************************************

    BattleToads

    Video hardware emulation

**************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "video/tlc34076.h"
#include "includes/btoads.h"


#define BT_DEBUG 0



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( btoads )
{
	btoads_state *state = machine.driver_data<btoads_state>();
	/* initialize the swapped pointers */
	state->m_vram_fg_draw = (UINT8 *)state->m_vram_fg0;
	state->m_vram_fg_display = (UINT8 *)state->m_vram_fg1;

	state_save_register_global(machine, state->m_xscroll0);
	state_save_register_global(machine, state->m_xscroll1);
	state_save_register_global(machine, state->m_yscroll0);
	state_save_register_global(machine, state->m_yscroll1);
	state_save_register_global(machine, state->m_screen_control);

	state_save_register_global(machine, state->m_sprite_source_offs);
	state_save_register_global(machine, state->m_sprite_dest_offs);
	state_save_register_global(machine, state->m_misc_control);
}



/*************************************
 *
 *  Control registers
 *
 *************************************/

WRITE16_HANDLER( btoads_misc_control_w )
{
	btoads_state *state = space->machine().driver_data<btoads_state>();
	COMBINE_DATA(&state->m_misc_control);

	/* bit 3 controls sound reset line */
	cputag_set_input_line(space->machine(), "audiocpu", INPUT_LINE_RESET, (state->m_misc_control & 8) ? CLEAR_LINE : ASSERT_LINE);
}


WRITE16_HANDLER( btoads_display_control_w )
{
	btoads_state *state = space->machine().driver_data<btoads_state>();
	if (ACCESSING_BITS_8_15)
	{
		/* allow multiple changes during display */
		int scanline = space->machine().primary_screen->vpos();
		if (scanline > 0)
			space->machine().primary_screen->update_partial(scanline - 1);

		/* bit 15 controls which page is rendered and which page is displayed */
		if (data & 0x8000)
		{
			state->m_vram_fg_draw = (UINT8 *)state->m_vram_fg1;
			state->m_vram_fg_display = (UINT8 *)state->m_vram_fg0;
		}
		else
		{
			state->m_vram_fg_draw = (UINT8 *)state->m_vram_fg0;
			state->m_vram_fg_display = (UINT8 *)state->m_vram_fg1;
		}

		/* stash the remaining data for later */
		state->m_screen_control = data >> 8;
	}
}



/*************************************
 *
 *  Scroll registers
 *
 *************************************/

WRITE16_HANDLER( btoads_scroll0_w )
{
	btoads_state *state = space->machine().driver_data<btoads_state>();
	/* allow multiple changes during display */
	space->machine().primary_screen->update_now();

	/* upper bits are Y scroll, lower bits are X scroll */
	if (ACCESSING_BITS_8_15)
		state->m_yscroll0 = data >> 8;
	if (ACCESSING_BITS_0_7)
		state->m_xscroll0 = data & 0xff;
}


WRITE16_HANDLER( btoads_scroll1_w )
{
	btoads_state *state = space->machine().driver_data<btoads_state>();
	/* allow multiple changes during display */
	space->machine().primary_screen->update_now();

	/* upper bits are Y scroll, lower bits are X scroll */
	if (ACCESSING_BITS_8_15)
		state->m_yscroll1 = data >> 8;
	if (ACCESSING_BITS_0_7)
		state->m_xscroll1 = data & 0xff;
}



/*************************************
 *
 *  Palette RAM
 *
 *************************************/

WRITE16_HANDLER( btoads_paletteram_w )
{
	tlc34076_w(space->machine().device("tlc34076"), offset/2, data);
}


READ16_HANDLER( btoads_paletteram_r )
{
	return tlc34076_r(space->machine().device("tlc34076"), offset/2);
}



/*************************************
 *
 *  Background video RAM
 *
 *************************************/

WRITE16_HANDLER( btoads_vram_bg0_w )
{
	btoads_state *state = space->machine().driver_data<btoads_state>();
	COMBINE_DATA(&state->m_vram_bg0[offset & 0x3fcff]);
}


WRITE16_HANDLER( btoads_vram_bg1_w )
{
	btoads_state *state = space->machine().driver_data<btoads_state>();
	COMBINE_DATA(&state->m_vram_bg1[offset & 0x3fcff]);
}


READ16_HANDLER( btoads_vram_bg0_r )
{
	btoads_state *state = space->machine().driver_data<btoads_state>();
	return state->m_vram_bg0[offset & 0x3fcff];
}


READ16_HANDLER( btoads_vram_bg1_r )
{
	btoads_state *state = space->machine().driver_data<btoads_state>();
	return state->m_vram_bg1[offset & 0x3fcff];
}



/*************************************
 *
 *  Foreground video RAM
 *
 *************************************/

WRITE16_HANDLER( btoads_vram_fg_display_w )
{
	btoads_state *state = space->machine().driver_data<btoads_state>();
	if (ACCESSING_BITS_0_7)
		state->m_vram_fg_display[offset] = data;
}


WRITE16_HANDLER( btoads_vram_fg_draw_w )
{
	btoads_state *state = space->machine().driver_data<btoads_state>();
	if (ACCESSING_BITS_0_7)
		state->m_vram_fg_draw[offset] = data;
}


READ16_HANDLER( btoads_vram_fg_display_r )
{
	btoads_state *state = space->machine().driver_data<btoads_state>();
	return state->m_vram_fg_display[offset];
}


READ16_HANDLER( btoads_vram_fg_draw_r )
{
	btoads_state *state = space->machine().driver_data<btoads_state>();
	return state->m_vram_fg_draw[offset];
}



/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

static void render_sprite_row(btoads_state *state, UINT16 *sprite_source, UINT32 address)
{
	int flipxor = ((*state->m_sprite_control >> 10) & 1) ? 0xffff : 0x0000;
	int width = (~*state->m_sprite_control & 0x1ff) + 2;
	int color = (~*state->m_sprite_control >> 8) & 0xf0;
	int srcoffs = state->m_sprite_source_offs << 8;
	int srcend = srcoffs + (width << 8);
	int srcstep = 0x100 - state->m_sprite_scale[0];
	int dststep = 0x100 - state->m_sprite_scale[8];
	int dstoffs = state->m_sprite_dest_offs << 8;

	/* non-shadow case */
	if (!(state->m_misc_control & 0x10))
	{
		for ( ; srcoffs < srcend; srcoffs += srcstep, dstoffs += dststep)
		{
			UINT16 src = sprite_source[(srcoffs >> 10) & 0x1ff];
			if (src)
			{
				src = (src >> (((srcoffs ^ flipxor) >> 6) & 0x0c)) & 0x0f;
				if (src)
					state->m_sprite_dest_base[(dstoffs >> 8) & 0x1ff] = src | color;
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
					state->m_sprite_dest_base[(dstoffs >> 8) & 0x1ff] = color;
			}
		}
	}

	state->m_sprite_source_offs += width;
	state->m_sprite_dest_offs = dstoffs >> 8;
}



/*************************************
 *
 *  Shift register read/write
 *
 *************************************/

void btoads_to_shiftreg(address_space *space, UINT32 address, UINT16 *shiftreg)
{
	btoads_state *state = space->machine().driver_data<btoads_state>();
	address &= ~0x40000000;

	/* reads from this first region are usual shift register reads */
	if (address >= 0xa0000000 && address <= 0xa3ffffff)
		memcpy(shiftreg, &state->m_vram_fg_display[TOWORD(address & 0x3fffff)], TOBYTE(0x1000));

	/* reads from this region set the sprite destination address */
	else if (address >= 0xa4000000 && address <= 0xa7ffffff)
	{
		state->m_sprite_dest_base = &state->m_vram_fg_draw[TOWORD(address & 0x3fc000)];
		state->m_sprite_dest_offs = (address & 0x003fff) >> 5;
	}

	/* reads from this region set the sprite source address */
	else if (address >= 0xa8000000 && address <= 0xabffffff)
	{
		memcpy(shiftreg, &state->m_vram_fg_data[TOWORD(address & 0x7fc000)], TOBYTE(0x2000));
		state->m_sprite_source_offs = (address & 0x003fff) >> 3;
	}

	else
		logerror("%s:btoads_to_shiftreg(%08X)\n", space->machine().describe_context(), address);
}


void btoads_from_shiftreg(address_space *space, UINT32 address, UINT16 *shiftreg)
{
	btoads_state *state = space->machine().driver_data<btoads_state>();
	address &= ~0x40000000;

	/* writes to this first region are usual shift register writes */
	if (address >= 0xa0000000 && address <= 0xa3ffffff)
		memcpy(&state->m_vram_fg_display[TOWORD(address & 0x3fc000)], shiftreg, TOBYTE(0x1000));

	/* writes to this region are ignored for our purposes */
	else if (address >= 0xa4000000 && address <= 0xa7ffffff)
		;

	/* writes to this region copy standard data */
	else if (address >= 0xa8000000 && address <= 0xabffffff)
		memcpy(&state->m_vram_fg_data[TOWORD(address & 0x7fc000)], shiftreg, TOBYTE(0x2000));

	/* writes to this region render the current sprite data */
	else if (address >= 0xac000000 && address <= 0xafffffff)
		render_sprite_row(state, shiftreg, address);

	else
		logerror("%s:btoads_from_shiftreg(%08X)\n", space->machine().describe_context(), address);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

void btoads_scanline_update(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params)
{
	btoads_state *state = screen.machine().driver_data<btoads_state>();
	UINT32 fulladdr = ((params->rowaddr << 16) | params->coladdr) >> 4;
	UINT16 *bg0_base = &state->m_vram_bg0[(fulladdr + (state->m_yscroll0 << 10)) & 0x3fc00];
	UINT16 *bg1_base = &state->m_vram_bg1[(fulladdr + (state->m_yscroll1 << 10)) & 0x3fc00];
	UINT8 *spr_base = &state->m_vram_fg_display[fulladdr & 0x3fc00];
	UINT32 *dst = BITMAP_ADDR32(bitmap, scanline, 0);
	const rgb_t *pens = tlc34076_get_pens(screen.machine().device("tlc34076"));
	int coladdr = fulladdr & 0x3ff;
	int x;

	/* for each scanline, switch off the render mode */
	switch (state->m_screen_control & 3)
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
					UINT16 bg0pix = bg0_base[(coladdr + state->m_xscroll0) & 0xff];
					UINT16 bg1pix = bg1_base[(coladdr + state->m_xscroll1) & 0xff];
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
					UINT16 bg0pix = bg0_base[(coladdr + state->m_xscroll0) & 0xff];
					UINT16 bg1pix = bg1_base[(coladdr + state->m_xscroll1) & 0xff];

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
					UINT16 bg0pix = bg0_base[(coladdr + state->m_xscroll0) & 0xff];
					UINT16 bg1pix = bg1_base[(coladdr + state->m_xscroll1) & 0xff];

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
				UINT16 bg0pix = bg0_base[(coladdr + state->m_xscroll0) & 0xff];
				UINT16 bg1pix = bg1_base[(coladdr + state->m_xscroll1) & 0xff];
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
	popmessage("screen_control = %02X", state->m_screen_control & 0x7f);

	if (input_code_pressed(screen.machine(), KEYCODE_X))
	{
		char name[10];
		FILE *f;
		int i;

		while (input_code_pressed(screen.machine(), KEYCODE_X)) ;

		sprintf(name, "disp%d.log", state->m_xcount++);
		f = fopen(name, "w");
		fprintf(f, "screen_control = %04X\n\n", state->m_screen_control << 8);

		for (i = 0; i < 3; i++)
		{
			UINT16 *base = (i == 0) ? (UINT16 *)state->m_vram_fg_display : (i == 1) ? state->m_vram_bg0 : state->m_vram_bg1;
			int xscr = (i == 0) ? 0 : (i == 1) ? state->m_xscroll0 : state->m_xscroll1;
			int yscr = (i == 0) ? 0 : (i == 1) ? state->m_yscroll0 : state->m_yscroll1;
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
