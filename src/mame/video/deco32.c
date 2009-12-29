#include "driver.h"
#include "includes/deco16ic.h"
#include "includes/deco32.h"

UINT32 *deco32_pf1_data,*deco32_pf2_data,*deco32_pf3_data,*deco32_pf4_data;
UINT32 *deco32_pf12_control,*deco32_pf34_control;
UINT32 *deco32_pf1_rowscroll,*deco32_pf2_rowscroll,*deco32_pf3_rowscroll,*deco32_pf4_rowscroll;
UINT32 *dragngun_sprite_layout_0_ram, *dragngun_sprite_layout_1_ram;
UINT32 *dragngun_sprite_lookup_0_ram, *dragngun_sprite_lookup_1_ram;
UINT32 *deco32_ace_ram;

static UINT8 *dirty_palette;
static tilemap_t *pf1_tilemap,*pf1a_tilemap,*pf2_tilemap,*pf3_tilemap,*pf4_tilemap;
static int deco32_pf1_bank,deco32_pf2_bank,deco32_pf3_bank,deco32_pf4_bank;
static int deco32_pf1_flip,deco32_pf2_flip,deco32_pf3_flip,deco32_pf4_flip;
static int deco32_pf2_colourbank,deco32_pf4_colourbank,deco32_pri;

static bitmap_t *sprite0_mix_bitmap, *sprite1_mix_bitmap, *tilemap_alpha_bitmap;

static UINT32 dragngun_sprite_ctrl;
static int deco32_ace_ram_dirty, has_ace_ram;

int deco32_raster_display_position;
UINT16 *deco32_raster_display_list;

/******************************************************************************/

WRITE32_HANDLER( deco32_pf1_data_w )
{
	COMBINE_DATA(&deco32_pf1_data[offset]);
	tilemap_mark_tile_dirty(pf1_tilemap,offset);
	if (pf1a_tilemap && offset<0x400)
		tilemap_mark_tile_dirty(pf1a_tilemap,offset);
}

WRITE32_HANDLER( deco32_pf2_data_w )
{
	COMBINE_DATA(&deco32_pf2_data[offset]);
	tilemap_mark_tile_dirty(pf2_tilemap,offset);
}

WRITE32_HANDLER( deco32_pf3_data_w )
{
	COMBINE_DATA(&deco32_pf3_data[offset]);
	tilemap_mark_tile_dirty(pf3_tilemap,offset);
}

WRITE32_HANDLER( deco32_pf4_data_w )
{
	COMBINE_DATA(&deco32_pf4_data[offset]);
	tilemap_mark_tile_dirty(pf4_tilemap,offset);
}

/******************************************************************************/

WRITE32_HANDLER( deco32_pri_w )
{
	deco32_pri=data;
}

WRITE32_HANDLER( dragngun_sprite_control_w )
{
	dragngun_sprite_ctrl=data;
}

WRITE32_HANDLER( dragngun_spriteram_dma_w )
{
	/* DMA spriteram to private sprite chip area, and clear cpu ram */
	memcpy(space->machine->generic.buffered_spriteram.u32,space->machine->generic.spriteram.u32,space->machine->generic.spriteram_size);
	memset(space->machine->generic.spriteram.u32,0,0x2000);
}

WRITE32_HANDLER( deco32_ace_ram_w )
{
	/* Some notes pieced together from Tattoo Assassins info:

        Bytes 0 to 0x58 - object alpha control?
        Bytes 0x5c to 0x7c - tilemap alpha control

        0 = opaque, 0x10 = 50% transparent, 0x20 = fully transparent

        Byte 0x00: ACEO000P0
                            P8
                            1P0
                            1P8
                            O010C1
                            o010C8
                            ??

        Hardware fade registers:

        Byte 0x80: fadeptred
        Byte 0x84: fadeptgreen
        Byte 0x88: fadeptblue
        Byte 0x8c: fadestred
        Byte 0x90: fadestgreen
        Byte 0x94: fadestblue
        Byte 0x98: fadetype

        The 'ST' value lerps between the 'PT' value and the palette entries.  So, if PT==0,
        then ST ranging from 0 to 255 will cause a fade to black (when ST==255 the palette
        becomes zero).

        'fadetype' - 1100 for multiplicative fade, 1000 for additive
    */
	if (offset>=(0x80/4) && (data!=deco32_ace_ram[offset]))
		deco32_ace_ram_dirty=1;

	COMBINE_DATA(&deco32_ace_ram[offset]);
}

