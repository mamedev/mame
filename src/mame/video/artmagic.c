/***************************************************************************

    Art & Magic hardware

***************************************************************************/

#include "driver.h"
#include "profiler.h"
#include "cpu/tms34010/tms34010.h"
#include "video/tlc34076.h"
#include "includes/artmagic.h"


#define INSTANT_BLIT		1


UINT16 *artmagic_vram0;
UINT16 *artmagic_vram1;

/* decryption parameters */
int artmagic_xor[16], artmagic_is_stoneball;

static UINT16 *blitter_base;
static UINT32 blitter_mask;
static UINT16 blitter_data[8];
static UINT8 blitter_page;

#if (!INSTANT_BLIT)
static attotime blitter_busy_until;
#endif



/*************************************
 *
 *  Inlines
 *
 *************************************/

INLINE UINT16 *address_to_vram(offs_t *address)
{
	offs_t original = *address;
	*address = TOWORD(original & 0x001fffff);
	if (original >= 0x00000000 && original < 0x001fffff)
		return artmagic_vram0;
	else if (original >= 0x00400000 && original < 0x005fffff)
		return artmagic_vram1;
	return NULL;
}



/*************************************
 *
 *  Video start
 *
 *************************************/

VIDEO_START( artmagic )
{
	blitter_base = (UINT16 *)memory_region(machine, "gfx1");
	blitter_mask = memory_region_length(machine, "gfx1")/2 - 1;

	tlc34076_state_save(machine);
	state_save_register_global_array(machine, artmagic_xor);
	state_save_register_global(machine, artmagic_is_stoneball);
	state_save_register_global_array(machine, blitter_data);
	state_save_register_global(machine, blitter_page);
}



/*************************************
 *
 *  Shift register transfers
 *
 *************************************/

void artmagic_to_shiftreg(const address_space *space, offs_t address, UINT16 *data)
{
	UINT16 *vram = address_to_vram(&address);
	if (vram)
		memcpy(data, &vram[address], TOBYTE(0x2000));
}


void artmagic_from_shiftreg(const address_space *space, offs_t address, UINT16 *data)
{
	UINT16 *vram = address_to_vram(&address);
	if (vram)
		memcpy(&vram[address], data, TOBYTE(0x2000));
}



/*************************************
 *
 *  Custom blitter
 *
 *************************************/

