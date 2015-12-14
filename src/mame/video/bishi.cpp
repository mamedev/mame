// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

 Bishi Bashi Champ Mini Game Senshuken
 (c) 1996 Konami

 Video hardware emulation.

***************************************************************************/

#include "emu.h"
#include "includes/bishi.h"


#if 0
K056832_CB_MEMBER(bishi_state::tile_callback)
{
//  *code -= '0';
//  *color = m_layer_colorbase[layer] | (*color>>2 & 0x0f);
//  K055555GX_decode_vmixcolor(layer, color);
//  if (*color) osd_printf_debug("plane %x col %x [55 %x %x]\n", layer, *color, layer_colorbase[layer], K055555_get_palette_index(layer));

	*color = m_layer_colorbase[layer] + ((*color & 0xf0));
}
#endif

void bishi_state::video_start()
{
	assert(m_screen->format() == BITMAP_FORMAT_RGB32);

#if 0
	m_k056832->set_layer_association(0);

	m_k056832->set_layer_offs(0, -2, 0);
	m_k056832->set_layer_offs(1,  2, 0);
	m_k056832->set_layer_offs(2,  4, 0);
	m_k056832->set_layer_offs(3,  6, 0);
#endif

	// the 55555 is set to "0x10, 0x11, 0x12, 0x13", but these values are almost correct...
	m_layer_colorbase[0] = 0x00;
	m_layer_colorbase[1] = 0x40;    // this one is wrong
	m_layer_colorbase[2] = 0x80;
	m_layer_colorbase[3] = 0xc0;
}

uint32_t bishi_state::screen_update_bishi(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
#if 0
	int layers[4], layerpri[4], i;/*, old;*/
/*  int bg_colorbase, new_colorbase, plane, dirty; */
	static const int pris[4] = { K55_PRIINP_0, K55_PRIINP_3, K55_PRIINP_6, K55_PRIINP_7 };
	static const int enables[4] = { K55_INP_VRAM_A, K55_INP_VRAM_B, K55_INP_VRAM_C, K55_INP_VRAM_D };

	m_k054338->update_all_shadows(0, *m_palette);
	m_k054338->fill_solid_bg(bitmap, cliprect);

	for (i = 0; i < 4; i++)
	{
		layers[i] = i;
		layerpri[i] = m_k055555->K055555_read_register(pris[i]);
	}

	konami_sortlayers4(layers, layerpri);

	screen.priority().fill(0, cliprect);

	for (i = 0; i < 4; i++)
	{
		if (m_k055555->K055555_read_register(K55_INPUT_ENABLES) & enables[layers[i]])
		{
			m_k056832->tilemap_draw(screen, bitmap, cliprect, layers[i], 0, 1 << i);
		}
	}
#endif

	return 0;
}