static void updateAceRam(running_machine* machine)
{
	int r,g,b,i;
	UINT8 fadeptr=deco32_ace_ram[0x20];
	UINT8 fadeptg=deco32_ace_ram[0x21];
	UINT8 fadeptb=deco32_ace_ram[0x22];
	UINT8 fadepsr=deco32_ace_ram[0x23];
	UINT8 fadepsg=deco32_ace_ram[0x24];
	UINT8 fadepsb=deco32_ace_ram[0x25];
//  UINT8 mode=deco32_ace_ram[0x26];

	deco32_ace_ram_dirty=0;

	for (i=0; i<2048; i++)
	{
		/* Lerp palette entry to 'fadept' according to 'fadeps' */
		b = (machine->generic.paletteram.u32[i] >>16) & 0xff;
		g = (machine->generic.paletteram.u32[i] >> 8) & 0xff;
		r = (machine->generic.paletteram.u32[i] >> 0) & 0xff;

		if (i>255) /* Screenshots seem to suggest ACE fades do not affect playfield 1 palette (0-255) */
		{
			/* Yeah, this should really be fixed point, I know */
			b = (UINT8)((float)b + (((float)fadeptb - (float)b) * (float)fadepsb/255.0f));
			g = (UINT8)((float)g + (((float)fadeptg - (float)g) * (float)fadepsg/255.0f));
			r = (UINT8)((float)r + (((float)fadeptr - (float)r) * (float)fadepsr/255.0f));
		}

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

/******************************************************************************/

/* Later games have double buffered paletteram - the real palette ram is
only updated on a DMA call */

WRITE32_HANDLER( deco32_nonbuffered_palette_w )
{
	int r,g,b;

	COMBINE_DATA(&space->machine->generic.paletteram.u32[offset]);

	b = (space->machine->generic.paletteram.u32[offset] >>16) & 0xff;
	g = (space->machine->generic.paletteram.u32[offset] >> 8) & 0xff;
	r = (space->machine->generic.paletteram.u32[offset] >> 0) & 0xff;

	palette_set_color(space->machine,offset,MAKE_RGB(r,g,b));
}

WRITE32_HANDLER( deco32_buffered_palette_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u32[offset]);
	dirty_palette[offset]=1;
}

WRITE32_HANDLER( deco32_palette_dma_w )
{
	const int m=space->machine->config->total_colors;
	int r,g,b,i;

	for (i=0; i<m; i++) {
		if (dirty_palette[i]) {
			dirty_palette[i]=0;

			if (has_ace_ram)
			{
				deco32_ace_ram_dirty=1;
			}
			else
			{
				b = (space->machine->generic.paletteram.u32[i] >>16) & 0xff;
				g = (space->machine->generic.paletteram.u32[i] >> 8) & 0xff;
				r = (space->machine->generic.paletteram.u32[i] >> 0) & 0xff;

				palette_set_color(space->machine,i,MAKE_RGB(r,g,b));
			}
		}
	}
}

/******************************************************************************/

static void captaven_draw_sprites(running_machine* machine, bitmap_t *bitmap, const rectangle *cliprect, const UINT32 *spritedata, int gfxbank)
{
	int offs;

	/*
        Word 0:
            0x8000: Y flip
            0x4000: X flip
            0x2000: Flash (Sprite toggles on/off every frame)
            0x1fff: Y value
        Word 1:
            0xffff: X value
        Word 2:
            0xf000: Block height
            0x0f00: Block width
            0x0080: Unused?
            0x0040: Priority
            0x001f: Colour
        Word 3:
            0xffff: Sprite value
    */

	for (offs = 0x400-4;offs >=0;offs -= 4)
	{
		int sx,sy,sprite,colour,fx,fy,x_mult,y_mult,w,h,x,y,prival;

		sy = spritedata[offs+0];
		sprite = spritedata[offs+3] & 0xffff;

		if (sy==0x00000108 && !sprite)
			continue; //fix!!!!!

		if ((spritedata[offs+2]&0x60)==0x00)
		{
			prival = 0; // above everything
		}
		else if ((spritedata[offs+2]&0x60)==0x20)
		{
			prival = 0xfff0; // above the 2nd playfield
		}
		else if ((spritedata[offs+2]&0x60)==0x40)
		{
			prival = 0xfffc; // above the 1st playfield
		}
		else
		{
			// never used?
			prival = 0xfffe; // under everything
		}

		sx = spritedata[offs+1];

		if ((sy&0x2000) && (video_screen_get_frame_number(machine->primary_screen) & 1)) continue;

		colour = (spritedata[offs+2] >>0) & 0x1f;

		h = (spritedata[offs+2]&0xf000)>>12;
		w = (spritedata[offs+2]&0x0f00)>> 8;
		fx = !(spritedata[offs+0]&0x4000);
		fy = !(spritedata[offs+0]&0x8000);

		if (!flip_screen_get(machine)) {
			sx = sx & 0x01ff;
			sy = sy & 0x01ff;
			if (sx>0x180) sx=-(0x200 - sx);
			if (sy>0x180) sy=-(0x200 - sy);

			if (fx) { x_mult=-16; sx+=16*w; } else { x_mult=16; sx-=16; }
			if (fy) { y_mult=-16; sy+=16*h; } else { y_mult=16; sy-=16; }
		} else {
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;

			sx = sx & 0x01ff;
			sy = sy & 0x01ff;
			if (sx&0x100) sx=-(0x100 - (sx&0xff));
			if (sy&0x100) sy=-(0x100 - (sy&0xff));
			sx = 304 - sx;
			sy = 240 - sy;
			if (sx >= 432) sx -= 512;
			if (sy >= 384) sy -= 512;
			if (fx) { x_mult=-16; sx+=16; } else { x_mult=16; sx-=16*w; }
			if (fy) { y_mult=-16; sy+=16; } else { y_mult=16; sy-=16*h; }
		}

		for (x=0; x<w; x++) {
			for (y=0; y<h; y++) {
				pdrawgfx_transpen(bitmap,cliprect,machine->gfx[gfxbank],
						sprite + y + h * x,
						colour,
						fx,fy,
						sx + x_mult * (w-x),sy + y_mult * (h-y),
						machine->priority_bitmap,prival,0);

				// wrap-around y
				pdrawgfx_transpen(bitmap,cliprect,machine->gfx[gfxbank],
						sprite + y + h * x,
						colour,
						fx,fy,
						sx + x_mult * (w-x),sy + y_mult * (h-y) - 512,
						machine->priority_bitmap,prival,0);
			}
		}
	}
}

static void fghthist_draw_sprites(running_machine* machine, bitmap_t *bitmap, const rectangle *cliprect, const UINT32 *spritedata, int gfxbank, int mask, int colourmask)
{
	int offs;

	for (offs = 0x400 - 4; offs >= 0; offs -=4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult,pri=0;
		int alpha = 0xff;

		sprite = spritedata[offs+1] & 0xffff;

		y = spritedata[offs];
		flash=y&0x1000;
		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1)) continue;

		x = spritedata[offs+2];
		colour = (x >>9) & colourmask;

		if ((y&0x8000))
			pri=1;
		else
			pri=4;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		mult=+16;

		if (fx) fx=0; else fx=1;
		if (fy) fy=0; else fy=1;

		while (multi >= 0)
		{
			deco16_pdrawgfx(
					bitmap,cliprect,machine->gfx[gfxbank],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					0,pri,1<<gfxbank, 1, alpha);

			multi--;
		}
	}
}

/*
    This renders sprites to a 16 bit bitmap, for later mixing.
    Bottom 8 bits per pixel is palettised sprite data, top 8 is
    colour/alpha/priority.
*/
static void deco32_draw_sprite(bitmap_t *dest,const rectangle *clip,const gfx_element *gfx,
		UINT32 code,UINT32 priority,int flipx,int flipy,int sx,int sy)
{
	const UINT8 *code_base = gfx_element_get_data(gfx, code % gfx->total_elements);
	int ox,oy,cx,cy;
	int x_index,y_index,x,y;

	/* check bounds */
	ox = sx;
	oy = sy;

	if (sx>319 || sy>247 || sx<-15 || sy<-7)
		return;

	if (sy<0) sy=0;
	if (sx<0) sx=0;
	if (sx>319) cx=319;
	else cx=ox+16;

	cy=(sy-oy);

	if (flipy) y_index=15-cy; else y_index=cy;

	for( y=0; y<16-cy; y++ )
	{
		const UINT8 *source = code_base + y_index * gfx->line_modulo;
		UINT16 *destb = BITMAP_ADDR16(dest, sy, 0);

		if (flipx) { source+=15-(sx-ox); x_index=-1; } else { x_index=1; source+=(sx-ox); }

		for (x=sx; x<cx; x++)
		{
			int c = *source;
			if( c )
				destb[x] = c | priority;

			source+=x_index;
		}

		sy++;
		if (sy>247)
			return;
		if (flipy) y_index--; else y_index++;
	}
}

// Merge with Tattass & Fghthist sprite routines later
static void nslasher_draw_sprites(running_machine* machine, bitmap_t *bitmap, const rectangle *cliprect, const UINT32 *spritedata, int gfxbank)
{
	int offs;

	// Draw sprites back to front saving priority & alpha data per pixel for later mixing
	for (offs = 0; offs<0x400; offs+=4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult; /*,pri=0,spri=0;*/
		//int trans;

		sprite = spritedata[offs+1] & 0xffff;

		y = spritedata[offs];
		flash=y&0x1000;
		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1)) continue;

		//trans=TRANSPARENCY_PEN;
		x = spritedata[offs+2];

		// Prepare colour, priority and alpha info
		colour = (x>>9) & 0x7f;
		if (y&0x8000)
			colour|=0x80;
		colour<<=8;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		mult=+16;

		if (fx) fx=0; else fx=1;
		if (fy) fy=0; else fy=1;

		while (multi >= 0)
		{
			deco32_draw_sprite(bitmap,cliprect,machine->gfx[gfxbank],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi);

			multi--;
		}
	}
}

