/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/

#include "driver.h"
#include "mw8080bw.h"



VIDEO_UPDATE( mw8080bw )
{
	UINT8 x = 0;
	UINT8 y = MW8080BW_VCOUNTER_START_NO_VBLANK;
	UINT8 video_data = 0;

	while (1)
	{
		/* plot the current pixel */
		pen_t pen = (video_data & 0x01) ? RGB_WHITE : RGB_BLACK;
		*BITMAP_ADDR32(bitmap, y - MW8080BW_VCOUNTER_START_NO_VBLANK, x) = pen;

		/* next pixel */
		video_data = video_data >> 1;
		x = x + 1;

		/* end of line? */
		if (x == 0)
		{
			/* yes, flush out the shift register */
			int i;

			for (i = 0; i < 4; i++)
			{
				pen = (video_data & 0x01) ? RGB_WHITE : RGB_BLACK;
				*BITMAP_ADDR32(bitmap, y - MW8080BW_VCOUNTER_START_NO_VBLANK, 256 + i) = pen;

				video_data = video_data >> 1;
			}

			/* next row, video_data is now 0, so the next line will start
               with 4 blank pixels */
			y = y + 1;

			/* end of screen? */
			if (y == 0)
				break;
		}
		/* the video RAM is read at every 8 pixels starting with pixel 4 */
		else if ((x & 0x07) == 0x04)
		{
			offs_t offs = ((offs_t)y << 5) | (x >> 3);
			video_data = mw8080bw_ram[offs];
		}
	}

	return 0;
}



/*************************************
 *
 *  Space Encounters
 *
 *************************************/


#define PHANTOM2_BOTTOM_TRENCH_DARK_RGB32_PEN	RGB_BLACK
#define PHANTOM2_BOTTOM_TRENCH_LIGHT_RGB32_PEN	MAKE_RGB(0x5a, 0x5a, 0x5a)
#define PHANTOM2_TOP_TRENCH_DARK_RGB32_PEN		RGB_BLACK
#define PHANTOM2_TOP_TRENCH_LIGHT_RGB32_PEN		RGB_WHITE
#define PHANTOM2_SIDE_TRENCH_DARK_RGB32_PEN		RGB_BLACK
#define PHANTOM2_SIDE_TRENCH_LIGHT_RGB32_PEN	MAKE_RGB(0x72, 0x72, 0x72)


