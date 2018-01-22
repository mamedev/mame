// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina,Pierpaolo Prazzoli
#include "emu.h"
#include "includes/eolith.h"


WRITE32_MEMBER(eolith_state::eolith_vram_w)
{
	uint32_t *dest = &m_vram[offset+(0x40000/4)*m_buffer];

	if (mem_mask == 0xffffffff)
	{
		// candy needs this to always write to RAM (verified that certain glitches, for example the high score table, don't occur on real hw)
		// other games clearly don't.
		// is there a cpu bug, or is there more to this logic / a flag which disables it?

		if (~data & 0x80000000)
			*dest = (*dest & 0x0000ffff) | (data & 0xffff0000);

		if (~data & 0x00008000)
			*dest = (*dest & 0xffff0000) | (data & 0x0000ffff);
	}
	else if (((mem_mask == 0xffff0000) && (~data & 0x80000000)) ||
				((mem_mask == 0x0000ffff) && (~data & 0x00008000)))
		COMBINE_DATA(dest);

}


READ32_MEMBER(eolith_state::eolith_vram_r)
{
	return m_vram[offset+(0x40000/4)*m_buffer];
}

VIDEO_START_MEMBER(eolith_state,eolith)
{
	m_vram = std::make_unique<uint32_t[]>(0x40000*2/4);
	save_pointer(NAME(m_vram.get()), 0x40000*2/4);
	save_item(NAME(m_buffer));
}

uint32_t eolith_state::screen_update_eolith(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 240; y++)
	{
		uint32_t *src = &m_vram[(m_buffer ? 0 : 0x10000) | (y * (336 / 2))];
		uint16_t *dest = &bitmap.pix16(y);

		for (int x = 0; x < 320; x += 2)
		{
			dest[0] = (*src >> 16) & 0x7fff;
			dest[1] = (*src >>  0) & 0x7fff;

			src++;
			dest += 2;
		}
	}

	return 0;
}
