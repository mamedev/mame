// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
// thanks-to:Eric Smith, Brad Oliver, Bernd Wiebelt, Aaron Giles, Andrew Caldwell
/*************************************************************************

    avgdvg.c: Atari DVG and AVG

    Some parts of this code are based on the original version by Eric
    Smith, Brad Oliver, Bernd Wiebelt, Aaron Giles, Andrew Caldwell

    The schematics and Jed Margolin's article on Vector Generators were
    very helpful in understanding the hardware.


**************************************************************************/

#include "emu.h"
#include "avgdvg.h"
#include "screen.h"

/*************************************
 *
 *  Macros and defines
 *
 *************************************/

#define MASTER_CLOCK (12096000)
#define VGSLICE      (10000)
#define VGVECTOR 0
#define VGCLIP 1

#define OP0 (m_op & 1)
#define OP1 (m_op & 2)
#define OP2 (m_op & 4)
#define OP3 (m_op & 8)

#define ST3 (m_state_latch & 8)


/*************************************
 *
 *  Flipping
 *
 *************************************/

void avgdvg_device::apply_flipping(int *x, int *y)
{
	if (m_flip_x)
		*x += (m_xcenter - *x) << 1;
	if (m_flip_y)
		*y += (m_ycenter - *y) << 1;
}


/*************************************
 *
 *  Vector buffering
 *
 *************************************/

void avgdvg_device::vg_flush()
{
	int cx0 = 0, cy0 = 0, cx1 = 0x5000000, cy1 = 0x5000000;
	int i = 0;

	while (m_vectbuf[i].status == VGCLIP)
		i++;
	int xs = m_vectbuf[i].x;
	int ys = m_vectbuf[i].y;

	for (i = 0; i < m_nvect; i++)
	{
		if (m_vectbuf[i].status == VGVECTOR)
		{
			int xe = m_vectbuf[i].x;
			int ye = m_vectbuf[i].y;
			int x0 = xs, y0 = ys, x1 = xe, y1 = ye;

			xs = xe;
			ys = ye;

			if ((x0 < cx0 && x1 < cx0) || (x0 > cx1 && x1 > cx1))
				continue;

			if (x0 < cx0)
			{
				y0 += s64(cx0 - x0) * s64(y1 - y0) / (x1 - x0);
				x0 = cx0;
			}
			else if (x0 > cx1)
			{
				y0 += s64(cx1 - x0) * s64(y1 - y0) / (x1 - x0);
				x0 = cx1;
			}
			if (x1 < cx0)
			{
				y1 += s64(cx0 - x1) * s64(y1 - y0) / (x1 - x0);
				x1 = cx0;
			}
			else if (x1 > cx1)
			{
				y1 += s64(cx1 - x1) * s64(y1 - y0) / (x1 - x0);
				x1 = cx1;
			}

			if ((y0 < cy0 && y1 < cy0) || (y0 > cy1 && y1 > cy1))
				continue;

			if (y0 < cy0)
			{
				x0 += s64(cy0 - y0) * s64(x1 - x0) / (y1 - y0);
				y0 = cy0;
			}
			else if (y0 > cy1)
			{
				x0 += s64(cy1 - y0) * s64(x1 - x0) / (y1 - y0);
				y0 = cy1;
			}
			if (y1 < cy0)
			{
				x1 += s64(cy0 - y1) * s64(x1 - x0) / (y1 - y0);
				y1 = cy0;
			}
			else if (y1 > cy1)
			{
				x1 += s64(cy1 - y1) * s64(x1 - x0) / (y1 - y0);
				y1 = cy1;
			}

			m_vector->add_point(x0, y0, m_vectbuf[i].color, 0);
			m_vector->add_point(x1, y1, m_vectbuf[i].color, m_vectbuf[i].intensity);
		}

		if (m_vectbuf[i].status == VGCLIP)
		{
			cx0 = m_vectbuf[i].x;
			cy0 = m_vectbuf[i].y;
			cx1 = m_vectbuf[i].arg1;
			cy1 = m_vectbuf[i].arg2;
			if (cx0 > cx1)
			{
				const int t = cx1;
				cx1 = cx0;
				cx0 = t;
			}
			if (cy0 > cx1)
			{
				const int t = cy1;
				cy1 = cy0;
				cy0 = t;
			}
		}
	}

	m_nvect = 0;
}

void avgdvg_device::vg_add_point_buf(int x, int y, rgb_t color, int intensity)
{
	if (m_nvect < MAXVECT)
	{
		m_vectbuf[m_nvect].status = VGVECTOR;
		m_vectbuf[m_nvect].x = x;
		m_vectbuf[m_nvect].y = y;
		m_vectbuf[m_nvect].color = color;
		m_vectbuf[m_nvect].intensity = intensity;
		m_nvect++;
	}
}

void avgdvg_device::vg_add_clip(int xmin, int ymin, int xmax, int ymax)
{
	if (m_nvect < MAXVECT)
	{
		m_vectbuf[m_nvect].status = VGCLIP;
		m_vectbuf[m_nvect].x = xmin;
		m_vectbuf[m_nvect].y = ymin;
		m_vectbuf[m_nvect].arg1 = xmax;
		m_vectbuf[m_nvect].arg2 = ymax;
		m_nvect++;
	}
}


/*************************************
 *
 *  DVG handler functions
 *
 *************************************/

