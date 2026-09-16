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

   dataeast/backfire.cpp
   dataeast/boogwing.cpp
   dataeast/cbuster.cpp (could probably use pdrawgfx, not m_sprite_bitmap)
   dataeast/cninja.cpp
   dataeast/darkseal.cpp
   dataeast/dassault.cpp
   dataeast/dblewing.cpp
   dataeast/deco32.cpp
   dataeast/deco156.cpp
   dataeast/dietgo.cpp
   dataeast/funkyjet.cpp
   dataeast/lemmings.cpp
   dataeast/mirage.cpp (could probably use pdrawgfx, not m_sprite_bitmap)
   dataeast/pktgaldx.cpp
   dataeast/rohga.cpp
   dataeast/simpl156.cpp
   dataeast/sshangha.cpp (could probably use pdrawgfx, not m_sprite_bitmap)
   dataeast/supbtime.cpp

   (bootleg) dataeast/tumbleb.cpp
   (bootleg) f32/crospang.cpp
   (bootleg) f32/silvmil.cpp
   (bootleg) misc/esd16.cpp
   (bootleg) misc/gotcha.cpp
   (bootleg) yunsung/nmg5.cpp

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
p = priority
c = colour palette

offs +3
-------- --------
tttttttt tttttttt

t = sprite tile

TODO: the priority callback for using pdrawgfx should really pack those 8 bits, and pass them instead of currently just
passing offs+2 which lacks the extra priority bit

*/

#include "emu.h"
#include "decospr.h"
#include "screen.h"

DECOSPR_COLOUR_CB_MEMBER(decospr_device::default_col_cb)
{
	return (col >> 9) & 0x1f;
}

DEFINE_DEVICE_TYPE(DECO_SPRITE, decospr_device, "decospr", "DECO 52 Sprite")

decospr_device::decospr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DECO_SPRITE, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_gfx_interface(mconfig, *this)
	, m_pri_cb(*this)
	, m_col_cb(*this, DEVICE_SELF, FUNC(decospr_device::default_col_cb)) // default color callback
	, m_alt_format(false)
	, m_pixmask(0xf)
	, m_raw_shift(4) // set to 8 on tattass / nslashers for the custom mixing (because they have 5bpp sprites, and shifting by 4 isn't good enough)
	, m_flip_screen(false)
	, m_is_bootleg(false)
	, m_bootleg_type(0)
	, m_x_offset(0)
	, m_y_offset(0)
	, m_flipallx(0)
	, m_transpen(0)
{
}

void decospr_device::device_start()
{
	m_pri_cb.resolve();
	m_col_cb.resolve();

	save_item(NAME(m_flip_screen));
}

void decospr_device::device_reset()
{
	//logerror("decospr_device::device_reset()\n");
}

void decospr_device::alloc_sprite_bitmap()
{
	screen().register_screen_bitmap(m_sprite_bitmap);
}