static void execute_blit(running_machine *machine)
{
	UINT16 *dest = blitter_page ? artmagic_vram0 : artmagic_vram1;
	int offset = ((blitter_data[1] & 0xff) << 16) | blitter_data[0];
	int color = (blitter_data[1] >> 4) & 0xf0;
	int x = (INT16)blitter_data[2];
	int y = (INT16)blitter_data[3];
	int maskx = blitter_data[6] & 0xff;
	int masky = blitter_data[6] >> 8;
	int w = ((blitter_data[7] & 0xff) + 1) * 4;
	int h = (blitter_data[7] >> 8) + 1;
	int i, j, sx, sy, last;

#if 0
{
	static UINT32 hit_list[50000];
	static int hit_index;
	static FILE *f;

	logerror("%s:Blit from %06X to (%d,%d) %dx%d -- %04X %04X %04X %04X %04X %04X %04X %04X\n",
				cpuexec_describe_context(machine), offset, x, y, w, h,
				blitter_data[0], blitter_data[1],
				blitter_data[2], blitter_data[3],
				blitter_data[4], blitter_data[5],
				blitter_data[6], blitter_data[7]);

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
					cpuexec_describe_context(machine), offset, x, y, w, h,
					blitter_data[0], blitter_data[1],
					blitter_data[2], blitter_data[3],
					blitter_data[4], blitter_data[5],
					blitter_data[6], blitter_data[7]);

		fprintf(f, "\t");
		for (i = 0; i < h; i++)
		{
			for (j = 0; j < w; j += 4)
				fprintf(f, "%04X ", blitter_base[tempoffs++]);
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
				last ^= (blitter_data[7] & 0x0001);
				if (artmagic_is_stoneball)
					last ^= ((blitter_data[0] & 0x0020) >> 3);
				else	/* ultennis */
					last ^= ((blitter_data[0] & 0x0040) >> 4);

				/* cheesech */
				last ^= ((blitter_data[7] & 0x0400) >> 9);
				last ^= ((blitter_data[0] & 0x2000) >> 10);
			}
			else	/* following lines */
			{
				int val = blitter_base[tempoffs];

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
				int val = blitter_base[tempoffs++];
				int p1, p2, p3, p4;
				p1 = last = ((val ^ artmagic_xor[last]) >>  0) & 0xf;
				p2 = last = ((val ^ artmagic_xor[last]) >>  4) & 0xf;
				p3 = last = ((val ^ artmagic_xor[last]) >>  8) & 0xf;
				p4 = last = ((val ^ artmagic_xor[last]) >> 12) & 0xf;
				fprintf(f, "%c%c%c%c ", hex[p1], hex[p2], hex[p3], hex[p4]);
			}
			fprintf(f, "\n\t");
		}
		fprintf(f, "\n");
	}
}
#endif

	profiler_mark_start(PROFILER_VIDEO);

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
					last ^= (blitter_data[7] & 0x0001);
					if (artmagic_is_stoneball)
						last ^= ((blitter_data[0] & 0x0020) >> 3);
					else	/* ultennis */
						last ^= (((blitter_data[0] + 1) & 0x0040) >> 4);

					/* cheesech */
					last ^= ((blitter_data[7] & 0x0400) >> 9);
					last ^= ((blitter_data[0] & 0x2000) >> 10);
				}
				else	/* following lines */
				{
					int val = blitter_base[offset & blitter_mask];

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
					UINT16 val = blitter_base[(offset + j/4) & blitter_mask];
					if (sx < 508)
					{
						if (h == 1 && artmagic_is_stoneball)
							last = ((val) >>  0) & 0xf;
						else
							last = ((val ^ artmagic_xor[last]) >>  0) & 0xf;
						if (!((maskx << ((j/2) & 7)) & 0x80))
						{
							if (last && sx >= 0 && sx < 512)
								dest[tsy + sx] = color | (last);
							sx++;
						}

						if (h == 1 && artmagic_is_stoneball)
							last = ((val) >>  4) & 0xf;
						else
							last = ((val ^ artmagic_xor[last]) >>  4) & 0xf;
						{
							if (last && sx >= 0 && sx < 512)
								dest[tsy + sx] = color | (last);
							sx++;
						}

						if (h == 1 && artmagic_is_stoneball)
							last = ((val) >>  8) & 0xf;
						else
							last = ((val ^ artmagic_xor[last]) >>  8) & 0xf;
						if (!((maskx << ((j/2) & 7)) & 0x40))
						{
							if (last && sx >= 0 && sx < 512)
								dest[tsy + sx] = color | (last);
							sx++;
						}

						if (h == 1 && artmagic_is_stoneball)
							last = ((val) >> 12) & 0xf;
						else
							last = ((val ^ artmagic_xor[last]) >> 12) & 0xf;
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

	profiler_mark_end();

#if (!INSTANT_BLIT)
	blitter_busy_until = attotime_add(timer_get_time(machine), ATTOTIME_IN_NSEC(w*h*20));
#endif
}


READ16_HANDLER( artmagic_blitter_r )
{
	/*
        bit 1 is a busy flag; loops tightly if clear
        bit 2 is tested in a similar fashion
        bit 4 reflects the page
    */
	UINT16 result = 0xffef | (blitter_page << 4);
#if (!INSTANT_BLIT)
	if (attotime_compare(timer_get_time(space->machine), blitter_busy_until) < 0)
		result ^= 6;
#endif
	return result;
}


WRITE16_HANDLER( artmagic_blitter_w )
{
	COMBINE_DATA(&blitter_data[offset]);

	/* offset 3 triggers the blit */
	if (offset == 3)
		execute_blit(space->machine);

	/* offset 4 contains the target page */
	else if (offset == 4)
		blitter_page = (data >> 1) & 1;
}



/*************************************
 *
 *  Video update
 *
 *************************************/

void artmagic_scanline(const device_config *screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params)
{
	offs_t offset = (params->rowaddr << 12) & 0x7ff000;
	UINT16 *vram = address_to_vram(&offset);
	UINT32 *dest = BITMAP_ADDR32(bitmap, scanline, 0);
	const rgb_t *pens = tlc34076_get_pens();
	int coladdr = params->coladdr << 1;
	int x;

	vram += offset;
	for (x = params->heblnk; x < params->hsblnk; x++)
		dest[x] = pens[vram[coladdr++ & 0x1ff] & 0xff];
}
