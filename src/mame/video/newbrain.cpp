// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "includes/newbrain.h"
#include "rendlay.h"
#include "newbrain.lh"

void newbrain_state::video_start()
{
	/* register for state saving */
	save_item(NAME(m_tvcnsl));
	save_item(NAME(m_tvctl));
	save_item(NAME(m_tvram));
	save_item(NAME(m_segment_data));
}

void newbrain_state::screen_update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	int y, sx;
	int columns = (m_tvctl & NEWBRAIN_VIDEO_80L) ? 80 : 40;
	int excess = (m_tvctl & NEWBRAIN_VIDEO_32_40) ? 24 : 4;
	int ucr = (m_tvctl & NEWBRAIN_VIDEO_UCR) ? 1 : 0;
	int fs = (m_tvctl & NEWBRAIN_VIDEO_FS) ? 1 : 0;
	int rv = (m_tvctl & NEWBRAIN_VIDEO_RV) ? 1 : 0;
	int gr = 0;

	UINT16 videoram_addr = m_tvram;
	UINT8 rc = 0;

	for (y = 0; y < 250; y++)
	{
		int x = 0;

		for (sx = 0; sx < columns; sx++)
		{
			int bit;

			UINT8 videoram_data = program.read_byte(videoram_addr + sx);
			UINT8 charrom_data;

			if (gr)
			{
				/* render video ram data */
				charrom_data = videoram_data;
			}
			else
			{
				/* render character rom data */
				UINT16 charrom_addr = (rc << 8) | ((BIT(videoram_data, 7) & fs) << 7) | (videoram_data & 0x7f);
				charrom_data = m_char_rom->base()[charrom_addr & 0xfff];

				if ((videoram_data & 0x80) && !fs)
				{
					/* invert character */
					charrom_data ^= 0xff;
				}

				if ((videoram_data & 0x60) && !ucr)
				{
					/* strip bit D0 */
					charrom_data &= 0xfe;
				}
			}

			for (bit = 0; bit < 8; bit++)
			{
				int color = BIT(charrom_data, 7) ^ rv;

				bitmap.pix32(y, x++) = m_palette->pen(color);

				if (columns == 40)
				{
					bitmap.pix32(y, x++) = m_palette->pen(color);
				}

				charrom_data <<= 1;
			}
		}

		if (gr)
		{
			/* get new data for each line */
			videoram_addr += columns;
			videoram_addr += excess;
		}
		else
		{
			/* increase row counter */
			rc++;

			if (rc == (ucr ? 8 : 10))
			{
				/* reset row counter */
				rc = 0;

				videoram_addr += columns;
				videoram_addr += excess;
			}
		}
	}
}

UINT32 newbrain_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_enrg1 & NEWBRAIN_ENRG1_TVP)
	{
		screen_update(bitmap, cliprect);
	}
	else
	{
		bitmap.fill(rgb_t::black, cliprect);
	}

	return 0;
}

/* Machine Drivers */

MACHINE_CONFIG_FRAGMENT( newbrain_video )
	MCFG_DEFAULT_LAYOUT(layout_newbrain)

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(newbrain_state, screen_update)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 250)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 249)

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")
MACHINE_CONFIG_END
