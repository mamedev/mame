/***************************************************************************

    System 16 / 18 bootleg video

    Bugs to check against real HW

    System16A Tilemap bootlegs

    - Shinobi (Datsu bootleg) has a black bar down the right hand side,
      which turns red on the high score table.  This is in the Text layer.
      According to other games this should be the correct alignment for
      this bootleg HW.

    - After inserting a coin in Passing Shot (2p version) the layers are
      misaligned by 1 pixel (happens on non-bootlegs too)

    - Causing a 'fault' in the Passing Shot 2p bootleg (not hitting the ball
      on your serve) causes the tilemaps to be erased and not updated
      properly (mirroring?, bootleg protection?, missed case in bootleg?)

    System18 Tilemap bootlegs

    - Alien Storm Credit text for Player 1 is not displayed after inserting
      a coin.

***************************************************************************/
#include "driver.h"
#include "system16.h"
#include "video/resnet.h"


UINT16* system16a_bootleg_bg0_tileram;
UINT16* system16a_bootleg_bg1_tileram;

UINT16 *sys16_tileram;
UINT16 *sys16_textram;
UINT16 *sys16_spriteram;

static int num_sprites;

#define MAXCOLOURS 0x2000 /* 8192 */

int sys16_sh_shadowpal;
int sys16_MaxShadowColors;

/* video driver constants (potentially different for each game) */
int sys16_gr_bitmap_width;
int (*sys16_spritesystem)( struct sys16_sprite_attributes *sprite, const UINT16 *source, int bJustGetColor );
const int *sys16_obj_bank;
int sys16_sprxoffset;
int sys16_bgxoffset;
int sys16_fgxoffset;
int sys16_textlayer_lo_min;
int sys16_textlayer_lo_max;
int sys16_textlayer_hi_min;
int sys16_textlayer_hi_max;
int sys16_bg1_trans; // alien syn + sys18
int sys16_bg_priority_mode;
int sys16_fg_priority_mode;
int sys16_bg_priority_value;
int sys16_fg_priority_value;
static int sys16_18_mode;
int sys16_tilebank_switch;
int sys16_rowscroll_scroll;
int shinobl_kludge;

/* video registers */
int sys16_tile_bank1;
int sys16_tile_bank0;
int sys16_refreshenable;

int sys16_bg_scrollx, sys16_bg_scrolly;
int sys16_bg2_scrollx, sys16_bg2_scrolly;
int sys16_fg_scrollx, sys16_fg_scrolly;
int sys16_fg2_scrollx, sys16_fg2_scrolly;

int sys16_bg_page[4];
int sys16_bg2_page[4];
int sys16_fg_page[4];
int sys16_fg2_page[4];

int sys18_bg2_active;
int sys18_fg2_active;
UINT16 *sys18_splittab_bg_x;
UINT16 *sys18_splittab_bg_y;
UINT16 *sys18_splittab_fg_x;
UINT16 *sys18_splittab_fg_y;

UINT16 *sys16_gr_ver;
UINT16 *sys16_gr_hor;
UINT16 *sys16_gr_pal;
UINT16 *sys16_gr_flip;
int sys16_gr_palette;
int sys16_gr_palette_default;
UINT8 sys16_gr_colorflip[2][4];
UINT16 *sys16_gr_second_road;

static tilemap *background, *foreground, *text_layer;
static tilemap *background2, *foreground2;
static int old_bg_page[4],old_fg_page[4], old_tile_bank1, old_tile_bank0;
static int old_bg2_page[4],old_fg2_page[4];


// doesn't use te page system
#if 0

//static UINT8 bg0_page[4];
//static UINT8 bg1_page[4];

static TILE_GET_INFO( get_system16b_bootleg_tile_info0 )
{
	/* TILEMAP_MAPPER can't be dynamic, so we do the page lookup here */
	int data, tile_number;
	int page = 0;

	if (tile_index & 0x0040)
		page+=1;

	if (tile_index & 0x1000)
		page+=2;

	tile_index = ((tile_index & 0x03f) >> 0) |
				 ((tile_index & 0xf80) >> 1);

	tile_index += bg0_page[page] * 0x800;

	data = sys16_tileram[tile_index];
	tile_number = data&0xfff;

	SET_TILE_INFO(
			0,
			tile_number,
			(data>>6)&0x7f,
			0);
}

static TILE_GET_INFO( get_system16b_bootleg_tile_info1 )
{
	/* TILEMAP_MAPPER can't be dynamic, so we do the page lookup here */
	int data, tile_number;
	int page = 0;

	if (tile_index & 0x0040)
		page+=1;

	if (tile_index & 0x1000)
		page+=2;

	tile_index = ((tile_index & 0x03f) >> 0) |
				 ((tile_index & 0xf80) >> 1);

	tile_index += bg1_page[page] * 0x800;

	data = sys16_tileram[tile_index];
	tile_number = data&0xfff;

	SET_TILE_INFO(
			0,
			tile_number,
			(data>>6)&0x7f,
			0);
}
#endif


/***************************************************************************/


/*
    We mark the priority buffer as follows:
        text    (0xf)
        fg (hi) (0x7)
        fg (lo) (0x3)
        bg (hi) (0x1)
        bg (lo) (0x0)

    Each sprite has 4 levels of priority, specifying where they are placed between bg(lo) and text.
*/

