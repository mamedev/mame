// license:BSD-3-Clause
// copyright-holders:Farfetch'd, David Haywood
#include "emu.h"
#include "includes/gstriker.h"



/*** VIDEO UPDATE/START **********************************************/

void gstriker_state::video_start()
{
	// Initialize the chip for the score plane
	m_tx->set_transparent_pen(0xf);

	// Initialize the chip for the screen plane
	m_bg->set_transparent_pen(0xf);

	m_buffered_spriteram = std::make_unique<uint16_t[]>(0x2000);
	m_buffered_spriteram2 = std::make_unique<uint16_t[]>(0x2000);
	save_pointer(NAME(m_buffered_spriteram), 0x2000);
	save_pointer(NAME(m_buffered_spriteram2), 0x2000);
}

WRITE_LINE_MEMBER(gstriker_state::screen_vblank)
{
	// sprites are two frames ahead
	// TODO: probably all Video System games are (Aero Fighters definitely desyncs wrt background)
	if(state)
	{
		memcpy(m_buffered_spriteram.get(), m_CG10103_m_vram, 0x2000);
		memcpy(m_buffered_spriteram2.get(), m_buffered_spriteram.get(), 0x2000);
	}
}


uint32_t gstriker_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_mixerregs[8]&0x07ff, cliprect); // complete guess, causes green behind test grid in twc94 and blue behind title screen on gstriker

	/*
	[4] AAAA BBBB ---- ---- sprite priority number A/B?
	[5] xxxx ---- ---- ---- background layer priority number?
	[6] xxxx ---- ---- ---- foreground layer priority number?
	*/
//  popmessage("%04x %04x %04x %04x %04x %04x %04x %04x | %04x %04x %04x %04x %04x %04x %04x %04x", m_mixerregs[0],m_mixerregs[1],m_mixerregs[2],m_mixerregs[3],m_mixerregs[4],m_mixerregs[5],m_mixerregs[6],m_mixerregs[7],  m_mixerregs[8],m_mixerregs[9],m_mixerregs[10],m_mixerregs[11],m_mixerregs[12],m_mixerregs[13],m_mixerregs[14],m_mixerregs[15] );

	m_spr->set_pal_base( (m_mixerregs[0]&0xf000)>>8 );
	m_bg->set_pal_base( (m_mixerregs[1]&0xf000)>>8 );
	m_tx->set_pal_base( (m_mixerregs[2]&0xf000)>>8 );


	// Sandwitched screen/sprite0/score/sprite1. Surely wrong, probably needs sprite orthogonality
	m_bg->draw( screen, bitmap,cliprect, 0);

	m_spr->draw_sprites(m_buffered_spriteram2.get(), 0x2000, screen, bitmap, cliprect, 0x2, 0x0);

	m_tx->draw(screen, bitmap, cliprect, 0);

	m_spr->draw_sprites(m_buffered_spriteram2.get(), 0x2000, screen, bitmap, cliprect, 0x2, 0x2);

	return 0;
}