INLINE void dragngun_drawgfxzoom(
		bitmap_t *dest_bmp,const rectangle *clip,const gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		int transparent_color,
		int scalex, int scaley,bitmap_t *pri_buffer,UINT32 pri_mask, int sprite_screen_width, int  sprite_screen_height, UINT8 alpha )
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
	if(clip)
	{
		myclip.min_x = clip->min_x;
		myclip.max_x = clip->max_x;
		myclip.min_y = clip->min_y;
		myclip.max_y = clip->max_y;

		if (myclip.min_x < 0) myclip.min_x = 0;
		if (myclip.max_x >= dest_bmp->width) myclip.max_x = dest_bmp->width-1;
		if (myclip.min_y < 0) myclip.min_y = 0;
		if (myclip.max_y >= dest_bmp->height) myclip.max_y = dest_bmp->height-1;

		clip=&myclip;
	}

	{
		if( gfx )
		{
			const pen_t *pal = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * (color % gfx->total_colors)];
			const UINT8 *code_base = gfx_element_get_data(gfx, code % gfx->total_elements);

			if (sprite_screen_width && sprite_screen_height)
			{
				/* compute sprite increment per screen pixel */
				int dx = (gfx->width<<16)/sprite_screen_width;
				int dy = (gfx->height<<16)/sprite_screen_height;

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

				if( clip )
				{
					if( sx < clip->min_x)
					{ /* clip left */
						int pixels = clip->min_x-sx;
						sx += pixels;
						x_index_base += pixels*dx;
					}
					if( sy < clip->min_y )
					{ /* clip top */
						int pixels = clip->min_y-sy;
						sy += pixels;
						y_index += pixels*dy;
					}
					/* NS 980211 - fixed incorrect clipping */
					if( ex > clip->max_x+1 )
					{ /* clip right */
						int pixels = ex-clip->max_x-1;
						ex -= pixels;
					}
					if( ey > clip->max_y+1 )
					{ /* clip bottom */
						int pixels = ey-clip->max_y-1;
						ey -= pixels;
					}
				}

				if( ex>sx )
				{ /* skip if inner loop doesn't draw anything */
					int y;

					/* case 1: no alpha */
					if (alpha == 0xff)
					{
						if (pri_buffer)
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = code_base + (y_index>>16) * gfx->line_modulo;
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);
								UINT8 *pri = BITMAP_ADDR8(pri_buffer, y, 0);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c != transparent_color )
									{
										if (((1 << pri[x]) & pri_mask) == 0)
											dest[x] = pal[c];
										pri[x] = 31;
									}
									x_index += dx;
								}

								y_index += dy;
							}
						}
						else
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = code_base + (y_index>>16) * gfx->line_modulo;
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c != transparent_color ) dest[x] = pal[c];
									x_index += dx;
								}

								y_index += dy;
							}
						}
					}

					/* alpha-blended */
					else
					{
						if (pri_buffer)
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = code_base + (y_index>>16) * gfx->line_modulo;
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);
								UINT8 *pri = BITMAP_ADDR8(pri_buffer, y, 0);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c != transparent_color )
									{
										if (((1 << pri[x]) & pri_mask) == 0)
											dest[x] = alpha_blend_r32(dest[x], pal[c], alpha);
										pri[x] = 31;
									}
									x_index += dx;
								}

								y_index += dy;
							}
						}
						else
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = code_base + (y_index>>16) * gfx->line_modulo;
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c != transparent_color ) dest[x] = alpha_blend_r32(dest[x], pal[c], alpha);
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

static void dragngun_draw_sprites(running_machine* machine, bitmap_t *bitmap, const rectangle *cliprect, const UINT32 *spritedata)
{
	const UINT32 *layout_ram;
	const UINT32 *lookup_ram;
	int offs;

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

	/* Sprite global disable bit */
	if (dragngun_sprite_ctrl&0x40000000)
		return;

	for (offs = 0;offs < 0x800;offs += 8)
	{
		int sx,sy,colour,fx,fy,w,h,x,y,bx,by,alpha,scalex,scaley;
		int zoomx,zoomy;
		int xpos,ypos;

		scalex=spritedata[offs+4]&0x3ff;
		scaley=spritedata[offs+5]&0x3ff;
		if (!scalex || !scaley) /* Zero pixel size in X or Y - skip block */
			continue;

		if (spritedata[offs+0]&0x400)
			layout_ram = dragngun_sprite_layout_1_ram + ((spritedata[offs+0]&0x1ff)*4); //CHECK!
		else
			layout_ram = dragngun_sprite_layout_0_ram + ((spritedata[offs+0]&0x1ff)*4); //1ff in drag gun code??
		h = (layout_ram[1]>>0)&0xf;
		w = (layout_ram[1]>>4)&0xf;
		if (!h || !w)
			continue;

		sx = spritedata[offs+2] & 0x3ff;
		sy = spritedata[offs+3] & 0x3ff;
		bx = layout_ram[2] & 0x1ff;
		by = layout_ram[3] & 0x1ff;
		if (bx&0x100) bx=1-(bx&0xff);
		if (by&0x100) by=1-(by&0xff); /* '1 - ' is strange, but correct for Dragongun 'Winners' screen. */
		if (sx >= 512) sx -= 1024;
		if (sy >= 512) sy -= 1024;

		colour = spritedata[offs+6]&0x1f;

		if (spritedata[offs+6]&0x80)
			alpha=0x80;
		else
			alpha=0xff;

		fx = spritedata[offs+4]&0x8000;
		fy = spritedata[offs+5]&0x8000;

//      if (spritedata[offs+0]&0x400)
		if (layout_ram[0]&0x2000)
			lookup_ram = dragngun_sprite_lookup_1_ram + (layout_ram[0]&0x1fff);
		else
			lookup_ram = dragngun_sprite_lookup_0_ram + (layout_ram[0]&0x1fff);

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

				sprite = ((*(lookup_ram++))&0x3fff);

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

				if (zoomx!=0x10000 || zoomy!=0x10000)
					dragngun_drawgfxzoom(
						bitmap,cliprect,machine->gfx[bank],
						sprite,
						colour,
						fx,fy,
						xpos>>16,ypos>>16,
						15,zoomx,zoomy,NULL,0,
						((xpos+(zoomx<<4))>>16) - (xpos>>16), ((ypos+(zoomy<<4))>>16) - (ypos>>16), alpha );
				else
					drawgfx_alpha(bitmap,cliprect,machine->gfx[bank],
						sprite,
						colour,
						fx,fy,
						xpos>>16,ypos>>16,
						15,alpha);

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
}

/******************************************************************************/

static UINT32 deco16_scan_rows(UINT32 col,UINT32 row,UINT32 num_cols,UINT32 num_rows)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5);
}

static TILE_GET_INFO( get_pf1_tile_info )
{
	int tile=deco32_pf1_data[tile_index];
	SET_TILE_INFO(0,(tile&0xfff)|deco32_pf1_bank,(tile>>12)&0xf,0);
}

static TILE_GET_INFO( get_pf1a_tile_info )
{

	int tile=deco32_pf1_data[tile_index];
	SET_TILE_INFO(1,(tile&0xfff)|deco32_pf1_bank,(tile>>12)&0xf,0);
}

static TILE_GET_INFO( get_pf2_tile_info )
{
	UINT32 tile=deco32_pf2_data[tile_index];
	UINT8	colour=(tile>>12)&0xf;
	UINT8 flags=0;

	if (tile&0x8000) {
		if ((deco32_pf12_control[6]>>8)&0x01) {
			flags|=TILE_FLIPX;
			colour&=0x7;
		}
		if ((deco32_pf12_control[6]>>8)&0x02) {
			flags|=TILE_FLIPY;
			colour&=0x7;
		}
	}

	SET_TILE_INFO(1,(tile&0xfff)|deco32_pf2_bank,colour+deco32_pf2_colourbank,flags);
}

static TILE_GET_INFO( get_pf3_tile_info )
{
	UINT32 tile=deco32_pf3_data[tile_index];
	UINT8	colour=(tile>>12)&0xf;
	UINT8 flags=0;

	if (tile&0x8000) {
		if ((deco32_pf34_control[6]>>0)&0x01) {
			flags|=TILE_FLIPX;
			colour&=0x7;
		}
		if ((deco32_pf34_control[6]>>0)&0x02) {
			flags|=TILE_FLIPY;
			colour&=0x7;
		}
	}

	SET_TILE_INFO(2,(tile&0xfff)|deco32_pf3_bank,colour,flags);
}

static TILE_GET_INFO( get_pf4_tile_info )
{
	UINT32 tile=deco32_pf4_data[tile_index];
	UINT8	colour=(tile>>12)&0xf;
	UINT8 flags=0;

	if (tile&0x8000) {
		if ((deco32_pf34_control[6]>>8)&0x01) {
			flags|=TILE_FLIPX;
			colour&=0x7;
		}
		if ((deco32_pf34_control[6]>>8)&0x02) {
			flags|=TILE_FLIPY;
			colour&=0x7;
		}
	}

	SET_TILE_INFO(2,(tile&0xfff)|deco32_pf4_bank,colour+deco32_pf4_colourbank,flags);
}

