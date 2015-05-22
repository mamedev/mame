// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "taito_helper.h"

/* These scanline drawing routines, currently used by the pc080sn, tc0080vco, tc0150rod and tc0480scp devices, were lifted from Taito F3: optimise / merge ? */


void taitoic_drawscanline( bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y,
		const UINT16 *src, int transparent, UINT32 orient, bitmap_ind8 &priority, int pri)
{
	UINT16 *dsti = &bitmap.pix16(y, x);
	UINT8 *dstp = &priority.pix8(y, x);
	int length = cliprect.width();

	src += cliprect.min_x;
	dsti += cliprect.min_x;
	dstp += cliprect.min_x;
	if (transparent)
	{
		while (length--)
		{
			UINT32 spixel = *src++;

			if (spixel < 0x7fff)
			{
				*dsti = spixel;
				*dstp = pri;
			}

			dsti++;
			dstp++;
		}
	}
	else    /* Not transparent case */
	{
		while (length--)
		{
			*dsti++ = *src++;
			*dstp++ = pri;
		}
	}
}
