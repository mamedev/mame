/* Data East Sprite Chip
   DECO 52

   note, we have pri callbacks and checks to drop back to plain drawgfx because not all drivers are using pdrawgfx yet, they probably should be...
   some games have different visible areas, but are confirmed as the same sprite chip.

   games with alpha aren't supported here yet, in most cases they need better mixing anyway, probably rendering to screen buffers and manual mixing.
   see m_sprite_bitmap... (note, if using that you must also use BITMAP_FORMAT_RGB32 in the machine config)

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

   partially converted:
   deco32.c - video mixing

   difficult to convert:
   rohga.c - complex video mixing, 6bpp gfx..
   lemmings.c - priority stuff (plus sprites seem somewhat different anyway??)
   dassault.c - complex video mixing
   boogwing.c - complex video mixing

   notes:
   does the chip natively support 5bpp (tattass / nslasher) in hw, or is it done with doubled up chips?
   the information in deco_tilegen1 lists 3x sprite chips on those games, but there are only 2 spritelists.

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
S = size (width) (double wings)

offs +3
-------- --------
tttttttt tttttttt

t = sprite tile

todo: the priotity callback for using pdrawgfx should really pack those 8 bits, and pass them instead of currently just
passing offs+2 which lacks the extra priority bit

todo: basic blend mixing

*/

#include "emu.h"
#include "decospr.h"


void decospr_device::set_gfx_region(device_t &device, int gfxregion)
{
	decospr_device &dev = downcast<decospr_device &>(device);
	dev.m_gfxregion = gfxregion;
//  printf("decospr_device::set_gfx_region()\n");
}

void decospr_device::set_pri_callback(device_t &device, decospr_priority_callback_func callback)
{
	decospr_device &dev = downcast<decospr_device &>(device);
	dev.m_pricallback = callback;
//  printf("decospr_device::set_pri_callback()\n");
}


const device_type DECO_SPRITE = &device_creator<decospr_device>;

decospr_device::decospr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DECO_SPRITE, "decospr_device", tag, owner, clock),
	  m_gfxregion(0),
	  m_pricallback(NULL)
{
}

void decospr_device::device_start()
{
//  sprite_kludge_x = sprite_kludge_y = 0;
//  printf("decospr_device::device_start()\n");
	m_sprite_bitmap = 0;
	m_alt_format = 0;
	m_pixmask = 0xf;
	m_raw_shift = 4; // set to 8 on tattass / nslashers for the custom mixing (because they have 5bpp sprites, and shifting by 4 isn't good enough)
}

void decospr_device::device_reset()
{
	//printf("decospr_device::device_reset()\n");
}

/*
void decospr_device::decospr_sprite_kludge(int x, int y)
{
    sprite_kludge_x = x;
    sprite_kludge_y = y;
}
*/

void decospr_device::alloc_sprite_bitmap(running_machine& machine)
{
	m_sprite_bitmap =  auto_bitmap_alloc(machine, machine.primary_screen->width(), machine.primary_screen->height(), BITMAP_FORMAT_INDEXED16);
}

void decospr_device::set_pri_callback(decospr_priority_callback_func callback)
{
	m_pricallback = callback;
}




