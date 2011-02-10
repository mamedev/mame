/*****************************************************************************************

 Speed Attack video hardware emulation

*****************************************************************************************/
#include "emu.h"
#include "includes/speedatk.h"
#include "video/mc6845.h"


PALETTE_INIT( speedatk )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x10);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x10; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}

WRITE8_HANDLER( speedatk_videoram_w )
{
	speedatk_state *state = space->machine->driver_data<speedatk_state>();

	state->videoram[offset] = data;
}

WRITE8_HANDLER( speedatk_colorram_w )
{
	speedatk_state *state = space->machine->driver_data<speedatk_state>();

	state->colorram[offset] = data;
}

VIDEO_START( speedatk )
{

}

WRITE8_HANDLER( speedatk_6845_w )
{
	speedatk_state *state = space->machine->driver_data<speedatk_state>();

	if(offset == 0)
	{
		state->crtc_index = data;
		mc6845_address_w(space->machine->device("crtc"),0,data);
	}
	else
	{
		state->crtc_vreg[state->crtc_index] = data;
		mc6845_register_w(space->machine->device("crtc"),0,data);
	}
}

VIDEO_UPDATE( speedatk )
{
	speedatk_state *state = screen->machine->driver_data<speedatk_state>();
	int x,y;
	int count;
	UINT16 tile;
	UINT8 color, region;

	bitmap_fill(bitmap, cliprect, 0);

	count = (state->crtc_vreg[0x0c]<<8)|(state->crtc_vreg[0x0d] & 0xff);

	if(state->flip_scr) { count = 0x3ff - count; }

	for(y=0;y<state->crtc_vreg[6];y++)
	{
		for(x=0;x<state->crtc_vreg[1];x++)
		{
			tile = state->videoram[count] + ((state->colorram[count] & 0xe0) << 3);
			color = state->colorram[count] & 0x1f;
			region = (state->colorram[count] & 0x10) >> 4;

			drawgfx_opaque(bitmap,cliprect,screen->machine->gfx[region],tile,color,state->flip_scr,state->flip_scr,x*8,y*8);

			count = (state->flip_scr) ? count-1 : count+1;
			count&=0x3ff;
		}
	}

	return 0;
}
