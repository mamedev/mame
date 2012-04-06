/***************************************************************************

    Entertainment Sciences Real-Time Image Processor (RIP) video hardware

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "includes/esripsys.h"


INTERRUPT_GEN( esripsys_vblank_irq )
{
	esripsys_state *state = device->machine().driver_data<esripsys_state>();
	cputag_set_input_line(device->machine(), "game_cpu", M6809_IRQ_LINE, ASSERT_LINE);
	state->m_frame_vbl = 0;
}

static TIMER_CALLBACK( hblank_start_callback )
{
	esripsys_state *state = machine.driver_data<esripsys_state>();
	int v = machine.primary_screen->vpos();

	if (state->m_video_firq)
	{
		state->m_video_firq = 0;
		cputag_set_input_line(machine, "game_cpu", M6809_FIRQ_LINE, CLEAR_LINE);
	}

	/* Not sure if this is totally accurate - I couldn't find the circuit that generates the FIRQs! */
	if (!(v % 6) && v && state->m_video_firq_en && v < ESRIPSYS_VBLANK_START)
	{
		state->m_video_firq = 1;
		cputag_set_input_line(machine, "game_cpu", M6809_FIRQ_LINE, ASSERT_LINE);
	}

	/* Adjust for next scanline */
	if (++v >= ESRIPSYS_VTOTAL)
		v = 0;

	/* Set end of HBLANK timer */
	state->m_hblank_end_timer->adjust(machine.primary_screen->time_until_pos(v, ESRIPSYS_HBLANK_END), v);
	state->m_hblank = 0;
}

static TIMER_CALLBACK( hblank_end_callback )
{
	esripsys_state *state = machine.driver_data<esripsys_state>();
	int v = machine.primary_screen->vpos();

	if (v > 0)
		machine.primary_screen->update_partial(v - 1);

	state->m_12sel ^= 1;
	state->m_hblank_start_timer->adjust(machine.primary_screen->time_until_pos(v, ESRIPSYS_HBLANK_START));

	state->m_hblank = 1;
}

VIDEO_START( esripsys )
{
	esripsys_state *state = machine.driver_data<esripsys_state>();
	struct line_buffer_t *line_buffer = state->m_line_buffer;
	int i;

	/* Allocate memory for the two 512-pixel line buffers */
	line_buffer[0].colour_buf = auto_alloc_array(machine, UINT8, 512);
	line_buffer[0].intensity_buf = auto_alloc_array(machine, UINT8, 512);
	line_buffer[0].priority_buf = auto_alloc_array(machine, UINT8, 512);

	line_buffer[1].colour_buf = auto_alloc_array(machine, UINT8, 512);
	line_buffer[1].intensity_buf = auto_alloc_array(machine, UINT8, 512);
	line_buffer[1].priority_buf = auto_alloc_array(machine, UINT8, 512);

	/* Create and initialise the HBLANK timers */
	state->m_hblank_start_timer = machine.scheduler().timer_alloc(FUNC(hblank_start_callback));
	state->m_hblank_end_timer = machine.scheduler().timer_alloc(FUNC(hblank_end_callback));
	state->m_hblank_start_timer->adjust(machine.primary_screen->time_until_pos(0, ESRIPSYS_HBLANK_START));

	/* Create the sprite scaling table */
	state->m_scale_table = auto_alloc_array(machine, UINT8, 64 * 64);

	for (i = 0; i < 64; ++i)
	{
		int j;

		for (j = 1; j < 65; ++j)
		{
			int p0 = 0;
			int p1 = 0;
			int p2 = 0;
			int p3 = 0;
			int p4 = 0;
			int p5 = 0;

			if (i & 0x1)
				p0 = BIT(j, 5) && !BIT(j, 4) && !BIT(j,3) && !BIT(j, 2) && !BIT(j, 1) && !BIT(j, 0);
			if (i & 0x2)
				p1 = BIT(j, 4) && !BIT(j, 3) && !BIT(j, 2) && !BIT(j, 1) && !BIT(j, 0);
			if (i & 0x4)
				p2 = BIT(j,3) && !BIT(j, 2) && !BIT(j, 1) && !BIT(j, 0);
			if (i & 0x8)
				p3 = BIT(j, 2) && !BIT(j,1) && !BIT(j,0);
			if (i & 0x10)
				p4 = BIT(j, 1) && !BIT(j, 0);
			if (i & 0x20)
				p5 = BIT(j, 0);

			state->m_scale_table[i * 64 + j - 1] = p0 | p1 | p2 | p3 | p4 | p5;
		}
	}

	/* Now create a lookup table for scaling the sprite 'fig' value */
	state->m_fig_scale_table = auto_alloc_array(machine, UINT8, 1024 * 64);

	for (i = 0; i < 1024; ++i)
	{
		int scale;

		for (scale = 0; scale < 64; ++scale)
		{
			int input_pixels = i + 1;
			int scaled_pixels = 0;

			while (input_pixels)
			{
				if (state->m_scale_table[scale * 64 + (scaled_pixels & 0x3f)] == 0)
					input_pixels--;

				scaled_pixels++;
			}

			state->m_fig_scale_table[i * 64 + scale] = scaled_pixels - 1;
		}
	}

	/* Register stuff for state saving */
	state_save_register_global_pointer(machine, line_buffer[0].colour_buf, 512);
	state_save_register_global_pointer(machine, line_buffer[0].intensity_buf, 512);
	state_save_register_global_pointer(machine, line_buffer[0].priority_buf, 512);

	state_save_register_global_pointer(machine, line_buffer[1].colour_buf, 512);
	state_save_register_global_pointer(machine, line_buffer[1].intensity_buf, 512);
	state_save_register_global_pointer(machine, line_buffer[1].priority_buf, 512);

	state_save_register_global(machine, state->m_video_firq);
	state_save_register_global(machine, state->m_bg_intensity);
	state_save_register_global(machine, state->m_hblank);
	state_save_register_global(machine, state->m_video_firq_en);
	state_save_register_global(machine, state->m_frame_vbl);
	state_save_register_global(machine, state->m_12sel);
}

