#include "driver.h"
#include "includes/tatsumi.h"

static tilemap_t *tx_layer;
static tilemap_t *layer0, *layer1, *layer2, *layer3;

static bitmap_t *temp_bitmap;

static UINT8 *apache3_road_x_ram;
static UINT8 apache3_road_z;

UINT16 *roundup_r_ram, *roundup_p_ram, *roundup_l_ram;
UINT16 *cyclwarr_videoram0, *cyclwarr_videoram1;
UINT16* tatsumi_sprite_control_ram;
static UINT16* roundup5_vram;

static UINT16 bigfight_bank, bigfight_last_bank;

static UINT8 roundupt_crt_selected_reg;
static UINT8 roundupt_crt_reg[64];

static UINT8* shadow_pen_array;

/******************************************************************************/

WRITE16_HANDLER(tatsumi_sprite_control_w)
{
	COMBINE_DATA(&tatsumi_sprite_control_ram[offset]);

	/* 0xe0 is bank switch, others unknown */
//  if ((offset==0xe0 && data&0xefff) || offset!=0xe0)
//      logerror("%08x:  Tatsumi TZB215 sprite control %04x %08x\n", cpu_get_pc(space->cpu), offset, data);
}

/******************************************************************************/

WRITE16_HANDLER( apache3_road_z_w )
{
	apache3_road_z = data & 0xff;
}

WRITE8_HANDLER( apache3_road_x_w )
{
	// Note: Double buffered. Yes, this is correct :)
	apache3_road_x_ram[data] = offset;
}

READ16_HANDLER( roundup5_vram_r )
{
	offset+=((tatsumi_control_word&0x0c00)>>10) * 0xc000;
	return roundup5_vram[offset];
}

WRITE16_HANDLER( roundup5_vram_w )
{
	offset+=((tatsumi_control_word&0x0c00)>>10) * 0xc000;

//  if (offset>=0x30000)
//      logerror("effective write to vram %06x %02x (control %04x)\n",offset,data,tatsumi_control_word);

	COMBINE_DATA(&roundup5_vram[offset]);

	offset=offset%0xc000;

	gfx_element_mark_dirty(space->machine->gfx[1],offset/0x10);
}


WRITE16_HANDLER( roundup5_palette_w )
{
//  static int hack=0;
	int word;

	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);

//  if (offset==0xbfe)
//      hack++;

//  if (hack>1)
//      return;

/*
apache 3 schematics state

bit 4:  250
bit 3:  500
bit 2:  1k
bit 1:  2k
bit 0:  3.9kOhm resistor

*/

//  logerror("PAL: %04x %02x\n",offset,data);

	offset&=~1;
	word = ((space->machine->generic.paletteram.u16[offset] & 0xff)<<8) | (space->machine->generic.paletteram.u16[offset+1] & 0xff);
	palette_set_color_rgb(space->machine,offset/2,pal5bit(word >> 10),pal5bit(word >> 5),pal5bit(word >> 0));
}


WRITE16_HANDLER( apache3_palette_w )
{
//  static int hack=0;

	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);

//  if (offset==0xbfe)
//      hack++;

//  if (hack>1)
//      return;

/*
apache 3 schematics state

bit 4:  250
bit 3:  500
bit 2:  1k
bit 1:  2k
bit 0:  3.9kOhm resistor

*/

//  logerror("PAL: %04x %02x\n",offset,data);

	data = space->machine->generic.paletteram.u16[offset];
	palette_set_color_rgb(space->machine,offset,pal5bit(data >> 10),pal5bit(data >> 5),pal5bit(data >> 0));
}


WRITE16_HANDLER( roundup5_text_w )
{
	COMBINE_DATA(&space->machine->generic.videoram.u16[offset]);
	tilemap_mark_tile_dirty( tx_layer,offset);
}

READ16_HANDLER( cyclwarr_videoram0_r )
{
	 return cyclwarr_videoram0[offset];
}

READ16_HANDLER( cyclwarr_videoram1_r )
{
	 return cyclwarr_videoram1[offset];
}

WRITE16_HANDLER( cyclwarr_videoram0_w )
{
	COMBINE_DATA(&cyclwarr_videoram0[offset]);
	if (offset>=0x400)
	{
		tilemap_mark_tile_dirty( layer0, offset-0x400);
		tilemap_mark_tile_dirty( layer1, offset-0x400);
	}
}

WRITE16_HANDLER( cyclwarr_videoram1_w )
{
	COMBINE_DATA(&cyclwarr_videoram1[offset]);
	if (offset>=0x400)
	{
		tilemap_mark_tile_dirty( layer2, offset-0x400);
		tilemap_mark_tile_dirty( layer3, offset-0x400);
	}
}

