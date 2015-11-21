// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Sega vector hardware

*************************************************************************/

#include "emu.h"
#include "includes/segag80v.h"

#define VECTOR_CLOCK        15468480            /* master clock */
#define U34_CLOCK           (VECTOR_CLOCK/3)    /* clock for interrupt chain */
#define VCL_CLOCK           (U34_CLOCK/2)       /* clock for vector generator */
#define U51_CLOCK           (VCL_CLOCK/16)      /* clock for phase generator */
#define IRQ_CLOCK           (U34_CLOCK/0x1f788) /* 40Hz interrupt */




/*

    Vector system is clocked by a 15-phase clock.

    The counter is a LS161 4-bit binary counter at U51, and its output
    goes to a LS154 1-of-16 decoder at U50.

    Each phase various things happen. The phases are:

     0 -> (sheet 7/7) clocks CD7 in

     1 -> (sheet 5/7) loads CD0-7 into counters at U15/U16
          (sheet 6/7) clear LS175 flip flops at U35, U36, U37, U38

     2 -> (sheet 5/7) loads CD0-3 into counter at U17

     3 -> (sheet 5/7) loads CD0-7 into counters at U18/U19

     4 -> (sheet 5/7) loads CD0-3 into counter at U20

     5 ->

     6 ->

     7 -> (sheet 6/7) at end, latches CD0-7 into LS374 tri-state flip flop at U55 (SYM angle)

     8 -> (sheet 6/7) at end, latches CD0-1 into LS74 flip flops at U26 (upper SYM angle)

     9 -> (sheet 4/7) at end, latches CD0-7 into 25LS14 multiplier X input at U8 (scale)

    10 -> (sheet 4/7) at end, latches CD0-CD7 into LS374 tri-state flip flop at U2 (attributes)
          (sheet 7/7) at end, latches CD7 into U52 (low), which sets the preload value
            for the LS161 at U51 to be either 0 (if CD7==1) or 10 (if CD7==0)

    11 -> (sheet 4/7) at end, starts multiply circuit

    12 -> (sheet 6/7) at end, latches CD0-7 into LS374 tri-state flip flop at U56 (VEC angle)

    13 -> (sheet 6/7) at end, latches output from 2708 PROM into tri-state flip flop at U48
          (sheet 6/7) at end, latches bit $200 of angle into D/UX output

    14 -> (sheet 6/7) at end, latches output from 2708 PROM into tri-state flip flop at U49
          (sheet 6/7) at end, latches bit $200 of angle into D/UY output
          (sheet 7/7) signals /PE on the LS161 at U51, loading the new value for the state clock
          (sheet 7/7) sets up the DRAW signal to clock on the next VCL edge

    15 ->



    PROM inputs:
        A0 = GND
        A1-A8 = sum of VEC angle and SYM angle (low 8 bits)
        A9 = sum of bit 8 of VEC angle and SYM angle, plus 1 for phase 13


*/


inline int segag80v_state::adjust_xy(int rawx, int rawy, int *outx, int *outy)
{
	int clipped = FALSE;

	/* first apply the XOR at 0x200 */
	*outx = (rawx & 0x7ff) ^ 0x200;
	*outy = (rawy & 0x7ff) ^ 0x200;

	/* apply clipping logic to X */
	if ((*outx & 0x600) == 0x200)
		*outx = 0x000, clipped = TRUE;
	else if ((*outx & 0x600) == 0x400)
		*outx = 0x3ff, clipped = TRUE;
	else
		*outx &= 0x3ff;

	/* apply clipping logic to Y */
	if ((*outy & 0x600) == 0x200)
		*outy = 0x000, clipped = TRUE;
	else if ((*outy & 0x600) == 0x400)
		*outy = 0x3ff, clipped = TRUE;
	else
		*outy &= 0x3ff;

	/* convert into .16 values */
	*outx = (*outx - (m_min_x - 512)) << 16;
	*outy = (*outy - (m_min_y - 512)) << 16;
	return clipped;
}


