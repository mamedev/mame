/***************************************************************************

    Sun Electronics Kangaroo hardware

***************************************************************************/

#include "emu.h"
#include "includes/kangaroo.h"

static void blitter_execute(running_machine &machine);


/*************************************
 *
 *  Video setup
 *
 *************************************/

VIDEO_START( kangaroo )
{
	kangaroo_state *state = machine.driver_data<kangaroo_state>();

	/* video RAM is accessed 32 bits at a time (two planes, 4bpp each, 4 pixels) */
	state->m_videoram = auto_alloc_array(machine, UINT32, 256 * 64);
	state->save_pointer(NAME(state->m_videoram), 256 * 64);
}



/*************************************
 *
 *  Video RAM accesses
 *
 *************************************/

static void videoram_write( running_machine &machine, UINT16 offset, UINT8 data, UINT8 mask )
{
	kangaroo_state *state = machine.driver_data<kangaroo_state>();
	UINT32 expdata, layermask;

	/* data contains 4 2-bit values packed as DCBADCBA; expand these into 4 8-bit values */
	expdata = 0;
	if (data & 0x01) expdata |= 0x00000055;
	if (data & 0x10) expdata |= 0x000000aa;
	if (data & 0x02) expdata |= 0x00005500;
	if (data & 0x20) expdata |= 0x0000aa00;
	if (data & 0x04) expdata |= 0x00550000;
	if (data & 0x40) expdata |= 0x00aa0000;
	if (data & 0x08) expdata |= 0x55000000;
	if (data & 0x80) expdata |= 0xaa000000;

	/* determine which layers are enabled */
	layermask = 0;
	if (mask & 0x08) layermask |= 0x30303030;
	if (mask & 0x04) layermask |= 0xc0c0c0c0;
	if (mask & 0x02) layermask |= 0x03030303;
	if (mask & 0x01) layermask |= 0x0c0c0c0c;

	/* update layers */
	state->m_videoram[offset] = (state->m_videoram[offset] & ~layermask) | (expdata & layermask);
}


WRITE8_MEMBER(kangaroo_state::kangaroo_videoram_w)
{
	videoram_write(machine(), offset, data, m_video_control[8]);
}



/*************************************
 *
 *  Video control writes
 *
 *************************************/

WRITE8_MEMBER(kangaroo_state::kangaroo_video_control_w)
{
	m_video_control[offset] = data;

	switch (offset)
	{
		case 5:	/* blitter start */
			blitter_execute(machine());
			break;

		case 8:	/* bank select */
			memory_set_bank(machine(), "bank1", (data & 0x05) ? 0 : 1);
			break;
	}
}



/*************************************
 *
 *  DMA blitter
 *
 *************************************/

static void blitter_execute( running_machine &machine )
{
	kangaroo_state *state = machine.driver_data<kangaroo_state>();
	UINT32 gfxhalfsize = machine.region("gfx1")->bytes() / 2;
	const UINT8 *gfxbase = machine.region("gfx1")->base();
	UINT16 src = state->m_video_control[0] + 256 * state->m_video_control[1];
	UINT16 dst = state->m_video_control[2] + 256 * state->m_video_control[3];
	UINT8 height = state->m_video_control[5];
	UINT8 width = state->m_video_control[4];
	UINT8 mask = state->m_video_control[8];
	int x, y;

	/* during DMA operations, the top 2 bits are ORed together, as well as the bottom 2 bits */
	/* adjust the mask to account for this */
	if (mask & 0x0c) mask |= 0x0c;
	if (mask & 0x03) mask |= 0x03;

	/* loop over height, then width */
	for (y = 0; y <= height; y++, dst += 256)
		for (x = 0; x <= width; x++)
		{
			UINT16 effdst = (dst + x) & 0x3fff;
			UINT16 effsrc = src++ & (gfxhalfsize - 1);
			videoram_write(machine, effdst, gfxbase[0 * gfxhalfsize + effsrc], mask & 0x05);
			videoram_write(machine, effdst, gfxbase[1 * gfxhalfsize + effsrc], mask & 0x0a);
		}
}



/*************************************
 *
 *  Video updater
 *
 *************************************/

SCREEN_UPDATE_RGB32( kangaroo )
{
	kangaroo_state *state = screen.machine().driver_data<kangaroo_state>();
	UINT8 scrolly = state->m_video_control[6];
	UINT8 scrollx = state->m_video_control[7];
	UINT8 maska = (state->m_video_control[10] & 0x28) >> 3;
	UINT8 maskb = (state->m_video_control[10] & 0x07) >> 0;
	UINT8 xora = (state->m_video_control[9] & 0x20) ? 0xff : 0x00;
	UINT8 xorb = (state->m_video_control[9] & 0x10) ? 0xff : 0x00;
	UINT8 enaa = (state->m_video_control[9] & 0x08);
	UINT8 enab = (state->m_video_control[9] & 0x04);
	UINT8 pria = (~state->m_video_control[9] & 0x02);
	UINT8 prib = (~state->m_video_control[9] & 0x01);
	rgb_t pens[8];
	int x, y;

	/* build up the pens arrays */
	for (x = 0; x < 8; x++)
		pens[x] = MAKE_RGB(pal1bit(x >> 2), pal1bit(x >> 1), pal1bit(x >> 0));

	/* iterate over pixels */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT32 *dest = &bitmap.pix32(y);

		for (x = cliprect.min_x; x <= cliprect.max_x; x += 2)
		{
			UINT8 effxa = scrollx + ((x / 2) ^ xora);
			UINT8 effya = scrolly + (y ^ xora);
			UINT8 effxb = (x / 2) ^ xorb;
			UINT8 effyb = y ^ xorb;
			UINT8 pixa = (state->m_videoram[effya + 256 * (effxa / 4)] >> (8 * (effxa % 4) + 0)) & 0x0f;
			UINT8 pixb = (state->m_videoram[effyb + 256 * (effxb / 4)] >> (8 * (effxb % 4) + 4)) & 0x0f;
			UINT8 finalpens;

			/* for each layer, contribute bits if (a) enabled, and (b) either has priority or the opposite plane is 0 */
			finalpens = 0;
			if (enaa && (pria || pixb == 0))
				finalpens |= pixa;
			if (enab && (prib || pixa == 0))
				finalpens |= pixb;

			/* store the first of two pixels, which is always full brightness */
			dest[x + 0] = pens[finalpens & 7];

			/* KOS1 alternates at 5MHz, offset from the pixel clock by 1/2 clock */
			/* when 0, it enables the color mask for pixels with Z = 0 */
			finalpens = 0;
			if (enaa && (pria || pixb == 0))
			{
				if (!(pixa & 0x08)) pixa &= maska;
				finalpens |= pixa;
			}
			if (enab && (prib || pixa == 0))
			{
				if (!(pixb & 0x08)) pixb &= maskb;
				finalpens |= pixb;
			}

			/* store the second of two pixels, which is affected by KOS1 and the A/B masks */
			dest[x + 1] = pens[finalpens & 7];
		}
	}

	return 0;
}
