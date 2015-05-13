// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Gal's Panic II =-

                    driver by   Luca Elia (l.elia@tin.it)


***************************************************************************/

#include "emu.h"
#include "includes/galpani2.h"

/*
304000:0040 0000 0100 0000-0000 0000 0000 0000      (Sprites regs)
304010:16C0 0200 16C0 0200-16C0 0200 16C0 0200
*/

/***************************************************************************


                        Palettized Background Layers


***************************************************************************/


#ifdef UNUSED_DEFINITION
inline UINT16 galpani2_state::galpani2_bg8_regs_r(address_space &space, offs_t offset, int n)
{
	switch (offset * 2)
	{
		case 0x16:  return machine().rand() & 1;
		default:
			logerror("CPU #0 PC %06X : Warning, bg8 #%d screen reg %04X read\n",space.cpu->safe_pc(),_n_,offset*2);
	}
	return m_bg8_regs[_n_][offset];
}

/*
    000-3ff     row? scroll
    400         ?
    800-bff     col? scroll
    c04         0003 flip, 0300 flip?
    c1c/e       01ff scroll, 3000 ?
*/
inline void galpani2_state::galpani2_bg8_regs_w(address_space &space, offs_t offset, UINT16 data, UINT16 mem_mask, int _n_)
{
	COMBINE_DATA(&m_bg8_regs[_n_][offset]);
}

READ16_MEMBER( galpani2_bg8_regs_0_r ) { return galpani2_bg8_regs_r(space, offset, 0); }
READ16_MEMBER( galpani2_bg8_regs_1_r ) { return galpani2_bg8_regs_r(space, offset, 1); }

WRITE16_MEMBER( galpani2_bg8_regs_0_w ) { galpani2_bg8_regs_w(space, offset, data, mem_mask, 0); }
WRITE16_MEMBER( galpani2_bg8_regs_1_w ) { galpani2_bg8_regs_w(space, offset, data, mem_mask, 1); }
#endif



/***************************************************************************


                            Video Init Functions


***************************************************************************/

PALETTE_INIT_MEMBER(galpani2_state, galpani2)
{
	int i;
	/* first $4200 colors are dynamic */

	/* initialize 555 RGB lookup */
	for (i = 0; i < 0x8000; i++)
		palette.set_pen_color(i,pal5bit(i >> 5),pal5bit(i >> 10),pal5bit(i >> 0));
}

void galpani2_state::video_start()
{
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

// based on videos these 8-bit layers actually get *blended* against the RGB555 layer
// it should be noted that in the layer at 0x500000 the upper 8 bits are set too, this could be related

// scrolling is based on sync between sprite and this layer during the 'keyhole viewer' between rounds (use cheats)
void galpani2_state::copybg8(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer)
{
	int x = + ( *m_bg8_scrollx[layer] + 0x200 - 0x1be );
	int y = + ( *m_bg8_scrolly[layer] + 0x200 - 0x0f5 );
	UINT16* ram = m_bg8[layer];

	const pen_t *clut = &m_bg8palette->pen(0);
	for (int xx = 0; xx < 320; xx++)
	{
		for (int yy = 0; yy < 240; yy++)
		{
			UINT16 pen = ram[(((y + yy) & 0xff) * 512) + ((x + xx) & 0x1ff)];
			if (pen) bitmap.pix32(yy, xx) = clut[pen & 0xff];
		}
	}
}

// this seems to be 256x256 pages (arranged as 1024*256), but the game resolution is 320x240
// https://www.youtube.com/watch?v=2b2SLFtC0uA is a video of the galpanic2j set, and shows the RGB pattern at
// startup covering all screen lines - is the hardware mixing bitmaps of different resolutions or is there a
// line select somewhere?  I should find the gal images and find what resolution they're stored at too.
// (or is this just wrong format / layout due to protection?)
void galpani2_state::copybg15(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT16* ram = m_bg15 + 0x40000/2;

	//int x = 0;
	//int y = 0;

	const pen_t *clut = &m_bg15palette->pen(0);
	for (int xx = 0; xx < 320; xx++)
	{
		for (int yy = 0; yy < 240; yy++)
		{
			UINT16 pen = ram[(xx * 0x800) + yy];
			bitmap.pix32(yy, xx) = clut[pen & 0x7fff];
		}
	}
}

UINT32 galpani2_state::screen_update_galpani2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#if 1 // MAME_DEBUG
if (machine().input().code_pressed(KEYCODE_Z))
{
	int msk = 0;
	if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
	if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
	if (machine().input().code_pressed(KEYCODE_E))  msk |= 4;
	if (machine().input().code_pressed(KEYCODE_A))  msk |= 8;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	bitmap.fill(0, cliprect);
	screen.priority().fill(0, cliprect);

/*  test mode:
    304000:0040 0000 0100 0000-0000 0000 0000 0000      (Sprite regs)
    304010:16C0 0200 16C0 0200-16C0 0200 16C0 0200
    16c0/40 = 5b        200/40 = 8
    scrollx = f5, on screen x should be 0 (f5+5b = 150) */

	if (layers_ctrl & 0x1) copybg15(screen, bitmap, cliprect);
	if (layers_ctrl & 0x2) copybg8(screen, bitmap, cliprect, 0);
	if (layers_ctrl & 0x4) copybg8(screen, bitmap, cliprect, 1);
	if (layers_ctrl & 0x8) m_kaneko_spr->kaneko16_render_sprites(bitmap, cliprect, screen.priority(), m_spriteram, m_spriteram.bytes());
	return 0;
}
