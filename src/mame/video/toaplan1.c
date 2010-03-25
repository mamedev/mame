/***************************************************************************

  Functions to emulate the video hardware of some Toaplan games,
  which use the BCU-2 tile controller, and the FCU-2 Sprite controller -
  and SCU Sprite controller (Only Rally Bike uses the SCU controller).


  There are 4 scrolling layers of graphics, stored in planes of 64x64 tiles.
  Each tile in each plane is assigned a priority between 1 and 15, higher
  numbers have greater priority.

 BCU controller. Each tile takes up 32 bits - the format is:

  0         1         2         3
  ---- ---- ---- ---- -ttt tttt tttt tttt = Tile number (0 - $7fff)
  ---- ---- ---- ---- h--- ---- ---- ---- = Hidden
  ---- ---- --cc cccc ---- ---- ---- ---- = Color (0 - $3f)
  pppp ---- ---- ---- ---- ---- ---- ---- = Priority (0-$f)
  ---- ???? ??-- ---- ---- ---- ---- ---- = Unknown / Unused

  Scroll Reg

  0         1         2         3
  xxxx xxxx x--- ---- ---- ---- ---- ---- = X position
  ---- ---- ---- ---- yyyy yyyy y--- ---- = Y position
  ---- ---- -??? ???? ---- ---- -??? ???? = Unknown / Unused



 FCU controller. Sprite RAM format  (except Rally Bike)

  0         1         2         3
  -sss ssss ssss ssss ---- ---- ---- ---- = Sprite number (0 - $7fff)
  h--- ---- ---- ---- ---- ---- ---- ---- = Hidden
  ---- ---- ---- ---- ---- ---- --cc cccc = Color (0 - $3f)
  ---- ---- ---- ---- ---- dddd dd-- ---- = Dimension (pointer to Size RAM)
  ---- ---- ---- ---- pppp ---- ---- ---- = Priority (0-$f)

  4         5         6         7
  ---- ---- ---- ---- xxxx xxxx x--- ---- = X position
  yyyy yyyy y--- ---- ---- ---- ---- ---- = Y position
  ---- ---- -??? ???? ---- ---- -??? ???? = Unknown



 SCU controller. Sprite RAM format  (Rally Bike)

  0         1         2         3
  ---- -sss ssss ssss ---- ---- ---- ---- = Sprite number (0 - $7FF)
  ---- ---- ---- ---- ---- ---- --cc cccc = Color (0 - $3F)
  ---- ---- ---- ---- ---- ---x ---- ---- = Flip X
  ---- ---- ---- ---- ---- --y- ---- ---- = Flip Y
  ---- ---- ---- ---- ---- pp-- ---- ---- = Priority (0h,4h,8h,Ch (shifted < 2 places))
  ???? ?--- ---- ---- ???? ---- ??-- ---- = Unknown / Unused

  4         5         6         7
  xxxx xxxx x--- ---- ---- ---- ---- ---- = X position
  ---- ---- ---- ---- yyyy yyyy y--- ---- = Y position
  ---- ---- -??? ???? ---- ---- -??? ???? = Unknown



  The tiles use a palette of 1024 colors, the sprites use a different palette
  of 1024 colors.


           BCU Controller writes                Tile Offsets
 Game      reg0  reg1  reg2  reg3         X     Y     flip-X  flip-Y
RallyBik   41e0  2e1e  148c  0f09        01e6  00fc     <- same --
Truxton    41e0  2717  0e86  0c06        01b7  00f2     0188  01fd
HellFire   41e0  2717  0e86  0c06        01b7  0102     0188  000d
ZeroWing   41e0  2717  0e86  0c06        01b7  0102     0188  000d
DemonWld   41e0  2e1e  148c  0f09        01a9  00fc     0196  0013
FireShrk   41e0  2717  0e86  0c06        01b7  00f2     0188  01fd
Out-Zone   41e0  2e1e  148c  0f09        01a9  00ec     0196  0003
Vimana     41e0  2717  0e86  0c06        01b7  00f2     0188  01fd


Sprites are of varying sizes between 8x8 and 128x128 with any variation
in between, in multiples of 8 either way.
Here we draw the first 8x8 part of the sprite, then by using the sprite
dimensions, we draw the rest of the 8x8 parts to produce the complete
sprite.


Abnormalities:
 How/when do priority 0 Tile layers really get displayed ?

 What are the video PROMs for ? Priority maybe ?

 Zerowing flashes red when an enemy is shot, and this is done by flipping
 layer 2 into its oversized second half which is all red. In flipscreen
 mode, this doesn't work properly as the flip scroll value doesn't equate
 properly.
 Possibly a bug with the game itself using a wrong scroll value ??
 Here's some notes:
 First values are non red flash scrolls. Second values are red flash scrolls.

 Scrolls    PF1-X  PF1-Y    PF2-X  PF2-Y    PF3-X  PF3-Y    PF4-X  PF4-Y
 ------>    #4180  #f880    #1240  #f880    #4380  #f880    #e380  #f880
 -flip->    #1500  #7f00    #e8c0  #7f00    #1300  #7f00    #bb00  #7f00

 ------>    #4100  #f880    #1200  #7880    #4300  #f880    #e380  #f880
 -flip->    #1500  #7f00    #e8c0  #8580??  #1300  #7f00    #bb00  #7f00
                                      |
                                      |
                                    f880 = 111110001xxxxxxx -> 1f1 scroll
                                    7f00 = 011111110xxxxxxx -> 0fe scroll
                                    7880 = 011110001xxxxxxx -> 0f1 scroll
                                    8580 = 100001011xxxxxxx -> 10b scroll

 So a snapshot of the scroll equations become: (from the functions below)
    1f1 - (102 - 101) == 1f0   star background
    0fe - (00d - 1ef) == 0e0   star background (flipscreen)
    0f1 - (102 - 101) == 0f0   red  background
    10b - (00d - 1ef) == 0ed   red  background (flipscreen) wrong!
    10b - (00d - 0ef) == 1ed   red  background (flipscreen) should somehow equate to this


***************************************************************************/


#include "emu.h"
#include "includes/toaplan1.h"
#include "cpu/m68000/m68000.h"


#define TOAPLAN1_TILEVRAM_SIZE       0x4000	/* 4 tile layers each this RAM size */
#define TOAPLAN1_SPRITERAM_SIZE      0x800	/* sprite ram */
#define TOAPLAN1_SPRITESIZERAM_SIZE  0x80	/* sprite size ram */

