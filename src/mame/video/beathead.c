/***************************************************************************

    Atari "Stella on Steroids" hardware

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#include "emu.h"
#include "includes/beathead.h"



/*************************************
 *
 *  Video start/stop
 *
 *************************************/

void beathead_state::video_start()
{
	state_save_register_global(machine(), m_finescroll);
	state_save_register_global(machine(), m_vram_latch_offset);
	state_save_register_global(machine(), m_hsyncram_offset);
	state_save_register_global(machine(), m_hsyncram_start);
	state_save_register_global_array(machine(), m_hsyncram);
}



/*************************************
 *
 *  VRAM handling
 *
 *************************************/

WRITE32_MEMBER( beathead_state::vram_transparent_w )
{
	/* writes to this area appear to handle transparency */
	if (!(data & 0x000000ff)) mem_mask &= ~0x000000ff;
	if (!(data & 0x0000ff00)) mem_mask &= ~0x0000ff00;
	if (!(data & 0x00ff0000)) mem_mask &= ~0x00ff0000;
	if (!(data & 0xff000000)) mem_mask &= ~0xff000000;
	COMBINE_DATA(&m_videoram[offset]);
}


WRITE32_MEMBER( beathead_state::vram_bulk_w )
{
	/* it appears that writes to this area pass in a mask for 4 words in VRAM */
	/* allowing them to be filled from a preset latch */
	offset &= ~3;
	data = data & mem_mask & 0x0f0f0f0f;

	/* for now, just handle the bulk fill case; the others we'll catch later */
	if (data == 0x0f0f0f0f)
		m_videoram[offset+0] = m_videoram[offset+1] = m_videoram[offset+2] = m_videoram[offset+3] = *m_vram_bulk_latch;
	else
		logerror("Detected bulk VRAM write with mask %08x\n", data);
}


WRITE32_MEMBER( beathead_state::vram_latch_w )
{
	/* latch the address */
	m_vram_latch_offset = (4 * offset) & 0x7ffff;
}


WRITE32_MEMBER( beathead_state::vram_copy_w )
{
	/* copy from VRAM to VRAM, for 1024 bytes */
	offs_t dest_offset = (4 * offset) & 0x7ffff;
	memcpy(&m_videoram[dest_offset / 4], &m_videoram[m_vram_latch_offset / 4], 0x400);
}



/*************************************
 *
 *  Scroll offset handling
 *
 *************************************/

WRITE32_MEMBER( beathead_state::finescroll_w )
{
	UINT32 oldword = m_finescroll;
	UINT32 newword = COMBINE_DATA(&m_finescroll);

	/* if VBLANK is going off on a scanline other than the last, suspend time */
	if ((oldword & 8) && !(newword & 8) && space.machine().primary_screen->vpos() != 261)
	{
		logerror("Suspending time! (scanline = %d)\n", space.machine().primary_screen->vpos());
		space.machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
}



/*************************************
 *
 *  Palette handling
 *
 *************************************/

WRITE32_MEMBER( beathead_state::palette_w )
{
	int newword = COMBINE_DATA(&m_paletteram[offset]);
	int r = ((newword >> 9) & 0x3e) | ((newword >> 15) & 0x01);
	int g = ((newword >> 4) & 0x3e) | ((newword >> 15) & 0x01);
	int b = ((newword << 1) & 0x3e) | ((newword >> 15) & 0x01);
	palette_set_color_rgb(space.machine(), offset, pal6bit(r), pal6bit(g), pal6bit(b));
}



/*************************************
 *
 *  HSYNC RAM handling
 *
 *************************************/

READ32_MEMBER( beathead_state::hsync_ram_r )
{
	/* offset 0 is probably write-only */
	if (offset == 0)
		logerror("%08X:Unexpected HSYNC RAM read at offset 0\n", space.device().safe_pcbase());

	/* offset 1 reads the data */
	else
		return m_hsyncram[m_hsyncram_offset];

	return 0;
}

WRITE32_MEMBER( beathead_state::hsync_ram_w )
{
	/* offset 0 selects the address, and can specify the start address */
	if (offset == 0)
	{
		COMBINE_DATA(&m_hsyncram_offset);
		if (m_hsyncram_offset & 0x800)
			m_hsyncram_start = m_hsyncram_offset & 0x7ff;
	}

	/* offset 1 writes the data */
	else
		COMBINE_DATA(&m_hsyncram[m_hsyncram_offset]);
}



/*************************************
 *
 *  Main screen refresher
 *
 *************************************/

UINT32 beathead_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = reinterpret_cast<UINT8 *>(m_videoram.target());
	int x, y;

	/* generate the final screen */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		pen_t pen_base = (*m_palette_select & 0x7f) * 256;
		UINT16 scanline[336];

		/* blanking */
		if (m_finescroll & 8)
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
				scanline[x] = pen_base;

		/* non-blanking */
		else
		{
			offs_t scanline_offset = m_vram_latch_offset + (m_finescroll & 3);
			offs_t src = scanline_offset + cliprect.min_x;

			/* unswizzle the scanline first */
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
				scanline[x] = pen_base | videoram[BYTE4_XOR_LE(src++)];
		}

		/* then draw it */
		draw_scanline16(bitmap, cliprect.min_x, y, cliprect.width(), &scanline[cliprect.min_x], NULL);
	}
	return 0;
}
