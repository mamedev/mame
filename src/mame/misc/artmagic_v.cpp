// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Nicola Salmoria
/***************************************************************************

    Art & Magic hardware

***************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "video/tlc34076.h"
#include "artmagic.h"


#define INSTANT_BLIT        1


/*************************************
 *
 *  Inlines
 *
 *************************************/

inline uint16_t *artmagic_state::address_to_vram(offs_t *address)
{
	offs_t original = *address;
	*address = (original & 0x001fffff) >> 4;
	if (original < 0x001fffff)
		return m_vram[0];
	else if (original >= 0x00400000 && original < 0x005fffff)
		return m_vram[1];
	return nullptr;
}



/*************************************
 *
 *  Video start
 *
 *************************************/

void artmagic_state::video_start()
{
	// assumes it can make an address mask from m_blitter_base.length() - 1
	assert(!(m_blitter_base.length() & (m_blitter_base.length() - 1)));

	save_item(NAME(m_xor));
	save_item(NAME(m_is_stoneball));
	save_item(NAME(m_blitter_data));
	save_item(NAME(m_blitter_page));
}



/*************************************
 *
 *  Shift register transfers
 *
 *************************************/

TMS340X0_TO_SHIFTREG_CB_MEMBER(artmagic_state::to_shiftreg)
{
	uint16_t *vram = address_to_vram(&address);
	if (vram)
		memcpy(shiftreg, &vram[address], 0x400);
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(artmagic_state::from_shiftreg)
{
	uint16_t *vram = address_to_vram(&address);
	if (vram)
		memcpy(&vram[address], shiftreg, 0x400);
}



/*************************************
 *
 *  Custom blitter
 *
 *************************************/

void artmagic_state::execute_blit()
{
	uint16_t *dest = m_vram[m_blitter_page ^ 1];
	int offset = ((m_blitter_data[1] & 0xff) << 16) | m_blitter_data[0];
	int color = (m_blitter_data[1] >> 4) & 0xf0;
	int x = (int16_t)m_blitter_data[2];
	int y = (int16_t)m_blitter_data[3];
	int maskx = m_blitter_data[6] & 0xff;
	int masky = m_blitter_data[6] >> 8;
	int w = ((m_blitter_data[7] & 0xff) + 1) * 4;
	int h = (m_blitter_data[7] >> 8) + 1;
	int i, j, sx, sy, last;

#if 0
{
	static uint32_t hit_list[50000];
	static int hit_index;
	static FILE *f;

	logerror("%s:Blit from %06X to (%d,%d) %dx%d -- %04X %04X %04X %04X %04X %04X %04X %04X\n",
				machine().describe_context(), offset, x, y, w, h,
				m_blitter_data[0], m_blitter_data[1],
				m_blitter_data[2], m_blitter_data[3],
				m_blitter_data[4], m_blitter_data[5],
				m_blitter_data[6], m_blitter_data[7]);

	if (!f) f = fopen("artmagic.log", "w");

	for (i = 0; i < hit_index; i++)
		if (hit_list[i] == offset)
			break;
	if (i == hit_index)
	{
		int tempoffs = offset;
		hit_list[hit_index++] = offset;

		fprintf(f, "----------------------\n"
					"%s:Blit from %06X to (%d,%d) %dx%d -- %04X %04X %04X %04X %04X %04X %04X %04X\n",
					machine().describe_context(), offset, x, y, w, h,
					m_blitter_data[0], m_blitter_data[1],
					m_blitter_data[2], m_blitter_data[3],
					m_blitter_data[4], m_blitter_data[5],
					m_blitter_data[6], m_blitter_data[7]);

		fprintf(f, "\t");
		for (i = 0; i < h; i++)
		{
			for (j = 0; j < w; j += 4)
				fprintf(f, "%04X ", m_blitter_base[tempoffs++]);
			fprintf(f, "\n\t");
		}
		fprintf(f, "\n\t");
		tempoffs = offset;
		for (i = 0; i < h; i++)
		{
			last = 0;
			if (i == 0) /* first line */
			{
				/* ultennis, stonebal */
				last ^= (m_blitter_data[7] & 0x0001);
				if (m_is_stoneball)
					last ^= ((m_blitter_data[0] & 0x0020) >> 3);
				else    /* ultennis */
					last ^= ((m_blitter_data[0] & 0x0040) >> 4);

				/* cheesech */
				last ^= ((m_blitter_data[7] & 0x0400) >> 9);
				last ^= ((m_blitter_data[0] & 0x2000) >> 10);
			}
			else    /* following lines */
			{
				int val = m_blitter_base[tempoffs];

				/* ultennis, stonebal */
				last ^= 4;
				last ^= ((val & 0x0400) >> 8);
				last ^= ((val & 0x5000) >> 12);

				/* cheesech */
				last ^= 8;
				last ^= ((val & 0x0800) >> 8);
				last ^= ((val & 0xa000) >> 12);
			}

			for (j = 0; j < w; j += 4)
			{
				static const char hex[] = ".123456789ABCDEF";
				int val = m_blitter_base[tempoffs++];
				int p1, p2, p3, p4;
				p1 = last = ((val ^ m_xor[last]) >>  0) & 0xf;
				p2 = last = ((val ^ m_xor[last]) >>  4) & 0xf;
				p3 = last = ((val ^ m_xor[last]) >>  8) & 0xf;
				p4 = last = ((val ^ m_xor[last]) >> 12) & 0xf;
				fprintf(f, "%c%c%c%c ", hex[p1], hex[p2], hex[p3], hex[p4]);
			}
			fprintf(f, "\n\t");
		}
		fprintf(f, "\n");
	}
}
#endif

	g_profiler.start(PROFILER_VIDEO);

	last = 0;
	sy = y;
	for (i = 0; i < h; i++)
	{
		if ((i & 1) || !((masky << ((i/2) & 7)) & 0x80))
		{
			if (sy >= 0 && sy < 256)
			{
				int tsy = sy * 0x200;
				sx = x;

				/* The first pixel of every line doesn't have a previous pixel
				   to depend on, so it takes the "feed" from other bits.
				   The very first pixel blitted is also treated differently.

				   ultennis/stonebal use a different encryption from cheesech,
				   however the former only need to set bits 0 and 2 of the
				   feed (the others are irrelevant), while the latter only
				   bits 1 and 3, so I can handle both at the same time.
				 */
				last = 0;
				if (i == 0) /* first line */
				{
					/* ultennis, stonebal */
					last ^= (m_blitter_data[7] & 0x0001);
					if (m_is_stoneball)
						last ^= ((m_blitter_data[0] & 0x0020) >> 3);
					else    /* ultennis */
						last ^= (((m_blitter_data[0] + 1) & 0x0040) >> 4);

					/* cheesech */
					last ^= ((m_blitter_data[7] & 0x0400) >> 9);
					last ^= ((m_blitter_data[0] & 0x2000) >> 10);
				}
				else    /* following lines */
				{
					int val = m_blitter_base[offset & (m_blitter_base.length() - 1)];

					/* ultennis, stonebal */
					last ^= 4;
					last ^= ((val & 0x0400) >> 8);
					last ^= ((val & 0x5000) >> 12);

					/* cheesech */
					last ^= 8;
					last ^= ((val & 0x0800) >> 8);
					last ^= ((val & 0xa000) >> 12);
				}

				for (j = 0; j < w; j += 4)
				{
					uint16_t val = m_blitter_base[(offset + j/4) & (m_blitter_base.length() - 1)];
					if (sx < 508)
					{
						if (h == 1 && m_is_stoneball)
							last = ((val) >>  0) & 0xf;
						else
							last = ((val ^ m_xor[last]) >>  0) & 0xf;
						if (!((maskx << ((j/2) & 7)) & 0x80))
						{
							if (last && sx >= 0 && sx < 512)
								dest[tsy + sx] = color | (last);
							sx++;
						}

						if (h == 1 && m_is_stoneball)
							last = ((val) >>  4) & 0xf;
						else
							last = ((val ^ m_xor[last]) >>  4) & 0xf;
						{
							if (last && sx >= 0 && sx < 512)
								dest[tsy + sx] = color | (last);
							sx++;
						}

						if (h == 1 && m_is_stoneball)
							last = ((val) >>  8) & 0xf;
						else
							last = ((val ^ m_xor[last]) >>  8) & 0xf;
						if (!((maskx << ((j/2) & 7)) & 0x40))
						{
							if (last && sx >= 0 && sx < 512)
								dest[tsy + sx] = color | (last);
							sx++;
						}

						if (h == 1 && m_is_stoneball)
							last = ((val) >> 12) & 0xf;
						else
							last = ((val ^ m_xor[last]) >> 12) & 0xf;
						{
							if (last && sx >= 0 && sx < 512)
								dest[tsy + sx] = color | (last);
							sx++;
						}
					}
				}
			}
			sy++;
		}
		offset += w/4;
	}

	g_profiler.stop();

#if (!INSTANT_BLIT)
	m_blitter_busy_until = machine.time() + attotime::from_nsec(w*h*20);
#endif
}


uint16_t artmagic_state::blitter_r()
{
	/*
	    bit 1 is a busy flag; loops tightly if clear
	    bit 2 is tested in a similar fashion
	    bit 4 reflects the page
	*/
	uint16_t result = 0xffef | (m_blitter_page << 4);
#if (!INSTANT_BLIT)
	if (attotime_compare(machine().time(), m_blitter_busy_until) < 0)
		result ^= 6;
#endif
	return result;
}


void artmagic_state::blitter_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_blitter_data[offset]);

	/* offset 3 triggers the blit */
	if (offset == 3)
		execute_blit();

	/* offset 4 contains the target page */
	else if (offset == 4)
		m_blitter_page = (data >> 1) & 1;
}



/*************************************
 *
 *  Video update
 *
 *************************************/

TMS340X0_SCANLINE_RGB32_CB_MEMBER(artmagic_state::scanline)
{
	offs_t offset = (params->rowaddr << 12) & 0x7ff000;
	uint16_t const *vram = address_to_vram(&offset);
	uint32_t *const dest = &bitmap.pix(scanline);
	pen_t const *const pens = m_tlc34076->pens();
	int coladdr = params->coladdr << 1;

	vram += offset;
	for (int x = params->heblnk; x < params->hsblnk; x++)
		dest[x] = pens[vram[coladdr++ & 0x1ff] & 0xff];
}
