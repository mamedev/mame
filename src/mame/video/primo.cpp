// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/***************************************************************************

  primo.c

  Functions to emulate the video hardware of Primo.

  Krzysztof Strzecha

***************************************************************************/

#include "emu.h"
#include "includes/primo.h"



void primo_state::primo_draw_scanline(bitmap_ind16 &bitmap, int primo_scanline)
{
	int x, i;
	uint8_t data;

	/* set up scanline */
	uint16_t *scanline = &bitmap.pix16(primo_scanline);

	/* address of current line in Primo video memory */
	const uint8_t* primo_video_ram_line = &m_video_ram[m_video_memory_base + 32 * primo_scanline];

	for (x=0; x<256; x+=8)
	{
		data = primo_video_ram_line[x/8];

		for (i=0; i<8; i++)
			scanline[x+i]=(data & (0x80>>i)) ? 1 : 0;

	}
}


uint32_t primo_state::screen_update_primo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int primo_scanline;

	for (primo_scanline=0; primo_scanline<192; primo_scanline++)
		primo_draw_scanline( bitmap, primo_scanline);
	return 0;
}