static void draw_sprite(running_machine *machine,
	bitmap_t *bitmap,
	const rectangle *cliprect,
	const UINT8 *addr, int pitch,
	pen_t pen_base,
	int x0, int y0, int screen_width, int screen_height,
	int width, int height,
	int flipx, int flipy,
	int priority,
	int shadow,
	int shadow_pen, int eos )
{
	pen_t shadow_pen_base = machine->gfx[0]->color_base + (machine->config->total_colors/2);
	const UINT8 *source;
	int full_shadow=shadow&SYS16_SPR_SHADOW;
	int partial_shadow=shadow&SYS16_SPR_PARTIAL_SHADOW;
	int shadow_mask=(machine->config->total_colors/2)-1;
	int sx, x, xcount;
	int sy, y, ycount = 0;
	int dx,dy;
	UINT16 *dest;
	UINT8 *pri;
	unsigned pen, data;

	priority = 1<<priority;

	if( flipy ){
		dy = -1;
		y0 += screen_height-1;
	}
	else {
		dy = 1;
	}

	if( flipx ){
		dx = -1;
		x0 += screen_width-1;
	}
	else {
		dx = 1;
	}

	if (!eos)
	{
		sy = y0;
		for( y=height; y; y-- ){
			ycount += screen_height;
			while( ycount>=height ){
				if( sy>=cliprect->min_y && sy<=cliprect->max_y ){
					source = addr;
					dest = BITMAP_ADDR16(bitmap, sy, 0);
					pri = BITMAP_ADDR8(priority_bitmap, sy, 0);
					sx = x0;
					xcount = 0;
					for( x=width; x; x-=2 ){
						data = (unsigned)*source++; /* next 2 pixels */
						pen = data>>4;
						xcount += screen_width;
						while( xcount>=width )
						{
							if( pen && pen!=0xf && sx>=cliprect->min_x && sx<=cliprect->max_x ){
								if(!(pri[sx]&priority)){
									if (full_shadow)
										dest[sx] = shadow_pen_base + (dest[sx]&shadow_mask);
									else if (partial_shadow && pen==shadow_pen)
										dest[sx] = shadow_pen_base + (dest[sx]&shadow_mask);
									else
										dest[sx] = pen_base + pen;
								}
							}
							xcount -= width;
							sx+=dx;
						}
						pen = data&0xf;
						xcount += screen_width;
						while( xcount>=width )
						{
							if( pen && pen!=0xf && sx>=cliprect->min_x && sx<=cliprect->max_x ){
								if(!(pri[sx]&priority)){
									if (full_shadow)
										dest[sx] = shadow_pen_base + (dest[sx]&shadow_mask);
									else if (partial_shadow && pen==shadow_pen)
										dest[sx] = shadow_pen_base + (dest[sx]&shadow_mask);
									else
										dest[sx] = pen_base + pen;
								}
							}
							xcount -= width;
							sx+=dx;
						}
					}
				}
				ycount -= height;
				sy+=dy;
			}
			addr += pitch;
		}
	}
	else
	{
		sy = y0;
		for( y=height; y; y-- ){
			ycount += screen_height;
			while( ycount>=height ){
				if( sy>=cliprect->min_y && sy<=cliprect->max_y ){
					source = addr;
					dest = BITMAP_ADDR16(bitmap, sy, 0);
					pri = BITMAP_ADDR8(priority_bitmap, sy, 0);
					sx = x0;
					xcount = 0;
					for( x=width; x; x-=2 ){
						data = (unsigned)*source++; /* next 2 pixels */
						pen = data>>4;
						if (pen==0xf) break;
						xcount += screen_width;
						while( xcount>=width )
						{
							if( pen && pen!=0xf && sx>=cliprect->min_x && sx<=cliprect->max_x )
								if(!(pri[sx]&priority)) dest[sx] = pen_base + pen;
							xcount -= width;
							sx+=dx;
						}
						pen = data&0xf;
						xcount += screen_width;
						while( xcount>=width )
						{
							if( pen && pen!=0xf && sx>=cliprect->min_x && sx<=cliprect->max_x )
								if(!(pri[sx]&priority)) dest[sx] = pen_base + pen;
							xcount -= width;
							sx+=dx;
						}
					}
				}
				ycount -= height;
				sy+=dy;
			}
			addr += pitch;
		}
	}
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int b3d ) //*
{
	pen_t pen_base = machine->gfx[0]->color_base;
	const UINT8 *base_gfx = memory_region(machine, "gfx2");
	const int gfx_rom_size = memory_region_length(machine, "gfx2");
	const UINT16 *source = sys16_spriteram;
	struct sys16_sprite_attributes sprite;
	int xpos, ypos, screen_width, width, logical_height, pitch, flipy, flipx;
	int i, mod_h, mod_x, eos;
	unsigned gfx;

	memset(&sprite, 0x00, sizeof(sprite));

	for(i=0; i<num_sprites; i++)
	{
		sprite.flags = 0;
		if (sys16_spritesystem(&sprite, source, 0)) return; /* end-of-spritelist */
		source += 8;

		if( sprite.flags & SYS16_SPR_VISIBLE )
		{
			xpos = sprite.x;
			flipx = sprite.flags & SYS16_SPR_FLIPX;
			ypos = sprite.y;
			pitch = sprite.pitch;
			flipy = pitch & 0x80;
			width = pitch & 0x7f;
			if (pitch & 0x80) width = 0x80 - width;
			pitch = width << 1;
			width <<= 2;
			eos = 0;

			if( b3d ) // outrun/aburner
			{
				if (b3d == 2) eos = 1;
				if (xpos < 0 && flipx) continue;
				if (ypos >= 240) ypos -= 256;
				sprite.screen_height++;
				logical_height = (sprite.screen_height<<4)*sprite.zoomy/0x2000;
				screen_width = width*0x200/sprite.zoomx;

				if (flipx && flipy) { mod_h = -logical_height;   mod_x = 4; }
				else if     (flipx) { mod_h = -1; xpos++;        mod_x = 4; }
				else if     (flipy) { mod_h = -logical_height;   mod_x = 0; }
				else                { mod_h = 0;                 mod_x = 0; }

				if( sprite.flags & SYS16_SPR_DRAW_TO_TOP )
				{
					ypos -= sprite.screen_height;
					flipy = !flipy;
				}

				if( sprite.flags & SYS16_SPR_DRAW_TO_LEFT )
				{
					xpos -= screen_width;
					flipx = !flipx;
				}
			}
			else
			{
				if (!width) { width = 512; eos = 1; } // used by fantasy zone for laser
				screen_width = width;
				logical_height = sprite.screen_height;

				if (sprite.zoomy) logical_height = logical_height*(0x400 + sprite.zoomy)/0x400 - 1;
				if (sprite.zoomx) screen_width = screen_width*(0x800 - sprite.zoomx)/0x800 + 2;

// fix, 5-bit zoom field
//              if (sprite.zoomy) logical_height = logical_height*(0x20 + sprite.zoomy)/0x20 - 1;
//              if (sprite.zoomx) screen_width = screen_width*(0x40 - sprite.zoomx)/0x40 + 2;

				if (flipx && flipy) { mod_h = -logical_height-1; mod_x = 2; }
				else if     (flipx) { mod_h = 0;                 mod_x = 2; }
				else if     (flipy) { mod_h = -logical_height;   mod_x = 0; }
				else                { mod_h = 1;                 mod_x = 0; }
			}

			gfx = sprite.gfx + pitch * mod_h + mod_x;
			if (gfx >= gfx_rom_size) gfx %= gfx_rom_size;

			gfx &= 0x1fffff; // temp kludge to stop line of fire crashing

			draw_sprite(machine,
				bitmap,cliprect,
				base_gfx + gfx, pitch,
				pen_base + (sprite.color<<4),
				xpos, ypos, screen_width, sprite.screen_height,
				width, logical_height,
				flipx, flipy,
				sprite.priority,
				sprite.flags,
				sprite.shadow_pen, eos);
		}
	}
}

