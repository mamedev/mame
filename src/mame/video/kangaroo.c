/***************************************************************************

    Sun Electronics Kangaroo hardware

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "kangaroo.h"


UINT8 *kangaroo_video_control;


static void blitter_execute(void);



/*************************************
 *
 *  Video setup
 *
 *************************************/

VIDEO_START( kangaroo )
{
	videoram = auto_malloc(256 * 256);
	state_save_register_global_pointer(videoram, 256 * 256);
}



/*************************************
 *
 *  Video control writes
 *
 *************************************/

WRITE8_HANDLER( kangaroo_video_control_w )
{
	kangaroo_video_control[offset] = data;
	
	switch (offset)
	{
		case 5:	/* blitter start */
			blitter_execute();
			break;
	
		case 8:	/* bank select */
			memory_set_bank(1, (data & 0x05) ? 0 : 1);
			break;
	}
}



/*************************************
 *
 *  DMA blitter
 *
 *************************************/

static void blitter_execute(void)
{
	int src,dest;
	int x,y,width,height,old_bank_select,new_bank_select;

	src = kangaroo_video_control[0] + 256 * kangaroo_video_control[1];
	dest = kangaroo_video_control[2] + 256 * kangaroo_video_control[3];

	width = kangaroo_video_control[5];
	height = kangaroo_video_control[4];

	old_bank_select = new_bank_select = kangaroo_video_control[8];

	if (new_bank_select & 0x0c)  new_bank_select |= 0x0c;
	if (new_bank_select & 0x03)  new_bank_select |= 0x03;

	kangaroo_video_control_w(8, new_bank_select & 0x05);

	for (x = 0; x <= width; x++)
	{
		for (y = 0; y <= height; y++)
			program_write_byte(dest + y, program_read_byte(src++));

		dest += 256;
	}

	src = kangaroo_video_control[0] + 256 * kangaroo_video_control[1];
	dest = kangaroo_video_control[2] + 256 * kangaroo_video_control[3];

	kangaroo_video_control_w(8, new_bank_select & 0x0a);

	for (x = 0; x <= width; x++)
	{
		for (y = 0; y <= height; y++)
			program_write_byte(dest + y, program_read_byte(src++));

		dest += 256;
	}

	kangaroo_video_control_w(8, old_bank_select);
}



/*************************************
 *
 *  Video RAM writes
 *
 *************************************/

WRITE8_HANDLER( kangaroo_videoram_w )
{
	int sx, sy, offs;

	sx = (offset / 256) * 4;
	sy = offset % 256;
	offs = sy * 256 + sx;
	
	/* rearrange the bits to a more convenient pattern (from DCBADCBA to DDCCBBAA) */
	data = BITSWAP8(data, 7,3,6,2,5,1,4,0);

	/* B layer, green & blue bits */
	if (kangaroo_video_control[8] & 0x08)
	{
		videoram[offs  ] = (videoram[offs  ] & 0xcf) | (((data >> 0) & 3) << 4);
		videoram[offs+1] = (videoram[offs+1] & 0xcf) | (((data >> 2) & 3) << 4);
		videoram[offs+2] = (videoram[offs+2] & 0xcf) | (((data >> 4) & 3) << 4);
		videoram[offs+3] = (videoram[offs+3] & 0xcf) | (((data >> 6) & 3) << 4);
	}

	/* B layer, Z & red bits */
	if (kangaroo_video_control[8] & 0x04)
	{
		videoram[offs  ] = (videoram[offs  ] & 0x3f) | (((data >> 0) & 3) << 6);
		videoram[offs+1] = (videoram[offs+1] & 0x3f) | (((data >> 2) & 3) << 6);
		videoram[offs+2] = (videoram[offs+2] & 0x3f) | (((data >> 4) & 3) << 6);
		videoram[offs+3] = (videoram[offs+3] & 0x3f) | (((data >> 6) & 3) << 6);
	}

	/* A layer, green & blue bits */
	if (kangaroo_video_control[8] & 0x02)
	{
		videoram[offs  ] = (videoram[offs  ] & 0xfc) | (((data >> 0) & 3) << 0);
		videoram[offs+1] = (videoram[offs+1] & 0xfc) | (((data >> 2) & 3) << 0);
		videoram[offs+2] = (videoram[offs+2] & 0xfc) | (((data >> 4) & 3) << 0);
		videoram[offs+3] = (videoram[offs+3] & 0xfc) | (((data >> 6) & 3) << 0);
	}

	/* A layer, Z & red bits */
	if (kangaroo_video_control[8] & 0x01)
	{
		videoram[offs  ] = (videoram[offs  ] & 0xf3) | (((data >> 0) & 3) << 2);
		videoram[offs+1] = (videoram[offs+1] & 0xf3) | (((data >> 2) & 3) << 2);
		videoram[offs+2] = (videoram[offs+2] & 0xf3) | (((data >> 4) & 3) << 2);
		videoram[offs+3] = (videoram[offs+3] & 0xf3) | (((data >> 6) & 3) << 2);
	}
}



/*************************************
 *
 *  Video updater
 *
 *************************************/

VIDEO_UPDATE( kangaroo )
{
	UINT8 scrolly = kangaroo_video_control[6];
	UINT8 scrollx = kangaroo_video_control[7];
	UINT8 maska = kangaroo_video_control[10] >> 3;
	UINT8 maskb = kangaroo_video_control[10] >> 0;
	UINT8 xora = (kangaroo_video_control[9] & 0x20) ? 0xff : 0x00;
	UINT8 xorb = (kangaroo_video_control[9] & 0x10) ? 0xff : 0x00;
	UINT8 enaa = (kangaroo_video_control[9] & 0x08);
	UINT8 enab = (kangaroo_video_control[9] & 0x04);
	UINT8 pria = (~kangaroo_video_control[9] & 0x02);
	UINT8 prib = (~kangaroo_video_control[9] & 0x01);
	rgb_t pens[8];
	int x, y;
	
	/* build up the pens arrays */
	for (x = 0; x < 8; x++)
		pens[x] = MAKE_RGB(pal1bit(x >> 2), pal1bit(x >> 1), pal1bit(x >> 0));

	/* iterate over pixels */	
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT32 *dest = BITMAP_ADDR32(bitmap, y, 0);
		
		for (x = cliprect->min_x; x <= cliprect->max_x; x += 2)
		{
			UINT8 effxa = scrollx + ((x / 2) ^ xora);
			UINT8 effya = scrolly + (y ^ xora);
			UINT8 effxb = (x / 2) ^ xorb;
			UINT8 effyb = y ^ xorb;
			UINT8 pixa = videoram[effya * 256 + effxa] & 0x0f;
			UINT8 pixb = videoram[effyb * 256 + effxb] >> 4;
			UINT8 finalpens;
			
			finalpens = 0;
			if (enaa && (pria || pixb == 0))
				finalpens |= pixa;
			if (enab && (prib || pixa == 0))
				finalpens |= pixb;

			dest[x + 0] = pens[finalpens & 7];

			/* KOS1 alternates at 5MHz, offset from the pixel clock by 1/2 clock */
			/* when 0, it enables the color mask for pixels with Z = 0 */
			finalpens = 0;
			if (enaa && (pria || pixb == 0))
			{
				if (!(pixa & 0x08)) pixa &= ~maska;
				finalpens |= pixa;
			}
			if (enab && (prib || pixa == 0))
			{
				if (!(pixb & 0x08)) pixb &= ~maskb;
				finalpens |= pixb;
			}

			dest[x + 1] = pens[finalpens & 7];
		}
	}
	
	return 0;
}
