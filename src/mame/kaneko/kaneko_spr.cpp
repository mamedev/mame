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
#include "screen.h"

DEFINE_DEVICE_TYPE(KANEKO_VU002_SPRITE, kaneko_vu002_sprite_device, "kaneko_vu002", "Kaneko VU002 Sprites")
DEFINE_DEVICE_TYPE(KANEKO_KC002_SPRITE, kaneko_kc002_sprite_device, "kaneko_kc002", "Kaneko KC002 Sprites")

kaneko16_sprite_device::kaneko16_sprite_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, nullptr)
	, device_video_interface(mconfig, *this)
	, m_colbase(0)
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

void kaneko16_sprite_device::device_start()
{
	m_first_sprite = std::make_unique<struct tempsprite_t[]>(0x400);
	m_sprites_regs = make_unique_clear<u16[]>(0x20/2);
	screen().register_screen_bitmap(m_sprites_bitmap);
	screen().register_screen_bitmap(m_sprites_maskmap);

	save_item(NAME(m_sprite_flipx));
	save_item(NAME(m_sprite_flipy));
	save_pointer(NAME(m_sprites_regs), 0x20/2);
	save_item(NAME(m_keep_sprites));
	save_item(NAME(m_sprites_bitmap));
	save_item(NAME(m_sprites_maskmap));
}


GFXDECODE_MEMBER(kaneko_vu002_sprite_device::gfxinfo)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, gfx_8x8x4_row_2x2_group_packed_msb, 0, 0x40)
GFXDECODE_END


void kaneko_vu002_sprite_device::device_start()
{
	decode_gfx(gfxinfo);
	gfx(0)->set_colorbase(m_colbase);
	kaneko16_sprite_device::device_start();
}


/*
    16x16x8 made of 4 8x8x8 blocks arranged like:   01
                                                    23
*/
static gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8),   STEP8(8*8*8*1,8)   },
	{ STEP8(0,8*8), STEP8(8*8*8*2,8*8) },
	16*16*8
};

GFXDECODE_MEMBER(kaneko_kc002_sprite_device::gfxinfo)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, layout_16x16x8, 0, 0x40)
GFXDECODE_END


void kaneko_kc002_sprite_device::device_start()
{
	decode_gfx(gfxinfo);
	gfx(0)->set_colorbase(m_colbase);
	kaneko16_sprite_device::device_start();
}


