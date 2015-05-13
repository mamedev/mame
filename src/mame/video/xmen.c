// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/xmen.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(xmen_state::tile_callback)
{
	/* (color & 0x02) is flip y handled internally by the 052109 */
	if (layer == 0)
		*color = m_layer_colorbase[layer] + ((*color & 0xf0) >> 4);
	else
		*color = m_layer_colorbase[layer] + ((*color & 0x7c) >> 2);
}

/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

K053246_CB_MEMBER(xmen_state::sprite_callback)
{
	int pri = (*color & 0x00e0) >> 4;   /* ??????? */

	if (pri <= m_layerpri[2])
		*priority_mask = 0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;

	*color = m_sprite_colorbase + (*color & 0x001f);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(xmen_state,xmen6p)
{
	m_k053246->k053247_get_ram( &m_k053247_ram);

	m_screen_left  = auto_bitmap_ind16_alloc(machine(), 64 * 8, 32 * 8);
	m_screen_right = auto_bitmap_ind16_alloc(machine(), 64 * 8, 32 * 8);

	save_item(NAME(*m_screen_left));
	save_item(NAME(*m_screen_right));
}


/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 xmen_state::screen_update_xmen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layer[3], bg_colorbase;

	bg_colorbase = m_k053251->get_palette_index(K053251_CI4);
	m_sprite_colorbase = m_k053251->get_palette_index(K053251_CI1);
	m_layer_colorbase[0] = m_k053251->get_palette_index(K053251_CI3);
	m_layer_colorbase[1] = m_k053251->get_palette_index(K053251_CI0);
	m_layer_colorbase[2] = m_k053251->get_palette_index(K053251_CI2);

	m_k052109->tilemap_update();

	layer[0] = 0;
	m_layerpri[0] = m_k053251->get_priority(K053251_CI3);
	layer[1] = 1;
	m_layerpri[1] = m_k053251->get_priority(K053251_CI0);
	layer[2] = 2;
	m_layerpri[2] = m_k053251->get_priority(K053251_CI2);

	konami_sortlayers3(layer, m_layerpri);

	screen.priority().fill(0, cliprect);
	/* note the '+1' in the background color!!! */
	bitmap.fill(16 * bg_colorbase + 1, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[0], 0, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

/* this isn't supported anymore and it is unsure if still needed; keeping here for reference
    pdrawgfx_shadow_lowpri = 1; fix shadows of boulders in front of feet */
	m_k053246->k053247_sprites_draw( bitmap, cliprect);
	return 0;
}


UINT32 xmen_state::screen_update_xmen6p_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;

	for(y = 0; y < 32 * 8; y++)
	{
		UINT16* line_dest = &bitmap.pix16(y);
		UINT16* line_src = &m_screen_left->pix16(y);

		for (x = 12 * 8; x < 52 * 8; x++)
			line_dest[x] = line_src[x];
	}

	return 0;
}

UINT32 xmen_state::screen_update_xmen6p_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;

	for(y = 0; y < 32 * 8; y++)
	{
		UINT16* line_dest = &bitmap.pix16(y);
		UINT16* line_src = &m_screen_right->pix16(y);

		for (x = 12 * 8; x < 52 * 8; x++)
			line_dest[x] = line_src[x];
	}

	return 0;
}

/* my lefts and rights are mixed up in several places.. */
void xmen_state::screen_eof_xmen6p(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		int layer[3], bg_colorbase;
		bitmap_ind16 * renderbitmap;
		rectangle cliprect;
		int offset;

	//  const rectangle *visarea = m_screen->visible_area();
	//  cliprect = *visarea;

		cliprect.set(0, 64 * 8 - 1, 2 * 8, 30 * 8 - 1);


		address_space &space = machine().driver_data()->generic_space();
		if (m_screen->frame_number() & 0x01)
		{
			/* copy the desired spritelist to the chip */
			memcpy(m_k053247_ram, m_xmen6p_spriteramright, 0x1000);

			/* we write the entire content of the tileram to the chip to ensure
			   everything gets marked as dirty and the desired tilemap is rendered
			   this is not very efficient!
			   */
			for (offset = 0; offset < (0xc000 / 2); offset++)
			{
	//          m_k052109->lsb_w
				m_k052109->write(space, offset, m_xmen6p_tilemapright[offset] & 0x00ff);
			}


			renderbitmap = m_screen_right;
		}
		else
		{
			/* copy the desired spritelist to the chip */
			memcpy(m_k053247_ram, m_xmen6p_spriteramleft, 0x1000);

			/* we write the entire content of the tileram to the chip to ensure
			   everything gets marked as dirty and the desired tilemap is rendered

			   this is not very efficient!
			   */
			for (offset = 0; offset < (0xc000 / 2); offset++)
			{
	//          m_k052109->lsb_w
				m_k052109->write(space, offset, m_xmen6p_tilemapleft[offset] & 0x00ff);
			}


			renderbitmap = m_screen_left;
		}


		bg_colorbase = m_k053251->get_palette_index(K053251_CI4);
		m_sprite_colorbase = m_k053251->get_palette_index(K053251_CI1);
		m_layer_colorbase[0] = m_k053251->get_palette_index(K053251_CI3);
		m_layer_colorbase[1] = m_k053251->get_palette_index(K053251_CI0);
		m_layer_colorbase[2] = m_k053251->get_palette_index(K053251_CI2);

		m_k052109->tilemap_update();

		layer[0] = 0;
		m_layerpri[0] = m_k053251->get_priority(K053251_CI3);
		layer[1] = 1;
		m_layerpri[1] = m_k053251->get_priority(K053251_CI0);
		layer[2] = 2;
		m_layerpri[2] = m_k053251->get_priority(K053251_CI2);

		konami_sortlayers3(layer, m_layerpri);

		m_screen->priority().fill(0, cliprect);
		/* note the '+1' in the background color!!! */
		renderbitmap->fill(16 * bg_colorbase + 1, cliprect);
		m_k052109->tilemap_draw(m_screen, *renderbitmap, cliprect, layer[0], 0, 1);
		m_k052109->tilemap_draw(m_screen, *renderbitmap, cliprect, layer[1], 0, 2);
		m_k052109->tilemap_draw(m_screen, *renderbitmap, cliprect, layer[2], 0, 4);

	/* this isn't supported anymore and it is unsure if still needed; keeping here for reference
	    pdrawgfx_shadow_lowpri = 1; fix shadows of boulders in front of feet */
		m_k053246->k053247_sprites_draw( *renderbitmap, cliprect);
	}
}
