// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/* Kaneko Sprites */

/*
    Kaneko 16-bit sprites
       VU-002 052 151021   ('type 0') (blazeon, berlwall etc., confirmed)
       KC-002 L0002 023 9321EK702 ('type 1') (gtmr, bloodwar etc., confirmed)


    [ 1024 Sprites ]

        Sprites are 16 x 16 x 4 in the older games, 16 x 16 x 8 in
        gtmr & gtmr2.
        Sprite type 0 also has a simple effect for keeping
        sprites on the screen


    Notes:
     - Blaze On has 2 sprite chips, with 2 sets of sprite registers, the
       existing code here just handles it as one, ignoring the second
       set of registers, we should really be producing 2 sprite bitmaps
       and manually mixing them.

*/

#include "emu.h"
#include "kaneko_spr.h"

//const device_type KANEKO16_SPRITE = &device_creator<kaneko16_sprite_device>; // we have pure virtual functions
const device_type KANEKO_VU002_SPRITE = &device_creator<kaneko_vu002_sprite_device>;
const device_type KANEKO_KC002_SPRITE = &device_creator<kaneko_kc002_sprite_device>;

kaneko16_sprite_device::kaneko16_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock, device_type type)
	: device_t(mconfig, type, "Kaneko 16-bit Sprites", tag, owner, clock, "kaneko16_sprite", __FILE__),
		device_video_interface(mconfig, *this),
		m_gfxdecode(*this)
{
	m_keep_sprites = 0; // default disabled for games not using it


	m_sprite_xoffs = 0;
	m_sprite_yoffs = 0;

	m_sprite_fliptype = 0;
/*
    Sx = Sprites with priority x, x = tiles with priority x,
    Sprites - Tiles Order (bottom -> top):

    0   S0  1   2   3
    0   1   S1  2   3
    0   1   2   S2  3
    0   1   2   3   S3
*/
	m_priority.sprite[0] = 1;   // above tile[0],   below the others
	m_priority.sprite[1] = 2;   // above tile[0-1], below the others
	m_priority.sprite[2] = 3;   // above tile[0-2], below the others
	m_priority.sprite[3] = 8;   // above all
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void kaneko16_sprite_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<kaneko16_sprite_device &>(device).m_gfxdecode.set_tag(tag);
}

void kaneko16_sprite_device::device_start()
{
	m_first_sprite = auto_alloc_array(machine(), struct kan_tempsprite, 0x400);
	m_sprites_regs = auto_alloc_array_clear(machine(), UINT16, 0x20/2);
	m_screen->register_screen_bitmap(m_sprites_bitmap);

	save_item(NAME(m_sprite_flipx));
	save_item(NAME(m_sprite_flipy));
	save_pointer(NAME(m_sprites_regs), 0x20/2);
	save_item(NAME(m_keep_sprites));
	save_item(NAME(m_sprites_bitmap));
}



void kaneko16_sprite_device::device_reset()
{
	m_sprite_flipx = 0;
	m_sprite_flipy = 0;
}

void kaneko16_sprite_device::set_priorities(device_t &device, int pri0, int pri1, int pri2, int pri3)
{
	kaneko16_sprite_device &dev = downcast<kaneko16_sprite_device &>(device);

	dev.m_priority.sprite[0] = pri0;
	dev.m_priority.sprite[1] = pri1;
	dev.m_priority.sprite[2] = pri2;
	dev.m_priority.sprite[3] = pri3;
}


void kaneko16_sprite_device::set_fliptype(device_t &device, int fliptype)
{
	kaneko16_sprite_device &dev = downcast<kaneko16_sprite_device &>(device);
	dev.m_sprite_fliptype = fliptype;

}

void kaneko16_sprite_device::set_offsets(device_t &device, int xoffs, int yoffs)
{
	kaneko16_sprite_device &dev = downcast<kaneko16_sprite_device &>(device);
	dev.m_sprite_xoffs = xoffs;
	dev.m_sprite_yoffs = yoffs;
}