SCREEN_UPDATE_RGB32( esripsys )
{
	esripsys_state *state = screen.machine().driver_data<esripsys_state>();
	struct line_buffer_t *line_buffer = state->m_line_buffer;
	int x, y;

	UINT8 *colour_buf = line_buffer[state->m_12sel ? 0 : 1].colour_buf;
	UINT8 *intensity_buf = line_buffer[state->m_12sel ? 0 : 1].intensity_buf;
	UINT8 *priority_buf = line_buffer[state->m_12sel ? 0 : 1].priority_buf;

	for (y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		UINT32 *dest = &bitmap.pix32(y, cliprect.min_x);

		for (x = 0; x < 512; ++x)
		{
			int idx = colour_buf[x];
			int r = (state->m_pal_ram[idx] & 0xf);
			int g = (state->m_pal_ram[256 + idx] & 0xf);
			int b = (state->m_pal_ram[512 + idx] & 0xf);
			int i = intensity_buf[x];

			*dest++ = MAKE_RGB(r*i, g*i, b*i);

			/* Clear the line buffer as we scan out */
			colour_buf[x] = 0xff;
			intensity_buf[x] = state->m_bg_intensity;
			priority_buf[x] = 0;
		}
	}

	return 0;
}

WRITE8_MEMBER(esripsys_state::esripsys_bg_intensity_w)
{
	m_bg_intensity = data & 0xf;
}