WRITE16_HANDLER( roundup5_crt_w )
{
	if (offset==0 && ACCESSING_BITS_0_7)
		roundupt_crt_selected_reg=data&0x3f;
	if (offset==1 && ACCESSING_BITS_0_7) {
		roundupt_crt_reg[roundupt_crt_selected_reg]=data;
//      if (roundupt_crt_selected_reg!=0xa && roundupt_crt_selected_reg!=0xb && roundupt_crt_selected_reg!=29)
//      logerror("%08x:  Crt write %02x %02x\n",cpu_get_pc(space->cpu),roundupt_crt_selected_reg,data);
	}
}

/********************************************************************/

static TILE_GET_INFO( get_text_tile_info )
{
	int tile = machine->generic.videoram.u16[tile_index];
	SET_TILE_INFO(
			1,
			tile & 0xfff,
			tile >> 12,
			0);
}

static TILE_GET_INFO( get_tile_info_bigfight_0 )
{
	int tile=cyclwarr_videoram0[(tile_index+0x400)%0x8000];
	int bank = (bigfight_a40000[0] >> (((tile&0xc00)>>10)*4))&0xf;
	SET_TILE_INFO(1,(tile&0x3ff)+(bank<<10),(tile>>12)&0xf,0);
}

static TILE_GET_INFO( get_tile_info_bigfight_1 )
{
	int tile=cyclwarr_videoram1[(tile_index+0x400)%0x8000];
	int bank = (bigfight_a40000[0] >> (((tile&0xc00)>>10)*4))&0xf;
	SET_TILE_INFO(1,(tile&0x3ff)+(bank<<10),(tile>>12)&0xf,0);
}

/********************************************************************/

VIDEO_START( apache3 )
{
	tx_layer = tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,8,8,64,64);
	shadow_pen_array = auto_alloc_array_clear(machine, UINT8, 8192);
	temp_bitmap = auto_bitmap_alloc(machine, 512, 512, BITMAP_FORMAT_RGB32);
	apache3_road_x_ram = auto_alloc_array(machine, UINT8, 512);

	tilemap_set_transparent_pen(tx_layer,0);
}

VIDEO_START( roundup5 )
{
	tx_layer = tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,8,8,128,64);
	shadow_pen_array = auto_alloc_array_clear(machine, UINT8, 8192);
	roundup5_vram = auto_alloc_array(machine, UINT16, (0x48000 * 4)/2);

	tilemap_set_transparent_pen(tx_layer,0);

	gfx_element_set_source(machine->gfx[1], (UINT8 *)roundup5_vram);
}

VIDEO_START( cyclwarr )
{
	layer0 = tilemap_create(machine, get_tile_info_bigfight_0,tilemap_scan_rows,8,8,64,512);
	//layer1 = tilemap_create(machine, get_tile_info_bigfight_0,tilemap_scan_rows,8,8,64,512);
	layer1 = tilemap_create(machine, get_tile_info_bigfight_0,tilemap_scan_rows,8,8,128,256);
	layer2 = tilemap_create(machine, get_tile_info_bigfight_1,tilemap_scan_rows,8,8,64,512);
	layer3 = tilemap_create(machine, get_tile_info_bigfight_1,tilemap_scan_rows,8,8,64,512);

	shadow_pen_array = auto_alloc_array_clear(machine, UINT8, 8192);
}

VIDEO_START( bigfight )
{
	layer0 = tilemap_create(machine, get_tile_info_bigfight_0,tilemap_scan_rows,8,8,128,256);
	layer1 = tilemap_create(machine, get_tile_info_bigfight_0,tilemap_scan_rows,8,8,128,256);
	layer2 = tilemap_create(machine, get_tile_info_bigfight_1,tilemap_scan_rows,8,8,128,256);
	layer3 = tilemap_create(machine, get_tile_info_bigfight_1,tilemap_scan_rows,8,8,128,256);

	shadow_pen_array = auto_alloc_array_clear(machine, UINT8, 8192);
}

/********************************************************************/