void dvg_device::update_databus() // dvg_data
{
	/*
	 * DVG uses low bit of state for address
	 */
	m_data = m_vectorram[(m_pc << 1) | (m_state_latch & 1)];
}

u8 dvg_device::state_addr() // dvg_state_addr
{
	u8 addr = ((((m_state_latch >> 4) ^ 1) & 1) << 7) | (m_state_latch & 0xf);

	if (OP3)
		addr |= ((m_op & 7) << 4);

	return addr;
}

int dvg_device::handler_0() // dvg_dmapush
{
	if (OP0 == 0)
	{
		m_sp = (m_sp + 1) & 0xf;
		m_stack[m_sp & 3] = m_pc;
	}
	return 0;
}

int dvg_device::handler_1() // dvg_dmald
{
	if (OP0)
	{
		m_pc = m_stack[m_sp & 3];
		m_sp = (m_sp - 1) & 0xf;
	}
	else
	{
		m_pc = m_dvy;
	}

	return 0;
}

void dvg_device::dvg_draw_to(int x, int y, int intensity)
{
	apply_flipping(&x, &y);

	if (((x | y) & 0x400) == 0)
		vg_add_point_buf((m_xmin + x - 512) << 16,
							(m_ymin + 512 - y) << 16,
							vector_device::color111(7), intensity << 4);
}

int dvg_device::handler_2() //dvg_gostrobe
{
	int scale;

	if (m_op == 0xf)
	{
		scale = (m_scale +
					(((m_dvy & 0x800) >> 11)
					| (((m_dvx & 0x800) ^ 0x800) >> 10)
					| ((m_dvx & 0x800)  >> 9))) & 0xf;

		m_dvy &= 0xf00;
		m_dvx &= 0xf00;
	}
	else
	{
		scale = (m_scale + m_op) & 0xf;
	}

	int fin = 0xfff - (((2 << scale) & 0x7ff) ^ 0xfff);

	/* Count up or down */
	const int dx = (m_dvx & 0x400) ? -1 : +1;
	const int dy = (m_dvy & 0x400) ? -1 : +1;

	/* Scale factor for rate multipliers */
	const int mx = (m_dvx << 2) & 0xfff;
	const int my = (m_dvy << 2) & 0xfff;

	const int cycles = 8 * fin;
	int c = 0;

	while (fin--)
	{
		/*
		 *  The 7497 Bit Rate Multiplier is a 6 bit counter with
		 *  clever decoding of output bits to perform the following
		 *  operation:
		 *
		 *  fout = m/64 * fin
		 *
		 *  where fin is the input frequency, fout is the output
		 *  frequency and m is a factor at the input pins. Output
		 *  pulses are more or less evenly spaced so we get straight
		 *  lines. The DVG has two cascaded 7497s for each coordinate.
		 */

		int countx = 0;
		int county = 0;

		for (int bit = 0; bit < 12; bit++)
		{
			if ((c & ((1 << (bit+1)) - 1)) == ((1 << bit) - 1))
			{
				if (mx & (1 << (11 - bit)))
					countx = 1;

				if (my & (1 << (11 - bit)))
					county = 1;
			}
		}

		c = (c + 1) & 0xfff;

		/*
		 *  Since x- and y-counters always hold the correct count
		 *  wrt. to each other, we can do clipping exactly like the
		 *  hardware does. That is, as soon as any counter's bit 10
		 *  changes to high, we finish the vector. If bit 10 changes
		 *  from high to low, we start a new vector.
		 */

		if (countx)
		{
			/* Is y valid and x entering or leaving the valid range? */
			if (((m_ypos & 0x400) == 0)
				&& ((m_xpos ^ (m_xpos + dx)) & 0x400))
			{
				if ((m_xpos + dx) & 0x400)
					/* We are leaving the valid range */
					dvg_draw_to(m_xpos, m_ypos, m_intensity);
				else
					/* We are entering the valid range */
					dvg_draw_to((m_xpos + dx) & 0xfff, m_ypos, 0);
			}
			m_xpos = (m_xpos + dx) & 0xfff;
		}

		if (county)
		{
			if (((m_xpos & 0x400) == 0)
				&& ((m_ypos ^ (m_ypos + dy)) & 0x400))
			{
				if ((m_xpos & 0x400) == 0)
				{
					if ((m_ypos + dy) & 0x400)
						dvg_draw_to(m_xpos, m_ypos, m_intensity);
					else
						dvg_draw_to(m_xpos, (m_ypos + dy) & 0xfff, 0);
				}
			}
			m_ypos = (m_ypos + dy) & 0xfff;
		}

	}

	dvg_draw_to(m_xpos, m_ypos, m_intensity);

	return cycles;
}

int dvg_device::handler_3() // dvg_haltstrobe
{
	m_halt = OP0;

	if (OP0 == 0)
	{
		m_xpos = m_dvx & 0xfff;
		m_ypos = m_dvy & 0xfff;
		dvg_draw_to(m_xpos, m_ypos, 0);
	}
	return 0;
}

int dvg_device::handler_7() // dvg_latch3
{
	m_dvx = (m_dvx & 0xff) |  ((m_data & 0xf) << 8);
	m_intensity = m_data >> 4;
	return 0;
}

int dvg_device::handler_6() // dvg_latch2
{
	m_dvx &= 0xf00;
	if (m_op != 0xf)
		m_dvx = (m_dvx & 0xf00) | m_data;

	if ((m_op & 0xa) == 0xa)
		m_scale = m_intensity;

	m_pc++;
	return 0;
}