/***************************************************************************/

static TILEMAP_MAPPER( sys16_bg_map )
{
	int page = 0;
	if( row<32 ){ /* top */
		if( col<64 ) page = 0; else page = 1;
	}
	else { /* bottom */
		if( col<64 ) page = 2; else page = 3;
	}
	row = row%32;
	col = col%64;
	return page*64*32+row*64+col;
}

static TILEMAP_MAPPER( sys16_text_map )
{
	return row*64+col+(64-40);
}

/***************************************************************************/

/*
    Color generation details

    Each color is made up of 5 bits, connected through one or more resistors like so:

    Bit 0 = 1 x 3.9K ohm
    Bit 1 = 1 x 2.0K ohm
    Bit 2 = 1 x 1.0K ohm
    Bit 3 = 2 x 1.0K ohm
    Bit 4 = 4 x 1.0K ohm

    Another data bit is connected by a tristate buffer to the color output through a 470 ohm resistor.
    The buffer allows the resistor to have no effect (tristate), halve brightness (pull-down) or double brightness (pull-up).
    The data bit source is a PPI pin in some of the earlier hardware (Hang-On, Pre-System 16) or bit 15 of each
    color RAM entry (Space Harrier, System 16B and most later boards).
*/

static const int resistances_normal[6] = {3900, 2000, 1000, 1000/2, 1000/4, 0};
static const int resistances_sh[6] = {3900, 2000, 1000, 1000/2, 1000/4, 470};
static double weights[2][3][6];

WRITE16_HANDLER( sys16_paletteram_w )
{
	UINT16 newword;
	COMBINE_DATA( &paletteram16[offset] );
	newword = paletteram16[offset];

	/*  sBGR BBBB GGGG RRRR */
	/*  x000 4321 4321 4321 */
	{
		int r, g, b, rs, gs, bs, rh, gh, bh;
		int r0 = (newword >> 12) & 1;
		int r1 = (newword >>  0) & 1;
		int r2 = (newword >>  1) & 1;
		int r3 = (newword >>  2) & 1;
		int r4 = (newword >>  3) & 1;
		int g0 = (newword >> 13) & 1;
		int g1 = (newword >>  4) & 1;
		int g2 = (newword >>  5) & 1;
		int g3 = (newword >>  6) & 1;
		int g4 = (newword >>  7) & 1;
		int b0 = (newword >> 14) & 1;
		int b1 = (newword >>  8) & 1;
		int b2 = (newword >>  9) & 1;
		int b3 = (newword >> 10) & 1;
		int b4 = (newword >> 11) & 1;

		/* Normal colors */
		r = combine_6_weights(weights[0][0], r0, r1, r2, r3, r4, 0);
		g = combine_6_weights(weights[0][1], g0, g1, g2, g3, g4, 0);
		b = combine_6_weights(weights[0][2], b0, b1, b2, b3, b4, 0);

		/* Shadow colors */
		rs = combine_6_weights(weights[1][0], r0, r1, r2, r3, r4, 0);
		gs = combine_6_weights(weights[1][1], g0, g1, g2, g3, g4, 0);
		bs = combine_6_weights(weights[1][2], b0, b1, b2, b3, b4, 0);

		/* Highlight colors */
		rh = combine_6_weights(weights[1][0], r0, r1, r2, r3, r4, 1);
		gh = combine_6_weights(weights[1][1], g0, g1, g2, g3, g4, 1);
		bh = combine_6_weights(weights[1][2], b0, b1, b2, b3, b4, 1);

		palette_set_color( space->machine, offset, MAKE_RGB(r, g, b) );

		palette_set_color( space->machine, offset+space->machine->config->total_colors/2,MAKE_RGB(rs,gs,bs));
	}
}