VIDEO_UPDATE( spcenctr )
{
	UINT8 line_buf[256]; /* 256x1 bit RAM */

	UINT8 x = 0;
	UINT8 y = MW8080BW_VCOUNTER_START_NO_VBLANK;
	UINT8 video_data = 0;
	UINT8 draw_line = 0;
	UINT8 draw_trench = 0;
	UINT8 draw_floor = 0;
	UINT8 width = spcenctr_get_trench_width();
	UINT8 floor_width = width;
	UINT8 center = spcenctr_get_trench_center();

	memset(line_buf, 0, 256);

	while (1)
	{
		/* plot the current pixel */
		UINT8 bit = video_data & 0x01;
		pen_t pen = bit ? RGB_WHITE : RGB_BLACK;

		/* possibly draw trench in the background, top of trench first */
		if (!(width & 0x80) && draw_trench)
		{
			line_buf[x] = draw_line;

			if (!bit)
				pen = draw_line ? PHANTOM2_TOP_TRENCH_LIGHT_RGB32_PEN : PHANTOM2_TOP_TRENCH_DARK_RGB32_PEN;
		}
		/* sides of trench? */
		else if (!(floor_width & 0x80) && (draw_trench || draw_floor))
		{
			if (!bit)
				pen = line_buf[x] ? PHANTOM2_SIDE_TRENCH_LIGHT_RGB32_PEN : PHANTOM2_SIDE_TRENCH_DARK_RGB32_PEN;
		}
		/* bottom of trench? */
		else if (draw_floor)
		{
			line_buf[x] = line_buf[x - 1];

			if (!bit)
				pen = line_buf[x] ? PHANTOM2_BOTTOM_TRENCH_LIGHT_RGB32_PEN : PHANTOM2_BOTTOM_TRENCH_DARK_RGB32_PEN;
		}

		*BITMAP_ADDR32(bitmap, y - MW8080BW_VCOUNTER_START_NO_VBLANK, x) = pen;

		center = center + 1;
		width = width + ((center & 0x80) ? -1 : 1);
		floor_width = floor_width + ((center & 0x80) ? -1 : 1);

		/* next pixel */
		video_data = video_data >> 1;
		x = x + 1;

		/* end of line? */
		if (x == 0)
		{
			offs_t offs;
			UINT8 trench_control;

			/* yes, flush out the shift register */
			int i;

			for (i = 0; i < 4; i++)
			{
				pen = (video_data & 0x01) ? RGB_WHITE : RGB_BLACK;
				*BITMAP_ADDR32(bitmap, y - MW8080BW_VCOUNTER_START_NO_VBLANK, 256 + i) = pen;

				video_data = video_data >> 1;
			}

			/* update the trench control for the next line */
			offs = ((offs_t)y << 5) | 0x1f;
			trench_control = mw8080bw_ram[offs];

			if (trench_control & 0x40)
				draw_trench = 1;

			if (trench_control & 0x20)
				draw_trench = 0;

			if (trench_control & 0x10)
				draw_floor = 1;

			if (trench_control & 0x08)
				draw_floor = 0;

			draw_line = (trench_control & 0x80) >> 7;

			/* add the lower 2 bits stored in the slope array to width */
			if (draw_trench)
				width = width + (spcenctr_get_trench_slope(y) & 0x03);

			/* add the higher 2 bits stored in the slope array to floor width */
			if (draw_floor)
				floor_width = floor_width + ((spcenctr_get_trench_slope(y) & 0x0c) >> 2);

			/* next row, video_data is now 0, so the next line will start
               with 4 blank pixels */
			y = y + 1;

			/* end of screen? */
			if (y == 0)
				break;
		}
		/* the video RAM is read at every 8 pixels starting with pixel 4 */
		else if ((x & 0x07) == 0x04)
		{
			offs_t offs = ((offs_t)y << 5) | (x >> 3);
			video_data = mw8080bw_ram[offs];
		}
	}

	return 0;
}



/*************************************
 *
 *  Phantom II
 *
 *************************************/


/* the cloud generator comprises of 2 counters and a shift register:

   * counter 1 is 8 bits and and clocked every pixel. It gets cleared at the end of HBLANK .
     Bit 0 is used to clock the shift register, thus repeating every pixel twice.
     Bits 4-7 go to address line A0-A3 of the cloud gfx prom.
   * counter 2 is 12 bits starting from 0xe0b and counts up to 0xfff.  It gets clocked at the
     beginning of HBLANK and is never cleared.
     Bits 1-7 go to address line A4-A10 of the cloud gfx prom.
*/

#define PHANTOM2_CLOUD_COUNTER_START	(0x0e0b)
#define PHANTOM2_CLOUD_COUNTER_END		(0x1000)
#define PHANTOM2_CLOUD_COUNTER_PERIOD	(PHANTOM2_CLOUD_COUNTER_END - PHANTOM2_CLOUD_COUNTER_START)

#define PHANTOM2_RGB32_CLOUD_PEN		MAKE_RGB(0xc0, 0xc0, 0xc0)