int dvg_device::handler_5() //  dvg_latch1
{
	m_dvy = (m_dvy & 0xff)
		| ((m_data & 0xf) << 8);
	m_op = m_data >> 4;

	if (m_op == 0xf)
	{
		m_dvx &= 0xf00;
		m_dvy &= 0xf00;
	}

	return 0;
}

int dvg_device::handler_4() // dvg_latch0
{
	m_dvy &= 0xf00;
	if (m_op == 0xf)
		handler_7(); //dvg_latch3
	else
		m_dvy = (m_dvy & 0xf00) | m_data;

	m_pc++;
	return 0;
}

void dvg_device::vggo() // dvg_vggo
{
	m_dvy = 0;
	m_op = 0;
}

void dvg_device::vgrst() // dvg_vgrst
{
	m_state_latch = 0;
	m_dvy = 0;
	m_op = 0;
}


/********************************************************************
 *
 *  AVG handler functions
 *
 *  AVG is in many ways different from DVG. The only thing they have
 *  in common is the state machine approach. There are small
 *  differences among the AVGs, mostly related to color and vector
 *  clipping.
 *
 *******************************************************************/

u8 avg_device::state_addr() // avg_state_addr
{
	return (((m_state_latch >> 4) ^ 1) << 7)
		| (m_op << 4)
		| (m_state_latch & 0xf);
}


void avg_device::update_databus() // avg_data
{
	m_data = m_vectorram[m_pc ^ 1];
}

void avg_device::vggo() // avg_vggo
{
	m_pc = 0;
	m_sp = 0;
}


void avg_device::vgrst() // avg_vgrst
{
	m_state_latch = 0;
	m_bin_scale = 0;
	m_scale = 0;
	m_color = 0;
}

int avg_device::handler_0() // avg_latch0
{
	m_dvy = (m_dvy & 0x1f00) | m_data;
	m_pc++;

	return 0;
}

int avg_device::handler_1() // avg_latch1
{
	m_dvy12 = (m_data >> 4) & 1;
	m_op = m_data >> 5;

	m_int_latch = 0;
	m_dvy = (m_dvy12 << 12)
		| ((m_data & 0xf) << 8);
	m_dvx = 0;
	m_pc++;

	return 0;
}

int avg_device::handler_2() // avg_latch2
{
	m_dvx = (m_dvx & 0x1f00) | m_data;
	m_pc++;

	return 0;
}

int avg_device::handler_3() // avg_latch3
{
	m_int_latch = m_data >> 4;
	m_dvx = ((m_int_latch & 1) << 12)
		| ((m_data & 0xf) << 8)
		| (m_dvx & 0xff);
	m_pc++;

	return 0;
}

int avg_device::handler_4() // avg_strobe0
{
	if (OP0)
	{
		m_stack[m_sp & 3] = m_pc;
	}
	else
	{
		/*
		 * Normalization is done to get roughly constant deflection
		 * speeds. See Jed's essay why this is important. In addition
		 * to the intensity and overall time saving issues it is also
		 * needed to avoid accumulation of DAC errors. The X/Y DACs
		 * only use bits 3-12. The normalization ensures that the
		 * first three bits hold no important information.
		 *
		 * The circuit doesn't check for dvx=dvy=0. In this case
		 * shifting goes on as long as VCTR, SCALE and CNTR are
		 * low. We cut off after 16 shifts.
		 */
		int i = 0;
		while ((((m_dvy ^ (m_dvy << 1)) & 0x1000) == 0)
				&& (((m_dvx ^ (m_dvx << 1)) & 0x1000) == 0)
				&& (i++ < 16))
		{
			m_dvy = (m_dvy & 0x1000) | ((m_dvy << 1) & 0x1fff);
			m_dvx = (m_dvx & 0x1000) | ((m_dvx << 1) & 0x1fff);
			m_timer >>= 1;
			m_timer |= 0x4000 | (OP1 << 6);
		}

		if (OP1)
			m_timer &= 0xff;
	}

	return 0;
}


int avg_device::avg_common_strobe1()
{
	if (OP2)
	{
		if (OP1)
			m_sp = (m_sp - 1) & 0xf;
		else
			m_sp = (m_sp + 1) & 0xf;
	}
	return 0;
}

int avg_device::handler_5() // avg_strobe1
{
	if (OP2 == 0)
	{
		for (int i = m_bin_scale; i > 0; i--)
		{
			m_timer >>= 1;
			m_timer |= 0x4000 | (OP1 << 6);
		}
		if (OP1)
			m_timer &= 0xff;
	}

	return avg_common_strobe1();
}


int avg_device::avg_common_strobe2()
{
	if (OP2)
	{
		if (OP0)
		{
			m_pc = m_dvy << 1;

			if (m_dvy == 0)
			{
				/*
				 * Tempest and Quantum keep the AVG in an endless
				 * loop. I.e. at one point the AVG jumps to address 0
				 * and starts over again. The main CPU updates vector
				 * RAM while AVG is running. The hardware takes care
				 * that the AVG doesn't read vector RAM while the CPU
				 * writes to it. Usually we wait until the AVG stops
				 * (halt flag) and then draw all vectors at once. This
				 * doesn't work for Tempest and Quantum so we wait for
				 * the jump to zero and draw vectors then.
				 *
				 * Note that this has nothing to do with the real hardware
				 * because for a vector monitor it is perfectly okay to
				 * have the AVG drawing all the time. In the emulation we
				 * somehow have to divide the stream of vectors into
				 * 'frames'.
				 */

				m_vector->clear_list();
				vg_flush();
			}
		}
		else
		{
			m_pc = m_stack[m_sp & 3];
		}
	}
	else
	{
		if (m_dvy12)
		{
			m_scale = m_dvy & 0xff;
			m_bin_scale = (m_dvy >> 8) & 7;
		}
	}

	return 0;
}