/* Draw graphics to a line buffer */
int esripsys_draw(running_machine &machine, int l, int r, int fig, int attr, int addr, int col, int x_scale, int bank)
{
	esripsys_state *state = machine.driver_data<esripsys_state>();
	struct line_buffer_t *line_buffer = state->m_line_buffer;
	UINT8 *colour_buf = line_buffer[state->m_12sel ? 1 : 0].colour_buf;
	UINT8 *intensity_buf = line_buffer[state->m_12sel ? 1 : 0].intensity_buf;
	UINT8 *priority_buf = line_buffer[state->m_12sel ? 1 : 0].priority_buf;

	UINT8 pri = attr & 0xff;
	UINT8 iny = (attr >> 8) & 0xf;
	UINT8 pal = col << 4;
	int x_flip = x_scale & 0x80;
	int xs_typ = x_scale & 0x40;
	int xs_val = x_scale & 0x3f;

	/* TODO: Check me */
	addr ^= bank * 0x8000;

	/* Fig is the number of pixels to draw / 2 - 1 */
	if (xs_typ)
		fig = state->m_fig_scale_table[fig * 64 + xs_val];

	/* 8bpp case */
	if (attr & 0x8000)
	{
		int ptr = 0;
		int cnt;
		UINT8 *rom_l;
		UINT8 *rom_r;
		UINT32 lpos = l;
		UINT32 rpos = r;

		if (x_flip)
		{
			rom_l = machine.region("8bpp_r")->base();
			rom_r = machine.region("8bpp_l")->base();
		}
		else
		{
			rom_l = machine.region("8bpp_l")->base();
			rom_r = machine.region("8bpp_r")->base();
		}

		for (cnt = 0; cnt <= fig; cnt++)
		{
			UINT32 rom_addr = (ptr * 0x10000) + addr;
			UINT8 pix1 = rom_l[rom_addr];
			UINT8 pix2 = rom_r[rom_addr];

			if (lpos < 512)
			{
				if ((pri > priority_buf[lpos]) && pix1 != 0xff)
				{
					colour_buf[lpos] = pix1;
					priority_buf[lpos] = pri;
					intensity_buf[lpos] = iny;
				}
			}

			if (rpos < 512)
			{
				if ((pri > priority_buf[rpos]) && pix2 != 0xff)
				{
					colour_buf[rpos] = pix2;
					priority_buf[rpos] = pri;
					intensity_buf[rpos] = iny;
				}
			}

			/* Shrink */
			if (!xs_typ)
			{
				if (state->m_scale_table[xs_val * 64 + (cnt & 0x3f)])
				{
					--lpos;
					++rpos;
				}

				if (++ptr == 4)
				{
					++addr;
					ptr = 0;
				}
			}
			else
			{
				if (!state->m_scale_table[xs_val * 64 + (cnt & 0x3f)])
				{
					if (++ptr == 4)
					{
						++addr;
						ptr = 0;
					}
				}

				lpos--;
				rpos++;
			}
		}
	}
	/* 4bpp case */
	else
	{
		const UINT8* const rom = machine.region("4bpp")->base();
		int ptr = 0;
		int cnt;
		UINT32 lpos = l;
		UINT32 rpos = r;

		for (cnt = 0; cnt <= fig; cnt++)
		{
			UINT8 px8 = rom[(ptr * 0x10000) + addr];
			UINT8 px1;
			UINT8 px2;

			if (x_flip)
			{
				px1 = px8 & 0xf;
				px2 = (px8 >> 4) & 0xf;
			}
			else
			{
				px2 = px8 & 0xf;
				px1 = (px8 >> 4) & 0xf;
			}

			if (lpos < 512)
			{
				if (pri > priority_buf[lpos] && px1 != 0xf)
				{
					colour_buf[lpos] = pal | px1;
					priority_buf[lpos] = pri;
					intensity_buf[lpos] = iny;
				}
			}

			if (rpos < 512)
			{
				if (pri > priority_buf[rpos] && px2 != 0xf)
				{
					colour_buf[rpos] = pal | px2;
					priority_buf[rpos] = pri;
					intensity_buf[rpos] = iny;
				}
			}

			/* Shrink */
			if (!xs_typ)
			{
				if (state->m_scale_table[xs_val * 64 + (cnt & 0x3f)])
				{
					lpos--;
					rpos++;
				}

				if (++ptr == 4)
				{
					addr++;
					ptr = 0;
				}
			}
			else
			{
				if (!state->m_scale_table[xs_val * 64 + (cnt & 0x3f)])
				{
					if (++ptr == 4)
					{
						addr++;
						ptr = 0;
					}
				}
				lpos--;
				rpos++;
			}
		}
	}

	return fig + 1;
}