template<class BitmapClass>
void decospr_device::draw_sprites_common(BitmapClass &bitmap, const rectangle &cliprect, uint16_t* spriteram, int sizewords)
{
	//logerror("cliprect %04x, %04x\n", cliprect.top(), cliprect.bottom());

	if (m_sprite_bitmap.valid() && !m_pri_cb.isnull())
		fatalerror("m_sprite_bitmap && m_pri_cb is invalid\n");

	if (m_sprite_bitmap.valid())
		m_sprite_bitmap.fill(0, cliprect);

	int offs, end, incr;

	bool const flipscreen = m_flip_screen;

	if (!m_pri_cb.isnull())
	{
		offs = sizewords - 4;
		end = -4;
		incr = -4;
	}
	else
	{
		offs = 0;
		end = sizewords;
		incr = 4;
	}

	while (offs != end)
	{
		int x, y, sprite, colour, multi, mult2, fx, fy, inc, flash, mult, h, w, pri;

		if (!m_alt_format)
		{
			sprite = spriteram[offs + 1];
			y = spriteram[offs];

			if (m_is_bootleg && (m_bootleg_type == 1))
				flash = BIT(y, 10);
			else
				flash = BIT(y, 12);

			w = BIT(y, 11);

			if (!(flash && (screen().frame_number() & 1)))
			{
				x = spriteram[offs + 2];

				if (!m_sprite_bitmap.valid())
				{
					colour = m_col_cb(x, BIT(y, 15));
				}
				else
				{
					colour = (x >> 9) & 0x7f;
					if (BIT(y, 15)) colour |= 0x80; // fghthist uses this to mark priority
				}

				if (!m_pri_cb.isnull())
					pri = m_pri_cb(x, BIT(y, 15));
				else
					pri = 0;

				fx = BIT(y, 13);
				fy = BIT(y, 14);

				int tempwidth = 0;

				if (m_is_bootleg && (m_bootleg_type == 1))  // puzzlove
					tempwidth |= bitswap<2>(y, 9, 12);
				else
					tempwidth |= bitswap<2>(y, 10, 9);

				multi = (1 << (tempwidth)) - 1; // 1x, 2x, 4x, 8x height

				// bootleg support (misc/esd16.cpp)
				if (flipscreen)
					x = ((x & 0x1ff) - m_x_offset) & 0x1ff;
				else
					x = ((x & 0x1ff) + m_x_offset) & 0x1ff;
				y = ((y & 0x1ff) + m_y_offset) & 0x1ff;

				if (cliprect.right() > 256)
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

					if (flipscreen ^ m_flipallx)
					{
						if (cliprect.right() > 256)
							x = 304 - x;
						else
							x = 240 - x;

						fx = fx ? 0 : 1;
					}

					mult2 = multi + 1;

					while (multi >= 0)
					{
						const int ypos = y + mult * multi;
						if ((ypos <= cliprect.bottom()) && (ypos >= (cliprect.top()) - 16))
						{
							if (!m_sprite_bitmap.valid())
							{
								if ((ypos <= cliprect.bottom()) && (ypos >= (cliprect.top()) - 16))
								{
									if (!m_pri_cb.isnull())
										gfx(0)->prio_transpen(bitmap, cliprect,
											sprite - multi * inc,
											colour,
											fx, fy,
											x, ypos,
											screen().priority(), pri, m_transpen);
									else
										gfx(0)->transpen(bitmap, cliprect,
											sprite - multi * inc,
											colour,
											fx, fy,
											x, ypos,
											m_transpen);
								}

								// double wing uses this flag
								if (w)
								{
									if (!m_pri_cb.isnull())
										gfx(0)->prio_transpen(bitmap, cliprect,
												(sprite - multi * inc) - mult2,
												colour,
												fx, fy,
												!flipscreen ? (x - 16) : (x + 16), ypos,
												screen().priority(), pri, m_transpen);
									else
										gfx(0)->transpen(bitmap, cliprect,
												(sprite - multi * inc) - mult2,
												colour,
												fx, fy,
												!flipscreen ? (x - 16) : (x + 16), ypos,
												m_transpen);
								}
							}
							else
							{
								// if we have a sprite bitmap draw raw data to it for manual mixing
								gfx(0)->transpen_raw(m_sprite_bitmap, cliprect,
									sprite - multi * inc,
									colour << m_raw_shift,
									fx, fy,
									x, ypos,
									m_transpen);
								if (w)
								{
									gfx(0)->transpen_raw(m_sprite_bitmap, cliprect,
										(sprite - multi * inc) - mult2,
										colour << m_raw_shift,
										fx, fy,
										!flipscreen ? (x - 16) : (x + 16), ypos,
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
			y = spriteram[offs + 0];
			sprite = spriteram[offs + 3] & 0xffff;

			if (!m_pri_cb.isnull())
				pri = m_pri_cb(spriteram[offs + 2] & 0x00ff, false);
			else
				pri = 0;

			x = spriteram[offs + 1];

			if (!(BIT(y, 13) && (screen().frame_number() & 1)))
			{
				if (!m_sprite_bitmap.valid())
					colour = (spriteram[offs + 2] >> 0) & 0x1f;
				else
					colour = (spriteram[offs + 2] >> 0) & 0xff; // store all bits for manual mixing

				h = (spriteram[offs + 2] & 0xf000) >> 12;
				w = (spriteram[offs + 2] & 0x0f00) >>  8;
				fx = BIT(~spriteram[offs + 0], 14);
				fy = BIT(~spriteram[offs + 0], 15);

				if (!flipscreen)
				{
					x = x & 0x01ff;
					y = y & 0x01ff;
					if (x > 0x180) x -= 512;
					if (y > 0x180) y -= 512;

					if (fx) { mult = -16; x += 16 * w; } else { mult = 16; x -= 16; }
					if (fy) { mult2 = -16; y += 16 * h; } else { mult2 = 16; y -= 16; }
				}
				else
				{
					fx = fx ? 0 : 1;
					fy = fy ? 0 : 1;

					x = 304 - util::sext(x, 9);
					y = 240 - util::sext(y, 9);
					if (x >= 432) x -= 512;
					if (y >= 384) y -= 512;
					if (fx) { mult = -16; x += 16; } else { mult = 16; x -= 16 * w; }
					if (fy) { mult2 = -16; y += 16; } else { mult2 = 16; y -= 16 * h; }
				}

				for (int xx = 0; xx < w; xx++)
				{
					for (int yy = 0; yy < h; yy++)
					{
						if (!m_sprite_bitmap.valid())
						{
							if (!m_pri_cb.isnull())
							{
								int ypos = y + mult2 * (h - yy);

								if ((ypos <= cliprect.bottom()) && (ypos >= (cliprect.top()) - 16))
								{
									gfx(0)->prio_transpen(bitmap, cliprect,
											sprite + yy + h * xx,
											colour,
											fx, fy,
											x + mult * (w - xx), ypos,
											screen().priority(), pri, m_transpen);
								}

								ypos -= 512; // wrap-around y

								if ((ypos <= cliprect.bottom()) && (ypos >= (cliprect.top() - 16)))
								{
									gfx(0)->prio_transpen(bitmap, cliprect,
											sprite + yy + h * xx,
											colour,
											fx, fy,
											x + mult * (w - xx), ypos,
											screen().priority(), pri, m_transpen);
								}

							}
							else
							{
								int ypos = y + mult2 * (h - yy);

								if ((ypos <= cliprect.bottom()) && (ypos >= (cliprect.top()) - 16))
								{
									gfx(0)->transpen(bitmap, cliprect,
											sprite + yy + h * xx,
											colour,
											fx, fy,
											x + mult * (w - xx), ypos,
											m_transpen);
								}

								ypos -= 512; // wrap-around y

								if ((ypos <= cliprect.bottom()) && (ypos >= (cliprect.top() - 16)))
								{
									gfx(0)->transpen(bitmap, cliprect,
											sprite + yy + h * xx,
											colour,
											fx, fy,
											x + mult * (w - xx), ypos,
											m_transpen);
								}
							}
						}
						else
						{
							int ypos = y + mult2 * (h - yy);

							if ((ypos <= cliprect.bottom()) && (ypos >= (cliprect.top()) - 16))
							{
								gfx(0)->transpen_raw(m_sprite_bitmap,cliprect,
										sprite + yy + h * xx,
										colour << m_raw_shift,
										fx, fy,
										x + mult * (w - xx), ypos,
										m_transpen);
							}

							ypos -= 512; // wrap-around y

							if ((ypos <= cliprect.bottom()) && (ypos >= (cliprect.top() - 16)))
							{
								gfx(0)->transpen_raw(m_sprite_bitmap,cliprect,
										sprite + yy + h * xx,
										colour << m_raw_shift,
										fx, fy,
										x + mult * (w - xx), ypos,
										m_transpen);
							}
						}
					}
				}
			}
		}

		offs += incr;
	}
}

void decospr_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t* spriteram, int sizewords)
{ draw_sprites_common(bitmap, cliprect, spriteram, sizewords); }

void decospr_device::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t* spriteram, int sizewords)
{ draw_sprites_common(bitmap, cliprect, spriteram, sizewords); }


// inefficient, we should be able to mix in a single pass by comparing the existing priority bitmap from the tilemaps
void decospr_device::inefficient_copy_sprite_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t pri, uint16_t priority_mask, uint16_t colbase, uint16_t palmask, uint8_t alpha)
{
	if (!m_sprite_bitmap.valid())
		fatalerror("decospr_device::inefficient_copy_sprite_bitmap with no m_sprite_bitmap\n");

	pen_t const *const paldata = palette().pens();

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint16_t const *const srcline = &m_sprite_bitmap.pix(y);
		uint32_t *const dstline = &bitmap.pix(y);

		if (alpha == 0xff)
		{
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
			{
				uint16_t const pix = srcline[x];

				if (pix & 0xf)
				{
					if ((pix & priority_mask) == pri)
						dstline[x] = paldata[(pix & palmask) + colbase];
				}
			}
		}
		else
		{
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
			{
				uint16_t const pix = srcline[x];

				if (pix & m_pixmask)
				{
					if ((pix & priority_mask) == pri)
					{
						uint32_t const pal1 = paldata[(pix & palmask) + colbase];
						uint32_t const pal2 = dstline[x];
						dstline[x] = alpha_blend_r32(pal2, pal1, alpha);
					}
				}
			}
		}
	}
}