int avg_device::handler_6() // avg_strobe2
{
	if ((OP2 == 0) && (m_dvy12 == 0))
	{
		m_color = m_dvy & 0x7;
		m_intensity = (m_dvy >> 4) & 0xf;
	}

	return avg_common_strobe2();
}

int avg_device::avg_common_strobe3()
{
	int cycles = 0;

	m_halt = OP0;

	if ((m_op & 5) == 0)
	{
		if (OP1)
		{
			cycles = 0x100 - (m_timer & 0xff);
		}
		else
		{
			cycles = 0x8000 - m_timer;
		}
		m_timer = 0;

		m_xpos += ((((m_dvx >> 3) ^ m_xdac_xor) - 0x200) * cycles * (m_scale ^ 0xff)) >> 4;
		m_ypos -= ((((m_dvy >> 3) ^ m_ydac_xor) - 0x200) * cycles * (m_scale ^ 0xff)) >> 4;
	}
	if (OP2)
	{
		cycles = 0x8000 - m_timer;
		m_timer = 0;
		m_xpos = m_xcenter;
		m_ypos = m_ycenter;
		vg_add_point_buf(m_xpos, m_ypos, 0, 0);
	}

	return cycles;
}

int avg_device::handler_7() // avg_strobe3
{
	const int cycles = avg_common_strobe3();

	if ((m_op & 5) == 0)
	{
		vg_add_point_buf(m_xpos, m_ypos, vector_device::color111(m_color),
							(((m_int_latch >> 1) == 1)? m_intensity: m_int_latch & 0xe) << 4);
	}

	return cycles;
}

/*************************************
 *
 *  Tempest handler functions
 *
 *************************************/

int avg_tempest_device::handler_6() // tempest_strobe2
{
	if ((OP2 == 0) && (m_dvy12 == 0))
	{
		/* Contrary to previous documentation in MAME,
		Tempest does not have the m_enspkl bit. */
		if (m_dvy & 0x800)
			m_color = m_dvy & 0xf;
		else
			m_intensity = (m_dvy >> 4) & 0xf;
	}

	return avg_common_strobe2();
}

int avg_tempest_device::handler_7() // tempest_strobe3
{
	const int cycles = avg_common_strobe3();

	if ((m_op & 5) == 0)
	{
		const u8 data = m_colorram[m_color];
		const u8 bit3 = (~data >> 3) & 1;
		const u8 bit2 = (~data >> 2) & 1;
		const u8 bit1 = (~data >> 1) & 1;
		const u8 bit0 = (~data >> 0) & 1;

		const u8 r = bit1 * 0xf3 + bit0 * 0x0c;
		const u8 g = bit3 * 0xf3;
		const u8 b = bit2 * 0xf3;

		int x = m_xpos;
		int y = m_ypos;

		apply_flipping(&x, &y);

		vg_add_point_buf(y - m_ycenter + m_xcenter,
							x - m_xcenter + m_ycenter, rgb_t(r, g, b),
							(((m_int_latch >> 1) == 1)? m_intensity: m_int_latch & 0xe) << 4);
	}

	return cycles;
}

#if 0
void avg_tempest_device::vggo() // tempest_vggo
{
	m_pc = 0;
	m_sp = 0;
	/*
	 * Tempest and Quantum trigger VGGO from time to time even though
	 * the VG runs in an endless loop for these games (see
	 * avg_common_strobe2). If we don't discard all vectors in the
	 * current buffer at this point, the screen starts flickering.
	 */
	m_nvect = 0;
}
#endif

/*************************************
*
*  Mhavoc handler functions
*
*************************************/

int avg_mhavoc_device::handler_1() //  mhavoc_latch1
{
	/*
	 * Major Havoc just has ymin clipping
	 */

	if (m_lst == 0)
	{
		vg_add_clip(0, m_ypos, m_xmax << 16, m_ymax << 16);
	}
	m_lst = 1;

	return avg_device::handler_1(); //avg_latch1()
}

int avg_mhavoc_device::handler_6() // mhavoc_strobe2
{
	if (OP2 == 0)
	{
		if (m_dvy12)
		{
			if (m_dvy & 0x800)
				m_lst = 0;
		}
		else
		{
			m_color = m_dvy & 0xf;

			m_intensity = (m_dvy >> 4) & 0xf;
			m_map = (m_dvy >> 8) & 0x3;
			if (m_dvy & 0x800)
			{
				m_enspkl = 1;
				m_spkl_shift = ((m_dvy >> 3) & 1)
					| ((m_dvy >> 1) & 2)
					| ((m_dvy << 1) & 4)
					| ((m_dvy << 2) & 8)
					| ((machine().rand() & 0x7) << 4);
			}
			else
			{
				m_enspkl = 0;
			}

			/* Major Havoc can do X-flipping by inverting the DAC input */
			if (m_dvy & 0x400)
				m_xdac_xor = 0x1ff;
			else
				m_xdac_xor = 0x200;
		}
	}

	return avg_common_strobe2();
}

