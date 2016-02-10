// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,David Haywood

/* the sprites used by DragonGun + Lock 'n' Loaded */

/* probably 186 + 187 custom chips, although there is a 145 too? */

// helicopter in lockload is only half drawn? (3rd attract demo)
// how are we meant to mix these with the tilemaps, parts must go above / behind parts of tilemap '4/7' in lockload, could be more priority masking sprites we're missing / not drawing tho?
// dragongun also does a masking trick on the dragon during the attract intro, it should not be visible but rather cause the fire to be invisible in the shape of the dragon (see note / hack in code to enable this effect)
// dragongun 'waterfall' prior to one of the bosses also needs correct priority

#include "emu.h"
#include "deco_zoomspr.h"

const device_type DECO_ZOOMSPR = &device_creator<deco_zoomspr_device>;

deco_zoomspr_device::deco_zoomspr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DECO_ZOOMSPR, "DECO Zooming Sprites", tag, owner, clock, "deco_zoomspr", __FILE__),
	m_palette(*this),
	m_gfxdecode(*this)
{
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void deco_zoomspr_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<deco_zoomspr_device &>(device).m_palette.set_tag(tag);
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void deco_zoomspr_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<deco_zoomspr_device &>(device).m_gfxdecode.set_tag(tag);
}


void deco_zoomspr_device::device_start()
{
}

void deco_zoomspr_device::device_reset()
{
}

/******************************************************************************/


inline void deco_zoomspr_device::dragngun_drawgfxzoom(
		bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		int transparent_color,
		int scalex, int scaley,bitmap_ind8 *pri_buffer,UINT32 pri_mask, int sprite_screen_width, int  sprite_screen_height, UINT8 alpha,
		bitmap_ind8 &pri_bitmap, bitmap_rgb32 &temp_bitmap,
		int priority)
{
	rectangle myclip;

	if (!scalex || !scaley) return;

	/*
	scalex and scaley are 16.16 fixed point numbers
	1<<15 : shrink to 50%
	1<<16 : uniform scale
	1<<17 : double to 200%
	*/

	/* KW 991012 -- Added code to force clip to bitmap boundary */
	myclip = clip;
	myclip &= temp_bitmap.cliprect();

	{
		if( gfx )
		{
			const pen_t *pal = &m_palette->pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
			const UINT8 *code_base = gfx->get_data(code % gfx->elements());

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

				if( ex>sx )
				{ /* skip if inner loop doesn't draw anything */
					int y;

					/* case 1: no alpha */
					if (alpha == 0xff)
					{
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = code_base + (y_index>>16) * gfx->rowbytes();
								UINT32 *dest = &temp_bitmap.pix32(y);
								UINT8 *pri = &pri_bitmap.pix8(y);


								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if (c != transparent_color)
									{
										if (priority >= pri[x])
										{
											dest[x] = pal[c];
											dest[x] |= 0xff000000;
										}
										else // sprites can have a 'masking' effect on other sprites
										{
											dest[x] = 0x00000000;
										}
									}

									x_index += dx;
								}

								y_index += dy;
							}
						}
					}

					/* alpha-blended */
					else
					{
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = code_base + (y_index>>16) * gfx->rowbytes();
								UINT32 *dest = &temp_bitmap.pix32(y);
								UINT8 *pri = &pri_bitmap.pix8(y);
								UINT32 *tmapcolor = &dest_bmp.pix32(y);


								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if (c != transparent_color)
									{
										if (priority >= pri[x])
										{
											// this logic doesn't seem correct.  Sprites CAN blend other sprites (needed in many places) but based on videos of the character select screen it appears that sprites can't blend already blended sprites
											// I'm not sure which colour gets used but the video shows a single shade of yellow rather than the yellow blending the yellow)

											if ((dest[x] & 0xff000000) == 0x00000000)
												dest[x] = alpha_blend_r32(tmapcolor[x] & 0x00ffffff, pal[c] & 0x00ffffff, alpha); // if nothing has been drawn pull the pixel from the tilemap to blend with
											else
												dest[x] = alpha_blend_r32(dest[x] & 0x00ffffff, pal[c] & 0x00ffffff, alpha); // otherwise blend with what was previously drawn

											dest[x] |= 0xff000000;
										}
										else // sprites can have a 'masking' effect on other sprites
										{
											dest[x] = 0x00000000;
										}
									}

									x_index += dx;
								}

								y_index += dy;
							}
						}
					}
				}
			}
		}
	}
}

