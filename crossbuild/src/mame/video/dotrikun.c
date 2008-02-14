/***************************************************************************

Dottori Kun (Head On's mini game)
(c)1990 SEGA

Driver by Takahiro Nogi (nogi@kt.rim.or.jp) 1999/12/15 -

***************************************************************************/

#include "driver.h"



static UINT8 color;


WRITE8_HANDLER( dotrikun_color_w )
{
	color = data;

	video_screen_update_partial(0, video_screen_get_vpos(0));
}


VIDEO_UPDATE( dotrikun )
{
	int offs;

	pen_t back_pen = MAKE_RGB(pal1bit(color >> 3), pal1bit(color >> 4), pal1bit(color >> 5));
	pen_t fore_pen = MAKE_RGB(pal1bit(color >> 0), pal1bit(color >> 1), pal1bit(color >> 2));

	for (offs = 0; offs < videoram_size; offs++)
	{
		int i;
		UINT8 data = videoram[offs];

		UINT8 x = offs << 4;
		UINT8 y = offs >> 4 << 1;

		for (i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x80) ? fore_pen : back_pen;

			/* I think the video hardware doubles pixels, screen would be too small otherwise */
			*BITMAP_ADDR32(bitmap, y + 0, x + 0) = pen;
			*BITMAP_ADDR32(bitmap, y + 0, x + 1) = pen;
			*BITMAP_ADDR32(bitmap, y + 1, x + 0) = pen;
			*BITMAP_ADDR32(bitmap, y + 1, x + 1) = pen;

			x = x + 2;
			data = data << 1;
		}
	}

	return 0;
}