int avg_mhavoc_device::handler_7()  // mhavoc_strobe3
{
	m_halt = OP0;
	int cycles = 0;

	if ((m_op & 5) == 0)
	{
		if (OP1)
		{
			cycles = 0x100 - (m_timer & 0xff);
		}
		else
		{
			cycles = 0x8000 - m_timer;
		}
		m_timer = 0;
		const int dx = ((((m_dvx >> 3) ^ m_xdac_xor) - 0x200) * (m_scale ^ 0xff));
		const int dy = ((((m_dvy >> 3) ^ m_ydac_xor) - 0x200) * (m_scale ^ 0xff));

		if (m_enspkl)
		{
			for (int i = 0; i < cycles / 8; i++)
			{
				m_xpos += dx / 2;
				m_ypos -= dy / 2;
				const u8 data = m_colorram[0xf +
										(((m_spkl_shift & 1) << 3)
										| (m_spkl_shift & 4)
										| ((m_spkl_shift & 0x10) >> 3)
										| ((m_spkl_shift & 0x40) >> 6))];
				const u8 bit3 = (~data >> 3) & 1;
				const u8 bit2 = (~data >> 2) & 1;
				const u8 bit1 = (~data >> 1) & 1;
				const u8 bit0 = (~data >> 0) & 1;
				const u8 r = bit3 * 0xcb + bit2 * 0x34;
				const u8 g = bit1 * 0xcb;
				const u8 b = bit0 * 0xcb;

				vg_add_point_buf(m_xpos, m_ypos, rgb_t(r, g, b),
									(((m_int_latch >> 1) == 1)? m_intensity: m_int_latch & 0xe) << 4);
				m_spkl_shift = (((m_spkl_shift & 0x40) >> 6)
									^ ((m_spkl_shift & 0x20) >> 5)
									^ 1)
					| (m_spkl_shift << 1);

				if ((m_spkl_shift & 0x7f) == 0x7f)
					m_spkl_shift = 0;
			}
		}
		else
		{
			m_xpos += (dx * cycles) >> 4;
			m_ypos -= (dy * cycles) >> 4;
			const u8 data = m_colorram[m_color];

			const u8 bit3 = (~data >> 3) & 1;
			const u8 bit2 = (~data >> 2) & 1;
			const u8 bit1 = (~data >> 1) & 1;
			const u8 bit0 = (~data >> 0) & 1;
			const u8 r = bit3 * 0xcb + bit2 * 0x34;
			const u8 g = bit1 * 0xcb;
			const u8 b = bit0 * 0xcb;

			vg_add_point_buf(m_xpos, m_ypos, rgb_t(r, g, b),
								(((m_int_latch >> 1) == 1)? m_intensity: m_int_latch & 0xe) << 4);
		}
	}
	if (OP2)
	{
		cycles = 0x8000 - m_timer;
		m_timer = 0;
		m_xpos = m_xcenter;
		m_ypos = m_ycenter;
		vg_add_point_buf(m_xpos, m_ypos, 0, 0);
	}

	return cycles;
}

void avg_mhavoc_device::update_databus() // mhavoc_data
{
	if (m_pc & 0x2000)
	{
		m_data = m_bank_region[(m_map << 13) | ((m_pc ^ 1) & 0x1fff)];
	}
	else
	{
		m_data = m_vectorram[m_pc ^ 1];
	}
}

void avg_mhavoc_device::vgrst() // mhavoc_vgrst
{
	avg_device::vgrst(); // avg_vgrst
	m_enspkl = 0;
}


/*************************************
 *
 *  Starwars handler functions
 *
 *************************************/

void avg_starwars_device::update_databus() // starwars_data
{
	m_data = m_vectorram[m_pc];
}


int avg_starwars_device::handler_6() // starwars_strobe2
{
	if ((OP2 == 0) && (m_dvy12 == 0))
	{
		m_intensity = m_dvy & 0xff;
		m_color = (m_dvy >> 8) & 0xf;
	}

	return avg_common_strobe2();
}

int avg_starwars_device::handler_7() // starwars_strobe3
{
	const int cycles = avg_common_strobe3();

	if ((m_op & 5) == 0)
	{
		vg_add_point_buf(m_xpos, m_ypos, vector_device::color111(m_color),
							((m_int_latch >> 1) * m_intensity) >> 3);
	}

	return cycles;
}

/*************************************
*
*  Quantum handler functions
*
*************************************/

void avg_quantum_device::update_databus() // quantum_data
{
	m_data = ((u16 *)m_vectorram.target())[m_pc >> 1];
}

void avg_quantum_device::vggo() // tempest_vggo
{
	m_pc = 0;
	m_sp = 0;
	/*
	 * Tempest and Quantum trigger VGGO from time to time even though
	 * the VG runs in an endless loop for these games (see
	 * avg_common_strobe2). If we don't discard all vectors in the
	 * current buffer at this point, the screen starts flickering.
	 */
	m_nvect = 0;
}

int avg_quantum_device::handler_0() // quantum_st2st3
{
	/* Quantum doesn't decode latch0 or latch2 but ST2 and ST3 are fed
	 * into the address controller which increments the PC
	 */
	m_pc++;
	return 0;
}

