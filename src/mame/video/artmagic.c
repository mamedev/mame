/***************************************************************************

    Art & Magic hardware

***************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "video/tlc34076.h"
#include "includes/artmagic.h"


#define INSTANT_BLIT		1


/*************************************
 *
 *  Inlines
 *
 *************************************/

INLINE UINT16 *address_to_vram(artmagic_state *state, offs_t *address)
{
	offs_t original = *address;
	*address = TOWORD(original & 0x001fffff);
	if (original >= 0x00000000 && original < 0x001fffff)
		return state->m_vram0;
	else if (original >= 0x00400000 && original < 0x005fffff)
		return state->m_vram1;
	return NULL;
}



/*************************************
 *
 *  Video start
 *
 *************************************/

VIDEO_START( artmagic )
{
	artmagic_state *state = machine.driver_data<artmagic_state>();
	state->m_blitter_base = (UINT16 *)machine.region("gfx1")->base();
	state->m_blitter_mask = machine.region("gfx1")->bytes()/2 - 1;

	state_save_register_global_array(machine, state->m_xor);
	state_save_register_global(machine, state->m_is_stoneball);
	state_save_register_global_array(machine, state->m_blitter_data);
	state_save_register_global(machine, state->m_blitter_page);
}



/*************************************
 *
 *  Shift register transfers
 *
 *************************************/

void artmagic_to_shiftreg(address_space *space, offs_t address, UINT16 *data)
{
	artmagic_state *state = space->machine().driver_data<artmagic_state>();
	UINT16 *vram = address_to_vram(state, &address);
	if (vram)
		memcpy(data, &vram[address], TOBYTE(0x2000));
}


void artmagic_from_shiftreg(address_space *space, offs_t address, UINT16 *data)
{
	artmagic_state *state = space->machine().driver_data<artmagic_state>();
	UINT16 *vram = address_to_vram(state, &address);
	if (vram)
		memcpy(&vram[address], data, TOBYTE(0x2000));
}



/*************************************
 *
 *  Custom blitter
 *
 *************************************/

