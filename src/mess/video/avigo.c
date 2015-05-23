// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/***************************************************************************

  avigo.c

  Functions to emulate the video hardware of the TI Avigo 10 PDA

***************************************************************************/

#include "emu.h"
#include "includes/avigo.h"

/***************************************************************************
  Start the video hardware emulation.
***************************************************************************/

/* mem size = 0x017c0 */


/* current column to read/write */

#define AVIGO_VIDEO_DEBUG 0
#define LOG(x) do { if (AVIGO_VIDEO_DEBUG) logerror x; } while (0)


READ8_MEMBER(avigo_state::vid_memory_r)
{
	if (!offset)
		return m_screen_column;

	if ((offset<0x0100) || (offset>=0x01f0) || (m_screen_column >= (AVIGO_SCREEN_WIDTH>>3)))
	{
		LOG(("vid mem read: %04x\n", offset));
		return 0;
	}

	/* 0x0100-0x01f0 contains data for selected column */
	return m_video_memory[m_screen_column + ((offset&0xff)*(AVIGO_SCREEN_WIDTH>>3))];
}

WRITE8_MEMBER(avigo_state::vid_memory_w)
{
	if (!offset)
	{
		/* select column to read/write */
		m_screen_column = data;

		LOG(("vid mem column write: %02x\n",data));

		if (data>=(AVIGO_SCREEN_WIDTH>>3))
		{
			LOG(("error: vid mem column write: %02x\n",data));
		}
		return;
	}

	if ((offset<0x0100) || (offset>=0x01f0) || (m_screen_column >= (AVIGO_SCREEN_WIDTH>>3)))
	{
		LOG(("vid mem write: %04x %02x\n", offset, data));
		return;
	}

	/* 0x0100-0x01f0 contains data for selected column */
	m_video_memory[m_screen_column + ((offset&0xff)*(AVIGO_SCREEN_WIDTH>>3))] = data;
}

void avigo_state::video_start()
{
	/* current selected column to read/write */
	m_screen_column = 0;

	/* allocate video memory */
	m_video_memory = machine().memory().region_alloc( "videoram", (AVIGO_SCREEN_WIDTH>>3) * AVIGO_SCREEN_HEIGHT + 1, 1, ENDIANNESS_LITTLE )->base();
	memset(m_video_memory, 0, (AVIGO_SCREEN_WIDTH>>3) * AVIGO_SCREEN_HEIGHT + 1);

	save_pointer(NAME(m_video_memory), (AVIGO_SCREEN_WIDTH>>3) * AVIGO_SCREEN_HEIGHT + 1);
}

/* Initialise the palette */
PALETTE_INIT_MEMBER(avigo_state, avigo)
{
	m_palette->set_pen_color(0,rgb_t(0xff,0xff,0xff)); /* white  */
	m_palette->set_pen_color(1,rgb_t(0x00,0x00,0x00)); /* black  */
}

UINT32 avigo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y;
	int b;
	int x;

	/* draw avigo display */
	for (y=0; y<AVIGO_SCREEN_HEIGHT; y++)
	{
		int by;
		UINT8 *line_ptr = m_video_memory + (y*(AVIGO_SCREEN_WIDTH>>3));

		x = 0;
		for (by=((AVIGO_SCREEN_WIDTH>>3)-1); by>=0; by--)
		{
			int px;
			UINT8 byte = line_ptr[0];

			px = x;
			for (b=7; b>=0; b--)
			{
				bitmap.pix16(y, px) = ((byte>>7) & 0x01);
				px++;
				byte = byte<<1;
			}

			x = px;
			line_ptr = line_ptr+1;
		}
	}
	return 0;
}