INLINE void roundupt_drawgfxzoomrotate(
		bitmap_t *dest_bmp, const rectangle *clip, const gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,UINT32 ssx,UINT32 ssy,
		int scalex, int scaley, int rotate, int write_priority_only )
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
			const UINT8 *shadow_pens = shadow_pen_array + (gfx->color_granularity * (color % gfx->total_colors));
			const UINT8 *code_base = gfx_element_get_data(gfx, code % gfx->total_elements);

			int block_size = 8 * scalex;
			int sprite_screen_height = ((ssy&0xffff)+block_size)>>16;
			int sprite_screen_width = ((ssx&0xffff)+block_size)>>16;

			if (sprite_screen_width && sprite_screen_height)
			{
				/* compute sprite increment per screen pixel */
				int dx = (gfx->width<<16)/sprite_screen_width;
				int dy = (gfx->height<<16)/sprite_screen_height;

				int sx;//=ssx>>16;
				int sy;//=ssy>>16;


//              int ex = sx+sprite_screen_width;
//              int ey = sy+sprite_screen_height;

				int x_index_base;
				int y_index;
				int ex,ey;

							int incxx=0x10000;//(int)((float)dx * cos(theta));
//                          int incxy=0x0;//(int)((float)dy * -sin(theta));
							int incyx=0x0;//(int)((float)dx * sin(theta));
//                          int incyy=0x10000;//(int)((float)dy * cos(theta));

							if (flipx)
							{

							}


				if (ssx&0x80000000) sx=0-(0x10000 - (ssx>>16)); else sx=ssx>>16;
				if (ssy&0x80000000) sy=0-(0x10000 - (ssy>>16)); else sy=ssy>>16;
				ex = sx+sprite_screen_width;
				ey = sy+sprite_screen_height;
				if( flipx )
				{
					x_index_base = (sprite_screen_width-1)*dx;
					dx = -dx;
					incxx=-incxx;
					incyx=-incyx;
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
#if 0
					{
						{
							int startx=0;
							int starty=0;

//                          int incxx=0x10000;
//                          int incxy=0;
//                          int incyx=0;
//                          int incyy=0x10000;
							double theta=rotate * ((2.0 * M_PI)/512.0);
							double c=cos(theta);
							double s=sin(theta);


						//  if (ey-sy > 0)
						//      dy=dy / (ey-sy);
							{
							float angleAsRadians=(float)rotate * (7.28f / 512.0f);
							//float ccx = cosf(angleAsRadians);
							//float ccy = sinf(angleAsRadians);
							float a=0;

							}

							for( y=sy; y<ey; y++ )
							{
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);
								int cx = startx;
								int cy = starty;

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									const UINT8 *source = code_base + (cy>>16) * gfx->line_modulo;
									int c = source[(cx >> 16)];
									if( c != transparent_color )
									{
										if (write_priority_only)
											dest[x]=shadow_pens[c];
										else
											dest[x]=pal[c];
									}
									cx += incxx;
									cy += incxy;
								}
								startx += incyx;
								starty += incyy;
							}
						}
					}
#endif
#if 1 // old
					{
						{
							for( y=sy; y<ey; y++ )
							{
								const UINT8 *source = code_base + (y_index>>16) * gfx->line_modulo;
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);
								UINT8 *priority_dest = BITMAP_ADDR8(dest_bmp, y, 0);

								int x, x_index = x_index_base;
								for( x=sx; x<ex; x++ )
								{
									int c = source[x_index>>16];
									if( c )
									{
										// Only draw shadow pens if writing priority buffer
										if (write_priority_only)
											priority_dest[x]=shadow_pens[c];
										else if (!shadow_pens[c])
											dest[x]=pal[c];
									}
									x_index += dx;
								}

								y_index += dy;
							}
						}
					}
#endif
				}
			}
		}
	}
}

