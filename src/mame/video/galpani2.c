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

inline void galpani2_state::galpani2_bg8_w(offs_t offset, UINT16 data, UINT16 mem_mask, int _n_)
{
	int x,y,pen;
	UINT16 newword = COMBINE_DATA(&m_bg8[_n_][offset]);
	pen =   newword & 0xff;
	x   =   (offset % 512); /* 512 x 256 */
	y   =   (offset / 512);
	m_bg8_bitmap[_n_]->pix16(y, x) = 0x4000 + pen;
}

WRITE16_MEMBER( galpani2_state::galpani2_bg8_0_w ) { galpani2_bg8_w(offset, data, mem_mask, 0); }
WRITE16_MEMBER( galpani2_state::galpani2_bg8_1_w ) { galpani2_bg8_w(offset, data, mem_mask, 1); }

inline void galpani2_state::galpani2_palette_w(offs_t offset, UINT16 data, UINT16 mem_mask, int _n_)
{
	UINT16 newword = COMBINE_DATA(&m_palette_val[_n_][offset]);
	m_palette->set_pen_color( offset + 0x4000 + _n_ * 0x100, pal5bit(newword >> 5), pal5bit(newword >> 10), pal5bit(newword >> 0) );
}

WRITE16_MEMBER( galpani2_state::galpani2_palette_0_w ) { galpani2_palette_w(offset, data, mem_mask, 0); }
WRITE16_MEMBER( galpani2_state::galpani2_palette_1_w ) { galpani2_palette_w(offset, data, mem_mask, 1); }


/***************************************************************************


                            xRGB  Background Layer


***************************************************************************/

/* 8 horizontal pages of 256x256 pixels? */
WRITE16_MEMBER( galpani2_state::galpani2_bg15_w )
{
	UINT16 newword = COMBINE_DATA(&m_bg15[offset]);

	int x = (offset % 256) + (offset / (256*256)) * 256 ;
	int y = (offset / 256) % 256;

	m_bg15_bitmap->pix16(y, x) = 0x4200 + (newword & 0x7fff);
}


/***************************************************************************


                            Video Init Functions


***************************************************************************/

PALETTE_INIT_MEMBER(galpani2_state, galpani2)
{
	int i;
	/* first $4200 colors are dynamic */

	/* initialize 555 RGB lookup */
	for (i = 0; i < 0x8000; i++)
		palette.set_pen_color(0x4200+i,pal5bit(i >> 5),pal5bit(i >> 10),pal5bit(i >> 0));
}

void galpani2_state::video_start()
{
	m_bg15_bitmap  = auto_bitmap_ind16_alloc(machine(), 256*8, 256);
	m_bg8_bitmap[0] = auto_bitmap_ind16_alloc(machine(), 512, 256);
	m_bg8_bitmap[1] = auto_bitmap_ind16_alloc(machine(), 512, 256);
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

UINT32 galpani2_state::screen_update_galpani2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
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

	if (layers_ctrl & 0x1)
	{
		int x = 0;
		int y = 0;
		copyscrollbitmap_trans(bitmap, *m_bg15_bitmap,
								1, &x, 1, &y,
								cliprect,0x4200 + 0);
	}

/*  test mode:
    304000:0040 0000 0100 0000-0000 0000 0000 0000      (Sprite regs)
    304010:16C0 0200 16C0 0200-16C0 0200 16C0 0200
    16c0/40 = 5b        200/40 = 8
    scrollx = f5, on screen x should be 0 (f5+5b = 150) */

	if (layers_ctrl & 0x2)
	{
		int x = - ( *m_bg8_scrollx[0] + 0x200 - 0x0f5 );
		int y = - ( *m_bg8_scrolly[0] + 0x200 - 0x1be );
		copyscrollbitmap_trans(bitmap, *m_bg8_bitmap[0],
								1, &x, 1, &y,
								cliprect,0x4000 + 0);
	}

	if (layers_ctrl & 0x4)
	{
		int x = - ( *m_bg8_scrollx[1] + 0x200 - 0x0f5 );
		int y = - ( *m_bg8_scrolly[1] + 0x200 - 0x1be );
		copyscrollbitmap_trans(bitmap, *m_bg8_bitmap[1],
								1, &x, 1, &y,
								cliprect,0x4000 + 0);
	}

	if (layers_ctrl & 0x8) m_kaneko_spr->kaneko16_render_sprites(machine(), bitmap, cliprect, screen.priority(), m_spriteram, m_spriteram.bytes());
	return 0;
}