static void execute_blit(running_machine &machine)
{
	artmagic_state *state = machine.driver_data<artmagic_state>();
	UINT16 *dest = state->m_blitter_page ? state->m_vram0 : state->m_vram1;
	int offset = ((state->m_blitter_data[1] & 0xff) << 16) | state->m_blitter_data[0];
	int color = (state->m_blitter_data[1] >> 4) & 0xf0;
	int x = (INT16)state->m_blitter_data[2];
	int y = (INT16)state->m_blitter_data[3];
	int maskx = state->m_blitter_data[6] & 0xff;
	int masky = state->m_blitter_data[6] >> 8;
	int w = ((state->m_blitter_data[7] & 0xff) + 1) * 4;
	int h = (state->m_blitter_data[7] >> 8) + 1;
	int i, j, sx, sy, last;

#if 0
{
	static UINT32 hit_list[50000];
	static int hit_index;
	static FILE *f;

	logerror("%s:Blit from %06X to (%d,%d) %dx%d -- %04X %04X %04X %04X %04X %04X %04X %04X\n",
				machine.describe_context(), offset, x, y, w, h,
				state->m_blitter_data[0], state->m_blitter_data[1],
				state->m_blitter_data[2], state->m_blitter_data[3],
				state->m_blitter_data[4], state->m_blitter_data[5],
				state->m_blitter_data[6], state->m_blitter_data[7]);

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
					machine.describe_context(), offset, x, y, w, h,
					state->m_blitter_data[0], state->m_blitter_data[1],
					state->m_blitter_data[2], state->m_blitter_data[3],
					state->m_blitter_data[4], state->m_blitter_data[5],
					state->m_blitter_data[6], state->m_blitter_data[7]);

		fprintf(f, "\t");
		for (i = 0; i < h; i++)
		{
			for (j = 0; j < w; j += 4)
				fprintf(f, "%04X ", state->m_blitter_base[tempoffs++]);
			fprintf(f, "\n\t");
		}
		fprintf(f, "\n\t");
		tempoffs = offset;
		for (i = 0; i < h; i++)
		{
			last = 0;
			if (i == 0)	/* first line */
			{
				/* ultennis, stonebal */
				last ^= (state->m_blitter_data[7] & 0x0001);
				if (state->m_is_stoneball)
					last ^= ((state->m_blitter_data[0] & 0x0020) >> 3);
				else	/* ultennis */
					last ^= ((state->m_blitter_data[0] & 0x0040) >> 4);

				/* cheesech */
				last ^= ((state->m_blitter_data[7] & 0x0400) >> 9);
				last ^= ((state->m_blitter_data[0] & 0x2000) >> 10);
			}
			else	/* following lines */
			{
				int val = state->m_blitter_base[tempoffs];

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
				int val = state->m_blitter_base[tempoffs++];
				int p1, p2, p3, p4;
				p1 = last = ((val ^ state->m_xor[last]) >>  0) & 0xf;
				p2 = last = ((val ^ state->m_xor[last]) >>  4) & 0xf;
				p3 = last = ((val ^ state->m_xor[last]) >>  8) & 0xf;
				p4 = last = ((val ^ state->m_xor[last]) >> 12) & 0xf;
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
				int tsy = sy * TOWORD(0x2000);
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
				if (i == 0)	/* first line */
				{
					/* ultennis, stonebal */
					last ^= (state->m_blitter_data[7] & 0x0001);
					if (state->m_is_stoneball)
						last ^= ((state->m_blitter_data[0] & 0x0020) >> 3);
					else	/* ultennis */
						last ^= (((state->m_blitter_data[0] + 1) & 0x0040) >> 4);

					/* cheesech */
					last ^= ((state->m_blitter_data[7] & 0x0400) >> 9);
					last ^= ((state->m_blitter_data[0] & 0x2000) >> 10);
				}
				else	/* following lines */
				{
					int val = state->m_blitter_base[offset & state->m_blitter_mask];

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
					UINT16 val = state->m_blitter_base[(offset + j/4) & state->m_blitter_mask];
					if (sx < 508)
					{
						if (h == 1 && state->m_is_stoneball)
							last = ((val) >>  0) & 0xf;
						else
							last = ((val ^ state->m_xor[last]) >>  0) & 0xf;
						if (!((maskx << ((j/2) & 7)) & 0x80))
						{
							if (last && sx >= 0 && sx < 512)
								dest[tsy + sx] = color | (last);
							sx++;
						}

						if (h == 1 && state->m_is_stoneball)
							last = ((val) >>  4) & 0xf;
						else
							last = ((val ^ state->m_xor[last]) >>  4) & 0xf;
						{
							if (last && sx >= 0 && sx < 512)
								dest[tsy + sx] = color | (last);
							sx++;
						}

						if (h == 1 && state->m_is_stoneball)
							last = ((val) >>  8) & 0xf;
						else
							last = ((val ^ state->m_xor[last]) >>  8) & 0xf;
						if (!((maskx << ((j/2) & 7)) & 0x40))
						{
							if (last && sx >= 0 && sx < 512)
								dest[tsy + sx] = color | (last);
							sx++;
						}

						if (h == 1 && state->m_is_stoneball)
							last = ((val) >> 12) & 0xf;
						else
							last = ((val ^ state->m_xor[last]) >> 12) & 0xf;
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
	state->m_blitter_busy_until = machine.time() + attotime::from_nsec(w*h*20);
#endif
}


READ16_MEMBER(artmagic_state::artmagic_blitter_r)
{
	/*
        bit 1 is a busy flag; loops tightly if clear
        bit 2 is tested in a similar fashion
        bit 4 reflects the page
    */
	UINT16 result = 0xffef | (m_blitter_page << 4);
#if (!INSTANT_BLIT)
	if (attotime_compare(machine().time(), m_blitter_busy_until) < 0)
		result ^= 6;
#endif
	return result;
}


WRITE16_MEMBER(artmagic_state::artmagic_blitter_w)
{
	COMBINE_DATA(&m_blitter_data[offset]);

	/* offset 3 triggers the blit */
	if (offset == 3)
		execute_blit(machine());

	/* offset 4 contains the target page */
	else if (offset == 4)
		m_blitter_page = (data >> 1) & 1;
}



/*************************************
 *
 *  Video update
 *
 *************************************/

void artmagic_scanline(screen_device &screen, bitmap_rgb32 &bitmap, int scanline, const tms34010_display_params *params)
{
	artmagic_state *state = screen.machine().driver_data<artmagic_state>();
	offs_t offset = (params->rowaddr << 12) & 0x7ff000;
	UINT16 *vram = address_to_vram(state, &offset);
	UINT32 *dest = &bitmap.pix32(scanline);
	const rgb_t *pens = tlc34076_get_pens(screen.machine().device("tlc34076"));
	int coladdr = params->coladdr << 1;
	int x;

	vram += offset;
	for (x = params->heblnk; x < params->hsblnk; x++)
		dest[x] = pens[vram[coladdr++ & 0x1ff] & 0xff];
}
