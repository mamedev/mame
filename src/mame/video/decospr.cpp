// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
/* Data East Sprite Chip
   DECO 52

   This is a flexible implementation of the Data East Spite chip 52 emulation, used in a large number of drivers.

   Both list formats are supported (how is this selected on HW, external pin?)

   Support for rendering to a raw indexed 16-bitmap is available allowing you to do complex mixing, which is required
   if a game uses alpha blending at the mixing stage.

   Basic callbacks priority and colour are supported for use with pdrawgfx if render-to-bitmap mode isn't being used.

   There is also a very simply 'drawgfx' path for games where the sprites are only of one priority level.

   Several features are included to support the various clone / bootleg chips derived from this device, it appears
   to have been a popular base for Korean developers (much as the Tumble Pop code was)

   used by:

   dblewing.c
   tumblep.c
   dietgo.c
   supbtime.c
   simpl156.c
   deco156.c
   pktgaldx.c
   backfire.c
   darkseal.c
   sshangha.c (could probably use pdrawgfx, not m_sprite_bitmap)
   cbuster.c (could probably use pdrawgfx, not m_sprite_bitmap)
   mirage.c (could probably use pdrawgfx, not m_sprite_bitmap)
   cninja.c
   lemmings.c
   deco32.c
   rohga.c
   dassault.c
   boogwing.c

   (bootleg) esd16.c
   (bootleg) nmg5.c
   (bootleg) tumbleb.c
   (bootleg) crospang.c
   (bootleg) silvmil.c
   (bootleg) gotcha.c

   to convert:

   (any other bootleg / clone chips?)

   notes:
   does the chip natively support 5bpp (tattass / nslasher) in hw, or is it done with doubled up chips?
   the information in deco_tilegen1 lists 3x sprite chips on those games, but there are only 2 spritelists.

   likewise Rohga appears to have 2x DECO52 for 1 list of 6bpp gfx.

*/


/*

STANDARD FORMAT

offs +0
-------- --------
efFbSssy yyyyyyyy

s = size (height)
S = size (width) (double wings)
f = flipy
F = flipx
b = flash/blink
y = ypos
e = extra priority bit (or at least externally detectable by mixer circuits)

offs +1
-------- --------
tttttttt tttttttt

t = sprite tile

offs +2
-------- --------
ppcccccx xxxxxxxx

c = colour palette
p = priority
x = xpos

offs +3
-------- -------- (unused)

in reality all the colour / priority (and extra priority) bits are externally detectable and can be used for
priority and mixing, the sprite palette in twocrude for example isn't in a single block (there is a gap for the tilemaps)
but the mixing circuits take care of this

ALTERNATE FORMAT (used by Mutant Fighter, Captain America)

offs +0
-------- --------
fFbyyyyy yyyyyyyy

f = flipy
F = flipx
b = flash/blink
y = ypos

offs +1
-------- --------
xxxxxxxx xxxxxxxx

x = xpos

offs +2
-------- --------
ssssSSSS pppccccc

s = size (height)
S = size (width)

offs +3
-------- --------
tttttttt tttttttt

t = sprite tile

todo: the priotity callback for using pdrawgfx should really pack those 8 bits, and pass them instead of currently just
passing offs+2 which lacks the extra priority bit

*/

#include "emu.h"
#include "decospr.h"

DECOSPR_COLOUR_CB_MEMBER(decospr_device::default_col_cb)
{
	return (col >> 9) & 0x1f;
}

void decospr_device::set_gfx_region(device_t &device, int gfxregion)
{
	decospr_device &dev = downcast<decospr_device &>(device);
	dev.m_gfxregion = gfxregion;
//  printf("decospr_device::set_gfx_region()\n");
}

const device_type DECO_SPRITE = &device_creator<decospr_device>;

