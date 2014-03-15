/***************************************************************************

        Radio-86RK video driver by Miodrag Milanovic

        06/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "video/i8275.h"
#include "includes/radio86.h"

I8275_DISPLAY_PIXELS(radio86_display_pixels)
{
	radio86_state *state = device->machine().driver_data<radio86_state>();
	int i;
	const rgb_t *palette = state->m_palette->palette()->entry_list_raw();
	const UINT8 *charmap = state->m_charmap;
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
	const rgb_t *palette = state->m_palette->palette()->entry_list_raw();
	const UINT8 *charmap = state->m_charmap + (state->m_mikrosha_font_page & 1) * 0x400;
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
	const rgb_t *palette = state->m_palette->palette()->entry_list_raw();
	const UINT8 *charmap = state->m_charmap + (gpa & 1) * 0x400;
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
	const rgb_t *palette = state->m_palette->palette()->entry_list_raw();
	const UINT8 *charmap = state->m_charmap + 0x400 * (gpa * 2 + hlgt);
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

static const rgb_t radio86_palette[3] = {
	rgb_t(0x00, 0x00, 0x00), // black
	rgb_t(0xa0, 0xa0, 0xa0), // white
	rgb_t(0xff, 0xff, 0xff)  // highlight
};

PALETTE_INIT_MEMBER(radio86_state,radio86)
{
	palette.set_pen_colors(0, radio86_palette, ARRAY_LENGTH(radio86_palette));
}

void radio86_state::video_start()
{
	m_charmap = memregion("gfx1")->base();
}