int avg_quantum_device::handler_1() // quantum_latch1
{
	m_dvy = m_data & 0x1fff;
	m_dvy12 = (m_data >> 12) & 1;
	m_op = m_data >> 13;

	m_int_latch = 0;
	m_dvx = 0;
	m_pc++;

	return 0;
}

int avg_quantum_device::handler_2() // quantum_st2st3
{
	/* Quantum doesn't decode latch0 or latch2 but ST2 and ST3 are fed
	 * into the address controller which increments the PC
	 */
	m_pc++;
	return 0;
}

int avg_quantum_device::handler_3() // quantum_latch3
{
	m_int_latch = m_data >> 12;
	m_dvx = m_data & 0xfff;
	m_pc++;

	return 0;
}


int avg_quantum_device::handler_4() // quantum_strobe0
{
	if (OP0)
	{
		m_stack[m_sp & 3] = m_pc;
	}
	else
	{
		/*
		 * Quantum normalizes to 12 bit
		 */
		int i = 0;
		while ((((m_dvy ^ (m_dvy << 1)) & 0x800) == 0)
				&& (((m_dvx ^ (m_dvx << 1)) & 0x800) == 0)
				&& (i++ < 16))
		{
			m_dvy = (m_dvy << 1) & 0xfff;
			m_dvx = (m_dvx << 1) & 0xfff;
			m_timer >>= 1;
			m_timer |= 0x2000 ;
		}
	}

	return 0;
}

int avg_quantum_device::handler_5() //  quantum_strobe1
{
	if (OP2 == 0)
	{
		for (int i = m_bin_scale; i > 0; i--)
		{
			m_timer >>= 1;
			m_timer |= 0x2000;
		}
	}

	return avg_common_strobe1();
}

int avg_quantum_device::handler_6() // quantum_strobe2
{
	if ((OP2 == 0) && (m_dvy12 == 0) && (m_dvy & 0x800))
	{
		m_color = m_dvy & 0xf;
		m_intensity = (m_dvy >> 4) & 0xf;
	}

	return avg_common_strobe2();
}

int avg_quantum_device::handler_7() // quantum_strobe3
{
	int cycles = 0;

	m_halt = OP0;

	if ((m_op & 5) == 0)
	{
		const u16 data = ((u16 *)m_colorram.target())[m_color];
		const u8 bit3 = (~data >> 3) & 1;
		const u8 bit2 = (~data >> 2) & 1;
		const u8 bit1 = (~data >> 1) & 1;
		const u8 bit0 = (~data >> 0) & 1;

		const u8 g = bit1 * 0xaa + bit0 * 0x54;
		const u8 b = bit2 * 0xce;
		const u8 r = bit3 * 0xce;

		cycles = 0x4000 - m_timer;
		m_timer = 0;

		m_xpos += (((((m_dvx & 0xfff) >> 2) ^ m_xdac_xor) - 0x200) * cycles * (m_scale ^ 0xff)) >> 4;
		m_ypos -= (((((m_dvy & 0xfff) >> 2) ^ m_ydac_xor) - 0x200) * cycles * (m_scale ^ 0xff)) >> 4;

		int x = m_xpos;
		int y = m_ypos;

		apply_flipping(&x, &y);

		vg_add_point_buf(y - m_ycenter + m_xcenter,
							x - m_xcenter + m_ycenter, rgb_t(r, g, b),
							((m_int_latch == 2)? m_intensity: m_int_latch) << 4);
	}
	if (OP2)
	{
		cycles = 0x4000 - m_timer;
		m_timer = 0;
		m_xpos = m_xcenter;
		m_ypos = m_ycenter;
		vg_add_point_buf(m_xpos, m_ypos, 0, 0);
	}

	return cycles;
}

/*************************************
*
*  Bzone handler functions
*
*************************************/
int avg_bzone_device::handler_1() // bzone_latch1
{
	/*
	 * Battle Zone has clipping hardware. We need to remember the
	 * position of the beam when the analog switches hst or lst get
	 * turned off.
	 */

	if (m_hst == 0)
	{
		m_clipx_max = m_xpos;
		m_clipy_min = m_ypos;
	}

	if (m_lst == 0)
	{
		m_clipx_min = m_xpos;
		m_clipy_max = m_ypos;
	}

	if (m_lst == 0 || m_hst == 0)
	{
		vg_add_clip(m_clipx_min, m_clipy_min, m_clipx_max, m_clipy_max);
	}
	m_lst = m_hst = 1;

	return avg_device::handler_1(); // avg_latch1()
}


int avg_bzone_device::handler_6() // bzone_strobe2
{
	if ((OP2 == 0) && (m_dvy12 == 0))
	{
		m_intensity = (m_dvy >> 4) & 0xf;

		if (!(m_dvy & 0x400))
		{
			m_lst = m_dvy & 0x200;
			m_hst = m_lst ^ 0x200;
			/*
			 * If izblank is true the zblank signal gets
			 * inverted. This behaviour can't be handled with the
			 * clipping we have right now. Battle Zone doesn't seem to
			 * invert zblank so it's no issue.
			 */
			m_izblank = m_dvy & 0x100;
		}
	}
	return avg_common_strobe2();
}


int avg_bzone_device::handler_7() // bzone_strobe3
{
	/* Battle Zone is B/W */
	const int cycles = avg_common_strobe3();

	if ((m_op & 5) == 0)
	{
		vg_add_point_buf(m_xpos, m_ypos, vector_device::color111(7),
							(((m_int_latch >> 1) == 1)? m_intensity: m_int_latch & 0xe) << 4);
	}

	return cycles;
}

