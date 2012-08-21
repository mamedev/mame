/***************************************************************************

  PeT mess@utanet.at

***************************************************************************/

#include "emu.h"
#include "includes/cbmb.h"
#include "video/mc6845.h"



VIDEO_START( cbmb_crtc )
{
}

void cbm600_vh_init(running_machine &machine)
{
	UINT8 *gfx = machine.root_device().memregion("gfx1")->base();
	int i;

	/* inversion logic on board */
	for (i=0; i<0x800; i++) {
		gfx[0x1000+i]=gfx[0x800+i];
		gfx[0x1800+i]=gfx[0x1000+i]^0xff;
		gfx[0x800+i]=gfx[i]^0xff;
	}
}

void cbm700_vh_init(running_machine &machine)
{
	UINT8 *gfx = machine.root_device().memregion("gfx1")->base();
	int i;
	for (i=0; i<0x800; i++) {
		gfx[0x1000+i]=gfx[0x800+i];
		gfx[0x1800+i]=gfx[0x1000+i]^0xff;
		gfx[0x800+i]=gfx[i]^0xff;
	}
}

VIDEO_START( cbm700 )
{
	int i;

    /* remove pixel column 9 for character codes 0 - 175 and 224 - 255 */
	for( i = 0; i < 256; i++)
	{
//      if( i < 176 || i > 223 )
		{
			int y;
			for( y = 0; y < machine.gfx[0]->height; y++ ) {
				machine.gfx[0]->gfxdata[(i * machine.gfx[0]->height + y) * machine.gfx[0]->width + 8] = 0;
				machine.gfx[1]->gfxdata[(i * machine.gfx[1]->height + y) * machine.gfx[1]->width + 8] = 0;
			}
		}
	}
}

void cbmb_vh_set_font(running_machine &machine, int font)
{
	cbmb_state *state = machine.driver_data<cbmb_state>();
	state->m_font=font;
}

MC6845_UPDATE_ROW( cbm600_update_row )
{
	cbmb_state *state = device->machine().driver_data<cbmb_state>();
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 *videoram = state->m_videoram;
	int i;

	for( i = 0; i < x_count; i++ ) {
		if ( i == cursor_x ) {
			bitmap.plot_box( device->machine().gfx[state->m_font]->width * i, y, device->machine().gfx[state->m_font]->width, 1, palette[1] );
		} else {
			drawgfx_opaque( bitmap, cliprect, device->machine().gfx[state->m_font], videoram[(ma+i )& 0x7ff], 0, 0, 0, device->machine().gfx[state->m_font]->width * i, y-ra );
		}
	}
}

MC6845_UPDATE_ROW( cbm700_update_row )
{
	cbmb_state *state = device->machine().driver_data<cbmb_state>();
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 *videoram = state->m_videoram;
	int i;

	for( i = 0; i < x_count; i++ ) {
		if ( i == cursor_x ) {
			bitmap.plot_box( device->machine().gfx[state->m_font]->width * i, y, device->machine().gfx[state->m_font]->width, 1, palette[1] );
		} else {
			drawgfx_opaque( bitmap, cliprect, device->machine().gfx[state->m_font], videoram[(ma+i) & 0x7ff], 0, 0, 0, device->machine().gfx[state->m_font]->width * i, y-ra );
		}
	}
}

WRITE_LINE_DEVICE_HANDLER( cbmb_display_enable_changed )
{
}

