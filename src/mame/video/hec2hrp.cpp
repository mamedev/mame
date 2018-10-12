// license:BSD-3-Clause
// copyright-holders:JJ Stacino
/////////////////////////////////////////////////////////////////////////////////
///// Hector video
/////////////////////////////////////////////////////////////////////////////////
/*      Hector 2HR+
        Victor
        Hector 2HR
        Hector HRX
        Hector MX40c
        Hector MX80c
        Hector 1
        Interact

        12/05/2009 Skeleton driver - Micko : mmicko@gmail.com
        31/06/2009 Video - Robbbert

        29/10/2009 Update skeleton to functional machine
                    by yo_fr            (jj.stac @ aliceadsl.fr)

                => add Keyboard,
                => add color,
                => add cassette,
                => add sn76477 sound and 1bit sound,
                => add joysticks (stick, pot, fire)
                => add BR/HR switching
                => add bank switch for HRX
                => add device MX80c and bank switching for the ROM
    Important note : the keyboard function has been taken from the
                    DChector project : http://dchector.free.fr/ made by DanielCoulom
                    (thanks Daniel)
    TODO :  Add the cartridge function,
            Adjust the one shot and A/D timing (sn76477)
*/

#include "emu.h"
#include "includes/hec2hrp.h"

#include "screen.h"


void hec2hrp_state::init_palette()
{
	m_hector_color[0] = 0; // black
	m_hector_color[1] = 1; // red
	m_hector_color[2] = 7; // white
	m_hector_color[3] = 3; // yellow

	// Full brightness
	m_palette->set_pen_color( 0,rgb_t(000,000,000)); // black
	m_palette->set_pen_color( 1,rgb_t(255,000,000)); // red
	m_palette->set_pen_color( 2,rgb_t(000,255,000)); // green
	m_palette->set_pen_color( 3,rgb_t(255,255,000)); // yellow
	m_palette->set_pen_color( 4,rgb_t(000,000,255)); // blue
	m_palette->set_pen_color( 5,rgb_t(255,000,255)); // magenta
	m_palette->set_pen_color( 6,rgb_t(000,255,255)); // cyan
	m_palette->set_pen_color( 7,rgb_t(255,255,255)); // white

	// Half brightness
	m_palette->set_pen_color( 8,rgb_t(000,000,000));  // black
	m_palette->set_pen_color( 9,rgb_t(128,000,000));  // red
	m_palette->set_pen_color( 10,rgb_t(000,128,000)); // green
	m_palette->set_pen_color( 11,rgb_t(128,128,000)); // yellow
	m_palette->set_pen_color( 12,rgb_t(000,000,128)); // blue
	m_palette->set_pen_color( 13,rgb_t(128,000,128)); // magenta
	m_palette->set_pen_color( 14,rgb_t(000,128,128)); // cyan
	m_palette->set_pen_color( 15,rgb_t(128,128,128)); // white
}

void hec2hrp_state::hector_hr(bitmap_ind16 &bitmap, uint8_t *page, int ymax, int yram)
{
	uint8_t *hector_color = m_hector_color;
	int sy = 0;
	int ma = 0;
	for (int y = 0; y <= ymax; y++)
	{
		uint16_t *pix = &bitmap.pix16(sy++);
		for (int x = ma; x < ma + yram; x++)
		{
			uint8_t gfx = *(page + x);
			*pix++ = hector_color[(gfx >> 0) & 0x03];
			*pix++ = hector_color[(gfx >> 2) & 0x03];
			*pix++ = hector_color[(gfx >> 4) & 0x03];
			*pix++ = hector_color[(gfx >> 6) & 0x03];
		}
		ma+=yram;
	}
}

void hec2hrp_state::hector_80c(bitmap_ind16 &bitmap, uint8_t *page, int ymax, int yram)
{
	int sy = 0;
	int ma = 0;
	for (int y = 0; y <= ymax; y++)
	{
		uint16_t *pix = &bitmap.pix16(sy++);
		for (int x = ma; x < ma + yram; x++)
		{
			uint8_t gfx = *(page + x);
			*pix++ = (gfx & 0x01) ? 7 : 0;
			*pix++ = (gfx & 0x02) ? 7 : 0;
			*pix++ = (gfx & 0x04) ? 7 : 0;
			*pix++ = (gfx & 0x08) ? 7 : 0;
			*pix++ = (gfx & 0x10) ? 7 : 0;
			*pix++ = (gfx & 0x20) ? 7 : 0;
			*pix++ = (gfx & 0x40) ? 7 : 0;
			*pix++ = (gfx & 0x80) ? 7 : 0;
		}
		ma += yram;
	}
}


VIDEO_START_MEMBER(hec2hrp_state,hec2hrp)
{
	init_palette();
}

uint32_t hec2hrp_state::screen_update_hec2hrp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *videoram = m_videoram;
	uint8_t *videoram_HR = m_hector_videoram;
	if (m_hector_flag_hr==1)
	{
		if (m_hector_flag_80c==0)
		{
			screen.set_visible_area(0, 243, 0, 227);
			hector_hr(bitmap , &videoram_HR[0], 227, 64);
		}
		else
		{
			screen.set_visible_area(0, 243*2, 0, 227);
			hector_80c(bitmap , &videoram_HR[0], 227, 64);
		}
	}
	else
	{
		screen.set_visible_area(0, 113, 0, 75);
		hector_hr(bitmap, videoram,  77, 32);
	}
	return 0;
}