static UINT16 *pf4_tilevram16;	/*  ||  Drawn in this order */
static UINT16 *pf3_tilevram16;	/*  ||  */
static UINT16 *pf2_tilevram16;	/* \||/ */
static UINT16 *pf1_tilevram16;	/*  \/  */

static UINT16 *toaplan1_spritesizeram16;
static UINT16 *toaplan1_buffered_spritesizeram16;

size_t toaplan1_colorram1_size;
size_t toaplan1_colorram2_size;
UINT16 *toaplan1_colorram1;
UINT16 *toaplan1_colorram2;

static INT32 bcu_flipscreen;		/* Tile   controller flip flag */
static INT32 fcu_flipscreen;		/* Sprite controller flip flag */

static INT32 pf_voffs;
static INT32 spriteram_offs;

static INT32 pf1_scrollx;
static INT32 pf1_scrolly;
static INT32 pf2_scrollx;
static INT32 pf2_scrolly;
static INT32 pf3_scrollx;
static INT32 pf3_scrolly;
static INT32 pf4_scrollx;
static INT32 pf4_scrolly;
static INT32 scrollx_offs1;
static INT32 scrollx_offs2;
static INT32 scrollx_offs3;
static INT32 scrollx_offs4;
static INT32 scrolly_offs;


#ifdef MAME_DEBUG
static int display_pf1;
static int display_pf2;
static int display_pf3;
static int display_pf4;
static int displog;
#endif

static INT32 tiles_offsetx;
static INT32 tiles_offsety;

static int toaplan1_reset;		/* Hack! See toaplan1_bcu_control below */

static tilemap_t *pf1_tilemap, *pf2_tilemap, *pf3_tilemap, *pf4_tilemap;


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_pf1_tile_info )
{
	int color, tile_number, attrib;

	tile_number = pf1_tilevram16[2*tile_index+1] & 0x7fff;
	attrib = pf1_tilevram16[2*tile_index];
	color = attrib & 0x3f;
	SET_TILE_INFO(
			0,
			tile_number,
			color,
			0);
	if (pf1_tilevram16[2*tile_index+1] & 0x8000) tileinfo->category = 0;
	else tileinfo->category = (attrib & 0xf000) >> 12;
}

static TILE_GET_INFO( get_pf2_tile_info )
{
	int color, tile_number, attrib;

	tile_number = pf2_tilevram16[2*tile_index+1] & 0x7fff;
	attrib = pf2_tilevram16[2*tile_index];
	color = attrib & 0x3f;
	SET_TILE_INFO(
			0,
			tile_number,
			color,
			0);
	if (pf2_tilevram16[2*tile_index+1] & 0x8000) tileinfo->category = 0;
	else tileinfo->category = (attrib & 0xf000) >> 12;
}

static TILE_GET_INFO( get_pf3_tile_info )
{
	int color, tile_number, attrib;

	tile_number = pf3_tilevram16[2*tile_index+1] & 0x7fff;
	attrib = pf3_tilevram16[2*tile_index];
	color = attrib & 0x3f;
	SET_TILE_INFO(
			0,
			tile_number,
			color,
			0);
	if (pf3_tilevram16[2*tile_index+1] & 0x8000) tileinfo->category = 0;
	else tileinfo->category = (attrib & 0xf000) >> 12;
}

static TILE_GET_INFO( get_pf4_tile_info )
{
	int color, tile_number, attrib;

	tile_number = pf4_tilevram16[2*tile_index+1] & 0x7fff;
	attrib = pf4_tilevram16[2*tile_index];
	color = attrib & 0x3f;
	SET_TILE_INFO(
			0,
			tile_number,
			color,
			0);
	if (pf4_tilevram16[2*tile_index+1] & 0x8000) tileinfo->category = 0;
	else tileinfo->category = (attrib & 0xf000) >> 12;
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static void toaplan1_create_tilemaps(running_machine *machine)
{
	pf1_tilemap = tilemap_create(machine, get_pf1_tile_info,tilemap_scan_rows,8,8,64,64);
	pf2_tilemap = tilemap_create(machine, get_pf2_tile_info,tilemap_scan_rows,8,8,64,64);
	pf3_tilemap = tilemap_create(machine, get_pf3_tile_info,tilemap_scan_rows,8,8,64,64);
	pf4_tilemap = tilemap_create(machine, get_pf4_tile_info,tilemap_scan_rows,8,8,64,64);

	tilemap_set_transparent_pen(pf1_tilemap,0);
	tilemap_set_transparent_pen(pf2_tilemap,0);
	tilemap_set_transparent_pen(pf3_tilemap,0);
	tilemap_set_transparent_pen(pf4_tilemap,0);
}


static void toaplan1_paletteram_alloc(running_machine *machine)
{
	machine->generic.paletteram.u16 = auto_alloc_array(machine, UINT16, (toaplan1_colorram1_size + toaplan1_colorram2_size)/2);
}

static void toaplan1_vram_alloc(running_machine *machine)
{
	pf1_tilevram16 = auto_alloc_array_clear(machine, UINT16, TOAPLAN1_TILEVRAM_SIZE/2);
	pf2_tilevram16 = auto_alloc_array_clear(machine, UINT16, TOAPLAN1_TILEVRAM_SIZE/2);
	pf3_tilevram16 = auto_alloc_array_clear(machine, UINT16, TOAPLAN1_TILEVRAM_SIZE/2);
	pf4_tilevram16 = auto_alloc_array_clear(machine, UINT16, TOAPLAN1_TILEVRAM_SIZE/2);

#ifdef MAME_DEBUG
	display_pf1 = 1;
	display_pf2 = 1;
	display_pf3 = 1;
	display_pf4 = 1;
	displog = 0;
#endif
}

static void toaplan1_spritevram_alloc(running_machine *machine)
{
	machine->generic.spriteram.u16 = auto_alloc_array_clear(machine, UINT16, TOAPLAN1_SPRITERAM_SIZE/2);
	machine->generic.buffered_spriteram.u16 = auto_alloc_array_clear(machine, UINT16, TOAPLAN1_SPRITERAM_SIZE/2);
	toaplan1_spritesizeram16 = auto_alloc_array_clear(machine, UINT16, TOAPLAN1_SPRITESIZERAM_SIZE/2);
	toaplan1_buffered_spritesizeram16 = auto_alloc_array_clear(machine, UINT16, TOAPLAN1_SPRITESIZERAM_SIZE/2);

	machine->generic.spriteram_size = TOAPLAN1_SPRITERAM_SIZE;
}

static void toaplan1_set_scrolls(void)
{
	tilemap_set_scrollx(pf1_tilemap,0,(pf1_scrollx >> 7) - (tiles_offsetx - scrollx_offs1));
	tilemap_set_scrollx(pf2_tilemap,0,(pf2_scrollx >> 7) - (tiles_offsetx - scrollx_offs2));
	tilemap_set_scrollx(pf3_tilemap,0,(pf3_scrollx >> 7) - (tiles_offsetx - scrollx_offs3));
	tilemap_set_scrollx(pf4_tilemap,0,(pf4_scrollx >> 7) - (tiles_offsetx - scrollx_offs4));
	tilemap_set_scrolly(pf1_tilemap,0,(pf1_scrolly >> 7) - (tiles_offsety - scrolly_offs));
	tilemap_set_scrolly(pf2_tilemap,0,(pf2_scrolly >> 7) - (tiles_offsety - scrolly_offs));
	tilemap_set_scrolly(pf3_tilemap,0,(pf3_scrolly >> 7) - (tiles_offsety - scrolly_offs));
	tilemap_set_scrolly(pf4_tilemap,0,(pf4_scrolly >> 7) - (tiles_offsety - scrolly_offs));
}

static STATE_POSTLOAD( rallybik_flipscreen )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	rallybik_bcu_flipscreen_w(space, 0, bcu_flipscreen, 0xffff);
}