VIDEO_UPDATE( phantom2 )
{
	UINT8 x = 0;
	UINT8 y = MW8080BW_VCOUNTER_START_NO_VBLANK;
	UINT8 video_data = 0;
	UINT8 cloud_data = 0;

	UINT16 cloud_counter = phantom2_get_cloud_counter();

	UINT8 *cloud_region = memory_region(REGION_PROMS);

	while (1)
	{
		int load_shift_reg;
		UINT8 cloud_data_to_load = 0;
		pen_t pen;

		/* plot the current pixel */
		UINT8 bit = video_data & 0x01;

		/* if background color, cloud gfx in the background */
		if ((bit == 0) && (cloud_data & 0x01))
			pen = PHANTOM2_RGB32_CLOUD_PEN;
		else
			pen = bit ? RGB_WHITE : RGB_BLACK;

		*BITMAP_ADDR32(bitmap, y - MW8080BW_VCOUNTER_START_NO_VBLANK, x) = pen;

		/* move to next pixel -- if ripple carry is currently set,
           prepare for loading the shift register */
		load_shift_reg = ((x & 0x0f) == 0x0f);

		if (load_shift_reg)
		{
			offs_t cloud_offs = ((cloud_counter & 0xfe) << 3) | (x >> 4);
			cloud_data_to_load = cloud_region[cloud_offs];
		}

		video_data = video_data >> 1;
		x = x + 1;

		/* the sift register is clocked on the falling edge of bit 0 */
		if (!(x & 0x01))
		{
			/* load or shift? */
			if (load_shift_reg)
				cloud_data = cloud_data_to_load;
			else
				cloud_data = cloud_data >> 1;
		}

		/* end of line? */
		if (x == 0)
		{
			/* yes, flush out the shift register */
			int i;

			for (i = 0; i < 4; i++)
			{
				pen = (video_data & 0x01) ? RGB_WHITE : RGB_BLACK;
				*BITMAP_ADDR32(bitmap, y - MW8080BW_VCOUNTER_START_NO_VBLANK, 256 + i) = pen;

				video_data = video_data >> 1;
			}

			/* next row of clouds */
			cloud_counter = cloud_counter + 1;

			if (cloud_counter == PHANTOM2_CLOUD_COUNTER_END)
				cloud_counter = PHANTOM2_CLOUD_COUNTER_START;

			/* next row of pixels, video_data is now 0, so the next
               line will start with 4 blank pixels */
			y = y + 1;

			/* end of screen? */
			if (y == 0)
				break;
		}
		/* the video RAM is read at every 8 pixels starting with pixel 4 */
		else if ((x & 0x07) == 0x04)
		{
			offs_t offs = ((offs_t)y << 5) | (x >> 3);
			video_data = mw8080bw_ram[offs];
		}
	}

	return 0;
}


VIDEO_EOF( phantom2 )
{
	UINT16 cloud_counter = phantom2_get_cloud_counter();

	cloud_counter = cloud_counter + MW8080BW_VTOTAL;

	if (cloud_counter >= PHANTOM2_CLOUD_COUNTER_END)
		cloud_counter = PHANTOM2_CLOUD_COUNTER_START + (cloud_counter - PHANTOM2_CLOUD_COUNTER_END);

	phantom2_set_cloud_counter(cloud_counter);
}



/*************************************
 *
 *  Space Invaders
 *
 *************************************/


/* the flip screen circuit is just a couple of relays on the monitor PCB */

VIDEO_UPDATE( invaders )
{
	UINT8 x = 0;
	UINT8 y = MW8080BW_VCOUNTER_START_NO_VBLANK;
	UINT8 video_data = 0;
	UINT8 flip = invaders_is_flip_screen();

	while (1)
	{
		/* plot the current pixel */
		pen_t pen = (video_data & 0x01) ? RGB_WHITE : RGB_BLACK;

		if (flip)
			*BITMAP_ADDR32(bitmap, MW8080BW_VBSTART - 1 - (y - MW8080BW_VCOUNTER_START_NO_VBLANK), MW8080BW_HPIXCOUNT - 1 - x) = pen;
		else
			*BITMAP_ADDR32(bitmap, y - MW8080BW_VCOUNTER_START_NO_VBLANK, x) = pen;

		/* next pixel */
		video_data = video_data >> 1;
		x = x + 1;

		/* end of line? */
		if (x == 0)
		{
			/* yes, flush out the shift register */
			int i;

			for (i = 0; i < 4; i++)
			{
				pen = (video_data & 0x01) ? RGB_WHITE : RGB_BLACK;

				if (flip)
					*BITMAP_ADDR32(bitmap, MW8080BW_VBSTART - 1 - (y - MW8080BW_VCOUNTER_START_NO_VBLANK), MW8080BW_HPIXCOUNT - 1 - (256 + i)) = pen;
				else
					*BITMAP_ADDR32(bitmap, y - MW8080BW_VCOUNTER_START_NO_VBLANK, 256 + i) = pen;

				video_data = video_data >> 1;
			}

			/* next row, video_data is now 0, so the next line will start
               with 4 blank pixels */
			y = y + 1;

			/* end of screen? */
			if (y == 0)
				break;
		}
		/* the video RAM is read at every 8 pixels starting with pixel 4 */
		else if ((x & 0x07) == 0x04)
		{
			offs_t offs = ((offs_t)y << 5) | (x >> 3);
			video_data = mw8080bw_ram[offs];
		}
	}

	return 0;
}