/* Captain America tilemap chip 2 has different banking and colour from normal */
static TILE_GET_INFO( get_ca_pf3_tile_info )
{
	int tile=deco32_pf3_data[tile_index];
	SET_TILE_INFO(2,(tile&0x3fff)+deco32_pf3_bank,(tile >> 14)&3,0);
}

static TILE_GET_INFO( get_ll_pf3_tile_info )
{
	UINT32 tile=deco32_pf3_data[tile_index];
	UINT8 flags=0;

	if (tile&0x8000) {
		if ((deco32_pf34_control[6]>>0)&0x01)
			flags|=TILE_FLIPX;
		if ((deco32_pf34_control[6]>>0)&0x02)
			flags|=TILE_FLIPY;
	}

	SET_TILE_INFO(2,(tile&0x0fff)|deco32_pf3_bank,(tile >> 12)&3,flags);
}

static TILE_GET_INFO( get_ll_pf4_tile_info )
{
	UINT32 tile=deco32_pf4_data[tile_index];
	UINT8 flags=0;

	if (tile&0x8000) {
		if ((deco32_pf34_control[6]>>8)&0x01)
			flags|=TILE_FLIPX;
		if ((deco32_pf34_control[6]>>8)&0x02)
			flags|=TILE_FLIPY;
	}

	SET_TILE_INFO(2,(tile&0x0fff)|deco32_pf4_bank,(tile >> 12)&3,flags);
}

VIDEO_START( captaven )
{
	pf1_tilemap = tilemap_create(machine, get_pf1_tile_info,    tilemap_scan_rows, 8, 8,64,32);
	pf1a_tilemap =tilemap_create(machine, get_pf1a_tile_info,   deco16_scan_rows,16,16,64,32);
	pf2_tilemap = tilemap_create(machine, get_pf2_tile_info,    deco16_scan_rows,16,16,64,32);
	pf3_tilemap = tilemap_create(machine, get_ca_pf3_tile_info, tilemap_scan_rows,16,16,32,32);

	tilemap_set_transparent_pen(pf1_tilemap,0);
	tilemap_set_transparent_pen(pf1a_tilemap,0);
	tilemap_set_transparent_pen(pf2_tilemap,0);
	tilemap_set_transparent_pen(pf3_tilemap,0);

	deco32_pf2_colourbank=16;
	deco32_pf4_colourbank=0;
	has_ace_ram=0;
}

VIDEO_START( fghthist )
{
	pf1_tilemap = tilemap_create(machine, get_pf1_tile_info, tilemap_scan_rows, 8, 8,64,32);
	pf2_tilemap = tilemap_create(machine, get_pf2_tile_info, deco16_scan_rows, 16,16,64,32);
	pf3_tilemap = tilemap_create(machine, get_pf3_tile_info, deco16_scan_rows, 16,16,64,32);
	pf4_tilemap = tilemap_create(machine, get_pf4_tile_info, deco16_scan_rows, 16,16,64,32);
	pf1a_tilemap =0;
	dirty_palette = auto_alloc_array(machine, UINT8, 4096);

	deco_allocate_sprite_bitmap(machine);

	tilemap_set_transparent_pen(pf1_tilemap,0);
	tilemap_set_transparent_pen(pf2_tilemap,0);
	tilemap_set_transparent_pen(pf3_tilemap,0);

	deco32_pf2_colourbank=deco32_pf4_colourbank=0;
	has_ace_ram=0;
}

VIDEO_START( dragngun )
{
	pf1_tilemap = tilemap_create(machine, get_pf1_tile_info,    tilemap_scan_rows, 8, 8,64,32);
	pf2_tilemap = tilemap_create(machine, get_pf2_tile_info,    deco16_scan_rows,16,16,64,32);
	pf3_tilemap = tilemap_create(machine, get_ll_pf3_tile_info, deco16_scan_rows,16,16,64,32);
	pf4_tilemap = tilemap_create(machine, get_ll_pf4_tile_info, deco16_scan_rows,     16,16,64,32);
	pf1a_tilemap =tilemap_create(machine, get_pf1a_tile_info,   deco16_scan_rows,16,16,64,32);
	dirty_palette = auto_alloc_array(machine, UINT8, 4096);
	deco32_raster_display_list = auto_alloc_array(machine, UINT16, 10 * 256 / 2);

	tilemap_set_transparent_pen(pf1_tilemap,0);
	tilemap_set_transparent_pen(pf2_tilemap,0);
	tilemap_set_transparent_pen(pf3_tilemap,0);
	tilemap_set_transparent_pen(pf1a_tilemap,0);
	memset(dirty_palette,0,4096);

	deco32_pf2_colourbank=deco32_pf4_colourbank=0;

	state_save_register_global(machine, dragngun_sprite_ctrl);
	has_ace_ram=0;
}

VIDEO_START( lockload )
{
	pf1_tilemap = tilemap_create(machine, get_pf1_tile_info,    tilemap_scan_rows, 8, 8,64,32);
	pf2_tilemap = tilemap_create(machine, get_pf2_tile_info,    deco16_scan_rows,16,16,64,32);
	pf3_tilemap = tilemap_create(machine, get_ll_pf3_tile_info, deco16_scan_rows,16,16,32,32);
	pf4_tilemap = tilemap_create(machine, get_ll_pf4_tile_info, deco16_scan_rows,     16,16,32,32);
	pf1a_tilemap =tilemap_create(machine, get_pf1a_tile_info,   deco16_scan_rows,16,16,64,32);
	dirty_palette = auto_alloc_array(machine, UINT8, 4096);
	deco32_raster_display_list = auto_alloc_array(machine, UINT16, 10 * 256 / 2);
	memset(deco32_raster_display_list, 0, 10 * 256);

	tilemap_set_transparent_pen(pf1_tilemap,0);
	tilemap_set_transparent_pen(pf2_tilemap,0);
	tilemap_set_transparent_pen(pf3_tilemap,0);
	tilemap_set_transparent_pen(pf1a_tilemap,0);
	memset(dirty_palette,0,4096);

	deco32_pf2_colourbank=deco32_pf4_colourbank=0;

	state_save_register_global(machine, dragngun_sprite_ctrl);
	has_ace_ram=0;
}

VIDEO_START( nslasher )
{
	int width, height;

	pf1_tilemap = tilemap_create(machine, get_pf1_tile_info, tilemap_scan_rows, 8, 8,64,32);
	pf2_tilemap = tilemap_create(machine, get_pf2_tile_info, deco16_scan_rows,16,16,64,32);
	pf3_tilemap = tilemap_create(machine, get_pf3_tile_info, deco16_scan_rows,16,16,64,32);
	pf4_tilemap = tilemap_create(machine, get_pf4_tile_info, deco16_scan_rows,     16,16,64,32);
	pf1a_tilemap =0;
	dirty_palette = auto_alloc_array(machine, UINT8, 4096);

	width = video_screen_get_width(machine->primary_screen);
	height = video_screen_get_height(machine->primary_screen);
	sprite0_mix_bitmap=auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED16 );
	sprite1_mix_bitmap=auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED16 );
	tilemap_alpha_bitmap=auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED16 );

	tilemap_set_transparent_pen(pf1_tilemap,0);
	tilemap_set_transparent_pen(pf2_tilemap,0);
	tilemap_set_transparent_pen(pf3_tilemap,0);
	memset(dirty_palette,0,4096);

	deco32_pf2_colourbank=16;
	deco32_pf4_colourbank=16;
	state_save_register_global(machine, deco32_pri);
	has_ace_ram=1;
}

/******************************************************************************/

VIDEO_EOF( captaven )
{
	memcpy(machine->generic.buffered_spriteram.u32,machine->generic.spriteram.u32,machine->generic.spriteram_size);
}

VIDEO_EOF( dragngun )
{
	deco32_raster_display_position=0;
}