void kaneko16_sprite_device::device_reset()
{
	m_sprite_flipx = false;
	m_sprite_flipy = false;
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

void kaneko_kc002_sprite_device::get_sprite_attributes(struct tempsprite_t *s, u16 attr)
{
	s->color    = (attr & 0x003f);
	s->priority = (attr & 0x00c0) >> 6;
	s->flipy    = BIT(attr, 8);
	s->flipx    = BIT(attr, 9);
	s->code    += (s->y & 1) << 16;   // bloodwar
}

void kaneko_vu002_sprite_device::get_sprite_attributes(struct tempsprite_t *s, u16 attr)
{
	s->flipy    = BIT(attr, 0);
	s->flipx    = BIT(attr, 1);
	s->color    = (attr & 0x00fc) >> 2;
	s->priority = (attr & 0x0300) >> 8;
}


int kaneko16_sprite_device::parse_sprite_type012(int i, struct tempsprite_t *s, u16* spriteram16, int spriteram16_bytes)
{
	const int offs = i * 8 / 2;

	if (offs >= (spriteram16_bytes / 2))  return -1;

	const u16 attr = spriteram16[offs + 0];
	s->code        = spriteram16[offs + 1];
	s->x           = spriteram16[offs + 2];
	s->y           = spriteram16[offs + 3];

	// this differs between each chip type
	get_sprite_attributes(s, attr);

	const u16 xoffs = (attr & 0x1800) >> 11;
	s->yoffs        = m_sprites_regs[0x10 / 2 + xoffs * 2 + 1];
	s->xoffs        = m_sprites_regs[0x10 / 2 + xoffs * 2 + 0];

	if (m_sprite_flipy)
	{
		s->yoffs -= m_sprites_regs[0x2/2];
		s->yoffs -= screen().visible_area().min_y << 6;
	}
	else
	{
		s->yoffs -= m_sprites_regs[0x2/2];
		s->yoffs += screen().visible_area().min_y << 6;
	}

	return  (BIT(attr, 13) ? USE_LATCHED_XY    : 0) |
			(BIT(attr, 14) ? USE_LATCHED_COLOR : 0) |
			(BIT(attr, 15) ? USE_LATCHED_CODE  : 0) ;
}

// custom function to draw a single sprite. needed to keep correct sprites - sprites and sprites - tilemaps priorities


void kaneko16_sprite_device::draw_sprites_custom(const rectangle &clip,gfx_element *gfx,
		u32 code,u32 color,bool flipx,bool flipy,int sx,int sy,
		int priority)
{
	const pen_t pen_base = gfx->granularity() * (color % gfx->colors());
	const u8 *source_base = gfx->get_data(code % gfx->elements());
	int dx, dy;

	int ex = sx+gfx->width();
	int ey = sy+gfx->height();

	int x_index_base;
	int y_index;

	if (flipx)
	{
		x_index_base = gfx->width() - 1;
		dx = -1;
	}
	else
	{
		x_index_base = 0;
		dx = 1;
	}

	if (flipy)
	{
		y_index = gfx->height() - 1;
		dy = -1;
	}
	else
	{
		y_index = 0;
		dy = 1;
	}

	if (sx < clip.min_x)
	{ /* clip left */
		int pixels = clip.min_x - sx;
		sx += pixels;
		x_index_base += pixels * dx;
	}
	if (sy < clip.min_y)
	{ /* clip top */
		int pixels = clip.min_y - sy;
		sy += pixels;
		y_index += pixels * dy;
	}
	/* NS 980211 - fixed incorrect clipping */
	if (ex > clip.max_x + 1)
	{ /* clip right */
		int pixels = ex-clip.max_x - 1;
		ex -= pixels;
	}
	if (ey > clip.max_y + 1)
	{ /* clip bottom */
		int pixels = ey-clip.max_y - 1;
		ey -= pixels;
	}

	if (ex > sx)
	{ /* skip if inner loop doesn't draw anything */
		for (int y = sy; y < ey; y++)
		{
			u8 const *const source = source_base + y_index * gfx->rowbytes();
			u16 *const dest = &m_sprites_bitmap.pix(y);
			u8 *const pri = &m_sprites_maskmap.pix(y);

			int x_index = x_index_base;
			for (int x = sx; x < ex; x++)
			{
				const u8 c = source[x_index];
				if (c != 0)
				{
					if (pri[x] == 0)
						dest[x] = ((pen_base + c) & 0x3fff) + ((priority & 3) << 14);

					pri[x] = 0xff; // mark it "already drawn"
				}
				x_index += dx;
			}
			y_index += dy;
		}
	}
}


/* Build a list of sprites to display & draw them */
void kaneko16_sprite_device::draw_sprites(const rectangle &cliprect, u16* spriteram16, int spriteram16_bytes)
{
	/* Sprites *must* be parsed from the first in RAM to the last,
	   because of the multisprite feature. But they *must* be drawn
	   from the last in RAM (frontmost) to the first in order to
	   cope with priorities using pdrawgfx.

	   Hence we parse them from first to last and put the result
	   in a temp buffer, then draw the buffer's contents from last
	   to first. */

	const int max =   (screen().width() > 0x100) ? (0x200 << 6) : (0x100 << 6);

	int i = 0;
	struct tempsprite_t *s = m_first_sprite.get();

	/* These values are latched from the last sprite. */
	int x           =   0;
	int y           =   0;
	int code        =   0;
	int color       =   0;
	int priority    =   0;
	int xoffs       =   0;
	int yoffs       =   0;
	bool flipx      =   false;
	bool flipy      =   false;

	while (1)
	{
		int flags = parse_sprite_type012(i,s, spriteram16, spriteram16_bytes);

		if (flags == -1)    // End of Sprites
			break;

		if (flags & USE_LATCHED_CODE)
			s->code = ++code;   // Use the latched code + 1 ..
		else
			code = s->code;     // .. or latch this value

		if (flags & USE_LATCHED_COLOR)
		{
			s->color    = color;
			s->priority = priority;
			s->xoffs    = xoffs;
			s->yoffs    = yoffs;

			if (m_sprite_fliptype == 0)
			{
				s->flipx = flipx;
				s->flipy = flipy;
			}
		}
		else
		{
			color    = s->color;
			priority = s->priority;
			xoffs    = s->xoffs;
			yoffs    = s->yoffs;

			if (m_sprite_fliptype == 0)
			{
				flipx = s->flipx;
				flipy = s->flipy;
			}
		}

		// brap boys explicitly doesn't want the flip to be latched, maybe there is a different bit to enable that behavior?
		if (m_sprite_fliptype == 1)
		{
			flipx = s->flipx;
			flipy = s->flipy;
		}

		if (flags & USE_LATCHED_XY)
		{
			s->x += x;
			s->y += y;
		}
		// Always latch the latest result
		x = s->x;
		y = s->y;

		/* We can now buffer this sprite */

		s->x  = s->xoffs + s->x;
		s->y  = s->yoffs + s->y;

		s->x += m_sprite_xoffs;
		s->y += m_sprite_yoffs;

		if (m_sprite_flipx) { s->x = max - s->x - (16 << 6); s->flipx = !s->flipx; }
		if (m_sprite_flipy) { s->y = max - s->y - (16 << 6); s->flipy = !s->flipy; }

		s->x  = ((s->x & 0x7fc0) - (s->x & 0x8000)) / 0x40;
		s->y  = ((s->y & 0x7fc0) - (s->y & 0x8000)) / 0x40;

		i++;
		s++;
	}


	/* Let's finally draw the sprites we buffered, in reverse order
	   (for pdrawgfx) */

	for (s--; s >= m_first_sprite.get(); s--)
	{
		draw_sprites_custom(
										cliprect,gfx(0),
										s->code,
										s->color,
										s->flipx, s->flipy,
										s->x, s->y,
										s->priority);
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

u16 kaneko16_sprite_device::regs_r(offs_t offset)
{
	return m_sprites_regs[offset];
}

void kaneko16_sprite_device::regs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_sprites_regs[offset]);
	const u16 new_data  = m_sprites_regs[offset];

	switch (offset)
	{
		case 0:
			if (ACCESSING_BITS_0_7)
			{
				m_sprite_flipx = BIT(new_data, 1);
				m_sprite_flipy = BIT(new_data, 0);

				if (get_sprite_type() == 0)
					m_keep_sprites = BIT(~new_data, 2);
			}

			break;
	}

//  logerror("%s : Warning, sprites reg %04X <- %04X\n",m_maincpu->pc(),offset*2,data);
}


void kaneko16_sprite_device::copybitmap(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap) { copybitmap_common(bitmap, cliprect, priority_bitmap); }
void kaneko16_sprite_device::copybitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap) { copybitmap_common(bitmap, cliprect, priority_bitmap); }