void segag80v_state::sega_generate_vector_list()
{
	UINT8 *sintable = memregion("proms")->base();
	double total_time = 1.0 / (double)IRQ_CLOCK;
	UINT16 symaddr = 0;
	UINT8 *vectorram = m_vectorram;

	m_vector->clear_list();

	/* Loop until we run out of time. */
	while (total_time > 0)
	{
		UINT16 curx, cury, xaccum, yaccum;
		UINT16 vecaddr, symangle;
		UINT8 scale, draw;

		/* The "draw" flag is clocked at the end of phase 0. */
		draw = vectorram[symaddr++ & 0xfff];

		/* The low byte of the X coordinate is latched into the        */
		/* up/down counters at U15/U16 during phase 1. */
		curx = vectorram[symaddr++ & 0xfff];

		/* The low 3 bits of the high byte of the X coordinate are     */
		/* latched into the up/down counter at U17 during phase 2.     */
		/* Bit 2 of the input is latched as both bit 2 and 3. */
		curx |= (vectorram[symaddr++ & 0xfff] & 7) << 8;
		curx |= (curx << 1) & 0x800;

		/* The low byte of the Y coordinate is latched into the        */
		/* up/down counters at U18/U19 during phase 3. */
		cury = vectorram[symaddr++ & 0xfff];

		/* The low 3 bits of the high byte of the X coordinate are     */
		/* latched into the up/down counter at U17 during phase 4.     */
		/* Bit 2 of the input is latched as both bit 2 and 3. */
		cury |= (vectorram[symaddr++ & 0xfff] & 7) << 8;
		cury |= (cury << 1) & 0x800;

		/* The low byte of the vector address is latched into the      */
		/* counters at U10/U11 during phase 5. */
		vecaddr = vectorram[symaddr++ & 0xfff];

		/* The low 4 bits of the high byte of the vector address is    */
		/* latched into the counter at U12 during phase 6. */
		vecaddr |= (vectorram[symaddr++ & 0xfff] & 0xf) << 8;

		/* The low byte of the symbol angle is latched into the tri-   */
		/* state flip flop at U55 at the end of phase 7. */
		symangle = vectorram[symaddr++ & 0xfff];

		/* The low 2 bits of the high byte of the symbol angle are     */
		/* latched into flip flops at U26 at the end of phase 8. */
		symangle |= (vectorram[symaddr++ & 0xfff] & 3) << 8;

		/* The scale is latched in phase 9 as the X input to the       */
		/* 25LS14 multiplier at U8. */
		scale = vectorram[symaddr++ & 0xfff];

		/* Account for the 10 phases so far. */
		total_time -= 10.0 / (double)U51_CLOCK;

		/* Skip the rest if we're not drawing this symbol. */
		if (draw & 1)
		{
			int adjx, adjy, clipped;

			/* Add a starting point to the vector list. */
			clipped = adjust_xy(curx, cury, &adjx, &adjy);
			if (!clipped)
				m_vector->add_point(adjx, adjy, 0, 0);

			/* Loop until we run out of time. */
			while (total_time > 0)
			{
				UINT16 vecangle, length, deltax, deltay;
				UINT8 attrib, intensity;
				UINT32 color;

				/* The 'attribute' byte is latched at the end of phase 10 into */
				/* the tri-state flip flop at U2. The low bit controls whether */
				/* or not the beam is enabled. Bits 1-6 control the RGB color  */
				/* (2 bits per component). In addition, bit 7 of this value is */
				/* latched into U52, which controls the pre-load value for the */
				/* phase generator. If bit 7 is high, then the phase generator */
				/* will reset back to 0 and draw a new symbol; if bit 7 is low */
				/* the phase generator will reset back to 10 and draw another  */
				/* vector. */
				attrib = vectorram[vecaddr++ & 0xfff];

				/* The length of the vector is loaded into the shift registers */
				/* at U6/U7 during phase 11. During phase 12, the 25LS14       */
				/* multiplier at U8 is used to multiply the length by the      */
				/* scale that was loaded during phase 9. The length is clocked */
				/* bit by bit out of U6/U7 and the result is clocked into the  */
				/* other side. After the multiply, the 9 MSBs are loaded into  */
				/* the counter chain at U15/16/17 and are used to count how    */
				/* long to draw the vector. */
				length = (vectorram[vecaddr++ & 0xfff] * scale) >> 7;

				/* The vector angle low byte is latched at the end of phase 12 */
				/* into the tri-state flip flop at U56. */
				vecangle = vectorram[vecaddr++ & 0xfff];

				/* The vector angle high byte is preset on the CD bus during   */
				/* phases 13 and 14, and is used as inputs to the adder at     */
				/* U46. */
				vecangle |= (vectorram[vecaddr++ & 0xfff] & 3) << 8;

				/* The X increment value is looked up first (phase 13). The    */
				/* sum of the latched symbol angle and the vector angle is     */
				/* used as input to the PROM at U39. A0 is tied to ground.     */
				/* A1-A9 map to bits 0-8 of the summed angles. The output from */
				/* the PROM is latched into U48. */
				deltax = sintable[((vecangle + symangle) & 0x1ff) << 1];

				/* The Y increment value is looked up second (phase 14). The   */
				/* angle sum is used once again as the input to the PROM, but  */
				/* this time an additional 0x100 is effectively added to it    */
				/* before it is used; this separates sin from cos. The output  */
				/* from the PROM is latched into U49. */
				deltay = sintable[((vecangle + symangle + 0x100) & 0x1ff) << 1];

				/* Account for the 4 phases for data fetching. */
				total_time -= 4.0 / (double)U51_CLOCK;

				/* Compute color/intensity values from the attributes */
				color = VECTOR_COLOR222((attrib >> 1) & 0x3f);
				if ((attrib & 1) && color)
					intensity = 0xff;
				else
					intensity = 0;

				/* Loop over the length of the vector. */
				clipped = adjust_xy(curx, cury, &adjx, &adjy);
				xaccum = yaccum = 0;
				while (length-- != 0 && total_time > 0)
				{
					int newclip;

					/* The adders at U44/U45 are used as X accumulators. The value */
					/* from U48 is repeatedly added to itself here. The carry out  */
					/* of bit 8 clocks the up/down counters at U15/U16/U17. Bit 7  */
					/* of the input value from U48 is used as a carry in to round  */
					/* small values downward and larger values upward. */
					xaccum += deltax + (deltax >> 7);

					/* Bit 9 of the summed angles controls the direction the up/   */
					/* down counters at U15/U16/U17. */
					if (((vecangle + symangle) & 0x200) == 0)
						curx += xaccum >> 8;
					else
						curx -= xaccum >> 8;
					xaccum &= 0xff;

					/* The adders at U46/U47 are used as Y accumulators. The value */
					/* from U49 is repeatedly added to itself here. The carry out  */
					/* of bit 8 clocks the up/down counters at U18/U19/U20. Bit 7  */
					/* of the input value from U49 is used as a carry in to round  */
					/* small values downward and larger values upward. */
					yaccum += deltay + (deltay >> 7);

					/* Bit 9 of the summed angles controls the direction the up/   */
					/* down counters at U18/U19/U20. */
					if (((vecangle + symangle + 0x100) & 0x200) == 0)
						cury += yaccum >> 8;
					else
						cury -= yaccum >> 8;
					yaccum &= 0xff;

					/* Apply the clipping from the DAC circuit. If the values clip */
					/* the beam is turned off, but the computations continue right */
					/* on going. */
					newclip = adjust_xy(curx, cury, &adjx, &adjy);
					if (newclip != clipped)
					{
						/* if we're just becoming unclipped, add an empty point */
						if (!newclip)
							m_vector->add_point(adjx, adjy, 0, 0);

						/* otherwise, add a colored point */
						else
							m_vector->add_point(adjx, adjy, color, intensity);
					}
					clipped = newclip;

					/* account for vector drawing time */
					total_time -= 1.0 / (double)VCL_CLOCK;
				}

				/* We're done; if we are not clipped, add a final point. */
				if (!clipped)
					m_vector->add_point(adjx, adjy, color, intensity);

				/* if the high bit of the attribute is set, we break out of   */
				/* this loop and fetch another symbol */
				if (attrib & 0x80)
					break;
			}
		}

		/* if the high bit of the draw flag is set, we break out of this loop */
		/* and stop the rendering altogether for this frame. */
		if (draw & 0x80)
			break;
	}
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void segag80v_state::video_start()
{
	assert_always(m_vectorram.bytes() != 0, "vectorram==0");

	m_min_x =m_screen->visible_area().min_x;
	m_min_y =m_screen->visible_area().min_y;
}


UINT32 segag80v_state::screen_update_segag80v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	sega_generate_vector_list();
	m_vector->screen_update(screen, bitmap, cliprect);
	return 0;
}
