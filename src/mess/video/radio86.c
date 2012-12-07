/***************************************************************************

        Radio-86RK video driver by Miodrag Milanovic

        06/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "video/i8275.h"
#include "includes/radio86.h"

void radio86_state::video_start()
{
	machine().primary_screen->register_screen_bitmap(m_bitmap);
}

UINT32 radio86_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

I8275_DISPLAY_PIXELS(radio86_display_pixels)
{
	radio86_state *state = device->machine().driver_data<radio86_state>();
	int i;
	bitmap_rgb32 &bitmap = state->m_bitmap;
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 *charmap = state->memregion("gfx1")->base();
	UINT8 pixels = charmap[(linecount & 7) + (charcode << 3)] ^ 0xff;
	if (vsp) {
		pixels = 0;
	}
	if (lten) {
		pixels = 0xff;
	}
	if (rvv) {
		pixels ^= 0xff;
	}
	for(i=0;i<6;i++) {
		bitmap.pix32(y, x + i) = palette[(pixels >> (5-i)) & 1 ? (hlgt ? 2 : 1) : 0];
	}
}


I8275_DISPLAY_PIXELS(mikrosha_display_pixels)
{
	radio86_state *state = device->machine().driver_data<radio86_state>();
	int i;
	bitmap_rgb32 &bitmap = state->m_bitmap;
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 *charmap = state->memregion("gfx1")->base() + (state->m_mikrosha_font_page & 1) * 0x400;
	UINT8 pixels = charmap[(linecount & 7) + (charcode << 3)] ^ 0xff;
	if (vsp) {
		pixels = 0;
	}
	if (lten) {
		pixels = 0xff;
	}
	if (rvv) {
		pixels ^= 0xff;
	}
	for(i=0;i<6;i++) {
		bitmap.pix32(y, x + i) = palette[(pixels >> (5-i)) & 1 ? (hlgt ? 2 : 1) : 0];
	}
}

I8275_DISPLAY_PIXELS(apogee_display_pixels)
{
	radio86_state *state = device->machine().driver_data<radio86_state>();
	int i;
	bitmap_rgb32 &bitmap = state->m_bitmap;
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 *charmap = state->memregion("gfx1")->base() + (gpa & 1) * 0x400;
	UINT8 pixels = charmap[(linecount & 7) + (charcode << 3)] ^ 0xff;
	if (vsp) {
		pixels = 0;
	}
	if (lten) {
		pixels = 0xff;
	}
	if (rvv) {
		pixels ^= 0xff;
	}
	for(i=0;i<6;i++) {
		bitmap.pix32(y, x + i) = palette[(pixels >> (5-i)) & 1 ? (hlgt ? 2 : 1) : 0];
	}
}

I8275_DISPLAY_PIXELS(partner_display_pixels)
{
	radio86_state *state = device->machine().driver_data<radio86_state>();
	int i;
	bitmap_rgb32 &bitmap = state->m_bitmap;
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 *charmap = state->memregion("gfx1")->base() + 0x400 * (gpa * 2 + hlgt);
	UINT8 pixels = charmap[(linecount & 7) + (charcode << 3)] ^ 0xff;
	if (vsp) {
		pixels = 0;
	}
	if (lten) {
		pixels = 0xff;
	}
	if (rvv) {
		pixels ^= 0xff;
	}
	for(i=0;i<6;i++) {
		bitmap.pix32(y, x + i) = palette[(pixels >> (5-i)) & 1];
	}
}

UINT32 radio86_state::screen_update_radio86(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	device_t *devconf = machine().device("i8275");
	i8275_update( devconf, bitmap, cliprect);
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

static const rgb_t radio86_palette[3] = {
	MAKE_RGB(0x00, 0x00, 0x00), // black
	MAKE_RGB(0xa0, 0xa0, 0xa0), // white
	MAKE_RGB(0xff, 0xff, 0xff)	// highlight
};

PALETTE_INIT_MEMBER(radio86_state,radio86)
{
	palette_set_colors(machine(), 0, radio86_palette, ARRAY_LENGTH(radio86_palette));
}