#if 0
static void print_debug_info(bitmap_t *bitmap)
{
	int j;
	char buf[64*5];
	char *bufptr = buf;

	bufptr += sprintf(bufptr,"%04X %04X %04X %04X\n",deco32_pf12_control[0]&0xffff,deco32_pf12_control[1]&0xffff,deco32_pf12_control[2]&0xffff,deco32_pf12_control[3]&0xffff);
	bufptr += sprintf(bufptr,"%04X %04X %04X %04X\n",deco32_pf12_control[4]&0xffff,deco32_pf12_control[5]&0xffff,deco32_pf12_control[6]&0xffff,deco32_pf12_control[7]&0xffff);

	bufptr += sprintf(bufptr,"%04X %04X %04X %04X\n",deco32_pf34_control[0]&0xffff,deco32_pf34_control[1]&0xffff,deco32_pf34_control[2]&0xffff,deco32_pf34_control[3]&0xffff);
	bufptr += sprintf(bufptr,"%04X %04X %04X %04X\n",deco32_pf34_control[4]&0xffff,deco32_pf34_control[5]&0xffff,deco32_pf34_control[6]&0xffff,deco32_pf34_control[7]&0xffff);
	bufptr += sprintf(bufptr,"%04X\n",deco32_pri);

	ui_draw_text(buf,60,40);
}

#endif

static void deco32_setup_scroll(tilemap_t *pf_tilemap, UINT16 height, UINT8 control0, UINT8 control1, UINT16 sy, UINT16 sx, UINT32 *rowdata, UINT32 *coldata)
{
	int rows,offs;

	/* Colscroll - not fully supported yet! */
	if (control1&0x20 && coldata) {
		sy+=coldata[0];
		//popmessage("%08x",coldata[0]);
	}

	/* Rowscroll enable */
	if (control1&0x40 && rowdata) {
		tilemap_set_scroll_cols(pf_tilemap,1);
		tilemap_set_scrolly( pf_tilemap,0, sy );

		/* Several different rowscroll styles */
		switch ((control0>>3)&0xf) {
			case 0: rows=512; break;/* Every line of 512 height bitmap */
			case 1: rows=256; break;
			case 2: rows=128; break;
			case 3: rows=64; break;
			case 4: rows=32; break;
			case 5: rows=16; break;
			case 6: rows=8; break;
			case 7: rows=4; break;
			case 8: rows=2; break;
			default: rows=1; break;
		}
		if (height<rows) rows/=2; /* 8x8 tile layers have half as many lines as 16x16 */

		tilemap_set_scroll_rows(pf_tilemap,rows);
		for (offs = 0;offs < rows;offs++)
			tilemap_set_scrollx( pf_tilemap,offs, sx + rowdata[offs] );
	}
	else {
		tilemap_set_scroll_rows(pf_tilemap,1);
		tilemap_set_scroll_cols(pf_tilemap,1);
		tilemap_set_scrollx( pf_tilemap, 0, sx );
		tilemap_set_scrolly( pf_tilemap, 0, sy );
	}
}


static void combined_tilemap_draw(running_machine* machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	const bitmap_t *bitmap0 = tilemap_get_pixmap(pf3_tilemap);
	const bitmap_t *bitmap1 = tilemap_get_pixmap(pf4_tilemap);
	int x,y,p;

	const UINT16 width_mask=0x3ff;
	const UINT16 height_mask=0x1ff;
	const UINT16 y_src=deco32_pf34_control[2];
//  const UINT32 *rows=deco32_pf3_rowscroll;

	const UINT16 *bitmap0_y;
	const UINT16 *bitmap1_y;
	UINT32 *bitmap2_y;

	UINT16 x_src;

	for (y=8; y<248; y++) {
		const int py=(y_src+y)&height_mask;

		bitmap0_y=BITMAP_ADDR16(bitmap0, py, 0);
		bitmap1_y=BITMAP_ADDR16(bitmap1, py, 0);
		bitmap2_y=BITMAP_ADDR32(bitmap, y, 0);

		/* Todo:  Should add row enable, and col scroll, but never used as far as I can see */
		x_src=(deco32_pf34_control[1] + deco32_pf3_rowscroll[py])&width_mask;

		for (x=0; x<320; x++) {

			/* 0x200 is palette base for this tilemap */
			p = 0x200 +((bitmap0_y[x_src]&0xf) | ((bitmap0_y[x_src]&0x30)<<4) | ((bitmap1_y[x_src]&0xf)<<4));

			bitmap2_y[x]=machine->pens[p];

			x_src=(x_src+1)&width_mask;
		}
	}
}



/******************************************************************************/