static void mycopyrozbitmap_core(bitmap_t *bitmap,bitmap_t *srcbitmap,
		int dstx,int dsty, int srcwidth, int srcheight,int incxx,int incxy,int incyx,int incyy,
		const rectangle *clip,int transparent_color)
{
	UINT32 cx;
	UINT32 cy;
	int x;
	int sx;
	int sy;
	int ex;
	int ey;
//  const int xmask = srcbitmap->width-1;
//  const int ymask = srcbitmap->height-1;
	const int widthshifted = srcwidth << 16;
	const int heightshifted = srcheight << 16;
	UINT32 *dest;

	UINT32 startx=0;
	UINT32 starty=0;

	sx = dstx;
	sy = dsty;
	ex = dstx + srcwidth;
	ey = dsty + srcheight;

	if (sx<clip->min_x) sx=clip->min_x;
	if (ex>clip->max_x) ex=clip->max_x;
	if (sy<clip->min_y) sy=clip->min_y;
	if (ey>clip->max_y) ey=clip->max_y;

	if (sx <= ex)
	{

		while (sy <= ey)
		{
			x = sx;
			cx = startx;
			cy = starty;
			dest = BITMAP_ADDR32(bitmap, sy, sx);

			while (x <= ex)
			{
				if (cx < widthshifted && cy < heightshifted)
				{
					int c = *BITMAP_ADDR32(srcbitmap, cy >> 16, cx >> 16);

					if (c != transparent_color)
						*dest = c;
				}

				cx += incxx;
				cy += incxy;
				x++;
				dest++;
			}
			startx += incyx;
			starty += incyy;
			sy++;
		}
	}
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int write_priority_only, int rambank)
{
	UINT16 *spriteram16 = machine->generic.spriteram.u16;
	int offs,fx,x,y,color;
	int w,h,index,lines,scale,rotate;
	UINT8 *src1, *src2;

	int y_offset;

	int render_x, render_y;
	int extent_x, extent_y;

	// Sprite data is double buffered
	for (offs = rambank;offs < rambank + 0x800;offs += 6)
	{
		/*
            Sprite RAM itself uses an index into two ROM tables to actually draw the object.

            Sprite RAM format:

            Word 0: 0xf000 - ?
                    0x0fff - Index into ROM sprite table
            Word 1: 0x8000 - X Flip
                    0x7ff8 - Colour (TODO:  Check top bit on Apache 3/Round up)
                    0x0007 - ?
            Word 2: 0xffff - X position
            Word 3: 0xffff - Y position
            Word 4: 0x01ff - Scale
            Word 5: 0x01ff - Rotation

            Sprite ROM table format, alternate lines come from each bank, with the
            very first line indicating control information:

            First bank:
            Byte 0: Y destination offset (in scanlines, unaffected by scale).
            Byte 1: Always 0?
            Byte 2: Number of source scanlines to render from (so unaffected by destination scale).
            Byte 3: Usually 0, sometimes 0x80??

            Other banks:
            Byte 0: Width of line in tiles (-1)
            Byte 1: X offset to start drawing line at (multipled by scale * 8)
            Bytes 2/3: Tile index to start fetching tiles from (increments per tile).

        */
		y = spriteram16[offs+3];
		x = spriteram16[offs+2];
		scale = spriteram16[offs+4]&0x1ff;
		color = (spriteram16[offs+1]>>3)&0x1ff;
		fx = spriteram16[offs+1]&0x8000;
		rotate = 0;//spriteram16[offs+5]&0x1ff; // Todo:  Turned off for now

		index = spriteram16[offs];

//      if (spriteram16[offs+1]&0x7)
//          color=rand()%0xff;

		/* End of sprite list marker */
		if (index==0xffff || spriteram16[offs+4]==0xffff) //todo
			return;

		if (index>=0x4000)
			continue;

		src1 = tatsumi_rom_sprite_lookup1 + (index*4);
		src2 = tatsumi_rom_sprite_lookup2 + (index*4);

		lines=src1[2];
		y_offset=src1[0]&0xf8;
		lines-=y_offset;

		render_x=x<<16;
		render_y=y<<16;
		scale=scale<<9; /* 0x80 becomes 0x10000 */

		render_y+=y_offset * scale;

		if (rotate)
		{
			render_y=0;
			bitmap_fill(temp_bitmap, 0, 0);
		}

		extent_x=extent_y=0;

		src1+=4;
		h=0;
		while (lines>0) {
			int base, x_offs, x_width, x_pos, draw_this_line=1;
			int this_extent=0;

			/* Odd and even lines come from different banks */
			if (h&1) {
				x_width=src1[0]+1;
				x_offs=src1[1]*scale*8;
				base=src1[2] | (src1[3]<<8);
			}
			else {
				x_width=src2[0]+1;
				x_offs=src2[1]*scale*8;
				base=src2[2] | (src2[3]<<8);
			}

			if (draw_this_line) {
				base*=2;

				if (!rotate)
				{
					if (fx)
						x_pos=render_x-x_offs-scale*8;
					else
						x_pos=render_x+x_offs;
				}
				else
					x_pos=x_offs;

				for (w=0; w<x_width; w++) {
					if (rotate)
						roundupt_drawgfxzoomrotate(
								temp_bitmap,cliprect,machine->gfx[0],
								base,
								color,fx,0,x_pos,render_y,
								scale,scale,0,write_priority_only);
					else
						roundupt_drawgfxzoomrotate(
								bitmap,cliprect,machine->gfx[0],
								base,
								color,fx,0,x_pos,render_y,
								scale,scale,0,write_priority_only);
					base++;

					if (fx)
						x_pos-=scale*8;
					else
						x_pos+=scale*8;

					this_extent+=scale*8;
				}
				if (h&1)
					src1+=4;
				else
					src2+=4;

				if (this_extent > extent_x)
					extent_x=this_extent;
				this_extent=0;

				render_y+=8 * scale;
				extent_y+=8 * scale;
				h++;
				lines-=8;
			} else {
				h=32; //hack
			}
		}

		if (rotate)
		{
			double theta=rotate * ((2.0 * M_PI)/512.0);

			int incxx=(int)((float)65536.0 * cos(theta));
			int incxy=(int)((float)65536.0 * -sin(theta));
			int incyx=(int)((float)65536.0 * sin(theta));
			int incyy=(int)((float)65536.0 * cos(theta));

			extent_x=extent_x>>16;
			extent_y=extent_y>>16;
			if (extent_x>2 && extent_y>2)
			mycopyrozbitmap_core(bitmap, temp_bitmap, x/* + (extent_x/2)*/, y /*+ (extent_y/2)*/, extent_x, extent_y, incxx, incxy, incyx, incyy, cliprect, 0);
		}
	}
}

static void draw_sky(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect, int palette_base, int start_offset)
{
	// all todo
	int x,y;

	if (start_offset&0x8000)
		start_offset=-(0x10000 - start_offset);

	start_offset=-start_offset;

start_offset-=48;
	for (y=0; y<256; y++) {
		for (x=0; x<320; x++) {
			int col=palette_base + y + start_offset;
			if (col<palette_base) col=palette_base;
			if (col>palette_base+127) col=palette_base+127;

			*BITMAP_ADDR32(bitmap, y, x) = machine->pens[col];
		}
	}
}

