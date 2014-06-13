#include "emu.h"
#include "includes/gstriker.h"



/*** VIDEO UPDATE/START **********************************************/



UINT32 gstriker_state::screen_update_gstriker(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	
	// Sandwitched screen/sprite0/score/sprite1. Surely wrong, probably
	//  needs sprite orthogonality
	m_bg->draw( screen, bitmap,cliprect, 0);

	m_spr->draw_sprites(m_CG10103_m_vram, 0x2000, screen, bitmap, cliprect, 0x2, 0x0);

	m_tx->draw( screen, bitmap, cliprect, 0);

	m_spr->draw_sprites(m_CG10103_m_vram, 0x2000, screen, bitmap, cliprect, 0x2, 0x2);

	return 0;
}

VIDEO_START_MEMBER(gstriker_state,gstriker)
{
	// Palette bases are hardcoded, but should be probably extracted from the mixer registers

	// Initalize the chip for the score plane
	m_tx->set_gfx_region(0);
	m_tx->set_pal_base(0x30);
	m_tx->get_tilemap()->set_transparent_pen(0xf);

	// Initalize the chip for the screen plane
	m_bg->set_gfx_region( 1);
	m_bg->set_pal_base( 0);
	m_bg->get_tilemap()->set_transparent_pen(0xf);
}

VIDEO_START_MEMBER(gstriker_state,twrldc94)
{
	// Palette bases are hardcoded, but should be probably extracted from the mixer registers

	// Initalize the chip for the score plane
	m_tx->set_gfx_region(0);
	m_tx->set_pal_base(0x40);
	m_tx->get_tilemap()->set_transparent_pen(0xf);

	// Initalize the chip for the screen plane
	m_bg->set_gfx_region( 1);
	m_bg->set_pal_base( 0x50);
	m_bg->get_tilemap()->set_transparent_pen(0xf);
}

VIDEO_START_MEMBER(gstriker_state,vgoalsoc)
{
	// Palette bases are hardcoded, but should be probably extracted from the mixer registers

	// Initalize the chip for the score plane
	m_tx->set_gfx_region(0);
	m_tx->set_pal_base(0x30);
	m_tx->get_tilemap()->set_transparent_pen(0xf);

	// Initalize the chip for the screen plane
	m_bg->set_gfx_region( 1);
	m_bg->set_pal_base( 0x20);
	m_bg->get_tilemap()->set_transparent_pen(0xf);
}
