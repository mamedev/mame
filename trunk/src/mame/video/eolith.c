#include "emu.h"
#include "includes/eolith.h"


WRITE32_MEMBER(eolith_state::eolith_vram_w)
{
	UINT32 *dest = &m_vram[offset+(0x40000/4)*m_buffer];

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

VIDEO_START( eolith )
{
	eolith_state *state = machine.driver_data<eolith_state>();
	state->m_vram = auto_alloc_array(machine, UINT32, 0x40000*2/4);
}

SCREEN_UPDATE_IND16( eolith )
{
	eolith_state *state = screen.machine().driver_data<eolith_state>();
	int y;

	for (y = 0; y < 240; y++)
	{
		int x;
		UINT32 *src = &state->m_vram[(state->m_buffer ? 0 : 0x10000) | (y * (336 / 2))];
		UINT16 *dest = &bitmap.pix16(y);

		for (x = 0; x < 320; x += 2)
		{
			dest[0] = (*src >> 16) & 0x7fff;
			dest[1] = (*src >>  0) & 0x7fff;

			src++;
			dest += 2;
		}
	}

	return 0;
}