static void draw_road(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,bitmap_t *shadow_bitmap)
{
/*
0xf980 0x0008 0x8c80 0x4a00 - road right to below, width unknown (32 pixels guess)
0xfa80 0x0008 0x8c80 0x4a00 - road right to below, width unknown (32 pixels guess)

0xfb80 0x0008 0x8c80 0x4a00 - road in middle of screen, width unknown (32 pixels guess)

0xfc80 0x0008 0x8c80 0x4a00 - road width above to left, same width as above (ie, xpos - 32)
0xfd80 0x0008 0x8c80 0x4a00 - road width above to left, same width as above (ie, xpos - 32)
0xfe80 0x0008 0x8c80 0x4a00 - road width above to left, same width as above (ie, xpos - 32)
0xff80 0x0008 0x8c80 0x4a00 - road width above to left, same width as above (ie, xpos - 32)
0x0001                      - road half/width above to left, (ie, xpos - 16)
0x0081                      - road width to left as usual (xpos-16 from above, or 32 from above2)

0xfb0b 0x210b 0x8cf5 0x0dea - blue & left & right, with  blue|----|----|----|----|blue
in this mode changing columns 2 & 3 have no apparent effect
0xfb0b 0x7b09 0x8cf5 0x0dea - as above, but scaled up - perhaps 18 pixels shifted (twice that overall size)
0xfb0b 0x6c07 0x8cf5 0x0dea - as above, but scaled up - perhaps 40 pixels shifted from above
0xfb0b 0xaa06 0x8cf5 0x0dea - as above, but scaled up - perhaps 16 pixels shifted from above
0xfb0b 0xb005 0x8cf5 0x0dea - as above, but scaled up - perhaps 38 pixels shifted from above

b21 diff is 1a6
97b         20f
76c         c2
6aa         fa
5b0


0x0000 0x0008 0xxxxx 0xxxx - line starting at 0 for 128 pixels - 1 to 1 with road pixel data
0xff00 0x0008 0xxxxx 0xxxx - line starting at 32 for 128 pixels - 1 to 1 with road pixel data
0xfe00 0x0008 0xxxxx 0xxxx - line starting at 64 for 128 pixels - 1 to 1 with road pixel data



at standard zoom (0x800)
shift of 0x100 moves 32 pixels
so shift of 8 is assumed to move 1 pixel

at double zoom (0x1000)
assume shift of 0x100 only moves 16 pixels
so

0x100 * 0x400 => 0x40
0x100 * step 0x800 = must come out at 0x20
0x100 * step 0x1000 = must come out at 0x10
0x100 * step 0x2000 = 0x5

pos is 11.5 fixed point

-0x580 = middle
-0x180
-0x080
0
0x80

*/
	int y,x;
	int visible_line=0;
	const UINT16* data=roundup_r_ram;

	// Road layer enable (?)
	if ((roundup5_unknown0[0x1]&0x1)==0)
		return;

	// Road data bank select (double buffered)
	if (roundup5_e0000_ram[0]&0x10)
		data+=0x400;

	// ??  Todo: This is wrong - don't know how to clip the road properly
	y=256 - (roundup5_unknown0[0xb/2] >> 8);
	data+=y*4;

	visible_line=0;

	for (/*y=0*/; y<256; y++) {

		int shift=data[0];
		int shift2=data[2];
		int pal=4; //(data[3]>>8)&0xf;
		int step=((data[1]&0xff)<<8)|((data[1]&0xff00)>>8);
		int samplePos=0;
		const UINT16* linedata=roundup_p_ram;// + (0x100 * pal);
		int startPos=0, endPos=0;

		int palette_byte;//=roundup_l_ram[visible_line/8];

		/*
            Each road line consists of up to two sets of 128 pixel data that can be positioned
            on the x-axis and stretched/compressed on the x-axis.  Any screen pixels to the left
            of the first set are drawn with pen 0 of the road pixel data.  Any screen pixels to the
            right of the second set line are drawn with pen 127 of the road pixel data.

            The road control data is laid out as follows (4 words per screen line, with 2 banks):

            Word 0: Line shift for 1st set - 13.3 signed fixed point value.
            Word 1: Line scale - 5.11 fixed point value.  So 0x800 is 1:1, 0x400 is 1:2, etc
            Word 2: Line shift for 2nd set - 13.3 signed fixed point value.
            Word 3: ?

            The scale is shared between both pixel sets.  The 2nd set is only used when the road
            forks into two between stages.  The 2nd line shift is an offset from the last pixel
            of the 1st set.  The 2nd line shift uses a different palette bank.

2nd road uses upper palette - confirmed by water stage.
offset is from last pixel of first road segment?
//last pixel of first road is really colour from 2nd road line?

        */

		palette_byte=roundup_l_ram[visible_line/8];
		pal=4 + ((palette_byte>>(visible_line%8))&1);

		visible_line++;

		if (shift&0x8000)
			shift=-(0x10000 - shift);
		shift=-shift;

		if (step)
			startPos=((shift<<8) + 0x80 )/ step;

		/* Fill in left of road segment */
		for (x=0; x<startPos && x<320; x++) {
			int col = linedata[0]&0xf;
			UINT8 shadow=*BITMAP_ADDR8(shadow_bitmap, y, x);
			if (shadow)
				*BITMAP_ADDR32(bitmap, y, x) = machine->pens[768 + pal*16 + col];
			else
				*BITMAP_ADDR32(bitmap, y, x) = machine->pens[256 + pal*16 + col];
		}

		/* If startpos is negative, clip it and adjust the sampling position accordingly */
		if (startPos<0) {
			samplePos=step*(0-startPos);
			startPos=0;
		} else {
			samplePos=0;
		}

		/* Fill in main part of road, then right-hand side edge */
		for (x=startPos; x<320 && (samplePos>>11)<0x80; x++) {
			// look up colour
			int col = linedata[(samplePos>>11)&0x7f]&0xf;
			UINT8 shadow=*BITMAP_ADDR8(shadow_bitmap, y, x);

			/* Clamp if we have reached the end of the pixel data */
			//if ((samplePos>>11) > 0x7f)
			//  col=linedata[0x7f]&0xf;

			if (shadow)
				*BITMAP_ADDR32(bitmap, y, x) = machine->pens[768 + pal*16 + col];
			else
				*BITMAP_ADDR32(bitmap, y, x) = machine->pens[256 + pal*16 + col];

			samplePos+=step;
		}

		/* Now work out how many pixels until start of 2nd segment */
		startPos=x;

		if (shift2&0x8000)
			shift2=-(0x10000 - shift2);
		shift2=-shift2;

		if (step)
			endPos=((shift2<<8) + 0x80) / step;
		else
			endPos=0;
		endPos-=128;
		endPos=startPos+endPos;

		/* Fill pixels */
		for (x=startPos; x<320 && x<endPos; x++) {
			int col = linedata[0x80]&0xf;
			UINT8 shadow=*BITMAP_ADDR8(shadow_bitmap, y, x);

			/* Clamp if we have reached the end of the pixel data */
			//if ((samplePos>>11) > 0x7f)
			//  col=linedata[0x7f]&0xf;

			if (shadow)
				*BITMAP_ADDR32(bitmap, y, x) = machine->pens[768 + pal*16 + col + 32];
			else
				*BITMAP_ADDR32(bitmap, y, x) = machine->pens[256 + pal*16 + col + 32];
		}

		if (endPos<0) {
			samplePos=step*(0-startPos);
		}
		else if (endPos<x) {
			samplePos=step*(x-endPos);
		} else {
			samplePos=0; // todo
		}

		for (/*x=endPos*/; x<320; x++) {
			// look up colour
			int col = linedata[((samplePos>>11)&0x7f) + 0x200]&0xf;
			UINT8 shadow=*BITMAP_ADDR8(shadow_bitmap, y, x);

			/* Clamp if we have reached the end of the pixel data */
			if ((samplePos>>11) > 0x7f)
				col=linedata[0x7f + 0x200]&0xf;

			if (shadow)
				*BITMAP_ADDR32(bitmap, y, x) = machine->pens[768 + pal*16 + col + 32];
			else
				*BITMAP_ADDR32(bitmap, y, x) = machine->pens[256 + pal*16 + col + 32];

			samplePos+=step;
		}
		data+=4;
	}
}

