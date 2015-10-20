// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/rollerg.h"

/***************************************************************************

  Callbacks for the K053245

***************************************************************************/

K05324X_CB_MEMBER(rollerg_state::sprite_callback)
{
	enum { sprite_colorbase = 256 / 16 };
#if 0
	if (machine().input().code_pressed(KEYCODE_Q) && (*color & 0x80)) *color = rand();
	if (machine().input().code_pressed(KEYCODE_W) && (*color & 0x40)) *color = rand();
	if (machine().input().code_pressed(KEYCODE_E) && (*color & 0x20)) *color = rand();
	if (machine().input().code_pressed(KEYCODE_R) && (*color & 0x10)) *color = rand();
#endif
	*priority = (*color & 0x10) ? 0 : 0x02;
	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

K051316_CB_MEMBER(rollerg_state::zoom_callback)
{
	*flags = TILE_FLIPYX((*color & 0xc0) >> 6);
	*code |= ((*color & 0x0f) << 8);
	*color = ((*color & 0x30) >> 4);
}


/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 rollerg_state::screen_update_rollerg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int bg_colorbase = 16;

	screen.priority().fill(0, cliprect);
	bitmap.fill(16 * bg_colorbase, cliprect);
	m_k051316->zoom_draw(screen, bitmap, cliprect, 0, 1);
	m_k053244->sprites_draw(bitmap, cliprect, screen.priority());
	return 0;
}