decospr_device::decospr_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DECO_SPRITE, "DECO 52 Sprite", tag, owner, clock, "decospr", __FILE__),
		device_video_interface(mconfig, *this),
		m_gfxregion(0),
		m_is_bootleg(false),
		m_bootleg_type(0),
		m_x_offset(0),
		m_y_offset(0),
		m_flipallx(0),
		m_transpen(0),
		m_gfxdecode(*this),
		m_palette(*this)
{
	// default color callback
	m_col_cb =  decospr_col_cb_delegate(FUNC(decospr_device::default_col_cb), this);
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void decospr_device::static_set_gfxdecode_tag(device_t &device, std::string tag)
{
	downcast<decospr_device &>(device).m_gfxdecode.set_tag(tag);
}

void decospr_device::device_start()
{
	m_pri_cb.bind_relative_to(*owner());
	m_col_cb.bind_relative_to(*owner());

	m_alt_format = 0;
	m_pixmask = 0xf;
	m_raw_shift = 4; // set to 8 on tattass / nslashers for the custom mixing (because they have 5bpp sprites, and shifting by 4 isn't good enough)
}

void decospr_device::device_reset()
{
	//printf("decospr_device::device_reset()\n");
}

void decospr_device::alloc_sprite_bitmap()
{
	m_screen->register_screen_bitmap(m_sprite_bitmap);
}

template<class _BitmapClass>
void decospr_device::draw_sprites_common(_BitmapClass &bitmap, const rectangle &cliprect, UINT16* spriteram, int sizewords, bool invert_flip )
{
	//printf("cliprect %04x, %04x\n", cliprect.min_y, cliprect.max_y);

	if (m_sprite_bitmap.valid() && !m_pri_cb.isnull())
		fatalerror("m_sprite_bitmap && m_pri_cb is invalid\n");

	if (m_sprite_bitmap.valid())
		m_sprite_bitmap.fill(0, cliprect);


	int offs, end, incr;

	int flipscreen = machine().driver_data()->flip_screen();

	if (invert_flip)
		flipscreen = !flipscreen;


	if (!m_pri_cb.isnull())
	{
		offs = sizewords-4;
		end = -4;
		incr = -4;
	}
	else
	{
		offs = 0;
		end = sizewords;
		incr = 4;
	}

	while (offs!=end)
	{
		int x, y, sprite, colour, multi, mult2, fx, fy, inc, flash, mult, h, w, pri;

		if (!m_alt_format)
		{
			sprite = spriteram[offs + 1];
			y = spriteram[offs];

			if (m_is_bootleg && (m_bootleg_type == 1))
			{
				flash = y & 0x0400;
			}
			else
			{
				flash = y & 0x1000;
			}

			w = y & 0x0800;


			if (!(flash && (m_screen->frame_number() & 1)))
			{
				x = spriteram[offs + 2];

				if (!m_sprite_bitmap.valid())
				{
					colour = m_col_cb(x);
				}
				else
				{
					colour = (x >> 9) & 0x7f;
					if (y&0x8000) colour |= 0x80; // fghthist uses this to mark priority
				}


				if (!m_pri_cb.isnull())
					pri = m_pri_cb(x);
				else
					pri = 0;

				fx = y & 0x2000;
				fy = y & 0x4000;

				int tempwidth = 0;

				if (m_is_bootleg && (m_bootleg_type==1))  // puzzlove
				{
					tempwidth = (y & 0x1000) >> 12;
					tempwidth |= (y & 0x0200) >> 8;
				}
				else
				{
					tempwidth |= (y & 0x0600) >> 9;
				}

				multi = (1 << (tempwidth)) - 1; /* 1x, 2x, 4x, 8x height */

				/* bootleg support (esd16.c) */
				if (flipscreen) x = ((x&0x1ff) - m_x_offset)&0x1ff;
				else x = ((x&0x1ff) + m_x_offset)&0x1ff;
				y = ((y&0x1ff) + m_y_offset)&0x1ff;


				if (cliprect.max_x>256)
				{
					x = x & 0x01ff;
					y = y & 0x01ff;
					if (x >= 320) x -= 512;
					if (y >= 256) y -= 512;
					y = 240 - y;
					x = 304 - x;
				}
				else
				{
					x = x & 0x01ff;
					y = y & 0x01ff;
					if (x >= 256) x -= 512;
					if (y >= 256) y -= 512;
					y = 240 - y;
					x = 240 - x;
				}


				//if (x <= 320)
				{
					if (!m_is_bootleg) // several of the clone / bootleg chips don't do this, see jumpkids
						sprite &= ~multi;

					if (fy)
						inc = -1;
					else
					{
						sprite += multi;
						inc = 1;
					}

					if (flipscreen)
					{
						y = 240 - y;
						if (fy) fy = 0; else fy = 1;
						mult = 16;
					}
					else
						mult = -16;

					if (flipscreen || m_flipallx)
					{
						if (cliprect.max_x>256)
							x = 304 - x;
						else
							x = 240 - x;

						if (fx) fx = 0; else fx = 1;
					}



					mult2 = multi + 1;

					while (multi >= 0)
					{
						int ypos;
						ypos = y + mult * multi;
						if ((ypos<=cliprect.max_y) && (ypos>=(cliprect.min_y)-16))
						{
							if(!m_sprite_bitmap.valid())
							{
								if ((ypos<=cliprect.max_y) && (ypos>=(cliprect.min_y)-16))
								{
									if (!m_pri_cb.isnull())
										m_gfxdecode->gfx(m_gfxregion)->prio_transpen(bitmap,cliprect,
											sprite - multi * inc,
											colour,
											fx,fy,
											x,ypos,
											m_screen->priority(),pri,m_transpen);
									else
										m_gfxdecode->gfx(m_gfxregion)->transpen(bitmap,cliprect,
											sprite - multi * inc,
											colour,
											fx,fy,
											x,ypos,
											m_transpen);
								}

								// double wing uses this flag
								if (w)
								{
									if (!m_pri_cb.isnull())
										m_gfxdecode->gfx(m_gfxregion)->prio_transpen(bitmap,cliprect,
												(sprite - multi * inc)-mult2,
												colour,
												fx,fy,
												x-16,ypos,
												m_screen->priority(),pri,m_transpen);
									else
										m_gfxdecode->gfx(m_gfxregion)->transpen(bitmap,cliprect,
												(sprite - multi * inc)-mult2,
												colour,
												fx,fy,
												x-16,ypos,
												m_transpen);
								}
							}
							else
							{
								// if we have a sprite bitmap draw raw data to it for manual mixing
								m_gfxdecode->gfx(m_gfxregion)->transpen_raw(m_sprite_bitmap,cliprect,
									sprite - multi * inc,
									colour<<m_raw_shift,
									fx,fy,
									x,ypos,
									m_transpen);
								if (w)
								{
									m_gfxdecode->gfx(m_gfxregion)->transpen_raw(m_sprite_bitmap,cliprect,
										(sprite - multi * inc)-mult2,
										colour<<m_raw_shift,
										fx,fy,
										x-16,ypos,
										m_transpen);
								}

							}
						}

						multi--;
					}
				}
			}
		}
		else // m_alt_format
		{
			y = spriteram[offs+0];
			sprite = spriteram[offs+3] & 0xffff;


			if (!m_pri_cb.isnull())
				pri = m_pri_cb(spriteram[offs+2]&0x00ff);
			else
				pri = 0;

			x = spriteram[offs+1];

			if (!((y&0x2000) && (m_screen->frame_number() & 1)))
			{
				if (!m_sprite_bitmap.valid())
					colour = (spriteram[offs+2] >>0) & 0x1f;
				else
					colour = (spriteram[offs+2] >>0) & 0xff; // store all bits for manual mixing


				h = (spriteram[offs+2]&0xf000)>>12;
				w = (spriteram[offs+2]&0x0f00)>> 8;
				fx = !(spriteram[offs+0]&0x4000);
				fy = !(spriteram[offs+0]&0x8000);

				if (!flipscreen) {
					x = x & 0x01ff;
					y = y & 0x01ff;
					if (x>0x180) x=-(0x200 - x);
					if (y>0x180) y=-(0x200 - y);

					if (fx) { mult=-16; x+=16*w; } else { mult=16; x-=16; }
					if (fy) { mult2=-16; y+=16*h; } else { mult2=16; y-=16; }
				} else {
					if (fx) fx=0; else fx=1;
					if (fy) fy=0; else fy=1;

					x = x & 0x01ff;
					y = y & 0x01ff;
					if (x&0x100) x=-(0x100 - (x&0xff));
					if (y&0x100) y=-(0x100 - (y&0xff));
					x = 304 - x;
					y = 240 - y;
					if (x >= 432) x -= 512;
					if (y >= 384) y -= 512;
					if (fx) { mult=-16; x+=16; } else { mult=16; x-=16*w; }
					if (fy) { mult2=-16; y+=16; } else { mult2=16; y-=16*h; }
				}
				int ypos;

				for (int xx=0; xx<w; xx++)
				{
					for (int yy=0; yy<h; yy++)
					{
						if(!m_sprite_bitmap.valid())
						{
							if (!m_pri_cb.isnull())
							{
								ypos = y + mult2 * (h-yy);

								if ((ypos<=cliprect.max_y) && (ypos>=(cliprect.min_y)-16))
								{
									m_gfxdecode->gfx(m_gfxregion)->prio_transpen(bitmap,cliprect,
											sprite + yy + h * xx,
											colour,
											fx,fy,
												x + mult * (w-xx),ypos,
											m_screen->priority(),pri,m_transpen);
								}

								ypos -= 512; // wrap-around y

								if ((ypos<=cliprect.max_y) && (ypos>=(cliprect.min_y-16)))
								{
									m_gfxdecode->gfx(m_gfxregion)->prio_transpen(bitmap,cliprect,
											sprite + yy + h * xx,
											colour,
											fx,fy,
											x + mult * (w-xx),ypos,
											m_screen->priority(),pri,m_transpen);
								}

							}
							else
							{
								ypos = y + mult2 * (h-yy);

								if ((ypos<=cliprect.max_y) && (ypos>=(cliprect.min_y)-16))
								{
									m_gfxdecode->gfx(m_gfxregion)->transpen(bitmap,cliprect,
											sprite + yy + h * xx,
											colour,
											fx,fy,
											x + mult * (w-xx),ypos,
											m_transpen);
								}

								ypos -= 512; // wrap-around y

								if ((ypos<=cliprect.max_y) && (ypos>=(cliprect.min_y-16)))
								{
									m_gfxdecode->gfx(m_gfxregion)->transpen(bitmap,cliprect,
											sprite + yy + h * xx,
											colour,
											fx,fy,
											x + mult * (w-xx),ypos,
											m_transpen);
								}
							}
						}
						else
						{
							ypos = y + mult2 * (h-yy);

							if ((ypos<=cliprect.max_y) && (ypos>=(cliprect.min_y)-16))
							{
								m_gfxdecode->gfx(m_gfxregion)->transpen_raw(m_sprite_bitmap,cliprect,
										sprite + yy + h * xx,
										colour<<m_raw_shift,
										fx,fy,
										x + mult * (w-xx),ypos,
										m_transpen);
							}

							ypos -= 512; // wrap-around y

							if ((ypos<=cliprect.max_y) && (ypos>=(cliprect.min_y-16)))
							{
								m_gfxdecode->gfx(m_gfxregion)->transpen_raw(m_sprite_bitmap,cliprect,
										sprite + yy + h * xx,
										colour<<m_raw_shift,
										fx,fy,
										x + mult * (w-xx),ypos,
										m_transpen);
							}
						}
					}
				}
			}
		}

		offs+=incr;
	}
}

void decospr_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16* spriteram, int sizewords, bool invert_flip )
{ draw_sprites_common(bitmap, cliprect, spriteram, sizewords, invert_flip); }

