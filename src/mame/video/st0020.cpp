// license:BSD-3-Clause
// copyright-holders:Luca Elia,David Haywood
/* ST0020 - Seta Zooming Sprites + Blitter

  (gdfs also has a tilemap, I don't know if this chip supplies that)

 The ST0032 seems very similar, used by the newer Jockey Club II boards

*/


#include "emu.h"
#include "st0020.h"
#include "render.h"

const device_type ST0020_SPRITES = &device_creator<st0020_device>;

st0020_device::st0020_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ST0020_SPRITES, "Seta ST0020 Sprites", tag, owner, clock, "st0020", __FILE__),
		m_gfxdecode(*this),
		m_palette(*this)
{
	m_is_st0032 = 0;
	m_is_jclub2 = 0;
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void st0020_device::static_set_gfxdecode_tag(device_t &device, std::string tag)
{
	downcast<st0020_device &>(device).m_gfxdecode.set_tag(tag);
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void st0020_device::static_set_palette_tag(device_t &device, std::string tag)
{
	downcast<st0020_device &>(device).m_palette.set_tag(tag);
}

void st0020_device::set_is_st0032(device_t &device, int is_st0032)
{
	st0020_device &dev = downcast<st0020_device &>(device);
	dev.m_is_st0032 = is_st0032;
}

void st0020_device::set_is_jclub2o(device_t &device, int is_st0032)
{
	st0020_device &dev = downcast<st0020_device &>(device);
	dev.m_is_jclub2 = is_st0032;
}



static const gfx_layout layout_16x8x8_2 =
{
	16,8,
	0x400000/(16*8),
	8,
	{   STEP8(0,1)      },
	{   STEP16(0,8)     },
	{   STEP8(0,16*8)   },
	16*8*8
};



void st0020_device::device_start()
{
	m_st0020_gfxram = make_unique_clear<UINT16[]>(4 * 0x100000 / 2);
	m_st0020_spriteram = make_unique_clear<UINT16[]>(0x80000 / 2);
	m_st0020_blitram = make_unique_clear<UINT16[]>(0x100 / 2);

	for (m_gfx_index = 0; m_gfx_index < MAX_GFX_ELEMENTS; m_gfx_index++)
		if (m_gfxdecode->gfx(m_gfx_index) == nullptr)
			break;

	m_gfxdecode->set_gfx(m_gfx_index, std::make_unique<gfx_element>(m_palette, layout_16x8x8_2, (UINT8 *)m_st0020_gfxram.get(), 0, m_palette->entries() / 64, 0));

	m_gfxdecode->gfx(m_gfx_index)->set_granularity(64); /* 256 colour sprites with palette selectable on 64 colour boundaries */

	save_pointer(NAME(m_st0020_gfxram.get()), 4 * 0x100000/2);
	save_pointer(NAME(m_st0020_spriteram.get()), 0x80000/2);
	save_pointer(NAME(m_st0020_blitram.get()), 0x100/2);
	save_item(NAME(m_st0020_gfxram_bank));
}

void st0020_device::device_reset()
{
	m_st0020_gfxram_bank = 0;
}


READ16_MEMBER(st0020_device::st0020_gfxram_r)
{
	UINT16 data;
	ST0020_ST0032_BYTESWAP_MEM_MASK

	data = m_st0020_gfxram[offset + m_st0020_gfxram_bank * 0x100000/2];

	ST0020_ST0032_BYTESWAP_DATA
	return data;
}

WRITE16_MEMBER(st0020_device::st0020_gfxram_w)
{
	ST0020_ST0032_BYTESWAP_MEM_MASK
	ST0020_ST0032_BYTESWAP_DATA

	offset += m_st0020_gfxram_bank * 0x100000/2;
	COMBINE_DATA(&m_st0020_gfxram[offset]);
	m_gfxdecode->gfx(m_gfx_index)->mark_dirty(offset / (16*8/2));
}

READ16_MEMBER(st0020_device::st0020_sprram_r)
{
	UINT16 data;

	data = m_st0020_spriteram[offset];

	return data;
}

WRITE16_MEMBER(st0020_device::st0020_sprram_w)
{
	COMBINE_DATA(&m_st0020_spriteram[offset]);
}

READ16_MEMBER(st0020_device::st0020_blit_r)
{
	switch (offset)
	{
		case 0x00/2:
			// blitter status? (bit C, bit A)
			return 0;
	}

	logerror("CPU #0 PC: %06X - Blit reg read: %02X\n",space.device().safe_pc(),offset*2);
	return 0;
}

READ16_MEMBER(st0020_device::st0020_blitram_r)
{
	UINT16 data;
	data = st0020_blit_r(space, offset, mem_mask);
	return data;
}

WRITE16_MEMBER(st0020_device::st0020_blit_w)
{
	UINT16 *st0020_blitram = m_st0020_blitram.get();

	COMBINE_DATA(&st0020_blitram[offset]);

	switch (offset)
	{
		case 0x8a/2:
		{
			if (data & ~0x43)
				logerror("CPU #0 PC: %06X - Unknown st0020_gfxram_bank bit written %04X\n",space.device().safe_pc(),data);

			if (ACCESSING_BITS_0_7)
				m_st0020_gfxram_bank = data & 3;
		}
		break;

		case 0xc0/2:
		case 0xc2/2:
		case 0xc4/2:
		case 0xc6/2:
		case 0xc8/2:
		break;

		case 0xca/2:
		{
			UINT32 src  =   (st0020_blitram[0xc0/2] + (st0020_blitram[0xc2/2] << 16)) << 1;
			UINT32 dst  =   (st0020_blitram[0xc4/2] + (st0020_blitram[0xc6/2] << 16)) << 4;
			UINT32 len  =   (st0020_blitram[0xc8/2]) << 4;

			UINT8 *rom  =   memregion(":st0020")->base();


			if (!rom)
			{
				logerror("CPU #0 PC: %06X - Blit out of range: src %x, dst %x, len %x\n",space.device().safe_pc(),src,dst,len);
				return;
			}

			size_t size =   memregion(":st0020")->bytes();

			if ( (src+len <= size) && (dst+len <= 4 * 0x100000) )
			{
				memcpy( &m_st0020_gfxram[dst/2], &rom[src], len );

				if (len % (16*8))   len = len / (16*8) + 1;
				else                len = len / (16*8);

				dst /= 16*8;
				while (len--)
				{
					m_gfxdecode->gfx(m_gfx_index)->mark_dirty(dst);
					dst++;
				}
			}
			else
			{
				logerror("CPU #0 PC: %06X - Blit out of range: src %x, dst %x, len %x\n",space.device().safe_pc(),src,dst,len);
			}
		}
		break;

		default:
			logerror("CPU #0 PC: %06X - Blit reg written: %02X <- %04X\n",space.device().safe_pc(),offset*2,data);
	}
}

WRITE16_MEMBER(st0020_device::st0020_blitram_w)
{
	st0020_blit_w(space, offset, data, mem_mask);
}



/*
    Sprites RAM is 0x80000 bytes long. The first 0x2000? bytes hold a list
    of sprites to display (the list can be made shorter using an end-of-list
    marker).

    Each entry in the list (16 bytes) is a multi-sprite (e.g it tells the
    hardware to display several single-sprites).

    The list looks like this:

    Offset:     Bits:                   Value:

        0.h     fedc ba-- ---- ----
                ---- --98 7654 3210     X displacement

        2.h     fedc ba-- ---- ----
                ---- --98 7654 3210     Y displacement

        4.h     f--- ---- ---- ----     List end
                -edc ba98 7654 3210     Offset of the single-sprite(s) data

        0.h                             Number of single-sprites (how many bits?)

    A single-sprite is:

    Offset:     Bits:                   Value:

        0.h                             Code

        2.h     f--- ---- ---- ----     Flip X
                -e-- ---- ---- ----     Flip Y
                ---- -a-- ---- ----     0 = 256 color steps, 1 = 64 color steps
                ---- --98 7654 3210     Color code

        4.h     fedc ba-- ---- ----
                ---- --98 7654 3210     X displacement

        6.h     fedc ba-- ---- ----
                ---- --98 7654 3210     Y displacement

        8.h     fedc ba98 ---- ----     Y Size
                ---- ---- 7654 3210     X Size

        A.h     fedc ba98 ---- ----
                ---- ---- 7654 ----     Priority
                ---- ---- ---- 32--     Y Tiles (1,2,4,8)
                ---- ---- ---- --10     X Tiles (1,2,4,8)

        C.h                             Unused

        E.h                             Unused

*/
void st0020_device::st0020_draw_zooming_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	/* Sprites list */
	UINT16 *spriteram16_2 = m_st0020_spriteram.get();
	UINT16 *s1  =   spriteram16_2;
	UINT16 *end1    =   spriteram16_2 + 0x02000/2;

	priority <<= 4;

	for ( ; s1 < end1; s1+=8/2 )
	{
		int attr, code, color, num, sprite, zoom, size;
		int sx, x, xoffs, flipx, xnum, xstart, xend, xinc, xdim, xscale;
		int sy, y, yoffs, flipy, ynum, ystart, yend, yinc, ydim, yscale;

		if (!m_is_st0032)
		{
			xoffs   =       s1[ 0 ];
			yoffs   =       s1[ 1 ];
			sprite  =       s1[ 2 ];
			num     =       s1[ 3 ] % 0x101; // how many?
		}
		else
		{
			// these seem to be swapped around on the st0032
			xoffs   =       s1[ 2 ];
			yoffs   =       s1[ 3 ];
			sprite  =       s1[ 1 ];
			num     =       s1[ 0 ] % 0x101; // how many?

		}



		/* Last sprite */
		if (sprite & 0x8000) break;

		int s2 = 0;
		int spritebase = (sprite & 0x7fff) * 16/2;

		for( ; num > 0; num--,s2+=16/2 )
		{
			code    =   spriteram16_2[(spritebase + s2 + 0 )&0x3ffff];
			attr    =   spriteram16_2[(spritebase + s2 + 1 )&0x3ffff];
			sx      =   spriteram16_2[(spritebase + s2 + 2 )&0x3ffff];
			sy      =   spriteram16_2[(spritebase + s2 + 3 )&0x3ffff];
			zoom    =   spriteram16_2[(spritebase + s2 + 4 )&0x3ffff];
			size    =   spriteram16_2[(spritebase + s2 + 5 )&0x3ffff];



			if (priority != (size & 0xf0))
				break;

			flipx   =   (attr & 0x8000);
			flipy   =   (attr & 0x4000);

/*
            if ((ssv_scroll[0x74/2] & 0x1000) && ((ssv_scroll[0x74/2] & 0x2000) == 0))
            {
                if (flipx == 0) flipx = 1; else flipx = 0;
            }
            if ((ssv_scroll[0x74/2] & 0x4000) && ((ssv_scroll[0x74/2] & 0x2000) == 0))
            {
                if (flipy == 0) flipy = 1; else flipy = 0;
            }
*/

			color   =   (attr & 0x0400) ? attr : attr * 4;

			/* Single-sprite tile size */
			xnum = 1 << ((size >> 0) & 3);
			ynum = 1 << ((size >> 2) & 3);

			xnum = (xnum + 1) / 2;

			if (flipx)  { xstart = xnum-1;  xend = -1;    xinc = -1; }
			else        { xstart = 0;       xend = xnum;  xinc = +1; }

			if (flipy)  { ystart = ynum-1;  yend = -1;    yinc = -1; }
			else        { ystart = 0;       yend = ynum;  yinc = +1; }

			/* Apply global offsets */
			sx  +=  xoffs;
			sy  +=  yoffs;

			/* Sign extend the position */
			sx  =   (sx & 0x1ff) - (sx & 0x200);
			sy  =   (sy & 0x1ff) - (sy & 0x200);

			sy  =   -sy;

			// otherwise everything is off-screen
			if (m_is_jclub2)
				sy += 0x100;

			/* Use fixed point values (16.16), for accuracy */
			sx <<= 16;
			sy <<= 16;

			xdim    =   ( ( ((zoom >> 0) & 0xff) + 1) << 16 ) / xnum;
			ydim    =   ( ( ((zoom >> 8) & 0xff) + 1) << 16 ) / ynum;

			xscale  =   xdim / 16;
			yscale  =   ydim / 8;

			/* Let's approximate to the nearest greater integer value
			   to avoid holes in between tiles */
			if (xscale & 0xffff)    xscale += (1<<16) / 16;
			if (yscale & 0xffff)    yscale += (1<<16) / 8;

			/* Draw the tiles */

			for (x = xstart; x != xend; x += xinc)
			{
				for (y = ystart; y != yend; y += yinc)
				{
					m_gfxdecode->gfx(m_gfx_index)->zoom_transpen(bitmap,cliprect,
									code++,
									color,
									flipx, flipy,
									(sx + x * xdim) / 0x10000, (sy + y * ydim) / 0x10000,
									xscale, yscale, 0
					);
				}
			}


#if 0 /* doesn't compile in a device context (can't use ui_draw_text? */
			if (machine().input().code_pressed(KEYCODE_Z))    /* Display some info on each sprite */
			{
				char buf[10];
				sprintf(buf, "%X",size);
				ui_draw_text(&machine().render().ui_container(), buf, sx / 0x10000, sy / 0x10000);
			}
#endif
		}   /* single-sprites */



	}   /* sprites list */
}



void st0020_device::st0020_draw_all(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int pri = 0; pri <= 0xf; pri++)
		st0020_draw_zooming_sprites(bitmap, cliprect, pri);
}