static STATE_POSTLOAD( toaplan1_flipscreen )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	toaplan1_bcu_flipscreen_w(space, 0, bcu_flipscreen, 0xffff);
}


VIDEO_START( rallybik )
{
	toaplan1_create_tilemaps(machine);
	toaplan1_paletteram_alloc(machine);
	toaplan1_vram_alloc(machine);

	scrollx_offs1 = 0x00d + 6;
	scrollx_offs2 = 0x00d + 4;
	scrollx_offs3 = 0x00d + 2;
	scrollx_offs4 = 0x00d + 0;
	scrolly_offs  = 0x111;

	bcu_flipscreen = -1;
	toaplan1_reset = 0;

	state_save_register_global_pointer(machine, machine->generic.paletteram.u16, (toaplan1_colorram1_size + toaplan1_colorram2_size)/2);
	state_save_register_global_pointer(machine, pf1_tilevram16, TOAPLAN1_TILEVRAM_SIZE/2);
	state_save_register_global_pointer(machine, pf2_tilevram16, TOAPLAN1_TILEVRAM_SIZE/2);
	state_save_register_global_pointer(machine, pf3_tilevram16, TOAPLAN1_TILEVRAM_SIZE/2);
	state_save_register_global_pointer(machine, pf4_tilevram16, TOAPLAN1_TILEVRAM_SIZE/2);

	state_save_register_global(machine, scrollx_offs1);
	state_save_register_global(machine, scrollx_offs2);
	state_save_register_global(machine, scrollx_offs3);
	state_save_register_global(machine, scrollx_offs4);
	state_save_register_global(machine, scrolly_offs);
	state_save_register_global(machine, bcu_flipscreen);
	state_save_register_global(machine, pf1_scrollx);
	state_save_register_global(machine, pf1_scrolly);
	state_save_register_global(machine, pf2_scrollx);
	state_save_register_global(machine, pf2_scrolly);
	state_save_register_global(machine, pf3_scrollx);
	state_save_register_global(machine, pf3_scrolly);
	state_save_register_global(machine, pf4_scrollx);
	state_save_register_global(machine, pf4_scrolly);
	state_save_register_global(machine, tiles_offsetx);
	state_save_register_global(machine, tiles_offsety);
	state_save_register_global(machine, pf_voffs);
	state_save_register_global(machine, spriteram_offs);

	state_save_register_postload(machine, rallybik_flipscreen, NULL);
}

VIDEO_START( toaplan1 )
{
	toaplan1_create_tilemaps(machine);
	toaplan1_paletteram_alloc(machine);
	toaplan1_vram_alloc(machine);
	toaplan1_spritevram_alloc(machine);

	scrollx_offs1 = 0x1ef + 6;
	scrollx_offs2 = 0x1ef + 4;
	scrollx_offs3 = 0x1ef + 2;
	scrollx_offs4 = 0x1ef + 0;
	scrolly_offs  = 0x101;

	bcu_flipscreen = -1;
	fcu_flipscreen = 0;
	toaplan1_reset = 1;

	state_save_register_global_pointer(machine, machine->generic.paletteram.u16, (toaplan1_colorram1_size + toaplan1_colorram2_size)/2);
	state_save_register_global_pointer(machine, pf1_tilevram16, TOAPLAN1_TILEVRAM_SIZE/2);
	state_save_register_global_pointer(machine, pf2_tilevram16, TOAPLAN1_TILEVRAM_SIZE/2);
	state_save_register_global_pointer(machine, pf3_tilevram16, TOAPLAN1_TILEVRAM_SIZE/2);
	state_save_register_global_pointer(machine, pf4_tilevram16, TOAPLAN1_TILEVRAM_SIZE/2);
	state_save_register_global_pointer(machine, machine->generic.spriteram.u16, TOAPLAN1_SPRITERAM_SIZE/2);
	state_save_register_global_pointer(machine, machine->generic.buffered_spriteram.u16, TOAPLAN1_SPRITERAM_SIZE/2);
	state_save_register_global_pointer(machine, toaplan1_spritesizeram16, TOAPLAN1_SPRITESIZERAM_SIZE/2);
	state_save_register_global_pointer(machine, toaplan1_buffered_spritesizeram16, TOAPLAN1_SPRITESIZERAM_SIZE/2);

	state_save_register_global(machine, scrollx_offs1);
	state_save_register_global(machine, scrollx_offs2);
	state_save_register_global(machine, scrollx_offs3);
	state_save_register_global(machine, scrollx_offs4);
	state_save_register_global(machine, scrolly_offs);
	state_save_register_global(machine, bcu_flipscreen);
	state_save_register_global(machine, fcu_flipscreen);
	state_save_register_global(machine, pf1_scrollx);
	state_save_register_global(machine, pf1_scrolly);
	state_save_register_global(machine, pf2_scrolly);
	state_save_register_global(machine, pf2_scrollx);
	state_save_register_global(machine, pf3_scrollx);
	state_save_register_global(machine, pf3_scrolly);
	state_save_register_global(machine, pf4_scrollx);
	state_save_register_global(machine, pf4_scrolly);
	state_save_register_global(machine, tiles_offsetx);
	state_save_register_global(machine, tiles_offsety);
	state_save_register_global(machine, pf_voffs);
	state_save_register_global(machine, spriteram_offs);

	state_save_register_postload(machine, toaplan1_flipscreen, NULL);
}