static void update_cluts(running_machine *machine, int fake_palette_offset, int object_base, int length)
{
	/* Object palettes are build from a series of cluts stored in the object roms.

        We update 'Mame palettes' from the clut here in order to simplify the
        draw routines.  We also note down any uses of the 'shadow' pen (index 255).
    */
	int i;
	const UINT8* bank1=tatsumi_rom_clut0;
	const UINT8* bank2=tatsumi_rom_clut1;
	for (i=0; i<length; i+=8) {
		palette_set_color(machine,fake_palette_offset+i+0,palette_get_color(machine,bank1[1]+object_base));
		shadow_pen_array[i+0]=(bank1[1]==255);
		palette_set_color(machine,fake_palette_offset+i+1,palette_get_color(machine,bank1[0]+object_base));
		shadow_pen_array[i+1]=(bank1[0]==255);
		palette_set_color(machine,fake_palette_offset+i+2,palette_get_color(machine,bank1[3]+object_base));
		shadow_pen_array[i+2]=(bank1[3]==255);
		palette_set_color(machine,fake_palette_offset+i+3,palette_get_color(machine,bank1[2]+object_base));
		shadow_pen_array[i+3]=(bank1[2]==255);

		palette_set_color(machine,fake_palette_offset+i+4,palette_get_color(machine,bank2[1]+object_base));
		shadow_pen_array[i+4]=(bank2[1]==255);
		palette_set_color(machine,fake_palette_offset+i+5,palette_get_color(machine,bank2[0]+object_base));
		shadow_pen_array[i+5]=(bank2[0]==255);
		palette_set_color(machine,fake_palette_offset+i+6,palette_get_color(machine,bank2[3]+object_base));
		shadow_pen_array[i+6]=(bank2[3]==255);
		palette_set_color(machine,fake_palette_offset+i+7,palette_get_color(machine,bank2[2]+object_base));
		shadow_pen_array[i+7]=(bank2[2]==255);

		bank1+=4;
		bank2+=4;
	}
}