template<class BitmapClass>
void kaneko16_sprite_device::copybitmap_common(BitmapClass &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap)
{
	pen_t const *const pal = gfx(0)->palette().pens();

	constexpr bool rgb = sizeof(typename BitmapClass::pixel_t) != 2;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		typename BitmapClass::pixel_t *const dstbitmap = &bitmap.pix(y);
		u8 *const dstprimap = &priority_bitmap.pix(y);
		u16 *const srcbitmap = &m_sprites_bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const u16 pri = (srcbitmap[x] & 0xc000) >> 14;
			const u16 pix = srcbitmap[x] & 0x3fff;
			if (m_priority.sprite[pri] > dstprimap[x])
			{
				if (pix & 0x3fff)
				{
					if (!rgb) dstbitmap[x] = m_colbase + pix;
					else dstbitmap[x] = pal[m_colbase + pix];
				}
			}
		}
	}
}


void kaneko16_sprite_device::render_sprites(const rectangle &cliprect, u16* spriteram16, int spriteram16_bytes)
{
	/* Sprites last (rendered with pdrawgfx, so they can slip
	   in between the layers) */

	m_sprites_maskmap.fill(0, cliprect);
	/* keep sprites on screen - used by mgcrystl when you get the first gem and it shows instructions */
	if (!m_keep_sprites)
		m_sprites_bitmap.fill(0, cliprect);

	draw_sprites(cliprect, spriteram16, spriteram16_bytes);
}

kaneko_vu002_sprite_device::kaneko_vu002_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: kaneko16_sprite_device(mconfig, KANEKO_VU002_SPRITE, tag, owner, clock)
{
}

kaneko_kc002_sprite_device::kaneko_kc002_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: kaneko16_sprite_device(mconfig, KANEKO_KC002_SPRITE, tag, owner, clock)
{
}

// this is a bootleg implementation, used by Gals Hustler and Zip Zap, the latter not really working at all well with the original
// link features (assuming the bad program roms aren't the cause)  it's clearly derived from this sprite system tho.
void kaneko16_sprite_device::bootleg_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, u16* spriteram16, int spriteram16_bytes)
{
//  u16 *spriteram16 = m_spriteram;
	int sx = 0, sy = 0;

	for (int offs = 0; offs < spriteram16_bytes / 2; offs += 4)
	{
		const u32 code   =  spriteram16[offs + 1] & 0x1fff;
		const u32 color  = (spriteram16[offs] & 0x003c) >> 2;
		const bool flipx =  BIT(spriteram16[offs], 1);
		const bool flipy =  BIT(spriteram16[offs], 0);

		if ((spriteram16[offs] & 0x6000) == 0x6000) /* Link bits */
		{
			sx += spriteram16[offs + 2] >> 6;
			sy += spriteram16[offs + 3] >> 6;
		}
		else
		{
			sx = spriteram16[offs + 2] >> 6;
			sy = spriteram16[offs + 3] >> 6;
		}

		sx = (sx & 0x1ff) - (sx & 0x200);
		sy = (sy & 0x1ff) - (sy & 0x200);

		gfx(0)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,0);
	}
}