static void update_page( void ){
	int all_dirty = 0;
	int i,offset;
	if( old_tile_bank1 != sys16_tile_bank1 ){
		all_dirty = 1;
		old_tile_bank1 = sys16_tile_bank1;
	}
	if( old_tile_bank0 != sys16_tile_bank0 ){
		all_dirty = 1;
		old_tile_bank0 = sys16_tile_bank0;
		tilemap_mark_all_tiles_dirty( text_layer );
	}
	if( all_dirty ){
		tilemap_mark_all_tiles_dirty( background );
		tilemap_mark_all_tiles_dirty( foreground );
		if( sys16_18_mode ){
			tilemap_mark_all_tiles_dirty( background2 );
			tilemap_mark_all_tiles_dirty( foreground2 );
		}
	}
	else {
		for(i=0;i<4;i++){
			int page0 = 64*32*i;
			if( old_bg_page[i]!=sys16_bg_page[i] ){
				old_bg_page[i] = sys16_bg_page[i];
				for( offset = page0; offset<page0+64*32; offset++ ){
					tilemap_mark_tile_dirty( background, offset );
				}
			}
			if( old_fg_page[i]!=sys16_fg_page[i] ){
				old_fg_page[i] = sys16_fg_page[i];
				for( offset = page0; offset<page0+64*32; offset++ ){
					tilemap_mark_tile_dirty( foreground, offset );
				}
			}
			if( sys16_18_mode ){
				if( old_bg2_page[i]!=sys16_bg2_page[i] ){
					old_bg2_page[i] = sys16_bg2_page[i];
					for( offset = page0; offset<page0+64*32; offset++ ){
						tilemap_mark_tile_dirty( background2, offset );
					}
				}
				if( old_fg2_page[i]!=sys16_fg2_page[i] ){
					old_fg2_page[i] = sys16_fg2_page[i];
					for( offset = page0; offset<page0+64*32; offset++ ){
						tilemap_mark_tile_dirty( foreground2, offset );
					}
				}
			}
		}
	}
}

static TILE_GET_INFO( get_bg_tile_info ){
	const UINT16 *source = 64*32*sys16_bg_page[tile_index/(64*32)] + sys16_tileram;
	int data = source[tile_index%(64*32)];
	int tile_number = (data&0xfff) + 0x1000*((data&sys16_tilebank_switch)?sys16_tile_bank1:sys16_tile_bank0);

	if (!shinobl_kludge)
	{
		SET_TILE_INFO(
				0,
				tile_number,
				(data>>6)&0x7f,
				0);
	}
	else
	{
		SET_TILE_INFO(
				0,
				tile_number,
				(data>>5)&0x7f,
				0);
	}

	switch(sys16_bg_priority_mode) {
	case 1: // Alien Syndrome
		tileinfo->category = (data&0x8000)?1:0;
		break;
	case 2: // Body Slam / wrestwar
		tileinfo->category = ((data&0xff00) >= sys16_bg_priority_value)?1:0;
		break;
	case 3: // sys18 games
		if( data&0x8000 ){
			tileinfo->category = 2;
		}
		else {
			tileinfo->category = ((data&0xff00) >= sys16_bg_priority_value)?1:0;
		}
		break;
	}
}

static TILE_GET_INFO( get_fg_tile_info ){
	const UINT16 *source = 64*32*sys16_fg_page[tile_index/(64*32)] + sys16_tileram;
	int data = source[tile_index%(64*32)];
	int tile_number = (data&0xfff) + 0x1000*((data&sys16_tilebank_switch)?sys16_tile_bank1:sys16_tile_bank0);

	if (!shinobl_kludge)
	{
		SET_TILE_INFO(
				0,
				tile_number,
				(data>>6)&0x7f,
				0);
	}
	else
	{
		SET_TILE_INFO(
				0,
				tile_number,
				(data>>5)&0x7f,
				0);
	}

	switch(sys16_fg_priority_mode){
	case 1: // alien syndrome
		tileinfo->category = (data&0x8000)?1:0;
		break;

	case 3:
		tileinfo->category = ((data&0xff00) >= sys16_fg_priority_value)?1:0;
		break;

	default:
		if( sys16_fg_priority_mode>=0 ){
			tileinfo->category = (data&0x8000)?1:0;
		}
		break;
	}
}