void decospr_device::draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT16* spriteram, int sizewords, bool invert_flip )
{
	//printf("cliprect %04x, %04x\n", cliprect->min_y, cliprect->max_y);

	if (m_sprite_bitmap && m_pricallback)
		fatalerror("m_sprite_bitmap && m_pricallback is invalid");

	if (m_sprite_bitmap)
		bitmap_fill(m_sprite_bitmap, cliprect, 0);


	int offs, end, incr;

	int flipscreen = flip_screen_get(machine);

	if (invert_flip)
		flipscreen = !flipscreen;


	if (m_pricallback)
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
			flash = y & 0x1000;
			w = y & 0x0800;


			if (!(flash && (machine.primary_screen->frame_number() & 1)))
			{

				x = spriteram[offs + 2];

				if (!m_sprite_bitmap)
					colour = (x >> 9) & 0x1f;
				else
				{
					colour = (x >> 9) & 0x7f;
					if (y&0x8000) colour |= 0x80; // fghthist uses this to mark priority
				}


				if (m_pricallback)
					pri = m_pricallback(x);
				else
					pri = 0;

				fx = y & 0x2000;
				fy = y & 0x4000;
				multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

				if (cliprect->max_x>256)
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

						if (cliprect->max_x>256)
							x = 304 - x;
						else
							x = 240 - x;

						if (fx) fx = 0; else fx = 1;
						if (fy) fy = 0; else fy = 1;
						mult = 16;
					}
					else
						mult = -16;

					mult2 = multi + 1;

					while (multi >= 0)
					{
						int ypos;
						ypos = y + mult * multi;
						if ((ypos<=cliprect->max_y) && (ypos>=(cliprect->min_y)-16))
						{
							if(!m_sprite_bitmap)
							{
								if ((ypos<=cliprect->max_y) && (ypos>=(cliprect->min_y)-16))
								{
									if (m_pricallback)
										pdrawgfx_transpen(bitmap,cliprect,machine.gfx[m_gfxregion],
											sprite - multi * inc,
											colour,
											fx,fy,
											x,ypos,
											machine.priority_bitmap,pri,0);
									else
										drawgfx_transpen(bitmap,cliprect,machine.gfx[m_gfxregion],
											sprite - multi * inc,
											colour,
											fx,fy,
											x,ypos,
											0);
								}

								// double wing uses this flag
								if (w)
								{
									if (m_pricallback)
										pdrawgfx_transpen(bitmap,cliprect,machine.gfx[m_gfxregion],
												(sprite - multi * inc)-mult2,
												colour,
												fx,fy,
												x-16,ypos,
												machine.priority_bitmap,pri,0);
									else
										drawgfx_transpen(bitmap,cliprect,machine.gfx[m_gfxregion],
												(sprite - multi * inc)-mult2,
												colour,
												fx,fy,
												x-16,ypos,
												0);
								}
							}
							else
							{
								// if we have a sprite bitmap draw raw data to it for manual mixing
								drawgfx_transpen_raw(m_sprite_bitmap,cliprect,machine.gfx[m_gfxregion],
									sprite - multi * inc,
									colour<<m_raw_shift,
									fx,fy,
									x,ypos,
									0);
								if (w)
								{
									drawgfx_transpen_raw(m_sprite_bitmap,cliprect,machine.gfx[m_gfxregion],
										(sprite - multi * inc)-mult2,
										colour<<m_raw_shift,
										fx,fy,
										x-16,ypos,
										0);
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


			if (m_pricallback)
				pri = m_pricallback(spriteram[offs+2]&0x00ff);
			else
				pri = 0;

			x = spriteram[offs+1];

			if (!((y&0x2000) && (machine.primary_screen->frame_number() & 1)))
			{
				if (!m_sprite_bitmap)
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

						if(!m_sprite_bitmap)
						{
							if (m_pricallback)
							{
								ypos = y + mult2 * (h-yy);

								if ((ypos<=cliprect->max_y) && (ypos>=(cliprect->min_y)-16))
								{
									pdrawgfx_transpen(bitmap,cliprect,machine.gfx[m_gfxregion],
											sprite + yy + h * xx,
											colour,
											fx,fy,
											x + mult * (w-xx),ypos,
											machine.priority_bitmap,pri,0);
								}

								ypos -= 512; // wrap-around y

								if ((ypos<=cliprect->max_y) && (ypos>=(cliprect->min_y-16)))
								{
									pdrawgfx_transpen(bitmap,cliprect,machine.gfx[m_gfxregion],
											sprite + yy + h * xx,
											colour,
											fx,fy,
											x + mult * (w-xx),ypos,
											machine.priority_bitmap,pri,0);
								}

							}
							else
							{
								ypos = y + mult2 * (h-yy);

								if ((ypos<=cliprect->max_y) && (ypos>=(cliprect->min_y)-16))
								{
									drawgfx_transpen(bitmap,cliprect,machine.gfx[m_gfxregion],
											sprite + yy + h * xx,
											colour,
											fx,fy,
											x + mult * (w-xx),ypos,
											0);
								}

								ypos -= 512; // wrap-around y

								if ((ypos<=cliprect->max_y) && (ypos>=(cliprect->min_y-16)))
								{
									drawgfx_transpen(bitmap,cliprect,machine.gfx[m_gfxregion],
											sprite + yy + h * xx,
											colour,
											fx,fy,
											x + mult * (w-xx),ypos,
											0);
								}
							}
						}
						else
						{
							ypos = y + mult2 * (h-yy);

							if ((ypos<=cliprect->max_y) && (ypos>=(cliprect->min_y)-16))
							{
								drawgfx_transpen_raw(m_sprite_bitmap,cliprect,machine.gfx[m_gfxregion],
										sprite + yy + h * xx,
										colour<<m_raw_shift,
										fx,fy,
										x + mult * (w-xx),ypos,
										0);
							}

							ypos -= 512; // wrap-around y

							if ((ypos<=cliprect->max_y) && (ypos>=(cliprect->min_y-16)))
							{
								drawgfx_transpen_raw(m_sprite_bitmap,cliprect,machine.gfx[m_gfxregion],
										sprite + yy + h * xx,
										colour<<m_raw_shift,
										fx,fy,
										x + mult * (w-xx),ypos,
										0);
							}
						}
					}
				}
			}
		}

		offs+=incr;
	}
}


// inefficient, we should be able to mix in a single pass by comparing the existing priority bitmap from the tilemaps
void decospr_device::inefficient_copy_sprite_bitmap(running_machine& machine, bitmap_t *bitmap, const rectangle *cliprect, UINT16 pri, UINT16 priority_mask, UINT16 colbase, UINT16 palmask, UINT8 alpha)
{
	if (!m_sprite_bitmap)
		fatalerror("decospr_device::inefficient_copy_sprite_bitmap with no m_sprite_bitmap");

	int y, x;
	const pen_t *paldata = machine.pens;

	UINT16* srcline;
	UINT32* dstline;

	for (y=cliprect->min_y;y<=cliprect->max_y;y++)
	{
		srcline= BITMAP_ADDR16(m_sprite_bitmap, y, 0);
		dstline= BITMAP_ADDR32(bitmap, y, 0);

		if (alpha==0xff)
		{
			for (x=cliprect->min_x;x<=cliprect->max_x;x++)
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
			for (x=cliprect->min_x;x<=cliprect->max_x;x++)
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