/*************************************
 *
 *  Tomcat handler functions
 *
 *************************************/

int avg_tomcat_device::handler_6() // starwars_strobe2
{
	if ((OP2 == 0) && (m_dvy12 == 0))
	{
		m_intensity = m_dvy & 0xff;
		m_color = (m_dvy >> 8) & 0xf;
	}

	return avg_common_strobe2();
}

int avg_tomcat_device::handler_7() // starwars_strobe3
{
	const int cycles = avg_common_strobe3();

	if ((m_op & 5) == 0)
	{
		vg_add_point_buf(m_xpos, m_ypos, vector_device::color111(m_color),
							((m_int_latch >> 1) * m_intensity) >> 3);
	}

	return cycles;
}


/*************************************
 *
 *  halt functions
 *
 *************************************/

void avgdvg_device::vg_set_halt(int dummy)
{
	m_halt = dummy;
	m_sync_halt = dummy;
}

TIMER_CALLBACK_MEMBER(avgdvg_device::vg_set_halt_callback)
{
	vg_set_halt(param);
}


/********************************************************************
 *
 * State Machine
 *
 * The state machine is a 256x4 bit PROM connected to a latch. The
 * address of the next state is generated from the latched previous
 * state, an op code and the halt flag. Op codes come from vector
 * RAM/ROM. The state machine is clocked with 1.5 MHz. Three bits of
 * the state are decoded and used to trigger various parts of the
 * hardware.
 *
 *******************************************************************/

TIMER_CALLBACK_MEMBER(avgdvg_device::run_state_machine)
{
	int cycles = 0;

	while (cycles < VGSLICE)
	{
		/* Get next state */
		m_state_latch = (m_state_latch & 0x10)
			| (m_prom[state_addr()] & 0xf);

		if (ST3)
		{
			/* Read vector RAM/ROM */
			update_databus();

			/* Decode state and call the corresponding handler */
			switch(m_state_latch & 7) {
				case 0 : cycles += handler_0(); break;
				case 1 : cycles += handler_1(); break;
				case 2 : cycles += handler_2(); break;
				case 3 : cycles += handler_3(); break;
				case 4 : cycles += handler_4(); break;
				case 5 : cycles += handler_5(); break;
				case 6 : cycles += handler_6(); break;
				case 7 : cycles += handler_7(); break;
			}
		}

		/* If halt flag was set, let CPU catch up before we make halt visible */
		if (m_halt && !(m_state_latch & 0x10))
			m_vg_halt_timer->adjust(attotime::from_hz(MASTER_CLOCK) * cycles, 1);

		m_state_latch = (m_halt << 4) | (m_state_latch & 0xf);
		cycles += 8;
	}

	m_vg_run_timer->adjust(attotime::from_hz(MASTER_CLOCK) * cycles);
}


/*************************************
 *
 *  VG halt/vggo
 *
 ************************************/

CUSTOM_INPUT_MEMBER(avgdvg_device::done_r)
{
	return m_sync_halt ? 0x01 : 0x00;
}

void avgdvg_device::go_w(u8 data)
{
	vggo();

	if (m_sync_halt && (m_nvect > 10))
	{
		/*
		 * This is a good time to start a new frame. Major Havoc
		 * sometimes sets VGGO after a very short vector list. That's
		 * why we ignore frames with less than 10 vectors.
		 */
		m_vector->clear_list();
	}
	vg_flush();

	vg_set_halt(0);
	m_vg_run_timer->adjust(attotime::zero);
}

void avgdvg_device::go_word_w(u16 data)
{
	go_w(data);
}


/*************************************
 *
 *  Reset
 *
 ************************************/

void avgdvg_device::reset_w(u8 data)
{
	vgrst();
	vg_set_halt(1);
}

void avgdvg_device::reset_word_w(u16 data)
{
	reset_w(data);
}

/*************************************
 *
 *  Vector generator init
 *
 ************************************/

void avgdvg_device::register_state()
{
	save_item(NAME(m_pc));
	save_item(NAME(m_sp));
	save_item(NAME(m_dvx));
	save_item(NAME(m_dvy));
	save_item(NAME(m_dvy12));
	save_item(NAME(m_timer));
	save_item(NAME(m_stack));
	save_item(NAME(m_data));
	save_item(NAME(m_state_latch));
	save_item(NAME(m_int_latch));
	save_item(NAME(m_scale));
	save_item(NAME(m_bin_scale));
	save_item(NAME(m_intensity));
	save_item(NAME(m_color));
	save_item(NAME(m_enspkl));
	save_item(NAME(m_spkl_shift));
	save_item(NAME(m_map));
	save_item(NAME(m_hst));
	save_item(NAME(m_lst));
	save_item(NAME(m_izblank));
	save_item(NAME(m_op));
	save_item(NAME(m_halt));
	save_item(NAME(m_sync_halt));
	save_item(NAME(m_xdac_xor));
	save_item(NAME(m_ydac_xor));
	save_item(NAME(m_xpos));
	save_item(NAME(m_ypos));
	save_item(NAME(m_clipx_min));
	save_item(NAME(m_clipy_min));
	save_item(NAME(m_clipx_max));
	save_item(NAME(m_clipy_max));

	save_item(NAME(m_flip_x));
	save_item(NAME(m_flip_y));
}

