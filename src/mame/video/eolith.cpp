// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina,Pierpaolo Prazzoli
#include "emu.h"
#include "includes/eolith.h"


WRITE16_MEMBER(eolith_state::eolith_vram_w)
{
	if ((mem_mask == 0xffff) && (~data & 0x8000))
	{
		// candy needs this to always write to RAM (verified that certain glitches, for example the high score table, don't occur on real hw)
		// other games clearly don't.
		// is there a cpu bug, or is there more to this logic / a flag which disables it?
		COMBINE_DATA(&m_vram[offset+(0x40000/2)*m_buffer]);
	}
}


READ16_MEMBER(eolith_state::eolith_vram_r)
{
	return m_vram[offset+(0x40000/2)*m_buffer];
}

VIDEO_START_MEMBER(eolith_state,eolith)
{
	m_vram = std::make_unique<uint16_t[]>(0x40000);
	save_pointer(NAME(m_vram), 0x40000);
	save_item(NAME(m_buffer));
}

uint32_t eolith_state::screen_update_eolith(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 240; y++)
	{
		for (int x = 0; x < 320; x++)
		{
			bitmap.pix16(y, x) = m_vram[(0x40000/2) * (m_buffer ^ 1) + (y * 336) + x] & 0x7fff;
		}
	}

	return 0;
}
