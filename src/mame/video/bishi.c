/***************************************************************************

 Bishi Bashi Champ Mini Game Senshuken
 (c) 1996 Konami

 Video hardware emulation.

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/bishi.h"


void bishi_tile_callback( running_machine &machine, int layer, int *code, int *color, int *flags )
{
	bishi_state *state = machine.driver_data<bishi_state>();

//  *code -= '0';
//  *color = state->m_layer_colorbase[layer] | (*color>>2 & 0x0f);
//  K055555GX_decode_vmixcolor(layer, color);
//  if (*color) mame_printf_debug("plane %x col %x [55 %x %x]\n", layer, *color, layer_colorbase[layer], K055555_get_palette_index(layer));

	*color = state->m_layer_colorbase[layer] + ((*color & 0xf0));
}

void bishi_state::video_start()
{
	assert(machine().primary_screen->format() == BITMAP_FORMAT_RGB32);

	k056832_set_layer_association(m_k056832, 0);

	k056832_set_layer_offs(m_k056832, 0, -2, 0);
	k056832_set_layer_offs(m_k056832, 1,  2, 0);
	k056832_set_layer_offs(m_k056832, 2,  4, 0);
	k056832_set_layer_offs(m_k056832, 3,  6, 0);

	// the 55555 is set to "0x10, 0x11, 0x12, 0x13", but these values are almost correct...
	m_layer_colorbase[0] = 0x00;
	m_layer_colorbase[1] = 0x40;    // this one is wrong
	m_layer_colorbase[2] = 0x80;
	m_layer_colorbase[3] = 0xc0;
}

UINT32 bishi_state::screen_update_bishi(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int layers[4], layerpri[4], i;/*, old;*/
/*  int bg_colorbase, new_colorbase, plane, dirty; */
	static const int pris[4] = { K55_PRIINP_0, K55_PRIINP_3, K55_PRIINP_6, K55_PRIINP_7 };
	static const int enables[4] = { K55_INP_VRAM_A, K55_INP_VRAM_B, K55_INP_VRAM_C, K55_INP_VRAM_D };

	k054338_update_all_shadows(m_k054338, 0);
	k054338_fill_backcolor(m_k054338, bitmap, 0);

	for (i = 0; i < 4; i++)
	{
		layers[i] = i;
		layerpri[i] = k055555_read_register(m_k055555, pris[i]);
	}

	konami_sortlayers4(layers, layerpri);

	machine().priority_bitmap.fill(0, cliprect);

	for (i = 0; i < 4; i++)
	{
		if (k055555_read_register(m_k055555, K55_INPUT_ENABLES) & enables[layers[i]])
		{
			k056832_tilemap_draw(m_k056832, bitmap, cliprect, layers[i], 0, 1 << i);
		}
	}
	return 0;
}
