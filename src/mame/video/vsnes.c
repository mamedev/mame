// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
#include "emu.h"
#include "video/ppu2c0x.h"
#include "includes/vsnes.h"


PALETTE_INIT_MEMBER(vsnes_state,vsnes)
{
	m_ppu1->init_palette_rgb(palette, 0);
}

PALETTE_INIT_MEMBER(vsnes_state,vsdual)
{
	m_ppu1->init_palette_rgb(palette, 0);
	m_ppu2->init_palette_rgb(palette, 8 * 4 * 16);
}

void vsnes_state::ppu_irq_1(int *ppu_regs)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

void vsnes_state::ppu_irq_2(int *ppu_regs)
{
	m_subcpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

VIDEO_START_MEMBER(vsnes_state,vsnes )
{
}

VIDEO_START_MEMBER(vsnes_state,vsdual )
{
}

/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 vsnes_state::screen_update_vsnes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* render the ppu */
	m_ppu1->render(bitmap, 0, 0, 0, 0);
	return 0;
}

UINT32 vsnes_state::screen_update_vsnes_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_ppu2->render(bitmap, 0, 0, 0, 0);
	return 0;
}
