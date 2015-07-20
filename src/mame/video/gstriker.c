// license:BSD-3-Clause
// copyright-holders:Farfetch'd, David Haywood
#include "emu.h"
#include "includes/gstriker.h"



/*** VIDEO UPDATE/START **********************************************/



UINT32 gstriker_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_mixerregs1[8]&0x07ff, cliprect); // complete guess, causes green behind test grid in twc94 and blue behind title screen on gstriker

	popmessage("%04x %04x %04x %04x %04x %04x %04x %04x | %04x %04x %04x %04x %04x %04x %04x %04x", m_mixerregs1[0],m_mixerregs1[1],m_mixerregs1[2],m_mixerregs1[3],m_mixerregs1[4],m_mixerregs1[5],m_mixerregs1[6],m_mixerregs1[7],  m_mixerregs1[8],m_mixerregs1[9],m_mixerregs1[10],m_mixerregs1[11],m_mixerregs1[12],m_mixerregs1[13],m_mixerregs1[14],m_mixerregs1[15] );
//  popmessage("%04x %04x %04x %04x %04x %04x %04x %04x | %04x %04x %04x %04x %04x %04x %04x %04x", m_mixerregs2[0],m_mixerregs2[1],m_mixerregs2[2],m_mixerregs2[3],m_mixerregs2[4],m_mixerregs2[5],m_mixerregs2[6],m_mixerregs2[7],  m_mixerregs2[8],m_mixerregs2[9],m_mixerregs2[10],m_mixerregs2[11],m_mixerregs2[12],m_mixerregs2[13],m_mixerregs2[14],m_mixerregs2[15] );

	m_spr->set_pal_base( (m_mixerregs1[0]&0xf000)>>8 );
	m_bg->set_pal_base( (m_mixerregs1[1]&0xf000)>>8 );
	m_tx->set_pal_base( (m_mixerregs1[2]&0xf000)>>8 );


	// Sandwitched screen/sprite0/score/sprite1. Surely wrong, probably
	//  needs sprite orthogonality
	m_bg->draw( screen, bitmap,cliprect, 0);

	m_spr->draw_sprites(m_CG10103_m_vram, 0x2000, screen, bitmap, cliprect, 0x2, 0x0);

	m_tx->draw( screen, bitmap, cliprect, 0);

	m_spr->draw_sprites(m_CG10103_m_vram, 0x2000, screen, bitmap, cliprect, 0x2, 0x2);

	return 0;
}

void gstriker_state::video_start()
{
	// Initalize the chip for the score plane
	m_tx->get_tilemap()->set_transparent_pen(0xf);

	// Initalize the chip for the screen plane
	m_bg->get_tilemap()->set_transparent_pen(0xf);
}