/**********************************************************************/

static void draw_bg(running_machine *machine, bitmap_t *dst, tilemap_t *src, const UINT16* scrollx, const UINT16* scrolly, const UINT16* tilemap_ram, int tile_bank, int xscroll_offset, int yscroll_offset, int xsize, int ysize)
{
	/*
        Each tile (0x4000 of them) has a lookup table in ROM to build an individual 3-bit palette
        from sets of 8 bit palettes!
    */
	const UINT8* tile_cluts = memory_region(machine, "gfx4");
	const bitmap_t *src_bitmap = tilemap_get_pixmap(src);
	int src_y_mask=ysize-1;
	int src_x_mask=xsize-1;
	int tile_y_mask=(ysize/8)-1;
	int tile_x_mask=(xsize/8)-1;
	int tiles_per_x_line=(xsize/8);
	int x, y, p, pp, ppp;

	for (y=0; y<240; y++)
	{
		for (x=0; x<320; x++)
		{
			int src_x = x + scrollx[y] + xscroll_offset;
			int src_y = y + scrolly[y] + yscroll_offset;
			int tile_index = (((src_x>>3)&tile_x_mask) + (((src_y>>3)&tile_y_mask) * tiles_per_x_line));
			int bank = (tile_bank >> (((tilemap_ram[(tile_index+0x400)&0x7fff]&0xc00)>>10)*4))&0xf;
			int tile = (tilemap_ram[(tile_index+0x400)&0x7fff]&0x3ff) | (bank<<10);

			p=*BITMAP_ADDR16(src_bitmap, src_y&src_y_mask, src_x&src_x_mask);
			pp=tile_cluts[tile*8 + (p&0x7)];
			ppp=pp + ((p&0x78)<<5);

			if ((p&0x7)!=0 || ((p&0x7)==0 && (pp&0x7)!=0)) // Transparent pixels are set by both the tile pixel data==0 AND colour palette==0
				*BITMAP_ADDR32(dst, y, x) = machine->pens[ppp];
		}
	}
}

/* Draw the sky and ground, applying rotation (eventually). Experimental! */
#if 0
static void draw_ground(running_machine *machine, bitmap_t *dst, const rectangle *cliprect)
{
	int x, y;
	const UINT8 *lut = memory_region(machine, "proms");

	UINT16 gva = 0x180; // TODO
	UINT8 sky_val = apache3_rotate_ctrl[1] & 0xff;

	for (y = cliprect->min_y; y <= cliprect->max_y; ++y)
	{
		UINT16 rgdb = 0;//apache3_road_x_ram[gva & 0xff];
		UINT16 gha = 0xf60; // test
		int ln = (((lut[gva & 0x7f] & 0x7f) + (apache3_road_z & 0x7f)) >> 5) & 3;

		if (gva & 0x100)
		{
			/* Sky */
			for (x = cliprect->min_x; x <= cliprect->max_x; ++x)
			{
				*BITMAP_ADDR32(dst, y, x) = machine->pens[0x100 + (sky_val & 0x7f)];

				/* Update horizontal counter? */
				gha = (gha + 1) & 0xfff;
			}
		}
		else
		{
			/* Ground */
			for (x = cliprect->min_x; x <= cliprect->max_x; ++x)
			{
				UINT8 colour;
				UINT16 hval;
				UINT8 pixels;
				int pix_sel;

				hval = (rgdb + gha) & 0xfff; // Not quite

				if (hval & 0x800)
					hval ^= 0x1ff; // TEST
				else
					hval = hval;

				pixels = apache3_g_ram[(((gva & 0xff) << 7) | ((hval >> 2) & 0x7f))];
				pix_sel = hval & 3;

				colour = (pixels >> (pix_sel << 1)) & 3;
				colour = (BIT(hval, 11) << 4) | (colour << 2) | ln;

				/* Draw the pixel */
				*BITMAP_ADDR32(dst, y, x) = machine->pens[0x200 + colour];

				/* Update horizontal counter */
				gha = (gha + 1) & 0xfff;
			}
		}

		/* Update sky counter */
		sky_val++;
		gva = (gva + 1) & 0x1ff;
	}
}
#endif
/**********************************************************************/