static TILE_GET_INFO( get_bg2_tile_info ){
	const UINT16 *source = 64*32*sys16_bg2_page[tile_index/(64*32)] + sys16_tileram;
	int data = source[tile_index%(64*32)];
	int tile_number = (data&0xfff) + 0x1000*((data&0x1000)?sys16_tile_bank1:sys16_tile_bank0);

	SET_TILE_INFO(
			0,
			tile_number,
			(data>>6)&0x7f,
			0);

	tileinfo->category = 0;
}

static TILE_GET_INFO( get_fg2_tile_info ){
	const UINT16 *source = 64*32*sys16_fg2_page[tile_index/(64*32)] + sys16_tileram;
	int data = source[tile_index%(64*32)];
	int tile_number = (data&0xfff) + 0x1000*((data&0x1000)?sys16_tile_bank1:sys16_tile_bank0);

	SET_TILE_INFO(
			0,
			tile_number,
			(data>>6)&0x7f,
			0);

	if((data&0xff00) >= sys16_fg_priority_value) tileinfo->category = 1;
	else tileinfo->category = 0;
}

WRITE16_HANDLER( sys16_tileram_w ){
	UINT16 oldword = sys16_tileram[offset];
	COMBINE_DATA( &sys16_tileram[offset] );
	if( oldword != sys16_tileram[offset] ){
		int page = offset/(64*32);
		offset = offset%(64*32);

		if( sys16_bg_page[0]==page ) tilemap_mark_tile_dirty( background, offset+64*32*0 );
		if( sys16_bg_page[1]==page ) tilemap_mark_tile_dirty( background, offset+64*32*1 );
		if( sys16_bg_page[2]==page ) tilemap_mark_tile_dirty( background, offset+64*32*2 );
		if( sys16_bg_page[3]==page ) tilemap_mark_tile_dirty( background, offset+64*32*3 );

		if( sys16_fg_page[0]==page ) tilemap_mark_tile_dirty( foreground, offset+64*32*0 );
		if( sys16_fg_page[1]==page ) tilemap_mark_tile_dirty( foreground, offset+64*32*1 );
		if( sys16_fg_page[2]==page ) tilemap_mark_tile_dirty( foreground, offset+64*32*2 );
		if( sys16_fg_page[3]==page ) tilemap_mark_tile_dirty( foreground, offset+64*32*3 );

		if( sys16_18_mode ){
			if( sys16_bg2_page[0]==page ) tilemap_mark_tile_dirty( background2, offset+64*32*0 );
			if( sys16_bg2_page[1]==page ) tilemap_mark_tile_dirty( background2, offset+64*32*1 );
			if( sys16_bg2_page[2]==page ) tilemap_mark_tile_dirty( background2, offset+64*32*2 );
			if( sys16_bg2_page[3]==page ) tilemap_mark_tile_dirty( background2, offset+64*32*3 );

			if( sys16_fg2_page[0]==page ) tilemap_mark_tile_dirty( foreground2, offset+64*32*0 );
			if( sys16_fg2_page[1]==page ) tilemap_mark_tile_dirty( foreground2, offset+64*32*1 );
			if( sys16_fg2_page[2]==page ) tilemap_mark_tile_dirty( foreground2, offset+64*32*2 );
			if( sys16_fg2_page[3]==page ) tilemap_mark_tile_dirty( foreground2, offset+64*32*3 );
		}
	}
}

/***************************************************************************/

static TILE_GET_INFO( get_text_tile_info ){
	const UINT16 *source = sys16_textram;
	int tile_number = source[tile_index];
	int pri = tile_number >> 8;

	if (!shinobl_kludge)
	{
		SET_TILE_INFO(
				0,
				(tile_number&0x1ff) + sys16_tile_bank0 * 0x1000,
				(tile_number>>9)%8,
				0);
	}
	else
	{
		SET_TILE_INFO(
				0,
				(tile_number&0xff)  + sys16_tile_bank0 * 0x1000,
				(tile_number>>8)%8,
				0);
	}

	if(pri>=sys16_textlayer_lo_min && pri<=sys16_textlayer_lo_max)
		tileinfo->category = 1;
	if(pri>=sys16_textlayer_hi_min && pri<=sys16_textlayer_hi_max)
		tileinfo->category = 0;
}

WRITE16_HANDLER( sys16_textram_w ){
	COMBINE_DATA( &sys16_textram[offset] );
	tilemap_mark_tile_dirty( text_layer, offset );
}

/***************************************************************************/