/***************************************************************************

                                Sprites Drawing

    Sprite data is layed out in RAM in different ways for different games
    (type 0,1). This basically involves the bits in the attribute
    word to be shuffled around and/or the words being in different order.

    Each sprite is always stuffed in 4 words. There may be some extra
    padding words though

    Examples are:

    Type 0: shogwarr, blazeon, bakubrkr.
    Type 1: gtmr.

Offset:         Format:                     Value:

0000.w          Attribute (type 0 & 2)

                    f--- ---- ---- ----     Multisprite: Use Latched Code + 1
                    -e-- ---- ---- ----     Multisprite: Use Latched Color (And Flip?)
                    --d- ---- ---- ----     Multisprite: Use Latched X,Y As Offsets
                    ---c b--- ---- ----     Index Of XY Offset
                    ---- -a-- ---- ----
                    ---- --9- ---- ----     High Priority (vs FG Tiles Of High Priority)
                    ---- ---8 ---- ----     High Priority (vs BG Tiles Of High Priority)
                    ---- ---- 7654 32--     Color
                    ---- ---- ---- --1-     X Flip
                    ---- ---- ---- ---0     Y Flip

                Attribute (type 1)

                    f--- ---- ---- ----     Multisprite: Use Latched Code + 1
                    -e-- ---- ---- ----     Multisprite: Use Latched Color (And Flip?)
                    --d- ---- ---- ----     Multisprite: Use Latched X,Y As Offsets
                    ---c b--- ---- ----     Index Of XY Offset
                    ---- -a-- ---- ----
                    ---- --9- ---- ----     X Flip
                    ---- ---8 ---- ----     Y Flip
                    ---- ---- 7--- ----     High Priority (vs FG Tiles Of High Priority)
                    ---- ---- -6-- ----     High Priority (vs BG Tiles Of High Priority)
                    ---- ---- --54 3210     Color

0002.w                                      Code
0004.w                                      X Position << 6
0006.w                                      Y Position << 6

***************************************************************************/

#define USE_LATCHED_XY      1
#define USE_LATCHED_CODE    2
#define USE_LATCHED_COLOR   4

void kaneko_kc002_sprite_device::get_sprite_attributes(struct kan_tempsprite *s, UINT16 attr)
{
	s->color        =       (attr & 0x003f);
	s->priority     =       (attr & 0x00c0) >> 6;
	s->flipy        =       (attr & 0x0100);
	s->flipx        =       (attr & 0x0200);
	s->code         +=      (s->y & 1) << 16;   // bloodwar
}

void kaneko_vu002_sprite_device::get_sprite_attributes(struct kan_tempsprite *s, UINT16 attr)
{
	s->flipy        =       (attr & 0x0001);
	s->flipx        =       (attr & 0x0002);
	s->color        =       (attr & 0x00fc) >> 2;
	s->priority     =       (attr & 0x0300) >> 8;
}


int kaneko16_sprite_device::kaneko16_parse_sprite_type012(int i, struct kan_tempsprite *s, UINT16* spriteram16, int spriteram16_bytes)
{
	int attr, xoffs, offs;

	offs = i * 8/2;

	if (offs >= (spriteram16_bytes/2))  return -1;

	attr            =       spriteram16[offs + 0];
	s->code         =       spriteram16[offs + 1];
	s->x            =       spriteram16[offs + 2];
	s->y            =       spriteram16[offs + 3];

	// this differs between each chip type
	get_sprite_attributes(s, attr);

	xoffs           =       (attr & 0x1800) >> 11;
	s->yoffs        =       m_sprites_regs[0x10/2 + xoffs*2 + 1];
	s->xoffs        =       m_sprites_regs[0x10/2 + xoffs*2 + 0];

	if (m_sprite_flipy)
	{
		s->yoffs        -=      m_sprites_regs[0x2/2];
		s->yoffs        -=      m_screen->visible_area().min_y<<6;
	}
	else
	{
		s->yoffs        -=      m_sprites_regs[0x2/2];
		s->yoffs        +=      m_screen->visible_area().min_y<<6;
	}

	return                  ( (attr & 0x2000) ? USE_LATCHED_XY    : 0 ) |
							( (attr & 0x4000) ? USE_LATCHED_COLOR : 0 ) |
							( (attr & 0x8000) ? USE_LATCHED_CODE  : 0 ) ;
}

// custom function to draw a single sprite. needed to keep correct sprites - sprites and sprites - tilemaps priorities