VIDEO_UPDATE( apache3 )
{
	update_cluts(screen->machine, 1024, 0, 2048);

	tilemap_set_scrollx(tx_layer,0,24);

	bitmap_fill(bitmap,cliprect,screen->machine->pens[0]);
	draw_sky(screen->machine, bitmap, cliprect, 256, apache3_rotate_ctrl[1]);
//  draw_ground(screen->machine, bitmap, cliprect);
	draw_sprites(screen->machine, bitmap,cliprect,0, (tatsumi_sprite_control_ram[0x20]&0x1000) ? 0x1000 : 0);
	tilemap_draw(bitmap,cliprect,tx_layer,0,0);
	return 0;
}

VIDEO_UPDATE( roundup5 )
{
//  UINT16 bg_x_scroll=roundup5_unknown1[0];
//  UINT16 bg_y_scroll=roundup5_unknown2[0];

	update_cluts(screen->machine, 1024, 512, 4096);

	tilemap_set_scrollx(tx_layer,0,24);
	tilemap_set_scrolly(tx_layer,0,0); //(((roundupt_crt_reg[0xe]<<8)|roundupt_crt_reg[0xf])>>5) + 96);

	bitmap_fill(bitmap,cliprect,screen->machine->pens[384]); // todo
	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	draw_sprites(screen->machine, screen->machine->priority_bitmap,cliprect,1,(tatsumi_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0); // Alpha pass only
	draw_road(screen->machine, bitmap,cliprect,screen->machine->priority_bitmap);
	draw_sprites(screen->machine, bitmap,cliprect,0,(tatsumi_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0); // Full pass
	tilemap_draw(bitmap,cliprect,tx_layer,0,0);
	return 0;
}

VIDEO_UPDATE( cyclwarr )
{
	bigfight_bank=bigfight_a40000[0];
	if (bigfight_bank!=bigfight_last_bank)
	{
		tilemap_mark_all_tiles_dirty(layer0);
		tilemap_mark_all_tiles_dirty(layer1);
		tilemap_mark_all_tiles_dirty(layer2);
		tilemap_mark_all_tiles_dirty(layer3);
		bigfight_last_bank=bigfight_bank;
	}

	bitmap_fill(bitmap,cliprect,screen->machine->pens[0]);

	draw_bg(screen->machine, bitmap, layer3, &cyclwarr_videoram1[0x000], &cyclwarr_videoram1[0x100], cyclwarr_videoram1, bigfight_a40000[0], 8, -0x80, 512, 4096);
	draw_bg(screen->machine, bitmap, layer2, &cyclwarr_videoram1[0x200], &cyclwarr_videoram1[0x300], cyclwarr_videoram1, bigfight_a40000[0], 8, -0x80, 512, 4096);
	draw_bg(screen->machine, bitmap, layer1, &cyclwarr_videoram0[0x000], &cyclwarr_videoram0[0x100], cyclwarr_videoram0, bigfight_a40000[0], 8, -0x40, 1024, 2048);
	update_cluts(screen->machine, 8192, 4096, 8192);
	draw_sprites(screen->machine, bitmap,cliprect,0,(tatsumi_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0);
	draw_bg(screen->machine, bitmap, layer0, &cyclwarr_videoram0[0x200], &cyclwarr_videoram0[0x300], cyclwarr_videoram0, bigfight_a40000[0], 0x10, -0x80, 512, 4096);

	return 0;
}

VIDEO_UPDATE( bigfight )
{
	bigfight_bank=bigfight_a40000[0];
	if (bigfight_bank!=bigfight_last_bank)
	{
		tilemap_mark_all_tiles_dirty(layer0);
		tilemap_mark_all_tiles_dirty(layer1);
		tilemap_mark_all_tiles_dirty(layer2);
		tilemap_mark_all_tiles_dirty(layer3);
		bigfight_last_bank=bigfight_bank;
	}

	bitmap_fill(bitmap,cliprect,screen->machine->pens[0]);
	draw_bg(screen->machine, bitmap, layer3, &cyclwarr_videoram1[0x000], &cyclwarr_videoram1[0x100], cyclwarr_videoram1, bigfight_a40000[0], 8, -0x40, 1024, 2048);
	draw_bg(screen->machine, bitmap, layer2, &cyclwarr_videoram1[0x200], &cyclwarr_videoram1[0x300], cyclwarr_videoram1, bigfight_a40000[0], 8, -0x40, 1024, 2048);
	draw_bg(screen->machine, bitmap, layer1, &cyclwarr_videoram0[0x000], &cyclwarr_videoram0[0x100], cyclwarr_videoram0, bigfight_a40000[0], 8, -0x40, 1024, 2048);
	update_cluts(screen->machine, 8192, 4096, 8192);
	draw_sprites(screen->machine, bitmap,cliprect,0,(tatsumi_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0);
	draw_bg(screen->machine, bitmap, layer0, &cyclwarr_videoram0[0x200], &cyclwarr_videoram0[0x300], cyclwarr_videoram0, bigfight_a40000[0], 0x10, -0x40, 1024, 2048);

	return 0;
}
