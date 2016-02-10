// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni,Lee Taylor, Valerio Verrando, Zsolt Vasvari
// thanks-to:Michael Strutts, Marco Cassili
/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/

#include "emu.h"
#include "includes/mw8080bw.h"


UINT32 mw8080bw_state::screen_update_mw8080bw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 x = 0;
	UINT8 y = MW8080BW_VCOUNTER_START_NO_VBLANK;
	UINT8 video_data = 0;

	while (1)
	{
		/* plot the current pixel */
		pen_t pen = (video_data & 0x01) ? rgb_t::white : rgb_t::black;
		bitmap.pix32(y - MW8080BW_VCOUNTER_START_NO_VBLANK, x) = pen;

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
				pen = (video_data & 0x01) ? rgb_t::white : rgb_t::black;
				bitmap.pix32(y - MW8080BW_VCOUNTER_START_NO_VBLANK, 256 + i) = pen;

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
			video_data = m_main_ram[offs];
		}
	}

	return 0;
}



/*************************************
 *
 *  Space Encounters
 *
 *************************************/


#define PHANTOM2_BOTTOM_TRENCH_DARK_RGB32_PEN    rgb_t::black
#define PHANTOM2_BOTTOM_TRENCH_LIGHT_RGB32_PEN   rgb_t(0x5a, 0x5a, 0x5a)
#define PHANTOM2_TOP_TRENCH_DARK_RGB32_PEN       rgb_t::black
#define PHANTOM2_TOP_TRENCH_LIGHT_RGB32_PEN      rgb_t::white
#define PHANTOM2_SIDE_TRENCH_DARK_RGB32_PEN      rgb_t::black
#define PHANTOM2_SIDE_TRENCH_LIGHT_RGB32_PEN     rgb_t(0x72, 0x72, 0x72)


UINT32 mw8080bw_state::screen_update_spcenctr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 line_buf[256]; /* 256x1 bit RAM */

	UINT8 x = 0;
	UINT8 y = MW8080BW_VCOUNTER_START_NO_VBLANK;
	UINT8 video_data = 0;
	UINT8 draw_line = 0;
	UINT8 draw_trench = 0;
	UINT8 draw_floor = 0;
	UINT8 width = m_spcenctr_trench_width;
	UINT8 floor_width = width;
	UINT8 center = m_spcenctr_trench_center;

	memset(line_buf, 0, 256);

	while (1)
	{
		/* plot the current pixel */
		UINT8 bit = video_data & 0x01;
		pen_t pen = bit ? rgb_t::white : rgb_t::black;

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

		bitmap.pix32(y - MW8080BW_VCOUNTER_START_NO_VBLANK, x) = pen;

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
				pen = (video_data & 0x01) ? rgb_t::white : rgb_t::black;
				bitmap.pix32(y - MW8080BW_VCOUNTER_START_NO_VBLANK, 256 + i) = pen;

				video_data = video_data >> 1;
			}

			/* update the trench control for the next line */
			offs = ((offs_t)y << 5) | 0x1f;
			trench_control = m_main_ram[offs];

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
				width = width + (m_spcenctr_trench_slope[y & 0x0f] & 0x03);

			/* add the higher 2 bits stored in the slope array to floor width */
			if (draw_floor)
				floor_width = floor_width + ((m_spcenctr_trench_slope[y & 0x0f] & 0x0c) >> 2);

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
			video_data = m_main_ram[offs];
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

#define PHANTOM2_CLOUD_COUNTER_START      (0x0e0b)
#define PHANTOM2_CLOUD_COUNTER_END        (0x1000)
#define PHANTOM2_CLOUD_COUNTER_PERIOD     (PHANTOM2_CLOUD_COUNTER_END - PHANTOM2_CLOUD_COUNTER_START)

#define PHANTOM2_RGB32_CLOUD_PEN          rgb_t(0xc0, 0xc0, 0xc0)


UINT32 mw8080bw_state::screen_update_phantom2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 x = 0;
	UINT8 y = MW8080BW_VCOUNTER_START_NO_VBLANK;
	UINT8 video_data = 0;
	UINT8 cloud_data = 0;

	UINT16 cloud_counter = m_phantom2_cloud_counter;

	UINT8 *cloud_region = memregion("proms")->base();

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
			pen = bit ? rgb_t::white : rgb_t::black;

		bitmap.pix32(y - MW8080BW_VCOUNTER_START_NO_VBLANK, x) = pen;

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
				pen = (video_data & 0x01) ? rgb_t::white : rgb_t::black;
				bitmap.pix32(y - MW8080BW_VCOUNTER_START_NO_VBLANK, 256 + i) = pen;

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
			video_data = m_main_ram[offs];
		}
	}

	return 0;
}


void mw8080bw_state::screen_eof_phantom2(screen_device &screen, bool state)
{
	// falling edge
	if (!state)
	{
		m_phantom2_cloud_counter += MW8080BW_VTOTAL;

		if (m_phantom2_cloud_counter >= PHANTOM2_CLOUD_COUNTER_END)
			m_phantom2_cloud_counter = PHANTOM2_CLOUD_COUNTER_START + (m_phantom2_cloud_counter - PHANTOM2_CLOUD_COUNTER_END);
	}
}


/*************************************
 *
 *  Space Invaders
 *
 *************************************/


/* the flip screen circuit is just a couple of relays on the monitor PCB */

UINT32 mw8080bw_state::screen_update_invaders(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 x = 0;
	UINT8 y = MW8080BW_VCOUNTER_START_NO_VBLANK;
	UINT8 video_data = 0;
	UINT8 flip = m_flip_screen;

	while (1)
	{
		/* plot the current pixel */
		pen_t pen = (video_data & 0x01) ? rgb_t::white : rgb_t::black;

		if (flip)
			bitmap.pix32(MW8080BW_VBSTART - 1 - (y - MW8080BW_VCOUNTER_START_NO_VBLANK), MW8080BW_HPIXCOUNT - 1 - x) = pen;
		else
			bitmap.pix32(y - MW8080BW_VCOUNTER_START_NO_VBLANK, x) = pen;

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
				pen = (video_data & 0x01) ? rgb_t::white : rgb_t::black;

				if (flip)
					bitmap.pix32(MW8080BW_VBSTART - 1 - (y - MW8080BW_VCOUNTER_START_NO_VBLANK), MW8080BW_HPIXCOUNT - 1 - (256 + i)) = pen;
				else
					bitmap.pix32(y - MW8080BW_VCOUNTER_START_NO_VBLANK, 256 + i) = pen;

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
			video_data = m_main_ram[offs];
		}
	}

	return 0;
}