void deco_zoomspr_device::dragngun_draw_sprites( bitmap_rgb32 &bitmap, const rectangle &cliprect, const UINT32 *spritedata, UINT32* dragngun_sprite_layout_0_ram, UINT32* dragngun_sprite_layout_1_ram, UINT32* dragngun_sprite_lookup_0_ram, UINT32* dragngun_sprite_lookup_1_ram, UINT32 dragngun_sprite_ctrl,  bitmap_ind8 &pri_bitmap, bitmap_rgb32 &temp_bitmap)
{
	const UINT32 *layout_ram;
	const UINT32 *lookup_ram;
	int offs;
	temp_bitmap.fill(0x00000000, cliprect);

	/*
	    Sprites are built from main control ram, which references tile
	    layout ram, which finally references tile lookup ram which holds
	    the actual tile indices to draw and index into the banking
	    control.  Tile lookup and tile layout ram are double buffered.


	    Main sprite control ram, 8 * 32 bit words per sprite, so

	    Word 0:
	        0x0400 - Banking control for tile layout RAM + tile lookup ram
	        0x0200 - ?
	        0x01ff - Index into tile layout RAM
	    Word 1 :
	    Word 2 : X base position
	    Word 3 : Y base position
	    Word 4 :
	        0x8000: X flip
	        0x03ff: X size of block in pixels (for scaling)
	    Word 5 :
	        0x8000: Y flip
	        0x03ff: Y size of block in pixels (for scaling)
	    Word 6 :
	        0x1f - colour.
	        0x20 - ?  Used for background at 'frog' boss and title screen dragon.
	        0x40 - ?  priority?
	        0x80 - Alpha blending enable
	    Word 7 :


	    Tile layout ram, 4 * 32 bit words per sprite, so

	    Word 0:
	        0x2000 - Selector for tile lookup bank!?!?!?!?!?!?
	        0x1fff - Index into tile lookup ram (16 bit word based, NOT 32)
	    Word 1:
	        0xff00 - ?
	        0x00f0 - Width
	        0x000f - Height
	    Word 2:
	        0x01ff - X block offset
	    Word 3:
	        0x01ff - Y block offset
	*/

	/* Sprite global disable bit - can't be, it's set in lockload calibration menu where the targets are sprites */
//  if (dragngun_sprite_ctrl&0x40000000)
//      return;

	for (offs = 0;offs < 0x800;offs += 8)
	{
		int sx,sy,colour,fx,fy,w,h,x,y,bx,by,alpha,scalex,scaley;
		int zoomx,zoomy;
		int xpos,ypos;

		scalex=spritedata[offs+4]&0x3ff;
		scaley=spritedata[offs+5]&0x3ff;
		if (!scalex || !scaley) /* Zero pixel size in X or Y - skip block */
			continue;

		int layoutram_offset = (spritedata[offs + 0] & 0x1ff) * 4;

		if (spritedata[offs + 0] & 0x400)
			layout_ram = dragngun_sprite_layout_1_ram;
		else
			layout_ram = dragngun_sprite_layout_0_ram;
		h = (layout_ram[layoutram_offset + 1]>>0)&0xf;
		w = (layout_ram[layoutram_offset + 1]>>4)&0xf;
		if (!h || !w)
			continue;

		sx = spritedata[offs+2] & 0x3ff;
		sy = spritedata[offs+3] & 0x3ff;
		bx = layout_ram[layoutram_offset + 2] & 0x1ff;
		by = layout_ram[layoutram_offset + 3] & 0x1ff;
		if (bx&0x100) bx=1-(bx&0xff);
		if (by&0x100) by=1-(by&0xff); /* '1 - ' is strange, but correct for Dragongun 'Winners' screen. */
		if (sx >= 512) sx -= 1024;
		if (sy >= 512) sy -= 1024;

		colour = spritedata[offs+6]&0x1f;

		int priority = (spritedata[offs + 6] & 0x60) >> 5;




//      printf("%02x\n", priority);

		if (priority == 0) priority = 7;
		else if (priority == 1) priority = 7; // set to 1 to have the 'masking effect' with the dragon on the dragngun attract mode, but that breaks the player select where it needs to be 3, probably missing some bits..
		else if (priority == 2) priority = 7;
		else if (priority == 3) priority = 7;

		if (spritedata[offs+6]&0x80)
			alpha=0x80;
		else
			alpha=0xff;

		fx = spritedata[offs+4]&0x8000;
		fy = spritedata[offs+5]&0x8000;

		int lookupram_offset = layout_ram[layoutram_offset + 0] & 0x1fff;

//      if (spritedata[offs+0]&0x400)
		if (layout_ram[layoutram_offset + 0] & 0x2000)
			lookup_ram = dragngun_sprite_lookup_1_ram;
		else
			lookup_ram = dragngun_sprite_lookup_0_ram;

		zoomx=scalex * 0x10000 / (w*16);
		zoomy=scaley * 0x10000 / (h*16);

		if (!fy)
			ypos=(sy<<16) - (by*zoomy); /* The block offset scales with zoom, the base position does not */
		else
			ypos=(sy<<16) + (by*zoomy) - (16*zoomy);

		for (y=0; y<h; y++) {
			if (!fx)
				xpos=(sx<<16) - (bx*zoomx); /* The block offset scales with zoom, the base position does not */
			else
				xpos=(sx<<16) + (bx*zoomx) - (16*zoomx);

			for (x=0; x<w; x++) {
				int bank,sprite;

				sprite = lookup_ram[lookupram_offset];
				sprite &= 0x3fff;

				lookupram_offset++;

				/* High bits of the sprite reference into the sprite control bits for banking */
				switch (sprite&0x3000) {
				default:
				case 0x0000: sprite=(sprite&0xfff) | ((dragngun_sprite_ctrl&0x000f)<<12); break;
				case 0x1000: sprite=(sprite&0xfff) | ((dragngun_sprite_ctrl&0x00f0)<< 8); break;
				case 0x2000: sprite=(sprite&0xfff) | ((dragngun_sprite_ctrl&0x0f00)<< 4); break;
				case 0x3000: sprite=(sprite&0xfff) | ((dragngun_sprite_ctrl&0xf000)<< 0); break;
				}

				/* Because of the unusual interleaved rom layout, we have to mangle the bank bits
				even further to suit our gfx decode */
				switch (sprite&0xf000) {
				case 0x0000: sprite=0xc000 | (sprite&0xfff); break;
				case 0x1000: sprite=0xd000 | (sprite&0xfff); break;
				case 0x2000: sprite=0xe000 | (sprite&0xfff); break;
				case 0x3000: sprite=0xf000 | (sprite&0xfff); break;

				case 0xc000: sprite=0x0000 | (sprite&0xfff); break;
				case 0xd000: sprite=0x1000 | (sprite&0xfff); break;
				case 0xe000: sprite=0x2000 | (sprite&0xfff); break;
				case 0xf000: sprite=0x3000 | (sprite&0xfff); break;
				}

				if (sprite&0x8000) bank=4; else bank=3;
				sprite&=0x7fff;

					dragngun_drawgfxzoom(
						bitmap,cliprect,m_gfxdecode->gfx(bank),
						sprite,
						colour,
						fx,fy,
						xpos>>16,ypos>>16,
						15,zoomx,zoomy,nullptr,0,
						((xpos+(zoomx<<4))>>16) - (xpos>>16), ((ypos+(zoomy<<4))>>16) - (ypos>>16), alpha,
						pri_bitmap, temp_bitmap,
						priority
						);


				if (fx)
					xpos-=zoomx<<4;
				else
					xpos+=zoomx<<4;
			}
			if (fy)
				ypos-=zoomy<<4;
			else
				ypos+=zoomy<<4;
		}
	}

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT32 *src = &temp_bitmap.pix32(y);
		UINT32 *dst = &bitmap.pix32(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			UINT32 srcpix = src[x];

			if ((srcpix & 0xff000000) == 0xff000000)
			{
				dst[x] = srcpix & 0x00ffffff;
			}
		}

	}

}