/***************************************************************************

  Video I/O port hardware.

***************************************************************************/

READ16_HANDLER( toaplan1_frame_done_r )
{
	return video_screen_get_vblank(space->machine->primary_screen);
}

WRITE16_HANDLER( toaplan1_tile_offsets_w )
{
	if ( offset == 0 )
	{
		COMBINE_DATA(&tiles_offsetx);
		logerror("Tiles_offsetx now = %08x\n",tiles_offsetx);
	}
	else
	{
		COMBINE_DATA(&tiles_offsety);
		logerror("Tiles_offsety now = %08x\n",tiles_offsety);
	}
	toaplan1_reset = 1;
	toaplan1_set_scrolls();
}

WRITE16_HANDLER( rallybik_bcu_flipscreen_w )
{
	if (ACCESSING_BITS_0_7 && (data != bcu_flipscreen))
	{
		logerror("Setting BCU controller flipscreen port to %04x\n",data);
		bcu_flipscreen = data & 0x01;		/* 0x0001 = flip, 0x0000 = no flip */
		tilemap_set_flip_all(space->machine, (data ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0));
		if (bcu_flipscreen)
		{
			scrollx_offs1 = 0x1c0 - 6;
			scrollx_offs2 = 0x1c0 - 4;
			scrollx_offs3 = 0x1c0 - 2;
			scrollx_offs4 = 0x1c0 - 0;
			scrolly_offs  = 0x0e8;
		}
		else
		{
			scrollx_offs1 = 0x00d + 6;
			scrollx_offs2 = 0x00d + 4;
			scrollx_offs3 = 0x00d + 2;
			scrollx_offs4 = 0x00d + 0;
			scrolly_offs  = 0x111;
		}
		toaplan1_set_scrolls();
	}
}

WRITE16_HANDLER( toaplan1_bcu_flipscreen_w )
{
	if (ACCESSING_BITS_0_7 && (data != bcu_flipscreen))
	{
		logerror("Setting BCU controller flipscreen port to %04x\n",data);
		bcu_flipscreen = data & 0x01;		/* 0x0001 = flip, 0x0000 = no flip */
		tilemap_set_flip_all(space->machine, (data ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0));
		if (bcu_flipscreen)
		{
			const rectangle *visarea = video_screen_get_visible_area(space->machine->primary_screen);

			scrollx_offs1 = 0x151 - 6;
			scrollx_offs2 = 0x151 - 4;
			scrollx_offs3 = 0x151 - 2;
			scrollx_offs4 = 0x151 - 0;
			scrolly_offs  = 0x1ef;
			scrolly_offs += ((visarea->max_y + 1) - ((visarea->max_y + 1) - visarea->min_y)) * 2;	/* Horizontal games are offset so adjust by +0x20 */
		}
		else
		{
			scrollx_offs1 = 0x1ef + 6;
			scrollx_offs2 = 0x1ef + 4;
			scrollx_offs3 = 0x1ef + 2;
			scrollx_offs4 = 0x1ef + 0;
			scrolly_offs  = 0x101;
		}
		toaplan1_set_scrolls();
	}
}

WRITE16_HANDLER( toaplan1_fcu_flipscreen_w )
{
	if (ACCESSING_BITS_8_15)
	{
		logerror("Setting FCU controller flipscreen port to %04x\n",data);
		fcu_flipscreen = data & 0x8000;	/* 0x8000 = flip, 0x0000 = no flip */
	}
}

READ16_HANDLER( toaplan1_spriteram_offs_r ) /// this aint really needed ?
{
	return spriteram_offs;
}

WRITE16_HANDLER( toaplan1_spriteram_offs_w )
{
	COMBINE_DATA(&spriteram_offs);
}


/* tile palette */
READ16_HANDLER( toaplan1_colorram1_r )
{
	return toaplan1_colorram1[offset];
}

WRITE16_HANDLER( toaplan1_colorram1_w )
{
	COMBINE_DATA(&toaplan1_colorram1[offset]);
	paletteram16_xBBBBBGGGGGRRRRR_word_w(space,offset, data, mem_mask);
}

/* sprite palette */
READ16_HANDLER( toaplan1_colorram2_r )
{
	return toaplan1_colorram2[offset];
}

WRITE16_HANDLER( toaplan1_colorram2_w )
{
	COMBINE_DATA(&toaplan1_colorram2[offset]);
	paletteram16_xBBBBBGGGGGRRRRR_word_w(space,offset+(toaplan1_colorram1_size/2), data, mem_mask);
}

READ16_HANDLER( toaplan1_spriteram16_r )
{
	return space->machine->generic.spriteram.u16[spriteram_offs & ((TOAPLAN1_SPRITERAM_SIZE/2)-1)];
}

WRITE16_HANDLER( toaplan1_spriteram16_w )
{
	COMBINE_DATA(&space->machine->generic.spriteram.u16[spriteram_offs & ((TOAPLAN1_SPRITERAM_SIZE/2)-1)]);

#ifdef MAME_DEBUG
	if (spriteram_offs >= (TOAPLAN1_SPRITERAM_SIZE/2))
	{
		logerror("Sprite_RAM_word_w, %08x out of range !\n", spriteram_offs);
		return;
	}
#endif

	spriteram_offs++;
}

READ16_HANDLER( toaplan1_spritesizeram16_r )
{
	return toaplan1_spritesizeram16[spriteram_offs & ((TOAPLAN1_SPRITESIZERAM_SIZE/2)-1)];
}

WRITE16_HANDLER( toaplan1_spritesizeram16_w )
{
	COMBINE_DATA(&toaplan1_spritesizeram16[spriteram_offs & ((TOAPLAN1_SPRITESIZERAM_SIZE/2)-1)]);

#ifdef MAME_DEBUG
	if (spriteram_offs >= (TOAPLAN1_SPRITESIZERAM_SIZE/2))
	{
		logerror("Sprite_Size_RAM_word_w, %08x out of range !\n", spriteram_offs);
		return;
	}
#endif

	spriteram_offs++;	/// really ? shouldn't happen on the sizeram
}



WRITE16_HANDLER( toaplan1_bcu_control_w )
{
	logerror("BCU tile controller register:%02x now = %04x\n",offset,data);

	/*** Hack for Zero Wing and OutZone, to reset the sound system on */
	/*** soft resets. These two games don't have a sound reset port,  */
	/*** unlike the other games */

	if (toaplan1_unk_reset_port && toaplan1_reset)
	{
		toaplan1_reset = 0;
		toaplan1_reset_sound(space,0,0,0);
	}
}