void decospr_device::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT16* spriteram, int sizewords, bool invert_flip )
{ draw_sprites_common(bitmap, cliprect, spriteram, sizewords, invert_flip); }


// inefficient, we should be able to mix in a single pass by comparing the existing priority bitmap from the tilemaps
void decospr_device::inefficient_copy_sprite_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT16 pri, UINT16 priority_mask, UINT16 colbase, UINT16 palmask, UINT8 alpha)
{
	if (!m_sprite_bitmap.valid())
		fatalerror("decospr_device::inefficient_copy_sprite_bitmap with no m_sprite_bitmap\n");

	int y, x;
	const pen_t *paldata = m_palette->pens();

	UINT16* srcline;
	UINT32* dstline;

	for (y=cliprect.min_y;y<=cliprect.max_y;y++)
	{
		srcline= &m_sprite_bitmap.pix16(y);
		dstline= &bitmap.pix32(y);

		if (alpha==0xff)
		{
			for (x=cliprect.min_x;x<=cliprect.max_x;x++)
			{
				UINT16 pix = srcline[x];

				if (pix&0xf)
				{
					if ((pix & priority_mask) ==pri )
					{
						dstline[x] = paldata[(pix&palmask) + colbase];
					}
				}
			}
		}
		else
		{
			for (x=cliprect.min_x;x<=cliprect.max_x;x++)
			{
				UINT16 pix = srcline[x];

				if (pix&m_pixmask)
				{
					if ((pix & priority_mask) ==pri )
					{
						UINT32 pal1 = paldata[(pix&palmask) + colbase];
						UINT32 pal2 = dstline[x];
						dstline[x] = alpha_blend_r32(pal2, pal1, alpha);
					}
				}
			}
		}
	}
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void decospr_device::static_set_palette_tag(device_t &device, std::string tag)
{
	downcast<decospr_device &>(device).m_palette.set_tag(tag);
}