VIDEO_UPDATE( captaven )
{
	int pf1_enable,pf2_enable,pf3_enable;
	static int last_pf3_bank;

	flip_screen_set(screen->machine, deco32_pf12_control[0]&0x80);
	tilemap_set_flip_all(screen->machine,flip_screen_get(screen->machine) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	deco32_setup_scroll(pf1_tilemap, 256,(deco32_pf12_control[5]>>0)&0xff,(deco32_pf12_control[6]>>0)&0xff,deco32_pf12_control[2],deco32_pf12_control[1],deco32_pf1_rowscroll,deco32_pf1_rowscroll+0x200);
	deco32_setup_scroll(pf1a_tilemap,512,(deco32_pf12_control[5]>>0)&0xff,(deco32_pf12_control[6]>>0)&0xff,deco32_pf12_control[2],deco32_pf12_control[1],deco32_pf1_rowscroll,deco32_pf1_rowscroll+0x200);
	deco32_setup_scroll(pf2_tilemap, 512,(deco32_pf12_control[5]>>8)&0xff,(deco32_pf12_control[6]>>8)&0xff,deco32_pf12_control[4],deco32_pf12_control[3],deco32_pf2_rowscroll,deco32_pf2_rowscroll+0x200);
	deco32_setup_scroll(pf3_tilemap, 512,(deco32_pf34_control[5]>>0)&0xff,(deco32_pf34_control[6]>>0)&0xff,deco32_pf34_control[4],deco32_pf34_control[3],deco32_pf3_rowscroll,deco32_pf3_rowscroll+0x200);

	/* PF1 & PF2 only have enough roms for 1 bank */
	deco32_pf1_bank=0;//(deco32_pf12_control[7]>> 4)&0xf;
	deco32_pf2_bank=0;//(deco32_pf12_control[7]>>12)&0xf;
	deco32_pf3_bank=(deco32_pf34_control[7]>> 4)&0xf;

	if (deco32_pf34_control[7]&0x0020) deco32_pf3_bank=0x4000; else deco32_pf3_bank=0;
	if (deco32_pf3_bank!=last_pf3_bank) tilemap_mark_all_tiles_dirty(pf3_tilemap);
	last_pf3_bank=deco32_pf3_bank;

	pf1_enable=deco32_pf12_control[5]&0x0080;
	pf2_enable=deco32_pf12_control[5]&0x8000;
	pf3_enable=deco32_pf34_control[5]&0x0080;

	tilemap_set_enable(pf1_tilemap,pf1_enable);
	tilemap_set_enable(pf1a_tilemap,pf1_enable);
	tilemap_set_enable(pf2_tilemap,pf2_enable);
	tilemap_set_enable(pf3_tilemap,pf3_enable);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	if ((deco32_pri&1)==0) {
		if (pf3_enable)
			tilemap_draw(bitmap,cliprect,pf3_tilemap,TILEMAP_DRAW_OPAQUE,1);
		else
			bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

		tilemap_draw(bitmap,cliprect,pf2_tilemap,0,2);
	} else {
		if (pf2_enable) {
			tilemap_draw(bitmap,cliprect,pf2_tilemap,0,1);
		}
		else
			bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

		tilemap_draw(bitmap,cliprect,pf3_tilemap,0,2);
	}

	/* PF1 can be in 8x8 mode or 16x16 mode */
	if (deco32_pf12_control[6]&0x80)
		tilemap_draw(bitmap,cliprect,pf1_tilemap,0,4);
	else
		tilemap_draw(bitmap,cliprect,pf1a_tilemap,0,4);

	captaven_draw_sprites(screen->machine,bitmap,cliprect,screen->machine->generic.buffered_spriteram.u32,3);

	return 0;
}

VIDEO_UPDATE( dragngun )
{
	/* Tilemap graphics banking */
	if ((((deco32_pf12_control[7]>> 4)&0x7)<<12)!=deco32_pf1_bank || deco32_pf1_flip!=((deco32_pf12_control[6]>>0)&0x3)) {
		tilemap_mark_all_tiles_dirty(pf1_tilemap);
		tilemap_mark_all_tiles_dirty(pf1a_tilemap);
	}
	if ((((deco32_pf12_control[7]>>12)&0x7)<<12)!=deco32_pf2_bank || deco32_pf2_flip!=((deco32_pf12_control[6]>>8)&0x3))
		tilemap_mark_all_tiles_dirty(pf2_tilemap);
	if ((((deco32_pf34_control[7]>> 5)&0x7)<<12)!=deco32_pf3_bank || deco32_pf3_flip!=((deco32_pf34_control[6]>>0)&0x3))
		tilemap_mark_all_tiles_dirty(pf3_tilemap);
	if ((((deco32_pf34_control[7]>>13)&0x7)<<12)!=deco32_pf4_bank || deco32_pf4_flip!=((deco32_pf34_control[6]>>8)&0x3))
		tilemap_mark_all_tiles_dirty(pf4_tilemap);
	deco32_pf1_bank=((deco32_pf12_control[7]>> 5)&0x7)<<12;
	deco32_pf2_bank=((deco32_pf12_control[7]>>12)&0x7)<<12;
	deco32_pf3_bank=((deco32_pf34_control[7]>> 5)&0x7)<<12;
	deco32_pf4_bank=((deco32_pf34_control[7]>>13)&0x7)<<12;

	deco32_pf1_flip=(deco32_pf12_control[6]>>0)&0x3;
	deco32_pf2_flip=(deco32_pf12_control[6]>>8)&0x3;
	deco32_pf3_flip=(deco32_pf34_control[6]>>0)&0x3;
	deco32_pf4_flip=(deco32_pf34_control[6]>>8)&0x3;

	deco32_setup_scroll(pf1_tilemap, 256,(deco32_pf12_control[5]>>0)&0xff,(deco32_pf12_control[6]>>0)&0xff,deco32_pf12_control[2],deco32_pf12_control[1],deco32_pf1_rowscroll,deco32_pf1_rowscroll+0x200);
	deco32_setup_scroll(pf1a_tilemap,512,(deco32_pf12_control[5]>>0)&0xff,(deco32_pf12_control[6]>>0)&0xff,deco32_pf12_control[2],deco32_pf12_control[1],deco32_pf1_rowscroll,deco32_pf1_rowscroll+0x200);
	deco32_setup_scroll(pf2_tilemap, 512,(deco32_pf12_control[5]>>8)&0xff,(deco32_pf12_control[6]>>8)&0xff,deco32_pf12_control[4],deco32_pf12_control[3],deco32_pf2_rowscroll,deco32_pf2_rowscroll+0x200);
	deco32_setup_scroll(pf3_tilemap, 512,(deco32_pf34_control[5]>>0)&0xff,(deco32_pf34_control[6]>>0)&0xff,deco32_pf34_control[2],deco32_pf34_control[1],deco32_pf3_rowscroll,deco32_pf3_rowscroll+0x200);
	deco32_setup_scroll(pf4_tilemap, 512,(deco32_pf34_control[5]>>8)&0xff,(deco32_pf34_control[6]>>8)&0xff,deco32_pf34_control[4],deco32_pf34_control[3],deco32_pf4_rowscroll,deco32_pf4_rowscroll+0x200);

	tilemap_set_enable(pf1_tilemap, deco32_pf12_control[5]&0x0080);
	tilemap_set_enable(pf1a_tilemap,deco32_pf12_control[5]&0x0080);
	tilemap_set_enable(pf2_tilemap, deco32_pf12_control[5]&0x8000);
	tilemap_set_enable(pf3_tilemap, deco32_pf34_control[5]&0x0080);
	tilemap_set_enable(pf4_tilemap, deco32_pf34_control[5]&0x8000);

	if ((deco32_pf34_control[5]&0x8000)==0)
		bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	tilemap_draw(bitmap,cliprect,pf4_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,pf3_tilemap,0,0);

	/* Raster update */
	if (deco32_raster_display_position) {
		int ptr=0,start,end=0;
		rectangle clip;
		int overflow=deco32_raster_display_position;

		clip.min_x = cliprect->min_x;
		clip.max_x = cliprect->max_x;

		/* Finish list up to end of visible display */
		deco32_raster_display_list[overflow++]=255;
		deco32_raster_display_list[overflow++]=deco32_pf12_control[1];
		deco32_raster_display_list[overflow++]=deco32_pf12_control[2];
		deco32_raster_display_list[overflow++]=deco32_pf12_control[3];
		deco32_raster_display_list[overflow++]=deco32_pf12_control[4];

		while (ptr<overflow) {
			start=end;
			end=deco32_raster_display_list[ptr++];
			deco32_pf12_control[1]=deco32_raster_display_list[ptr++];
			deco32_pf12_control[2]=deco32_raster_display_list[ptr++];
			deco32_pf12_control[3]=deco32_raster_display_list[ptr++];
			deco32_pf12_control[4]=deco32_raster_display_list[ptr++];

			clip.min_y = start;
			clip.max_y = end;

			deco32_setup_scroll(pf2_tilemap, 512,(deco32_pf12_control[5]>>8)&0xff,(deco32_pf12_control[6]>>8)&0xff,deco32_pf12_control[4],deco32_pf12_control[3],deco32_pf2_rowscroll,deco32_pf2_rowscroll+0x200);
			tilemap_draw(bitmap,&clip,pf2_tilemap,0,0);
		}
	} else {
		tilemap_draw(bitmap,cliprect,pf2_tilemap,0,0);
	}

	dragngun_draw_sprites(screen->machine,bitmap,cliprect,screen->machine->generic.buffered_spriteram.u32);

	/* PF1 can be in 8x8 mode or 16x16 mode */
	if (deco32_pf12_control[6]&0x80)
		tilemap_draw(bitmap,cliprect,pf1_tilemap,0,0);
	else
		tilemap_draw(bitmap,cliprect,pf1a_tilemap,0,0);
	return 0;
}

VIDEO_UPDATE( fghthist )
{
	/* Dirty tilemaps if any globals change */
	if (deco32_pf1_flip!=((deco32_pf12_control[6]>>0)&0x3))
		tilemap_mark_all_tiles_dirty(pf1_tilemap);
	if (deco32_pf2_flip!=((deco32_pf12_control[6]>>8)&0x3))
		tilemap_mark_all_tiles_dirty(pf2_tilemap);

	if ((((deco32_pf12_control[7]>>13)&0x7)<<12)!=deco32_pf2_bank || deco32_pf2_flip!=((deco32_pf12_control[6]>>8)&0x3))
		tilemap_mark_all_tiles_dirty(pf2_tilemap);
	if ((((deco32_pf34_control[7]>> 5)&0x7)<<12)!=deco32_pf3_bank || deco32_pf3_flip!=((deco32_pf34_control[6]>>0)&0x3))
		tilemap_mark_all_tiles_dirty(pf3_tilemap);
	if ((((deco32_pf34_control[7]>>13)&0x7)<<12)!=deco32_pf4_bank || deco32_pf4_flip!=((deco32_pf34_control[6]>>8)&0x3))
		tilemap_mark_all_tiles_dirty(pf4_tilemap);

	deco32_pf2_bank=((deco32_pf12_control[7]>>12)&0x3)<<12;
	deco32_pf3_bank=((deco32_pf34_control[7]>> 5)&0x3)<<12;  //WRONG WRONG WRONG  check masks
	deco32_pf4_bank=((deco32_pf34_control[7]>>12)&0x3)<<12;
	deco32_pf1_flip=(deco32_pf12_control[6]>>0)&0x3;
	deco32_pf2_flip=(deco32_pf12_control[6]>>8)&0x3;
	deco32_pf3_flip=(deco32_pf34_control[6]>>0)&0x3;
	deco32_pf4_flip=(deco32_pf34_control[6]>>8)&0x3;

	/* Enable registers */
	tilemap_set_enable(pf1_tilemap, deco32_pf12_control[5]&0x0080);
	tilemap_set_enable(pf2_tilemap, deco32_pf12_control[5]&0x8000);
	tilemap_set_enable(pf3_tilemap, deco32_pf34_control[5]&0x0080);
	tilemap_set_enable(pf4_tilemap, deco32_pf34_control[5]&0x8000);

	/* Setup scroll registers */
	deco32_setup_scroll(pf1_tilemap, 256,(deco32_pf12_control[5]>>0)&0xff,(deco32_pf12_control[6]>>0)&0xff,deco32_pf12_control[2],deco32_pf12_control[1],deco32_pf1_rowscroll,deco32_pf1_rowscroll+0x200);
	deco32_setup_scroll(pf2_tilemap, 512,(deco32_pf12_control[5]>>8)&0xff,(deco32_pf12_control[6]>>8)&0xff,deco32_pf12_control[4],deco32_pf12_control[3],deco32_pf2_rowscroll,deco32_pf2_rowscroll+0x200);
	deco32_setup_scroll(pf3_tilemap, 512,(deco32_pf34_control[5]>>0)&0xff,(deco32_pf34_control[6]>>0)&0xff,deco32_pf34_control[2],deco32_pf34_control[1],deco32_pf3_rowscroll,deco32_pf3_rowscroll+0x200);
	deco32_setup_scroll(pf4_tilemap, 512,(deco32_pf34_control[5]>>8)&0xff,(deco32_pf34_control[6]>>8)&0xff,deco32_pf34_control[4],deco32_pf34_control[3],deco32_pf4_rowscroll,deco32_pf4_rowscroll+0x200);

	/* Draw screen */
	deco16_clear_sprite_priority_bitmap();
	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,screen->machine->pens[0x000]); // Palette index not confirmed
	tilemap_draw(bitmap,cliprect,pf4_tilemap,0,0);
	if(deco32_pri&1)
	{
		tilemap_draw(bitmap,cliprect,pf2_tilemap,0,0);
		tilemap_draw(bitmap,cliprect,pf3_tilemap,0,2);
	}
	else
	{
		tilemap_draw(bitmap,cliprect,pf3_tilemap,0,0);
		tilemap_draw(bitmap,cliprect,pf2_tilemap,0,2);
	}
	fghthist_draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.buffered_spriteram.u32,3,0, 0xf);
	tilemap_draw(bitmap,cliprect,pf1_tilemap,0,0);
	return 0;
}