READ16_HANDLER( toaplan1_tileram_offs_r )
{
	return pf_voffs;
}

WRITE16_HANDLER( toaplan1_tileram_offs_w )
{
	if (data >= 0x4000)
		logerror("Hmmm, unknown video layer being selected (%08x)\n",data);
	COMBINE_DATA(&pf_voffs);
}


READ16_HANDLER( toaplan1_tileram16_r )
{
	offs_t vram_offset;
	UINT16 video_data = 0;

	switch (pf_voffs & 0xf000)	/* Locate Layer (PlayField) */
	{
		case 0x0000:
				vram_offset = ((pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				video_data = pf1_tilevram16[vram_offset];
				break;
		case 0x1000:
				vram_offset = ((pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				video_data = pf2_tilevram16[vram_offset];
				break;
		case 0x2000:
				vram_offset = ((pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				video_data = pf3_tilevram16[vram_offset];
				break;
		case 0x3000:
				vram_offset = ((pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				video_data = pf4_tilevram16[vram_offset];
				break;
		default:
				logerror("Hmmm, reading %04x from unknown playfield layer address %06x  Offset:%01x !!!\n",video_data,pf_voffs,offset);
				break;
	}

	return video_data;
}

READ16_HANDLER( rallybik_tileram16_r )
{
	UINT16 data = toaplan1_tileram16_r(space, offset, mem_mask);

	if (offset == 0)	/* some bit lines may be stuck to others */
	{
		data |= ((data & 0xf000) >> 4);
		data |= ((data & 0x0030) << 2);
	}
	return data;
}

WRITE16_HANDLER( toaplan1_tileram16_w )
{
	offs_t vram_offset;

	switch (pf_voffs & 0xf000)	/* Locate Layer (PlayField) */
	{
		case 0x0000:
				vram_offset = ((pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				COMBINE_DATA(&pf1_tilevram16[vram_offset]);
				tilemap_mark_tile_dirty(pf1_tilemap,vram_offset/2);
				break;
		case 0x1000:
				vram_offset = ((pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				COMBINE_DATA(&pf2_tilevram16[vram_offset]);
				tilemap_mark_tile_dirty(pf2_tilemap,vram_offset/2);
				break;
		case 0x2000:
				vram_offset = ((pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				COMBINE_DATA(&pf3_tilevram16[vram_offset]);
				tilemap_mark_tile_dirty(pf3_tilemap,vram_offset/2);
				break;
		case 0x3000:
				vram_offset = ((pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				COMBINE_DATA(&pf4_tilevram16[vram_offset]);
				tilemap_mark_tile_dirty(pf4_tilemap,vram_offset/2);
				break;
		default:
				logerror("Hmmm, writing %04x to unknown playfield layer address %06x  Offset:%01x\n",data,pf_voffs,offset);
				break;
	}
}



READ16_HANDLER( toaplan1_scroll_regs_r )
{
	UINT16 scroll = 0;

	switch(offset)
	{
		case 00: scroll = pf1_scrollx; break;
		case 01: scroll = pf1_scrolly; break;
		case 02: scroll = pf2_scrollx; break;
		case 03: scroll = pf2_scrolly; break;
		case 04: scroll = pf3_scrollx; break;
		case 05: scroll = pf3_scrolly; break;
		case 06: scroll = pf4_scrollx; break;
		case 07: scroll = pf4_scrolly; break;
		default: logerror("Hmmm, reading unknown video scroll register (%08x) !!!\n",offset);
				 break;
	}
	return scroll;
}


WRITE16_HANDLER( toaplan1_scroll_regs_w )
{
	switch(offset)
	{
		case 00: COMBINE_DATA(&pf1_scrollx);		/* 1D3h */
				 tilemap_set_scrollx(pf1_tilemap,0,(pf1_scrollx >> 7) - (tiles_offsetx - scrollx_offs1));
				 break;
		case 01: COMBINE_DATA(&pf1_scrolly);		/* 1EBh */
				 tilemap_set_scrolly(pf1_tilemap,0,(pf1_scrolly >> 7) - (tiles_offsety - scrolly_offs));
				 break;
		case 02: COMBINE_DATA(&pf2_scrollx);		/* 1D5h */
				 tilemap_set_scrollx(pf2_tilemap,0,(pf2_scrollx >> 7) - (tiles_offsetx - scrollx_offs2));
				 break;
		case 03: COMBINE_DATA(&pf2_scrolly);		/* 1EBh */
				 tilemap_set_scrolly(pf2_tilemap,0,(pf2_scrolly >> 7) - (tiles_offsety - scrolly_offs));
				 break;
		case 04: COMBINE_DATA(&pf3_scrollx);		/* 1D7h */
				 tilemap_set_scrollx(pf3_tilemap,0,(pf3_scrollx >> 7) - (tiles_offsetx - scrollx_offs3));
				 break;
		case 05: COMBINE_DATA(&pf3_scrolly);		/* 1EBh */
				 tilemap_set_scrolly(pf3_tilemap,0,(pf3_scrolly >> 7) - (tiles_offsety - scrolly_offs));
				 break;
		case 06: COMBINE_DATA(&pf4_scrollx);		/* 1D9h */
				 tilemap_set_scrollx(pf4_tilemap,0,(pf4_scrollx >> 7) - (tiles_offsetx - scrollx_offs4));
				 break;
		case 07: COMBINE_DATA(&pf4_scrolly);		/* 1EBh */
				 tilemap_set_scrolly(pf4_tilemap,0,(pf4_scrolly >> 7) - (tiles_offsety - scrolly_offs));
				 break;
		default: logerror("Hmmm, writing %08x to unknown video scroll register (%08x) !!!\n",data ,offset);
				 break;
	}
}




static void toaplan1_log_vram(running_machine *machine)
{
#ifdef MAME_DEBUG
	if ( input_code_pressed(machine, KEYCODE_M) )
	{
		UINT16 *spriteram16 = machine->generic.spriteram.u16;
		UINT16 *buffered_spriteram16 = machine->generic.buffered_spriteram.u16;
		offs_t sprite_voffs;
		while (input_code_pressed(machine, KEYCODE_M)) ;
		if (toaplan1_spritesizeram16)			/* FCU controller */
		{
			int schar,sattr,sxpos,sypos,bschar,bsattr,bsxpos,bsypos;
			UINT16 *size  = (UINT16 *)(toaplan1_spritesizeram16);
			UINT16 *bsize = (UINT16 *)(toaplan1_buffered_spritesizeram16);
			logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
			logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",pf1_scrollx,pf1_scrolly,pf2_scrollx,pf2_scrolly,pf3_scrollx,pf3_scrolly,pf4_scrollx,pf4_scrolly);
			for ( sprite_voffs = 0; sprite_voffs < (machine->generic.spriteram_size/2); sprite_voffs += 4 )
			{
				bschar = buffered_spriteram16[sprite_voffs];
				bsattr = buffered_spriteram16[sprite_voffs + 1];
				bsxpos = buffered_spriteram16[sprite_voffs + 2];
				bsypos = buffered_spriteram16[sprite_voffs + 3];
				schar = spriteram16[sprite_voffs];
				sattr = spriteram16[sprite_voffs + 1];
				sxpos = spriteram16[sprite_voffs + 2];
				sypos = spriteram16[sprite_voffs + 3];
				logerror("$(%04x)  Tile-Attr-Xpos-Ypos Now:%04x %04x %04x.%01x %04x.%01x  nxt:%04x %04x %04x.%01x %04x.%01x\n", sprite_voffs,
											 schar, sattr, sxpos, size[( sattr>>6)&0x3f]&0xf, sypos,( size[( sattr>>6)&0x3f]>>4)&0xf,
											bschar,bsattr,bsxpos,bsize[(bsattr>>6)&0x3f]&0xf,bsypos,(bsize[(bsattr>>6)&0x3f]>>4)&0xf);
			}
		}
		else									/* SCU controller */
		{
			int schar,sattr,sxpos,sypos,bschar,bsattr,bsxpos,bsypos;
			logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
			logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",pf1_scrollx,pf1_scrolly,pf2_scrollx,pf2_scrolly,pf3_scrollx,pf3_scrolly,pf4_scrollx,pf4_scrolly);
			for ( sprite_voffs = 0; sprite_voffs < (machine->generic.spriteram_size/2); sprite_voffs += 4 )
			{
				bschar = buffered_spriteram16[sprite_voffs];
				bsattr = buffered_spriteram16[sprite_voffs + 1];
				bsypos = buffered_spriteram16[sprite_voffs + 2];
				bsxpos = buffered_spriteram16[sprite_voffs + 3];
				schar = spriteram16[sprite_voffs];
				sattr = spriteram16[sprite_voffs + 1];
				sypos = spriteram16[sprite_voffs + 2];
				sxpos = spriteram16[sprite_voffs + 3];
				logerror("$(%04x)  Tile-Attr-Xpos-Ypos Now:%04x %04x %04x %04x  nxt:%04x %04x %04x %04x\n", sprite_voffs,
											 schar, sattr, sxpos, sypos,
											bschar,bsattr,bsxpos, bsypos);
			}
		}
	}

	if ( input_code_pressed(machine, KEYCODE_SLASH) )
	{
		UINT16 *size  = (UINT16 *)(toaplan1_spritesizeram16);
		UINT16 *bsize = (UINT16 *)(toaplan1_buffered_spritesizeram16);
		offs_t offs;
		while (input_code_pressed(machine, KEYCODE_SLASH)) ;
		if (toaplan1_spritesizeram16)			/* FCU controller */
		{
			logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
			logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",pf1_scrollx,pf1_scrolly,pf2_scrollx,pf2_scrolly,pf3_scrollx,pf3_scrolly,pf4_scrollx,pf4_scrolly);
			for ( offs = 0; offs < (TOAPLAN1_SPRITESIZERAM_SIZE/2); offs +=4 )
			{
				logerror("SizeOffs:%04x   now:%04x %04x %04x %04x    next: %04x %04x %04x %04x\n", offs,
												bsize[offs+0], bsize[offs+1],
												bsize[offs+2], bsize[offs+3],
												size[offs+0], size[offs+1],
												size[offs+2], size[offs+3]);
			}
		}
	}

	if ( input_code_pressed(machine, KEYCODE_N) )
	{
		offs_t tile_voffs;
		int tchar[5], tattr[5];
		while (input_code_pressed(machine, KEYCODE_N)) ;	/* BCU controller */
		logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
		logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",pf1_scrollx,pf1_scrolly,pf2_scrollx,pf2_scrolly,pf3_scrollx,pf3_scrolly,pf4_scrollx,pf4_scrolly);
		for ( tile_voffs = 0; tile_voffs < (TOAPLAN1_TILEVRAM_SIZE/2); tile_voffs += 2 )
		{
			tchar[1] = pf1_tilevram16[tile_voffs + 1];
			tattr[1] = pf1_tilevram16[tile_voffs];
			tchar[2] = pf2_tilevram16[tile_voffs + 1];
			tattr[2] = pf2_tilevram16[tile_voffs];
			tchar[3] = pf3_tilevram16[tile_voffs + 1];
			tattr[3] = pf3_tilevram16[tile_voffs];
			tchar[4] = pf4_tilevram16[tile_voffs + 1];
			tattr[4] = pf4_tilevram16[tile_voffs];
//          logerror("PF3 offs:%04x   Tile:%04x  Attr:%04x\n", tile_voffs, tchar, tattr);
			logerror("$(%04x)  Attr-Tile PF1:%04x-%04x  PF2:%04x-%04x  PF3:%04x-%04x  PF4:%04x-%04x\n", tile_voffs,
									tattr[1], tchar[1],  tattr[2], tchar[2],
									tattr[3], tchar[3],  tattr[4], tchar[4]);
		}
	}

	if ( input_code_pressed(machine, KEYCODE_W) )
	{
		while (input_code_pressed(machine, KEYCODE_W)) ;
		logerror("Mark here\n");
	}
	if ( input_code_pressed(machine, KEYCODE_E) )
	{
		while (input_code_pressed(machine, KEYCODE_E)) ;
		displog += 1;
		displog &= 1;
	}
	if (displog)
	{
		logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
		logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",pf1_scrollx,pf1_scrolly,pf2_scrollx,pf2_scrolly,pf3_scrollx,pf3_scrolly,pf4_scrollx,pf4_scrolly);
	}
	if ( input_code_pressed(machine, KEYCODE_B) )
	{
//      while (input_code_pressed(machine, KEYCODE_B)) ;
		scrollx_offs1 += 0x1; scrollx_offs2 += 0x1; scrollx_offs3 += 0x1; scrollx_offs4 += 0x1;
		logerror("Scrollx_offs now = %08x\n",scrollx_offs4);
		tilemap_set_scrollx(pf1_tilemap,0,(pf1_scrollx >> 7) - (tiles_offsetx - scrollx_offs1));
		tilemap_set_scrollx(pf2_tilemap,0,(pf2_scrollx >> 7) - (tiles_offsetx - scrollx_offs2));
		tilemap_set_scrollx(pf3_tilemap,0,(pf3_scrollx >> 7) - (tiles_offsetx - scrollx_offs3));
		tilemap_set_scrollx(pf4_tilemap,0,(pf4_scrollx >> 7) - (tiles_offsetx - scrollx_offs4));
	}
	if ( input_code_pressed(machine, KEYCODE_V) )
	{
//      while (input_code_pressed(machine, KEYCODE_V)) ;
		scrollx_offs1 -= 0x1; scrollx_offs2 -= 0x1; scrollx_offs3 -= 0x1; scrollx_offs4 -= 0x1;
		logerror("Scrollx_offs now = %08x\n",scrollx_offs4);
		tilemap_set_scrollx(pf1_tilemap,0,(pf1_scrollx >> 7) - (tiles_offsetx - scrollx_offs1));
		tilemap_set_scrollx(pf2_tilemap,0,(pf2_scrollx >> 7) - (tiles_offsetx - scrollx_offs2));
		tilemap_set_scrollx(pf3_tilemap,0,(pf3_scrollx >> 7) - (tiles_offsetx - scrollx_offs3));
		tilemap_set_scrollx(pf4_tilemap,0,(pf4_scrollx >> 7) - (tiles_offsetx - scrollx_offs4));
	}
	if ( input_code_pressed(machine, KEYCODE_C) )
	{
//      while (input_code_pressed(machine, KEYCODE_C)) ;
		scrolly_offs += 0x1;
		logerror("Scrolly_offs now = %08x\n",scrolly_offs);
		tilemap_set_scrolly(pf1_tilemap,0,(pf1_scrolly >> 7) - (tiles_offsety - scrolly_offs));
		tilemap_set_scrolly(pf2_tilemap,0,(pf2_scrolly >> 7) - (tiles_offsety - scrolly_offs));
		tilemap_set_scrolly(pf3_tilemap,0,(pf3_scrolly >> 7) - (tiles_offsety - scrolly_offs));
		tilemap_set_scrolly(pf4_tilemap,0,(pf4_scrolly >> 7) - (tiles_offsety - scrolly_offs));
	}
	if ( input_code_pressed(machine, KEYCODE_X) )
	{
//      while (input_code_pressed(machine, KEYCODE_X)) ;
		scrolly_offs -= 0x1;
		logerror("Scrolly_offs now = %08x\n",scrolly_offs);
		tilemap_set_scrolly(pf1_tilemap,0,(pf1_scrolly >> 7) - (tiles_offsety - scrolly_offs));
		tilemap_set_scrolly(pf2_tilemap,0,(pf2_scrolly >> 7) - (tiles_offsety - scrolly_offs));
		tilemap_set_scrolly(pf3_tilemap,0,(pf3_scrolly >> 7) - (tiles_offsety - scrolly_offs));
		tilemap_set_scrolly(pf4_tilemap,0,(pf4_scrolly >> 7) - (tiles_offsety - scrolly_offs));
	}

	if ( input_code_pressed(machine, KEYCODE_L) )		/* Turn Playfield 4 on/off */
	{
		while (input_code_pressed(machine, KEYCODE_L)) ;
		display_pf4 += 1;
		display_pf4 &= 1;
		tilemap_set_enable(pf4_tilemap, display_pf4);
	}
	if ( input_code_pressed(machine, KEYCODE_K) )		/* Turn Playfield 3 on/off */
	{
		while (input_code_pressed(machine, KEYCODE_K)) ;
		display_pf3 += 1;
		display_pf3 &= 1;
		tilemap_set_enable(pf3_tilemap, display_pf3);
	}
	if ( input_code_pressed(machine, KEYCODE_J) )		/* Turn Playfield 2 on/off */
	{
		while (input_code_pressed(machine, KEYCODE_J)) ;
		display_pf2 += 1;
		display_pf2 &= 1;
		tilemap_set_enable(pf2_tilemap, display_pf2);
	}
	if ( input_code_pressed(machine, KEYCODE_H) )		/* Turn Playfield 1 on/off */
	{
		while (input_code_pressed(machine, KEYCODE_H)) ;
		display_pf1 += 1;
		display_pf1 &= 1;
		tilemap_set_enable(pf1_tilemap, display_pf1);
	}
#endif
}



/***************************************************************************
    Sprite Handlers
***************************************************************************/

// custom function to draw a single sprite. needed to keep correct sprites - sprites and sprites - tilemaps priorities
static void toaplan1_draw_sprite_custom(bitmap_t *dest_bmp,const rectangle *clip,const gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		int priority)
{
	int pal_base = gfx->color_base + gfx->color_granularity * (color % gfx->total_colors);
	const UINT8 *source_base = gfx_element_get_data(gfx, code % gfx->total_elements);
	bitmap_t *priority_bitmap = gfx->machine->priority_bitmap;
	int sprite_screen_height = ((1<<16)*gfx->height+0x8000)>>16;
	int sprite_screen_width = ((1<<16)*gfx->width+0x8000)>>16;

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

			for( y=sy; y<ey; y++ )
			{
				const UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
				UINT16 *dest = BITMAP_ADDR16(dest_bmp, y, 0);
				UINT8 *pri = BITMAP_ADDR8(priority_bitmap, y, 0);

				int x, x_index = x_index_base;
				for( x=sx; x<ex; x++ )
				{
					int c = source[x_index>>16];
					if( c != 0 )
					{
						if (pri[x] < priority)
							dest[x] = pal_base+c;
						pri[x] = 0xff; // mark it "already drawn"
					}
					x_index += dx;
				}

				y_index += dy;
			}
		}
	}
}


static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT16 *source = (UINT16 *)(machine->generic.buffered_spriteram.u16);
	UINT16 *size   = (UINT16 *)(toaplan1_buffered_spritesizeram16);

	int offs;

	for (offs = machine->generic.spriteram_size/2 - 4; offs >= 0; offs -= 4)
	{
		if (!(source[offs] & 0x8000))
		{
			int attrib, sprite, color, priority, sx, sy;
			int sprite_sizex, sprite_sizey, dim_x, dim_y, sx_base, sy_base;
			int sizeram_ptr;

			attrib = source[offs+1];
			priority = (attrib & 0xf000) >> 12;

			sprite = source[offs] & 0x7fff;
			color = attrib & 0x3f;

			/****** find sprite dimension ******/
			sizeram_ptr = (attrib >> 6) & 0x3f;
			sprite_sizex = ( size[sizeram_ptr]       & 0x0f) * 8;
			sprite_sizey = ((size[sizeram_ptr] >> 4) & 0x0f) * 8;

			/****** find position to display sprite ******/
			sx_base = (source[offs + 2] >> 7) & 0x1ff;
			sy_base = (source[offs + 3] >> 7) & 0x1ff;

			if (sx_base >= 0x180) sx_base -= 0x200;
			if (sy_base >= 0x180) sy_base -= 0x200;

			/****** flip the sprite layer ******/
			if (fcu_flipscreen)
			{
				const rectangle *visarea = video_screen_get_visible_area(machine->primary_screen);

				sx_base = ((visarea->max_x + 1) - visarea->min_x) - (sx_base + 8);	/* visarea.x = 320 */
				sy_base = ((visarea->max_y + 1) - visarea->min_y) - (sy_base + 8);	/* visarea.y = 240 */
				sy_base += ((visarea->max_y + 1) - ((visarea->max_y + 1) - visarea->min_y)) * 2;	/* Horizontal games are offset so adjust by +0x20 */
			}

			for (dim_y = 0; dim_y < sprite_sizey; dim_y += 8)
			{
				if (fcu_flipscreen) sy = sy_base - dim_y;
				else                sy = sy_base + dim_y;

				for (dim_x = 0; dim_x < sprite_sizex; dim_x += 8)
				{
					if (fcu_flipscreen) sx = sx_base - dim_x;
					else                sx = sx_base + dim_x;

					toaplan1_draw_sprite_custom(bitmap,cliprect,machine->gfx[1],
							                   sprite,color,
							                   fcu_flipscreen,fcu_flipscreen,
							                   sx,sy,
							                   priority);

					sprite++ ;
				}
			}
		}
	}
}


static void rallybik_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority )
{
	UINT16 *buffered_spriteram16 = machine->generic.buffered_spriteram.u16;
	int offs;

	for (offs = 0; offs < (machine->generic.spriteram_size/2); offs += 4)
	{
		int attrib, sx, sy, flipx, flipy;
		int sprite, color;

		attrib = buffered_spriteram16[offs + 1];
		if ((attrib & 0x0c00) == priority)
		{
			sy = (buffered_spriteram16[offs + 3] >> 7) & 0x1ff;
			if (sy != 0x0100)		/* sx = 0x01a0 or 0x0040*/
			{
				sprite = buffered_spriteram16[offs] & 0x7ff;
				color  = attrib & 0x3f;
				sx = (buffered_spriteram16[offs + 2] >> 7) & 0x1ff;
				flipx = attrib & 0x100;
				if (flipx) sx -= 15;
				flipy = attrib & 0x200;
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					sprite,
					color,
					flipx,flipy,
					sx-31,sy-16,0);
			}
		}
	}
}


/***************************************************************************
    Draw the game screen in the given bitmap_t.
***************************************************************************/

VIDEO_UPDATE( rallybik )
{
	int priority;

	toaplan1_log_vram(screen->machine);

	bitmap_fill(bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,pf1_tilemap,TILEMAP_DRAW_OPAQUE | 0,0);
	tilemap_draw(bitmap,cliprect,pf1_tilemap,TILEMAP_DRAW_OPAQUE | 1,0);

	for (priority = 1; priority < 16; priority++)
	{
		tilemap_draw(bitmap,cliprect,pf4_tilemap,priority,0);
		tilemap_draw(bitmap,cliprect,pf3_tilemap,priority,0);
		tilemap_draw(bitmap,cliprect,pf2_tilemap,priority,0);
		tilemap_draw(bitmap,cliprect,pf1_tilemap,priority,0);
		rallybik_draw_sprites(screen->machine, bitmap,cliprect,priority << 8);
	}
	return 0;
}

VIDEO_UPDATE( toaplan1 )
{
	int priority;

	toaplan1_log_vram(screen->machine);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,0x120);

	tilemap_draw(bitmap,cliprect,pf4_tilemap,TILEMAP_DRAW_OPAQUE,0);
	for (priority = 8; priority < 16; priority++)
		tilemap_draw(bitmap,cliprect,pf1_tilemap,TILEMAP_DRAW_OPAQUE | priority,0);

	for (priority = 1; priority < 16; priority++)
	{
		tilemap_draw_primask(bitmap,cliprect,pf4_tilemap,priority,priority,0);
		tilemap_draw_primask(bitmap,cliprect,pf3_tilemap,priority,priority,0);
		tilemap_draw_primask(bitmap,cliprect,pf2_tilemap,priority,priority,0);
		tilemap_draw_primask(bitmap,cliprect,pf1_tilemap,priority,priority,0);
	}

	draw_sprites(screen->machine, bitmap,cliprect);
	return 0;
}

VIDEO_UPDATE( demonwld )
{
	int priority;

	toaplan1_log_vram(screen->machine);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,0x120);

	tilemap_draw(bitmap,cliprect,pf1_tilemap,TILEMAP_DRAW_OPAQUE | 0,0);
	tilemap_draw(bitmap,cliprect,pf1_tilemap,TILEMAP_DRAW_OPAQUE | 1,0);

	for (priority = 1; priority < 16; priority++)
	{
		tilemap_draw_primask(bitmap,cliprect,pf4_tilemap,priority,priority,0);
		tilemap_draw_primask(bitmap,cliprect,pf3_tilemap,priority,priority,0);
		tilemap_draw_primask(bitmap,cliprect,pf2_tilemap,priority,priority,0);
		tilemap_draw_primask(bitmap,cliprect,pf1_tilemap,priority,priority,0);
	}

	draw_sprites(screen->machine, bitmap,cliprect);
	return 0;
}


/****************************************************************************
    Spriteram is always 1 frame ahead, suggesting spriteram buffering.
    There are no CPU output registers that control this so we
    assume it happens automatically every frame, at the end of vblank
****************************************************************************/

VIDEO_EOF( rallybik )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	buffer_spriteram16_w(space, 0, 0, 0xffff);
}

VIDEO_EOF( toaplan1 )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	buffer_spriteram16_w(space, 0, 0, 0xffff);
	memcpy(toaplan1_buffered_spritesizeram16, toaplan1_spritesizeram16, TOAPLAN1_SPRITESIZERAM_SIZE);
}

VIDEO_EOF( samesame )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	buffer_spriteram16_w(space, 0, 0, 0xffff);
	memcpy(toaplan1_buffered_spritesizeram16, toaplan1_spritesizeram16, TOAPLAN1_SPRITESIZERAM_SIZE);
	cputag_set_input_line(machine, "maincpu", M68K_IRQ_2, HOLD_LINE);	/* Frame done */
}