template<class _BitmapClass>
void kaneko16_sprite_device::kaneko16_draw_sprites_custom(_BitmapClass &dest_bmp,const rectangle &clip,gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		bitmap_ind8 &priority_bitmap, int priority)
{
	pen_t pen_base = gfx->colorbase() + gfx->granularity() * (color % gfx->colors());
	const UINT8 *source_base = gfx->get_data(code % gfx->elements());
	int sprite_screen_height = ((1<<16)*gfx->height()+0x8000)>>16;
	int sprite_screen_width = ((1<<16)*gfx->width()+0x8000)>>16;

	if (sprite_screen_width && sprite_screen_height)
	{
		/* compute sprite increment per screen pixel */
		int dx = (gfx->width()<<16)/sprite_screen_width;
		int dy = (gfx->height()<<16)/sprite_screen_height;

		int ex = sx+sprite_screen_width;
		int ey = sy+sprite_screen_height;

		int x_index_base;
		int y_index;

		if( flipx )
		{
			x_index_base = (sprite_screen_width-1)*dx;
			dx = -dx;
		}
		else
		{
			x_index_base = 0;
		}

		if( flipy )
		{
			y_index = (sprite_screen_height-1)*dy;
			dy = -dy;
		}
		else
		{
			y_index = 0;
		}

		if( sx < clip.min_x)
		{ /* clip left */
			int pixels = clip.min_x-sx;
			sx += pixels;
			x_index_base += pixels*dx;
		}
		if( sy < clip.min_y )
		{ /* clip top */
			int pixels = clip.min_y-sy;
			sy += pixels;
			y_index += pixels*dy;
		}
		/* NS 980211 - fixed incorrect clipping */
		if( ex > clip.max_x+1 )
		{ /* clip right */
			int pixels = ex-clip.max_x-1;
			ex -= pixels;
		}
		if( ey > clip.max_y+1 )
		{ /* clip bottom */
			int pixels = ey-clip.max_y-1;
			ey -= pixels;
		}

		if (ex > sx)
		{ /* skip if inner loop doesn't draw anything */

			typename _BitmapClass::pixel_t *dest;
			int rgb;
			if (sizeof(*dest) == 2) rgb = 0;
			else rgb = 1;

			const pen_t *pal = gfx->palette()->pens();

			for (int y = sy; y < ey; y++)
			{
				const UINT8 *source = source_base + (y_index >> 16) * gfx->rowbytes();
				dest = &dest_bmp.pix(y);
				UINT8 *pri = &priority_bitmap.pix8(y);

				int x_index = x_index_base;
				for (int x = sx; x < ex; x++)
				{
					int c = source[x_index >> 16];
					if (c != 0)
					{
						if (pri[x] < priority)
						{
							if (!rgb) dest[x] = pen_base + c;
							else dest[x] = pal[pen_base + c];

							pri[x] = 0xff; // mark it "already drawn"
						}
					}
					x_index += dx;
				}

				y_index += dy;
			}
		}
	}
}




/* Build a list of sprites to display & draw them */
template<class _BitmapClass>
void kaneko16_sprite_device::kaneko16_draw_sprites(_BitmapClass &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, UINT16* spriteram16, int spriteram16_bytes)
{
	/* Sprites *must* be parsed from the first in RAM to the last,
	   because of the multisprite feature. But they *must* be drawn
	   from the last in RAM (frontmost) to the first in order to
	   cope with priorities using pdrawgfx.

	   Hence we parse them from first to last and put the result
	   in a temp buffer, then draw the buffer's contents from last
	   to first. */

	int max =   (m_screen->width() > 0x100) ? (0x200<<6) : (0x100<<6);

	int i = 0;
	struct kan_tempsprite *s = m_first_sprite;

	/* These values are latched from the last sprite. */
	int x           =   0;
	int y           =   0;
	int code        =   0;
	int color       =   0;
	int priority    =   0;
	int xoffs       =   0;
	int yoffs       =   0;
	int flipx       =   0;
	int flipy       =   0;

	while (1)
	{
		int flags;

		flags = kaneko16_parse_sprite_type012(i,s, spriteram16, spriteram16_bytes);

		if (flags == -1)    // End of Sprites
			break;

		if (flags & USE_LATCHED_CODE)
			s->code = ++code;   // Use the latched code + 1 ..
		else
			code = s->code;     // .. or latch this value


		if (flags & USE_LATCHED_COLOR)
		{
			s->color        =   color;
			s->priority     =   priority;
			s->xoffs        =   xoffs;
			s->yoffs        =   yoffs;

			if (m_sprite_fliptype==0)
			{
				s->flipx        =   flipx;
				s->flipy        =   flipy;
			}
		}
		else
		{
			color       =   s->color;
			priority    =   s->priority;
			xoffs       =   s->xoffs;
			yoffs       =   s->yoffs;

			if (m_sprite_fliptype==0)
			{
				flipx = s->flipx;
				flipy = s->flipy;
			}
		}

		// brap boys explicitly doesn't want the flip to be latched, maybe there is a different bit to enable that behavior?
		if (m_sprite_fliptype==1)
		{
			flipx       =   s->flipx;
			flipy       =   s->flipy;
		}

		if (flags & USE_LATCHED_XY)
		{
			s->x += x;
			s->y += y;
		}
		// Always latch the latest result
		x   =   s->x;
		y   =   s->y;

		/* We can now buffer this sprite */

		s->x    =   s->xoffs + s->x;
		s->y    =   s->yoffs + s->y;

		s->x    +=  m_sprite_xoffs;
		s->y    +=  m_sprite_yoffs;

		if (m_sprite_flipx) { s->x = max - s->x - (16<<6);  s->flipx = !s->flipx;   }
		if (m_sprite_flipy) { s->y = max - s->y - (16<<6);  s->flipy = !s->flipy;   }

		s->x        =       ( (s->x & 0x7fc0) - (s->x & 0x8000) ) / 0x40;
		s->y        =       ( (s->y & 0x7fc0) - (s->y & 0x8000) ) / 0x40;

		i++;
		s++;
	}


	/* Let's finally draw the sprites we buffered, in reverse order
	   (for pdrawgfx) */

	for (s--; s >= m_first_sprite; s--)
	{
		int curr_pri = s->priority;

		UINT32 primask = m_priority.sprite[curr_pri];

		kaneko16_draw_sprites_custom(
										bitmap,cliprect,m_gfxdecode->gfx(0),
										s->code,
										s->color,
										s->flipx, s->flipy,
										s->x, s->y,
										priority_bitmap,
										primask );
	}
}