VIDEO_START( system16 ){
	static const int bank_default[16] = {
		0x0,0x1,0x2,0x3,
		0x4,0x5,0x6,0x7,
		0x8,0x9,0xa,0xb,
		0xc,0xd,0xe,0xf
	};
	if (!sys16_obj_bank)
		sys16_obj_bank = bank_default;

	/* Normal colors */
	compute_resistor_weights(0, 255, -1.0,
		6, resistances_normal, weights[0][0], 0, 0,
		6, resistances_normal, weights[0][1], 0, 0,
		6, resistances_normal, weights[0][2], 0, 0
		);

	/* Shadow/Highlight colors */
	compute_resistor_weights(0, 255, -1.0,
		6, resistances_sh, weights[1][0], 0, 0,
		6, resistances_sh, weights[1][1], 0, 0,
		6, resistances_sh, weights[1][2], 0, 0
		);

	if( !sys16_bg1_trans )
		background = tilemap_create(machine,
			get_bg_tile_info,
			sys16_bg_map,

			8,8,
			64*2,32*2 );
	else
		background = tilemap_create(machine,
			get_bg_tile_info,
			sys16_bg_map,

			8,8,
			64*2,32*2 );

	foreground = tilemap_create(machine,
		get_fg_tile_info,
		sys16_bg_map,

		8,8,
		64*2,32*2 );

	text_layer = tilemap_create(machine,
		get_text_tile_info,
		sys16_text_map,

		8,8,
		40,28 );

	num_sprites = 128*2; /* only 128 for most games; aburner uses 256 */

	{
		if(sys16_bg1_trans) tilemap_set_transparent_pen( background, 0 );
		tilemap_set_transparent_pen( foreground, 0 );
		tilemap_set_transparent_pen( text_layer, 0 );

		sys16_tile_bank0 = 0;
		sys16_tile_bank1 = 1;

		sys16_fg_scrollx = 0;
		sys16_fg_scrolly = 0;

		sys16_bg_scrollx = 0;
		sys16_bg_scrolly = 0;

		sys16_refreshenable = 1;

		/* common defaults */
		sys16_spritesystem = sys16_sprite_shinobi;
		if (!sys16_sprxoffset)
			sys16_sprxoffset = -0xb8;
		sys16_bgxoffset = 0;
		sys16_bg_priority_mode=0;
		sys16_fg_priority_mode=0;
		sys16_tilebank_switch=0x1000;

		// Defaults for sys16 games
		sys16_textlayer_lo_min=0;
		sys16_textlayer_lo_max=0x7f;
		sys16_textlayer_hi_min=0x80;
		sys16_textlayer_hi_max=0xff;

		sys16_18_mode=0;
	}
}

VIDEO_START( system18old )
{
	sys16_bg1_trans=1;

	background2 = tilemap_create(machine,
		get_bg2_tile_info,
		sys16_bg_map,

		8,8,
		64*2,32*2 );

	foreground2 = tilemap_create(machine,
		get_fg2_tile_info,
		sys16_bg_map,

		8,8,
		64*2,32*2 );

	VIDEO_START_CALL(system16);

	tilemap_set_transparent_pen( foreground2, 0 );

	if(sys18_splittab_fg_x){
		tilemap_set_scroll_rows( foreground , 64 );
		tilemap_set_scroll_rows( foreground2 , 64 );
	}
	if(sys18_splittab_bg_x){
		tilemap_set_scroll_rows( background , 64 );
		tilemap_set_scroll_rows( background2 , 64 );
	}

	sys16_textlayer_lo_min=0;
	sys16_textlayer_lo_max=0x1f;
	sys16_textlayer_hi_min=0x20;
	sys16_textlayer_hi_max=0xff;

	sys16_18_mode=1;
	sys16_bg_priority_mode=3;
	sys16_fg_priority_mode=3;
	sys16_bg_priority_value=0x1800;
	sys16_fg_priority_value=0x2000;
}


/*****************************************************************************************
 System 16A bootleg video

 The System16A bootlegs have extra RAM for 2 tilemaps.  The game code copies data from
 the usual 'tilemap ram' area to this new RAM and sets scroll registers as appropriate
 using additional registers not present on the original System 16A hardware.

 For some unknown reason the 2p Passing Shot bootleg end up blanking this area at times,
 this could be an emulation flaw or a problem with the original bootlegs.  Inserting a
 coin at the incorrect time can also cause missing graphics on the initial entry screen.
 See note at top of driver

 Sprites:
  ToDo

*****************************************************************************************/

static tilemap *system16a_bootleg_bg_tilemaps[2];
static tilemap *system16a_bootleg_text_tilemap;
static UINT16 system16a_bootleg_tilemapselect;


static int system16a_bootleg_bgscrolly;
static int system16a_bootleg_bgscrollx;
static int system16a_bootleg_fgscrolly;
static int system16a_bootleg_fgscrollx;

static TILE_GET_INFO( get_system16a_bootleg_tile_infotxt )
{
	int data, tile_number;

	data = sys16_textram[tile_index];
	tile_number = data&0x1ff;




	SET_TILE_INFO(
			0,
			tile_number,
			((data>>9)&0x7),
			0);
}

static TILE_GET_INFO( get_system16a_bootleg_tile_info0 )
{
	int data, tile_number;

	data = system16a_bootleg_bg0_tileram[tile_index];
	tile_number = data&0x1fff;


	SET_TILE_INFO(
			0,
			tile_number,
			(data>>6)&0x7f,
			0);
}

static TILE_GET_INFO( get_system16a_bootleg_tile_info1 )
{
	int data, tile_number;

	data = system16a_bootleg_bg1_tileram[tile_index];
	tile_number = data&0x1fff;

	SET_TILE_INFO(
			0,
			tile_number,
			(data>>6)&0x7f,
			0);
}

WRITE16_HANDLER( system16a_bootleg_bgscrolly_w )
{
	system16a_bootleg_bgscrolly = data;
}

WRITE16_HANDLER( system16a_bootleg_bgscrollx_w )
{
	system16a_bootleg_bgscrollx = data;
}

WRITE16_HANDLER( system16a_bootleg_fgscrolly_w )
{
	system16a_bootleg_fgscrolly = data;
}

WRITE16_HANDLER( system16a_bootleg_fgscrollx_w )
{
	system16a_bootleg_fgscrollx = data;
}