/*
    This function mimics the priority PROM/circuit on the pcb.  It takes
    the tilemaps & sprite bitmaps as inputs, and outputs a final pixel
    based on alpha & priority values.  Rendering sprites to temporary
    bitmaps is the only reasonable way to implement proper priority &
    blending support - it can't be done in-place on the final framebuffer
    without a lot of support bitmaps.
*/
static void mixDualAlphaSprites(bitmap_t *bitmap, const rectangle *cliprect, const gfx_element *gfx0, const gfx_element *gfx1, int mixAlphaTilemap)
{
	running_machine *machine = gfx0->machine;
	const pen_t *pens = machine->pens;
	const pen_t *pal0 = &pens[gfx0->color_base];
	const pen_t *pal1 = &pens[gfx1->color_base];
	const pen_t *pal2 = &pens[machine->gfx[(deco32_pri&1) ? 1 : 2]->color_base];
	int x,y;

	/* Mix sprites into main bitmap, based on priority & alpha */
	for (y=8; y<248; y++) {
		UINT8* tilemapPri=BITMAP_ADDR8(machine->priority_bitmap, y, 0);
		UINT16* sprite0=BITMAP_ADDR16(sprite0_mix_bitmap, y, 0);
		UINT16* sprite1=BITMAP_ADDR16(sprite1_mix_bitmap, y, 0);
		UINT32* destLine=BITMAP_ADDR32(bitmap, y, 0);
		UINT16* alphaTilemap=BITMAP_ADDR16(tilemap_alpha_bitmap, y, 0);

		for (x=0; x<320; x++) {
			UINT16 priColAlphaPal0=sprite0[x];
			UINT16 priColAlphaPal1=sprite1[x];
			UINT16 pri0=(priColAlphaPal0&0x6000)>>13;
			UINT16 pri1=(priColAlphaPal1&0x6000)>>13;
			UINT16 col0=((priColAlphaPal0&0x1f00)>>8) % gfx0->total_colors;
			UINT16 col1=((priColAlphaPal1&0x0f00)>>8) % gfx1->total_colors;
			UINT16 alpha1=priColAlphaPal1&0x8000;

			// Apply sprite bitmap 0 according to priority rules
			if ((priColAlphaPal0&0xff)!=0)
			{
				/*
                    Sprite 0 priority rules:

                    0 = Sprite above all layers
                    1 = Sprite under top playfield
                    2 = Sprite under top two playfields
                    3 = Sprite under all playfields
                */
				if ((pri0&0x3)==0 || (pri0&0x3)==1 || ((pri0&0x3)==2 && mixAlphaTilemap)) // Spri0 on top of everything, or under alpha playfield
				{
					destLine[x]=pal0[(priColAlphaPal0&0xff) + (gfx0->color_granularity * col0)];
				}
				else if ((pri0&0x3)==2) // Spri0 under top playfield
				{
					if (tilemapPri[x]<4)
						destLine[x]=pal0[(priColAlphaPal0&0xff) + (gfx0->color_granularity * col0)];
				}
				else // Spri0 under top & middle playfields
				{
					if (tilemapPri[x]<2)
						destLine[x]=pal0[(priColAlphaPal0&0xff) + (gfx0->color_granularity * col0)];
				}
			}

			// Apply sprite bitmap 1 according to priority rules
			if ((priColAlphaPal1&0xff)!=0)
			{
				// Apply alpha for this pixel based on Ace setting
				if (alpha1)
				{
					/*
                        Alpha rules:

                        Pri 0 - Over all tilemaps, but under sprite 0 pri 0, pri 1, pri 2
                        Pri 1 -
                        Pri 2 -
                        Pri 3 -
                    */

					/* Alpha values are tied to ACE ram... */
					//int alpha=((deco32_ace_ram[0x0 + (((priColAlphaPal1&0xf0)>>4)/2)]) * 8)-1;
					//if (alpha<0)
					//  alpha=0;

					/* I don't really understand how object ACE ram is really hooked up,
                        the only obvious place in Night Slashers is the stagecoach in level 2 */

					if (pri1==0 && (((priColAlphaPal0&0xff)==0 || ((pri0&0x3)!=0 && (pri0&0x3)!=1 && (pri0&0x3)!=2))))
					{
						if ((deco32_pri&1)==0 || ((deco32_pri&1)==1 && tilemapPri[x]<4) || ((deco32_pri&1)==1 && mixAlphaTilemap))
							destLine[x]=alpha_blend_r32(destLine[x], pal1[(priColAlphaPal1&0xff) + (gfx1->color_granularity * col1)], 0x80);
					}
					else if (pri1==1 && ((priColAlphaPal0&0xff)==0 || ((pri0&0x3)!=0 && (pri0&0x3)!=1 && (pri0&0x3)!=2)))
						destLine[x]=alpha_blend_r32(destLine[x], pal1[(priColAlphaPal1&0xff) + (gfx1->color_granularity * col1)], 0x80);
					else if (pri1==2)// TOdo
						destLine[x]=alpha_blend_r32(destLine[x], pal1[(priColAlphaPal1&0xff) + (gfx1->color_granularity * col1)], 0x80);
					else if (pri1==3)// TOdo
						destLine[x]=alpha_blend_r32(destLine[x], pal1[(priColAlphaPal1&0xff) + (gfx1->color_granularity * col1)], 0x80);
				}
				else
				{
					/*
                        Non alpha rules:

                        Pri 0 - Under sprite 0 pri 0, over all tilemaps
                    */
					if (pri1==0 && ((priColAlphaPal0&0xff)==0 || ((pri0&0x3)!=0)))
						destLine[x]=pal1[(priColAlphaPal1&0xff) + (gfx1->color_granularity * col1)];
					else if (pri1==1) // todo
						destLine[x]=pal1[(priColAlphaPal1&0xff) + (gfx1->color_granularity * col1)];
					else if (pri1==2) // todo
						destLine[x]=pal1[(priColAlphaPal1&0xff) + (gfx1->color_granularity * col1)];
					else if (pri1==3) // todo
						destLine[x]=pal1[(priColAlphaPal1&0xff) + (gfx1->color_granularity * col1)];
				}
			}

			/* Optionally mix in alpha tilemap */
			if (mixAlphaTilemap)
			{
				UINT16 p=alphaTilemap[x];
				if (p&0xf)
				{
					/* Alpha tilemap under top two sprite 0 priorities */
					if (((priColAlphaPal0&0xff)==0 || (pri0&0x3)==2 || (pri0&0x3)==3)
						&& ((priColAlphaPal1&0xff)==0 || (pri1&0x3)==2 || (pri1&0x3)==3 || alpha1))
					{
						/* Alpha values are tied to ACE ram */
						int alpha=((deco32_ace_ram[0x17 + (((p&0xf0)>>4)/2)]) * 8)-1;
						if (alpha<0)
							alpha=0;

						destLine[x]=alpha_blend_r32(destLine[x], pal2[p], 255-alpha);
					}
				}
			}
		}
	}
}

