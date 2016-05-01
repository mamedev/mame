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
    Importante note : the keyboard function add been piked from
                    DChector project : http://dchector.free.fr/ made by DanielCoulom
                    (thank's Daniel)
    TODO :  Add the cartridge function,
            Adjust the one shot and A/D timing (sn76477)
*/

#include "emu.h"

#include "includes/hec2hrp.h"


void hec2hrp_state::Init_Hector_Palette()
{
	UINT8 *hector_color = m_hector_color;
	// basic colors !
	hector_color[0] = 0;  // fond (noir)
	hector_color[1] = 1;  // HECTOR HRX (rouge)
	hector_color[2] = 7; // Point interrogation (Blanc)
	hector_color[3] = 3; // Ecriture de choix (jaune)

	// Color initialisation : full lightning
	m_palette->set_pen_color( 0,rgb_t(000,000,000));//Noir
	m_palette->set_pen_color( 1,rgb_t(255,000,000));//Rouge
	m_palette->set_pen_color( 2,rgb_t(000,255,000));//Vert
	m_palette->set_pen_color( 3,rgb_t(255,255,000));//Jaune
	m_palette->set_pen_color( 4,rgb_t(000,000,255));//Bleu
	m_palette->set_pen_color( 5,rgb_t(255,000,255));//Magneta
	m_palette->set_pen_color( 6,rgb_t(000,255,255));//Cyan
	m_palette->set_pen_color( 7,rgb_t(255,255,255));//Blanc
	// 1/2 lightning

	m_palette->set_pen_color( 8,rgb_t(000,000,000));//Noir
	m_palette->set_pen_color( 9,rgb_t(128,000,000));//Rouge
	m_palette->set_pen_color( 10,rgb_t(000,128,000));//Vert
	m_palette->set_pen_color( 11,rgb_t(128,128,000));//Jaune
	m_palette->set_pen_color( 12,rgb_t(000,000,128));//Bleu
	m_palette->set_pen_color( 13,rgb_t(128,000,128));//Magneta
	m_palette->set_pen_color( 14,rgb_t(000,128,128));//Cyan
	m_palette->set_pen_color( 15,rgb_t(128,128,128));//Blanc
}

void hec2hrp_state::hector_hr(bitmap_ind16 &bitmap, UINT8 *page, int ymax, int yram)
{
	UINT8 *hector_color = m_hector_color;
	UINT8 gfx,y;
	UINT16 sy=0,ma=0,x;
	for (y = 0; y <= ymax; y++) {  //224
		UINT16  *p = &bitmap.pix16(sy++);
		for (x = ma; x < ma + yram; x++) {  // 64
			gfx = *(page+x);
			/* Display a scanline of a character (4 pixels !) */
			*p++ = hector_color[(gfx >> 0) & 0x03];
			*p++ = hector_color[(gfx >> 2) & 0x03];
			*p++ = hector_color[(gfx >> 4) & 0x03];
			*p++ = hector_color[(gfx >> 6) & 0x03];
		}
		ma+=yram;
	}
}

void hec2hrp_state::hector_80c(bitmap_ind16 &bitmap, UINT8 *page, int ymax, int yram)
{
	UINT8 gfx,y;
	UINT16 sy=0,ma=0,x;
	for (y = 0; y <= ymax; y++) {  //224
		UINT16  *p = &bitmap.pix16(sy++);
		for (x = ma; x < ma + yram; x++) {  // 64
			gfx = *(page+x);
			/* Display a scanline of a character (8 pixels !) */
			*p++ = (gfx & 0x01) ? 7 : 0;
			*p++ = (gfx & 0x02) ? 7 : 0;
			*p++ = (gfx & 0x04) ? 7 : 0;
			*p++ = (gfx & 0x08) ? 7 : 0;
			*p++ = (gfx & 0x10) ? 7 : 0;
			*p++ = (gfx & 0x20) ? 7 : 0;
			*p++ = (gfx & 0x40) ? 7 : 0;
			*p++ = (gfx & 0x80) ? 7 : 0;
		}
		ma+=yram;
	}
}


VIDEO_START_MEMBER(hec2hrp_state,hec2hrp)
{
	Init_Hector_Palette();
}

UINT32 hec2hrp_state::screen_update_hec2hrp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;
	UINT8 *videoram_HR = m_hector_videoram;
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