/***************************************************************************


                            Sprites Registers

    Offset:         Format:                     Value:

    0000.w          f--- ---- ---- ----         Sprites Disable?? (see blazeon)
                    -edc ba98 7654 3---
                    ---- ---- ---- -2--         Keep sprites on screen (only sprites type 0?)
                    ---- ---- ---- --1-         Flip X
                    ---- ---- ---- ---0         Flip Y

    0002.w                                      Y Offset << 6 (Global)


    0004..000e.w                                ?


    0010.w                                      X Offset << 6 #0
    0012.w                                      Y Offset << 6 #0

    0014.w                                      X Offset << 6 #1
    0016.w                                      Y Offset << 6 #1

    0018.w                                      X Offset << 6 #2
    001a.w                                      Y Offset << 6 #2

    001c.w                                      X Offset << 6 #3
    001e.w                                      Y Offset << 6 #3

***************************************************************************/

/*
[gtmr]

Initial self test:
600000: 4BC0 94C0 4C40 94C0-0404 0002 0000 0000     (Layers 1 regs)
680000: 4BC0 94C0 4C40 94C0-1C1C 0002 0000 0000     (Layers 2 regs)
Race start:
600000: DC00 7D00 DC80 7D00-0404 0002 0000 0000     (Layers 1 regs)
680000: DC00 7D00 DC80 7D00-1C1C 0002 0000 0000     (Layers 2 regs)

[gtmr]
700000: 0040 0000 0001 0180-0000 0000 0000 0000     (Sprites  regs)
700010: 0040 0000 0040 0000-0040 0000 2840 1E00     ; 1,0 .. a1,78
                                                    ; a0*2=screenx/2
                                                    ; 78*2=screeny/2
FLIP ON:
700000: 0043 FFC0 0001 0180-0000 0000 0000 0000     (Sprites  regs)
700010: 2FC0 4400 2FC0 4400-2FC0 4400 57C0 6200     ; bf,110 .. 15f,188
                                                    ; 15f-bf=a0! 188-110=78!

[berlwall]
600000: 48CC 03C0 0001 0100-0000 0000 0000 0000     (Sprites  regs)
600010: 0000 0000 0000 0000-0000 0000 0000 0000
FLIP ON:
600000: 48CF FC00 0001 0100-0000 0000 0000 0000     (Sprites  regs)
600010: 0000 0000 0000 0000-0000 0000 0000 0000

[mgcrystl]
900000: 4FCC 0000 0040 00C0-xxxx 0001 0001 0001     (Sprites  regs)
900010: 0000 FC40 A000 9C40-1E00 1A40 0000 FC40
FLIP ON:
900000: 4FCF 0000 0040 00C0-xxxx 0001 0001 0001     (Sprites  regs)
900010: 0000 0400 A000 A400-1E00 2200 0000 0400     ; +1f<<6 on y
*/