WRITE16_HANDLER( system16a_bootleg_tilemapselect_w )
{
	COMBINE_DATA(&system16a_bootleg_tilemapselect);
	//printf("system16a_bootleg_tilemapselect %04x\n", system16a_bootleg_tilemapselect);
}



VIDEO_START( system16a_bootleg )
{
	/* Normal colors */
	compute_resistor_weights(0, 255, -1.0,
		6, resistances_normal, weights[0][0], 0, 0,
		6, resistances_normal, weights[0][1], 0, 0,
		6, resistances_normal, weights[0][2], 0, 0
		);

	/* Shadow/Highlight colors */
	compute_resistor_weights(0, 255, -1.0,
		6, resistances_sh, weights[1][0], 0, 0,
		6, resistances_sh, weights[1][1], 0, 0,
		6, resistances_sh, weights[1][2], 0, 0
		);



	system16a_bootleg_text_tilemap = tilemap_create(machine, get_system16a_bootleg_tile_infotxt, tilemap_scan_rows, 8,8, 64,32 );

	// the system16a bootlegs have simple tilemaps instead of the paged system
	system16a_bootleg_bg_tilemaps[0] = tilemap_create(machine, get_system16a_bootleg_tile_info0, tilemap_scan_rows, 8,8, 64,32 );
	system16a_bootleg_bg_tilemaps[1] = tilemap_create(machine, get_system16a_bootleg_tile_info1, tilemap_scan_rows, 8,8, 64,32 );

	tilemap_set_transparent_pen( system16a_bootleg_text_tilemap, 0 );
	tilemap_set_transparent_pen( system16a_bootleg_bg_tilemaps[0], 0 );
	tilemap_set_transparent_pen( system16a_bootleg_bg_tilemaps[1], 0 );


}

// Passing Shot (2 player), Shinobi (Datsu), Wonderboy 3
VIDEO_UPDATE( system16a_bootleg )
{
	int offset_txtx, offset_txty, offset_bg1x, offset_bg1y, offset_bg0x, offset_bg0y;

	// passing shot
	offset_txtx = 192;
	offset_txty = 0;
	offset_bg1x = 190;
	offset_bg1y = 0;
	offset_bg0x = 187;
	offset_bg0y = 0;

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	// I can't bring myself to care about dirty tile marking on something which runs at a bazillion % speed anyway, clean code is better
	tilemap_mark_all_tiles_dirty (system16a_bootleg_bg_tilemaps[0]);
	tilemap_mark_all_tiles_dirty (system16a_bootleg_bg_tilemaps[1]);
	tilemap_mark_all_tiles_dirty (system16a_bootleg_text_tilemap);

	tilemap_set_scrollx( system16a_bootleg_text_tilemap, 0, offset_txtx );
	tilemap_set_scrolly( system16a_bootleg_text_tilemap, 0, offset_txty );

	if ((system16a_bootleg_tilemapselect&0xff) == 0x12)
	{
		tilemap_set_scrollx( system16a_bootleg_bg_tilemaps[1], 0, system16a_bootleg_bgscrollx+offset_bg1x );
		tilemap_set_scrolly( system16a_bootleg_bg_tilemaps[1], 0, system16a_bootleg_bgscrolly+offset_bg1y );
		tilemap_set_scrollx( system16a_bootleg_bg_tilemaps[0], 0, system16a_bootleg_fgscrollx+offset_bg0x );
		tilemap_set_scrolly( system16a_bootleg_bg_tilemaps[0], 0, system16a_bootleg_fgscrolly+offset_bg0y );

		tilemap_draw( bitmap,cliprect, system16a_bootleg_bg_tilemaps[0], TILEMAP_DRAW_OPAQUE, 0 );
		tilemap_draw( bitmap,cliprect, system16a_bootleg_bg_tilemaps[1], 0, 0 );
		tilemap_draw( bitmap,cliprect, system16a_bootleg_text_tilemap, 0, 0 );
	}
	else if ((system16a_bootleg_tilemapselect&0xff) == 0x21)
	{
		tilemap_set_scrollx( system16a_bootleg_bg_tilemaps[0], 0, system16a_bootleg_bgscrollx+187 );
		tilemap_set_scrolly( system16a_bootleg_bg_tilemaps[0], 0, system16a_bootleg_bgscrolly );
		tilemap_set_scrollx( system16a_bootleg_bg_tilemaps[1], 0, system16a_bootleg_fgscrollx+187 );
		tilemap_set_scrolly( system16a_bootleg_bg_tilemaps[1], 0, system16a_bootleg_fgscrolly+1 );

		tilemap_draw( bitmap,cliprect, system16a_bootleg_bg_tilemaps[1], TILEMAP_DRAW_OPAQUE, 0 );
		tilemap_draw( bitmap,cliprect, system16a_bootleg_bg_tilemaps[0], 0, 0 );
		tilemap_draw( bitmap,cliprect, system16a_bootleg_text_tilemap, 0, 0 );
	}

	return 0;
}

