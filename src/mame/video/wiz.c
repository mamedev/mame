/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/wiz.h"


VIDEO_START( wiz )
{
	wiz_state *state = machine.driver_data<wiz_state>();
	state_save_register_global_array(machine, state->m_char_bank);
	state_save_register_global_array(machine, state->m_palbank);
	state_save_register_global(machine, state->m_flipx);
	state_save_register_global(machine, state->m_flipy);
	state_save_register_global(machine, state->m_bgpen);
}

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Stinger has three 256x4 palette PROMs (one per gun).
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 100 ohm resistor  -- RED/GREEN/BLUE
        -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 1  kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
PALETTE_INIT( wiz )
{
	int i;


	for (i = 0;i < machine.total_colors();i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;


		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x42 * bit2 + 0x90 * bit3;
		bit0 = (color_prom[machine.total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[machine.total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[machine.total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[machine.total_colors()] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x42 * bit2 + 0x90 * bit3;
		bit0 = (color_prom[2*machine.total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[2*machine.total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[2*machine.total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[2*machine.total_colors()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x42 * bit2 + 0x90 * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));

		color_prom++;
	}
}

WRITE8_MEMBER(wiz_state::wiz_palettebank_w)
{
	m_palbank[offset] = data & 1;
	m_palette_bank = m_palbank[0] + 2 * m_palbank[1];
}

WRITE8_MEMBER(wiz_state::wiz_bgcolor_w)
{
	m_bgpen = data;
}

WRITE8_MEMBER(wiz_state::wiz_char_bank_select_w)
{
	m_char_bank[offset] = data & 1;
}

WRITE8_MEMBER(wiz_state::wiz_flipx_w)
{
	m_flipx = data;
}


WRITE8_MEMBER(wiz_state::wiz_flipy_w)
{
	m_flipy = data;
}

static void draw_background(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank, int colortype)
{
	wiz_state *state = machine.driver_data<wiz_state>();
	UINT8 *videoram = state->m_videoram;
	int offs;

	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */

	for (offs = 0x400 - 1; offs >= 0; offs--)
	{
		int scroll,sx,sy,col;

		sx = offs % 32;
		sy = offs / 32;

		if (colortype)
		{
			col = (state->m_attributesram[2 * sx + 1] & 0x07);
		}
		else
		{
			col = (state->m_attributesram[2 * (offs % 32) + 1] & 0x04) + (videoram[offs] & 3);
		}

		scroll = (8*sy + 256 - state->m_attributesram[2 * sx]) % 256;
		if (state->m_flipy)
		{
		   scroll = (248 - scroll) % 256;
		}
		if (state->m_flipx) sx = 31 - sx;


		drawgfx_transpen(bitmap,cliprect,machine.gfx[bank],
			videoram[offs],
			col + 8 * state->m_palette_bank,
			state->m_flipx,state->m_flipy,
			8*sx,scroll,0);
	}
}

static void draw_foreground(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int colortype)
{
	wiz_state *state = machine.driver_data<wiz_state>();
	int offs;

	/* draw the frontmost playfield. They are characters, but draw them as sprites. */
	for (offs = 0x400 - 1; offs >= 0; offs--)
	{
		int scroll,sx,sy,col;


		sx = offs % 32;
		sy = offs / 32;

		if (colortype)
		{
			col = (state->m_attributesram2[2 * sx + 1] & 0x07);
		}
		else
		{
			col = (state->m_colorram2[offs] & 0x07);
		}

		scroll = (8*sy + 256 - state->m_attributesram2[2 * sx]) % 256;
		if (state->m_flipy)
		{
		   scroll = (248 - scroll) % 256;
		}
		if (state->m_flipx) sx = 31 - sx;


		drawgfx_transpen(bitmap,cliprect,machine.gfx[state->m_char_bank[1]],
			state->m_videoram2[offs],
			col + 8 * state->m_palette_bank,
			state->m_flipx,state->m_flipy,
			8*sx,scroll,0);
	}
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,
						 const rectangle &cliprect, UINT8* sprite_ram,
                         int bank)
{
	wiz_state *state = machine.driver_data<wiz_state>();
	int offs;

	for (offs = state->m_spriteram_size - 4;offs >= 0;offs -= 4)
	{
		int sx,sy;


		sx = sprite_ram[offs + 3];
		sy = sprite_ram[offs];

		if (!sx || !sy) continue;

		if ( state->m_flipx) sx = 240 - sx;
		if (!state->m_flipy) sy = 240 - sy;

		drawgfx_transpen(bitmap,cliprect,machine.gfx[bank],
				sprite_ram[offs + 1],
				(sprite_ram[offs + 2] & 0x07) + 8 * state->m_palette_bank,
				state->m_flipx,state->m_flipy,
				sx,sy,0);
	}
}


SCREEN_UPDATE_IND16( kungfut )
{
	wiz_state *state = screen.machine().driver_data<wiz_state>();
	bitmap.fill(state->m_bgpen, cliprect);
	draw_background(screen.machine(), bitmap, cliprect, 2 + state->m_char_bank[0] , 0);
	draw_foreground(screen.machine(), bitmap, cliprect, 0);
	draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram2, 4);
	draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram  , 5);
	return 0;
}

SCREEN_UPDATE_IND16( wiz )
{
	wiz_state *state = screen.machine().driver_data<wiz_state>();
	int bank;

	bitmap.fill(state->m_bgpen, cliprect);
	draw_background(screen.machine(), bitmap, cliprect, 2 + ((state->m_char_bank[0] << 1) | state->m_char_bank[1]), 0);
	draw_foreground(screen.machine(), bitmap, cliprect, 0);

	const rectangle spritevisiblearea(2*8, 32*8-1, 2*8, 30*8-1);
	const rectangle spritevisibleareaflipx(0*8, 30*8-1, 2*8, 30*8-1);
	const rectangle &visible_area = state->m_flipx ? spritevisibleareaflipx : spritevisiblearea;

	bank = 7 + *state->m_sprite_bank;

	draw_sprites(screen.machine(), bitmap, visible_area, state->m_spriteram2, 6);
	draw_sprites(screen.machine(), bitmap, visible_area, state->m_spriteram  , bank);
	return 0;
}


SCREEN_UPDATE_IND16( stinger )
{
	wiz_state *state = screen.machine().driver_data<wiz_state>();
	bitmap.fill(state->m_bgpen, cliprect);
	draw_background(screen.machine(), bitmap, cliprect, 2 + state->m_char_bank[0], 1);
	draw_foreground(screen.machine(), bitmap, cliprect, 1);
	draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram2, 4);
	draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram  , 5);
	return 0;
}
