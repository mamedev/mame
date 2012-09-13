/***************************************************************************
 *  Sharp MZ700
 *
 *  video hardware
 *
 *  Juergen Buchmueller <pullmoll@t-online.de>, Jul 2000
 *
 *  Reference: http://sharpmz.computingmuseum.com
 *
 ***************************************************************************/

#include "emu.h"
#include "machine/pit8253.h"
#include "includes/mz700.h"


#ifndef VERBOSE
#define VERBOSE 1
#endif

#define LOG(N,M,A)	\
	do { \
		if(VERBOSE>=N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s",machine.time().as_double(),(char*)M ); \
			logerror A; \
		} \
	} while (0)


void mz_state::palette_init()
{
	int i;

	machine().colortable = colortable_alloc(machine(), 8);

	for (i = 0; i < 8; i++)
		colortable_palette_set_color(machine().colortable, i, MAKE_RGB((i & 2) ? 0xff : 0x00, (i & 4) ? 0xff : 0x00, (i & 1) ? 0xff : 0x00));

	for (i = 0; i < 256; i++)
	{
		colortable_entry_set_value(machine().colortable, i*2, i & 7);
        	colortable_entry_set_value(machine().colortable, i*2+1, (i >> 4) & 7);
	}
}


SCREEN_UPDATE_IND16( mz700 )
{
	mz_state *state = screen.machine().driver_data<mz_state>();
	UINT8 *videoram = state->m_videoram;
	int offs;
	mz_state *mz = screen.machine().driver_data<mz_state>();

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	for(offs = 0; offs < 40*25; offs++)
	{
		int sx, sy, code, color;

		sy = (offs / 40) * 8;
		sx = (offs % 40) * 8;

		color = mz->m_colorram[offs];
		code = videoram[offs] | (color & 0x80) << 1;

		drawgfx_opaque(bitmap, cliprect, screen.machine().gfx[0], code, color, 0, 0, sx, sy);
	}

	return 0;
}


/***************************************************************************
    MZ-800
***************************************************************************/

VIDEO_START_MEMBER(mz_state,mz800)
{
	machine().gfx[0]->set_source(m_cgram);
}

SCREEN_UPDATE_IND16( mz800 )
{
	mz_state *state = screen.machine().driver_data<mz_state>();
	UINT8 *videoram = state->m_videoram;
	mz_state *mz = screen.machine().driver_data<mz_state>();

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	if (mz->m_mz700_mode)
		return SCREEN_UPDATE16_CALL(mz700);
	else
	{
		if (mz->m_hires_mode)
		{

		}
		else
		{
			int x, y;
			UINT8 *start_addr = videoram;

			for (x = 0; x < 40; x++)
			{
				for (y = 0; y < 200; y++)
				{
					bitmap.pix16(y, x * 8 + 0) = BIT(start_addr[x * 8 + y], 0);
					bitmap.pix16(y, x * 8 + 1) = BIT(start_addr[x * 8 + y], 1);
					bitmap.pix16(y, x * 8 + 2) = BIT(start_addr[x * 8 + y], 2);
					bitmap.pix16(y, x * 8 + 3) = BIT(start_addr[x * 8 + y], 3);
					bitmap.pix16(y, x * 8 + 4) = BIT(start_addr[x * 8 + y], 4);
					bitmap.pix16(y, x * 8 + 5) = BIT(start_addr[x * 8 + y], 5);
					bitmap.pix16(y, x * 8 + 6) = BIT(start_addr[x * 8 + y], 6);
					bitmap.pix16(y, x * 8 + 7) = BIT(start_addr[x * 8 + y], 7);
				}
			}
		}

		return 0;
	}
}

/***************************************************************************
    CGRAM
***************************************************************************/

WRITE8_MEMBER(mz_state::mz800_cgram_w)
{
	m_cgram[offset] = data;

	machine().gfx[0]->mark_dirty(offset/8);
}