/* The Passing Shot 4 Player bootleg has weird scroll registers (different offsets, ^0x7 xor) */
VIDEO_UPDATE( system16a_bootleg_passht4b )
{
	int offset_txtx, offset_txty, offset_bg1x, offset_bg1y, offset_bg0x, offset_bg0y;

	// passing shot
	offset_txtx = 192;
	offset_txty = 0;
	offset_bg1x = 3;
	offset_bg1y = 32;
	offset_bg0x = 5;
	offset_bg0y = 32;

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	// I can't bring myself to care about dirty tile marking on something which runs at a bazillion % speed anyway, clean code is better
	tilemap_mark_all_tiles_dirty (system16a_bootleg_bg_tilemaps[0]);
	tilemap_mark_all_tiles_dirty (system16a_bootleg_bg_tilemaps[1]);
	tilemap_mark_all_tiles_dirty (system16a_bootleg_text_tilemap);

	tilemap_set_scrollx( system16a_bootleg_text_tilemap, 0, offset_txtx );
	tilemap_set_scrolly( system16a_bootleg_text_tilemap, 0, offset_txty );

	if ((system16a_bootleg_tilemapselect&0xff) == 0x12)
	{
		tilemap_set_scrollx( system16a_bootleg_bg_tilemaps[1], 0, (system16a_bootleg_bgscrollx^0x7)+offset_bg1x );
		tilemap_set_scrolly( system16a_bootleg_bg_tilemaps[1], 0, system16a_bootleg_bgscrolly+offset_bg1y );
		tilemap_set_scrollx( system16a_bootleg_bg_tilemaps[0], 0, (system16a_bootleg_fgscrollx^0x7)+offset_bg0x );
		tilemap_set_scrolly( system16a_bootleg_bg_tilemaps[0], 0, system16a_bootleg_fgscrolly+offset_bg0y );

		tilemap_draw( bitmap,cliprect, system16a_bootleg_bg_tilemaps[0], TILEMAP_DRAW_OPAQUE, 0 );
		tilemap_draw( bitmap,cliprect, system16a_bootleg_bg_tilemaps[1], 0, 0 );
		tilemap_draw( bitmap,cliprect, system16a_bootleg_text_tilemap, 0, 0 );
	}

	return 0;
}


/***************************************************************************/

VIDEO_UPDATE( system16 )
{
	if (!sys16_refreshenable)
	{
		bitmap_fill(bitmap, cliprect, 0);
		return 0;
	}

	update_page();

	bitmap_fill(priority_bitmap,cliprect,0);

	// draw bg
	{
		tilemap_set_scrollx( background, 0, -320-sys16_bg_scrollx+sys16_bgxoffset );
		tilemap_set_scrolly( background, 0, -256+sys16_bg_scrolly );

		tilemap_draw( bitmap,cliprect, background, TILEMAP_DRAW_OPAQUE, 0x00 );

		if(sys16_bg_priority_mode)
		{
			tilemap_draw( bitmap,cliprect, background, TILEMAP_DRAW_OPAQUE | 1, 0x00 );
		}

		if( sys16_bg_priority_mode==2 )
		{
			tilemap_draw( bitmap,cliprect, background, 1, 0x01 );// body slam (& wrestwar??)
		}
		else if ( sys16_bg_priority_mode==1 )
		{
			tilemap_draw( bitmap,cliprect, background, 1, 0x03 );// alien syndrome / aurail
		}
	}

	// draw fg
	{
		tilemap_set_scrollx( foreground, 0, -320-sys16_fg_scrollx+sys16_fgxoffset );
		tilemap_set_scrolly( foreground, 0, -256+sys16_fg_scrolly );

		tilemap_draw( bitmap,cliprect, foreground, 0, 0x03 );
		tilemap_draw( bitmap,cliprect, foreground, 1, 0x07 );
	}


	if( sys16_textlayer_lo_max!=0 )
	{
		tilemap_draw( bitmap,cliprect, text_layer, 1, 7 );// needed for Body Slam
	}

	tilemap_draw( bitmap,cliprect, text_layer, 0, 0xf );

	draw_sprites(screen->machine, bitmap,cliprect,0 );
	return 0;
}


VIDEO_UPDATE( system18old )
{
	if (!sys16_refreshenable)
	{
		bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
		return 0;
	}

	update_page();

	bitmap_fill(priority_bitmap,NULL,0);
	if(sys18_bg2_active)
		tilemap_draw( bitmap,cliprect, background2, 0, 0 );
	else
		bitmap_fill(bitmap,cliprect,0);

	tilemap_draw( bitmap,cliprect, background, TILEMAP_DRAW_OPAQUE, 0 );
	tilemap_draw( bitmap,cliprect, background, TILEMAP_DRAW_OPAQUE | 1, 0 );	//??
	tilemap_draw( bitmap,cliprect, background, TILEMAP_DRAW_OPAQUE | 2, 0 );	//??
	tilemap_draw( bitmap,cliprect, background, 1, 0x1 );
	tilemap_draw( bitmap,cliprect, background, 2, 0x3 );

	if(sys18_fg2_active)
	{
		tilemap_draw( bitmap,cliprect, foreground2, 0, 0x3 );
	}

	tilemap_draw( bitmap,cliprect, foreground, 0, 0x3 );

	if(sys18_fg2_active)
	{
		tilemap_draw( bitmap,cliprect, foreground2, 1, 0x7 );
	}

	tilemap_draw( bitmap,cliprect, foreground, 1, 0x7 );

	tilemap_draw( bitmap,cliprect, text_layer, 1, 0x7 );
	tilemap_draw( bitmap,cliprect, text_layer, 0, 0xf );

	draw_sprites(screen->machine, bitmap,cliprect, 0 );
	return 0;
}