READ16_MEMBER(kaneko16_sprite_device::kaneko16_sprites_regs_r)
{
	return m_sprites_regs[offset];
}

WRITE16_MEMBER(kaneko16_sprite_device::kaneko16_sprites_regs_w)
{
	UINT16 new_data;

	COMBINE_DATA(&m_sprites_regs[offset]);
	new_data  = m_sprites_regs[offset];

	switch (offset)
	{
		case 0:
			if (ACCESSING_BITS_0_7)
			{
				m_sprite_flipx = new_data & 2;
				m_sprite_flipy = new_data & 1;

				if(get_sprite_type() == 0)
					m_keep_sprites = ~new_data & 4;
			}

			break;
	}

//  logerror("CPU #0 PC %06X : Warning, sprites reg %04X <- %04X\n",space.device().safe_pc(),offset*2,data);
}


void kaneko16_sprite_device::kaneko16_copybitmap(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap_trans(bitmap,m_sprites_bitmap,0,0,0,0,cliprect,0);
}

void kaneko16_sprite_device::kaneko16_copybitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pal = m_gfxdecode->gfx(0)->palette()->pens();
	UINT16* srcbitmap;
	UINT32* dstbitmap;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		srcbitmap = &m_sprites_bitmap.pix16(y);
		dstbitmap = &bitmap.pix32(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			UINT16 pix = srcbitmap[x];
			if (pix) dstbitmap[x] = pal[pix];
		}
	}
}



void kaneko16_sprite_device::kaneko16_render_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, UINT16* spriteram16, int spriteram16_bytes) { kaneko16_render_sprites_common(bitmap, cliprect, priority_bitmap, spriteram16, spriteram16_bytes); }
void kaneko16_sprite_device::kaneko16_render_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, UINT16* spriteram16, int spriteram16_bytes) { kaneko16_render_sprites_common(bitmap, cliprect, priority_bitmap, spriteram16, spriteram16_bytes); }

template<class _BitmapClass>
void kaneko16_sprite_device::kaneko16_render_sprites_common(_BitmapClass &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, UINT16* spriteram16, int spriteram16_bytes)
{
	/* Sprites last (rendered with pdrawgfx, so they can slip
	   in between the layers) */

	if(m_keep_sprites)
	{
		/* keep sprites on screen - used by mgcrystl when you get the first gem and it shows instructions */
		kaneko16_draw_sprites(m_sprites_bitmap, cliprect, priority_bitmap, spriteram16, spriteram16_bytes);
		kaneko16_copybitmap(bitmap,cliprect);
	}
	else
	{
		m_sprites_bitmap.fill(0, cliprect);
		kaneko16_draw_sprites(bitmap,cliprect, priority_bitmap, spriteram16, spriteram16_bytes);
	}
}

kaneko_vu002_sprite_device::kaneko_vu002_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: kaneko16_sprite_device(mconfig, tag, owner, clock, KANEKO_VU002_SPRITE)
{
}

kaneko_kc002_sprite_device::kaneko_kc002_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: kaneko16_sprite_device(mconfig, tag, owner, clock, KANEKO_KC002_SPRITE)
{
}

// this is a bootleg implementation, used by Gals Hustler and Zip Zap, the latter not really working at all well with the original
// link features (assuming the bad program roms aren't the cause)  it's clearly derived from this sprite system tho.
void kaneko16_sprite_device::bootleg_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16* spriteram16, int spriteram16_bytes)
{
//  UINT16 *spriteram16 = m_spriteram;
	int offs;
	int sx=0, sy=0;

	for (offs = 0;offs < spriteram16_bytes/2;offs += 4)
	{
		int code,color,flipx,flipy;

		code = spriteram16[offs + 1] & 0x1fff;
		color = (spriteram16[offs] & 0x003c) >> 2;
		flipx = spriteram16[offs] & 0x0002;
		flipy = spriteram16[offs] & 0x0001;

		if((spriteram16[offs] & 0x6000) == 0x6000) /* Link bits */
		{
			sx += spriteram16[offs + 2] >> 6;
			sy += spriteram16[offs + 3] >> 6;
		}
		else
		{
			sx = spriteram16[offs + 2] >> 6;
			sy = spriteram16[offs + 3] >> 6;
		}

		sx = (sx&0x1ff) - (sx&0x200);
		sy = (sy&0x1ff) - (sy&0x200);

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,0);
	}
}