void avg_device::device_start()
{
	if (!m_vector->started())
		throw device_missing_dependencies();

	const rectangle &visarea = m_vector->screen().visible_area();

	m_xmin = visarea.min_x;
	m_ymin = visarea.min_y;
	m_xmax = visarea.max_x;
	m_ymax = visarea.max_y;

	m_xcenter = ((m_xmax - m_xmin) / 2) << 16;
	m_ycenter = ((m_ymax - m_ymin) / 2) << 16;

	m_flip_x = m_flip_y = false;

	m_vg_halt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(avgdvg_device::vg_set_halt_callback),this));
	m_vg_run_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(avgdvg_device::run_state_machine),this));

	/*
	 * The x and y DACs use 10 bit of the counter values which are in
	 * two's complement representation. The DAC input is xored with
	 * 0x200 to convert the value to unsigned.
	 */
	m_xdac_xor = 0x200;
	m_ydac_xor = 0x200;

	register_state();
}

void dvg_device::device_start()
{
	if (!m_vector->started())
		throw device_missing_dependencies();

	const rectangle &visarea = m_vector->screen().visible_area();

	m_xmin = visarea.min_x;
	m_ymin = visarea.min_y;

	m_xcenter = 512;
	m_ycenter = 512;

	m_flip_x = m_flip_y = false;

	m_vg_halt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(avgdvg_device::vg_set_halt_callback),this));
	m_vg_run_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(avgdvg_device::run_state_machine),this));

	register_state();
}

avgdvg_device::avgdvg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_vectorram(*this, "vectorram", 0)
	, m_colorram(*this, "colorram", 0)
	, m_prom(*this, "prom")
	, m_vector(*this, finder_base::DUMMY_TAG)
{
	m_pc = 0;
	m_sp = 0;
	m_dvx = 0;
	m_dvy = 0;
	m_dvy12 = 0;
	m_timer = 0;
	m_stack[0] = 0;
	m_stack[1] = 0;
	m_stack[2] = 0;
	m_stack[3] = 0;
	m_data = 0;

	m_state_latch = 0;
	m_int_latch = 0;
	m_scale = 0;
	m_bin_scale = 0;
	m_intensity = 0;
	m_color = 0;
	m_enspkl = 0;
	m_spkl_shift = 0;
	m_map = 0;

	m_hst = 0;
	m_lst = 0;
	m_izblank = 0;

	m_op = 0;
	m_halt = 0;
	m_sync_halt = 0;

	m_xdac_xor = 0;
	m_ydac_xor = 0;

	m_xpos = 0;
	m_ypos = 0;

	m_clipx_min = 0;
	m_clipy_min = 0;
	m_clipx_max = 0;
	m_clipy_max = 0;

	m_xmin = 0;
	m_xmax = 0;
	m_ymin = 0;
	m_ymax = 0;
	m_xcenter = 0;
	m_ycenter = 0;
	m_flip_x = false;
	m_flip_y = false;
	m_nvect = 0;
}

dvg_device::dvg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	avgdvg_device(mconfig, DVG, tag, owner, clock)
{
}

avg_device::avg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	avg_device(mconfig, AVG, tag, owner, clock)
{
}

avg_device::avg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	avgdvg_device(mconfig, type, tag, owner, clock)
{
}


avg_tempest_device::avg_tempest_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	avg_device(mconfig, AVG_TEMPEST, tag, owner, clock)
{
}
avg_mhavoc_device::avg_mhavoc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	avg_device(mconfig, AVG_MHAVOC, tag, owner, clock),
	m_bank_region(*this, DEVICE_SELF)
{
}

avg_starwars_device::avg_starwars_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	avg_device(mconfig, AVG_STARWARS, tag, owner, clock)
{
}

avg_quantum_device::avg_quantum_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	avg_device(mconfig, AVG_QUANTUM, tag, owner, clock)
{
}

avg_bzone_device::avg_bzone_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	avg_device(mconfig, AVG_BZONE, tag, owner, clock)
{
}

avg_tomcat_device::avg_tomcat_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	avg_device(mconfig, AVG_TOMCAT, tag, owner, clock)
{
}

DEFINE_DEVICE_TYPE(DVG,          dvg_device,          "dvg",          "Atari DVG")
DEFINE_DEVICE_TYPE(AVG,          avg_device,          "avg",          "Atari AVG")
DEFINE_DEVICE_TYPE(AVG_TEMPEST,  avg_tempest_device,  "avg_tempest",  "Atari AVG (Tempest)")
DEFINE_DEVICE_TYPE(AVG_MHAVOC,   avg_mhavoc_device,   "avg_mhavoc",   "Atari AVG (Major Havoc)")
DEFINE_DEVICE_TYPE(AVG_STARWARS, avg_starwars_device, "avg_starwars", "Atari AVG (Star Wars)")
DEFINE_DEVICE_TYPE(AVG_QUANTUM,  avg_quantum_device,  "avg_quantum",  "Atari AVG (Quantum)")
DEFINE_DEVICE_TYPE(AVG_BZONE,    avg_bzone_device,    "avg_bzone",    "Atari AVG (Battle Zone)")
DEFINE_DEVICE_TYPE(AVG_TOMCAT,   avg_tomcat_device,   "avg_tomcat",   "Atari AVG (TomCat)")