VIDEO_UPDATE( nslasher )
{
	int alphaTilemap=0;

	/* Dirty tilemaps if any globals change */
	if (deco32_pf1_flip!=((deco32_pf12_control[6]>>0)&0x3))
		tilemap_mark_all_tiles_dirty(pf1_tilemap);
	if (deco32_pf2_flip!=((deco32_pf12_control[6]>>8)&0x3))
		tilemap_mark_all_tiles_dirty(pf2_tilemap);
	if ((((deco32_pf12_control[7]>>12)&0x7)<<12)!=deco32_pf2_bank || deco32_pf2_flip!=((deco32_pf12_control[6]>>8)&0x3))
		tilemap_mark_all_tiles_dirty(pf2_tilemap);
	if ((((deco32_pf34_control[7]>> 4)&0x3)<<12)!=deco32_pf3_bank || deco32_pf3_flip!=((deco32_pf34_control[6]>>0)&0x3))
		tilemap_mark_all_tiles_dirty(pf3_tilemap);
	if ((((deco32_pf34_control[7]>>12)&0x3)<<12)!=deco32_pf4_bank || deco32_pf4_flip!=((deco32_pf34_control[6]>>8)&0x3))
		tilemap_mark_all_tiles_dirty(pf4_tilemap);

	deco32_pf2_bank=((deco32_pf12_control[7]>>12)&0x3)<<12;
	deco32_pf3_bank=((deco32_pf34_control[7]>> 4)&0x3)<<12;
	deco32_pf4_bank=((deco32_pf34_control[7]>>12)&0x3)<<12;
	deco32_pf1_flip=(deco32_pf12_control[6]>>0)&0x3;
	deco32_pf2_flip=(deco32_pf12_control[6]>>8)&0x3;
	deco32_pf3_flip=(deco32_pf34_control[6]>>0)&0x3;
	deco32_pf4_flip=(deco32_pf34_control[6]>>8)&0x3;

	/* Setup scroll registers */
	deco32_setup_scroll(pf1_tilemap, 256,(deco32_pf12_control[5]>>0)&0xff,(deco32_pf12_control[6]>>0)&0xff,deco32_pf12_control[2],deco32_pf12_control[1],deco32_pf1_rowscroll,deco32_pf1_rowscroll+0x200);
	deco32_setup_scroll(pf2_tilemap, 512,(deco32_pf12_control[5]>>8)&0xff,(deco32_pf12_control[6]>>8)&0xff,deco32_pf12_control[4],deco32_pf12_control[3],deco32_pf2_rowscroll,deco32_pf2_rowscroll+0x200);
	deco32_setup_scroll(pf3_tilemap, 512,(deco32_pf34_control[5]>>0)&0xff,(deco32_pf34_control[6]>>0)&0xff,deco32_pf34_control[2],deco32_pf34_control[1],deco32_pf3_rowscroll,deco32_pf3_rowscroll+0x200);
	deco32_setup_scroll(pf4_tilemap, 512,(deco32_pf34_control[5]>>8)&0xff,(deco32_pf34_control[6]>>8)&0xff,deco32_pf34_control[4],deco32_pf34_control[3],deco32_pf4_rowscroll,deco32_pf4_rowscroll+0x200);

	/* Enable registers */
	tilemap_set_enable(pf1_tilemap, deco32_pf12_control[5]&0x0080);
	tilemap_set_enable(pf2_tilemap, deco32_pf12_control[5]&0x8000);
	tilemap_set_enable(pf3_tilemap, deco32_pf34_control[5]&0x0080);
	tilemap_set_enable(pf4_tilemap, deco32_pf34_control[5]&0x8000);

	/* This is not a conclusive test for deciding if tilemap needs alpha blending */
	if (deco32_ace_ram[0x17]!=0x0 && deco32_pri)
		alphaTilemap=1;

	if (deco32_ace_ram_dirty)
		updateAceRam(screen->machine);

	bitmap_fill(sprite0_mix_bitmap,cliprect,0);
	bitmap_fill(sprite1_mix_bitmap,cliprect,0);
	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	if ((deco32_pf34_control[5]&0x8000)==0)
		bitmap_fill(bitmap,cliprect,screen->machine->pens[0x200]);

	/* Draw sprites to temporary bitmaps, saving alpha & priority info for later mixing */
	nslasher_draw_sprites(screen->machine,sprite0_mix_bitmap,cliprect,screen->machine->generic.buffered_spriteram.u32,3);
	nslasher_draw_sprites(screen->machine,sprite1_mix_bitmap,cliprect,screen->machine->generic.buffered_spriteram2.u32,4);

	/* Render alpha-blended tilemap to seperate buffer for proper mixing */
	bitmap_fill(tilemap_alpha_bitmap,cliprect,0);

	/* Draw playfields & sprites */
	if (deco32_pri&2)
	{
		combined_tilemap_draw(screen->machine,bitmap,cliprect);
		tilemap_draw(bitmap,cliprect,pf2_tilemap,0,4);
	}
	else
	{
		tilemap_draw(bitmap,cliprect,pf4_tilemap,0,1);
		if (deco32_pri&1)
		{
			tilemap_draw(bitmap,cliprect,pf2_tilemap,0,2);
			if (alphaTilemap)
				tilemap_draw(tilemap_alpha_bitmap,cliprect,pf3_tilemap,0,4);
			else
				tilemap_draw(bitmap,cliprect,pf3_tilemap,0,4);
		}
		else
		{
			tilemap_draw(bitmap,cliprect,pf3_tilemap,0,2);
			if (alphaTilemap)
				tilemap_draw(tilemap_alpha_bitmap,cliprect,pf2_tilemap,0,4);
			else
				tilemap_draw(bitmap,cliprect,pf2_tilemap,0,4);
		}
	}

	mixDualAlphaSprites(bitmap, cliprect, screen->machine->gfx[3], screen->machine->gfx[4], alphaTilemap);

	tilemap_draw(bitmap,cliprect,pf1_tilemap,0,0);
	return 0;
}
